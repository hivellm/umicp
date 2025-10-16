# Changelog - UMICP TypeScript Binding

## [0.2.1] - 2025-10-16

### Added - Tool Discovery
- **Tool Discovery**: MCP-compatible tool discovery system
  - `DiscoverableService` interface for automatic tool introspection
  - `OperationSchema` with JSON Schema support for operation metadata
  - `ServerInfo` for server metadata and capabilities
  - Builder patterns: `OperationSchemaBuilder`, `ServerInfoBuilder`
  - `DiscoveryHelpers` for generating MCP-compatible responses
  - `SimpleDiscoverableService` reference implementation
- **Tests**: 10 new comprehensive tool discovery tests

### Note on Native Types
- TypeScript binding already supported native types (`Record<string, any>`) since v0.1.5+
- No breaking changes required for native type support
- Full JSON value support already available in capabilities

### Migration Guide
No migration needed! This is a purely additive release. New Tool Discovery features are optional.

### Example Usage
```typescript
import { 
  DiscoverableService, 
  OperationSchemaBuilder,
  ServerInfoBuilder 
} from '@hivellm/umicp';

class MyService implements DiscoverableService {
  listOperations() {
    return [
      new OperationSchemaBuilder('search', {
        type: 'object',
        properties: { query: { type: 'string' } }
      })
      .withTitle('Search')
      .withDescription('Search for content')
      .build()
    ];
  }
  
  getSchema(name: string) {
    return this.listOperations().find(op => op.name === name) || null;
  }
  
  getServerInfo() {
    return new ServerInfoBuilder('my-service', '1.0.0', 'UMICP/0.2')
      .withMcpCompatible(true)
      .build();
  }
}
```

## [0.1.5] - 2025-01-10

### Added
- StreamableHTTP transport support
- WebSocket peer-to-peer communication
- Full BIP-05 compliance
- Matrix operations with SIMD acceleration

### Fixed
- Various stability improvements
- Memory leak fixes

## [0.1.0] - 2024-12-01

### Added
- Initial TypeScript binding release
- Basic envelope system
- Matrix operations
- Transport layer
