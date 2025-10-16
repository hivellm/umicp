# UMICP SDK Review for v0.2.0

**Date**: 2025-10-16  
**Status**: Review Phase

---

## 🎯 Objective

Review all 7 language bindings to determine what needs updating for v0.2.0 native type support.

---

## 📊 Quick Analysis Results

### ✅ Already Support Native Types (2/7)

| Language | Current Version | Capabilities Type | Status | Effort |
|----------|----------------|-------------------|--------|--------|
| **Python** | 0.1.3 | `Dict[str, Any]` | ✅ Ready | 2h (add discovery) |
| **C#** | 1.0.0 | `Dictionary<string, object>` | ✅ Ready | 2h (add discovery) |

### ⚠️ Need Updates (5/7)

| Language | Current Version | Current Type | Target Type | Effort |
|----------|----------------|--------------|-------------|--------|
| **TypeScript** | 0.1.5 | `Record<string, string>` | `Record<string, any>` | 4-6h |
| **Go** | 0.1.3 | `map[string]string` | `map[string]interface{}` | 4-6h |
| **PHP** | 0.1.3 | TBD | `mixed` values | 3-4h |
| **Java** | 0.1.3 | TBD | `Map<String, JsonElement>` | 6-8h |
| **Kotlin** | 0.1.3 | TBD | `Map<String, JsonElement>` | 5-7h |

---

## 📋 Detailed Analysis

### 1. Python ✅ (Minimal Changes)

**Current State**:
```python
# umicp/bindings/python/umicp/envelope.py:24
capabilities: Optional[Dict[str, Any]] = None
```

**✅ Already uses `Any`!** Native types work out of the box.

**What's Needed**:
- [x] Capabilities type: Already supports native types
- [ ] Tool discovery: Add `DiscoverableService` Protocol
- [ ] Tests: Add tests for discovery
- [ ] Version: Bump to 0.2.0
- [ ] CHANGELOG: Document discovery addition

**Estimated Effort**: 2 hours

**Files to Update**:
- `umicp/discovery.py` - Already exists! Need to add DiscoverableService
- `pyproject.toml` - Version 0.1.3 → 0.2.0
- `CHANGELOG.md` - Add v0.2.0 entry
- `tests/` - Add discovery tests

---

### 2. C# ✅ (Minimal Changes)

**Current State**:
```csharp
// Umicp.Core/Envelope.cs:57
public Dictionary<string, object>? Capabilities { get; set; }
```

**✅ Already uses `object`!** Native types work with System.Text.Json.

**What's Needed**:
- [x] Capabilities type: Already supports native types
- [ ] Tool discovery: Add `IDiscoverableService` interface
- [ ] Tests: Add tests for discovery
- [ ] Version: Bump to 0.2.0 (or 1.1.0)
- [ ] CHANGELOG: Document discovery addition

**Estimated Effort**: 2 hours

**Files to Update**:
- `Umicp.Core/ToolDiscovery.cs` - NEW file
- `Umicp.Core.csproj` - Version update
- `CHANGELOG.md` - Add v0.2.0 entry
- `Umicp.Tests/ToolDiscoveryTests.cs` - NEW file

---

### 3. TypeScript ❌ (Needs Update)

**Current State**:
```typescript
// src/index.ts:276
setCapabilities(capabilities: Record<string, string>): Envelope
```

**❌ Uses `Record<string, string>`** - Only strings!

**What's Needed**:
- [ ] Capabilities type: `Record<string, string>` → `Record<string, any>`
- [ ] Update all capability-related methods
- [ ] Tool discovery: Add interfaces
- [ ] Tests: Add native type tests
- [ ] Version: 0.1.5 → 0.2.0

**Estimated Effort**: 4-6 hours

**Files to Update**:
- `src/index.ts` - Update Capabilities type
- `src/types.ts` - Add discovery interfaces
- `test/` - Add native type tests
- `package.json` - Version 0.1.5 → 0.2.0
- `CHANGELOG.md` - Document breaking changes

**Breaking Change**: Yes - API signature changes

---

### 4. Go ❌ (Needs Update)

**Current State**:
```go
// pkg/umicp/envelope.go:19
Capabilities map[string]string `json:"capabilities"`
```

**❌ Uses `map[string]string`** - Only strings!

**What's Needed**:
- [ ] Capabilities type: `map[string]string` → `map[string]interface{}`
- [ ] Update EnvelopeBuilder methods
- [ ] Tool discovery: Add interfaces
- [ ] Tests: Add native type tests
- [ ] Version: Bump to 0.2.0

**Estimated Effort**: 4-6 hours

**Files to Update**:
- `pkg/umicp/envelope.go` - Update Capabilities type
- `pkg/umicp/discovery.go` - NEW file
- `test/` - Add native type tests
- `go.mod` - Version update
- `README.md` - Update examples

**Breaking Change**: Yes - struct field type changes

---

### 5. PHP (Need to Check)

**Version**: 0.1.3

**Estimated Effort**: 3-4 hours

**Need to review**:
- Current Capabilities implementation
- PHP native type support (arrays support mixed naturally)

---

### 6. Java (Need to Check)

**Version**: 0.1.3

**Estimated Effort**: 6-8 hours

**Need to review**:
- Current Capabilities implementation
- Likely needs Gson/Jackson JsonElement

---

### 7. Kotlin (Need to Check)

**Version**: 0.1.3

**Estimated Effort**: 5-7 hours

**Need to review**:
- Current Capabilities implementation
- Likely uses kotlinx.serialization

---

## 🎯 Recommended Implementation Order

### Phase 1: Easy Wins (4-6 hours total)
1. **Python** (2h) - Add discovery Protocol
2. **C#** (2h) - Add IDiscoverableService interface

### Phase 2: Medium Effort (8-12 hours total)
3. **TypeScript** (4-6h) - Update to `any`, add discovery
4. **Go** (4-6h) - Update to `interface{}`, add discovery

### Phase 3: Complex (14-19 hours total)
5. **PHP** (3-4h) - Review and update
6. **Kotlin** (5-7h) - JsonElement integration
7. **Java** (6-8h) - Gson/Jackson integration

---

## 📝 Standard Updates for Each Binding

### 1. Capabilities Type Update
- Change from string-only to native types
- Update builder/setter methods
- Update serialization

### 2. Tool Discovery Interfaces
- `OperationSchema` struct/class
- `ServerInfo` struct/class
- `DiscoverableService` interface/trait/protocol

### 3. Tests
- Minimum 5 new tests for native types
- 3+ tests for discovery
- Ensure existing tests still pass

### 4. Documentation
- CHANGELOG with breaking changes
- README with new examples
- Migration guide

---

## ✅ Success Criteria

For each binding to be considered "updated to v0.2.0":

- [ ] Capabilities support int, bool, double, array, object
- [ ] DiscoverableService interface defined
- [ ] All existing tests passing
- [ ] New tests for v0.2.0 features (min 8 tests)
- [ ] CHANGELOG.md updated
- [ ] Version bumped
- [ ] Examples updated

---

## 🚀 Next Actions

1. **Start with Python** (easiest, already has native types)
2. **Then C#** (already has native types)
3. **Continue with TypeScript and Go** (medium effort)
4. **Finish with PHP, Kotlin, Java** (most complex)

---

**Estimated Total Time**: 30-40 hours  
**Can Parallelize**: Yes (different languages)  
**Priority**: Python → C# → TypeScript → Go → Others

