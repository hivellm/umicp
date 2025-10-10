using Umicp.Core.Discovery;
using Umicp.Core.Events;

namespace Umicp.Examples;

/// <summary>
/// Service discovery example
/// </summary>
public class ServiceDiscoveryExample
{
    public static async Task Run()
    {
        Console.WriteLine("=== Service Discovery Example ===\n");

        // Create service discovery instance
        using var discovery = new ServiceDiscovery(
            heartbeatInterval: TimeSpan.FromSeconds(10),
            serviceTimeout: TimeSpan.FromSeconds(30)
        );

        // Subscribe to events
        discovery.Events.On(EventType.Connect, evt =>
        {
            if (evt.Data.TryGetValue("service_id", out var serviceId))
            {
                Console.WriteLine($"Service registered: {serviceId}");
            }
        });

        discovery.Events.On(EventType.Disconnect, evt =>
        {
            if (evt.Data.TryGetValue("service_id", out var serviceId))
            {
                Console.WriteLine($"Service unregistered: {serviceId}");
            }
        });

        discovery.Events.On(EventType.Timeout, evt =>
        {
            if (evt.Data.TryGetValue("service_id", out var serviceId))
            {
                Console.WriteLine($"Service timeout: {serviceId}");
            }
        });

        // Register services
        var service1 = new ServiceInfo
        {
            Id = "service-001",
            Name = "API Gateway",
            Type = "gateway",
            Endpoint = "http://localhost:3000",
            Host = "localhost",
            Port = 3000,
            Health = "healthy"
        };
        service1.Metadata["region"] = "us-east-1";
        service1.Metadata["version"] = "1.0.0";

        var service2 = new ServiceInfo
        {
            Id = "service-002",
            Name = "Auth Service",
            Type = "auth",
            Endpoint = "http://localhost:3001",
            Host = "localhost",
            Port = 3001,
            Health = "healthy"
        };
        service2.Metadata["region"] = "us-east-1";

        Console.WriteLine("Registering services...");
        discovery.RegisterService(service1);
        discovery.RegisterService(service2);

        Console.WriteLine($"Total services: {discovery.ServiceCount}");
        Console.WriteLine($"Healthy services: {discovery.HealthyServiceCount}\n");

        // Find services by type
        var gateways = discovery.FindByType("gateway");
        Console.WriteLine($"Gateway services: {gateways.Length}");
        foreach (var svc in gateways)
        {
            Console.WriteLine($"  - {svc.Name} ({svc.Endpoint})");
        }

        // Find services by name
        var authServices = discovery.FindByName("Auth Service");
        Console.WriteLine($"\nAuth services: {authServices.Length}");
        foreach (var svc in authServices)
        {
            Console.WriteLine($"  - {svc.Id} ({svc.Endpoint})");
        }

        // Get specific service
        var specificService = discovery.GetService("service-001");
        if (specificService != null)
        {
            Console.WriteLine($"\nService details:");
            Console.WriteLine($"  ID: {specificService.Id}");
            Console.WriteLine($"  Name: {specificService.Name}");
            Console.WriteLine($"  Type: {specificService.Type}");
            Console.WriteLine($"  Endpoint: {specificService.Endpoint}");
            Console.WriteLine($"  Health: {specificService.Health}");
            Console.WriteLine($"  Metadata: {string.Join(", ", specificService.Metadata.Keys)}");
        }

        // Heartbeat
        Console.WriteLine("\nSending heartbeats...");
        discovery.Heartbeat("service-001");
        discovery.Heartbeat("service-002");

        // Update metadata
        discovery.UpdateMetadata("service-001", new Dictionary<string, object>
        {
            ["load"] = 0.45,
            ["requests"] = 1234
        });

        // List all healthy services
        Console.WriteLine("\nAll healthy services:");
        foreach (var svc in discovery.FindHealthyServices())
        {
            Console.WriteLine($"  - {svc.Name} ({svc.Id}) - {svc.Health}");
        }

        // Unregister a service
        Console.WriteLine("\nUnregistering service-002...");
        discovery.UnregisterService("service-002");
        Console.WriteLine($"Total services: {discovery.ServiceCount}\n");

        await Task.Delay(100); // Allow events to process
    }
}

