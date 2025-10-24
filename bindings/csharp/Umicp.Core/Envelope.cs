using System.Security.Cryptography;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using Umicp.SDK.Types;
using Umicp.SDK.Exceptions;

namespace Umicp.SDK;

/// <summary>
/// UMICP message envelope
/// </summary>
public class Envelope
{
    /// <summary>
    /// Sender identifier
    /// </summary>
    [JsonPropertyName("from")]
    public string FromId { get; set; }

    /// <summary>
    /// Recipient identifier
    /// </summary>
    [JsonPropertyName("to")]
    public string ToId { get; set; }

    /// <summary>
    /// Operation type
    /// </summary>
    [JsonPropertyName("operation")]
    public OperationType Operation { get; set; }

    /// <summary>
    /// Unique message identifier
    /// </summary>
    [JsonPropertyName("message_id")]
    public string MessageId { get; set; }

    /// <summary>
    /// Correlation ID for request/response pairs
    /// </summary>
    [JsonPropertyName("correlation_id")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? CorrelationId { get; set; }

    /// <summary>
    /// Message timestamp
    /// </summary>
    [JsonPropertyName("timestamp")]
    public DateTime Timestamp { get; set; }

    /// <summary>
    /// Metadata capabilities
    /// </summary>
    [JsonPropertyName("capabilities")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public Dictionary<string, object>? Capabilities { get; set; }

    /// <summary>
    /// List of payload hints
    /// </summary>
    [JsonPropertyName("payload_hints")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public List<PayloadHint>? PayloadHints { get; set; }

    /// <summary>
    /// List of payload references
    /// </summary>
    [JsonPropertyName("payload_refs")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public List<string>? PayloadRefs { get; set; }

    /// <summary>
    /// Envelope hash
    /// </summary>
    [JsonPropertyName("hash")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Hash { get; set; }

    /// <summary>
    /// Default constructor for deserialization
    /// </summary>
    public Envelope()
    {
        FromId = string.Empty;
        ToId = string.Empty;
        Operation = OperationType.Data;
        MessageId = Guid.NewGuid().ToString();
        Timestamp = DateTime.UtcNow;
    }

    /// <summary>
    /// Initialize a new envelope
    /// </summary>
    /// <param name="fromId">Sender identifier</param>
    /// <param name="toId">Recipient identifier</param>
    /// <param name="operation">Operation type</param>
    /// <param name="messageId">Unique message identifier (auto-generated if null)</param>
    /// <param name="correlationId">Optional correlation ID for request/response pairs</param>
    /// <param name="timestamp">Message timestamp (UTC now if null)</param>
    /// <param name="capabilities">Optional metadata capabilities</param>
    /// <param name="payloadHints">Optional list of payload hints</param>
    /// <param name="payloadRefs">Optional list of payload references</param>
    /// <param name="hash">Optional envelope hash</param>
    /// <exception cref="ValidationException">Thrown when required fields are invalid</exception>
    public Envelope(
        string fromId,
        string toId,
        OperationType operation,
        string? messageId = null,
        string? correlationId = null,
        DateTime? timestamp = null,
        Dictionary<string, object>? capabilities = null,
        List<PayloadHint>? payloadHints = null,
        List<string>? payloadRefs = null,
        string? hash = null)
    {
        FromId = fromId;
        ToId = toId;
        Operation = operation;
        MessageId = messageId ?? Guid.NewGuid().ToString();
        CorrelationId = correlationId;
        Timestamp = timestamp ?? DateTime.UtcNow;
        Capabilities = capabilities;
        PayloadHints = payloadHints;
        PayloadRefs = payloadRefs;
        Hash = hash;

        Validate();
    }

    /// <summary>
    /// Validate envelope fields
    /// </summary>
    /// <exception cref="ValidationException">Thrown when required fields are missing or invalid</exception>
    public void Validate()
    {
        if (string.IsNullOrWhiteSpace(FromId))
            throw new ValidationException("from_id is required");

        if (string.IsNullOrWhiteSpace(ToId))
            throw new ValidationException("to_id is required");

        if (string.IsNullOrWhiteSpace(MessageId))
            throw new ValidationException("message_id is required");
    }

    /// <summary>
    /// Compute SHA-256 hash of envelope
    /// </summary>
    /// <returns>Lowercase hexadecimal SHA-256 hash of the envelope</returns>
    public string ComputeHash()
    {
        var dict = ToDictionary();
        dict.Remove("hash");

        var json = JsonSerializer.Serialize(dict, new JsonSerializerOptions
        {
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
            WriteIndented = false,
            DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull
        });

        var bytes = Encoding.UTF8.GetBytes(json);
        var hashBytes = SHA256.HashData(bytes);
        return Convert.ToHexString(hashBytes).ToLower();
    }

    /// <summary>
    /// Convert to dictionary representation
    /// </summary>
    /// <returns>Dictionary containing envelope data with camelCase keys</returns>
    public Dictionary<string, object> ToDictionary()
    {
        var result = new Dictionary<string, object>
        {
            ["from"] = FromId,
            ["to"] = ToId,
            ["operation"] = Operation.ToString().ToLower(),
            ["message_id"] = MessageId,
            ["timestamp"] = Timestamp.ToString("yyyy-MM-ddTHH:mm:ss.fffZ")
        };

        if (CorrelationId != null)
            result["correlation_id"] = CorrelationId;

        if (Capabilities != null)
            result["capabilities"] = Capabilities;

        if (PayloadHints != null && PayloadHints.Count > 0)
            result["payload_hints"] = PayloadHints.Select(h => h.ToDictionary()).ToList();

        if (PayloadRefs != null && PayloadRefs.Count > 0)
            result["payload_refs"] = PayloadRefs;

        if (Hash != null)
            result["hash"] = Hash;

        return result;
    }

    /// <summary>
    /// Serialize to JSON string
    /// </summary>
    /// <returns>JSON representation of the envelope</returns>
    /// <exception cref="SerializationException">Thrown when serialization fails</exception>
    public string ToJson()
    {
        try
        {
            return JsonSerializer.Serialize(this, new JsonSerializerOptions
            {
                PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
                WriteIndented = false,
                DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
                Converters = { new JsonStringEnumConverter(JsonNamingPolicy.CamelCase) }
            });
        }
        catch (Exception ex)
        {
            throw new SerializationException($"Failed to serialize envelope: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Create envelope from dictionary
    /// </summary>
    /// <param name="data">Dictionary containing envelope data</param>
    /// <returns>New envelope instance</returns>
    /// <exception cref="DeserializationException">Thrown when deserialization fails</exception>
    public static Envelope FromDictionary(Dictionary<string, object> data)
    {
        try
        {
            var timestampStr = data["timestamp"].ToString()!;
            var timestamp = DateTime.Parse(timestampStr.Replace("Z", ""));

            List<PayloadHint>? payloadHints = null;
            if (data.TryGetValue("payload_hints", out var hintsObj) && hintsObj is JsonElement hintsElement)
            {
                payloadHints = JsonSerializer.Deserialize<List<PayloadHint>>(hintsElement.GetRawText());
            }

            return new Envelope(
                fromId: data["from"].ToString()!,
                toId: data["to"].ToString()!,
                operation: Enum.Parse<OperationType>(data["operation"].ToString()!, ignoreCase: true),
                messageId: data.TryGetValue("message_id", out var msgId) ? msgId.ToString() : null,
                correlationId: data.TryGetValue("correlation_id", out var corrId) ? corrId.ToString() : null,
                timestamp: timestamp,
                capabilities: data.TryGetValue("capabilities", out var caps) ? caps as Dictionary<string, object> : null,
                payloadHints: payloadHints,
                payloadRefs: data.TryGetValue("payload_refs", out var refs) ? refs as List<string> : null,
                hash: data.TryGetValue("hash", out var hash) ? hash.ToString() : null
            );
        }
        catch (Exception ex)
        {
            throw new SerializationException($"Failed to deserialize envelope: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Create envelope from JSON string
    /// </summary>
    public static Envelope FromJson(string json)
    {
        try
        {
            var options = new JsonSerializerOptions
            {
                PropertyNameCaseInsensitive = true,
                Converters = { new JsonStringEnumConverter(JsonNamingPolicy.CamelCase) }
            };

            return JsonSerializer.Deserialize<Envelope>(json, options)
                ?? throw new SerializationException("Deserialized envelope is null");
        }
        catch (JsonException ex)
        {
            throw new SerializationException($"Invalid JSON: {ex.Message}", ex);
        }
    }

    public override string ToString()
    {
        return $"Envelope(from={FromId}, to={ToId}, operation={Operation}, message_id={MessageId})";
    }
}

/// <summary>
/// Builder for creating envelopes
/// </summary>
public class EnvelopeBuilder
{
    private string? _fromId;
    private string? _toId;
    private OperationType? _operation;
    private string? _messageId;
    private string? _correlationId;
    private DateTime? _timestamp;
    private Dictionary<string, object> _capabilities = new();
    private List<PayloadHint> _payloadHints = new();
    private List<string> _payloadRefs = new();

    /// <summary>
    /// Set sender ID
    /// </summary>
    public EnvelopeBuilder FromId(string fromId)
    {
        _fromId = fromId;
        return this;
    }

    /// <summary>
    /// Set recipient ID
    /// </summary>
    public EnvelopeBuilder ToId(string toId)
    {
        _toId = toId;
        return this;
    }

    /// <summary>
    /// Set operation type
    /// </summary>
    public EnvelopeBuilder Operation(OperationType operation)
    {
        _operation = operation;
        return this;
    }

    /// <summary>
    /// Set message ID
    /// </summary>
    public EnvelopeBuilder MessageId(string messageId)
    {
        _messageId = messageId;
        return this;
    }

    /// <summary>
    /// Set correlation ID
    /// </summary>
    public EnvelopeBuilder CorrelationId(string correlationId)
    {
        _correlationId = correlationId;
        return this;
    }

    /// <summary>
    /// Set timestamp
    /// </summary>
    public EnvelopeBuilder Timestamp(DateTime timestamp)
    {
        _timestamp = timestamp;
        return this;
    }

    /// <summary>
    /// Add capability
    /// </summary>
    public EnvelopeBuilder Capability(string key, object value)
    {
        _capabilities[key] = value;
        return this;
    }

    /// <summary>
    /// Add payload hint
    /// </summary>
    public EnvelopeBuilder PayloadHint(PayloadHint hint)
    {
        _payloadHints.Add(hint);
        return this;
    }

    /// <summary>
    /// Add payload reference
    /// </summary>
    public EnvelopeBuilder PayloadRef(string reference)
    {
        _payloadRefs.Add(reference);
        return this;
    }

    /// <summary>
    /// Build the envelope
    /// </summary>
    public Envelope Build()
    {
        if (string.IsNullOrWhiteSpace(_fromId))
            throw new ValidationException("from_id is required");

        if (string.IsNullOrWhiteSpace(_toId))
            throw new ValidationException("to_id is required");

        if (!_operation.HasValue)
            throw new ValidationException("operation is required");

        var envelope = new Envelope(
            fromId: _fromId,
            toId: _toId,
            operation: _operation.Value,
            messageId: _messageId,
            correlationId: _correlationId,
            timestamp: _timestamp,
            capabilities: _capabilities.Count > 0 ? _capabilities : null,
            payloadHints: _payloadHints.Count > 0 ? _payloadHints : null,
            payloadRefs: _payloadRefs.Count > 0 ? _payloadRefs : null
        );

        // Compute hash
        envelope.Hash = envelope.ComputeHash();

        return envelope;
    }
}

