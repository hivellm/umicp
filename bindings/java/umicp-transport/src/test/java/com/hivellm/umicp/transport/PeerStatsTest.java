package com.hivellm.umicp.transport;

import org.junit.jupiter.api.Test;

import java.util.HashMap;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for PeerStats.
 */
class PeerStatsTest {

    @Test
    void testConstructor_withValidData() {
        Map<String, TransportStats> perPeerStats = new HashMap<>();
        perPeerStats.put("peer-1", new TransportStats());

        PeerStats stats = new PeerStats(
            10,
            6,
            4,
            100L,
            200L,
            perPeerStats
        );

        assertEquals(10, stats.getTotalPeers());
        assertEquals(6, stats.getIncomingConnections());
        assertEquals(4, stats.getOutgoingConnections());
        assertEquals(100L, stats.getTotalMessagesSent());
        assertEquals(200L, stats.getTotalMessagesReceived());
        assertEquals(1, stats.getPerPeerStats().size());
    }

    @Test
    void testConstructor_withNullPerPeerStats() {
        PeerStats stats = new PeerStats(5, 3, 2, 50L, 60L, null);

        assertEquals(5, stats.getTotalPeers());
        assertEquals(3, stats.getIncomingConnections());
        assertEquals(2, stats.getOutgoingConnections());
        assertEquals(50L, stats.getTotalMessagesSent());
        assertEquals(60L, stats.getTotalMessagesReceived());
        assertNotNull(stats.getPerPeerStats());
        assertTrue(stats.getPerPeerStats().isEmpty());
    }

    @Test
    void testConstructor_withEmptyPerPeerStats() {
        Map<String, TransportStats> perPeerStats = new HashMap<>();

        PeerStats stats = new PeerStats(0, 0, 0, 0L, 0L, perPeerStats);

        assertEquals(0, stats.getTotalPeers());
        assertEquals(0, stats.getIncomingConnections());
        assertEquals(0, stats.getOutgoingConnections());
        assertEquals(0L, stats.getTotalMessagesSent());
        assertEquals(0L, stats.getTotalMessagesReceived());
        assertTrue(stats.getPerPeerStats().isEmpty());
    }

    @Test
    void testPerPeerStats_isDefensiveCopy() {
        Map<String, TransportStats> perPeerStats = new HashMap<>();
        perPeerStats.put("peer-1", new TransportStats());

        PeerStats stats = new PeerStats(1, 1, 0, 10L, 20L, perPeerStats);

        // Modify original
        perPeerStats.put("peer-2", new TransportStats());

        // Stats should not be affected
        assertEquals(1, stats.getPerPeerStats().size());
        assertTrue(stats.getPerPeerStats().containsKey("peer-1"));
        assertFalse(stats.getPerPeerStats().containsKey("peer-2"));
    }

    @Test
    void testPerPeerStats_returnedMapIsDefensiveCopy() {
        Map<String, TransportStats> perPeerStats = new HashMap<>();
        perPeerStats.put("peer-1", new TransportStats());

        PeerStats stats = new PeerStats(1, 1, 0, 10L, 20L, perPeerStats);

        // Get and modify
        Map<String, TransportStats> retrieved = stats.getPerPeerStats();
        retrieved.put("peer-2", new TransportStats());

        // Stats should not be affected
        assertEquals(1, stats.getPerPeerStats().size());
        assertFalse(stats.getPerPeerStats().containsKey("peer-2"));
    }

    @Test
    void testToString_containsKeyInfo() {
        PeerStats stats = new PeerStats(
            10,
            6,
            4,
            100L,
            200L,
            new HashMap<>()
        );

        String str = stats.toString();
        assertTrue(str.contains("10"));
        assertTrue(str.contains("6"));
        assertTrue(str.contains("4"));
        assertTrue(str.contains("100"));
        assertTrue(str.contains("200"));
    }

    @Test
    void testGetters_returnCorrectValues() {
        Map<String, TransportStats> perPeerStats = new HashMap<>();
        TransportStats ts1 = new TransportStats();
        TransportStats ts2 = new TransportStats();
        perPeerStats.put("peer-1", ts1);
        perPeerStats.put("peer-2", ts2);

        PeerStats stats = new PeerStats(
            15,
            8,
            7,
            500L,
            750L,
            perPeerStats
        );

        assertEquals(15, stats.getTotalPeers());
        assertEquals(8, stats.getIncomingConnections());
        assertEquals(7, stats.getOutgoingConnections());
        assertEquals(500L, stats.getTotalMessagesSent());
        assertEquals(750L, stats.getTotalMessagesReceived());
        assertEquals(2, stats.getPerPeerStats().size());
    }

    @Test
    void testZeroValues() {
        PeerStats stats = new PeerStats(0, 0, 0, 0L, 0L, new HashMap<>());

        assertEquals(0, stats.getTotalPeers());
        assertEquals(0, stats.getIncomingConnections());
        assertEquals(0, stats.getOutgoingConnections());
        assertEquals(0L, stats.getTotalMessagesSent());
        assertEquals(0L, stats.getTotalMessagesReceived());
        assertTrue(stats.getPerPeerStats().isEmpty());
    }

    @Test
    void testLargeValues() {
        PeerStats stats = new PeerStats(
            1000,
            600,
            400,
            Long.MAX_VALUE,
            Long.MAX_VALUE - 1,
            new HashMap<>()
        );

        assertEquals(1000, stats.getTotalPeers());
        assertEquals(600, stats.getIncomingConnections());
        assertEquals(400, stats.getOutgoingConnections());
        assertEquals(Long.MAX_VALUE, stats.getTotalMessagesSent());
        assertEquals(Long.MAX_VALUE - 1, stats.getTotalMessagesReceived());
    }
}

