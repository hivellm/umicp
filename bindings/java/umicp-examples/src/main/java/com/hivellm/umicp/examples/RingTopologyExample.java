package com.hivellm.umicp.examples;

import com.hivellm.umicp.core.*;
import com.hivellm.umicp.types.OperationType;
import com.hivellm.umicp.transport.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * Example demonstrating ring topology with 3 multiplexed peers.
 *
 * <p>Network topology:
 * <pre>
 *   Peer A (20081) → Peer B (20082) → Peer C (20083) → Peer A
 * </pre>
 */
public class RingTopologyExample {

    private static final Logger logger = LoggerFactory.getLogger(RingTopologyExample.class);

    public static void main(String[] args) {
        logger.info("=== UMICP Ring Topology Example ===\n");
        UMICP.printVersion();

        try {
            runExample();
        } catch (Exception e) {
            logger.error("Error in example", e);
        }
    }

    private static void runExample() throws Exception {
        // Create Peer A
        PeerOptions optionsA = PeerOptions.builder("peer-a")
            .autoProtocol(true)
            .build();

        try (UMICPWebSocketPeer peerA = new UMICPWebSocketPeer(optionsA)) {

            // Start server on Peer A
            peerA.startServer(20081).get();
            logger.info("Peer A started on port 20081");

            // Add event listener
            peerA.addEventListener(createPeerListener("Peer A", peerA));

            // Create Peer B
            PeerOptions optionsB = PeerOptions.builder("peer-b")
                .autoProtocol(true)
                .build();

            try (UMICPWebSocketPeer peerB = new UMICPWebSocketPeer(optionsB)) {

                peerB.startServer(20082).get();
                logger.info("Peer B started on port 20082");
                peerB.addEventListener(createPeerListener("Peer B", peerB));

                // Create Peer C
                PeerOptions optionsC = PeerOptions.builder("peer-c")
                    .autoProtocol(true)
                    .build();

                try (UMICPWebSocketPeer peerC = new UMICPWebSocketPeer(optionsC)) {

                    peerC.startServer(20083).get();
                    logger.info("Peer C started on port 20083\n");
                    peerC.addEventListener(createPeerListener("Peer C", peerC));

                    // Connect in ring: A → B → C → A
                    logger.info("Connecting peers in ring topology...");

                    Map<String, String> metadataAB = new HashMap<>();
                    metadataAB.put("connection", "A-to-B");
                    peerA.connectToPeer("ws://localhost:20082/umicp", metadataAB).get();
                    logger.info("Connected: A → B");

                    Map<String, String> metadataBC = new HashMap<>();
                    metadataBC.put("connection", "B-to-C");
                    peerB.connectToPeer("ws://localhost:20083/umicp", metadataBC).get();
                    logger.info("Connected: B → C");

                    Map<String, String> metadataCA = new HashMap<>();
                    metadataCA.put("connection", "C-to-A");
                    peerC.connectToPeer("ws://localhost:20081/umicp", metadataCA).get();
                    logger.info("Connected: C → A\n");

                    // Wait for handshakes
                    Thread.sleep(2000);

                    // Print stats
                    printStats("Peer A", peerA);
                    printStats("Peer B", peerB);
                    printStats("Peer C", peerC);

                    // Send test messages
                    logger.info("\n=== Sending Test Messages ===\n");

                    // Peer A broadcasts
                    Envelope msgA = UMICP.createEnvelope(
                        UMICP.envelopeOptions()
                            .from("peer-a")
                            .to("*")
                            .operation(OperationType.DATA)
                            .messageId(UUID.randomUUID().toString())
                            .capability("message", "Hello from Peer A!")
                            .build()
                    );

                    int sent = peerA.broadcast(msgA).get();
                    logger.info("Peer A broadcasted to {} peers", sent);

                    Thread.sleep(1000);

                    // Keep running
                    logger.info("\nRing network running... (Ctrl+C to stop)");
                    Thread.sleep(30000);
                }
            }
        }
    }

    private static PeerEventListener createPeerListener(String name, UMICPWebSocketPeer peer) {
        return new PeerEventListener() {
            @Override
            public void onMessage(Envelope envelope, PeerConnection connection) {
                logger.info("[{}] Message from {}: {}", name, connection.getId(),
                          envelope.getCapabilities().get("message"));

                // Echo response
                if (envelope.getOperation() == OperationType.DATA) {
                    Envelope response = UMICP.createEnvelope(
                        UMICP.envelopeOptions()
                            .from(name.toLowerCase().replace(" ", "-"))
                            .to(envelope.getFrom())
                            .operation(OperationType.ACK)
                            .messageId("ack-" + System.currentTimeMillis())
                            .capability("status", "received")
                            .build()
                    );

                    try {
                        peer.sendToPeer(connection.getId(), response).get();
                    } catch (Exception e) {
                        logger.error("Failed to send response", e);
                    }
                }
            }

            @Override
            public void onPeerConnect(PeerConnection connection) {
                logger.info("[{}] Peer connected: {} ({})", name, connection.getId(), connection.getType());
            }

            @Override
            public void onPeerDisconnect(PeerConnection connection) {
                logger.info("[{}] Peer disconnected: {}", name, connection.getId());
            }

            @Override
            public void onPeerReady(PeerConnection connection, PeerInfo info) {
                logger.info("[{}] Peer ready: {}", name, info.getPeerId());
            }

            @Override
            public void onError(Exception error, PeerConnection connection) {
                logger.error("[{}] Error: {}", name, error.getMessage());
            }
        };
    }

    private static void printStats(String name, UMICPWebSocketPeer peer) {
        PeerStats stats = peer.getStats();
        logger.info("{} Statistics:", name);
        logger.info("  Total Peers: {}", stats.getTotalPeers());
        logger.info("  Incoming: {}", stats.getIncomingConnections());
        logger.info("  Outgoing: {}", stats.getOutgoingConnections());
    }
}

