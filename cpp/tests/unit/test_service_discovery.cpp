/**
 * @file test_service_discovery.cpp
 * @brief Unit tests for Service Discovery
 */

#include <gtest/gtest.h>
#include "service_discovery.h"
#include <thread>
#include <chrono>

using namespace umicp;

class ServiceDiscoveryTest : public ::testing::Test {
protected:
    void SetUp() override {
        discovery = std::make_unique<ServiceDiscovery>();
    }

    void TearDown() override {
        discovery.reset();
    }

    std::unique_ptr<ServiceDiscovery> discovery;
};

// ============================================================================
// ServiceInfo Tests
// ============================================================================

TEST_F(ServiceDiscoveryTest, ServiceInfoCreation) {
    ServiceInfo service("service-001", "Test Service", "ws://localhost:8080");

    EXPECT_EQ(service.getServiceId(), "service-001");
    EXPECT_EQ(service.getName(), "Test Service");
    EXPECT_EQ(service.getAddress(), "ws://localhost:8080");
    EXPECT_EQ(service.getVersion(), "1.0.0");
    EXPECT_TRUE(service.getCapabilities().empty());
    EXPECT_TRUE(service.getMetadata().empty());
}

TEST_F(ServiceDiscoveryTest, ServiceInfoCapabilities) {
    ServiceInfo service("service-001", "Test Service", "ws://localhost:8080");

    service.addCapability("storage");
    service.addCapability("compute");
    service.addCapability("storage"); // Duplicate

    EXPECT_EQ(service.getCapabilities().size(), 2);
    EXPECT_TRUE(service.hasCapability("storage"));
    EXPECT_TRUE(service.hasCapability("compute"));
    EXPECT_FALSE(service.hasCapability("network"));
}

TEST_F(ServiceDiscoveryTest, ServiceInfoMetadata) {
    ServiceInfo service("service-001", "Test Service", "ws://localhost:8080");

    service.addMetadata("region", "us-east-1");
    service.addMetadata("environment", "production");

    EXPECT_EQ(service.getMetadataValue("region"), "us-east-1");
    EXPECT_EQ(service.getMetadataValue("environment"), "production");
    EXPECT_EQ(service.getMetadataValue("nonexistent"), "");
}

TEST_F(ServiceDiscoveryTest, ServiceInfoLastSeen) {
    ServiceInfo service("service-001", "Test Service", "ws://localhost:8080");

    auto initial_time = service.getLastSeen();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    service.updateLastSeen();
    auto updated_time = service.getLastSeen();

    EXPECT_GT(updated_time, initial_time);
}

TEST_F(ServiceDiscoveryTest, ServiceInfoStaleDetection) {
    ServiceInfo service("service-001", "Test Service", "ws://localhost:8080");

    // Fresh service should not be stale
    EXPECT_FALSE(service.isStale(60));

    // Simulate old service by not updating last_seen
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Should be stale with 1 second timeout
    EXPECT_TRUE(service.isStale(1));

    // Should not be stale with 10 second timeout
    EXPECT_FALSE(service.isStale(10));
}

// ============================================================================
// ServiceDiscovery Tests
// ============================================================================

TEST_F(ServiceDiscoveryTest, RegisterAndGetService) {
    ServiceInfo service("service-001", "Test Service", "ws://localhost:8080");

    discovery->registerService(service);

    auto retrieved = discovery->getService("service-001");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getServiceId(), "service-001");
    EXPECT_EQ(retrieved->getName(), "Test Service");
}

TEST_F(ServiceDiscoveryTest, RegisterLocalService) {
    ServiceInfo service("local-001", "Local Service", "ws://localhost:8080");
    service.addCapability("local");

    discovery->registerLocal(service);

    auto local = discovery->getLocalService();
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->getServiceId(), "local-001");
    EXPECT_TRUE(local->hasCapability("local"));
}

TEST_F(ServiceDiscoveryTest, UnregisterService) {
    ServiceInfo service("service-001", "Test Service", "ws://localhost:8080");

    discovery->registerService(service);
    EXPECT_EQ(discovery->getServiceCount(), 1);

    bool removed = discovery->unregisterService("service-001");
    EXPECT_TRUE(removed);
    EXPECT_EQ(discovery->getServiceCount(), 0);

    // Try to unregister non-existent service
    removed = discovery->unregisterService("nonexistent");
    EXPECT_FALSE(removed);
}

TEST_F(ServiceDiscoveryTest, GetAllServices) {
    ServiceInfo service1("service-001", "Service 1", "ws://localhost:8080");
    ServiceInfo service2("service-002", "Service 2", "ws://localhost:8081");
    ServiceInfo service3("service-003", "Service 3", "ws://localhost:8082");

    discovery->registerService(service1);
    discovery->registerService(service2);
    discovery->registerService(service3);

    auto services = discovery->getAllServices();
    EXPECT_EQ(services.size(), 3);
}

TEST_F(ServiceDiscoveryTest, FindByCapability) {
    ServiceInfo service1("service-001", "Storage Service", "ws://localhost:8080");
    service1.addCapability("storage");
    service1.addCapability("cache");

    ServiceInfo service2("service-002", "Compute Service", "ws://localhost:8081");
    service2.addCapability("compute");
    service2.addCapability("ml");

    ServiceInfo service3("service-003", "Hybrid Service", "ws://localhost:8082");
    service3.addCapability("storage");
    service3.addCapability("compute");

    discovery->registerService(service1);
    discovery->registerService(service2);
    discovery->registerService(service3);

    // Find storage services
    auto storage_services = discovery->findByCapability("storage");
    EXPECT_EQ(storage_services.size(), 2);

    // Find compute services
    auto compute_services = discovery->findByCapability("compute");
    EXPECT_EQ(compute_services.size(), 2);

    // Find cache services
    auto cache_services = discovery->findByCapability("cache");
    EXPECT_EQ(cache_services.size(), 1);

    // Find non-existent capability
    auto network_services = discovery->findByCapability("network");
    EXPECT_EQ(network_services.size(), 0);
}

TEST_F(ServiceDiscoveryTest, FindByMetadata) {
    ServiceInfo service1("service-001", "Service 1", "ws://localhost:8080");
    service1.addMetadata("region", "us-east-1");
    service1.addMetadata("environment", "production");

    ServiceInfo service2("service-002", "Service 2", "ws://localhost:8081");
    service2.addMetadata("region", "us-west-1");
    service2.addMetadata("environment", "production");

    ServiceInfo service3("service-003", "Service 3", "ws://localhost:8082");
    service3.addMetadata("region", "us-east-1");
    service3.addMetadata("environment", "development");

    discovery->registerService(service1);
    discovery->registerService(service2);
    discovery->registerService(service3);

    // Find by region
    auto east_services = discovery->findByMetadata("region", "us-east-1");
    EXPECT_EQ(east_services.size(), 2);

    auto west_services = discovery->findByMetadata("region", "us-west-1");
    EXPECT_EQ(west_services.size(), 1);

    // Find by environment
    auto prod_services = discovery->findByMetadata("environment", "production");
    EXPECT_EQ(prod_services.size(), 2);

    auto dev_services = discovery->findByMetadata("environment", "development");
    EXPECT_EQ(dev_services.size(), 1);
}

TEST_F(ServiceDiscoveryTest, FindByName) {
    ServiceInfo service1("service-001", "Storage Service", "ws://localhost:8080");
    ServiceInfo service2("service-002", "Compute Service", "ws://localhost:8081");
    ServiceInfo service3("service-003", "Storage Cache Service", "ws://localhost:8082");

    discovery->registerService(service1);
    discovery->registerService(service2);
    discovery->registerService(service3);

    // Find by name pattern
    auto storage_services = discovery->findByName("Storage");
    EXPECT_EQ(storage_services.size(), 2);

    auto compute_services = discovery->findByName("Compute");
    EXPECT_EQ(compute_services.size(), 1);

    auto cache_services = discovery->findByName("Cache");
    EXPECT_EQ(cache_services.size(), 1);

    auto service_services = discovery->findByName("Service");
    EXPECT_EQ(service_services.size(), 3);
}

TEST_F(ServiceDiscoveryTest, CleanupStaleServices) {
    // Create discovery with short timeout
    auto short_discovery = std::make_unique<ServiceDiscovery>(1); // 1 second timeout

    ServiceInfo service1("service-001", "Service 1", "ws://localhost:8080");
    ServiceInfo service2("service-002", "Service 2", "ws://localhost:8081");

    short_discovery->registerService(service1);
    short_discovery->registerService(service2);

    EXPECT_EQ(short_discovery->getServiceCount(), 2);

    // Wait for services to become stale
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Cleanup stale services
    int removed = short_discovery->cleanupStaleServices();
    EXPECT_EQ(removed, 2);
    EXPECT_EQ(short_discovery->getServiceCount(), 0);
}

TEST_F(ServiceDiscoveryTest, CleanupPreservesLocalService) {
    // Create discovery with short timeout
    auto short_discovery = std::make_unique<ServiceDiscovery>(1);

    ServiceInfo local_service("local-001", "Local Service", "ws://localhost:8080");
    ServiceInfo remote_service("remote-001", "Remote Service", "ws://localhost:8081");

    short_discovery->registerLocal(local_service);
    short_discovery->registerService(remote_service);

    EXPECT_EQ(short_discovery->getServiceCount(), 2);

    // Wait for services to become stale
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Cleanup should preserve local service
    int removed = short_discovery->cleanupStaleServices();
    EXPECT_EQ(removed, 1); // Only remote service removed
    EXPECT_EQ(short_discovery->getServiceCount(), 1);

    auto local = short_discovery->getLocalService();
    ASSERT_NE(local, nullptr);
    EXPECT_EQ(local->getServiceId(), "local-001");
}

TEST_F(ServiceDiscoveryTest, TimeoutConfiguration) {
    EXPECT_EQ(discovery->getTimeout(), 60); // Default timeout

    discovery->setTimeout(120);
    EXPECT_EQ(discovery->getTimeout(), 120);

    auto custom_discovery = std::make_unique<ServiceDiscovery>(30);
    EXPECT_EQ(custom_discovery->getTimeout(), 30);
}

TEST_F(ServiceDiscoveryTest, AnnounceService) {
    ServiceInfo service("service-001", "Test Service", "ws://localhost:8080");

    bool announced = discovery->announceService(service);
    EXPECT_TRUE(announced);

    // Service should be registered after announcement
    auto retrieved = discovery->getService("service-001");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getServiceId(), "service-001");
}

TEST_F(ServiceDiscoveryTest, UpdateExistingService) {
    ServiceInfo service1("service-001", "Service V1", "ws://localhost:8080");
    service1.addCapability("v1-capability");

    discovery->registerService(service1);

    // Register updated version
    ServiceInfo service2("service-001", "Service V2", "ws://localhost:8080");
    service2.addCapability("v2-capability");
    service2.setVersion("2.0.0");

    discovery->registerService(service2);

    // Should still have only one service
    EXPECT_EQ(discovery->getServiceCount(), 1);

    // Should have updated information
    auto retrieved = discovery->getService("service-001");
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->getName(), "Service V2");
    EXPECT_TRUE(retrieved->hasCapability("v2-capability"));
    EXPECT_EQ(retrieved->getVersion(), "2.0.0");
}

TEST_F(ServiceDiscoveryTest, ThreadSafety) {
    // Test concurrent operations
    const int num_threads = 10;
    const int services_per_thread = 10;

    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; t++) {
        threads.emplace_back([this, t, services_per_thread]() {
            for (int i = 0; i < services_per_thread; i++) {
                std::string id = "service-" + std::to_string(t) + "-" + std::to_string(i);
                ServiceInfo service(id, "Service " + id, "ws://localhost:808" + std::to_string(i));
                service.addCapability("capability-" + std::to_string(t));

                discovery->registerService(service);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(discovery->getServiceCount(), num_threads * services_per_thread);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

