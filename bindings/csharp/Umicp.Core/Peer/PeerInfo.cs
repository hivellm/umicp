using System.Text.Json.Serialization;

namespace Umicp.SDK.Peer;

/// <summary>
/// Information about a peer in the UMICP network
/// </summary>
public class PeerInfo
{
    /// <summary>
    /// Unique peer identifier
    /// </summary>
    [JsonPropertyName("id")]
    public string Id { get; set; } = string.Empty;

    /// <summary>
    /// Peer address (URL or endpoint)
    /// </summary>
    [JsonPropertyName("address")]
    public string? Address { get; set; }

    /// <summary>
    /// Peer capabilities
    /// </summary>
    [JsonPropertyName("capabilities")]
    public Dictionary<string, object> Capabilities { get; set; } = new();

    /// <summary>
    /// Peer metadata
    /// </summary>
    [JsonPropertyName("metadata")]
    public Dictionary<string, object> Metadata { get; set; } = new();

    /// <summary>
    /// When the peer connected
    /// </summary>
    [JsonPropertyName("connected_at")]
    public DateTime ConnectedAt { get; set; } = DateTime.UtcNow;

    /// <summary>
    /// Last activity timestamp
    /// </summary>
    [JsonPropertyName("last_seen")]
    public DateTime LastSeen { get; set; } = DateTime.UtcNow;

    /// <summary>
    /// Protocol version
    /// </summary>
    [JsonPropertyName("protocol_version")]
    public string ProtocolVersion { get; set; } = "1.0.0";

    public PeerInfo() { }

    public PeerInfo(string id, string? address = null)
    {
        Id = id;
        Address = address;
    }

    public override string ToString()
    {
        return $"PeerInfo(id={Id}, address={Address})";
    }
}

