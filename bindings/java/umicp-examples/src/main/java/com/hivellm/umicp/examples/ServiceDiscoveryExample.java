package com.hivellm.umicp.examples;

import com.hivellm.umicp.discovery.ServiceInfo;
import com.hivellm.umicp.discovery.ServiceDiscovery;

import java.util.List;

/**
 * Example demonstrating UMICP Service Discovery
 */
public class ServiceDiscoveryExample {
    public static void main(String[] args) throws InterruptedException {
        System.out.println("=== UMICP Service Discovery Example ===\n");

        // Create service discovery manager
        ServiceDiscovery discovery = new ServiceDiscovery();
        System.out.println("✓ Service discovery manager created\n");

        // Register local service
        System.out.println("1. Registering local service...");
        ServiceInfo localService = new ServiceInfo("service-local", "My Local Service", "ws://localhost:8080");
        localService.addCapability("storage");
        localService.addCapability("cache");
        localService.addMetadata("region", "us-east-1");
        localService.addMetadata("version", "1.0.0");
        localService.addMetadata("environment", "production");

        discovery.registerLocal(localService);
        System.out.println("   ✓ Local service registered: " + localService.getName());
        System.out.println("     - ID: " + localService.getServiceId());
        System.out.println("     - Address: " + localService.getAddress());
        System.out.println("     - Capabilities: " + localService.getCapabilities());
        System.out.println();

        // Register more services
        System.out.println("2. Discovering remote services...");

        ServiceInfo computeService1 = new ServiceInfo("service-compute-1", "Compute Service A", "ws://192.168.1.10:8080");
        computeService1.addCapability("compute");
        computeService1.addCapability("ml");
        computeService1.addCapability("training");
        computeService1.addMetadata("region", "us-west-1");
        computeService1.addMetadata("gpu", "nvidia-v100");
        computeService1.setVersion("2.1.0");
        discovery.registerService(computeService1);
        System.out.println("   ✓ Discovered: " + computeService1.getName());

        ServiceInfo computeService2 = new ServiceInfo("service-compute-2", "Compute Service B", "ws://192.168.1.11:8080");
        computeService2.addCapability("compute");
        computeService2.addCapability("ml");
        computeService2.addCapability("inference");
        computeService2.addMetadata("region", "us-west-1");
        computeService2.addMetadata("gpu", "nvidia-a100");
        computeService2.setVersion("2.2.0");
        discovery.registerService(computeService2);
        System.out.println("   ✓ Discovered: " + computeService2.getName());

        ServiceInfo storageService = new ServiceInfo("service-storage-1", "Storage Service", "ws://192.168.1.20:8080");
        storageService.addCapability("storage");
        storageService.addCapability("backup");
        storageService.addMetadata("region", "us-east-1");
        storageService.addMetadata("capacity", "100TB");
        discovery.registerService(storageService);
        System.out.println("   ✓ Discovered: " + storageService.getName());

        ServiceInfo networkService = new ServiceInfo("service-network-1", "Network Gateway", "ws://192.168.1.1:8080");
        networkService.addCapability("network");
        networkService.addCapability("routing");
        networkService.addMetadata("region", "us-east-1");
        networkService.addMetadata("bandwidth", "10Gbps");
        discovery.registerService(networkService);
        System.out.println("   ✓ Discovered: " + networkService.getName());
        System.out.println();

        // Show all services
        System.out.println("3. All registered services (" + discovery.getServiceCount() + " total):");
        List<ServiceInfo> allServices = discovery.getAllServices();
        for (ServiceInfo service : allServices) {
            System.out.println("   - " + service.getName() + " (" + service.getServiceId() + ")");
        }
        System.out.println();

        // Find services by capability
        System.out.println("4. Finding services by capability:");

        List<ServiceInfo> computeServices = discovery.findByCapability("compute");
        System.out.println("   Compute services (" + computeServices.size() + "):");
        for (ServiceInfo service : computeServices) {
            System.out.println("     - " + service.getName() + " at " + service.getAddress());
        }

        List<ServiceInfo> storageServices = discovery.findByCapability("storage");
        System.out.println("   Storage services (" + storageServices.size() + "):");
        for (ServiceInfo service : storageServices) {
            System.out.println("     - " + service.getName() + " at " + service.getAddress());
        }

        List<ServiceInfo> mlServices = discovery.findByCapability("ml");
        System.out.println("   ML services (" + mlServices.size() + "):");
        for (ServiceInfo service : mlServices) {
            System.out.println("     - " + service.getName() + " at " + service.getAddress());
        }
        System.out.println();

        // Find services by metadata
        System.out.println("5. Finding services by metadata:");

        List<ServiceInfo> usEast = discovery.findByMetadata("region", "us-east-1");
        System.out.println("   US-EAST-1 region (" + usEast.size() + "):");
        for (ServiceInfo service : usEast) {
            System.out.println("     - " + service.getName());
        }

        List<ServiceInfo> usWest = discovery.findByMetadata("region", "us-west-1");
        System.out.println("   US-WEST-1 region (" + usWest.size() + "):");
        for (ServiceInfo service : usWest) {
            System.out.println("     - " + service.getName());
        }
        System.out.println();

        // Find by name pattern
        System.out.println("6. Finding services by name pattern:");
        List<ServiceInfo> computeByName = discovery.findByName("Compute");
        System.out.println("   Services with 'Compute' in name (" + computeByName.size() + "):");
        for (ServiceInfo service : computeByName) {
            System.out.println("     - " + service.getName());
        }
        System.out.println();

        // Get specific service
        System.out.println("7. Getting specific service by ID:");
        ServiceInfo specific = discovery.getService("service-compute-1");
        if (specific != null) {
            System.out.println("   Found: " + specific.getName());
            System.out.println("   Version: " + specific.getVersion());
            System.out.println("   GPU: " + specific.getMetadataValue("gpu"));
            System.out.println("   Capabilities: " + specific.getCapabilities());
        }
        System.out.println();

        // Demonstrate stale service cleanup
        System.out.println("8. Testing stale service cleanup:");
        ServiceDiscovery shortDiscovery = new ServiceDiscovery(2); // 2 second timeout

        ServiceInfo tempService = new ServiceInfo("temp-001", "Temporary Service", "ws://localhost:9000");
        tempService.addCapability("temporary");
        shortDiscovery.registerService(tempService);

        System.out.println("   ✓ Registered temporary service");
        System.out.println("   Services count: " + shortDiscovery.getServiceCount());

        System.out.println("   Waiting 3 seconds for service to become stale...");
        Thread.sleep(3000);

        int removed = shortDiscovery.cleanupStaleServices();
        System.out.println("   ✓ Cleaned up " + removed + " stale service(s)");
        System.out.println("   Services count: " + shortDiscovery.getServiceCount());
        System.out.println();

        // Get local service
        System.out.println("9. Accessing local service:");
        ServiceInfo local = discovery.getLocalService();
        if (local != null) {
            System.out.println("   Local service: " + local.getName());
            System.out.println("   Address: " + local.getAddress());
        }
        System.out.println();

        System.out.println("=== Service Discovery Example Complete ===");
    }
}

