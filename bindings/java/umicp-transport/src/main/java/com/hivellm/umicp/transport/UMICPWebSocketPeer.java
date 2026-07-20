package com.hivellm.umicp.transport;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.types.ConnectionException;
import com.hivellm.umicp.types.OperationType;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.InetSocketAddress;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.*;
import java.util.stream.Collectors;

/**
 * Multiplexed WebSocket peer (server + multiple clients).
 *
 * <p>This is the key class that enables true peer-to-peer communication where
 * each peer can BOTH accept incoming connections AND connect to multiple remote peers.
 */
public class UMICPWebSocketPeer implements AutoCloseable {

    private static final Logger logger = LoggerFactory.getLogger(UMICPWebSocketPeer.class);

    private final String peerId;
    private final PeerOptions options;
    private final ConcurrentHashMap<String, PeerConnection> peers;
    private final ConcurrentHashMap<String, UMICPWebSocketClient> outgoingClients;
    private final CopyOnWriteArrayList<PeerEventListener> listeners;
    private final ScheduledExecutorService scheduler;

    private UMICPWebSocketServer server;

    public UMICPWebSocketPeer(@NotNull String peerId) {
        this(PeerOptions.builder(peerId).build());
    }

    public UMICPWebSocketPeer(@NotNull PeerOptions options) {
        this.peerId = options.getPeerId();
        this.options = options;
        this.peers = new ConcurrentHashMap<>();
        this.outgoingClients = new ConcurrentHashMap<>();
        this.listeners = new CopyOnWriteArrayList<>();
        this.scheduler = Executors.newScheduledThreadPool(2);
    }

    // ==================== Server Component ====================

    @NotNull
    public CompletableFuture<Void> startServer(int port) {
        return startServer(new InetSocketAddress(port));
    }

    @NotNull
    public CompletableFuture<Void> startServer(@NotNull InetSocketAddress address) {
        return CompletableFuture.runAsync(() -> {
            try {
                server = new UMICPWebSocketServer(address.getPort(), options.getServerOptions());

                server.addEventListener(new ServerEventListener() {
                    @Override
                    public void onServerStarted() {
                        emitServerReady();
                    }

                    @Override
                    public void onClientConnected(ClientConnection client) {
                        handleIncomingConnection(client);
                    }

                    @Override
                    public void onClientDisconnected(ClientConnection client, int code, String reason) {
                        handleIncomingDisconnection(client);
                    }

                    @Override
                    public void onMessage(Envelope envelope, ClientConnection client) {
                        handleIncomingMessage(envelope, client);
                    }

                    @Override
                    public void onError(Exception error, ClientConnection client) {
                        handleIncomingError(error, client);
                    }
                });

                server.start().get();
            } catch (Exception e) {
                logger.error("Failed to start server", e);
                throw new CompletionException(e);
            }
        });
    }

    // ==================== Client Component ====================

    @NotNull
    public CompletableFuture<String> connectToPeer(@NotNull String url) {
        return connectToPeer(url, new HashMap<>());
    }

    @NotNull
    public CompletableFuture<String> connectToPeer(@NotNull String url,
                                                   @NotNull Map<String, String> metadata) {
        return CompletableFuture.supplyAsync(() -> {
            try {
                logger.info("Connecting to peer: {}", url);

                UMICPWebSocketClient client = new UMICPWebSocketClient(url, options.getClientOptions());
                String connectionId = UUID.randomUUID().toString();

                PeerConnection peerConn = new PeerConnection(
                    connectionId,
                    PeerConnection.PeerType.OUTGOING,
                    url,
                    metadata
                );

                client.addEventListener(new TransportEventListener() {
                    @Override
                    public void onConnected() {
                        handleOutgoingConnection(peerConn, client);
                    }

                    @Override
                    public void onDisconnected(int code, String reason) {
                        handleOutgoingDisconnection(peerConn);
                    }

                    @Override
                    public void onMessage(Envelope envelope) {
                        handleOutgoingMessage(envelope, peerConn);
                    }

                    @Override
                    public void onError(Exception error) {
                        handleOutgoingError(error, peerConn);
                    }
                });

                client.connect().get();
                outgoingClients.put(connectionId, client);
                peers.put(connectionId, peerConn);

                if (options.isAutoProtocol()) {
                    sendHello(peerConn);
                }

                return connectionId;

            } catch (Exception e) {
                logger.error("Failed to connect to peer: {}", url, e);
                throw new CompletionException(e);
            }
        });
    }

    @NotNull
    public CompletableFuture<Boolean> disconnectPeer(@NotNull String peerId) {
        return CompletableFuture.supplyAsync(() -> {
            PeerConnection peer = peers.remove(peerId);
            if (peer == null) {
                return false;
            }

            if (peer.getType() == PeerConnection.PeerType.OUTGOING) {
                UMICPWebSocketClient client = outgoingClients.remove(peerId);
                if (client != null) {
                    try {
                        client.disconnect().get();
                    } catch (Exception e) {
                        logger.error("Error disconnecting peer", e);
                    }
                }
            }

            emitPeerDisconnect(peer);
            return true;
        });
    }

    // ==================== Message Sending ====================

    @NotNull
    public CompletableFuture<Boolean> sendToPeer(@NotNull String peerId, @NotNull Envelope envelope) {
        return CompletableFuture.supplyAsync(() -> {
            PeerConnection peer = peers.get(peerId);
            if (peer == null) {
                logger.warn("Peer not found: {}", peerId);
                return false;
            }

            try {
                if (peer.getType() == PeerConnection.PeerType.OUTGOING) {
                    UMICPWebSocketClient client = outgoingClients.get(peerId);
                    if (client != null) {
                        return client.send(envelope).get();
                    }
                } else if (peer.getType() == PeerConnection.PeerType.INCOMING && server != null) {
                    return server.send(envelope, peerId).get();
                }
                return false;
            } catch (Exception e) {
                logger.error("Failed to send to peer {}", peerId, e);
                return false;
            }
        });
    }

    @NotNull
    public CompletableFuture<Integer> broadcast(@NotNull Envelope envelope) {
        return broadcast(envelope, null);
    }

    @NotNull
    public CompletableFuture<Integer> broadcast(@NotNull Envelope envelope, @Nullable String excludePeerId) {
        return CompletableFuture.supplyAsync(() -> {
            int count = 0;
            for (Map.Entry<String, PeerConnection> entry : peers.entrySet()) {
                if (excludePeerId != null && entry.getKey().equals(excludePeerId)) {
                    continue;
                }
                try {
                    if (sendToPeer(entry.getKey(), envelope).get()) {
                        count++;
                    }
                } catch (Exception e) {
                    logger.error("Failed to broadcast to peer {}", entry.getKey(), e);
                }
            }
            return count;
        });
    }

    @NotNull
    public CompletableFuture<Integer> broadcastToType(@NotNull Envelope envelope,
                                                      @NotNull PeerConnection.PeerType type) {
        return broadcastToType(envelope, type, null);
    }

    @NotNull
    public CompletableFuture<Integer> broadcastToType(@NotNull Envelope envelope,
                                                      @NotNull PeerConnection.PeerType type,
                                                      @Nullable String excludePeerId) {
        return CompletableFuture.supplyAsync(() -> {
            List<String> targetPeers = peers.entrySet().stream()
                .filter(e -> e.getValue().getType() == type)
                .filter(e -> excludePeerId == null || !e.getKey().equals(excludePeerId))
                .map(Map.Entry::getKey)
                .collect(Collectors.toList());

            int count = 0;
            for (String id : targetPeers) {
                try {
                    if (sendToPeer(id, envelope).get()) {
                        count++;
                    }
                } catch (Exception e) {
                    logger.error("Failed to broadcast to peer {}", id, e);
                }
            }
            return count;
        });
    }

    // ==================== Peer Management ====================

    @NotNull
    public List<PeerConnection> getPeers() {
        return new ArrayList<>(peers.values());
    }

    @NotNull
    public List<PeerConnection> getPeersByType(@NotNull PeerConnection.PeerType type) {
        return peers.values().stream()
            .filter(p -> p.getType() == type)
            .collect(Collectors.toList());
    }

    @NotNull
    public Optional<PeerConnection> getPeer(@NotNull String peerId) {
        return Optional.ofNullable(peers.get(peerId));
    }

    @NotNull
    public PeerStats getStats() {
        int totalPeers = peers.size();
        long incoming = peers.values().stream().filter(p -> p.getType() == PeerConnection.PeerType.INCOMING).count();
        long outgoing = peers.values().stream().filter(p -> p.getType() == PeerConnection.PeerType.OUTGOING).count();

        return new PeerStats(totalPeers, (int)incoming, (int)outgoing, 0, 0, new HashMap<>());
    }

    public void addEventListener(@NotNull PeerEventListener listener) {
        listeners.add(listener);
    }

    public void removeEventListener(@NotNull PeerEventListener listener) {
        listeners.remove(listener);
    }

    // ==================== Handshake Protocol ====================

    private void sendHello(PeerConnection peer) {
        Map<String, Object> caps = new HashMap<>();
        caps.put("type", "hello");
        caps.put("peer_id", peerId);
        caps.put("version", "1.0");
        caps.putAll(peer.getMetadata());

        Envelope hello = new Envelope()
            .setFrom(peerId)
            .setTo("*")
            .setOperation(OperationType.CONTROL)
            .setMessageId(UUID.randomUUID().toString())
            .setCapabilities(caps);

        sendToPeer(peer.getId(), hello);
        scheduleHandshakeTimeout(peer);
    }

    /** Envelope capabilities are Map&lt;String,Object&gt;; PeerInfo/handshake use Map&lt;String,String&gt;. */
    private static Map<String, String> toStringMap(Map<String, Object> source) {
        Map<String, String> result = new HashMap<>();
        if (source != null) {
            source.forEach((k, v) -> result.put(k, v == null ? null : String.valueOf(v)));
        }
        return result;
    }

    private void handleHello(Envelope envelope, PeerConnection peer) {
        Map<String, String> caps = toStringMap(envelope.getCapabilities());
        String remotePeerId = caps.get("peer_id");

        PeerInfo info = new PeerInfo(remotePeerId, new HashMap<>(caps), caps, Instant.now());
        peer.setPeerInfo(info);
        peer.setHandshakeComplete(true);

        // Send ACK
        Map<String, Object> ackCaps = new HashMap<>();
        ackCaps.put("type", "ack");
        ackCaps.put("peer_id", peerId);
        ackCaps.put("version", "1.0");

        Envelope ack = new Envelope()
            .setFrom(peerId)
            .setTo(remotePeerId)
            .setOperation(OperationType.ACK)
            .setMessageId(UUID.randomUUID().toString())
            .setCapabilities(ackCaps);

        sendToPeer(peer.getId(), ack);
        emitPeerReady(peer, info);
    }

    private void handleAck(Envelope envelope, PeerConnection peer) {
        Map<String, String> caps = toStringMap(envelope.getCapabilities());
        String remotePeerId = caps.get("peer_id");

        PeerInfo info = new PeerInfo(remotePeerId, new HashMap<>(caps), caps, Instant.now());
        peer.setPeerInfo(info);
        peer.setHandshakeComplete(true);

        emitPeerReady(peer, info);
        if (peer.getUrl() != null) {
            emitConnected(peer.getId(), peer.getUrl());
        }
    }

    private void scheduleHandshakeTimeout(PeerConnection peer) {
        scheduler.schedule(() -> {
            if (!peer.isHandshakeComplete()) {
                logger.warn("Handshake timeout for peer {}", peer.getId());
                emitHandshakeTimeout(peer);
                disconnectPeer(peer.getId());
            }
        }, options.getHandshakeTimeout().toMillis(), TimeUnit.MILLISECONDS);
    }

    // ==================== Incoming Connection Handlers ====================

    private void handleIncomingConnection(ClientConnection client) {
        PeerConnection peer = new PeerConnection(
            client.getId(),
            PeerConnection.PeerType.INCOMING,
            null,
            client.getMetadata()
        );
        peers.put(client.getId(), peer);
        emitPeerConnect(peer);

        if (options.isAutoProtocol()) {
            sendHello(peer);
        }
    }

    private void handleIncomingDisconnection(ClientConnection client) {
        PeerConnection peer = peers.remove(client.getId());
        if (peer != null) {
            emitPeerDisconnect(peer);
        }
    }

    private void handleIncomingMessage(Envelope envelope, ClientConnection client) {
        PeerConnection peer = peers.get(client.getId());
        if (peer == null) return;

        emitMessage(envelope, peer);

        if (envelope.getOperation() == OperationType.CONTROL) {
            String type = (String) envelope.getCapabilities().get("type");
            if ("hello".equals(type)) {
                handleHello(envelope, peer);
            } else if ("ack".equals(type)) {
                handleAck(envelope, peer);
            }
        } else if (envelope.getOperation() == OperationType.DATA) {
            emitData(envelope, peer);
        }
    }

    private void handleIncomingError(Exception error, ClientConnection client) {
        PeerConnection peer = peers.get(client.getId());
        if (peer != null) {
            emitError(error, peer);
        }
    }

    // ==================== Outgoing Connection Handlers ====================

    private void handleOutgoingConnection(PeerConnection peer, UMICPWebSocketClient client) {
        emitPeerConnect(peer);

        if (options.isAutoProtocol()) {
            sendHello(peer);
        }
    }

    private void handleOutgoingDisconnection(PeerConnection peer) {
        peers.remove(peer.getId());
        outgoingClients.remove(peer.getId());
        emitPeerDisconnect(peer);
    }

    private void handleOutgoingMessage(Envelope envelope, PeerConnection peer) {
        emitMessage(envelope, peer);

        if (envelope.getOperation() == OperationType.CONTROL) {
            String type = (String) envelope.getCapabilities().get("type");
            if ("hello".equals(type)) {
                handleHello(envelope, peer);
            } else if ("ack".equals(type)) {
                handleAck(envelope, peer);
            }
        } else if (envelope.getOperation() == OperationType.DATA) {
            emitData(envelope, peer);
        }
    }

    private void handleOutgoingError(Exception error, PeerConnection peer) {
        emitError(error, peer);
    }

    // ==================== Event Emission ====================

    private void emitServerReady() {
        for (PeerEventListener listener : listeners) {
            try {
                listener.onServerReady();
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitPeerConnect(PeerConnection peer) {
        for (PeerEventListener listener : listeners) {
            try {
                listener.onPeerConnect(peer);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitPeerDisconnect(PeerConnection peer) {
        for (PeerEventListener listener : listeners) {
            try {
                listener.onPeerDisconnect(peer);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitMessage(Envelope envelope, PeerConnection peer) {
        for (PeerEventListener listener : listeners) {
            try {
                listener.onMessage(envelope, peer);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitData(Envelope envelope, PeerConnection peer) {
        for (PeerEventListener listener : listeners) {
            try {
                listener.onData(envelope, peer);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitPeerReady(PeerConnection peer, PeerInfo info) {
        for (PeerEventListener listener : listeners) {
            try {
                listener.onPeerReady(peer, info);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitHandshakeTimeout(PeerConnection peer) {
        for (PeerEventListener listener : listeners) {
            try {
                listener.onHandshakeTimeout(peer);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitError(Exception error, PeerConnection peer) {
        for (PeerEventListener listener : listeners) {
            try {
                listener.onError(error, peer);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitConnected(String peerId, String url) {
        for (PeerEventListener listener : listeners) {
            try {
                listener.onConnected(peerId, url);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    @Override
    public void close() throws Exception {
        logger.info("Closing multiplexed peer");

        // Disconnect all outgoing clients
        for (UMICPWebSocketClient client : outgoingClients.values()) {
            try {
                client.close();
            } catch (Exception e) {
                logger.error("Error closing client", e);
            }
        }

        // Stop server
        if (server != null) {
            server.close();
        }

        scheduler.shutdown();
        if (!scheduler.awaitTermination(5, TimeUnit.SECONDS)) {
            scheduler.shutdownNow();
        }
    }
}

