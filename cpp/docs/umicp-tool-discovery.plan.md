<!-- b54bb44a-9d15-4409-8d43-9135217986a2 7279c2a9-1a93-4d4a-885a-c421bb51299d -->
# UMICP Tool Discovery - Multi-Repository Implementation

**Last Updated**: 2025-10-16  
**Status**: Phase 1 Complete (Rust PoC) → Phase 2 In Progress (C++ Core)

---

## 🎯 Revised Strategy: C++ Core First

After completing the Rust binding as a **Proof of Concept**, we've decided to properly implement this in the **C++ core first**, then align all bindings.

### Why C++ Core First?
- ✅ Maintains architectural consistency
- ✅ All 8 language bindings inherit from C++ core
- ✅ Single source of truth for protocol behavior
- ✅ Better long-term maintainability

---

## 📊 Current Status

### ✅ Phase 1: Rust PoC (COMPLETE)
- [x] Rust binding updated to 0.2.0 with native types
- [x] Tool discovery trait implemented
- [x] All tests passing (40/40)
- [x] CHANGELOG.md complete
- [x] Committed to `feat/tool-discovery-native-types` branch

### 🔄 Phase 2: C++ Core (IN PROGRESS)
- [x] Design document created (`umicp/cpp/docs/DESIGN_NATIVE_TYPES.md`)
- [x] JSON library analysis (nlohmann/json vs json-c)
- [ ] **PENDING APPROVAL**: Proceed with nlohmann/json implementation
- [ ] Update C++ core to v1.1.0
- [ ] Align Rust binding with C++ core

---

## Scope

This is a **major breaking change** affecting:

- **UMICP C++ Core** (envelope definition) - v1.0.0 → v1.1.0
- **UMICP Rust Binding** - Align with C++ core
- **UMICP TypeScript Binding** - v0.1.3 → v0.2.0
- **UMICP Python Binding** - v0.1.3 → v0.2.0
- **UMICP Go Binding** - v0.1.3 → v0.2.0
- **UMICP Java Binding** - v0.1.3 → v0.2.0
- **UMICP PHP Binding** - v0.1.3 → v0.2.0
- **UMICP C# Binding** - v1.0.0 → v1.1.0
- **UMICP Kotlin Binding** - v0.1.3 → v0.2.0
- **Vectorizer UMICP integration** - After all bindings updated

---

## Breaking Changes

### Core Protocol Changes
- **Capabilities**: `string → string` map → `string → JSON value` map
- **Version**: 1.0.0 → 1.1.0 (C++ core)
- **All bindings** need updates to support native types

### Benefits
- ✅ Native JSON types (numbers, booleans, arrays, objects)
- ✅ Tool discovery for auto-discovery services
- ✅ JSON Schema support (MCP-compatible)
- ✅ Better developer experience
- ✅ No manual serialization/deserialization

---

## Implementation Phases

### Phase 2: C++ Core (v1.1.0) - CURRENT

**Branch**: `feat/tool-discovery-native-types` (in umicp repo)

#### Step 1: Update Dependencies
**File**: `umicp/cpp/CMakeLists.txt`

```cmake
# Replace json-c with nlohmann/json
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)
```

#### Step 2: Update Type Definitions
**File**: `umicp/cpp/include/umicp_types.h`

```cpp
#include <nlohmann/json.hpp>

namespace umicp {

using json = nlohmann::json;

// BREAKING CHANGE v1.1.0
using CapabilitiesMap = std::unordered_map<std::string, json>;

struct Envelope {
    std::optional<CapabilitiesMap> capabilities;  // ← Changed from StringMap
    // ... other fields
};

} // namespace umicp
```

#### Step 3: Update Envelope Implementation
**File**: `umicp/cpp/src/envelope.cpp`

```cpp
// Serialization (simplified with nlohmann/json)
Result<std::string> EnvelopeImpl::serialize() {
    json root;
    root["v"] = envelope.version;
    root["msg_id"] = envelope.msg_id;
    // ...
    
    if (envelope.capabilities) {
        root["capabilities"] = *envelope.capabilities;  // Automatic!
    }
    
    return Result<std::string>(root.dump());
}

// Deserialization
Result<void> EnvelopeImpl::deserialize(const std::string& json_str) {
    json root = json::parse(json_str);
    
    if (root.contains("capabilities")) {
        envelope.capabilities = root["capabilities"].get<CapabilitiesMap>();
    }
    
    return Result<void>();
}
```

#### Step 4: Update Envelope Builder
**File**: `umicp/cpp/include/envelope.h`

```cpp
class EnvelopeBuilder {
public:
    // BREAKING CHANGE: accepts json values
    EnvelopeBuilder& capability(const std::string& key, const json& value);
    
    // Convenience methods (backward compatible)
    EnvelopeBuilder& capability_str(const std::string& key, const std::string& value);
    EnvelopeBuilder& capability_int(const std::string& key, int64_t value);
    EnvelopeBuilder& capability_bool(const std::string& key, bool value);
    
    Result<Envelope> build();
};
```

#### Step 5: Add Tool Discovery
**File**: `umicp/cpp/include/tool_discovery.h` (NEW)

```cpp
#ifndef UMICP_TOOL_DISCOVERY_H
#define UMICP_TOOL_DISCOVERY_H

#include <nlohmann/json.hpp>

namespace umicp {

struct OperationSchema {
    std::string name;
    std::optional<std::string> title;
    std::optional<std::string> description;
    json input_schema;
    std::optional<json> output_schema;
    std::optional<json> annotations;
};

struct ServerInfo {
    std::string server;
    std::string version;
    std::string protocol;
    std::optional<std::vector<std::string>> features;
    std::optional<size_t> operations_count;
    std::optional<bool> mcp_compatible;
};

class DiscoverableService {
public:
    virtual ~DiscoverableService() = default;
    virtual std::vector<OperationSchema> list_operations() const = 0;
    virtual ServerInfo server_info() const = 0;
};

} // namespace umicp

#endif
```

#### Step 6: Update Tests
**Files**: All test files in `umicp/cpp/tests/`

- Update envelope tests for native types
- Add tool discovery tests
- Verify backward compatibility where possible

#### Step 7: Documentation
- [x] `umicp/cpp/docs/DESIGN_NATIVE_TYPES.md` - Design document
- [ ] `umicp/cpp/CHANGELOG.md` - Breaking changes for v1.1.0
- [ ] `umicp/cpp/README.md` - Update examples
- [ ] Migration guide for all bindings

**To-dos for Phase 2**:
- [ ] Implement nlohmann/json in CMakeLists.txt
- [ ] Update umicp_types.h with CapabilitiesMap
- [ ] Refactor envelope.cpp serialization
- [ ] Update EnvelopeBuilder API
- [ ] Add tool_discovery.h interface
- [ ] Update all C++ tests
- [ ] Create CHANGELOG for v1.1.0

---

### Phase 3: Rust Binding Alignment

**Goal**: Align Rust 0.2.0 with C++ 1.1.0

**Changes**:
- Review Rust implementation against C++ core
- Ensure FFI compatibility
- Match API signatures
- Update examples to match C++ patterns

**To-dos**:
- [ ] Compare Rust 0.2.0 with C++ 1.1.0 design
- [ ] Adjust Rust types if needed
- [ ] Verify FFI compatibility
- [ ] Update Rust examples to match C++ patterns
- [ ] Run cross-language integration tests

---

### Phase 4: TypeScript Binding (v0.2.0)

**File**: `umicp/bindings/typescript/src/envelope.ts`

```typescript
interface Capabilities {
    [key: string]: any;  // Native JSON types!
}

class EnvelopeBuilder {
    capability(key: string, value: any): this {
        this.capabilities[key] = value;  // Numbers, booleans, arrays, objects
        return this;
    }
}
```

**To-dos**:
- [ ] Update Capabilities interface
- [ ] Update EnvelopeBuilder
- [ ] Add DiscoverableService interface
- [ ] Update tests
- [ ] Update CHANGELOG

---

### Phase 5: Python Binding (v0.2.0)

**File**: `umicp/bindings/python/umicp/envelope.py`

```python
from typing import Any, Dict

class Envelope:
    def __init__(self):
        self.capabilities: Dict[str, Any] = {}  # Native types!
    
class EnvelopeBuilder:
    def capability(self, key: str, value: Any) -> 'EnvelopeBuilder':
        self.capabilities[key] = value  # int, bool, list, dict, etc.
        return self
```

**To-dos**:
- [ ] Update type hints (Dict[str, Any])
- [ ] Update EnvelopeBuilder
- [ ] Add DiscoverableService Protocol
- [ ] Update tests
- [ ] Update CHANGELOG

---

### Phase 6: Go Binding (v0.2.0)

**File**: `umicp/bindings/go/pkg/envelope.go`

```go
type Capabilities map[string]interface{}  // Native types!

type EnvelopeBuilder struct {
    capabilities Capabilities
}

func (b *EnvelopeBuilder) Capability(key string, value interface{}) *EnvelopeBuilder {
    b.capabilities[key] = value  // int, bool, []interface{}, map[string]interface{}
    return b
}
```

**To-dos**:
- [ ] Update Capabilities to map[string]interface{}
- [ ] Update EnvelopeBuilder
- [ ] Add DiscoverableService interface
- [ ] Update tests
- [ ] Update CHANGELOG

---

### Phase 7: Java Binding (v0.2.0)

**File**: `umicp/bindings/java/umicp-core/src/main/java/com/umicp/Envelope.java`

```java
import com.google.gson.JsonElement;

public class Envelope {
    private Map<String, JsonElement> capabilities;  // Native types via Gson!
}

public class EnvelopeBuilder {
    public EnvelopeBuilder capability(String key, JsonElement value) {
        this.capabilities.put(key, value);
        return this;
    }
    
    // Convenience methods
    public EnvelopeBuilder capability(String key, int value) {
        return capability(key, new JsonPrimitive(value));
    }
}
```

**To-dos**:
- [ ] Update to use Gson JsonElement
- [ ] Update EnvelopeBuilder
- [ ] Add DiscoverableService interface
- [ ] Update tests
- [ ] Update CHANGELOG

---

### Phase 8: PHP Binding (v0.2.0)

**File**: `umicp/bindings/php/src/Envelope.php`

```php
class Envelope {
    private array $capabilities = [];  // Native PHP arrays support all JSON types!
}

class EnvelopeBuilder {
    public function capability(string $key, mixed $value): self {
        $this->capabilities[$key] = $value;  // int, bool, array, object
        return $this;
    }
}
```

**To-dos**:
- [ ] Update to use `mixed` type (PHP 8+)
- [ ] Update EnvelopeBuilder
- [ ] Add DiscoverableService interface
- [ ] Update tests
- [ ] Update CHANGELOG

---

### Phase 9: C# Binding (v1.1.0)

**File**: `umicp/bindings/csharp/Umicp.Core/Envelope.cs`

```csharp
using System.Text.Json;

public class Envelope {
    public Dictionary<string, JsonElement>? Capabilities { get; set; }  // Native types via System.Text.Json!
}

public class EnvelopeBuilder {
    public EnvelopeBuilder Capability(string key, JsonElement value) {
        Capabilities[key] = value;
        return this;
    }
    
    // Convenience
    public EnvelopeBuilder Capability(string key, int value) {
        return Capability(key, JsonDocument.Parse(value.ToString()).RootElement);
    }
}
```

**To-dos**:
- [ ] Update to use System.Text.Json
- [ ] Update EnvelopeBuilder
- [ ] Add IDiscoverableService interface
- [ ] Update tests
- [ ] Update CHANGELOG

---

### Phase 10: Kotlin Binding (v0.2.0)

**File**: `umicp/bindings/kotlin/src/main/kotlin/umicp/Envelope.kt`

```kotlin
import kotlinx.serialization.json.*

class Envelope {
    var capabilities: Map<String, JsonElement>? = null  // Native types via kotlinx.serialization!
}

class EnvelopeBuilder {
    fun capability(key: String, value: JsonElement): EnvelopeBuilder {
        capabilities[key] = value
        return this
    }
    
    // Convenience
    fun capability(key: String, value: Int) = capability(key, JsonPrimitive(value))
    fun capability(key: String, value: Boolean) = capability(key, JsonPrimitive(value))
}
```

**To-dos**:
- [ ] Update to use kotlinx.serialization JsonElement
- [ ] Update EnvelopeBuilder
- [ ] Add DiscoverableService interface
- [ ] Update tests
- [ ] Update CHANGELOG

---

### Phase 11: Vectorizer Integration

**ONLY AFTER** all UMICP bindings are updated.

**File**: `vectorizer/Cargo.toml`

```toml
umicp-core = { version = "0.2", features = ["http2", "websocket"] }
```

**File**: `vectorizer/src/umicp/handlers.rs`

```rust
pub async fn handle_umicp_request(...) -> Result<Envelope> {
    let operation = caps.get("operation")
        .and_then(|v| v.as_str())
        .ok_or("Missing operation")?;
    
    match operation {
        "_list_operations" => handle_list_operations(envelope).await,
        "_get_schema" => handle_get_schema(envelope, caps).await,
        "_server_info" => handle_server_info(envelope).await,
        _ => handle_regular_operation(state, envelope, operation, caps).await,
    }
}
```

**To-dos**:
- [ ] Update vectorizer Cargo.toml to umicp-core 0.2
- [ ] Implement _list_operations handler
- [ ] Implement _get_schema handler
- [ ] Implement _server_info handler
- [ ] Update vectorizer tests
- [ ] Create MCP_VS_UMICP_ANALYSIS.md

---

## Timeline (Revised)

### Week 1-2: C++ Core (v1.1.0)
- [ ] Day 1-2: Implement nlohmann/json integration
- [ ] Day 3-4: Update envelope implementation
- [ ] Day 5-6: Add tool discovery interfaces
- [ ] Day 7-10: Tests, examples, documentation

### Week 3: Rust Alignment
- [ ] Review and align Rust 0.2.0 with C++ 1.1.0
- [ ] FFI compatibility verification
- [ ] Cross-language integration tests

### Week 4: TypeScript + Python
- [ ] Day 1-2: TypeScript binding
- [ ] Day 3-4: Python binding
- [ ] Day 5: Testing

### Week 5: Go + Java
- [ ] Day 1-2: Go binding
- [ ] Day 3-4: Java binding
- [ ] Day 5: Testing

### Week 6: PHP + C# + Kotlin
- [ ] Day 1-2: PHP binding
- [ ] Day 3: C# binding
- [ ] Day 4: Kotlin binding
- [ ] Day 5: Testing

### Week 7-8: Vectorizer Integration
- [ ] Week 7: Integration implementation
- [ ] Week 8: Testing, documentation, final review

---

## Success Criteria

### C++ Core (v1.1.0)
- [ ] nlohmann/json integrated successfully
- [ ] All tests passing (241+ tests)
- [ ] Native types work (int, bool, array, object)
- [ ] Tool discovery interfaces defined
- [ ] CHANGELOG complete
- [ ] Examples updated

### All Bindings
- [ ] 100% feature parity with C++ core
- [ ] All tests passing
- [ ] Native types supported
- [ ] Tool discovery implemented
- [ ] Documentation complete

### Vectorizer
- [ ] Tool discovery endpoints working
- [ ] All 40+ MCP tools discoverable via UMICP
- [ ] Tests passing
- [ ] Documentation complete

---

## Current Blockers

1. **AWAITING APPROVAL**: C++ core design document
   - Decision needed: Proceed with nlohmann/json?
   - Alternative: Extend json-c (not recommended)

---

## References

- **Design Document**: `umicp/cpp/docs/DESIGN_NATIVE_TYPES.md`
- **Rust PoC**: Branch `feat/tool-discovery-native-types` (umicp repo)
- **Rust PoC Commit**: `bc00057` (2025-10-16)
- **Rust CHANGELOG**: `umicp/bindings/rust/CHANGELOG.md`

---

**Maintained by**: HiveLLM Team  
**Last Updated**: 2025-10-16  
**Status**: Phase 2 (C++ Core) - Awaiting Approval

