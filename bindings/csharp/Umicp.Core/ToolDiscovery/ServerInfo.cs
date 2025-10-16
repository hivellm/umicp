using System.Text.Json;
using System.Text.Json.Serialization;

namespace Umicp.Core.ToolDiscovery;

/// <summary>
/// Server information for discovery
/// </summary>
public class ServerInfo
{
    /// <summary>
    /// Server name/identifier
    /// </summary>
    [JsonPropertyName("server")]
    public string Server { get; set; } = string.Empty;

    /// <summary>
    /// Server version
    /// </summary>
    [JsonPropertyName("version")]
    public string Version { get; set; } = string.Empty;

    /// <summary>
    /// Protocol version (e.g., "UMICP/0.2")
    /// </summary>
    [JsonPropertyName("protocol")]
    public string Protocol { get; set; } = string.Empty;

    /// <summary>
    /// List of supported features
    /// </summary>
    [JsonPropertyName("features")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public List<string>? Features { get; set; }

    /// <summary>
    /// Number of available operations
    /// </summary>
    [JsonPropertyName("operations_count")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? OperationsCount { get; set; }

    /// <summary>
    /// Whether server is MCP-compatible
    /// </summary>
    [JsonPropertyName("mcp_compatible")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public bool? McpCompatible { get; set; }

    /// <summary>
    /// Additional metadata
    /// </summary>
    [JsonPropertyName("metadata")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public JsonElement? Metadata { get; set; }

    /// <summary>
    /// Create new server info
    /// </summary>
    public ServerInfo(string server, string version, string protocol)
    {
        Server = server;
        Version = version;
        Protocol = protocol;
    }

    /// <summary>
    /// Set features (builder pattern)
    /// </summary>
    public ServerInfo WithFeatures(List<string> features)
    {
        Features = features;
        return this;
    }

    /// <summary>
    /// Set operations count (builder pattern)
    /// </summary>
    public ServerInfo WithOperationsCount(int count)
    {
        OperationsCount = count;
        return this;
    }

    /// <summary>
    /// Set MCP compatibility (builder pattern)
    /// </summary>
    public ServerInfo WithMcpCompatible(bool compatible)
    {
        McpCompatible = compatible;
        return this;
    }

    /// <summary>
    /// Set metadata (builder pattern)
    /// </summary>
    public ServerInfo WithMetadata(JsonElement metadata)
    {
        Metadata = metadata;
        return this;
    }
}

