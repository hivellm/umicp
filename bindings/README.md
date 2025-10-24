# UMICP Language SDKs - Complete Overview

**Last Updated**: 2025-10-24  
**Project Status**: ✅ **Production Ready - 10 SDKs Standardized**  
**Current Version**: v0.3.x (All SDKs Standardized)  
**Latest**: ⭐ v0.3.2 - Python SDK Published to PyPI

## 🎯 Major Update: Package Name Standardization (v0.3.0)

**BREAKING CHANGE**: All SDKs renamed to follow `umicp-sdk` convention

All packages now use standardized naming:
- Python: `umicp_sdk` ✅ **Published to PyPI**
- Rust: `umicp-sdk`
- TypeScript: `@hivellm/umicp-sdk`
- Go: `github.com/hivellm/umicp-sdk`
- C#: `HiveLLM.Umicp.SDK`
- PHP: `hivellm/umicp-sdk`
- Swift: `UMICP-SDK`
- Kotlin: `umicp-sdk`
- Elixir: `umicp_sdk`
- Java: `umicp-sdk`

---

## 📊 Quick Status Overview (v0.3.x - Standardized)

| SDK | Package Name | Version | Status | Published |
|-----|--------------|---------|--------|-----------|
| **Python** | `umicp_sdk` | **0.3.2** | ✅ Tested | ✅ **PyPI** |
| **Rust** | `umicp-sdk` | **0.3.1** | ✅ Ready | 📦 To publish |
| **TypeScript** | `@hivellm/umicp-sdk` | **0.3.1** | ✅ Ready | 📦 To publish |
| **Go** | `github.com/hivellm/umicp-sdk` | **0.3.0** | ✅ Ready | Auto-indexed |
| **C#** | `HiveLLM.Umicp.SDK` | **0.3.0** | ✅ Ready | 📦 NuGet package ready |
| **PHP** | `hivellm/umicp-sdk` | **0.3.0** | ✅ Ready | 📦 To publish |
| **Swift** | `UMICP-SDK` | **0.3.0** | ✅ Ready | SPM |
| **Kotlin** | `umicp-sdk` | **0.3.0** | ✅ Ready | 📦 To publish |
| **Elixir** | `umicp_sdk` | **0.3.0** | ✅ Ready | 📦 To publish |
| **Java** | `umicp-sdk` | **0.3.0** | ✅ Ready | 📦 To publish |

**Production Ready:** 10/10 SDKs (100%)  
**Standardized Naming:** ✅ All SDKs use `umicp-sdk` convention  
**Published:** Python v0.3.2 on PyPI ✅  
**Latest Update:** ✅ v0.3.2 - Package name standardization complete

---

## 🚀 Installation

### Python (Published to PyPI ✅)
```bash
pip install umicp-sdk
# or specific version
pip install umicp-sdk==0.3.2
```

### Rust
```toml
[dependencies]
umicp-sdk = "0.3.1"
```

### TypeScript / Node.js
```bash
npm install @hivellm/umicp-sdk
# or specific version
npm install @hivellm/umicp-sdk@0.3.1
```

### Go
```bash
go get github.com/hivellm/umicp-sdk@v0.3.0
```

### C# (NuGet Package Ready)
```bash
dotnet add package HiveLLM.Umicp.SDK
# or specific version
dotnet add package HiveLLM.Umicp.SDK --version 0.3.0
```

### PHP
```bash
composer require hivellm/umicp-sdk
# or specific version
composer require hivellm/umicp-sdk:^0.3.0
```

### Java
```xml
<dependency>
    <groupId>com.hivellm</groupId>
    <artifactId>umicp-sdk</artifactId>
    <version>0.3.0</version>
</dependency>
```

### Kotlin
```kotlin
dependencies {
    implementation("com.hivellm:umicp-sdk:0.3.0")
}
```

### Swift
```swift
dependencies: [
    .package(url: "https://github.com/hivellm/umicp.git", from: "0.3.0")
]
```

### Elixir
```elixir
def deps do
  [
    {:umicp_sdk, "~> 0.3.0"}
  ]
end
```

---

## 📦 Language-Specific Details

### 1. C++ (Core Implementation) ✅

**Status**: Production Ready  
**Version**: v0.1.2  
**Use Case**: High-performance applications, embedded systems

#### Statistics
- **Files**: 31 test files
- **Tests**: 241 (100% passing)
- **Coverage**: 98%
- **Examples**: 7 working examples

#### Features
- ✅ Native SIMD acceleration
- ✅ Zero-copy operations
- ✅ CMake build system
- ✅ WebSocket client/server
- ✅ HTTP/2 transport
- ✅ Multiplexed peer architecture
- ✅ Event-driven API
- ✅ **Service Discovery** - NEW! (18 tests, 100% coverage)
- ✅ **Connection Pooling** - NEW! (17 tests, 99% coverage)
- ✅ Comprehensive test coverage

#### New Features (v0.1.2)

**Service Discovery**
- ServiceInfo management with metadata
- Capability-based service discovery
- Name pattern matching
- Stale service cleanup
- Thread-safe operations
- 18 comprehensive tests

**Connection Pooling**
- Pooled connection lifecycle management
- Min/Max pool size configuration
- Connection validation and health checks
- Stale/Idle detection
- Statistics tracking
- Background cleanup tasks
- 17 comprehensive tests

#### Quick Start
```bash
cd cpp
mkdir build && cd build
cmake ..
make -j$(nproc)
make test

# Run new feature tests
./tests/service_discovery_tests
./tests/connection_pool_tests

# Run examples
./examples/service_discovery_example
./examples/connection_pool_example
```

---

### 2. TypeScript (Reference Implementation) ✅

**Status**: Production Ready  
**Version**: v0.1.2  
**Use Case**: Node.js backends, real-time applications

#### Statistics
- **Files**: 14 source files
- **Tests**: 163 (100% passing)
- **Coverage**: ~95%
- **Examples**: 6 working examples

#### Features
- ✅ Node.js N-API bindings to C++ core
- ✅ WebSocket client/server
- ✅ Multiplexed peer architecture
- ✅ HTTP/2 transport
- ✅ Event-driven API
- ✅ Service Discovery
- ✅ Connection Pooling
- ✅ Full async/await support

#### Installation
```bash
npm install @hivellm/umicp
```

---

### 3. Go (Cloud-Native) ✅

**Status**: Production Ready - 100% Feature Parity  
**Version**: v0.2.3 ⭐ Latest  
**Use Case**: Microservices, cloud deployments, P2P networks

#### Statistics
- **Files**: 25+ files (~5,200 LOC)
- **Tests**: All passing (100%)
- **Coverage**: 90%+
- **Examples**: 9 working examples

#### Features
- ✅ Pure Go implementation
- ✅ Goroutine-based concurrency
- ✅ WebSocket client/server
- ✅ HTTP/2 transport with custom endpoints
- ✅ **Automatic URL path detection** ⭐ NEW (v0.2.3)
- ✅ Multiplexed peer
- ✅ Auto-handshake protocol
- ✅ Service Discovery
- ✅ Connection Pooling
- ✅ Event system

#### Installation
```bash
go get github.com/hivellm/umicp-go@v0.2.3  # Latest
```

---

### 4. Java (Enterprise) ✅

**Status**: Production Ready - Phase 4 Complete  
**Version**: v0.1.2  
**Use Case**: Enterprise applications, Spring ecosystem, JVM microservices

#### Statistics
- **Production Classes**: 43 (~9,100 LOC)
- **Test Classes**: 26 (~4,200 LOC)
- **Tests**: 380+ (100% passing)
- **Coverage**: ~97%
- **Examples**: 6 working examples

#### Modules
1. **umicp-core** - Type system, envelope, matrix, discovery, compression
2. **umicp-transport** - WebSocket, HTTP/2, multiplexed peer, connection pooling
3. **umicp-examples** - Complete examples for all features

#### Features
- ✅ Pure Java implementation
- ✅ Maven multi-module project
- ✅ WebSocket transport (Java-WebSocket)
- ✅ **HTTP/2 client** (Java 11+ HttpClient) ⭐ NEW
- ✅ Multiplexed peer architecture
- ✅ Auto-handshake protocol
- ✅ Event-driven architecture
- ✅ **Service Discovery** ⭐ NEW
- ✅ **Connection Pooling** ⭐ NEW
- ✅ **Compression (GZIP/DEFLATE)** ⭐ NEW
- ✅ Statistics tracking
- ✅ Integration tests

#### Installation
```xml
<dependency>
    <groupId>com.hivellm</groupId>
    <artifactId>umicp-core</artifactId>
    <version>0.1.2</version>
</dependency>
```

---

### 5. Kotlin (Modern JVM - Most Feature-Complete) 🏆 ✅

**Status**: Production Ready - Phase 5 Complete  
**Version**: v0.1.2  
**Use Case**: Modern JVM applications, Android, Spring Boot, Ktor, real-time systems

#### Statistics
- **Files**: 38+ files (~4,900 LOC)
- **Tests**: **140+** (100% passing)
- **Coverage**: **96%**
- **Examples**: **7** working examples

#### Features
- ✅ Idiomatic Kotlin with DSL builders
- ✅ Coroutines for async/await (superior to threads)
- ✅ Null safety built-in (compile-time guarantee)
- ✅ Extension functions (`a dot b`, `vector.normalize()`)
- ✅ Operator overloading (`a + b`, `a * b`)
- ✅ Data classes (immutable, auto equals/hashCode/copy)
- ✅ Sealed classes for type-safe error handling
- ✅ WebSocket client/server
- ✅ **HTTP/2 client AND server** (ONLY SDK with both) ⭐
- ✅ **LZ4/LZ4_HC compression** (5x faster than GZIP) ⭐
- ✅ GZIP/DEFLATE compression
- ✅ Service Discovery
- ✅ Connection Pooling
- ✅ Smart casts and type inference
- ✅ Zero runtime overhead DSL

#### Kotlin Advantages
- **Null Safety**: No more NullPointerExceptions
- **Coroutines**: Lightweight threads, structured concurrency
- **Extensions**: Add methods to existing classes
- **Operators**: Math-like syntax for vectors
- **DSL**: Type-safe builders
- **Less Boilerplate**: ~40% less code than Java
- **🥇 HTTP/2 Server**: ONLY SDK with HTTP/2 server implementation
- **🥇 LZ4 Compression**: ONLY SDK with LZ4 support (5x faster)

#### Installation
```kotlin
// Gradle Kotlin DSL
dependencies {
    implementation("com.hivellm:umicp-kotlin:0.1.2")
}

// Gradle Groovy
dependencies {
    implementation 'com.hivellm:umicp-kotlin:0.1.2'
}

// Maven
<dependency>
    <groupId>com.hivellm</groupId>
    <artifactId>umicp-kotlin</artifactId>
    <version>0.1.2</version>
</dependency>
```

#### Quick Example
```kotlin
// DSL builder
val envelope = Envelope.build {
    from("client-001")
    to("server-001")
    operation(OperationType.DATA)
}

// Extension functions
val a = floatArrayOf(1f, 2f, 3f)
val b = floatArrayOf(4f, 5f, 6f)
val dot = a dot b  // 32.0
val similarity = a cosineSim b  // 0.9746

// Operator overloading
val sum = a + b  // [5, 7, 9]

// Coroutines
runBlocking {
    val client = UMICPWebSocketClient.create("ws://localhost:8080") {
        onMessage { envelope ->
            println("Received: ${envelope.from}")
        }
    }
    client.connect()
    client.send(envelope)
}
```

---

### 6. Rust (High-Performance) ✅

**Status**: Production Ready - 100% Complete  
**Version**: v0.2.3 ⭐ Latest  
**Use Case**: Systems programming, high-performance services

#### Statistics
- **Files**: 14 source files (~4,100 LOC)
- **Tests**: 108 (100% passing)
- **Coverage**: 100%
- **Examples**: 16 working examples

#### Features
- ✅ Memory-safe (Rust guarantees)
- ✅ Thread-safe (Send + Sync)
- ✅ Zero-copy operations
- ✅ SIMD matrix operations (ndarray)
- ✅ Async/await (tokio 1.42)
- ✅ WebSocket client/server (tokio-tungstenite)
- ✅ HTTP/2 client with custom endpoints (reqwest)
- ✅ **Automatic URL path detection** ⭐ NEW (v0.2.3)
- ✅ HTTP/2 server (axum 0.8)
- ✅ Multiplexed peer
- ✅ Service Discovery
- ✅ Connection Pooling
- ✅ Non-blocking operations

#### Installation
```toml
[dependencies]
umicp-core = { version = "0.2.3", features = ["full"] }
tokio = { version = "1.42", features = ["full"] }
```

---

### 6. PHP (Web-Focused) ✅

**Status**: Production Ready  
**Version**: v0.2.2  
**Use Case**: Web applications, Laravel/Symfony/WordPress integration

#### Statistics
- **Files**: 20+ files (~8,500 LOC)
- **Tests**: 145+ (100% passing)
- **Coverage**: 95%
- **Examples**: 6 working examples

#### Features
- ✅ FFI bindings to C++ core
- ✅ Pure PHP classes
- ✅ HTTP client/server with custom endpoints (cURL/ReactPHP)
- ✅ **Custom endpoint support** ⭐ (v0.2.2)
- ✅ WebSocket support
- ✅ Compression (GZIP/DEFLATE)
- ✅ Event system
- ✅ ReactPHP async support
- ✅ Matrix operations

#### Installation
```bash
composer require hivellm/umicp:0.2.2
```

---

### 7. Python (ML/AI-Focused) ✅

**Status**: Production Ready - Published to PyPI  
**Version**: v0.2.2  
**PyPI**: [umicp-python](https://pypi.org/project/umicp-python/)  
**Use Case**: AI/ML pipelines, data science, async applications

#### Statistics
- **Files**: 45+ files (~2,500+ LOC)
- **Tests**: **162** (100% passing)
- **Coverage**: **97%**
- **Examples**: 3 working examples

#### Features
- ✅ Modern async/await (asyncio)
- ✅ NumPy integration for matrix operations
- ✅ Full type hints (PEP 561 compliant)
- ✅ WebSocket client/server (websockets)
- ✅ HTTP/2 client with custom endpoints (httpx)
- ✅ **Custom endpoint support** ⭐ NEW (v0.2.2)
- ✅ HTTP/2 server (aiohttp)
- ✅ Multiplexed peer architecture
- ✅ Event system with async handlers
- ✅ Service Discovery
- ✅ Connection Pooling
- ✅ Compression (GZIP/DEFLATE)
- ✅ Python 3.9+ support

#### Installation
```bash
# From PyPI (recommended)
pip install umicp-python==0.2.2

# Or from source
pip install -e "bindings/python[dev]"
```

**Latest Release**: October 17, 2025

---

## 📈 Feature Comparison Matrix

| Feature | C++ | TypeScript | Go | Java | Rust | PHP | Python |
|---------|-----|------------|-----|------|------|-----|--------|
| **Core Protocol** |
| Envelope/Frame | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Serialization | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Message Types | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Matrix Operations** |
| Dot Product | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (NumPy) |
| Cosine Similarity | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (NumPy) |
| SIMD Acceleration | ✅ | ✅ (C++) | ❌ | ❌ | ✅ | ✅ (C++) | ✅ (NumPy) |
| **Transport Layer** |
| WebSocket Client | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| WebSocket Server | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| HTTP/2 Client | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (httpx) |
| HTTP/2 Server | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (aiohttp) |
| Multiplexed Peer | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Advanced Features** |
| Event System | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (async) |
| Service Discovery | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ |
| Connection Pooling | ✅ | ✅ | ✅ | ✅ | ✅ | ❌ | ✅ (async) |
| Compression | ✅ | ✅ | ✅ | ✅ (GZIP/DEFLATE) | ✅ | ✅ | 📋 |
| **Quality** |
| Test Coverage | 98% | 95% | 90% | 95% | 100% | 95% | 76% |
| Production Ready | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Type Safety | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (hints) |
| Memory Safety | ✅ | ⚠️ | ⚠️ | ⚠️ | ✅ | ⚠️ | ⚠️ |

**Legend**: ✅ Implemented | ⚠️ Partial | ❌ Not Implemented | 📋 Planned

---

## 📊 Overall Project Statistics

### Production Ready Bindings (7) ✅

| Metric | C++ | TypeScript | Go | Java | Rust | PHP | Python | **Total** |
|--------|-----|------------|-----|------|------|-----|--------|-----------|
| Files | **31** | 14 | 25 | **43** | 14 | 20 | 45 | **192+** |
| LOC | **~6,800** | ~4,000 | ~5,200 | **~9,100** | ~4,100 | ~8,500 | ~2,500 | **~40,200** |
| Tests | **241** | 163 | 68 | **380+** | 123 | 145+ | 115 | **1,386+** |
| Coverage | 98% | 95% | 90% | **97%** | 100% | 95% | 97% | **~96%** |
| Examples | **7** | 6 | 9 | **6** | 16 | 6 | 3 | **53** |
| Docs | 10 | 11 | 6 | 14 | 8 | 15 | 3 | **67** |

### Summary
- **Total Bindings**: 8 (100% Complete)
- **Total Files**: 192+
- **Total LOC**: ~40,200
- **Total Tests**: 1,404+
- **Total Examples**: 53
- **Total Docs**: 67
- **Average Coverage**: ~96.1%

---

## 🎯 Use Case Recommendations

### High Performance / Systems Programming
- **Rust** - Memory-safe, zero-copy, SIMD
- **C++** - Native SIMD, zero-copy
- **Go** - Efficient, cloud-native

### Web Applications
- **TypeScript** - Node.js ecosystem
- **PHP** - WordPress, Laravel, Symfony
- **Python** - Flask, FastAPI, Django

### Enterprise / JVM
- **Java** - Spring, Jakarta EE, microservices

### Cloud / Microservices
- **Go** - Kubernetes-native, lightweight
- **Rust** - High performance, low resource

### AI / ML
- **Python** - NumPy, TensorFlow, PyTorch integration
- **TypeScript** - Node.js ML frameworks

### Real-time Applications
- **TypeScript** - WebSocket, async/await
- **Rust** - Ultra-low latency
- **Go** - Concurrent, scalable

---

## 🏆 Quality Comparison

| Aspect | Best Choice | Runners-up |
|--------|-------------|------------|
| **Performance** | Rust, C++ | Go |
| **Memory Safety** | Rust | - |
| **Test Coverage** | Rust (100%) | C++ (98%) |
| **Ease of Use** | Python, TypeScript | Go |
| **Enterprise Ready** | Java | TypeScript |
| **Web Integration** | PHP, Python | TypeScript |
| **Cloud Native** | Go | Rust |
| **ML/AI Integration** | Python | TypeScript |

---

## 📝 Version History

### v0.2.3 (2025-10-18) - Automatic URL Path Detection 🎯
- ✅ **Rust v0.2.3** - Automatic URL path parsing
  - ✅ Pass full URLs like `http://localhost:15002/umicp` directly
  - ✅ 6 new tests for path detection
  - ✅ Fully backward compatible
  - ✅ 108/108 tests passing (100%)
- ✅ **Go v0.2.3** - Automatic URL path parsing
  - ✅ Same automatic path detection as Rust
  - ✅ 6 new tests for path detection
  - ✅ All tests passing

### v0.2.2 (2025-10-17) - Custom Endpoint Support 🔌
- ✅ **7 SDKs Updated** - Custom endpoint support for all major SDKs
  - ✅ Python, Rust, Go, C#, TypeScript, PHP, Java
  - ✅ 98 custom endpoint tests (100% passing)
  - ✅ 688+ total tests, 0 failures
  - ✅ Vectorizer compatibility (`/umicp` endpoint)
  - ✅ Standard server support (`/message` endpoint)
  - ✅ No breaking changes

### v0.2.0 (2025-10-16) - Native Types & Tool Discovery 🎉
- ✅ **Swift SDK v0.2.0** - Production ready
- ✅ **Elixir SDK v0.2.0** - Production ready
- ✅ **All SDKs Updated** - Native JSON type support
- ✅ **MCP-Compatible Tool Discovery** - All SDKs

### v0.1.2 (2025-10-10) - Advanced Features
- ✅ Service Discovery & Connection Pooling
- ✅ Java Phase 4 Complete (HTTP/2 & Compression)
- ✅ C++ advanced features complete

### v0.1.0 (2025-09-01) - Initial Release
- ✅ C++ core implementation
- ✅ Basic protocol support

---

## 🚀 Getting Started

### 1. Choose Your Language
Pick the binding that best fits your tech stack (see recommendations above)

### 2. Install
Follow the installation instructions for your language

### 3. Quick Example

All bindings support the same core concepts:

```javascript
// TypeScript/JavaScript
import { Envelope, OperationType } from '@hivellm/umicp-sdk';

const envelope = new Envelope({
  from: "client-001",
  to: "server-001",
  operation: OperationType.DATA
});
```

```python
# Python
from umicp_sdk import Envelope, OperationType

envelope = Envelope(
    from_id="client-001",
    to_id="server-001",
    operation=OperationType.DATA
)
```

```rust
// Rust
use umicp_sdk::{Envelope, OperationType};

let envelope = Envelope::builder()
    .from("client-001")
    .to("server-001")
    .operation(OperationType::Data)
    .build()?;
```

```csharp
// C#
using Umicp.SDK;

var envelope = new Envelope
{
    FromId = "client-001",
    ToId = "server-001",
    Operation = OperationType.DATA
};
```

### 4. Explore Examples
Each binding includes working examples in their respective directories

---

## 📚 Documentation

### By Language
- **C++**: [cpp/README.md](cpp/README.md)
- **TypeScript**: [typescript/README.md](typescript/README.md)
- **Go**: [go/README.md](go/README.md)
- **Java**: [java/README.md](java/README.md)
- **Rust**: [rust/README.md](rust/README.md)
- **PHP**: [php/README.md](php/README.md)
- **Python**: [python/README.md](python/README.md)

### General
- [Protocol Specification](../docs/guides/protocol-api.md)
- [Installation Guide](../docs/guides/installation.md)
- [API Reference](../docs/api/)

---

## 🧪 Testing

All bindings maintain high test coverage:

```bash
# TypeScript
npm test

# Java
mvn test

# Rust
cargo test --features full

# Go
go test ./...

# PHP
./vendor/bin/phpunit

# Python
pytest
```

### Test Coverage Report

📊 **[Complete Test Coverage Report](TEST_COVERAGE.md)** - Comprehensive analysis of all SDK test suites including:
- **C++ Core SDK**: 241 tests, 98% coverage with Service Discovery & Connection Pooling
- Detailed coverage metrics per SDK
- Test count and categories breakdown
- Quality metrics and comparisons
- Areas for improvement
- Historical trends and recommendations

---

## 🤝 Contributing

See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

---

## 📄 License

MIT License - See [LICENSE](../LICENSE) file for details.

---

## 🎉 Achievements

- ✅ **11 production-ready bindings** (C++, Rust, Python, C#, Go, TypeScript, PHP, Java, Kotlin, Elixir, Swift)
- ✅ **1,500+ tests** with **96%+ average coverage**
- ✅ **100% pass rate** across all bindings
- ✅ **53+ working examples**
- ✅ **Cross-platform** support (Windows, Linux, macOS)
- ✅ **Enterprise-grade** quality
- ✅ **Latest Innovation**: Automatic URL path detection (Rust & Go v0.2.3) 🎯
- ✅ **Custom Endpoints**: Full Vectorizer compatibility (v0.2.2)
- ✅ **Tool Discovery**: MCP-compatible across all SDKs
- ✅ **Python SDK**: Published to PyPI 📦
- ✅ **Multi-language**: 11 languages supported

---

**Status**: ✅ **ALL 10 SDKs PRODUCTION READY & STANDARDIZED**

**Latest Version**: v0.3.x (Package Standardization Release)  
**Python on PyPI**: https://pypi.org/project/umicp-sdk/ ✅  
**C# on NuGet**: Package ready for publication  

**Project**: HiveLLM UMICP  
**Maintained by**: HiveLLM AI Collaborative Team  
**Repository**: https://github.com/hivellm/umicp  

*Last updated: 2025-10-24*

