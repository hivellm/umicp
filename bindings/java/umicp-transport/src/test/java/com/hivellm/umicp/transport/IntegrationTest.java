package com.hivellm.umicp.transport;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.types.OperationType;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.Timeout;

import java.util.UUID;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Integration tests for UMICP WebSocket transport.
 *
 * <p>These tests verify the complete client-server communication flow.
 */
@Timeout(10)
class IntegrationTest {

    private UMICPWebSocketServer server;
    private UMICPWebSocketClient client;

    private static final int TEST_PORT = 19080;

    @BeforeEach
    void setUp() throws Exception {
        // Start server
        ServerOptions serverOptions = ServerOptions.builder()
            .maxClients(10)
            .build();

        server = new UMICPWebSocketServer(TEST_PORT, serverOptions);

        CountDownLatch serverLatch = new CountDownLatch(1);
        server.addEventListener(new ServerEventListener() {
            @Override
            public void onServerStarted() {
                serverLatch.countDown();
            }
        });

        server.start().get();
        serverLatch.await(3, TimeUnit.SECONDS);

        // Give server time to fully start
        Thread.sleep(200);
    }

    @AfterEach
    void tearDown() throws Exception {
        if (client != null) {
            try {
                client.close();
            } catch (Exception e) {
                // Ignore
            }
        }

        if (server != null) {
            try {
                server.close();
            } catch (Exception e) {
                // Ignore
            }
        }

        // Give time for ports to be released
        Thread.sleep(200);
    }

    @Test
    void testClientConnectsToServer() throws Exception {
        CountDownLatch clientLatch = new CountDownLatch(1);
        CountDownLatch serverLatch = new CountDownLatch(1);

        server.addEventListener(new ServerEventListener() {
            @Override
            public void onClientConnected(ClientConnection connection) {
                serverLatch.countDown();
            }
        });

        ClientOptions clientOptions = ClientOptions.builder()
            .autoReconnect(false)
            .build();

        client = new UMICPWebSocketClient("ws://localhost:" + TEST_PORT + "/umicp", clientOptions);

        client.addEventListener(new TransportEventListener() {
            @Override
            public void onConnected() {
                clientLatch.countDown();
            }
        });

        client.connect().get();

        assertTrue(clientLatch.await(3, TimeUnit.SECONDS), "Client should connect");
        assertTrue(serverLatch.await(3, TimeUnit.SECONDS), "Server should receive connection");
        assertTrue(client.isConnected());
        assertEquals(1, server.getClientCount());
    }

    @Test
    void testClientSendsMessageToServer() throws Exception {
        CountDownLatch messageLatch = new CountDownLatch(1);
        AtomicReference<Envelope> receivedEnvelope = new AtomicReference<>();

        server.addEventListener(new ServerEventListener() {
            @Override
            public void onMessage(Envelope envelope, ClientConnection connection) {
                receivedEnvelope.set(envelope);
                messageLatch.countDown();
            }
        });

        ClientOptions clientOptions = ClientOptions.builder()
            .autoReconnect(false)
            .build();

        client = new UMICPWebSocketClient("ws://localhost:" + TEST_PORT + "/umicp", clientOptions);
        client.connect().get();

        Thread.sleep(100);

        Envelope sentEnvelope = new Envelope()
            .setFrom("client")
            .setTo("server")
            .setOperation(OperationType.DATA)
            .setMessageId("msg-" + UUID.randomUUID());

        client.send(sentEnvelope).get();

        assertTrue(messageLatch.await(3, TimeUnit.SECONDS), "Server should receive message");
        assertNotNull(receivedEnvelope.get());
        assertEquals(sentEnvelope.getMessageId(), receivedEnvelope.get().getMessageId());
    }

    @Test
    void testServerSendsMessageToClient() throws Exception {
        CountDownLatch clientConnectedLatch = new CountDownLatch(1);
        CountDownLatch messageLatch = new CountDownLatch(1);
        AtomicReference<String> clientId = new AtomicReference<>();
        AtomicReference<Envelope> receivedEnvelope = new AtomicReference<>();

        server.addEventListener(new ServerEventListener() {
            @Override
            public void onClientConnected(ClientConnection connection) {
                clientId.set(connection.getId());
                clientConnectedLatch.countDown();
            }
        });

        ClientOptions clientOptions = ClientOptions.builder()
            .autoReconnect(false)
            .build();

        client = new UMICPWebSocketClient("ws://localhost:" + TEST_PORT + "/umicp", clientOptions);

        client.addEventListener(new TransportEventListener() {
            @Override
            public void onMessage(Envelope envelope) {
                receivedEnvelope.set(envelope);
                messageLatch.countDown();
            }
        });

        client.connect().get();

        assertTrue(clientConnectedLatch.await(3, TimeUnit.SECONDS));
        assertNotNull(clientId.get());

        Envelope sentEnvelope = new Envelope()
            .setFrom("server")
            .setTo("client")
            .setOperation(OperationType.DATA)
            .setMessageId("msg-" + UUID.randomUUID());

        server.send(sentEnvelope, clientId.get()).get();

        assertTrue(messageLatch.await(3, TimeUnit.SECONDS), "Client should receive message");
        assertNotNull(receivedEnvelope.get());
        assertEquals(sentEnvelope.getMessageId(), receivedEnvelope.get().getMessageId());
    }

    @Test
    void testServerBroadcastsToMultipleClients() throws Exception {
        UMICPWebSocketClient client2 = null;

        try {
            CountDownLatch client1Latch = new CountDownLatch(1);
            CountDownLatch client2Latch = new CountDownLatch(1);
            CountDownLatch serverLatch = new CountDownLatch(2);

            server.addEventListener(new ServerEventListener() {
                @Override
                public void onClientConnected(ClientConnection connection) {
                    serverLatch.countDown();
                }
            });

            ClientOptions clientOptions = ClientOptions.builder()
                .autoReconnect(false)
                .build();

            // Connect client 1
            client = new UMICPWebSocketClient("ws://localhost:" + TEST_PORT + "/umicp", clientOptions);
            client.addEventListener(new TransportEventListener() {
                @Override
                public void onMessage(Envelope envelope) {
                    client1Latch.countDown();
                }
            });
            client.connect().get();

            // Connect client 2
            client2 = new UMICPWebSocketClient("ws://localhost:" + TEST_PORT + "/umicp", clientOptions);
            client2.addEventListener(new TransportEventListener() {
                @Override
                public void onMessage(Envelope envelope) {
                    client2Latch.countDown();
                }
            });
            client2.connect().get();

            assertTrue(serverLatch.await(3, TimeUnit.SECONDS));
            assertEquals(2, server.getClientCount());

            Thread.sleep(100);

            // Broadcast message
            Envelope broadcast = new Envelope()
                .setFrom("server")
                .setTo("*")
                .setOperation(OperationType.DATA)
                .setMessageId("broadcast-" + UUID.randomUUID());

            int count = server.broadcast(broadcast).get();
            assertEquals(2, count);

            assertTrue(client1Latch.await(3, TimeUnit.SECONDS), "Client 1 should receive broadcast");
            assertTrue(client2Latch.await(3, TimeUnit.SECONDS), "Client 2 should receive broadcast");

        } finally {
            if (client2 != null) {
                try {
                    client2.close();
                } catch (Exception e) {
                    // Ignore
                }
            }
        }
    }

    @Test
    void testClientDisconnect() throws Exception {
        CountDownLatch connectLatch = new CountDownLatch(1);
        CountDownLatch disconnectLatch = new CountDownLatch(1);

        server.addEventListener(new ServerEventListener() {
            @Override
            public void onClientConnected(ClientConnection connection) {
                connectLatch.countDown();
            }

            @Override
            public void onClientDisconnected(ClientConnection connection, int code, String reason) {
                disconnectLatch.countDown();
            }
        });

        ClientOptions clientOptions = ClientOptions.builder()
            .autoReconnect(false)
            .build();

        client = new UMICPWebSocketClient("ws://localhost:" + TEST_PORT + "/umicp", clientOptions);
        client.connect().get();

        assertTrue(connectLatch.await(3, TimeUnit.SECONDS));
        assertEquals(1, server.getClientCount());

        client.disconnect().get();

        assertTrue(disconnectLatch.await(3, TimeUnit.SECONDS));
        assertFalse(client.isConnected());
    }

    @Test
    void testMultipleMessagesSequential() throws Exception {
        CountDownLatch messageLatch = new CountDownLatch(5);
        AtomicInteger messageCount = new AtomicInteger(0);

        server.addEventListener(new ServerEventListener() {
            @Override
            public void onMessage(Envelope envelope, ClientConnection connection) {
                messageCount.incrementAndGet();
                messageLatch.countDown();
            }
        });

        ClientOptions clientOptions = ClientOptions.builder()
            .autoReconnect(false)
            .build();

        client = new UMICPWebSocketClient("ws://localhost:" + TEST_PORT + "/umicp", clientOptions);
        client.connect().get();

        Thread.sleep(100);

        // Send 5 messages
        for (int i = 0; i < 5; i++) {
            Envelope envelope = new Envelope()
                .setFrom("client")
                .setTo("server")
                .setOperation(OperationType.DATA)
                .setMessageId("msg-" + i);

            client.send(envelope).get();
        }

        assertTrue(messageLatch.await(5, TimeUnit.SECONDS), "Server should receive all 5 messages");
        assertEquals(5, messageCount.get());
    }

    @Test
    void testStatisticsTracking() throws Exception {
        CountDownLatch messageLatch = new CountDownLatch(3);

        server.addEventListener(new ServerEventListener() {
            @Override
            public void onMessage(Envelope envelope, ClientConnection connection) {
                messageLatch.countDown();
            }
        });

        ClientOptions clientOptions = ClientOptions.builder()
            .autoReconnect(false)
            .build();

        client = new UMICPWebSocketClient("ws://localhost:" + TEST_PORT + "/umicp", clientOptions);
        client.connect().get();

        Thread.sleep(100);

        // Send 3 messages
        for (int i = 0; i < 3; i++) {
            Envelope envelope = new Envelope()
                .setFrom("client")
                .setTo("server")
                .setOperation(OperationType.DATA)
                .setMessageId("msg-" + i);

            client.send(envelope).get();
        }

        assertTrue(messageLatch.await(3, TimeUnit.SECONDS));

        TransportStats clientStats = client.getStats();
        TransportStats serverStats = server.getStats();

        assertTrue(clientStats.getMessagesSent() >= 3);
        assertTrue(serverStats.getMessagesReceived() >= 3);
    }
}

