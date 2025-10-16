# Changelog

All notable changes to the UMICP Rust bindings will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] - 2025-10-16

### ⚠️ BREAKING CHANGES

- **Capabilities Type Change**: `Capabilities` type changed from `HashMap<String, String>` to `HashMap<String, serde_json::Value>`
  - Now supports native JSON types (numbers, booleans, arrays, objects)
  - `Envelope::add_capability()` now accepts `serde_json::Value` instead of `&str`
  - New `Envelope::add_capability_str()` convenience method for string values
  - `EnvelopeBuilder::capability()` now accepts `serde_json::Value`
  - New `EnvelopeBuilder::capability_str()` convenience method for string values

### Added

- **Tool Discovery Module** (`tool_discovery.rs`)
  - `OperationSchema` struct for defining operation schemas (MCP-compatible)
  - `ServerInfo` struct for server metadata
  - `DiscoverableService` trait for services that support tool discovery
  - Full JSON Schema support for operation parameters
  - Builder patterns for easy schema construction

- **Native Type Support in Capabilities**
  - Numbers, booleans, arrays, and objects now supported directly
  - No need to serialize/deserialize values manually
  - Better type safety and validation

### Changed

- **Envelope API Updates**
  - `add_capability()` signature changed to accept `serde_json::Value`
  - `capability()` builder method changed to accept `serde_json::Value`
  - Added `_str` variants for backward compatibility convenience

- **Discovery Module Updates**
  - Updated to use native types in capability handling
  - Service discovery envelopes now use `capability_str()` for string values

### Migration Guide

#### Before (v0.1.x):
```rust
let envelope = Envelope::builder()
    .from("client")
    .to("server")
    .capability("message", "hello")
    .capability("count", "10")  // String "10"
    .build()?;
```

#### After (v0.2.x):
```rust
use serde_json::json;

let envelope = Envelope::builder()
    .from("client")
    .to("server")
    .capability_str("message", "hello")  // Use _str for strings
    .capability("count", json!(10))       // Native number!
    .build()?;

// Or use native types directly
let mut caps = HashMap::new();
caps.insert("message".to_string(), json!("hello"));
caps.insert("count".to_string(), json!(10));
caps.insert("enabled".to_string(), json!(true));
caps.insert("tags".to_string(), json!(["a", "b", "c"]));

let envelope = Envelope::builder()
    .from("client")
    .to("server")
    .capabilities(caps)
    .build()?;
```

### For Service Implementors

If you're implementing a discoverable service:

```rust
use umicp_core::{DiscoverableService, OperationSchema, ServerInfo};
use serde_json::json;

struct MyService;

impl DiscoverableService for MyService {
    fn list_operations(&self) -> Vec<OperationSchema> {
        vec![
            OperationSchema::new(
                "search",
                json!({
                    "type": "object",
                    "properties": {
                        "query": {"type": "string"},
                        "limit": {"type": "integer", "default": 10}
                    },
                    "required": ["query"]
                })
            )
            .title("Search")
            .description("Search for items")
            .annotations(json!({"read_only": true}))
        ]
    }

    fn server_info(&self) -> ServerInfo {
        ServerInfo::new("my-service", "1.0.0", "UMICP/2.0")
            .features(vec!["discovery".to_string(), "search".to_string()])
            .operations_count(1)
            .mcp_compatible(true)
    }
}
```

### Fixed

- Envelope validation no longer attempts to validate non-string capability values as strings
- Discovery module properly handles native JSON types in capabilities

### Internal

- Added comprehensive tests for tool discovery module
- Updated examples to use new capability API
- Removed unused imports (warnings cleaned)

## [0.1.3] - 2024-XX-XX

### Added
- Initial stable release
- WebSocket and HTTP/2 transport support
- Matrix operations
- Service discovery
- Connection pooling
- Load balancing
- Comprehensive test suite

[0.2.0]: https://github.com/hivellm/umicp/compare/v0.1.3...v0.2.0
[0.1.3]: https://github.com/hivellm/umicp/releases/tag/v0.1.3
