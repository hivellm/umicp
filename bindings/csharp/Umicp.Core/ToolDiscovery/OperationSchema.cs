using System.Text.Json;
using System.Text.Json.Serialization;

namespace Umicp.SDK.ToolDiscovery;

/// <summary>
/// Operation schema compatible with MCP JSON Schema
/// </summary>
public class OperationSchema
{
    /// <summary>
    /// Operation name
    /// </summary>
    [JsonPropertyName("name")]
    public string Name { get; set; } = string.Empty;

    /// <summary>
    /// Display title
    /// </summary>
    [JsonPropertyName("title")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Title { get; set; }

    /// <summary>
    /// Description of what the operation does
    /// </summary>
    [JsonPropertyName("description")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Description { get; set; }

    /// <summary>
    /// JSON Schema for input parameters
    /// </summary>
    [JsonPropertyName("input_schema")]
    public JsonElement InputSchema { get; set; }

    /// <summary>
    /// Optional output schema
    /// </summary>
    [JsonPropertyName("output_schema")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public JsonElement? OutputSchema { get; set; }

    /// <summary>
    /// Annotations (read_only, idempotent, destructive, etc.)
    /// </summary>
    [JsonPropertyName("annotations")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public JsonElement? Annotations { get; set; }

    /// <summary>
    /// Create a new operation schema
    /// </summary>
    public OperationSchema(string name, JsonElement inputSchema)
    {
        Name = name;
        InputSchema = inputSchema;
    }

    /// <summary>
    /// Set title (builder pattern)
    /// </summary>
    public OperationSchema WithTitle(string title)
    {
        Title = title;
        return this;
    }

    /// <summary>
    /// Set description (builder pattern)
    /// </summary>
    public OperationSchema WithDescription(string description)
    {
        Description = description;
        return this;
    }

    /// <summary>
    /// Set output schema (builder pattern)
    /// </summary>
    public OperationSchema WithOutputSchema(JsonElement schema)
    {
        OutputSchema = schema;
        return this;
    }

    /// <summary>
    /// Set annotations (builder pattern)
    /// </summary>
    public OperationSchema WithAnnotations(JsonElement annotations)
    {
        Annotations = annotations;
        return this;
    }
}

