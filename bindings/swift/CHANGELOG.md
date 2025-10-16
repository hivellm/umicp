# Changelog

All notable changes to the UMICP Swift bindings will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] - 2025-10-16

### Added

#### Core Features
- Initial release of UMICP Swift bindings
- **Native JSON Type Support**: Capabilities now support native types (String, Int, Bool, Double, Arrays, Dictionaries)
- `AnyCodable` wrapper for type-erased JSON values
- `Envelope` struct with full Codable conformance
- `EnvelopeBuilder` with fluent API
- SHA-256 hash calculation and verification
- Input validation and error handling

#### Tool Discovery (MCP-Compatible)
- `DiscoverableService` protocol for tool discovery
- `OperationSchema` with JSON Schema support
- `ServerInfo` for service metadata
- `DiscoveryHelpers` for generating JSON responses
- Full MCP (Model Context Protocol) compatibility

#### Transport Layer
- `UMICPWebSocket` actor for WebSocket communication (async/await)
- `UMICPHTTP2` struct for HTTP/2 transport
- Automatic reconnection support
- Message handlers with callbacks

#### Matrix Operations
- SIMD-optimized operations using Accelerate framework
- Vector addition, subtraction, scaling
- Dot product calculation
- Matrix multiplication
- Vector magnitude and normalization
- Fallback implementations for Linux

#### Testing & Examples
- 100+ comprehensive tests covering:
  - Core functionality (AnyCodable, Envelope, Types)
  - Tool discovery (OperationSchema, ServerInfo, DiscoverableService)
  - Matrix operations (vectors, matrices)
  - Transport (WebSocket, HTTP/2)
  - Integration workflows
- 3 working examples:
  - BasicEnvelope.swift
  - ToolDiscovery.swift
  - MatrixOperations.swift

#### Documentation
- Comprehensive README with quick start guide
- API reference documentation
- Usage examples
- Performance benchmarks
- Cross-platform support guide

### Technical Details

#### Breaking Changes
- New project structure
- Capabilities type changed from `[String: String]` to `[String: AnyCodable]`
- MCP-compatible tool discovery required

#### Platform Support
- macOS 12.0+
- iOS 15.0+
- Linux (with Swift 5.9+)

#### Performance
- Envelope creation: ~0.1ms
- Serialization: ~1ms
- SIMD-optimized matrix operations
- 10,000+ messages/second throughput

#### Dependencies
- No external dependencies
- Uses Swift Standard Library only
- Optional: Accelerate framework for SIMD (Apple platforms)

### Quality Metrics
- 100+ tests passing
- Cross-platform validated
- Zero external dependencies
- Complete API coverage

---

## [Unreleased]

### Planned Features
- Swift Concurrency improvements
- AsyncSequence for streaming
- SwiftNIO integration for Linux
- Advanced compression support
- gRPC transport layer

---

**Version History**

- **0.2.0** (2025-10-16): Initial release with native types and tool discovery

