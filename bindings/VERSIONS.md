# UMICP Bindings - Version Information

**Current Version**: 0.1.1  
**Last Updated**: 2025-10-10

---

## Version by Binding

| Binding | Version | Package Manager | Status |
|---------|---------|-----------------|--------|
| **TypeScript** | 0.1.1 | NPM | ✅ Production |
| **Java** | 0.1.1 | Maven Central | ✅ Production |
| **Rust** | 0.1.1 | Crates.io | ✅ Production |
| **Go** | 0.1.1 | Go Modules | 🟢 MVP |
| **PHP** | 0.1.1 | Packagist | 🟡 Core Only |

---

## Installation Commands

### TypeScript
```bash
npm install @hivellm/umicp@0.1.1
```

### Java
```xml
<dependency>
    <groupId>com.hivellm</groupId>
    <artifactId>umicp-core</artifactId>
    <version>0.1.1</version>
</dependency>
```

### Rust
```toml
[dependencies]
umicp-core = "0.1.1"
```

### Go
```bash
go get github.com/hivellm/umicp-go@v0.1.1
```

### PHP
```bash
composer require hivellm/umicp:0.1.1
```

---

## Version Files

### TypeScript
- `package.json` - Line 3: `"version": "0.1.1"`

### Java
- `pom.xml` - Line 10: `<version>0.1.1</version>`
- `umicp-core/pom.xml` - Parent version: `0.1.1`
- `umicp-transport/pom.xml` - Parent version: `0.1.1`
- `umicp-examples/pom.xml` - Parent version: `0.1.1`

### Rust
- `Cargo.toml` - Line 3: `version = "0.1.1"`

### Go
- `version.go` - Constant: `Version = "0.1.1"`

### PHP
- `composer.json` - Line 3: `"version": "0.1.1"`

---

## Git Tags

To publish versions, create and push tags:

```bash
# TypeScript
git tag typescript-v0.1.1
git push origin typescript-v0.1.1

# Java
git tag java-v0.1.1
git push origin java-v0.1.1

# Rust
git tag rust-v0.1.1
git push origin rust-v0.1.1

# Go
git tag go-v0.1.1
git push origin go-v0.1.1

# PHP
git tag php-v0.1.1
git push origin php-v0.1.1
```

---

## Changelog

### v0.1.1 (2025-10-10)

#### Added
- **Java**: Phase 2 complete - WebSocket transport
  - 135 new tests
  - Multiplexed peer support
  - Auto-handshake protocol
  - Integration tests

- **Rust**: Full implementation
  - Multiplexed peer complete
  - Non-blocking server
  - 100% test coverage
  - All integration tests enabled

- **All**: Version alignment across bindings

#### Changed
- Unified version number across all bindings
- Updated documentation with current version
- Production readiness for TypeScript, Java, and Rust

#### Documentation
- Comprehensive status reports
- Final implementation reports
- Cross-binding comparison

---

## Protocol Version

All bindings support **UMICP Protocol Version 1.0**

---

## Compatibility Matrix

| Binding | Min Version | Max Version | Protocol |
|---------|-------------|-------------|----------|
| TypeScript | Node 16+ | Latest | 1.0 |
| Java | Java 17+ | Java 21+ | 1.0 |
| Rust | 1.70+ | Latest | 1.0 |
| Go | Go 1.21+ | Latest | 1.0 |
| PHP | PHP 8.1+ | Latest | 1.0 |

---

## Release Status

### Production Ready ✅
- TypeScript 0.1.1
- Java 0.1.1
- Rust 0.1.1

### MVP Ready 🟢
- Go 0.1.1

### Development 🟡
- PHP 0.1.1

---

**Maintained by**: HiveLLM Team  
**License**: MIT

