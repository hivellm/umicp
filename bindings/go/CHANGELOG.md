# Changelog - UMICP Go Binding

## [0.2.1] - 2025-10-16

### Added - Tool Discovery

- **Tool Discovery**: MCP-compatible tool discovery system
  - `DiscoverableService` interface for automatic tool introspection
  - `OperationSchema` struct with JSON Schema support
  - `ServerInfo` struct for server metadata and capabilities
  - Builder patterns: `OperationSchemaBuilder`, `ServerInfoBuilder`
  - `DiscoveryHelpers` for generating MCP-compatible responses
  - `SimpleDiscoverableService` reference implementation
- **Tests**: 10 new comprehensive tool discovery tests

### Note on Native Types

- Go binding already supported native types (`map[string]interface{}`) since the beginning
- No breaking changes required for native type support
- Full JSON value support already available in capabilities

### Migration Guide

No migration needed! This is a purely additive release. New Tool Discovery features are optional.

### Example Usage

```go
package main

import (
    "fmt"
    "github.com/hivellm/umicp/pkg/umicp"
)

type MyService struct {
    operations []umicp.OperationSchema
}

func (s *MyService) ListOperations() []umicp.OperationSchema {
    return s.operations
}

func (s *MyService) GetSchema(name string) *umicp.OperationSchema {
    for i := range s.operations {
        if s.operations[i].Name == name {
            return &s.operations[i]
        }
    }
    return nil
}

func (s *MyService) GetServerInfo() umicp.ServerInfo {
    return umicp.ServerInfo{
        Server:          "my-service",
        Version:         "1.0.0",
        Protocol:        "UMICP/1.0",
        MCPCompatible:   true,
        OperationsCount: len(s.operations),
    }
}

func main() {
    // Create operation schema using builder
    schema := umicp.NewOperationSchema("search", umicp.JSONSchema{
        "type": "object",
        "properties": map[string]interface{}{
            "query": map[string]string{"type": "string"},
        },
    }).
        WithTitle("Search Operation").
        WithDescription("Searches the database").
        Build()

    service := &MyService{
        operations: []umicp.OperationSchema{schema},
    }

    // Generate discovery responses
    helpers := umicp.DiscoveryHelpers{}
    opsResponse := helpers.GenerateOperationsResponse(service)
    
    fmt.Printf("Available operations: %v\n", opsResponse)
}
```

## [0.1.3] - Previous Release

See Git history for previous changes.

