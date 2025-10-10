package com.hivellm.umicp.transport;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.types.OperationType;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for UMICPWebSocketClient.
 */
class UMICPWebSocketClientTest {

    @Test
    void testConstructor_withUrl() {
        ClientOptions options = ClientOptions.defaults();
        UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", options);

        assertNotNull(client);
    }

    @Test
    void testConstructor_withNullUrl_throwsException() {
        ClientOptions options = ClientOptions.defaults();
        assertThrows(Exception.class, () -> {
            new UMICPWebSocketClient(null, options);
        });
    }

    @Test
    void testConstructor_withEmptyUrl_throwsException() {
        ClientOptions options = ClientOptions.defaults();
        assertThrows(Exception.class, () -> {
            new UMICPWebSocketClient("", options);
        });
    }

    @Test
    void testConstructor_withInvalidUrl_throwsException() {
        ClientOptions options = ClientOptions.defaults();
        assertThrows(Exception.class, () -> {
            new UMICPWebSocketClient("invalid-url", options);
        });
    }

    @Test
    void testConstructor_withNullOptions_usesDefaults() {
        assertDoesNotThrow(() -> {
            new UMICPWebSocketClient("ws://localhost:8080/umicp", null);
        });
    }

    @Test
    void testInitialState_notConnected() {
        ClientOptions options = ClientOptions.builder()
            .autoReconnect(false)
            .build();

        UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", options);

        assertFalse(client.isConnected());
    }

    @Test
    void testGetStats_returnsValidStats() {
        ClientOptions options = ClientOptions.defaults();
        UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", options);

        TransportStats stats = client.getStats();

        assertNotNull(stats);
        assertEquals(0, stats.getMessagesSent());
        assertEquals(0, stats.getMessagesReceived());
    }

    @Test
    void testAddEventListener() {
        ClientOptions options = ClientOptions.defaults();
        UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", options);

        TransportEventListener listener = new TransportEventListener() {};
        assertDoesNotThrow(() -> client.addEventListener(listener));
    }

    @Test
    void testRemoveEventListener() {
        ClientOptions options = ClientOptions.defaults();
        UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", options);

        TransportEventListener listener = new TransportEventListener() {};
        client.addEventListener(listener);
        assertDoesNotThrow(() -> client.removeEventListener(listener));
    }

    @Test
    void testSend_whenNotConnected_queuesMessage() {
        ClientOptions options = ClientOptions.builder()
            .autoReconnect(false)
            .messageQueueSize(10)
            .build();

        UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", options);

        Envelope envelope = new Envelope()
            .setFrom("client")
            .setTo("server")
            .setOperation(OperationType.DATA)
            .setMessageId("msg-1");

        // Should not throw, message gets queued
        assertDoesNotThrow(() -> client.send(envelope));
    }

    @Test
    void testDisconnect_whenNotConnected() {
        ClientOptions options = ClientOptions.builder()
            .autoReconnect(false)
            .build();

        UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", options);

        // Should not throw
        assertDoesNotThrow(() -> client.disconnect().get());
    }

    @Test
    void testClose_whenNotConnected() {
        ClientOptions options = ClientOptions.defaults();
        UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", options);

        assertDoesNotThrow(() -> client.close());
    }

    @Test
    void testMultipleEventListeners() {
        ClientOptions options = ClientOptions.defaults();
        UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", options);

        TransportEventListener listener1 = new TransportEventListener() {};
        TransportEventListener listener2 = new TransportEventListener() {};

        assertDoesNotThrow(() -> {
            client.addEventListener(listener1);
            client.addEventListener(listener2);
        });
    }

    @Test
    void testEventListenerRemoval() {
        ClientOptions options = ClientOptions.defaults();
        UMICPWebSocketClient client = new UMICPWebSocketClient("ws://localhost:8080/umicp", options);

        TransportEventListener listener = new TransportEventListener() {};

        client.addEventListener(listener);
        client.removeEventListener(listener);

        // Removing again should not throw
        assertDoesNotThrow(() -> client.removeEventListener(listener));
    }
}

