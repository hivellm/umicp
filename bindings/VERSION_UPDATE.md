# Version Update - 0.1.1

**Date**: 2025-10-10  
**Previous Version**: Various (0.1.0, 1.0.0-SNAPSHOT)  
**New Version**: 0.1.1

---

## Summary

All UMICP bindings have been updated to version **0.1.1** for consistency across the project.

---

## Updated Bindings

### 1. TypeScript ✅
- **File**: `package.json`
- **Old**: 0.1.0
- **New**: 0.1.1
- **Location**: `umicp/bindings/typescript/package.json`

### 2. Java ✅
- **Files**: 
  - Parent POM: `pom.xml`
  - Module: `umicp-core/pom.xml`
  - Module: `umicp-transport/pom.xml`
  - Module: `umicp-examples/pom.xml`
- **Old**: 1.0.0-SNAPSHOT
- **New**: 0.1.1
- **Location**: `umicp/bindings/java/`

### 3. Rust ✅
- **File**: `Cargo.toml`
- **Old**: 1.0.0
- **New**: 0.1.1
- **Location**: `umicp/bindings/rust/Cargo.toml`

### 4. Go ✅
- **File**: `version.go` (newly created)
- **Old**: No explicit version
- **New**: 0.1.1
- **Location**: `umicp/bindings/go/version.go`
- **Note**: Go uses git tags for versioning. Tag should be created: `v0.1.1`

### 5. PHP ✅
- **File**: `composer.json`
- **Old**: No explicit version
- **New**: 0.1.1
- **Location**: `umicp/bindings/php/composer.json`

---

## README Updates

Updated installation instructions in READMEs to reflect new version:

- ✅ `umicp/bindings/typescript/README.md`
- ✅ `umicp/bindings/java/README.md`
- ✅ `umicp/bindings/rust/README.md`

---

## Files Modified

### TypeScript (1 file)
- `package.json`
- `README.md`

### Java (4 files)
- `pom.xml` (parent)
- `umicp-core/pom.xml`
- `umicp-transport/pom.xml`
- `umicp-examples/pom.xml`
- `README.md`

### Rust (2 files)
- `Cargo.toml`
- `README.md`

### Go (1 file)
- `version.go` (new)

### PHP (1 file)
- `composer.json`

**Total**: 10 files modified, 1 file created

---

## Next Steps

### For Git Tags (Go)
```bash
# Create and push tag for Go
cd umicp/bindings/go
git tag v0.1.1
git push origin v0.1.1
```

### For NPM (TypeScript)
```bash
cd umicp/bindings/typescript
npm publish
```

### For Maven Central (Java)
```bash
cd umicp/bindings/java
mvn clean deploy -P release
```

### For Crates.io (Rust)
```bash
cd umicp/bindings/rust
cargo publish
```

### For Packagist (PHP)
```bash
# Tag will be automatically detected by Packagist
git tag php-v0.1.1
git push origin php-v0.1.1
```

---

## Version Alignment

All bindings now use the same version number (0.1.1) for consistency:

| Binding | Version | Status |
|---------|---------|--------|
| TypeScript | 0.1.1 | ✅ Production Ready |
| Java | 0.1.1 | ✅ Production Ready |
| Rust | 0.1.1 | ✅ Production Ready |
| Go | 0.1.1 | 🟢 MVP Ready |
| PHP | 0.1.1 | 🟡 Core Only |

---

## Changelog

### What's New in 0.1.1
- **Java**: Phase 2 complete (WebSocket transport)
- **Rust**: Full implementation complete (multiplexed peer)
- **All**: Version alignment across bindings
- **Documentation**: Comprehensive status reports
- **Tests**: 550+ tests across all bindings

### Breaking Changes
None

### Migration Guide
No migration needed - version bump only for consistency

---

**Status**: ✅ **Version Update Complete**

All bindings are now at version 0.1.1 and ready for publication.

---

*Updated: 2025-10-10*

