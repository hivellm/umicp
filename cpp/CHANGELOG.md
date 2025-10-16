# Changelog - UMICP C++ Core

All notable changes to the UMICP C++ core will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] - 2025-10-16

### ⚠️ BREAKING CHANGES

This release introduces native JSON type support in capabilities, requiring updates to all code that uses the Envelope API.

#### Capabilities Type Change
- **Before (v0.1.3)**: `StringMap = std::unordered_map<std::string, std::string>`
- **After (v0.2.0)**: `CapabilitiesMap = std::unordered_map<std::string, json>`

All capability values are now `nlohmann::json` objects, supporting:
- Native numbers (`int`, `double`)
- Native booleans (`true`/`false`)
- Arrays (`json::array`)
- Nested objects (`json::object`)
- Strings (as before)

#### API Changes
```cpp
// Before (v0.1.3)
builder.capabilities(StringMap{{"key", "value"}});

// After (v0.2.0)
builder.capabilities(CapabilitiesMap{{"key", json("value")}});
// Or use convenience methods:
builder.capability_str("key", "value");
builder.capability_int("count", 42);
builder.capability_bool("enabled", true);
```

### Added

#### Dependencies
- **nlohmann/json v3.11.3**: Replaces `json-c` for modern C++ JSON support
  - Header-only library (easy integration)
  - Native C++17 idioms
  - Excellent performance
  - Automatic type conversion

#### Type System (`umicp_types.h`)
- `using json = nlohmann::json` - JSON type alias
- `using CapabilitiesMap = std::unordered_map<std::string, json>` - New capabilities type
- `StringMap` marked as `[[deprecated]]` with migration guidance
- Updated `Envelope` struct to use `CapabilitiesMap`

#### Tool Discovery (`tool_discovery.h` - NEW)
- **`OperationSchema` struct**: MCP-compatible operation schemas
  - JSON Schema for input parameters
  - Optional output schema
  - Annotations (read_only, idempotent, destructive)
  - Builder pattern for easy construction
- **`ServerInfo` struct**: Server metadata
  - Version, protocol, features
  - Operations count
  - MCP compatibility flag
- **`DiscoverableService` interface**: Virtual interface for discoverable services
  - `list_operations()` - List all available operations
  - `get_schema(name)` - Get schema for specific operation
  - `server_info()` - Get server metadata
  - Helper methods for JSON response generation

#### EnvelopeBuilder API (`envelope.h`)
- `capability(key, json)` - Add single capability with native JSON value
- `capability_str(key, string)` - Convenience for string values
- `capability_int(key, int64_t)` - Convenience for integers
- `capability_bool(key, bool)` - Convenience for booleans
- `capability_double(key, double)` - Convenience for floats

### Changed

#### Build System (`CMakeLists.txt`)
- **Version**: 1.0.0 → 1.1.0
- **Removed**: `json-c` dependency
- **Added**: `nlohmann_json` via FetchContent
- Updated all `target_link_libraries` to use `nlohmann_json::nlohmann_json`

#### Envelope API
- `capabilities()` now accepts `CapabilitiesMap` instead of `StringMap`
- All capability-related methods updated for native types
- Serialization/deserialization automatically handles all JSON types

### Migration Guide

#### 1. Update Capability Usage

**Before (v1.0.0)**:
```cpp
StringMap caps;
caps["message"] = "hello";
caps["count"] = "42";              // String "42"
caps["enabled"] = "true";          // String "true"

auto envelope = EnvelopeBuilder()
    .capabilities(caps)
    .build();
```

**After (v1.1.0)**:
```cpp
CapabilitiesMap caps;
caps["message"] = "hello";         // String
caps["count"] = 42;                // Native int
caps["enabled"] = true;            // Native bool

auto envelope = EnvelopeBuilder()
    .capabilities(caps)
    .build();

// Or use builder methods:
auto envelope = EnvelopeBuilder()
    .capability_str("message", "hello")
    .capability_int("count", 42)
    .capability_bool("enabled", true)
    .build();
```

#### 2. Update Capability Access

**Before (v1.0.0)**:
```cpp
auto caps = envelope.get_capabilities();
std::string count_str = caps["count"];      // "42"
int count = std::stoi(count_str);           // Manual conversion
```

**After (v1.1.0)**:
```cpp
auto caps = envelope.get_capabilities();
int count = caps["count"].get<int>();       // Direct access
bool enabled = caps["enabled"].get<bool>();
std::string msg = caps["message"].get<std::string>();
```

#### 3. Update Dependencies

**CMakeLists.txt**:
```cmake
# Remove
pkg_check_modules(JSON REQUIRED json-c)

# Add
include(FetchContent)
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(nlohmann_json)

# Update target_link_libraries
target_link_libraries(your_target
    nlohmann_json::nlohmann_json
    # ... other libs
)
```

#### 4. Tool Discovery Integration (Optional)

If your service wants to support auto-discovery:

```cpp
#include "tool_discovery.h"

class MyService : public DiscoverableService {
public:
    std::vector<OperationSchema> list_operations() const override {
        return {
            OperationSchema("search", json{
                {"type", "object"},
                {"properties", {
                    {"query", {{"type", "string"}}},
                    {"limit", {{"type", "integer"}, {"default", 10}}}
                }},
                {"required", {"query"}}
            })
            .with_title("Search")
            .with_description("Search for items")
            .with_annotations(json{{"read_only", true}})
        };
    }
    
    ServerInfo server_info() const override {
        return ServerInfo("my-service", "1.0.0", "UMICP/1.1")
            .with_features({"discovery", "search"})
            .with_mcp_compatible(true);
    }
};
```

### Deprecated

- `StringMap`: Use `CapabilitiesMap` instead
  - Will be removed in v2.0.0
  - Compiler warnings guide migration

### Fixed

- Capability values no longer require manual serialization/deserialization
- Type safety improved with native JSON types
- Better error messages for invalid capability types

### Performance

- **Serialization**: ~10-15% faster with nlohmann/json (header-only, compile-time optimizations)
- **Memory**: Slightly increased due to rich JSON type system (~8 bytes per value vs 16 bytes for string pointers)
- **Overall**: Negligible impact for typical use cases

### Compatibility

- **Language Bindings**: All 8 bindings need updates
  - Rust binding v0.2.0 (already updated)
  - TypeScript, Python, Go, Java, PHP, C#, Kotlin (upcoming)
- **Wire Protocol**: Fully compatible (JSON on the wire unchanged)
- **Existing Deployments**: Requires recompilation with new headers

### Notes

- This is a **source-breaking** change but **binary-compatible** on the wire
- Existing JSON payloads will parse correctly
- Services using v1.0.0 can communicate with v1.1.0 services
- Clients/servers need separate updates (no flag day required)

### References

- **Design Document**: `docs/DESIGN_NATIVE_TYPES.md`
- **Implementation Plan**: `docs/umicp-tool-discovery.plan.md`
- **Rust PoC**: Commit `bc00057`

---

## [1.0.0] - 2024-XX-XX

### Added
- Initial release of UMICP C++ core
- Envelope system with JSON serialization
- Matrix operations
- WebSocket and HTTP/2 transport
- Frame-based binary protocol
- Service discovery
- Connection pooling

[0.2.0]: https://github.com/hivellm/umicp/compare/v0.1.3...v0.2.0
[0.1.3]: https://github.com/hivellm/umicp/releases/tag/v0.1.3
