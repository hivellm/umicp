# Changelog

All notable changes to the UMICP C# SDK will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-10

### Added

#### Core Features
- **Envelope class** with full serialization support
  - JSON serialization/deserialization
  - SHA-256 hash computation
  - Builder pattern implementation
  - Validation support

#### Types
- `OperationType` enum (Control, Data, Ack, Error, Request, Response)
- `PayloadType` enum (Vector, Text, Metadata, Binary, Json, Matrix)
- `EncodingType` enum (Float32, Float64, Int32, Int64, Utf8, Base64, Hex)
- `ConnectionState` enum (Disconnected, Connecting, Connected, Reconnecting, Disconnecting, Error)
- `PayloadHint` class for payload metadata
- `TransportStats` class for connection statistics

#### Matrix Operations
- SIMD-accelerated matrix operations using `System.Numerics.Vectors`
- Dot product computation
- Cosine similarity calculation
- Matrix multiplication
- Element-wise addition and subtraction
- Matrix transpose
- Matrix normalization (L2)
- Scalar multiplication
- Euclidean distance calculation
- Vector magnitude calculation

#### Transport Layer
- **WebSocketClient** with full async support
  - Automatic reconnection support
  - Event-driven message handling
  - Background receive loop
  - Connection statistics tracking
- **HttpClient** for REST API communication
  - JSON serialization/deserialization
  - Binary data support
  - Configurable timeouts

#### Peer-to-Peer
- **MultiplexedPeer** for P2P communication
  - Automatic handshake protocol (HELLO → ACK)
  - Peer connection management
  - Broadcast capabilities
  - Per-peer statistics
- **PeerConnection** for individual peer management
- **PeerInfo** for peer metadata
- **HandshakeProtocol** for connection negotiation

#### Event System
- **EventEmitter** with observer pattern
- Support for multiple listeners per event
- One-time event handlers (`Once`)
- Event data passing
- Thread-safe event emission
- Error handling in event handlers

#### Service Discovery
- **ServiceDiscovery** for service registration
- Automatic health checking
- Service heartbeat mechanism
- Service timeout detection
- Query by type, name, or health status
- Metadata support

#### Connection Pooling
- **ConnectionPool** for efficient connection management
- Configurable min/max connections
- Automatic connection validation
- Connection lifecycle management
- Acquire/release pattern
- Execute pattern for automatic resource management

#### Testing
- Comprehensive unit test suite
- Tests for Envelope serialization
- Tests for Matrix operations
- Tests for Event system
- XUnit test framework

#### Examples
- Basic envelope creation and serialization
- Matrix operations with SIMD
- WebSocket client usage
- Multiplexed peer communication
- Service discovery usage

#### Documentation
- Complete README with usage examples
- API documentation with XML comments
- Architecture documentation
- Contributing guidelines

### Technical Details

- **Target Framework**: .NET 8.0
- **Language**: C# 12
- **Dependencies**:
  - System.Text.Json (9.0.9)
  - System.Numerics.Vectors (4.6.1)
- **Test Framework**: XUnit
- **Architecture**: Modern async/await patterns throughout

### Performance

- SIMD acceleration for matrix operations
- Zero-copy operations where possible
- Efficient memory management
- Thread-safe implementations
- Connection pooling for reduced overhead

### Standards Compliance

- Implements BIP-05 UMICP specification
- Compatible with other UMICP language bindings
- Standard .NET conventions and patterns

## [Unreleased]

### Planned Features

- WebSocket Server implementation
- HTTP Server implementation
- Compression support (GZIP, LZ4)
- Enhanced security features
- TLS/SSL configuration
- Binary serialization (CBOR, MessagePack)
- Performance benchmarks
- Integration tests
- NuGet package publication

---

## Version History

- **v1.0.0** (2025-10-10) - Initial production-ready release

