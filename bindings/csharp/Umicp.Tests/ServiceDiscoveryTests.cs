using Xunit;
using Umicp.SDK.Discovery;
using Umicp.SDK.Events;

namespace Umicp.Tests;

public class ServiceDiscoveryTests
{
    [Fact]
    public void ServiceDiscovery_ShouldInitialize()
    {
        // Arrange & Act
        using var discovery = new ServiceDiscovery();

        // Assert
        Assert.NotNull(discovery.Events);
        Assert.NotNull(discovery.Services);
        Assert.Equal(0, discovery.ServiceCount);
    }

    [Fact]
    public void ServiceDiscovery_ShouldRegisterService()
    {
        // Arrange
        using var discovery = new ServiceDiscovery();
        var service = new ServiceInfo
        {
            Id = "service-001",
            Name = "Test Service",
            Type = "test",
            Endpoint = "http://localhost:3000"
        };
        var eventFired = false;

        discovery.Events.On(EventType.Connect, evt => eventFired = true);

        // Act
        discovery.RegisterService(service);

        // Assert
        Assert.Equal(1, discovery.ServiceCount);
        Assert.Equal(1, discovery.HealthyServiceCount);
        Assert.True(eventFired);
    }

    [Fact]
    public void ServiceDiscovery_ShouldUnregisterService()
    {
        // Arrange
        using var discovery = new ServiceDiscovery();
        var service = new ServiceInfo("service-001", "Test", "test", "http://localhost:3000");
        discovery.RegisterService(service);
        var eventFired = false;

        discovery.Events.On(EventType.Disconnect, evt => eventFired = true);

        // Act
        var result = discovery.UnregisterService("service-001");

        // Assert
        Assert.True(result);
        Assert.Equal(0, discovery.ServiceCount);
        Assert.True(eventFired);
    }

    [Fact]
    public void ServiceDiscovery_ShouldGetService()
    {
        // Arrange
        using var discovery = new ServiceDiscovery();
        var service = new ServiceInfo("service-001", "Test", "test", "http://localhost:3000");
        discovery.RegisterService(service);

        // Act
        var retrieved = discovery.GetService("service-001");

        // Assert
        Assert.NotNull(retrieved);
        Assert.Equal("service-001", retrieved.Id);
    }

    [Fact]
    public void ServiceDiscovery_ShouldReturnNullForNonExistentService()
    {
        // Arrange
        using var discovery = new ServiceDiscovery();

        // Act
        var retrieved = discovery.GetService("non-existent");

        // Assert
        Assert.Null(retrieved);
    }

    [Fact]
    public void ServiceDiscovery_ShouldFindByType()
    {
        // Arrange
        using var discovery = new ServiceDiscovery();
        discovery.RegisterService(new ServiceInfo("s1", "API", "api", "http://localhost:3000"));
        discovery.RegisterService(new ServiceInfo("s2", "DB", "database", "http://localhost:3001"));
        discovery.RegisterService(new ServiceInfo("s3", "API2", "api", "http://localhost:3002"));

        // Act
        var apiServices = discovery.FindByType("api");

        // Assert
        Assert.Equal(2, apiServices.Length);
        Assert.All(apiServices, s => Assert.Equal("api", s.Type));
    }

    [Fact]
    public void ServiceDiscovery_ShouldFindByName()
    {
        // Arrange
        using var discovery = new ServiceDiscovery();
        discovery.RegisterService(new ServiceInfo("s1", "API Gateway", "api", "http://localhost:3000"));
        discovery.RegisterService(new ServiceInfo("s2", "Auth Service", "auth", "http://localhost:3001"));
        discovery.RegisterService(new ServiceInfo("s3", "API Gateway", "api", "http://localhost:3002"));

        // Act
        var gateways = discovery.FindByName("API Gateway");

        // Assert
        Assert.Equal(2, gateways.Length);
        Assert.All(gateways, s => Assert.Equal("API Gateway", s.Name));
    }

    [Fact]
    public void ServiceDiscovery_ShouldFindHealthyServices()
    {
        // Arrange
        using var discovery = new ServiceDiscovery();
        var s1 = new ServiceInfo("s1", "Service1", "type1", "http://localhost:3000");
        var s2 = new ServiceInfo("s2", "Service2", "type2", "http://localhost:3001");
        
        discovery.RegisterService(s1);
        discovery.RegisterService(s2);
        
        // Mark s2 as unhealthy after registration
        s2.Health = "unhealthy";

        // Act
        var healthy = discovery.FindHealthyServices();

        // Assert
        Assert.Single(healthy);
        Assert.Equal("s1", healthy[0].Id);
    }

    [Fact]
    public void ServiceDiscovery_ShouldUpdateHeartbeat()
    {
        // Arrange
        using var discovery = new ServiceDiscovery();
        var service = new ServiceInfo("service-001", "Test", "test", "http://localhost:3000");
        discovery.RegisterService(service);
        var originalHeartbeat = service.LastHeartbeat;
        
        System.Threading.Thread.Sleep(10);

        // Act
        discovery.Heartbeat("service-001");

        // Assert
        var retrieved = discovery.GetService("service-001");
        Assert.NotNull(retrieved);
        Assert.True(retrieved.LastHeartbeat > originalHeartbeat);
        Assert.Equal("healthy", retrieved.Health);
    }

    [Fact]
    public void ServiceDiscovery_ShouldUpdateMetadata()
    {
        // Arrange
        using var discovery = new ServiceDiscovery();
        var service = new ServiceInfo("service-001", "Test", "test", "http://localhost:3000");
        discovery.RegisterService(service);

        // Act
        discovery.UpdateMetadata("service-001", new Dictionary<string, object>
        {
            ["version"] = "1.0.0",
            ["region"] = "us-east-1"
        });

        // Assert
        var retrieved = discovery.GetService("service-001");
        Assert.NotNull(retrieved);
        Assert.Equal("1.0.0", retrieved.Metadata["version"]);
        Assert.Equal("us-east-1", retrieved.Metadata["region"]);
    }

    [Fact]
    public void ServiceDiscovery_ShouldClearServices()
    {
        // Arrange
        using var discovery = new ServiceDiscovery();
        discovery.RegisterService(new ServiceInfo("s1", "S1", "t1", "http://localhost:3000"));
        discovery.RegisterService(new ServiceInfo("s2", "S2", "t2", "http://localhost:3001"));

        // Act
        discovery.Clear();

        // Assert
        Assert.Equal(0, discovery.ServiceCount);
    }

    [Fact]
    public void ServiceInfo_ShouldInitializeWithDefaultValues()
    {
        // Arrange & Act
        var info = new ServiceInfo();

        // Assert
        Assert.Empty(info.Id);
        Assert.Empty(info.Name);
        Assert.Empty(info.Type);
        Assert.Empty(info.Endpoint);
        Assert.Equal("unknown", info.Health);
        Assert.NotNull(info.Metadata);
    }

    [Fact]
    public void ServiceInfo_ShouldInitializeWithConstructorValues()
    {
        // Arrange & Act
        var info = new ServiceInfo("id-001", "My Service", "api", "http://localhost:3000");

        // Assert
        Assert.Equal("id-001", info.Id);
        Assert.Equal("My Service", info.Name);
        Assert.Equal("api", info.Type);
        Assert.Equal("http://localhost:3000", info.Endpoint);
    }

    [Fact]
    public void ServiceInfo_ShouldFormatToString()
    {
        // Arrange
        var info = new ServiceInfo("id-001", "My Service", "api", "http://localhost:3000");

        // Act
        var str = info.ToString();

        // Assert
        Assert.Contains("id-001", str);
        Assert.Contains("My Service", str);
        Assert.Contains("api", str);
        Assert.Contains("http://localhost:3000", str);
    }
}

