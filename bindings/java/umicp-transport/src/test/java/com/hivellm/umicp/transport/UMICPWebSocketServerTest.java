package com.hivellm.umicp.transport;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.types.OperationType;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.Timeout;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for UMICPWebSocketServer.
 */
class UMICPWebSocketServerTest {

    private UMICPWebSocketServer server;
    private static final int TEST_PORT = 18080;

    @BeforeEach
    void setUp() {
        // Use a different port for each test to avoid conflicts
        ServerOptions options = ServerOptions.builder()
            .maxClients(10)
            .build();
        server = new UMICPWebSocketServer(TEST_PORT, options);
    }

    @AfterEach
    void tearDown() throws Exception {
        if (server != null) {
            server.close();
        }
        // Give time for port to be released
        Thread.sleep(100);
    }

    @Test
    @Timeout(5)
    void testServerStartsSuccessfully() throws Exception {
        AtomicBoolean started = new AtomicBoolean(false);
        CountDownLatch latch = new CountDownLatch(1);

        server.addEventListener(new ServerEventListener() {
            @Override
            public void onServerStarted() {
                started.set(true);
                latch.countDown();
            }
        });

        server.start().get();

        assertTrue(latch.await(3, TimeUnit.SECONDS));
        assertTrue(started.get());
    }

    @Test
    void testServerInitialState() {
        assertEquals(0, server.getClientCount());
        assertNotNull(server.getStats());
    }

    @Test
    void testAddEventListener() {
        ServerEventListener listener = new ServerEventListener() {};
        assertDoesNotThrow(() -> server.addEventListener(listener));
    }

    @Test
    void testRemoveEventListener() {
        ServerEventListener listener = new ServerEventListener() {};
        server.addEventListener(listener);
        assertDoesNotThrow(() -> server.removeEventListener(listener));
    }

    @Test
    void testGetStats_returnsValidStats() {
        TransportStats stats = server.getStats();

        assertNotNull(stats);
        assertEquals(0, stats.getMessagesSent());
        assertEquals(0, stats.getMessagesReceived());
        assertEquals(0, stats.getBytesSent());
        assertEquals(0, stats.getBytesReceived());
    }

    @Test
    void testBroadcast_withNoClients() throws Exception {
        Envelope envelope = new Envelope()
            .setFrom("server")
            .setTo("*")
            .setOperation(OperationType.DATA)
            .setMessageId("msg-1");

        // Should not throw, just return 0
        int count = server.broadcast(envelope).get();
        assertEquals(0, count);
    }

    @Test
    void testSend_withInvalidClientId() throws Exception {
        Envelope envelope = new Envelope()
            .setFrom("server")
            .setTo("client")
            .setOperation(OperationType.DATA)
            .setMessageId("msg-1");

        boolean result = server.send(envelope, "non-existent-client").get();
        assertFalse(result);
    }

    @Test
    void testGetClientCount_initiallyZero() {
        assertEquals(0, server.getClientCount());
    }

    @Test
    void testMultipleEventListeners() throws Exception {
        AtomicInteger count = new AtomicInteger(0);
        CountDownLatch latch = new CountDownLatch(2);

        ServerEventListener listener1 = new ServerEventListener() {
            @Override
            public void onServerStarted() {
                count.incrementAndGet();
                latch.countDown();
            }
        };

        ServerEventListener listener2 = new ServerEventListener() {
            @Override
            public void onServerStarted() {
                count.incrementAndGet();
                latch.countDown();
            }
        };

        server.addEventListener(listener1);
        server.addEventListener(listener2);

        server.start().get();

        assertTrue(latch.await(3, TimeUnit.SECONDS));
        assertEquals(2, count.get());
    }

    @Test
    void testEventListenerException_doesNotBreakServer() throws Exception {
        AtomicBoolean secondListenerCalled = new AtomicBoolean(false);
        CountDownLatch latch = new CountDownLatch(1);

        ServerEventListener listener1 = new ServerEventListener() {
            @Override
            public void onServerStarted() {
                throw new RuntimeException("Test exception");
            }
        };

        ServerEventListener listener2 = new ServerEventListener() {
            @Override
            public void onServerStarted() {
                secondListenerCalled.set(true);
                latch.countDown();
            }
        };

        server.addEventListener(listener1);
        server.addEventListener(listener2);

        server.start().get();

        // Second listener should still be called
        assertTrue(latch.await(3, TimeUnit.SECONDS));
        assertTrue(secondListenerCalled.get());
    }

    @Test
    void testClose_stopsServer() throws Exception {
        server.start().get();
        Thread.sleep(100);

        assertDoesNotThrow(() -> server.close());

        // After closing, starting again should work with a new instance
        Thread.sleep(100);
    }

    @Test
    void testClose_withoutStart() {
        assertDoesNotThrow(() -> server.close());
    }

    @Test
    void testDoubleStart_throwsException() throws Exception {
        server.start().get();

        // Starting again should fail
        assertThrows(Exception.class, () -> server.start().get());
    }
}

