using System.Text.Json.Serialization;

namespace Umicp.SDK.Compression;

/// <summary>
/// Compression algorithms supported by UMICP
/// </summary>
[JsonConverter(typeof(JsonStringEnumConverter))]
public enum CompressionType
{
    /// <summary>
    /// No compression
    /// </summary>
    None,

    /// <summary>
    /// GZIP compression
    /// </summary>
    Gzip,

    /// <summary>
    /// Deflate compression
    /// </summary>
    Deflate
}

