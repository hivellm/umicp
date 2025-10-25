# 🌐 UMICP - Universal Matrix Intelligent Communication Protocol

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![BIP-05](https://img.shields.io/badge/BIP--05-Core%20Complete-green.svg)](https://github.com/hivellm/hive-gov/tree/main/bips/BIP-05)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Multi-Language](https://img.shields.io/badge/SDKs-10%20Languages-orange.svg)](#language-bindings)
[![PyPI](https://img.shields.io/badge/Python-PyPI-blue.svg)](https://pypi.org/project/umicp-sdk/)

> **BIP-05 Implementation** - High-performance communication protocol for AI model interoperability

**Latest Version**: v0.3.x (Package Standardization Release)  
**Published SDKs**: 6/10 (Python, Rust, TypeScript, C#, PHP, Elixir) ✅  
**Status**: All 10 SDKs Standardized & Production Ready

## 📋 What is UMICP?

UMICP enables efficient inter-model communication between AI systems with:

- **🚀 High Performance**: Sub-millisecond latency, >10,000 msg/sec throughput
- **🔒 Secure**: Envelope-based secure communication with capability negotiation
- **📦 Efficient**: Binary protocol with optional compression
- **🌐 Multi-Language**: 10 standardized SDKs - all production-ready (Python, Rust, TypeScript, Go, C#, PHP, Swift, Kotlin, Elixir, Java)
- **⚡ Real-time**: WebSocket transport with Streaming HTTP support
- **🤝 Peer-to-Peer**: True multiplexed architecture - each peer is server AND client

## 🛠️ Quick Start

### Prerequisites
- **C++17** compatible compiler
- **CMake** 3.15+
- **OpenSSL** 1.1.1+

### Build C++ Core

```bash
# Install dependencies
./cpp/install_dependencies.sh

# Build
mkdir build && cd build
cmake ../cpp
make -j$(nproc)

# Test
make test

# Run example
./examples/basic_example
```

## 🔌 Tools & Integrations

### MCP Bridge (Model Context Protocol)

Test UMICP implementations directly from Cursor using the MCP bridge:

```bash
# Install globally
npm install -g @hivellm/umicp2mcp

# Or run with npx
npx @hivellm/umicp2mcp
```

**Configure in Cursor** (`.cursor/mcp.json`):
```json
{
  "mcpServers": {
    "umicp": {
      "command": "npx",
      "args": ["@hivellm/umicp2mcp"]
    }
  }
}
```

**Usage**: Execute UMICP calls from Cursor's AI:
```json
{
  "server": { "port": 8080 },
  "method": "echo",
  "payload": { "message": "Hello, UMICP!" }
}
```

See [`tomcp/README.md`](tomcp/README.md) for complete documentation and examples.

### Language SDKs

#### Installation (v0.3.x - Standardized)

```bash
# Python (Published to PyPI ✅)
pip install umicp-sdk

# Rust
cargo add umicp-sdk

# TypeScript
npm install @hivellm/umicp-sdk

# Go
go get github.com/hivellm/umicp-sdk

# C# (NuGet package ready)
dotnet add package HiveLLM.Umicp.SDK

# PHP
composer require hivellm/umicp-sdk

# Swift
# Add to Package.swift: .package(url: "https://github.com/hivellm/umicp.git", from: "0.3.0")

# Kotlin
# implementation("com.hivellm:umicp-sdk:0.3.0")

# Elixir (Published to Hex.pm ✅)
# Add to mix.exs:
# {:umicp, "~> 0.3.0"}

# Java
# <artifactId>umicp-sdk</artifactId> <version>0.3.0</version>
```

#### Development

```bash
# TypeScript
cd bindings/typescript && npm install && npm test

# Rust  
cd bindings/rust && cargo test

# Python
cd bindings/python && pip install -e ".[dev]" && pytest

# C#
cd bindings/csharp && dotnet test

# Go
cd bindings/go && go test ./...

# PHP
cd bindings/php && composer install && ./vendor/bin/phpunit
```

## 📦 Language SDKs (v0.3.x - Standardized)

### Status Overview

| Language | Package Name | Version | Published | Production Ready |
|----------|--------------|---------|-----------|------------------|
| **Python** | `umicp_sdk` | **0.3.2** | ✅ **PyPI** | ✅ YES |
| **Rust** | `umicp-sdk` | **0.3.1** | ✅ **crates.io** | ✅ YES |
| **TypeScript** | `@hivellm/umicp-sdk` | **0.3.1** | ✅ **npm** | ✅ YES |
| **C#** | `HiveLLM.Umicp.SDK` | **0.3.0** | ✅ **NuGet** | ✅ YES |
| **PHP** | `hivellm/umicp-sdk` | **0.3.0** | ✅ **Packagist** | ✅ YES |
| **Elixir** | `umicp` | **0.3.0** | ✅ **Hex.pm** | ✅ YES |
| **Go** | `github.com/hivellm/umicp-sdk` | **0.3.0** | Auto-indexed | ✅ YES |
| **Swift** | `UMICP-SDK` | **0.3.0** | SPM | ✅ YES |
| **Kotlin** | `umicp-sdk` | **0.3.0** | 📦 To publish | ✅ YES |
| **Java** | `umicp-sdk` | **0.3.0** | 📦 To publish | ✅ YES |

**Summary**: 10/10 SDKs production-ready & standardized (100%) | **Published**: 6/10 (Python, Rust, TypeScript, C#, PHP, Elixir) ✅

**v0.3.0 Major Update**: Package name standardization across all SDKs

**Legend**: ✅ Published/Ready | 📦 Package ready to publish

### Detailed Feature Matrix

| Feature | C++ | TypeScript | Go | PHP | Rust | Java | Python | C# | Swift | Elixir | Kotlin |
|---------|-----|------------|----|----|------|------|--------|-----|-------|--------|--------|
| **Core Protocol** |
| Envelope/Frame | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Serialization (JSON/Binary) | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (JSON) | ✅ (JSON) | ✅ (JSON) | ✅ (JSON) | ✅ (JSON) |
| Message Types | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Payload Types | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **v0.2.0 Features** |
| Native JSON Types | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Tool Discovery (MCP) | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Matrix Operations** |
| Dot Product | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (NumPy) | ✅ | ✅ | ✅ | ✅ |
| Cosine Similarity | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (NumPy) | ✅ | ✅ | ✅ | ✅ |
| Matrix Multiply | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (NumPy) | ✅ | ✅ | ✅ | ✅ |
| SIMD Acceleration | ✅ | ✅ (via C++) | ❌ | ✅ (via C++) | ✅ (ndarray) | ❌ | ✅ (NumPy) | ✅ | ✅ (Accelerate) | ⚠️ (optional Nx) | ❌ |
| **Transport Layer** |
| WebSocket Client | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (websockets) | ✅ | ✅ (URLSession) | ✅ (Mint) | ✅ |
| WebSocket Server | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (websockets) | ✅ | ✅ | ✅ (GenServer) | ✅ |
| HTTP Client | ✅ | ✅ | ✅ | ✅ | ✅ (HTTP/2) | ✅ | ✅ (httpx) | ✅ | ✅ (HTTP/2) | ✅ (Finch) | ✅ |
| HTTP Server | ✅ | ✅ | ✅ | ✅ | ✅ (HTTP/2) | ✅ | ✅ (aiohttp) | ✅ | ✅ | ✅ (Finch) | ✅ |
| Multiplexed Peer | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| **Advanced Features** |
| Compression (GZIP/LZ4) | ✅ | ✅ | ✅ | ✅ (GZIP/DEFLATE) | ✅ | ✅ (GZIP/DEFLATE) | ✅ (GZIP/DEFLATE) | ✅ (GZIP/DEFLATE) | 📋 | 📋 | ✅ |
| Event System | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (async) | ✅ | ✅ | ✅ (OTP) | ✅ |
| Security/Encryption | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 🚧 | ✅ | ✅ | ✅ |
| Service Discovery | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Connection Pooling | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ (async) | ✅ | ✅ | ✅ (OTP) | ✅ |
| FFI/Native Bridge | N/A | ✅ (N-API) | N/A | ✅ (FFI) | N/A | N/A | N/A | N/A | N/A | N/A | N/A |
| **Framework Integration** |
| Native Language | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Async/Promises | ✅ | ✅ | ✅ | ✅ (ReactPHP) | ✅ (tokio) | ✅ | ✅ (asyncio) | ✅ (async/await) | ✅ (async/await) | ✅ (OTP/Task) | ✅ (coroutines) |
| Type Safety | ✅ | ✅ | ✅ | ✅ (8.1+) | ✅ (Strong) | ✅ | ✅ (hints) | ✅ (Strong) | ✅ (Strong) | ✅ (@spec) | ✅ (Strong) |

**Legend**: ✅ Implemented | ⚠️ Partial | 🚧 In Progress | ❌ Not Implemented | 📋 Planned

### Language-Specific Details

#### C++ (Core Implementation)
- **Status**: ✅ Production-ready
- **Tests**: 206/206 (100%)
- **Coverage**: 98%
- **Features**: Native implementation, SIMD acceleration, zero-copy
- **Use Case**: Core library, high-performance applications

#### TypeScript (Recommended)
- **Status**: ✅ Production-ready
- **Tests**: 163/163 passing (100%)
- **Coverage**: 95%
- **Version**: v0.1.3
- **Features**: Node.js N-API bindings, WebSocket, async/await, Service Discovery, Connection Pooling
- **Use Case**: Node.js backend, real-time applications

#### Go (Cloud-Native)
- **Status**: ✅ Complete & Production-ready (100% Feature Parity)
- **Tests**: 68+ tests (100%)
- **Coverage**: 90%
- **Version**: v1.0.1
- **Features**: Goroutines, WebSocket/HTTP/2 transport, Multiplexed Peer, Service Discovery, Connection Pooling, Pure Go
- **Use Case**: Microservices, cloud deployments, P2P networks, service mesh

#### PHP (Production Ready)
- **Status**: ✅ Complete & Production-ready
- **Tests**: 145+ tests (100%)
- **Coverage**: 95%
- **Version**: v0.1.3
- **Features**: FFI bindings, HTTP transport, WebSocket, Compression, Event system
- **Complete**: Pure PHP classes, HTTP, Compression, Events, Matrix operations
- **Use Case**: Web applications, Laravel/Symfony/WordPress integration

#### Rust (Production Ready - 100%)
- **Status**: ✅ Complete & Production-ready (100% Feature Complete)
- **Package**: `umicp-sdk`
- **Version**: **v0.3.1**
- **Crates.io**: Ready to publish
- **Installation**: `cargo add umicp-sdk`
- **Tests**: 123/123 tests (100% passing, 11 ignored)
- **Coverage**: 100%
- **Features**: 
  - ✅ Zero-copy operations
  - ✅ Memory-safe (Rust guarantees)
  - ✅ Async/await with tokio 1.42
  - ✅ SIMD matrix operations (via ndarray 0.16)
  - ✅ Multiplexed Peer architecture
  - ✅ EventEmitter pattern (async support)
  - ✅ WebSocket client/server (non-blocking)
  - ✅ HTTP/2 Client with `http2_prior_knowledge()`
  - ✅ HTTP/2 Server (axum 0.8 - fully compatible)
  - ✅ Service Discovery
  - ✅ Connection Pooling
- **Use Case**: Systems programming, high-performance services, cloud-native apps
- **Dependencies**: ✅ Updated to latest (tokio 1.42, reqwest 0.12, tokio-tungstenite 0.26, axum 0.8, thiserror 2.0)

#### Java (Production Ready)
- **Status**: ✅ Complete & Production-ready (Phase 1, 2, 3 & 4)
- **Tests**: 380+/380+ tests (100%)
- **Coverage**: 97%
- **Version**: v0.1.3
- **Features**: Pure Java, WebSocket, Multiplexed Peer, Service Discovery, Connection Pooling, HTTP/2 Client, Compression (GZIP/DEFLATE)
- **Modules**: umicp-core, umicp-transport, umicp-examples
- **Use Case**: Enterprise applications, Spring ecosystem, JVM microservices

#### Kotlin (Production Ready - Most Feature-Complete) 🏆
- **Status**: ✅ Complete & Production-ready (Phase 5 Complete)
- **Tests**: 140+ tests (100%)
- **Coverage**: 96%
- **Version**: v0.1.3
- **Features**:
  - ✅ Idiomatic Kotlin with DSL builders
  - ✅ Coroutines for async/await (superior to threads)
  - ✅ Null safety built-in (compile-time)
  - ✅ Extension functions (`a dot b`, `vector.normalize()`)
  - ✅ Operator overloading (`a + b`, `a * b`)
  - ✅ Data classes (immutable, auto equals/hashCode/copy)
  - ✅ Sealed classes for type-safe error handling
  - ✅ WebSocket client/server
  - ✅ **HTTP/2 client AND server** (only SDK with both) ⭐
  - ✅ **LZ4/LZ4_HC compression** (5x faster than GZIP) ⭐
  - ✅ GZIP/DEFLATE compression
  - ✅ Service Discovery
  - ✅ Connection Pooling
- **Use Case**: Modern JVM applications, Android, Spring Boot, Ktor, microservices, real-time systems
- **Advantages over Java**: HTTP/2 Server, LZ4 compression, null safety, coroutines, extension functions, operators, less boilerplate
- **Special**: 🥇 FIRST and ONLY SDK with HTTP/2 Server + LZ4 compression

#### Python (Published to PyPI ✅)
- **Status**: ✅ Complete & Production-ready
- **Package**: `umicp_sdk` (PEP 625 compliant)
- **Version**: **v0.3.2** ⭐
- **PyPI**: https://pypi.org/project/umicp-sdk/
- **Installation**: `pip install umicp-sdk`
- **Import**: `from umicp_sdk import Envelope`
- **Features**: 
  - ✅ AsyncIO throughout (modern async/await)
  - ✅ NumPy integration for matrix operations
  - ✅ Full type hints (PEP 561 compliant)
  - ✅ WebSocket client/server (websockets library)
  - ✅ HTTP/2 client (httpx) and server (aiohttp)
  - ✅ Multiplexed Peer architecture
  - ✅ Event system with async handlers
  - ✅ Service Discovery
  - ✅ Connection Pooling
  - ✅ Compression (GZIP/DEFLATE)
- **Use Case**: AI/ML pipelines, data science, async applications
- **Python**: 3.9+ required

#### C# (NuGet Package Ready)
- **Status**: ✅ Complete & Production-ready
- **Package**: `HiveLLM.Umicp.SDK`
- **Version**: **v0.3.0**
- **NuGet**: Package ready for publication
- **Installation**: `dotnet add package HiveLLM.Umicp.SDK`
- **Namespace**: `using Umicp.SDK;`
- **Tests**: 128/128 tests (100% passing)
- **Coverage**: 100%
- **Features**:
  - ✅ .NET 8.0 with modern C# 12
  - ✅ SIMD-accelerated matrix operations (System.Numerics.Vectors)
  - ✅ Full async/await support throughout
  - ✅ Strong type safety and null reference checks
  - ✅ WebSocket client transport
  - ✅ HTTP client (JSON and binary)
  - ✅ Multiplexed Peer architecture
  - ✅ Event system with observer pattern
  - ✅ Service Discovery with health checks
  - ✅ Connection Pooling with auto-scaling
  - ✅ Comprehensive XML documentation
  - ✅ Native JSON types (JsonElement)
  - ✅ Tool Discovery (MCP-compatible)
- **Use Case**: Enterprise applications, .NET microservices, Windows services, Unity/game engines
- **.NET**: 8.0+ required

#### Swift (Production Ready) 🆕
- **Status**: ✅ Complete & Production-ready
- **Tests**: 100+ tests (100% passing)
- **Coverage**: 100%
- **Version**: v0.2.0
- **Platform**: macOS 12+, iOS 15+, Linux
- **Features**:
  - ✅ Native JSON types (AnyCodable wrapper)
  - ✅ MCP-compatible tool discovery
  - ✅ WebSocket + HTTP/2 transport (async/await)
  - ✅ SIMD matrix operations (Accelerate framework)
  - ✅ Cross-platform support
  - ✅ Swift Package Manager
  - ✅ Zero external dependencies
  - ✅ 100% async/await
  - ✅ Type-safe with Codable
- **Use Case**: iOS/macOS applications, server-side Swift, cross-platform tools
- **Swift**: 5.9+ required

#### Elixir (Published to Hex.pm ✅)
- **Status**: ✅ Complete & Production-ready
- **Package**: `umicp`
- **Version**: **v0.3.0**
- **Hex.pm**: https://hex.pm/packages/umicp
- **Documentation**: https://hexdocs.pm/umicp
- **Installation**: `{:umicp, "~> 0.3.0"}`
- **Tests**: 100+ tests (100% passing)
- **Coverage**: 100%
- **Platform**: Elixir 1.15+, OTP 25+
- **Features**:
  - ✅ Native Elixir terms (no wrappers needed)
  - ✅ MCP-compatible tool discovery (behaviors)
  - ✅ WebSocket (GenServer) + HTTP/2 (Finch)
  - ✅ Pure Elixir matrix operations
  - ✅ OTP integration (supervision, behaviors)
  - ✅ Pipe-friendly API
  - ✅ Published to Hex.pm
  - ✅ Fault-tolerant design
  - ✅ Type specs (@type, @spec)
- **Use Case**: Distributed systems, Phoenix applications, real-time apps, fault-tolerant services
- **Elixir**: 1.15+ required

## 🔧 Core Features

### ✅ **Implemented Features (TypeScript)**
- **Protocol**: Binary envelope-based communication with capability negotiation
- **Transport**: WebSocket with Streaming HTTP support
- **Multiplexed Architecture**: Each peer functions as server AND client simultaneously
- **Message Types**: CONTROL, DATA, ACK, ERROR operations
- **Payload Types**: Vector, Text, Metadata, Binary data support
- **Matrix Operations**: SIMD-accelerated dot product, cosine similarity, matrix multiplication
- **Event-Driven API**: Node.js EventEmitter pattern with multiple listeners
- **Peer Discovery**: Automatic handshake (HELLO → ACK) with metadata exchange
- **Topologies**: Support for Full Mesh, Hub-and-Spoke, Pipeline, Hierarchical networks

### 📊 **Performance Metrics (Production-Validated)**
- **Latency**: ~10-50ms handshake, <1ms for established connections
- **Throughput**: >10,000 messages/second for small payloads (<1KB)
- **Memory**: ~200 bytes overhead per peer connection
- **Scalability**: Tested with 100+ simultaneous peer connections
- **Reconnection**: Automatic with configurable retry logic

### 🚧 **In Development**
- **Enhanced Security**: Advanced TLS/SSL configuration
- **Load Balancing**: Intelligent message distribution (partial support in Rust)
- **Message Routing**: Automatic routing in mesh networks
- **Compression**: Per-message deflate compression

### ✅ **Production Ready SDKs**
All 10 standardized SDKs (v0.3.x):

**Published (6/10):**
- **Python**: `umicp_sdk` v0.3.2 → [PyPI](https://pypi.org/project/umicp-sdk/) ✅
- **Rust**: `umicp-sdk` v0.3.1 → [crates.io](https://crates.io/crates/umicp-sdk) ✅
- **TypeScript**: `@hivellm/umicp-sdk` v0.3.1 → [npm](https://www.npmjs.com/package/@hivellm/umicp-sdk) ✅
- **C#**: `HiveLLM.Umicp.SDK` v0.3.0 → [NuGet](https://www.nuget.org/packages/HiveLLM.Umicp.SDK) ✅
- **PHP**: `hivellm/umicp-sdk` v0.3.0 → [Packagist](https://packagist.org/packages/hivellm/umicp-sdk) ✅
- **Elixir**: `umicp` v0.3.0 → [Hex.pm](https://hex.pm/packages/umicp) ✅

**Ready to Publish (4/10):**
- **Go**: `github.com/hivellm/umicp-sdk` v0.3.0 - Cloud-native, Microservices
- **Swift**: `UMICP-SDK` v0.3.0 - iOS/macOS/Linux, Accelerate SIMD
- **Kotlin**: `umicp-sdk` v0.3.0 - JVM, Coroutines, HTTP/2, LZ4
- **Java**: `umicp-sdk` v0.3.0 - Enterprise, Spring ecosystem

### 📋 **Planned Features**
- **Python Enhancements**: TensorFlow/PyTorch integrations, additional ML frameworks
- **Authentication**: Enhanced peer validation and authorization
- **Load Balancing**: Intelligent message distribution across peers
- **Message Routing**: Advanced routing algorithms for mesh networks
- **Compression**: Advanced compression algorithms across all bindings

## 🧪 Testing

### Test Coverage (v0.3.x)
- **Python**: 100% coverage (162+ tests) - ✅ Published to PyPI
- **Rust**: 100% coverage (123+ tests) - ✅ Published to crates.io
- **TypeScript**: 100% coverage (209+ tests) - ✅ Published to npm
- **C#**: 100% coverage (128+ tests) - ✅ Published to NuGet
- **PHP**: 95% coverage (145+ tests) - ✅ Published to Packagist
- **Elixir**: 100% coverage (100+ tests) - ✅ Published to Hex.pm
- **Go**: 90%+ coverage (85+ tests) - Package standardized
- **Swift**: 100% coverage (100+ tests) - Package standardized
- **Kotlin**: 96% coverage (140+ tests) - Package standardized
- **Java**: 97% coverage (380+ tests) - Package standardized

**Total**: 1,500+ tests across all SDKs | **Status**: All passing ✅

### Test Types (TypeScript)
- **Unit Tests**: Envelope, Frame, Matrix operations
- **Integration Tests**: WebSocket client/server, peer connections
- **Performance Tests**: Throughput benchmarks, latency distribution, resource utilization
- **E2E Tests**: Multiplexed peer networks, federated learning scenarios
- **Stress Tests**: 100+ simultaneous connections, memory leak detection

## 📚 Documentation

### Core Documentation
- **API Reference**: [docs/api/](./docs/api/)
- **Installation Guide**: [docs/guides/installation.md](./docs/guides/installation.md)
- **Protocol Specification**: [docs/guides/protocol-api.md](./docs/guides/protocol-api.md)

### TypeScript-Specific Documentation
- **Multiplexed Peer Architecture**: [bindings/typescript/docs/MULTIPLEXED_PEER.md](./bindings/typescript/docs/MULTIPLEXED_PEER.md)
- **Event-Driven Refactor**: [bindings/typescript/docs/EVENT_DRIVEN_REFACTOR.md](./bindings/typescript/docs/EVENT_DRIVEN_REFACTOR.md)
- **Quick Start Guide**: [bindings/typescript/docs/QUICKSTART_MULTIPLEXED.md](./bindings/typescript/docs/QUICKSTART_MULTIPLEXED.md)
- **Changelog**: [bindings/typescript/docs/CHANGELOG_MULTIPLEXED.md](./bindings/typescript/docs/CHANGELOG_MULTIPLEXED.md)
- **Auto Protocol**: [bindings/typescript/docs/AUTO_PROTOCOL.md](./bindings/typescript/docs/AUTO_PROTOCOL.md)
- **TypeScript README**: [bindings/typescript/README.md](./bindings/typescript/README.md)

### PHP-Specific Documentation
- **Implementation Status**: [bindings/php/IMPLEMENTATION_COMPLETE.md](./bindings/php/IMPLEMENTATION_COMPLETE.md)
- **Changelog**: [bindings/php/docs/CHANGELOG.md](./bindings/php/docs/CHANGELOG.md)
- **API Specification**: [bindings/php/docs/API_SPECIFICATION.md](./bindings/php/docs/API_SPECIFICATION.md)
- **FFI Integration Guide**: [bindings/php/docs/FFI_INTEGRATION_GUIDE.md](./bindings/php/docs/FFI_INTEGRATION_GUIDE.md)
- **PHP README**: [bindings/php/README.md](./bindings/php/README.md)

### Python-Specific Documentation
- **Python README**: [bindings/python/README.md](./bindings/python/README.md)
- **Test Report**: [bindings/python/TEST_REPORT.md](./bindings/python/TEST_REPORT.md)
- **Changelog**: [bindings/python/CHANGELOG.md](./bindings/python/CHANGELOG.md)

### Bindings Overview
- **Complete Overview**: [bindings/README.md](./bindings/README.md) - Consolidated guide for all language bindings

## 🔗 Part of HiveLLM Ecosystem

UMICP is a core component of the **HiveLLM ecosystem**, providing high-performance binary protocol for agent-to-agent communication:

- **Vectorizer**: Semantic search and vector database (uses UMICP for agent integration)
- **Task Queue**: Workflow orchestration (uses UMICP for distributed task coordination)
- **Agent Framework**: Multi-language agent platform with UMICP support for peer communication
- **Voxa**: Voice AI assistant with agent coordination via UMICP

**Use Case**: UMICP enables >10,000 msg/sec agent-to-agent communication with sub-millisecond latency, perfect for real-time multi-agent coordination, federated learning, and distributed AI systems.

See the [HiveLLM paper](../gov/papers/vectorizer_paper.md) for comprehensive ecosystem documentation.

## 📄 License

MIT License - See [LICENSE](LICENSE) file for details.

---

**BIP**: BIP-05 - Universal Matrix Intelligent Communication Protocol  
**Status**: ✅ Core Features Complete  
**Repository**: HiveLLM UMICP Implementation
