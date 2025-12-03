using System.Text.Json.Serialization;

namespace Umicp.SDK.Types;

/// <summary>
/// Data encoding types for UMICP payloads
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum EncodingType
{
    /// <summary>
    /// 32-bit floating point encoding
    /// </summary>
    Float32,

    /// <summary>
    /// 64-bit floating point encoding
    /// </summary>
    Float64,

    /// <summary>
    /// 32-bit integer encoding
    /// </summary>
    Int32,

    /// <summary>
    /// 64-bit integer encoding
    /// </summary>
    Int64,

    /// <summary>
    /// UTF-8 text encoding
    /// </summary>
    Utf8,

    /// <summary>
    /// Base64 encoding
    /// </summary>
    Base64,

    /// <summary>
    /// Hexadecimal encoding
    /// </summary>
    Hex
}

