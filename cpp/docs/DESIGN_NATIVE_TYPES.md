# UMICP C++ Core - Native Types Design Document

**Version**: 1.1.0  
**Date**: 2025-10-16  
**Status**: Design Phase

---

## Executive Summary

Atualizar o UMICP C++ core para suportar tipos nativos em `capabilities`, alinhando com a implementação Rust 0.2.0 e preparando terreno para todos os bindings.

---

## Current State (v1.0.0)

### Type Definitions
```cpp
// umicp/cpp/include/umicp_types.h:143
using StringMap = std::unordered_map<std::string, std::string>;

// Envelope struct
struct Envelope {
    std::optional<StringMap> capabilities;  // ← String only!
};
```

### Serialization (json-c)
```cpp
// umicp/cpp/src/envelope.cpp:62-66
if (envelope.capabilities) {
    json_object* caps = json_object_new_object();
    for (const auto& [key, value] : *envelope.capabilities) {
        json_object_object_add(caps, key.c_str(), 
            json_object_new_string(value.c_str()));  // ← String only!
    }
}
```

### Problems
- ❌ Only strings supported
- ❌ Must manually serialize/deserialize numbers, booleans, arrays
- ❌ No type safety
- ❌ Poor developer experience

---

## Proposed Solution

### Option A: Migrate to nlohmann/json (RECOMMENDED)

**Library**: https://github.com/nlohmann/json  
**License**: MIT  
**Type**: Header-only (single file)

#### Advantages
- ✅ C++17 native
- ✅ Modern C++ idioms (std::optional, std::variant)
- ✅ Header-only (easy integration)
- ✅ Excellent performance
- ✅ Wide adoption (30k+ stars)
- ✅ Active maintenance
- ✅ Natural JSON type system

#### New Type Definitions
```cpp
// umicp/cpp/include/umicp_types.h
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using CapabilitiesMap = std::unordered_map<std::string, json>;

struct Envelope {
    std::optional<CapabilitiesMap> capabilities;  // Native types!
};
```

#### Usage Example
```cpp
// Create envelope with native types
auto envelope = EnvelopeBuilder()
    .from("client")
    .to("server")
    .operation(OperationType::DATA)
    .capability("count", 42)                    // int
    .capability("enabled", true)                // bool
    .capability("tags", {"rust", "cpp"})        // array
    .capability("metadata", {{"key", "value"}}) // object
    .build();

// Access native types
if (auto caps = envelope.get_capabilities()) {
    int count = (*caps)["count"].get<int>();           // 42
    bool enabled = (*caps)["enabled"].get<bool>();     // true
    auto tags = (*caps)["tags"].get<std::vector<std::string>>();
}
```

#### New Serialization
```cpp
// Automatically handles all JSON types
json root;
root["capabilities"] = envelope.capabilities.value();  // Just works!
```

### Option B: Extend json-c (NOT RECOMMENDED)

**Keep current json-c, add type wrapper**

#### Advantages
- ✅ No dependency change
- ✅ Smaller diff

#### Disadvantages
- ❌ More complex code
- ❌ Manual type handling
- ❌ Less idiomatic C++
- ❌ Poor DX

```cpp
// Would need custom wrapper
struct JsonValue {
    enum Type { String, Int, Double, Bool, Array, Object };
    Type type;
    std::variant<std::string, int64_t, double, bool, /* ... */> value;
};

using CapabilitiesMap = std::unordered_map<std::string, JsonValue>;
// Much more complex!
```

---

## Recommended Approach: Option A (nlohmann/json)

### 1. Update Dependencies

**CMakeLists.txt**:
```cmake
# Remove json-c
# pkg_check_modules(JSON REQUIRED json-c)

# Add nlohmann/json (header-only)
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)
```

Or use system package:
```cmake
find_package(nlohmann_json 3.11.0 REQUIRED)
target_link_libraries(umicp PRIVATE nlohmann_json::nlohmann_json)
```

### 2. Update Type Definitions

**umicp_types.h**:
```cpp
#include <nlohmann/json.hpp>

namespace umicp {

using json = nlohmann::json;

// BREAKING CHANGE v1.1.0
using CapabilitiesMap = std::unordered_map<std::string, json>;

struct Envelope {
    std::string version;
    std::string msg_id;
    std::string ts;
    std::string from;
    std::string to;
    OperationType op;
    std::optional<CapabilitiesMap> capabilities;  // ← Changed!
    std::optional<std::string> schema_uri;
    std::optional<std::vector<std::string>> accept;
    std::optional<PayloadHint> payload_hint;
    std::optional<std::vector<PayloadRef>> payload_refs;
};

} // namespace umicp
```

### 3. Update Envelope Builder

**envelope.h**:
```cpp
class EnvelopeBuilder {
public:
    EnvelopeBuilder();
    ~EnvelopeBuilder();

    EnvelopeBuilder& from(const std::string& from);
    EnvelopeBuilder& to(const std::string& to);
    EnvelopeBuilder& operation(OperationType op);
    EnvelopeBuilder& message_id(const std::string& msg_id);
    
    // BREAKING CHANGE: accepts json values
    EnvelopeBuilder& capability(const std::string& key, const json& value);
    EnvelopeBuilder& capabilities(const CapabilitiesMap& caps);
    
    // Convenience methods
    EnvelopeBuilder& capability_str(const std::string& key, const std::string& value);
    EnvelopeBuilder& capability_int(const std::string& key, int64_t value);
    EnvelopeBuilder& capability_bool(const std::string& key, bool value);

    Result<Envelope> build();
};
```

### 4. Update Serialization

**envelope.cpp**:
```cpp
Result<std::string> EnvelopeImpl::serialize() {
    json root;
    
    // Required fields
    root["v"] = envelope.version;
    root["msg_id"] = envelope.msg_id;
    root["ts"] = envelope.ts;
    root["from"] = envelope.from;
    root["to"] = envelope.to;
    root["op"] = operation_to_string(envelope.op);
    
    // Optional fields
    if (envelope.capabilities) {
        root["capabilities"] = *envelope.capabilities;  // Just works!
    }
    
    // ... other fields
    
    return Result<std::string>(root.dump());  // Serialize
}

Result<void> EnvelopeImpl::deserialize(const std::string& json_str) {
    json root = json::parse(json_str);
    
    envelope.version = root["v"].get<std::string>();
    envelope.msg_id = root["msg_id"].get<std::string>();
    // ...
    
    if (root.contains("capabilities")) {
        envelope.capabilities = root["capabilities"].get<CapabilitiesMap>();
    }
    
    return Result<void>();
}
```

### 5. Add Tool Discovery

**tool_discovery.h** (NEW):
```cpp
#ifndef UMICP_TOOL_DISCOVERY_H
#define UMICP_TOOL_DISCOVERY_H

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <optional>

namespace umicp {

using json = nlohmann::json;

/**
 * Operation schema compatible with MCP JSON Schema
 */
struct OperationSchema {
    std::string name;
    std::optional<std::string> title;
    std::optional<std::string> description;
    json input_schema;  // JSON Schema
    std::optional<json> output_schema;
    std::optional<json> annotations;
};

/**
 * Server information for discovery
 */
struct ServerInfo {
    std::string server;
    std::string version;
    std::string protocol;
    std::optional<std::vector<std::string>> features;
    std::optional<size_t> operations_count;
    std::optional<bool> mcp_compatible;
    std::optional<json> metadata;
};

/**
 * Interface for services that support tool discovery
 */
class DiscoverableService {
public:
    virtual ~DiscoverableService() = default;
    
    /**
     * List all available operations with their schemas
     */
    virtual std::vector<OperationSchema> list_operations() const = 0;
    
    /**
     * Get schema for a specific operation by name
     */
    virtual std::optional<OperationSchema> get_schema(const std::string& name) const {
        auto ops = list_operations();
        for (const auto& op : ops) {
            if (op.name == name) return op;
        }
        return std::nullopt;
    }
    
    /**
     * Get server information and metadata
     */
    virtual ServerInfo server_info() const = 0;
};

} // namespace umicp

#endif // UMICP_TOOL_DISCOVERY_H
```

---

## Migration Path

### Phase 1: C++ Core (v1.1.0)
1. ✅ Add nlohmann/json dependency
2. ✅ Update umicp_types.h
3. ✅ Update envelope.h/cpp
4. ✅ Add tool_discovery.h
5. ✅ Update all tests
6. ✅ Update examples
7. ✅ Update CHANGELOG

### Phase 2: Rust Binding (align with C++ 1.1.0)
- Update Rust to follow C++ core exactly
- Ensure FFI compatibility

### Phase 3: All Other Bindings
- TypeScript
- Python
- Go
- Java
- PHP
- C#
- Kotlin

---

## Breaking Changes

### v1.0.0 → v1.1.0

#### Type Changes
```cpp
// Before (v1.0.0)
using StringMap = std::unordered_map<std::string, std::string>;
envelope.capabilities = StringMap{{"key", "value"}};

// After (v1.1.0)
using CapabilitiesMap = std::unordered_map<std::string, json>;
envelope.capabilities = CapabilitiesMap{{"key", json("value")}};
// Or just:
envelope.capabilities = CapabilitiesMap{{"key", "value"}};  // Auto-converts
```

#### Builder API
```cpp
// Before
builder.capability("count", "42");  // String

// After
builder.capability("count", 42);                    // int
builder.capability("enabled", true);                // bool
builder.capability("tags", json::array({"a", "b"}));  // array

// Convenience (backward compatible)
builder.capability_str("message", "hello");
```

---

## Timeline

- **Week 1**: Update C++ core with nlohmann/json
- **Week 2**: Add tool discovery interfaces
- **Week 3**: Update all tests and examples
- **Week 4**: Update Rust binding to match
- **Week 5-8**: Update all 8 bindings

---

## Decision

**APPROVE** Option A (nlohmann/json) to proceed?

- [ ] Yes, proceed with nlohmann/json
- [ ] No, use Option B (extend json-c)
- [ ] Alternative suggestion: _______________

---

**Maintained by**: HiveLLM Team  
**Last Updated**: 2025-10-16

