package com.hivellm.umicp.transport.pool;

import com.hivellm.umicp.transport.ClientOptions;
import com.hivellm.umicp.transport.UMICPWebSocketClient;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeEach;
import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.Mockito.*;

import java.net.URI;

/**
 * Unit tests for PooledConnection
 */
class PooledConnectionTest {
    private UMICPWebSocketClient mockClient;
    private PooledConnection connection;

    @BeforeEach
    void setUp() throws Exception {
        mockClient = mock(UMICPWebSocketClient.class);
        connection = new PooledConnection("ws://localhost:8080", mockClient);
    }

    @Test
    void testCreation() {
        assertNotNull(connection.getId());
        assertEquals("ws://localhost:8080", connection.getAddress());
        assertEquals(PoolConnectionState.AVAILABLE, connection.getState());
        assertEquals(0, connection.getUseCount());
        assertNotNull(connection.getClient());
    }

    @Test
    void testCreationWithNullAddress() {
        assertThrows(IllegalArgumentException.class, () -> {
            new PooledConnection(null, mockClient);
        });
    }

    @Test
    void testCreationWithNullClient() {
        assertThrows(IllegalArgumentException.class, () -> {
            new PooledConnection("ws://localhost:8080", null);
        });
    }

    @Test
    void testAcquireRelease() {
        assertEquals(PoolConnectionState.AVAILABLE, connection.getState());
        assertEquals(0, connection.getUseCount());

        connection.acquire();
        assertEquals(PoolConnectionState.IN_USE, connection.getState());
        assertEquals(1, connection.getUseCount());

        connection.release();
        assertEquals(PoolConnectionState.AVAILABLE, connection.getState());
        assertEquals(1, connection.getUseCount()); // Use count doesn't reset

        connection.acquire();
        assertEquals(2, connection.getUseCount());
    }

    @Test
    void testClose() {
        boolean closed = connection.close();
        assertTrue(closed);
        assertEquals(PoolConnectionState.CLOSED, connection.getState());
        verify(mockClient, times(1)).close();
    }

    @Test
    void testIsStale() throws InterruptedException {
        // Fresh connection should not be stale
        assertFalse(connection.isStale(10));

        Thread.sleep(2000);

        // Should be stale with 1 second max age
        assertTrue(connection.isStale(1));

        // Should not be stale with 10 second max age
        assertFalse(connection.isStale(10));
    }

    @Test
    void testIsIdle() throws InterruptedException {
        // Fresh connection should not be idle
        assertFalse(connection.isIdle(10));

        Thread.sleep(2000);

        // Should be idle with 1 second timeout
        assertTrue(connection.isIdle(1));

        // Should not be idle with 10 second timeout
        assertFalse(connection.isIdle(10));

        // Using connection resets idle timer
        connection.acquire();
        assertFalse(connection.isIdle(1));
    }

    @Test
    void testIsConnected() {
        when(mockClient.isConnected()).thenReturn(true);
        assertTrue(connection.isConnected());

        when(mockClient.isConnected()).thenReturn(false);
        assertFalse(connection.isConnected());
    }

    @Test
    void testToString() {
        String str = connection.toString();
        assertTrue(str.contains("PooledConnection"));
        assertTrue(str.contains("ws://localhost:8080"));
        assertTrue(str.contains("AVAILABLE"));
    }

    @Test
    void testLifecycle() throws InterruptedException {
        assertEquals(PoolConnectionState.AVAILABLE, connection.getState());

        connection.acquire();
        assertEquals(PoolConnectionState.IN_USE, connection.getState());

        var lastUsedBefore = connection.getLastUsed();
        Thread.sleep(100);

        connection.release();
        assertEquals(PoolConnectionState.AVAILABLE, connection.getState());
        var lastUsedAfter = connection.getLastUsed();
        assertTrue(lastUsedAfter.isAfter(lastUsedBefore));

        connection.close();
        assertEquals(PoolConnectionState.CLOSED, connection.getState());
    }
}

