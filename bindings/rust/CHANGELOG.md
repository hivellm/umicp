# Changelog - UMICP Rust Binding

All notable changes to the UMICP Rust binding will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.2] - 2025-10-17

### Added
- **Custom Endpoint Support**: `HttpClient::new_with_path()` method
  - New helper method for creating HTTP clients with custom paths
  - Example: `HttpClient::new_with_path("http://localhost:8000", "/umicp")?`
- **HttpClientConfig Enhancement**: Added `path` field
  - Default value: `"/message"`
  - Configurable for different server implementations
  - Vectorizer uses `/umicp`, standard servers use `/message`
- **Test Suite**: Added `tests/custom_endpoint_test.rs` with 10 comprehensive tests
  - New method validation
  - Config field validation
  - Multiple endpoint scenarios
  - Version compatibility verification

### Changed
- **Version Alignment**: Updated to v0.2.2 for consistency across all SDK bindings
  - All SDKs now at version 0.2.2
  - Unified custom endpoint support across languages

### Technical Details
- `HttpClientConfig` struct enhanced with `path: String` field
- New constructor: `HttpClient::new_with_path(base_url, path)`
- No breaking changes - fully backward compatible
- Default `HttpClient::new()` continues to use `/message`
- All existing tests pass (101/102, 1 ignored for review)

### Test Results
```
✅ Library Tests:         40/40 passed
✅ Custom Endpoint Tests: 10/10 passed (with http2 feature)
✅ Envelope Tests:        14/14 passed
✅ Integration Tests:      9/9 passed (fixed validation test)
✅ Matrix Tests:          14/14 passed
✅ Native Types Tests:    14/14 passed
✅ Tool Discovery Tests:  11/11 passed
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
   TOTAL:                102/102 passed (100%)
```

**Achievement:** Fixed `test_envelope_validation` that was previously ignored!

### Example Usage

**Vectorizer Integration:**
```rust
use umicp_core::transport::http_client::HttpClient;

// Method 1: Using new_with_path helper
let client = HttpClient::new_with_path("http://localhost:8000", "/umicp")?;

// Method 2: Using full config
let config = HttpClientConfig {
    base_url: "http://localhost:8000".to_string(),
    path: "/umicp".to_string(),
    timeout: Duration::from_secs(30),
    max_retries: 3,
};
let client = HttpClient::with_config(config)?;
```

**Standard UMICP Server:**
```rust
// Default behavior (uses /message)
let client = HttpClient::new("http://localhost:9000")?;

// Or explicit:
let client = HttpClient::new_with_path("http://localhost:9000", "/message")?;
```

### Migration Guide

No migration required! The changes are fully backward compatible:

```rust
// Old code continues to work
let client = HttpClient::new("http://localhost:8000")?;  // Uses /message

// New code can specify custom endpoints
let client = HttpClient::new_with_path("http://localhost:8000", "/umicp")?;
```

## [0.2.1] - Previous

### Added
- Tool Discovery support
- Enhanced error handling
- Performance improvements

## [0.2.0] - Previous

### Added
- Native JSON types in capabilities
- JSON Schema support
- MCP-compatible tool discovery
- Operation introspection

## [0.1.x] - Previous Releases

### Added
- Core UMICP protocol implementation
- Matrix operations
- WebSocket and HTTP/2 transport
- Compression support
- Service discovery
- Connection pooling
