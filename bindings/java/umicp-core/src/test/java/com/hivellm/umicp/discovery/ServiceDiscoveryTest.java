package com.hivellm.umicp.discovery;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeEach;
import static org.junit.jupiter.api.Assertions.*;

import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.time.Instant;

/**
 * Unit tests for ServiceDiscovery
 */
class ServiceDiscoveryTest {
    private ServiceDiscovery discovery;

    @BeforeEach
    void setUp() {
        discovery = new ServiceDiscovery();
    }

    @Test
    void testCreation() {
        assertNotNull(discovery);
        assertEquals(60, discovery.getTimeout());
        assertEquals(0, discovery.getServiceCount());
    }

    @Test
    void testCreationWithCustomTimeout() {
        ServiceDiscovery customDiscovery = new ServiceDiscovery(120);
        assertEquals(120, customDiscovery.getTimeout());
    }

    @Test
    void testRegisterService() {
        ServiceInfo service = new ServiceInfo("service-001", "Test Service", "ws://localhost:8080");
        discovery.registerService(service);

        assertEquals(1, discovery.getServiceCount());

        ServiceInfo retrieved = discovery.getService("service-001");
        assertNotNull(retrieved);
        assertEquals("service-001", retrieved.getServiceId());
    }

    @Test
    void testRegisterNullService() {
        assertThrows(IllegalArgumentException.class, () -> {
            discovery.registerService(null);
        });
    }

    @Test
    void testRegisterLocalService() {
        ServiceInfo service = new ServiceInfo("local-001", "Local Service", "ws://localhost:8080");
        service.addCapability("local");

        discovery.registerLocal(service);

        ServiceInfo local = discovery.getLocalService();
        assertNotNull(local);
        assertEquals("local-001", local.getServiceId());
        assertTrue(local.hasCapability("local"));
    }

    @Test
    void testUnregisterService() {
        ServiceInfo service = new ServiceInfo("service-001", "Test Service", "ws://localhost:8080");
        discovery.registerService(service);

        assertEquals(1, discovery.getServiceCount());

        boolean removed = discovery.unregisterService("service-001");
        assertTrue(removed);
        assertEquals(0, discovery.getServiceCount());
    }

    @Test
    void testUnregisterNonExistentService() {
        boolean removed = discovery.unregisterService("nonexistent");
        assertFalse(removed);
    }

    @Test
    void testUnregisterNullServiceId() {
        boolean removed = discovery.unregisterService(null);
        assertFalse(removed);
    }

    @Test
    void testGetAllServices() {
        ServiceInfo service1 = new ServiceInfo("service-001", "Service 1", "ws://localhost:8080");
        ServiceInfo service2 = new ServiceInfo("service-002", "Service 2", "ws://localhost:8081");
        ServiceInfo service3 = new ServiceInfo("service-003", "Service 3", "ws://localhost:8082");

        discovery.registerService(service1);
        discovery.registerService(service2);
        discovery.registerService(service3);

        List<ServiceInfo> services = discovery.getAllServices();
        assertEquals(3, services.size());
    }

    @Test
    void testFindByCapability() {
        ServiceInfo service1 = new ServiceInfo("service-001", "Storage Service", "ws://localhost:8080");
        service1.addCapability("storage");
        service1.addCapability("cache");

        ServiceInfo service2 = new ServiceInfo("service-002", "Compute Service", "ws://localhost:8081");
        service2.addCapability("compute");
        service2.addCapability("ml");

        ServiceInfo service3 = new ServiceInfo("service-003", "Hybrid Service", "ws://localhost:8082");
        service3.addCapability("storage");
        service3.addCapability("compute");

        discovery.registerService(service1);
        discovery.registerService(service2);
        discovery.registerService(service3);

        List<ServiceInfo> storageServices = discovery.findByCapability("storage");
        assertEquals(2, storageServices.size());

        List<ServiceInfo> computeServices = discovery.findByCapability("compute");
        assertEquals(2, computeServices.size());

        List<ServiceInfo> cacheServices = discovery.findByCapability("cache");
        assertEquals(1, cacheServices.size());

        List<ServiceInfo> networkServices = discovery.findByCapability("network");
        assertEquals(0, networkServices.size());
    }

    @Test
    void testFindByCapabilityNull() {
        List<ServiceInfo> services = discovery.findByCapability(null);
        assertTrue(services.isEmpty());
    }

    @Test
    void testFindByMetadata() {
        ServiceInfo service1 = new ServiceInfo("service-001", "Service 1", "ws://localhost:8080");
        service1.addMetadata("region", "us-east-1");
        service1.addMetadata("environment", "production");

        ServiceInfo service2 = new ServiceInfo("service-002", "Service 2", "ws://localhost:8081");
        service2.addMetadata("region", "us-west-1");
        service2.addMetadata("environment", "production");

        ServiceInfo service3 = new ServiceInfo("service-003", "Service 3", "ws://localhost:8082");
        service3.addMetadata("region", "us-east-1");
        service3.addMetadata("environment", "development");

        discovery.registerService(service1);
        discovery.registerService(service2);
        discovery.registerService(service3);

        List<ServiceInfo> eastServices = discovery.findByMetadata("region", "us-east-1");
        assertEquals(2, eastServices.size());

        List<ServiceInfo> westServices = discovery.findByMetadata("region", "us-west-1");
        assertEquals(1, westServices.size());

        List<ServiceInfo> prodServices = discovery.findByMetadata("environment", "production");
        assertEquals(2, prodServices.size());
    }

    @Test
    void testFindByMetadataNull() {
        List<ServiceInfo> services = discovery.findByMetadata(null, "value");
        assertTrue(services.isEmpty());

        services = discovery.findByMetadata("key", null);
        assertTrue(services.isEmpty());
    }

    @Test
    void testFindByName() {
        ServiceInfo service1 = new ServiceInfo("service-001", "Storage Service", "ws://localhost:8080");
        ServiceInfo service2 = new ServiceInfo("service-002", "Compute Service", "ws://localhost:8081");
        ServiceInfo service3 = new ServiceInfo("service-003", "Storage Cache Service", "ws://localhost:8082");

        discovery.registerService(service1);
        discovery.registerService(service2);
        discovery.registerService(service3);

        List<ServiceInfo> storageServices = discovery.findByName("Storage");
        assertEquals(2, storageServices.size());

        List<ServiceInfo> computeServices = discovery.findByName("Compute");
        assertEquals(1, computeServices.size());

        List<ServiceInfo> cacheServices = discovery.findByName("Cache");
        assertEquals(1, cacheServices.size());

        List<ServiceInfo> allServices = discovery.findByName("Service");
        assertEquals(3, allServices.size());
    }

    @Test
    void testFindByNameNull() {
        List<ServiceInfo> services = discovery.findByName(null);
        assertTrue(services.isEmpty());
    }

    @Test
    void testCleanupStaleServices() throws InterruptedException {
        ServiceDiscovery shortDiscovery = new ServiceDiscovery(1); // 1 second timeout

        ServiceInfo service1 = new ServiceInfo("service-001", "Service 1", "ws://localhost:8080");
        ServiceInfo service2 = new ServiceInfo("service-002", "Service 2", "ws://localhost:8081");

        shortDiscovery.registerService(service1);
        shortDiscovery.registerService(service2);

        assertEquals(2, shortDiscovery.getServiceCount());

        // Wait for services to become stale
        Thread.sleep(2000);

        int removed = shortDiscovery.cleanupStaleServices();
        assertEquals(2, removed);
        assertEquals(0, shortDiscovery.getServiceCount());
    }

    @Test
    void testCleanupPreservesLocalService() throws InterruptedException {
        ServiceDiscovery shortDiscovery = new ServiceDiscovery(1);

        ServiceInfo localService = new ServiceInfo("local-001", "Local Service", "ws://localhost:8080");
        ServiceInfo remoteService = new ServiceInfo("remote-001", "Remote Service", "ws://localhost:8081");

        shortDiscovery.registerLocal(localService);
        shortDiscovery.registerService(remoteService);

        assertEquals(2, shortDiscovery.getServiceCount());

        // Wait for services to become stale
        Thread.sleep(2000);

        int removed = shortDiscovery.cleanupStaleServices();
        assertEquals(1, removed); // Only remote service removed
        assertEquals(1, shortDiscovery.getServiceCount());

        ServiceInfo local = shortDiscovery.getLocalService();
        assertNotNull(local);
        assertEquals("local-001", local.getServiceId());
    }

    @Test
    void testSetTimeout() {
        assertEquals(60, discovery.getTimeout());

        discovery.setTimeout(120);
        assertEquals(120, discovery.getTimeout());
    }

    @Test
    void testSetNegativeTimeout() {
        assertThrows(IllegalArgumentException.class, () -> {
            discovery.setTimeout(-1);
        });
    }

    @Test
    void testAnnounceService() {
        ServiceInfo service = new ServiceInfo("service-001", "Test Service", "ws://localhost:8080");

        boolean announced = discovery.announceService(service);
        assertTrue(announced);

        ServiceInfo retrieved = discovery.getService("service-001");
        assertNotNull(retrieved);
        assertEquals("service-001", retrieved.getServiceId());
    }

    @Test
    void testAnnounceNullService() {
        boolean announced = discovery.announceService(null);
        assertFalse(announced);
    }

    @Test
    void testUpdateExistingService() throws InterruptedException {
        ServiceInfo service1 = new ServiceInfo("service-001", "Service V1", "ws://localhost:8080");
        service1.addCapability("v1-capability");

        discovery.registerService(service1);
        Instant firstSeen = service1.getLastSeen();

        Thread.sleep(100);

        // Register same service again (simulates update)
        ServiceInfo service2 = new ServiceInfo("service-001", "Service V2", "ws://localhost:8080");
        service2.addCapability("v2-capability");
        service2.setVersion("2.0.0");

        discovery.registerService(service2);

        // Should still have only one service
        assertEquals(1, discovery.getServiceCount());

        // Last seen should be updated
        ServiceInfo retrieved = discovery.getService("service-001");
        assertNotNull(retrieved);
        assertTrue(retrieved.getLastSeen().isAfter(firstSeen));
    }

    @Test
    void testThreadSafety() throws InterruptedException {
        int numThreads = 10;
        int servicesPerThread = 10;
        ExecutorService executor = Executors.newFixedThreadPool(numThreads);
        CountDownLatch latch = new CountDownLatch(numThreads);

        for (int t = 0; t < numThreads; t++) {
            final int threadId = t;
            executor.submit(() -> {
                try {
                    for (int i = 0; i < servicesPerThread; i++) {
                        String id = "service-" + threadId + "-" + i;
                        ServiceInfo service = new ServiceInfo(id, "Service " + id, "ws://localhost:808" + i);
                        service.addCapability("capability-" + threadId);
                        discovery.registerService(service);
                    }
                } finally {
                    latch.countDown();
                }
            });
        }

        latch.await(10, TimeUnit.SECONDS);
        executor.shutdown();

        assertEquals(numThreads * servicesPerThread, discovery.getServiceCount());
    }

    @Test
    void testGetServiceNull() {
        assertNull(discovery.getService(null));
        assertNull(discovery.getService(""));
    }
}

