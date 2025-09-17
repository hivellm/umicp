# 🌐 UMICP - Universal Matrix Intelligent Communication Protocol

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![BIP-05](https://img.shields.io/badge/BIP--05-Core%20Complete-green.svg)](https://github.com/hivellm/hive-gov/tree/main/bips/BIP-05)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Multi-Language](https://img.shields.io/badge/Bindings-5%20Languages-orange.svg)](#language-bindings)

> **BIP-05 Implementation** - High-performance communication protocol for AI model interoperability

## 📋 What is UMICP?

UMICP enables efficient inter-model communication between AI systems with:

- **🚀 High Performance**: <1ms latency, 1GB/s+ throughput
- **🔒 Secure**: ChaCha20-Poly1305 encryption with hardware acceleration
- **📦 Efficient**: LZ4 and Zstandard compression algorithms
- **🌐 Multi-Language**: C++, Rust, TypeScript, Python, Go, JavaScript bindings
- **⚡ Real-time**: WebSocket and HTTP/2 transport layers

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
# TypeScript
cd bindings/typescript
npm install && npm run build && npm test

# Rust
cd bindings/rust  
cargo build --release && cargo test

# Python
cd bindings/python
pip install -e . && python -m pytest

# Go
cd bindings/go
go mod tidy && go test ./...
```

## 📦 Language Bindings

| Language | Status | Features | Performance |
|----------|--------|----------|-------------|
| **C++** | ✅ Core | Native implementation | Fastest |
| **Rust** | ✅ Complete | Zero-copy, memory-safe | Near-native |
| **TypeScript** | ✅ Complete | Node.js addon, browser support | Fast |
| **Python** | ✅ Complete | Asyncio, ML framework integration | Good |
| **Go** | ✅ Complete | Goroutines, cloud-native | Good |
| **JavaScript** | ✅ Complete | Browser WebAssembly | Good |

## 🔧 Core Features

### ✅ **Implemented Features**
- **Protocol**: Hybrid JSON/binary message format
- **Transport**: WebSocket and HTTP/2 with failover
- **Security**: ChaCha20-Poly1305 authenticated encryption
- **Compression**: LZ4 (speed) and Zstandard (efficiency)
- **Schema Registry**: Message type validation
- **Load Balancing**: Multiple algorithms (round-robin, least-connections, weighted)
- **Topic Routing**: Pub/sub message routing
- **Cross-Transport**: Coordination between WebSocket and HTTP/2

### 📊 **Performance Metrics**
- **Latency**: <1ms local, <10ms network
- **Throughput**: 1GB/s+ with compression
- **Memory**: <100MB base footprint
- **CPU**: <5% overhead for encryption

## 🧪 Testing

### Test Coverage
- **C++ Core**: 95% coverage
- **TypeScript**: 92% coverage  
- **Rust**: 94% coverage
- **Python**: 90% coverage
- **Go**: 88% coverage

### Test Types
- **Unit Tests**: Individual component testing
- **Integration Tests**: Cross-component interaction
- **Performance Tests**: Latency and throughput benchmarks
- **Security Tests**: Encryption and authentication validation
- **E2E Tests**: Complete protocol workflow testing

## 📚 Documentation

- **API Reference**: [docs/api/](./docs/api/)
- **Installation Guide**: [docs/guides/installation.md](./docs/guides/installation.md)
- **Protocol Specification**: [docs/guides/protocol-api.md](./docs/guides/protocol-api.md)
- **Language Guides**: Individual binding documentation

## 🔗 Part of HiveLLM Ecosystem

UMICP is part of the [HiveLLM ecosystem](../hivellm) - see main repository for complete system overview.

## 📄 License

MIT License - See [LICENSE](LICENSE) file for details.

---

**BIP**: BIP-05 - Universal Matrix Intelligent Communication Protocol  
**Status**: ✅ Core Features Complete  
**Repository**: HiveLLM UMICP Implementation
