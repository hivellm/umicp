package com.hivellm.umicp.transport;

import org.junit.jupiter.api.Test;

import java.util.HashMap;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for PeerConnection.
 */
class PeerConnectionTest {

    @Test
    void testConstructor_withValidData() {
        Map<String, String> metadata = new HashMap<>();
        metadata.put("key1", "value1");

        PeerConnection conn = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.INCOMING,
            "ws://localhost:8080",
            metadata
        );

        assertEquals("peer-1", conn.getId());
        assertEquals(PeerConnection.PeerType.INCOMING, conn.getType());
        assertEquals("ws://localhost:8080", conn.getUrl());
        assertFalse(conn.isHandshakeComplete());
        assertNull(conn.getPeerInfo());
        assertNotNull(conn.getConnectedAt());
    }

    @Test
    void testConstructor_withNullUrl() {
        PeerConnection conn = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.OUTGOING,
            null,
            null
        );

        assertEquals("peer-1", conn.getId());
        assertNull(conn.getUrl());
        assertTrue(conn.getMetadata().isEmpty());
    }

    @Test
    void testMetadata_isDefensiveCopy() {
        Map<String, String> metadata = new HashMap<>();
        metadata.put("key1", "value1");

        PeerConnection conn = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.INCOMING,
            null,
            metadata
        );

        // Modify original
        metadata.put("key2", "value2");

        // Connection should not be affected
        assertEquals(1, conn.getMetadata().size());
        assertTrue(conn.getMetadata().containsKey("key1"));
        assertFalse(conn.getMetadata().containsKey("key2"));
    }

    @Test
    void testMetadata_returnedMapIsDefensiveCopy() {
        Map<String, String> metadata = new HashMap<>();
        metadata.put("key1", "value1");

        PeerConnection conn = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.INCOMING,
            null,
            metadata
        );

        // Get and modify
        Map<String, String> retrieved = conn.getMetadata();
        retrieved.put("key2", "value2");

        // Connection should not be affected
        assertEquals(1, conn.getMetadata().size());
        assertFalse(conn.getMetadata().containsKey("key2"));
    }

    @Test
    void testHandshakeComplete_defaultIsFalse() {
        PeerConnection conn = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.INCOMING,
            null,
            null
        );

        assertFalse(conn.isHandshakeComplete());
    }

    @Test
    void testHandshakeComplete_canBeSet() {
        PeerConnection conn = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.INCOMING,
            null,
            null
        );

        conn.setHandshakeComplete(true);
        assertTrue(conn.isHandshakeComplete());

        conn.setHandshakeComplete(false);
        assertFalse(conn.isHandshakeComplete());
    }

    @Test
    void testPeerInfo_canBeSet() {
        PeerConnection conn = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.INCOMING,
            null,
            null
        );

        assertNull(conn.getPeerInfo());

        PeerInfo info = new PeerInfo(
            "remote-peer",
            new HashMap<>(),
            new HashMap<>(),
            java.time.Instant.now()
        );

        conn.setPeerInfo(info);
        assertEquals(info, conn.getPeerInfo());
    }

    @Test
    void testEquals_sameId() {
        PeerConnection conn1 = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.INCOMING,
            null,
            null
        );

        PeerConnection conn2 = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.OUTGOING,
            "ws://localhost:8080",
            new HashMap<>()
        );

        assertEquals(conn1, conn2);
        assertEquals(conn1.hashCode(), conn2.hashCode());
    }

    @Test
    void testEquals_differentId() {
        PeerConnection conn1 = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.INCOMING,
            null,
            null
        );

        PeerConnection conn2 = new PeerConnection(
            "peer-2",
            PeerConnection.PeerType.INCOMING,
            null,
            null
        );

        assertNotEquals(conn1, conn2);
    }

    @Test
    void testEquals_withNull() {
        PeerConnection conn = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.INCOMING,
            null,
            null
        );

        assertNotEquals(null, conn);
    }

    @Test
    void testEquals_withSelf() {
        PeerConnection conn = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.INCOMING,
            null,
            null
        );

        assertEquals(conn, conn);
    }

    @Test
    void testToString_containsKeyInfo() {
        PeerConnection conn = new PeerConnection(
            "peer-1",
            PeerConnection.PeerType.INCOMING,
            "ws://localhost:8080",
            null
        );

        String str = conn.toString();
        assertTrue(str.contains("peer-1"));
        assertTrue(str.contains("INCOMING"));
        assertTrue(str.contains("ws://localhost:8080"));
        assertTrue(str.contains("handshakeComplete=false"));
    }

    @Test
    void testPeerType_values() {
        assertEquals(2, PeerConnection.PeerType.values().length);
        assertNotNull(PeerConnection.PeerType.INCOMING);
        assertNotNull(PeerConnection.PeerType.OUTGOING);
    }
}

