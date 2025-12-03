using System.Text.Json.Serialization;

namespace Umicp.SDK.Types;

/// <summary>
/// Connection state for UMICP transports
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum ConnectionState
{
    /// <summary>
    /// Not connected
    /// </summary>
    Disconnected,

    /// <summary>
    /// Attempting to connect
    /// </summary>
    Connecting,

    /// <summary>
    /// Successfully connected
    /// </summary>
    Connected,

    /// <summary>
    /// Attempting to reconnect
    /// </summary>
    Reconnecting,

    /// <summary>
    /// Attempting to disconnect
    /// </summary>
    Disconnecting,

    /// <summary>
    /// Error state
    /// </summary>
    Error
}

