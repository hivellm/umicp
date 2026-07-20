package com.hivellm.umicp.examples;

import com.hivellm.umicp.core.*;
import com.hivellm.umicp.types.OperationType;
import com.hivellm.umicp.transport.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.UUID;

/**
 * Simple client-server example.
 */
public class SimpleClientServerExample {

    private static final Logger logger = LoggerFactory.getLogger(SimpleClientServerExample.class);

    public static void main(String[] args) {
        logger.info("=== UMICP Simple Client-Server Example ===");

        try {
            runExample();
        } catch (Exception e) {
            logger.error("Error", e);
        }
    }

    private static void runExample() throws Exception {
        // Create server
        ServerOptions serverOpts = ServerOptions.builder()
            .maxClients(100)
            .compression(true)
            .build();

        try (UMICPWebSocketServer server = new UMICPWebSocketServer(8080, serverOpts)) {

            // Server event listener
            server.addEventListener(new ServerEventListener() {
                @Override
                public void onServerStarted() {
                    logger.info("Server started on port 8080");
                }

                @Override
                public void onClientConnected(ClientConnection client) {
                    logger.info("Client connected: {}", client.getId());
                }

                @Override
                public void onMessage(Envelope envelope, ClientConnection client) {
                    logger.info("Server received: {} from {}", envelope.getMessageId(), client.getId());

                    // Echo back
                    Envelope response = UMICP.createEnvelope(
                        UMICP.envelopeOptions()
                            .from("server")
                            .to(envelope.getFrom())
                            .operation(OperationType.ACK)
                            .messageId("ack-" + System.currentTimeMillis())
                            .capability("status", "received")
                            .capability("original-id", envelope.getMessageId())
                            .build()
                    );

                    try {
                        server.send(response, client.getId()).get();
                    } catch (Exception e) {
                        logger.error("Failed to send response", e);
                    }
                }
            });

            server.start().get();

            // Create client
            ClientOptions clientOpts = ClientOptions.builder()
                .autoReconnect(true)
                .build();

            try (UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", clientOpts)) {

                // Client event listener
                client.addEventListener(new TransportEventListener() {
                    @Override
                    public void onConnected() {
                        logger.info("Client connected to server");
                    }

                    @Override
                    public void onMessage(Envelope envelope) {
                        logger.info("Client received: {}", envelope.getCapabilities());
                    }
                });

                client.connect().get();

                // Send messages
                for (int i = 0; i < 5; i++) {
                    Envelope message = UMICP.createEnvelope(
                        UMICP.envelopeOptions()
                            .from("client")
                            .to("server")
                            .operation(OperationType.DATA)
                            .messageId(UUID.randomUUID().toString())
                            .capability("message", "Hello " + i)
                            .capability("timestamp", String.valueOf(System.currentTimeMillis()))
                            .build()
                    );

                    client.send(message).get();
                    logger.info("Sent message {}", i);
                    Thread.sleep(1000);
                }

                // Print stats
                logger.info("\nClient stats: {}", client.getStats());
                logger.info("Server stats: {}", server.getStats());
                logger.info("Connected clients: {}", server.getClientCount());

                Thread.sleep(2000);
            }
        }
    }
}

