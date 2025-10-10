package com.hivellm.umicp.transport;

import org.junit.jupiter.api.Test;

import java.time.Instant;
import java.util.HashMap;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for PeerInfo.
 */
class PeerInfoTest {

    @Test
    void testConstructor_withValidData() {
        Map<String, String> metadata = new HashMap<>();
        metadata.put("version", "1.0");

        Map<String, String> capabilities = new HashMap<>();
        capabilities.put("feature1", "enabled");

        Instant now = Instant.now();

        PeerInfo info = new PeerInfo("peer-1", metadata, capabilities, now);

        assertEquals("peer-1", info.getPeerId());
        assertEquals(1, info.getMetadata().size());
        assertEquals("1.0", info.getMetadata().get("version"));
        assertEquals(1, info.getCapabilities().size());
        assertEquals("enabled", info.getCapabilities().get("feature1"));
        assertEquals(now, info.getHandshakeCompletedAt());
    }

    @Test
    void testMetadata_isDefensiveCopy() {
        Map<String, String> metadata = new HashMap<>();
        metadata.put("key1", "value1");

        PeerInfo info = new PeerInfo(
            "peer-1",
            metadata,
            new HashMap<>(),
            Instant.now()
        );

        // Modify original
        metadata.put("key2", "value2");

        // Info should not be affected
        assertEquals(1, info.getMetadata().size());
        assertTrue(info.getMetadata().containsKey("key1"));
        assertFalse(info.getMetadata().containsKey("key2"));
    }

    @Test
    void testMetadata_returnedMapIsDefensiveCopy() {
        Map<String, String> metadata = new HashMap<>();
        metadata.put("key1", "value1");

        PeerInfo info = new PeerInfo(
            "peer-1",
            metadata,
            new HashMap<>(),
            Instant.now()
        );

        // Get and modify
        Map<String, String> retrieved = info.getMetadata();
        retrieved.put("key2", "value2");

        // Info should not be affected
        assertEquals(1, info.getMetadata().size());
        assertFalse(info.getMetadata().containsKey("key2"));
    }

    @Test
    void testCapabilities_isDefensiveCopy() {
        Map<String, String> capabilities = new HashMap<>();
        capabilities.put("cap1", "value1");

        PeerInfo info = new PeerInfo(
            "peer-1",
            new HashMap<>(),
            capabilities,
            Instant.now()
        );

        // Modify original
        capabilities.put("cap2", "value2");

        // Info should not be affected
        assertEquals(1, info.getCapabilities().size());
        assertTrue(info.getCapabilities().containsKey("cap1"));
        assertFalse(info.getCapabilities().containsKey("cap2"));
    }

    @Test
    void testCapabilities_returnedMapIsDefensiveCopy() {
        Map<String, String> capabilities = new HashMap<>();
        capabilities.put("cap1", "value1");

        PeerInfo info = new PeerInfo(
            "peer-1",
            new HashMap<>(),
            capabilities,
            Instant.now()
        );

        // Get and modify
        Map<String, String> retrieved = info.getCapabilities();
        retrieved.put("cap2", "value2");

        // Info should not be affected
        assertEquals(1, info.getCapabilities().size());
        assertFalse(info.getCapabilities().containsKey("cap2"));
    }

    @Test
    void testEquals_sameData() {
        Map<String, String> metadata = new HashMap<>();
        metadata.put("key", "value");

        Map<String, String> capabilities = new HashMap<>();
        capabilities.put("cap", "val");

        PeerInfo info1 = new PeerInfo(
            "peer-1",
            metadata,
            capabilities,
            Instant.now()
        );

        PeerInfo info2 = new PeerInfo(
            "peer-1",
            new HashMap<>(metadata),
            new HashMap<>(capabilities),
            Instant.now()
        );

        assertEquals(info1, info2);
    }

    @Test
    void testEquals_differentPeerId() {
        Map<String, String> metadata = new HashMap<>();
        Map<String, String> capabilities = new HashMap<>();

        PeerInfo info1 = new PeerInfo(
            "peer-1",
            metadata,
            capabilities,
            Instant.now()
        );

        PeerInfo info2 = new PeerInfo(
            "peer-2",
            metadata,
            capabilities,
            Instant.now()
        );

        assertNotEquals(info1, info2);
    }

    @Test
    void testEquals_differentMetadata() {
        Map<String, String> metadata1 = new HashMap<>();
        metadata1.put("key", "value1");

        Map<String, String> metadata2 = new HashMap<>();
        metadata2.put("key", "value2");

        PeerInfo info1 = new PeerInfo(
            "peer-1",
            metadata1,
            new HashMap<>(),
            Instant.now()
        );

        PeerInfo info2 = new PeerInfo(
            "peer-1",
            metadata2,
            new HashMap<>(),
            Instant.now()
        );

        assertNotEquals(info1, info2);
    }

    @Test
    void testEquals_withNull() {
        PeerInfo info = new PeerInfo(
            "peer-1",
            new HashMap<>(),
            new HashMap<>(),
            Instant.now()
        );

        assertNotEquals(null, info);
    }

    @Test
    void testEquals_withSelf() {
        PeerInfo info = new PeerInfo(
            "peer-1",
            new HashMap<>(),
            new HashMap<>(),
            Instant.now()
        );

        assertEquals(info, info);
    }

    @Test
    void testHashCode_sameData() {
        Map<String, String> metadata = new HashMap<>();
        metadata.put("key", "value");

        Map<String, String> capabilities = new HashMap<>();
        capabilities.put("cap", "val");

        PeerInfo info1 = new PeerInfo(
            "peer-1",
            metadata,
            capabilities,
            Instant.now()
        );

        PeerInfo info2 = new PeerInfo(
            "peer-1",
            new HashMap<>(metadata),
            new HashMap<>(capabilities),
            Instant.now()
        );

        assertEquals(info1.hashCode(), info2.hashCode());
    }

    @Test
    void testToString_containsKeyInfo() {
        Map<String, String> metadata = new HashMap<>();
        metadata.put("version", "1.0");

        PeerInfo info = new PeerInfo(
            "peer-1",
            metadata,
            new HashMap<>(),
            Instant.now()
        );

        String str = info.toString();
        assertTrue(str.contains("peer-1"));
        assertTrue(str.contains("metadata"));
    }
}

