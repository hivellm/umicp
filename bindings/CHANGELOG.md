# UMICP Bindings Changelog

## [0.3.2] - 2025-10-24

### Python SDK v0.3.2 - Published to PyPI ✅

**Package Name Standardization:**
- ✅ Published to PyPI: https://pypi.org/project/umicp-sdk/0.3.2/
- Package: `umicp_sdk` (PEP 625 compliant)
- Installation: `pip install umicp-sdk`
- Import: `from umicp_sdk import Envelope`

## [0.3.1] - 2025-10-24

### Version Bumps for Republishing

**Version Updates:**
- Python: 0.3.1 → 0.3.2
- Rust: 0.3.0 → 0.3.1
- TypeScript: 0.3.0 → 0.3.1

**Documentation:**
- ✅ All READMEs updated with standardized package names
- ✅ Installation instructions corrected
- ✅ Version references updated

## [0.3.0] - 2025-10-24

### BREAKING CHANGE: Package Name Standardization

**All SDKs renamed to follow `umicp-sdk` convention:**

| SDK | Old Name | New Name | Version |
|-----|----------|----------|---------|
| **Python** | `umicp-python` | `umicp_sdk` | 0.3.1 |
| **Rust** | `umicp-core` | `umicp-sdk` | 0.3.0 |
| **TypeScript** | `@hivellm/umicp` | `@hivellm/umicp-sdk` | 0.3.0 |
| **Go** | `umicp-go` | `github.com/hivellm/umicp-sdk` | 0.3.0 |
| **C#** | `HiveLLM.Umicp` | `HiveLLM.Umicp.SDK` | 0.3.0 |
| **PHP** | `hivellm/umicp` | `hivellm/umicp-sdk` | 0.3.0 |
| **Swift** | `UMICP` | `UMICP-SDK` | 0.3.0 |
| **Kotlin** | `umicp-kotlin` | `umicp-sdk` | 0.3.0 |
| **Elixir** | `umicp` | `umicp_sdk` | 0.3.0 |
| **Java** | `umicp-java-parent` | `umicp-sdk` | 0.3.0 |

### Migration Guide

**Python:**
```bash
# Old
pip install umicp-python
from umicp import Envelope

# New
pip install umicp-sdk
from umicp_sdk import Envelope
```

**Rust:**
```toml
# Old
[dependencies]
umicp-core = "0.2.3"

# New
[dependencies]
umicp-sdk = "0.3.1"
```

**TypeScript:**
```bash
# Old
npm install @hivellm/umicp

# New
npm install @hivellm/umicp-sdk
```

**C#:**
```bash
# Old
dotnet add package HiveLLM.Umicp

# New
dotnet add package HiveLLM.Umicp.SDK
```

**PHP:**
```bash
# Old
composer require hivellm/umicp

# New
composer require hivellm/umicp-sdk
```

### Why This Change

- **Consistency**: All SDKs now use `-sdk` suffix for clarity
- **PEP 625**: Python and Elixir use underscores (language convention)
- **Discoverability**: SDK suffix makes package purpose clear
- **Maintainability**: Easier to manage multiple language bindings

### Compatibility

- ✅ All APIs remain identical (only import/package names changed)
- ✅ All features from v0.2.x preserved
- ✅ No functionality removed
- ❌ Old package names deprecated

---

## [0.2.3] - 2025-10-17

### SDK Improvements - Automatic URL Path Detection

**Problem Solved:**
When passing a full URL with path (e.g., `http://localhost:15002/umicp`), SDKs were incorrectly concatenating the default path, resulting in errors like `http://localhost:15002/umicp/message` (405 Method Not Allowed).

---

### Rust SDK v0.2.3

**Automatic Path Detection:**
- `HttpClient::new()` now automatically parses URLs to extract custom paths
- URLs like `http://localhost:15002/umicp` automatically separate base URL and path
- Maintains backward compatibility: URLs without paths default to `/message`
- No breaking changes: `new_with_path()` method still available for explicit control

**Example:**
```rust
// Before: Manual path separation required
let client = HttpClient::new_with_path("http://localhost:15002", "/umicp")?;

// After: Automatic path detection
let client = HttpClient::new("http://localhost:15002/umicp")?;
// Automatically sets: base_url="http://localhost:15002", path="/umicp"

// Still works: Default behavior
let client = HttpClient::new("http://localhost:3000")?;
// Automatically sets: base_url="http://localhost:3000", path="/message"
```

**Tests Added:**
- 6 comprehensive tests validating path parsing behavior
- Tests cover: custom paths, no path, root path, with port
- All tests passing (100%)

**Implementation:**
- Added `url` crate dependency (v2.5)
- URL parsing in `HttpClient::new()` constructor
- Fallback to default `/message` for URLs without paths

---

### Go SDK v0.2.3

**Automatic Path Detection:**
- `NewClient()` now automatically parses URLs to extract custom paths  
- URLs like `http://localhost:15002/umicp` automatically separate base URL and path
- Maintains backward compatibility: URLs without paths default to `/umicp`
- Path in URL takes precedence over explicit `Path` config field

**Example:**
```go
// Before: Manual path separation required
config := ClientConfig{
    BaseURL: "http://localhost:15002",
    Path:    "/umicp",
}

// After: Automatic path detection
config := ClientConfig{
    BaseURL: "http://localhost:15002/umicp",
}
// Automatically sets: BaseURL="http://localhost:15002", Path="/umicp"

// Still works: Default behavior
config := ClientConfig{
    BaseURL: "http://localhost:3000",
}
// Automatically sets: BaseURL="http://localhost:3000", Path="/umicp"
```

**Tests Added:**
- 6 comprehensive tests validating path parsing behavior
- Tests cover: custom paths, no path, root path, with port, explicit path override
- All tests passing (100%)

**Implementation:**
- URL parsing in `NewClient()` function using `net/url`
- Fallback to default `/umicp` for URLs without paths

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

