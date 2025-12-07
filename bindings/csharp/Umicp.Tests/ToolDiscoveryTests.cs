using System.Text.Json;
using Xunit;
using Umicp.SDK.ToolDiscovery;

namespace Umicp.Tests;

// Test service implementation
public class TestService : IDiscoverableService
{
    public List<OperationSchema> ListOperations()
    {
        return new List<OperationSchema>
        {
            new OperationSchema(
                "search_vectors",
                JsonDocument.Parse(@"{
                    ""type"": ""object"",
                    ""properties"": {
                        ""collection"": {""type"": ""string""},
                        ""query"": {""type"": ""string""},
                        ""limit"": {""type"": ""integer"", ""default"": 10}
                    },
                    ""required"": [""collection"", ""query""]
                }").RootElement
            )
            .WithTitle("Search Vectors")
            .WithDescription("Search for semantically similar content")
            .WithAnnotations(JsonDocument.Parse(@"{""read_only"": true}").RootElement),

            new OperationSchema(
                "create_collection",
                JsonDocument.Parse(@"{
                    ""type"": ""object"",
                    ""properties"": {
                        ""name"": {""type"": ""string""},
                        ""dimension"": {""type"": ""integer""}
                    },
                    ""required"": [""name"", ""dimension""]
                }").RootElement
            )
            .WithTitle("Create Collection")
        };
    }

    public ServerInfo GetServerInfo()
    {
        return new ServerInfo("test-service", "1.0.0", "UMICP/0.2")
            .WithFeatures(new List<string> { "discovery", "search" })
            .WithOperationsCount(2)
            .WithMcpCompatible(true);
    }
}

public class ToolDiscoveryTests
{
    [Fact]
    public void OperationSchema_Creation_SetsNameAndSchema()
    {
        var schema = new OperationSchema(
            "test_op",
            JsonDocument.Parse(@"{""type"": ""object""}").RootElement
        );

        Assert.Equal("test_op", schema.Name);
        Assert.Equal("object", schema.InputSchema.GetProperty("type").GetString());
    }

    [Fact]
    public void OperationSchema_BuilderPattern_SetsAllFields()
    {
        var schema = new OperationSchema("test", JsonDocument.Parse(@"{""type"": ""object""}").RootElement)
            .WithTitle("Test Operation")
            .WithDescription("A test")
            .WithAnnotations(JsonDocument.Parse(@"{""read_only"": true}").RootElement);

        Assert.Equal("Test Operation", schema.Title);
        Assert.Equal("A test", schema.Description);
        Assert.NotNull(schema.Annotations);
    }

    [Fact]
    public void ServerInfo_Creation_SetsRequiredFields()
    {
        var info = new ServerInfo("my-service", "1.0.0", "UMICP/0.2");

        Assert.Equal("my-service", info.Server);
        Assert.Equal("1.0.0", info.Version);
        Assert.Equal("UMICP/0.2", info.Protocol);
    }

    [Fact]
    public void ServerInfo_BuilderPattern_SetsOptionalFields()
    {
        var info = new ServerInfo("test", "1.0", "UMICP/0.2")
            .WithFeatures(new List<string> { "discovery" })
            .WithOperationsCount(5)
            .WithMcpCompatible(true);

        Assert.NotNull(info.Features);
        Assert.Single(info.Features);
        Assert.Equal(5, info.OperationsCount);
        Assert.True(info.McpCompatible);
    }

    [Fact]
    public void DiscoverableService_ListOperations_ReturnsAll()
    {
        var service = new TestService();
        var operations = service.ListOperations();

        Assert.Equal(2, operations.Count);
        Assert.Equal("search_vectors", operations[0].Name);
        Assert.Equal("create_collection", operations[1].Name);
    }

    [Fact]
    public void DiscoverableService_GetSchema_FindsOperation()
    {
        IDiscoverableService service = new TestService();
        var schema = service.GetSchema("search_vectors");

        Assert.NotNull(schema);
        Assert.Equal("search_vectors", schema.Name);
        Assert.Equal("Search Vectors", schema.Title);
    }

    [Fact]
    public void DiscoverableService_GetSchema_ReturnsNullWhenNotFound()
    {
        IDiscoverableService service = new TestService();
        var schema = service.GetSchema("non_existent");

        Assert.Null(schema);
    }

    [Fact]
    public void DiscoverableService_GetServerInfo_ReturnsInfo()
    {
        var service = new TestService();
        var info = service.GetServerInfo();

        Assert.Equal("test-service", info.Server);
        Assert.Equal("1.0.0", info.Version);
        Assert.Equal("UMICP/0.2", info.Protocol);
        Assert.NotNull(info.Features);
        Assert.Contains("discovery", info.Features);
    }

    [Fact]
    public void DiscoveryHelpers_GenerateOperationsResponse_ContainsAllOperations()
    {
        var service = new TestService();
        var response = DiscoveryHelpers.GenerateOperationsResponse(service);

        var root = response.RootElement;
        Assert.True(root.TryGetProperty("operations", out var ops));
        Assert.Equal(JsonValueKind.Array, ops.ValueKind);
        Assert.Equal(2, ops.GetArrayLength());

        Assert.True(root.TryGetProperty("count", out var count));
        Assert.Equal(2, count.GetInt32());
    }

    [Fact]
    public void DiscoveryHelpers_GenerateSchemaResponse_Found()
    {
        var service = new TestService();
        var response = DiscoveryHelpers.GenerateSchemaResponse(service, "search_vectors");

        var root = response.RootElement;
        Assert.True(root.TryGetProperty("name", out var name));
        Assert.Equal("search_vectors", name.GetString());
    }

    [Fact]
    public void DiscoveryHelpers_GenerateSchemaResponse_NotFound()
    {
        var service = new TestService();
        var response = DiscoveryHelpers.GenerateSchemaResponse(service, "invalid");

        var root = response.RootElement;
        Assert.True(root.TryGetProperty("error", out var error));
        Assert.Equal("Operation not found", error.GetString());
    }

    [Fact]
    public void DiscoveryHelpers_GenerateServerInfoResponse_ContainsAllFields()
    {
        var service = new TestService();
        var response = DiscoveryHelpers.GenerateServerInfoResponse(service);

        var root = response.RootElement;
        Assert.True(root.TryGetProperty("server", out var server));
        Assert.Equal("test-service", server.GetString());

        Assert.True(root.TryGetProperty("version", out var version));
        Assert.Equal("1.0.0", version.GetString());
    }

    [Fact]
    public void OperationSchema_Serialization_ProducesValidJson()
    {
        var schema = new OperationSchema("test", JsonDocument.Parse(@"{""type"": ""object""}").RootElement)
            .WithTitle("Test");

        var json = JsonSerializer.Serialize(schema);
        Assert.Contains("test", json);
        Assert.Contains("Test", json);
    }

    [Fact]
    public void ServerInfo_Serialization_ProducesValidJson()
    {
        var info = new ServerInfo("service", "1.0", "UMICP/0.2")
            .WithMcpCompatible(true);

        var json = JsonSerializer.Serialize(info);
        Assert.Contains("service", json);
        Assert.Contains("1.0", json);
        Assert.Contains("true", json);
    }
}

