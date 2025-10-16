using System.Text.Json;

namespace Umicp.Core.ToolDiscovery;

/// <summary>
/// Interface for services that support tool discovery
/// </summary>
/// <remarks>
/// Implement this interface to provide automatic tool discovery
/// and introspection for your UMICP service, compatible with MCP.
/// </remarks>
/// <example>
/// <code>
/// public class MyService : IDiscoverableService
/// {
///     public List&lt;OperationSchema&gt; ListOperations()
///     {
///         return new List&lt;OperationSchema&gt;
///         {
///             new OperationSchema("search", JsonDocument.Parse(@"{
///                 ""type"": ""object"",
///                 ""properties"": {
///                     ""query"": {""type"": ""string""}
///                 }
///             }").RootElement)
///             .WithTitle("Search")
///             .WithAnnotations(JsonDocument.Parse(@"{""read_only"": true}").RootElement)
///         };
///     }
///
///     public ServerInfo GetServerInfo()
///     {
///         return new ServerInfo("my-service", "1.0.0", "UMICP/0.2")
///             .WithMcpCompatible(true);
///     }
/// }
/// </code>
/// </example>
public interface IDiscoverableService
{
    /// <summary>
    /// List all available operations with their schemas
    /// </summary>
    /// <returns>List of operation schemas</returns>
    List<OperationSchema> ListOperations();

    /// <summary>
    /// Get schema for a specific operation by name
    /// </summary>
    /// <param name="name">Operation name to look up</param>
    /// <returns>Operation schema if found, null otherwise</returns>
    OperationSchema? GetSchema(string name)
    {
        return ListOperations().FirstOrDefault(op => op.Name == name);
    }

    /// <summary>
    /// Get server information and metadata
    /// </summary>
    /// <returns>Server information</returns>
    ServerInfo GetServerInfo();
}

/// <summary>
/// Helper methods for generating discovery responses
/// </summary>
public static class DiscoveryHelpers
{
    /// <summary>
    /// Generate JSON response for _list_operations
    /// </summary>
    public static JsonDocument GenerateOperationsResponse(IDiscoverableService service)
    {
        var operations = service.ListOperations();
        var info = service.GetServerInfo();

        var response = new Dictionary<string, object>
        {
            ["operations"] = operations,
            ["count"] = operations.Count,
            ["protocol"] = info.Protocol,
            ["mcp_compatible"] = info.McpCompatible ?? false
        };

        return JsonSerializer.SerializeToDocument(response);
    }

    /// <summary>
    /// Generate JSON response for _get_schema
    /// </summary>
    public static JsonDocument GenerateSchemaResponse(IDiscoverableService service, string operationName)
    {
        var schema = service.GetSchema(operationName);

        if (schema != null)
        {
            return JsonSerializer.SerializeToDocument(schema);
        }
        else
        {
            var error = new Dictionary<string, object>
            {
                ["error"] = "Operation not found",
                ["operation"] = operationName
            };
            return JsonSerializer.SerializeToDocument(error);
        }
    }

    /// <summary>
    /// Generate JSON response for _server_info
    /// </summary>
    public static JsonDocument GenerateServerInfoResponse(IDiscoverableService service)
    {
        return JsonSerializer.SerializeToDocument(service.GetServerInfo());
    }
}

