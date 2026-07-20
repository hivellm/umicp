package com.hivellm.umicp.transport;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.types.SerializationException;
import org.java_websocket.WebSocket;
import org.java_websocket.handshake.ClientHandshake;
import org.java_websocket.server.WebSocketServer;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.function.Predicate;

/**
 * WebSocket server for UMICP communication.
 *
 * <p>This class provides WebSocket server functionality with client management,
 * broadcasting, and event-driven architecture.
 *
 * <p>Example usage:
 * <pre>{@code
 * ServerOptions options = ServerOptions.builder()
 *     .maxClients(1000)
 *     .compression(true)
 *     .build();
 *
 * UMICPWebSocketServer server = new UMICPWebSocketServer(8080, options);
 *
 * // Add event listener
 * server.addEventListener(new ServerEventListener() {
 *     @Override
 *     public void onMessage(Envelope envelope, ClientConnection client) {
 *         System.out.println("Received from " + client.getId());
 *
 *         // Echo back
 *         server.send(envelope, client.getId());
 *     }
 * });
 *
 * // Start server
 * server.start().get();
 *
 * // Broadcast message
 * Envelope broadcast = new Envelope()...;
 * server.broadcast(broadcast).get();
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see ServerOptions
 * @see ServerEventListener
 */
public class UMICPWebSocketServer implements AutoCloseable {

    private static final Logger logger = LoggerFactory.getLogger(UMICPWebSocketServer.class);

    private final int port;
    private final ServerOptions options;
    private final TransportStats stats;
    private final ConcurrentHashMap<String, ClientConnection> clients;
    private final ConcurrentHashMap<WebSocket, ClientConnection> socketToClient;
    private final CopyOnWriteArrayList<ServerEventListener> listeners;

    private WebSocketServer wsServer;

    /**
     * Creates a new WebSocket server with default options.
     *
     * @param port the port to listen on
     */
    public UMICPWebSocketServer(int port) {
        this(port, ServerOptions.defaults());
    }

    /**
     * Creates a new WebSocket server with the given options.
     *
     * @param port the port to listen on
     * @param options the server options
     */
    public UMICPWebSocketServer(int port, @NotNull ServerOptions options) {
        this.port = port;
        this.options = options;
        this.stats = new TransportStats();
        this.clients = new ConcurrentHashMap<>();
        this.socketToClient = new ConcurrentHashMap<>();
        this.listeners = new CopyOnWriteArrayList<>();

        initializeWebSocketServer();
    }

    /**
     * Initializes the WebSocket server.
     */
    private void initializeWebSocketServer() {
        InetSocketAddress address = new InetSocketAddress(port);

        wsServer = new WebSocketServer(address) {
            @Override
            public void onOpen(WebSocket conn, ClientHandshake handshake) {
                handleClientConnected(conn, handshake);
            }

            @Override
            public void onClose(WebSocket conn, int code, String reason, boolean remote) {
                handleClientDisconnected(conn, code, reason, remote);
            }

            @Override
            public void onMessage(WebSocket conn, String message) {
                handleClientMessage(conn, message);
            }

            @Override
            public void onError(WebSocket conn, Exception ex) {
                handleError(conn, ex);
            }

            @Override
            public void onStart() {
                handleServerStarted();
            }
        };
    }

    /**
     * Starts the WebSocket server.
     *
     * @return a CompletableFuture that completes when the server is started
     */
    @NotNull
    public CompletableFuture<Void> start() {
        return CompletableFuture.runAsync(() -> {
            logger.info("Starting WebSocket server on port {}", port);
            wsServer.start();
            logger.info("WebSocket server started on port {}", port);
        });
    }

    /**
     * Stops the WebSocket server.
     *
     * @return a CompletableFuture that completes when the server is stopped
     */
    @NotNull
    public CompletableFuture<Void> stop() {
        return CompletableFuture.runAsync(() -> {
            try {
                logger.info("Stopping WebSocket server");
                wsServer.stop(1000);
                logger.info("WebSocket server stopped");
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                logger.error("Server stop interrupted", e);
            }
        });
    }

    /**
     * Sends an envelope to a specific client.
     *
     * @param envelope the envelope to send
     * @param clientId the client ID
     * @return a CompletableFuture that completes when sent
     */
    @NotNull
    public CompletableFuture<Boolean> send(@NotNull Envelope envelope, @NotNull String clientId) {
        return CompletableFuture.supplyAsync(() -> {
            ClientConnection client = clients.get(clientId);
            if (client == null) {
                logger.warn("Client not found: {}", clientId);
                return false;
            }

            try {
                String json = envelope.serialize();
                WebSocket socket = findSocketForClient(client);

                if (socket != null && socket.isOpen()) {
                    socket.send(json);
                    client.recordMessageSent(json.length());
                    stats.recordMessageSent(json.length());
                    logger.debug("Sent message to client {}", clientId);
                    return true;
                }

                logger.warn("Socket not open for client {}", clientId);
                return false;

            } catch (SerializationException e) {
                logger.error("Failed to serialize envelope", e);
                stats.recordError();
                return false;
            }
        });
    }

    /**
     * Broadcasts an envelope to all connected clients.
     *
     * @param envelope the envelope to broadcast
     * @return a CompletableFuture that completes with the number of clients reached
     */
    @NotNull
    public CompletableFuture<Integer> broadcast(@NotNull Envelope envelope) {
        return broadcast(envelope, null);
    }

    /**
     * Broadcasts an envelope to clients matching the filter.
     *
     * @param envelope the envelope to broadcast
     * @param filter predicate to filter clients (null = all clients)
     * @return a CompletableFuture that completes with the number of clients reached
     */
    @NotNull
    public CompletableFuture<Integer> broadcast(@NotNull Envelope envelope,
                                                @Nullable Predicate<ClientConnection> filter) {
        return CompletableFuture.supplyAsync(() -> {
            try {
                String json = envelope.serialize();
                int count = 0;

                for (ClientConnection client : clients.values()) {
                    if (filter == null || filter.test(client)) {
                        WebSocket socket = findSocketForClient(client);
                        if (socket != null && socket.isOpen()) {
                            socket.send(json);
                            client.recordMessageSent(json.length());
                            count++;
                        }
                    }
                }

                stats.recordMessageSent(json.length() * count);
                logger.debug("Broadcasted to {} clients", count);
                return count;

            } catch (SerializationException e) {
                logger.error("Failed to serialize envelope for broadcast", e);
                stats.recordError();
                return 0;
            }
        });
    }

    /**
     * Returns all connected clients.
     *
     * @return list of client connections
     */
    @NotNull
    public List<ClientConnection> getClients() {
        return new ArrayList<>(clients.values());
    }

    /**
     * Returns a specific client by ID.
     *
     * @param clientId the client ID
     * @return the client connection, if found
     */
    @NotNull
    public Optional<ClientConnection> getClient(@NotNull String clientId) {
        return Optional.ofNullable(clients.get(clientId));
    }

    /**
     * Returns the number of connected clients.
     *
     * @return client count
     */
    public int getClientCount() {
        return clients.size();
    }

    /**
     * Returns transport statistics.
     *
     * @return the transport statistics
     */
    @NotNull
    public TransportStats getStats() {
        return stats;
    }

    /**
     * Adds a server event listener.
     *
     * @param listener the event listener
     */
    public void addEventListener(@NotNull ServerEventListener listener) {
        listeners.add(listener);
    }

    /**
     * Removes a server event listener.
     *
     * @param listener the event listener
     */
    public void removeEventListener(@NotNull ServerEventListener listener) {
        listeners.remove(listener);
    }

    // ==================== Event Handlers ====================

    private void handleClientConnected(WebSocket conn, ClientHandshake handshake) {
        String remoteAddress = conn.getRemoteSocketAddress().toString();
        ClientConnection client = new ClientConnection(remoteAddress);

        clients.put(client.getId(), client);
        socketToClient.put(conn, client);

        logger.info("Client connected: {} from {}", client.getId(), remoteAddress);
        emitClientConnected(client);
    }

    private void handleClientDisconnected(WebSocket conn, int code, String reason, boolean remote) {
        ClientConnection client = socketToClient.remove(conn);
        if (client != null) {
            clients.remove(client.getId());
            logger.info("Client disconnected: {} (code: {}, reason: {})",
                       client.getId(), code, reason);
            emitClientDisconnected(client, code, reason);
        }
    }

    private void handleClientMessage(WebSocket conn, String message) {
        ClientConnection client = socketToClient.get(conn);
        if (client == null) {
            logger.warn("Received message from unknown client");
            return;
        }

        try {
            client.recordMessageReceived(message.length());
            stats.recordMessageReceived(message.length());

            Envelope envelope = Envelope.deserialize(message);
            emitMessage(envelope, client);

        } catch (SerializationException e) {
            logger.error("Failed to deserialize message from client {}", client.getId(), e);
            stats.recordError();
            emitError(e, client);
        }
    }

    private void handleError(WebSocket conn, Exception ex) {
        ClientConnection client = socketToClient.get(conn);
        logger.error("WebSocket error for client {}",
                    client != null ? client.getId() : "unknown", ex);
        stats.recordError();
        emitError(ex, client);
    }

    private void handleServerStarted() {
        logger.info("WebSocket server started on port {}", port);
        emitServerStarted();
    }

    // ==================== Helper Methods ====================

    private WebSocket findSocketForClient(ClientConnection client) {
        for (var entry : socketToClient.entrySet()) {
            if (entry.getValue().equals(client)) {
                return entry.getKey();
            }
        }
        return null;
    }

    // ==================== Event Emission ====================

    private void emitServerStarted() {
        for (ServerEventListener listener : listeners) {
            try {
                listener.onServerStarted();
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitClientConnected(ClientConnection client) {
        for (ServerEventListener listener : listeners) {
            try {
                listener.onClientConnected(client);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitClientDisconnected(ClientConnection client, int code, String reason) {
        for (ServerEventListener listener : listeners) {
            try {
                listener.onClientDisconnected(client, code, reason);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitMessage(Envelope envelope, ClientConnection client) {
        for (ServerEventListener listener : listeners) {
            try {
                listener.onMessage(envelope, client);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    private void emitError(Exception error, ClientConnection client) {
        for (ServerEventListener listener : listeners) {
            try {
                listener.onError(error, client);
            } catch (Exception e) {
                logger.error("Error in event listener", e);
            }
        }
    }

    /**
     * Closes the server and releases resources.
     */
    @Override
    public void close() throws Exception {
        logger.info("Closing WebSocket server");
        stop().get();
    }
}

