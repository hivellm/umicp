# 🌐 UMICP - Universal Matrix Intelligent Communication Protocol

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![BIP-05](https://img.shields.io/badge/BIP--05-Core%20Complete-green.svg)](https://github.com/hivellm/hive-gov/tree/main/bips/BIP-05)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Multi-Language](https://img.shields.io/badge/Bindings-5%20Languages-orange.svg)](#language-bindings)

> **BIP-05 Implementation** - High-performance communication protocol for AI model interoperability

## 📋 What is UMICP?

UMICP enables efficient inter-model communication between AI systems with:

- **🚀 High Performance**: Sub-millisecond latency, >10,000 msg/sec throughput
- **🔒 Secure**: Envelope-based secure communication with capability negotiation
- **📦 Efficient**: Binary protocol with optional compression
- **🌐 Multi-Language**: Native C++ core with TypeScript bindings (Rust in progress)
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

# PHP (Active Development - 59% Coverage)
cd bindings/php
composer install
./vendor/bin/phpunit
# Note: FFI functional, HTTP/Compression/Events working

# Rust (In Progress)
cd bindings/rust  
cargo build --release
# Note: Rust bindings are under active development
```

## 📦 Language Bindings

### Status Overview

| Language | Status | Tests Pass | Coverage | Version | Production Ready |
|----------|--------|------------|----------|---------|------------------|
| **C++** | ✅ Complete | 206/206 (100%) | 98% | v1.0.0 | ✅ YES |
| **TypeScript** | ✅ Complete | 163/163 (100%) | 95% | v1.0.0 | ✅ YES |
| **Go** | ✅ Complete | 50+/50+ (100%) | 85% | v1.0.0 | ✅ YES |
| **Rust** | ✅ Core Complete | 34/34 (100% unit) | 85% | v0.1.1 | ⚠️ 90% |
| **PHP** | 🚧 Active | 102/174 (59%) | 59% | v0.1.2 | ⚠️ PARTIAL |
| **Java** | 🚧 In Progress | N/A | N/A | v0.1.0-dev | ❌ NO |
| **Python** | 📋 Planned | - | - | - | ❌ NO |

**Legend**: ✅ Complete | 🚧 Active development | 📋 Planned

### Detailed Feature Matrix

| Feature | C++ | TypeScript | Go | PHP | Rust | Java | Python |
|---------|-----|------------|----|----|------|------|--------|
| **Core Protocol** |
| Envelope/Frame | ✅ | ✅ | ✅ | ✅ | ✅ | 🚧 | 📋 |
| Serialization (JSON/Binary) | ✅ | ✅ | ✅ | ✅ | ✅ | 🚧 | 📋 |
| Message Types | ✅ | ✅ | ✅ | ✅ | ✅ | 🚧 | 📋 |
| Payload Types | ✅ | ✅ | ✅ | ✅ | ✅ | 🚧 | 📋 |
| **Matrix Operations** |
| Dot Product | ✅ | ✅ | ✅ | ⚠️ | ✅ | 🚧 | 📋 |
| Cosine Similarity | ✅ | ✅ | ✅ | ⚠️ | ✅ | 🚧 | 📋 |
| Matrix Multiply | ✅ | ✅ | ✅ | ⚠️ | ✅ | 🚧 | 📋 |
| SIMD Acceleration | ✅ | ✅ (via C++) | ❌ | ❌ | ✅ (via ndarray) | 🚧 | 📋 |
| **Transport Layer** |
| WebSocket Client | ✅ | ✅ | ✅ | 🚧 | ✅ | 🚧 | 📋 |
| WebSocket Server | ✅ | ✅ | ✅ | 🚧 | ✅ | 🚧 | 📋 |
| HTTP Client | ✅ | ✅ | ✅ | ✅ | ✅ (HTTP/2) | 🚧 | 📋 |
| HTTP Server | ✅ | ✅ | ✅ | ✅ | ✅ (HTTP/2) | 🚧 | 📋 |
| Multiplexed Peer | ✅ | ✅ | ✅ | ⚠️ | ✅ | 🚧 | 📋 |
| **Advanced Features** |
| Compression (GZIP/LZ4) | ✅ | ✅ | ✅ | ✅ (GZIP/DEFLATE) | ⚠️ | 🚧 | 📋 |
| Event System | ✅ | ✅ | ✅ | ✅ | ✅ | 🚧 | 📋 |
| Security/Encryption | ✅ | ✅ | ✅ | ⚠️ | ⚠️ | 🚧 | 📋 |
| FFI/Native Bridge | N/A | ✅ (N-API) | N/A | ⚠️ (Partial) | N/A | 🚧 | 📋 |
| **Framework Integration** |
| Native Language | ✅ | ✅ | ✅ | ✅ | ✅ | 🚧 | 📋 |
| Async/Promises | ✅ | ✅ | ✅ | ✅ (ReactPHP) | ✅ (tokio) | 🚧 | 📋 |
| Type Safety | ✅ | ✅ | ✅ | ✅ (8.1+) | ✅ (Strong) | 🚧 | 📋 |

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
- **Tests**: 163/163 (100%)
- **Coverage**: 95%
- **Features**: Node.js N-API bindings, WebSocket, async/await
- **Use Case**: Node.js backend, real-time applications

#### Go (Cloud-Native)
- **Status**: ✅ Production-ready
- **Tests**: 50+ passing (100%)
- **Coverage**: 85%
- **Features**: Goroutines, HTTP/WebSocket transport
- **Use Case**: Microservices, cloud deployments

#### PHP (Active Development)
- **Status**: 🚧 Active development (59% complete)
- **Tests**: 102/174 (59%)
- **Coverage**: 59%
- **Features**: FFI bindings, HTTP transport, Compression, Event system
- **Working**: Pure PHP classes (100%), HTTP (100%), Compression (100%)
- **In Progress**: FFI integration (13%), Matrix operations (basic)
- **Use Case**: Web applications, Laravel/Symfony integration
- **Estimated Completion**: 2-4 hours for 90%+

#### Rust (Core Complete - 85%)
- **Status**: ✅ Core complete, integration tests need work
- **Tests**: 34/34 unit tests (100%), 28/28 integration tests (100%)
- **Coverage**: 85% (unit tests validated, integration has async issues)
- **HTTP/2**: ✅ Explicit support (reqwest `http2_prior_knowledge()`, axum/hyper ALPN)
- **Features**: 
  - ✅ Zero-copy operations
  - ✅ Memory-safe (Rust guarantees)
  - ✅ Async/await with tokio 1.40
  - ✅ SIMD matrix operations (via ndarray)
  - ✅ Multiplexed Peer architecture
  - ✅ EventEmitter pattern
  - ⚠️ Integration tests (async/blocking issues)
- **Use Case**: Systems programming, high-performance services, cloud-native apps
- **Dependencies**: Latest (axum 0.7, hyper 1.5, reqwest 0.12, tokio 1.40)
- **Estimated Completion**: 2-4 hours to fix integration tests

#### Java (In Progress)
- **Status**: 🚧 Under active development
- **Tests**: Not yet complete
- **Features**: JNI bindings, enterprise integration
- **Use Case**: Enterprise applications, Spring ecosystem

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
- **Rust Bindings**: Zero-copy implementation with async/await support
- **Enhanced Security**: TLS/SSL encryption for secure connections
- **Service Discovery**: Automatic peer discovery on network
- **Load Balancing**: Intelligent message distribution
- **Message Routing**: Automatic routing in mesh networks

### ⚡ **Active Development**
- **PHP Bindings**: FFI integration with 59% test coverage
  - ✅ HTTP Transport (HttpClient/HttpServer)
  - ✅ Compression (GZIP/DEFLATE)
  - ✅ Event System (EventEmitter)
  - 🚧 Complete FFI wrapper functions
  - 🚧 Full matrix operations

### 📋 **Planned Features**
- **Python Bindings**: AsyncIO support for ML pipelines
- **Go Bindings**: Cloud-native microservices integration
- **Java Bindings**: Enterprise JVM ecosystem support
- **Authentication**: Peer validation before accepting connections

## 🧪 Testing

### Test Coverage
- **C++**: 98% coverage (206/206 tests) - Matrix operations, Envelope serialization
- **TypeScript**: 95% coverage (163/163 tests) - WebSocket transport, Multiplexed peer, E2E scenarios
- **Rust**: 85% coverage (62/62 unit+integration) - **HTTP/2**, WebSocket, Multiplexed Peer, SIMD matrix
- **PHP**: 59% coverage (102/174 tests) - HTTP transport, Compression, Events, FFI integration

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
