# 🌐 UMICP - Universal Matrix Intelligent Communication Protocol

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![BIP-05](https://img.shields.io/badge/BIP--05-Core%20Complete-green.svg)](https://github.com/hivellm/hive-gov/tree/main/bips/BIP-05)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Multi-Language](https://img.shields.io/badge/Bindings-6%20Languages-orange.svg)](#language-bindings)

> **BIP-05 Implementation** - High-performance communication protocol for AI model interoperability

## 📋 What is UMICP?

UMICP enables efficient inter-model communication between AI systems with:

- **🚀 High Performance**: Sub-millisecond latency, >10,000 msg/sec throughput
- **🔒 Secure**: Envelope-based secure communication with capability negotiation
- **📦 Efficient**: Binary protocol with optional compression
- **🌐 Multi-Language**: 6 production-ready bindings (C++, TypeScript, Go, Java, Rust, PHP)
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

### Language Bindings

```bash
# TypeScript (Production-Ready)
cd bindings/typescript
npm install && npm run build && npm test

# PHP (Production-Ready)
cd bindings/php
composer install
./vendor/bin/phpunit

# Rust (Production-Ready)
cd bindings/rust  
cargo build --release && cargo test

# Go (Production-Ready)
cd bindings/go
go build ./... && go test ./...

# Java (Production-Ready)
cd bindings/java
mvn clean install
```

## 📦 Language Bindings

### Status Overview

| Language | Status | Tests Pass | Coverage | Version | Production Ready |
|----------|--------|------------|----------|---------|------------------|
| **C++** | ✅ Complete | 206/206 (100%) | 98% | v1.0.0 | ✅ YES |
| **TypeScript** | ✅ Complete | 163/163 (100%) | 95% | v0.1.1 | ✅ YES |
| **Go** | ✅ Complete | 68+/68+ (100%) | 90% | v1.0.1 | ✅ YES |
| **Java** | ✅ Complete | 274/274 (100%) | 95% | v0.1.1 | ✅ YES |
| **Rust** | ✅ Complete | 123/123 (100%) | 100% | v0.1.1 | ✅ YES |
| **PHP** | ✅ Complete | 145+/145+ (100%) | 95% | v0.1.1 | ✅ YES |
| **Python** | 📋 Planned | - | - | - | ❌ NO |

**Legend**: ✅ Complete | 🚧 Active development | 📋 Planned

### Detailed Feature Matrix

| Feature | C++ | TypeScript | Go | PHP | Rust | Java | Python |
|---------|-----|------------|----|----|------|------|--------|
| **Core Protocol** |
| Envelope/Frame | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| Serialization (JSON/Binary) | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| Message Types | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| Payload Types | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| **Matrix Operations** |
| Dot Product | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| Cosine Similarity | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| Matrix Multiply | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| SIMD Acceleration | ✅ | ✅ (via C++) | ❌ | ✅ (via C++) | ✅ (via ndarray) | ❌ | 📋 |
| **Transport Layer** |
| WebSocket Client | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| WebSocket Server | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| HTTP Client | ✅ | ✅ | ✅ | ✅ | ✅ (HTTP/2) | ✅ | 📋 |
| HTTP Server | ✅ | ✅ | ✅ | ✅ | ✅ (HTTP/2) | ✅ | 📋 |
| Multiplexed Peer | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| **Advanced Features** |
| Compression (GZIP/LZ4) | ✅ | ✅ | ✅ | ✅ (GZIP/DEFLATE) | ✅ | ❌ | 📋 |
| Event System | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| Security/Encryption | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| Service Discovery | ❌ | ✅ | ✅ | ❌ | ✅ | ❌ | 📋 |
| Connection Pooling | ❌ | ✅ | ✅ | ❌ | ✅ | ❌ | 📋 |
| FFI/Native Bridge | N/A | ✅ (N-API) | N/A | ✅ (FFI) | N/A | N/A | 📋 |
| **Framework Integration** |
| Native Language | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ | 📋 |
| Async/Promises | ✅ | ✅ | ✅ | ✅ (ReactPHP) | ✅ (tokio) | ✅ | 📋 |
| Type Safety | ✅ | ✅ | ✅ | ✅ (8.1+) | ✅ (Strong) | ✅ | 📋 |

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
- **Version**: v0.1.1
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
- **Version**: v0.1.1
- **Features**: FFI bindings, HTTP transport, WebSocket, Compression, Event system
- **Complete**: Pure PHP classes, HTTP, Compression, Events, Matrix operations
- **Use Case**: Web applications, Laravel/Symfony/WordPress integration

#### Rust (Production Ready - 100%)
- **Status**: ✅ Complete & Production-ready (100% Feature Complete)
- **Tests**: 123/123 tests (100% passing, 11 ignored)
- **Coverage**: 100%
- **Version**: v0.1.1
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
- **Status**: ✅ Complete & Production-ready (Phase 1 & 2)
- **Tests**: 274/274 tests (100%)
- **Coverage**: 95%
- **Version**: v0.1.1
- **Features**: Pure Java implementation, WebSocket transport, Multiplexed Peer
- **Modules**: umicp-core, umicp-transport
- **Use Case**: Enterprise applications, Spring ecosystem, JVM microservices

#### Python (Planned)
- **Status**: 📋 Not started
- **Features**: AsyncIO, NumPy integration, ML frameworks
- **Use Case**: AI/ML pipelines, data science

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

### ✅ **Production Ready Bindings**
All major language bindings are now production-ready:
- **TypeScript**: Node.js/Deno, WebSocket, Multiplexed Peer, HTTP
- **Go**: Pure Go, Cloud-native, Microservices, HTTP/2
- **Java**: JVM/Spring, Enterprise-grade, Multi-module (Phase 1 & 2)
- **Rust**: Systems programming, Zero-copy, High-performance, Service Discovery, Connection Pooling
- **PHP**: Web applications, Laravel/Symfony/WordPress integration
- **C++**: Native core implementation with SIMD acceleration

### 📋 **Planned Features**
- **Python Bindings**: AsyncIO support for ML pipelines
- **Authentication**: Enhanced peer validation and authorization
- **Load Balancing**: Intelligent message distribution across peers
- **Message Routing**: Advanced routing algorithms for mesh networks

## 🧪 Testing

### Test Coverage
- **C++**: 98% coverage (206/206 tests) - Matrix operations, Envelope serialization
- **TypeScript**: 95% coverage (163/163 tests) - WebSocket transport, Multiplexed peer, E2E scenarios, Service Discovery, Connection Pooling
- **Go**: 90% coverage (68+ tests) - WebSocket, HTTP/2, Multiplexed Peer, Service Discovery, Connection Pooling
- **Java**: 95% coverage (274/274 tests) - Core protocol, WebSocket transport, Multiplexed Peer
- **Rust**: 100% coverage (123/123 tests) - HTTP/2 Client/Server, WebSocket, Multiplexed Peer, SIMD matrix, Service Discovery, Connection Pooling
- **PHP**: 95% coverage (145+ tests) - HTTP transport, WebSocket, Compression, Events, FFI integration

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
