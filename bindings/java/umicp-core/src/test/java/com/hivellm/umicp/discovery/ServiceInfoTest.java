package com.hivellm.umicp.discovery;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeEach;
import static org.junit.jupiter.api.Assertions.*;

import java.time.Instant;

/**
 * Unit tests for ServiceInfo
 */
class ServiceInfoTest {
    private ServiceInfo service;

    @BeforeEach
    void setUp() {
        service = new ServiceInfo("service-001", "Test Service", "ws://localhost:8080");
    }

    @Test
    void testCreation() {
        assertEquals("service-001", service.getServiceId());
        assertEquals("Test Service", service.getName());
        assertEquals("ws://localhost:8080", service.getAddress());
        assertEquals("1.0.0", service.getVersion());
        assertTrue(service.getCapabilities().isEmpty());
        assertTrue(service.getMetadata().isEmpty());
    }

    @Test
    void testCreationWithNullServiceId() {
        assertThrows(IllegalArgumentException.class, () -> {
            new ServiceInfo(null, "Test", "ws://localhost:8080");
        });
    }

    @Test
    void testCreationWithEmptyServiceId() {
        assertThrows(IllegalArgumentException.class, () -> {
            new ServiceInfo("", "Test", "ws://localhost:8080");
        });
    }

    @Test
    void testCreationWithNullName() {
        assertThrows(IllegalArgumentException.class, () -> {
            new ServiceInfo("id", null, "ws://localhost:8080");
        });
    }

    @Test
    void testCreationWithNullAddress() {
        assertThrows(IllegalArgumentException.class, () -> {
            new ServiceInfo("id", "name", null);
        });
    }

    @Test
    void testAddCapability() {
        service.addCapability("storage");
        service.addCapability("compute");

        assertEquals(2, service.getCapabilities().size());
        assertTrue(service.hasCapability("storage"));
        assertTrue(service.hasCapability("compute"));
    }

    @Test
    void testAddDuplicateCapability() {
        service.addCapability("storage");
        service.addCapability("storage");

        assertEquals(1, service.getCapabilities().size());
    }

    @Test
    void testAddNullCapability() {
        service.addCapability(null);
        assertTrue(service.getCapabilities().isEmpty());
    }

    @Test
    void testAddMetadata() {
        service.addMetadata("region", "us-east-1");
        service.addMetadata("environment", "production");

        assertEquals("us-east-1", service.getMetadataValue("region"));
        assertEquals("production", service.getMetadataValue("environment"));
        assertNull(service.getMetadataValue("nonexistent"));
    }

    @Test
    void testAddNullMetadata() {
        service.addMetadata(null, "value");
        service.addMetadata("key", null);
        assertTrue(service.getMetadata().isEmpty());
    }

    @Test
    void testHasCapability() {
        service.addCapability("storage");

        assertTrue(service.hasCapability("storage"));
        assertFalse(service.hasCapability("compute"));
    }

    @Test
    void testUpdateLastSeen() throws InterruptedException {
        Instant initial = service.getLastSeen();

        Thread.sleep(100);
        service.updateLastSeen();

        Instant updated = service.getLastSeen();
        assertTrue(updated.isAfter(initial));
    }

    @Test
    void testIsStale() throws InterruptedException {
        // Fresh service should not be stale
        assertFalse(service.isStale(60));

        // Wait 2 seconds
        Thread.sleep(2000);

        // Should be stale with 1 second timeout
        assertTrue(service.isStale(1));

        // Should not be stale with 10 second timeout
        assertFalse(service.isStale(10));
    }

    @Test
    void testSetVersion() {
        service.setVersion("2.0.0");
        assertEquals("2.0.0", service.getVersion());
    }

    @Test
    void testSetNullVersion() {
        assertThrows(IllegalArgumentException.class, () -> {
            service.setVersion(null);
        });
    }

    @Test
    void testSetEmptyVersion() {
        assertThrows(IllegalArgumentException.class, () -> {
            service.setVersion("");
        });
    }

    @Test
    void testEquality() {
        ServiceInfo service1 = new ServiceInfo("id-1", "Service 1", "ws://localhost:8080");
        ServiceInfo service2 = new ServiceInfo("id-1", "Service 2", "ws://localhost:8081");
        ServiceInfo service3 = new ServiceInfo("id-2", "Service 1", "ws://localhost:8080");

        assertEquals(service1, service2); // Same ID
        assertNotEquals(service1, service3); // Different ID
    }

    @Test
    void testHashCode() {
        ServiceInfo service1 = new ServiceInfo("id-1", "Service 1", "ws://localhost:8080");
        ServiceInfo service2 = new ServiceInfo("id-1", "Service 2", "ws://localhost:8081");

        assertEquals(service1.hashCode(), service2.hashCode());
    }

    @Test
    void testToString() {
        service.addCapability("storage");
        String str = service.toString();

        assertTrue(str.contains("service-001"));
        assertTrue(str.contains("Test Service"));
        assertTrue(str.contains("ws://localhost:8080"));
    }
}

