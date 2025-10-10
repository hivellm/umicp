using System.Text.Json.Serialization;

namespace Umicp.Core.Types;

/// <summary>
/// Transport statistics for UMICP connections
/// </summary>
public class TransportStats
{
    /// <summary>
    /// Number of messages sent
    /// </summary>
    [JsonPropertyName("messages_sent")]
    public long MessagesSent { get; set; }

    /// <summary>
    /// Number of messages received
    /// </summary>
    [JsonPropertyName("messages_received")]
    public long MessagesReceived { get; set; }

    /// <summary>
    /// Number of bytes sent
    /// </summary>
    [JsonPropertyName("bytes_sent")]
    public long BytesSent { get; set; }

    /// <summary>
    /// Number of bytes received
    /// </summary>
    [JsonPropertyName("bytes_received")]
    public long BytesReceived { get; set; }

    /// <summary>
    /// Number of errors encountered
    /// </summary>
    [JsonPropertyName("errors")]
    public long Errors { get; set; }

    /// <summary>
    /// Number of reconnection attempts
    /// </summary>
    [JsonPropertyName("reconnections")]
    public long Reconnections { get; set; }

    /// <summary>
    /// Timestamp when connected
    /// </summary>
    [JsonPropertyName("connected_at")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public DateTime? ConnectedAt { get; set; }

    /// <summary>
    /// Timestamp of last activity
    /// </summary>
    [JsonPropertyName("last_activity")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public DateTime? LastActivity { get; set; }

    /// <summary>
    /// Convert to dictionary representation
    /// </summary>
    public Dictionary<string, object?> ToDictionary()
    {
        return new Dictionary<string, object?>
        {
            ["messages_sent"] = MessagesSent,
            ["messages_received"] = MessagesReceived,
            ["bytes_sent"] = BytesSent,
            ["bytes_received"] = BytesReceived,
            ["errors"] = Errors,
            ["reconnections"] = Reconnections,
            ["connected_at"] = ConnectedAt?.ToString("O"),
            ["last_activity"] = LastActivity?.ToString("O")
        };
    }
}

