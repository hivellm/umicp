using System.Collections.Concurrent;
using Umicp.Core.Events;

namespace Umicp.Core.Discovery;

/// <summary>
/// Service discovery for UMICP
/// </summary>
public class ServiceDiscovery : IDisposable
{
    private readonly ConcurrentDictionary<string, ServiceInfo> _services = new();
    private readonly EventEmitter _events = new();
    private readonly TimeSpan _heartbeatInterval;
    private readonly TimeSpan _serviceTimeout;
    private Timer? _cleanupTimer;

    /// <summary>
    /// Event emitter
    /// </summary>
    public EventEmitter Events => _events;

    /// <summary>
    /// All registered services
    /// </summary>
    public IReadOnlyDictionary<string, ServiceInfo> Services => _services;

    /// <summary>
    /// Initialize service discovery
    /// </summary>
    public ServiceDiscovery(
        TimeSpan? heartbeatInterval = null,
        TimeSpan? serviceTimeout = null)
    {
        _heartbeatInterval = heartbeatInterval ?? TimeSpan.FromSeconds(30);
        _serviceTimeout = serviceTimeout ?? TimeSpan.FromMinutes(2);

        // Start cleanup timer
        _cleanupTimer = new Timer(
            CleanupStaleServices,
            null,
            _heartbeatInterval,
            _heartbeatInterval);
    }

    /// <summary>
    /// Register a service
    /// </summary>
    public void RegisterService(ServiceInfo service)
    {
        service.RegisteredAt = DateTime.UtcNow;
        service.LastHeartbeat = DateTime.UtcNow;
        service.Health = "healthy";

        if (_services.TryAdd(service.Id, service))
        {
            _events.Emit(EventType.Connect, new Dictionary<string, object>
            {
                ["service_id"] = service.Id,
                ["service"] = service
            });
        }
        else
        {
            _services[service.Id] = service;
        }
    }

    /// <summary>
    /// Unregister a service
    /// </summary>
    public bool UnregisterService(string serviceId)
    {
        if (_services.TryRemove(serviceId, out var service))
        {
            _events.Emit(EventType.Disconnect, new Dictionary<string, object>
            {
                ["service_id"] = serviceId,
                ["service"] = service
            });
            return true;
        }
        return false;
    }

    /// <summary>
    /// Get a service by ID
    /// </summary>
    public ServiceInfo? GetService(string serviceId)
    {
        _services.TryGetValue(serviceId, out var service);
        return service;
    }

    /// <summary>
    /// Find services by type
    /// </summary>
    public ServiceInfo[] FindByType(string type)
    {
        return _services.Values
            .Where(s => s.Type == type && s.Health == "healthy")
            .ToArray();
    }

    /// <summary>
    /// Find services by name
    /// </summary>
    public ServiceInfo[] FindByName(string name)
    {
        return _services.Values
            .Where(s => s.Name == name && s.Health == "healthy")
            .ToArray();
    }

    /// <summary>
    /// Find all healthy services
    /// </summary>
    public ServiceInfo[] FindHealthyServices()
    {
        return _services.Values
            .Where(s => s.Health == "healthy")
            .ToArray();
    }

    /// <summary>
    /// Update service heartbeat
    /// </summary>
    public void Heartbeat(string serviceId)
    {
        if (_services.TryGetValue(serviceId, out var service))
        {
            service.LastHeartbeat = DateTime.UtcNow;
            service.Health = "healthy";
        }
    }

    /// <summary>
    /// Update service metadata
    /// </summary>
    public void UpdateMetadata(string serviceId, Dictionary<string, object> metadata)
    {
        if (_services.TryGetValue(serviceId, out var service))
        {
            foreach (var kvp in metadata)
            {
                service.Metadata[kvp.Key] = kvp.Value;
            }
        }
    }

    /// <summary>
    /// Get service count
    /// </summary>
    public int ServiceCount => _services.Count;

    /// <summary>
    /// Get healthy service count
    /// </summary>
    public int HealthyServiceCount => _services.Values.Count(s => s.Health == "healthy");

    /// <summary>
    /// Cleanup stale services
    /// </summary>
    private void CleanupStaleServices(object? state)
    {
        var now = DateTime.UtcNow;
        var staleServices = _services.Values
            .Where(s => now - s.LastHeartbeat > _serviceTimeout)
            .ToList();

        foreach (var service in staleServices)
        {
            service.Health = "unhealthy";

            _events.Emit(EventType.Timeout, new Dictionary<string, object>
            {
                ["service_id"] = service.Id,
                ["service"] = service,
                ["last_heartbeat"] = service.LastHeartbeat
            });

            // Remove after marking unhealthy
            _services.TryRemove(service.Id, out _);
        }
    }

    /// <summary>
    /// Clear all services
    /// </summary>
    public void Clear()
    {
        _services.Clear();
    }

    public void Dispose()
    {
        _cleanupTimer?.Dispose();
        _cleanupTimer = null;
    }
}

