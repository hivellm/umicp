/**
 * @file service_discovery_example.cpp
 * @brief Example demonstrating UMICP Service Discovery
 */

#include "service_discovery.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace umicp;

int main() {
    std::cout << "=== UMICP Service Discovery Example ===\n\n";

    // Create service discovery manager
    ServiceDiscovery discovery;
    std::cout << "✓ Service discovery manager created\n\n";

    // Register local service
    std::cout << "1. Registering local service...\n";
    ServiceInfo local_service("service-local", "My Local Service", "ws://localhost:8080");
    local_service.addCapability("storage");
    local_service.addCapability("cache");
    local_service.addMetadata("region", "us-east-1");
    local_service.addMetadata("version", "1.0.0");
    local_service.addMetadata("environment", "production");

    discovery.registerLocal(local_service);
    std::cout << "   ✓ Local service registered: " << local_service.getName() << "\n";
    std::cout << "     - ID: " << local_service.getServiceId() << "\n";
    std::cout << "     - Address: " << local_service.getAddress() << "\n";
    std::cout << "     - Capabilities: ";
    for (const auto& cap : local_service.getCapabilities()) {
        std::cout << cap << " ";
    }
    std::cout << "\n\n";

    // Register more services
    std::cout << "2. Discovering remote services...\n";

    ServiceInfo compute_service("service-compute-1", "Compute Service A", "ws://192.168.1.10:8080");
    compute_service.addCapability("compute");
    compute_service.addCapability("ml");
    compute_service.addCapability("training");
    compute_service.addMetadata("region", "us-west-1");
    compute_service.addMetadata("gpu", "nvidia-v100");
    compute_service.setVersion("2.1.0");
    discovery.registerService(compute_service);
    std::cout << "   ✓ Discovered: " << compute_service.getName() << "\n";

    ServiceInfo compute_service_2("service-compute-2", "Compute Service B", "ws://192.168.1.11:8080");
    compute_service_2.addCapability("compute");
    compute_service_2.addCapability("ml");
    compute_service_2.addCapability("inference");
    compute_service_2.addMetadata("region", "us-west-1");
    compute_service_2.addMetadata("gpu", "nvidia-a100");
    compute_service_2.setVersion("2.2.0");
    discovery.registerService(compute_service_2);
    std::cout << "   ✓ Discovered: " << compute_service_2.getName() << "\n";

    ServiceInfo storage_service("service-storage-1", "Storage Service", "ws://192.168.1.20:8080");
    storage_service.addCapability("storage");
    storage_service.addCapability("backup");
    storage_service.addMetadata("region", "us-east-1");
    storage_service.addMetadata("capacity", "100TB");
    discovery.registerService(storage_service);
    std::cout << "   ✓ Discovered: " << storage_service.getName() << "\n";

    ServiceInfo network_service("service-network-1", "Network Gateway", "ws://192.168.1.1:8080");
    network_service.addCapability("network");
    network_service.addCapability("routing");
    network_service.addMetadata("region", "us-east-1");
    network_service.addMetadata("bandwidth", "10Gbps");
    discovery.registerService(network_service);
    std::cout << "   ✓ Discovered: " << network_service.getName() << "\n\n";

    // Show all services
    std::cout << "3. All registered services (" << discovery.getServiceCount() << " total):\n";
    auto all_services = discovery.getAllServices();
    for (const auto& service : all_services) {
        std::cout << "   - " << service->getName()
                  << " (" << service->getServiceId() << ")\n";
    }
    std::cout << "\n";

    // Find services by capability
    std::cout << "4. Finding services by capability:\n";

    auto compute_services = discovery.findByCapability("compute");
    std::cout << "   Compute services (" << compute_services.size() << "):\n";
    for (const auto& service : compute_services) {
        std::cout << "     - " << service->getName()
                  << " at " << service->getAddress() << "\n";
    }

    auto storage_services = discovery.findByCapability("storage");
    std::cout << "   Storage services (" << storage_services.size() << "):\n";
    for (const auto& service : storage_services) {
        std::cout << "     - " << service->getName()
                  << " at " << service->getAddress() << "\n";
    }

    auto ml_services = discovery.findByCapability("ml");
    std::cout << "   ML services (" << ml_services.size() << "):\n";
    for (const auto& service : ml_services) {
        std::cout << "     - " << service->getName()
                  << " at " << service->getAddress() << "\n";
    }
    std::cout << "\n";

    // Find services by metadata
    std::cout << "5. Finding services by metadata:\n";

    auto us_east = discovery.findByMetadata("region", "us-east-1");
    std::cout << "   US-EAST-1 region (" << us_east.size() << "):\n";
    for (const auto& service : us_east) {
        std::cout << "     - " << service->getName() << "\n";
    }

    auto us_west = discovery.findByMetadata("region", "us-west-1");
    std::cout << "   US-WEST-1 region (" << us_west.size() << "):\n";
    for (const auto& service : us_west) {
        std::cout << "     - " << service->getName() << "\n";
    }
    std::cout << "\n";

    // Find by name pattern
    std::cout << "6. Finding services by name pattern:\n";

    auto compute_by_name = discovery.findByName("Compute");
    std::cout << "   Services with 'Compute' in name (" << compute_by_name.size() << "):\n";
    for (const auto& service : compute_by_name) {
        std::cout << "     - " << service->getName() << "\n";
    }
    std::cout << "\n";

    // Get specific service
    std::cout << "7. Getting specific service by ID:\n";
    auto specific = discovery.getService("service-compute-1");
    if (specific) {
        std::cout << "   Found: " << specific->getName() << "\n";
        std::cout << "   Version: " << specific->getVersion() << "\n";
        std::cout << "   GPU: " << specific->getMetadataValue("gpu") << "\n";
        std::cout << "   Capabilities: ";
        for (const auto& cap : specific->getCapabilities()) {
            std::cout << cap << " ";
        }
        std::cout << "\n\n";
    }

    // Demonstrate stale service cleanup
    std::cout << "8. Testing stale service cleanup:\n";

    // Create a new discovery manager with 2-second timeout
    ServiceDiscovery short_discovery(2);

    ServiceInfo temp_service("temp-001", "Temporary Service", "ws://localhost:9000");
    temp_service.addCapability("temporary");
    short_discovery.registerService(temp_service);

    std::cout << "   ✓ Registered temporary service\n";
    std::cout << "   Services count: " << short_discovery.getServiceCount() << "\n";

    std::cout << "   Waiting 3 seconds for service to become stale...\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    int removed = short_discovery.cleanupStaleServices();
    std::cout << "   ✓ Cleaned up " << removed << " stale service(s)\n";
    std::cout << "   Services count: " << short_discovery.getServiceCount() << "\n\n";

    // Get local service
    std::cout << "9. Accessing local service:\n";
    auto local = discovery.getLocalService();
    if (local) {
        std::cout << "   Local service: " << local->getName() << "\n";
        std::cout << "   Address: " << local->getAddress() << "\n";
    }
    std::cout << "\n";

    std::cout << "=== Service Discovery Example Complete ===\n";
    return 0;
}

