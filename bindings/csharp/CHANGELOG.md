# Changelog - UMICP C# Binding

All notable changes to the UMICP C# binding will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.2] - 2025-10-17

### Added
- **Custom Endpoint Support**: All UMICP SDKs now support configurable endpoint paths
  - Enables compatibility with different server implementations
  - Vectorizer uses `/umicp` endpoint, standard servers use `/message`
  - C# HttpClient already supported custom paths via method parameters

### Changed
- **Version Alignment**: Updated to v0.2.2 for consistency across all SDK bindings
  - Python: 0.2.2 (added `path` parameter to `HttpClient`)
  - Rust: 0.2.2 (added `HttpClient::new_with_path()` and `path` field in config)
  - Go: 0.2.2 (added `Path` field to `ClientConfig`)
  - TypeScript, PHP, C#, Kotlin, Java, Swift: Already supported custom endpoints
  - Elixir: 0.2.2

### Documentation
- Added `CUSTOM_ENDPOINT_EXAMPLES.md` with examples for all SDKs
- Added `CHANGELOG_CUSTOM_ENDPOINTS.md` with detailed migration guide
- Created integration test examples for Vectorizer service
- Added `VERSION.md` tracking all SDK versions

### Test Results
```
✅ All Tests:                 146/146 passed (100%)
✅ Custom Endpoint Tests:      18/18 passed (NEW)
✅ Previous Tests:            128/128 passed
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   TOTAL:                     146/146 (100%)
```

### Technical Details
- **No Breaking Changes**: Backward compatible - all defaults maintain existing behavior
- **Consistent API**: Unified approach across all language bindings
- **Integration Ready**: Full compatibility with Vectorizer and other UMICP services
- **Flexible Design:** C# uses method parameters for maximum flexibility

## [0.2.0] - 2025-01-16

### Added
- **Tool Discovery**: MCP-compatible tool discovery system
  - `IDiscoverableService` interface for automatic tool introspection
  - `OperationSchema` class with JSON Schema support
  - `ServerInfo` class for server metadata
  - `DiscoveryHelpers` static class for response generation
- **Test Coverage**: 14 new tests for tool discovery functionality
  - Operation schema creation and serialization
  - Server info creation and serialization
  - Discovery helpers for generating MCP-compatible responses
  - Interface default method testing

### Changed
- **Package Description**: Updated to mention tool discovery feature
- **Documentation**: Updated with tool discovery examples and API reference

### Technical Details
- **No Breaking Changes**: C# already used `Dictionary<string, object>` for capabilities
- **MCP Compatible**: Full compatibility with Model Context Protocol tool discovery
- **Builder Pattern**: Fluent API for schema construction
- **JSON Schema**: Native support for input/output schema definitions
- **Total Tests**: 114 passing (100 existing + 14 new)

### Example Usage
```csharp
using Umicp.Core.ToolDiscovery;

public class MyService : IDiscoverableService
{
    public List<OperationSchema> ListOperations()
    {
        return new List<OperationSchema>
        {
            new OperationSchema("search", JsonDocument.Parse(@"{
                ""type"": ""object"",
                ""properties"": {
                    ""query"": {""type"": ""string""}
                },
                ""required"": [""query""]
            }").RootElement)
            .WithTitle("Search")
            .WithDescription("Search for content")
        };
    }

    public ServerInfo GetServerInfo()
    {
        return new ServerInfo("my-service", "1.0.0", "UMICP/0.2")
            .WithMcpCompatible(true);
    }
}
```

## [0.1.3] - 2025-01-10

### Added
- Full BIP-05 compliance
- SIMD matrix operations
- WebSocket/HTTP transport
- Compression (Gzip/Deflate)
- Peer-to-peer communication
- Service discovery
- Connection pooling

### Fixed
- Various stability improvements
- Memory optimization

## [0.1.0] - 2024-12-01

### Added
- Initial C# binding release
- Basic envelope system
- Matrix operations
- Transport layer
