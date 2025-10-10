using System.Text.Json.Serialization;

namespace Umicp.Core.Types;

/// <summary>
/// Message operation types for UMICP protocol
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum OperationType
{
    /// <summary>
    /// Control message type
    /// </summary>
    Control,

    /// <summary>
    /// Data transfer message type
    /// </summary>
    Data,

    /// <summary>
    /// Acknowledgment message type
    /// </summary>
    Ack,

    /// <summary>
    /// Error message type
    /// </summary>
    Error,

    /// <summary>
    /// Request message type
    /// </summary>
    Request,

    /// <summary>
    /// Response message type
    /// </summary>
    Response
}

