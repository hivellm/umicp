package com.hivellm.umicp.examples;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.core.UMICP;
import com.hivellm.umicp.types.OperationType;
import com.hivellm.umicp.transport.UMICPWebSocketPeer;
import com.hivellm.umicp.transport.PeerConnection;
import com.hivellm.umicp.transport.PeerInfo;
import com.hivellm.umicp.transport.PeerOptions;
import com.hivellm.umicp.transport.PeerStats;
import com.hivellm.umicp.events.PeerEventListener;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

/**
 * Multiplexed peer example demonstrating UMICP peer-to-peer communication.
 *
 * This example shows how to:
 * - Create multiplexed peers (server + multiple clients)
 * - Connect peers to each other
 * - Send and receive messages
 * - Broadcast messages to all peers
 * - Handle events (connection, disconnection, messages)
 * - Get peer statistics
 *
 * Network topology:
 * <pre>
 *   Peer A (port 20081) ←→ Peer B (port 20082) ←→ Peer C (port 20083)
 *        ↑                                                    ↓
 *        └────────────────────────────────────────────────────┘
 * </pre>
 *
 * NOTE: This is a planned example. Implementation is not yet complete.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
public class MultiplexedPeerExample {

    public static void main(String[] args) {
        System.out.println("=== UMICP Multiplexed Peer Example ===\n");

        try {
            // Create three peers forming a ring topology
            runRingTopologyExample();

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }

    /**
     * Ring topology example: A → B → C → A
     */
    private static void runRingTopologyExample() throws Exception {
        // Create Peer A
        PeerOptions optionsA = PeerOptions.builder()
            .peerId("peer-a")
            .autoProtocol(true)
            .handshakeTimeout(10, TimeUnit.SECONDS)
            .build();

        try (UMICPWebSocketPeer peerA = new UMICPWebSocketPeer(optionsA)) {

            // Start server on Peer A
            peerA.startServer(20081).get();
            System.out.println("Peer A server started on port 20081");

            // Add event listener to Peer A
            peerA.addEventListener(new PeerEventListener() {
                @Override
                public void onMessage(Envelope envelope, PeerConnection connection) {
                    System.out.println("[Peer A] Received message from: " + connection.getId());
                    System.out.println("         Content: " + envelope.getCapabilities().get("message"));

                    // Echo response
                    Envelope response = new Envelope()
                        .setFrom("peer-a")
                        .setTo(envelope.getFrom())
                        .setOperation(OperationType.ACK)
                        .setMessageId("ack-" + System.currentTimeMillis())
                        .setCapabilities(Map.of("status", "received", "original-id", envelope.getMessageId()));

                    try {
                        peerA.sendToPeer(connection.getId(), response).get();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void onPeerConnect(PeerConnection connection) {
                    System.out.println("[Peer A] Peer connected: " + connection.getId() +
                                     " (" + connection.getType() + ")");
                }

                @Override
                public void onPeerDisconnect(PeerConnection connection) {
                    System.out.println("[Peer A] Peer disconnected: " + connection.getId());
                }

                @Override
                public void onPeerReady(PeerConnection connection, PeerInfo info) {
                    System.out.println("[Peer A] Peer ready: " + info.getPeerId());
                    System.out.println("         Metadata: " + info.getMetadata());
                }

                @Override
                public void onError(Exception error, PeerConnection connection) {
                    System.err.println("[Peer A] Error: " + error.getMessage());
                }
            });

            // Create Peer B
            PeerOptions optionsB = PeerOptions.builder()
                .peerId("peer-b")
                .autoProtocol(true)
                .build();

            try (UMICPWebSocketPeer peerB = new UMICPWebSocketPeer(optionsB)) {

                // Start server on Peer B
                peerB.startServer(20082).get();
                System.out.println("Peer B server started on port 20082");

                // Add event listener to Peer B
                peerB.addEventListener(createEventListener("Peer B", peerB));

                // Create Peer C
                PeerOptions optionsC = PeerOptions.builder()
                    .peerId("peer-c")
                    .autoProtocol(true)
                    .build();

                try (UMICPWebSocketPeer peerC = new UMICPWebSocketPeer(optionsC)) {

                    // Start server on Peer C
                    peerC.startServer(20083).get();
                    System.out.println("Peer C server started on port 20083\n");

                    // Add event listener to Peer C
                    peerC.addEventListener(createEventListener("Peer C", peerC));

                    // Connect peers in ring topology
                    System.out.println("Connecting peers in ring topology...");

                    // A → B
                    Map<String, String> metadataAB = new HashMap<>();
                    metadataAB.put("connection", "A-to-B");
                    peerA.connectToPeer("ws://localhost:20082/umicp", metadataAB).get();
                    System.out.println("Connected: A → B");

                    // B → C
                    Map<String, String> metadataBC = new HashMap<>();
                    metadataBC.put("connection", "B-to-C");
                    peerB.connectToPeer("ws://localhost:20083/umicp", metadataBC).get();
                    System.out.println("Connected: B → C");

                    // C → A
                    Map<String, String> metadataCA = new HashMap<>();
                    metadataCA.put("connection", "C-to-A");
                    peerC.connectToPeer("ws://localhost:20081/umicp", metadataCA).get();
                    System.out.println("Connected: C → A\n");

                    // Wait for handshakes to complete
                    Thread.sleep(2000);

                    // Print peer statistics
                    printPeerStats("Peer A", peerA);
                    printPeerStats("Peer B", peerB);
                    printPeerStats("Peer C", peerC);

                    // Send test messages
                    System.out.println("\n=== Sending Test Messages ===\n");

                    // Peer A sends to all
                    Envelope msgFromA = new Envelope()
                        .setFrom("peer-a")
                        .setTo("*")
                        .setOperation(OperationType.DATA)
                        .setMessageId(UUID.randomUUID().toString())
                        .setCapabilities(Map.of("message", "Hello from Peer A!"));

                    int sentByA = peerA.broadcast(msgFromA).get();
                    System.out.println("Peer A broadcasted to " + sentByA + " peers");

                    Thread.sleep(1000);

                    // Peer B sends to Peer C only
                    String peerCId = peerB.getPeers().stream()
                        .filter(p -> p.getUrl() != null && p.getUrl().contains("20083"))
                        .findFirst()
                        .map(PeerConnection::getId)
                        .orElseThrow();

                    Envelope msgFromB = new Envelope()
                        .setFrom("peer-b")
                        .setTo("peer-c")
                        .setOperation(OperationType.DATA)
                        .setMessageId(UUID.randomUUID().toString())
                        .setCapabilities(Map.of("message", "Direct message from B to C"));

                    peerB.sendToPeer(peerCId, msgFromB).get();
                    System.out.println("Peer B sent direct message to Peer C");

                    Thread.sleep(1000);

                    // Request-response pattern
                    System.out.println("\n=== Request-Response Pattern ===\n");

                    String peerBId = peerA.getPeers().stream()
                        .filter(p -> p.getUrl() != null && p.getUrl().contains("20082"))
                        .findFirst()
                        .map(PeerConnection::getId)
                        .orElseThrow();

                    Envelope request = new Envelope()
                        .setFrom("peer-a")
                        .setTo("peer-b")
                        .setOperation(OperationType.REQUEST)
                        .setMessageId(UUID.randomUUID().toString())
                        .setCapabilities(Map.of("query", "What's your status?"));

                    CompletableFuture<Envelope> responseFuture =
                        peerA.sendAndWait(peerBId, request, java.time.Duration.ofSeconds(5));

                    Envelope response = responseFuture.get();
                    System.out.println("Received response: " + response.getCapabilities());

                    // Keep running for a while
                    System.out.println("\nPeers running... (press Ctrl+C to stop)");
                    Thread.sleep(30000);
                }
            }
        }
    }

    /**
     * Create a generic event listener for a peer.
     */
    private static PeerEventListener createEventListener(String peerName, UMICPWebSocketPeer peer) {
        return new PeerEventListener() {
            @Override
            public void onMessage(Envelope envelope, PeerConnection connection) {
                System.out.println("[" + peerName + "] Message from: " + connection.getId());
                System.out.println("              Content: " + envelope.getCapabilities().get("message"));
            }

            @Override
            public void onData(Envelope envelope, PeerConnection connection) {
                System.out.println("[" + peerName + "] Data from: " + connection.getId());
            }

            @Override
            public void onPeerConnect(PeerConnection connection) {
                System.out.println("[" + peerName + "] Peer connected: " + connection.getId());
            }

            @Override
            public void onPeerDisconnect(PeerConnection connection) {
                System.out.println("[" + peerName + "] Peer disconnected: " + connection.getId());
            }

            @Override
            public void onPeerReady(PeerConnection connection, PeerInfo info) {
                System.out.println("[" + peerName + "] Peer ready: " + info.getPeerId());
            }

            @Override
            public void onError(Exception error, PeerConnection connection) {
                System.err.println("[" + peerName + "] Error: " + error.getMessage());
            }
        };
    }

    /**
     * Print peer statistics.
     */
    private static void printPeerStats(String name, UMICPWebSocketPeer peer) {
        PeerStats stats = peer.getStats();
        System.out.println(name + " Statistics:");
        System.out.println("  Total Peers: " + stats.getTotalPeers());
        System.out.println("  Incoming: " + stats.getIncomingConnections());
        System.out.println("  Outgoing: " + stats.getOutgoingConnections());
        System.out.println("  Messages Sent: " + stats.getTotalMessagesSent());
        System.out.println("  Messages Received: " + stats.getTotalMessagesReceived());
        System.out.println();
    }
}

