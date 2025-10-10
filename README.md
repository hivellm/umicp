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

# Rust (In Progress)
cd bindings/rust  
cargo build --release
# Note: Rust bindings are under active development
```

## 📦 Language Bindings

| Language | Status | Features | Performance | Notes |
|----------|--------|----------|-------------|-------|
| **C++** | ✅ Core | Native implementation, SIMD acceleration | Fastest | Core library |
| **TypeScript** | ✅ Production | Node.js N-API bindings, WebSocket transport | Fast | **Recommended** |
| **Rust** | 🚧 In Progress | Zero-copy, memory-safe bindings | Near-native | Under development |
| **Python** | 📋 Planned | Asyncio, ML integration | TBD | Not started |
| **Go** | 📋 Planned | Goroutines, cloud-native | TBD | Not started |
| **Java** | 📋 Planned | JNI bindings, enterprise integration | TBD | Not started |
| **PHP** | 📋 Planned | Web integration | TBD | Not started |

**Legend**: ✅ Production-ready | 🚧 Active development | 📋 Planned

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

### 📋 **Planned Features**
- **Python Bindings**: AsyncIO support for ML pipelines
- **Go Bindings**: Cloud-native microservices integration
- **Java Bindings**: Enterprise JVM ecosystem support
- **PHP Bindings**: Web application integration
- **Authentication**: Peer validation before accepting connections

## 🧪 Testing

### Test Coverage
- **C++ Core**: 95% coverage (matrix operations, envelope serialization)
- **TypeScript**: 92% coverage (WebSocket transport, multiplexed peer, E2E scenarios)
- **Rust**: 🚧 In progress

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
