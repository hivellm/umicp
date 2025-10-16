# Changelog - UMICP TypeScript Binding

## [0.2.0] - 2025-01-16

### Added - Tool Discovery & Native Types
- **Tool Discovery**: MCP-compatible tool discovery system
  - `DiscoverableService` interface for automatic tool introspection
  - `OperationSchema` with JSON Schema support
  - `ServerInfo` for server metadata
  - Builder patterns: `OperationSchemaBuilder`, `ServerInfoBuilder`
  - Helper functions for response generation
- **Native Types**: `Record<string, any>` for capabilities (was `Record<string, string>`)
  - Support for integers, floats, booleans, arrays, objects, null
  - Full JSON value support in capabilities
- **Tests**: 14 new tool discovery tests (124 existing + 14 new = 138 total)

### Changed
- **BREAKING**: `EnvelopeOptions.capabilities` type changed from `Record<string, string>` to `Record<string, any>`
- **BREAKING**: `Envelope.setCapabilities()` and `getCapabilities()` now use `Record<string, any>`
- Updated package description

### Migration Guide
```typescript
// Old (v0.1.5)
envelope.setCapabilities({ model: "gpt-4", count: "100" });

// New (v0.2.0) - Native types supported
envelope.setCapabilities({ 
  model: "gpt-4", 
  count: 100,
  enabled: true,
  ratio: 0.75 
});
```

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
