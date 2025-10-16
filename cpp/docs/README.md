# UMICP C++ Documentation

**Version**: 0.2.0  
**Last Updated**: 2025-10-16

This directory contains comprehensive documentation for the UMICP C++ core implementation.

---

## 📚 Documentation Index

### 🚀 Getting Started
- **[IMPLEMENTATION_GUIDE.md](IMPLEMENTATION_GUIDE.md)** - How to implement UMICP in your project
- **[TESTING_GUIDE.md](TESTING_GUIDE.md)** - Testing guidelines and examples
- **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)** - Quick API reference

### 🔧 Technical Documentation
- **[HTTP_TRANSPORT.md](HTTP_TRANSPORT.md)** - HTTP/2 transport layer documentation
- **[WEBSOCKET_README.md](WEBSOCKET_README.md)** - WebSocket transport documentation

### ✨ Version 0.2.0 (Current)
- **[FINAL_VERIFICATION_v0.2.0.md](FINAL_VERIFICATION_v0.2.0.md)** - 100% verified, 84/84 tests passing
- **[DESIGN_NATIVE_TYPES.md](DESIGN_NATIVE_TYPES.md)** - Design document for native type support
- **[umicp-tool-discovery.plan.md](umicp-tool-discovery.plan.md)** - Implementation plan (multi-phase)

### 📦 Publishing
- **[PUBLISHING.md](PUBLISHING.md)** - Complete guide for publishing to Conan, vcpkg, Cargo, npm, PyPI, Maven, NuGet

---

## 🎯 Quick Links by Use Case

### "I want to use UMICP in my C++ project"
→ Start with [IMPLEMENTATION_GUIDE.md](IMPLEMENTATION_GUIDE.md)

### "I want to understand how native types work"
→ Read [DESIGN_NATIVE_TYPES.md](DESIGN_NATIVE_TYPES.md)

### "I want to see test results"
→ Check [FINAL_VERIFICATION_v0.2.0.md](FINAL_VERIFICATION_v0.2.0.md)

### "I want to publish/package UMICP"
→ Follow [PUBLISHING.md](PUBLISHING.md)

### "I need quick API reference"
→ Use [QUICK_REFERENCE.md](QUICK_REFERENCE.md)

### "I'm implementing HTTP/WebSocket transport"
→ See [HTTP_TRANSPORT.md](HTTP_TRANSPORT.md) or [WEBSOCKET_README.md](WEBSOCKET_README.md)

---

## 📋 Documentation Organization

```
docs/
├── README.md (this file)           ← You are here
│
├── Getting Started/
│   ├── IMPLEMENTATION_GUIDE.md     ← How to use UMICP
│   ├── TESTING_GUIDE.md            ← How to test
│   └── QUICK_REFERENCE.md          ← API quick ref
│
├── Technical/
│   ├── HTTP_TRANSPORT.md           ← HTTP/2 details
│   └── WEBSOCKET_README.md         ← WebSocket details
│
├── Version 0.2.0/
│   ├── FINAL_VERIFICATION_v0.2.0.md  ← Test results
│   ├── DESIGN_NATIVE_TYPES.md        ← Design doc
│   └── umicp-tool-discovery.plan.md  ← Implementation plan
│
└── Publishing/
    └── PUBLISHING.md                 ← All package managers
```

---

## 📊 Current Status (v0.2.0)

| Component | Status | Tests | Notes |
|-----------|--------|-------|-------|
| **C++ Core** | ✅ Complete | 44/44 | Production ready |
| **Rust Binding** | ✅ Complete | 40/40 | Aligned with C++ |
| **TypeScript** | ⏳ Pending | - | Needs v0.2.0 update |
| **Python** | ⏳ Pending | - | Needs v0.2.0 update |
| **Go** | ⏳ Pending | - | Needs v0.2.0 update |
| **Java** | ⏳ Pending | - | Needs v0.2.0 update |
| **PHP** | ⏳ Pending | - | Needs v0.2.0 update |
| **C#** | ⏳ Pending | - | Needs v0.2.0 update |
| **Kotlin** | ⏳ Pending | - | Needs v0.2.0 update |

---

## 🔄 Recent Changes (v0.2.0)

### BREAKING CHANGES
- Capabilities type changed to support native JSON types
- EnvelopeBuilder API updated
- See `../CHANGELOG.md` for full details

### New Features
- Native type support (int, bool, double, array, object)
- Tool discovery interfaces (MCP-compatible)
- nlohmann/json integration

### Migration
- See `../CHANGELOG.md` section "Migration Guide"
- Example code provided for all changes

---

## 📞 Support

- **Issues**: https://github.com/hivellm/umicp/issues
- **Discussions**: https://github.com/hivellm/umicp/discussions
- **Email**: team@hivellm.org

---

**Maintained by**: HiveLLM Team  
**License**: MIT
