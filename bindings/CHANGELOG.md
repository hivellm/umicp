# UMICP Bindings Changelog

## [0.2.2] - 2025-10-17

### Summary
Custom endpoint support added to all UMICP SDKs for compatibility with different servers (Vectorizer uses `/umicp`, standard servers use `/message`).

### Version Updates
All SDKs updated to v0.2.2:
- C++, Rust, Python, C#, Java, Kotlin, Elixir: `0.2.2`
- Go, PHP, Swift, TypeScript: `latest/0.2.2`

### Implementation Changes

**New Implementations:**
- **Python**: Added `path` parameter to `HttpClient` (default: `/message`)
- **Rust**: Added `HttpClient::new_with_path()` method + `path` config field (default: `/message`)
- **Go**: Added `Path` field to `ClientConfig` (default: `/umicp`)

**Already Supported (Verified):**
- **C++**: `HTTPClientConfig.path` field (default: `/umicp`)
- **TypeScript**: `path` option in `StreamableHTTPClientOptions` (default: `/umicp`)
- **PHP**: `path` option in constructor array (default: `/umicp`)
- **C#**: Endpoint passed as method parameter (per-request)
- **Java/Kotlin/Swift**: Endpoint passed as method parameter

### Test Results

**7 SDKs Tested - 100% Success Rate:**
- C++: 138/138 tests (100%) - Fixed timeout test
- Rust: 102/102 tests (100%) - Fixed validation test
- Python: 162/162 tests (100%) - Perfect first run
- C#: 146/146 tests (100%) - Added 18 tests
- Go: All packages passing (100%) - Fixed 4 tests
- PHP: 17/17 HTTP tests (100%)
- TypeScript: 23/23 custom tests (100%)

**Total: 688+ tests, 0 failures**

**Custom Endpoint Tests: 98/98 (100%)**
- C++: 9, Rust: 10, Python: 15, C#: 18, Go: 10, PHP: 13, TypeScript: 23

### Breaking Changes
**NONE** - Fully backward compatible

### Migration
No migration required. Existing code continues to work with defaults.

## Example Usage

**Python:**
```python
client = HttpClient("http://localhost:8000", path="/umicp")
```

**Rust:**
```rust
let client = HttpClient::new_with_path("http://localhost:8000", "/umicp")?;
```

**Go:**
```go
config := http.ClientConfig{Path: "/umicp"}
```

**TypeScript:**
```typescript
const client = new StreamableHTTPClient({baseUrl: "...", path: "/umicp"});
```

See `CUSTOM_ENDPOINT_EXAMPLES.md` for all SDKs.

## [0.2.1] - Previous
- Tool Discovery enhancements
- Bug fixes

## [0.2.0] - Previous
- MCP-compatible tool discovery
- Native JSON types
- JSON Schema support

