using System.Text.Json;
using System.Text.Json.Serialization;

namespace Umicp.Core.Types;

/// <summary>
/// Payload metadata hints for UMICP messages
/// </summary>
public class PayloadHint
{
    /// <summary>
    /// Type of payload
    /// </summary>
    [JsonPropertyName("type")]
    public PayloadType Type { get; set; }

    /// <summary>
    /// Size of payload in bytes
    /// </summary>
    [JsonPropertyName("size")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? Size { get; set; }

    /// <summary>
    /// Encoding type of payload
    /// </summary>
    [JsonPropertyName("encoding")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public EncodingType? Encoding { get; set; }

    /// <summary>
    /// Count of items in payload (for arrays/vectors)
    /// </summary>
    [JsonPropertyName("count")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public int? Count { get; set; }

    /// <summary>
    /// Compression type used (if any)
    /// </summary>
    [JsonPropertyName("compression")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Compression { get; set; }

    /// <summary>
    /// Convert to dictionary representation
    /// </summary>
    public Dictionary<string, object> ToDictionary()
    {
        var result = new Dictionary<string, object>
        {
            ["type"] = Type.ToString().ToLower()
        };

        if (Size.HasValue)
            result["size"] = Size.Value;

        if (Encoding.HasValue)
            result["encoding"] = Encoding.Value.ToString().ToLower();

        if (Count.HasValue)
            result["count"] = Count.Value;

        if (!string.IsNullOrEmpty(Compression))
            result["compression"] = Compression;

        return result;
    }

    /// <summary>
    /// Create from dictionary representation
    /// </summary>
    public static PayloadHint FromDictionary(Dictionary<string, object> data)
    {
        var hint = new PayloadHint
        {
            Type = Enum.Parse<PayloadType>(data["type"].ToString()!, ignoreCase: true)
        };

        if (data.TryGetValue("size", out var size))
            hint.Size = Convert.ToInt32(size);

        if (data.TryGetValue("encoding", out var encoding))
            hint.Encoding = Enum.Parse<EncodingType>(encoding.ToString()!, ignoreCase: true);

        if (data.TryGetValue("count", out var count))
            hint.Count = Convert.ToInt32(count);

        if (data.TryGetValue("compression", out var compression))
            hint.Compression = compression.ToString();

        return hint;
    }
}

