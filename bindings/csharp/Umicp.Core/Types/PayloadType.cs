using System.Text.Json.Serialization;

namespace Umicp.Core.Types;

/// <summary>
/// Payload data types for UMICP messages
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum PayloadType
{
    /// <summary>
    /// Vector data type
    /// </summary>
    Vector,

    /// <summary>
    /// Text data type
    /// </summary>
    Text,

    /// <summary>
    /// Metadata data type
    /// </summary>
    Metadata,

    /// <summary>
    /// Binary data type
    /// </summary>
    Binary,

    /// <summary>
    /// JSON data type
    /// </summary>
    Json,

    /// <summary>
    /// Matrix data type
    /// </summary>
    Matrix
}

