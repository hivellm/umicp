using System.Text.Json.Serialization;

namespace Umicp.Core.Discovery;

/// <summary>
/// Service information for discovery
/// </summary>
public class ServiceInfo
{
    /// <summary>
    /// Service ID
    /// </summary>
    [JsonPropertyName("id")]
    public string Id { get; set; } = string.Empty;

    /// <summary>
    /// Service name
    /// </summary>
    [JsonPropertyName("name")]
    public string Name { get; set; } = string.Empty;

    /// <summary>
    /// Service type
    /// </summary>
    [JsonPropertyName("type")]
    public string Type { get; set; } = string.Empty;

    /// <summary>
    /// Service endpoint URL
    /// </summary>
    [JsonPropertyName("endpoint")]
    public string Endpoint { get; set; } = string.Empty;

    /// <summary>
    /// Service host
    /// </summary>
    [JsonPropertyName("host")]
    public string Host { get; set; } = string.Empty;

    /// <summary>
    /// Service port
    /// </summary>
    [JsonPropertyName("port")]
    public int Port { get; set; }

    /// <summary>
    /// Service metadata
    /// </summary>
    [JsonPropertyName("metadata")]
    public Dictionary<string, object> Metadata { get; set; } = new();

    /// <summary>
    /// Service health status
    /// </summary>
    [JsonPropertyName("health")]
    public string Health { get; set; } = "unknown";

    /// <summary>
    /// Last heartbeat timestamp
    /// </summary>
    [JsonPropertyName("last_heartbeat")]
    public DateTime LastHeartbeat { get; set; } = DateTime.UtcNow;

    /// <summary>
    /// Registration timestamp
    /// </summary>
    [JsonPropertyName("registered_at")]
    public DateTime RegisteredAt { get; set; } = DateTime.UtcNow;

    public ServiceInfo() { }

    public ServiceInfo(string id, string name, string type, string endpoint)
    {
        Id = id;
        Name = name;
        Type = type;
        Endpoint = endpoint;
    }

    public override string ToString()
    {
        return $"ServiceInfo(id={Id}, name={Name}, type={Type}, endpoint={Endpoint})";
    }
}

