# Changelog

All notable changes to the UMICP Kotlin SDK will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.2] - 2025-10-10

### Added
- Initial release of UMICP Kotlin SDK
- **HTTP/2 transport** ⭐ NEW
  - HTTP/2 client with OkHttp
  - POST, GET, PUT, DELETE support
  - Automatic protocol negotiation
  - Protocol detection
- **Compression** ⭐ NEW
  - GZIP compression/decompression
  - DEFLATE compression/decompression
  - Configurable compression levels
  - Extension functions for compression
  - Compression statistics (ratio, savings)
- Core protocol implementation
  - `Envelope` data class with builder and DSL support
  - `PayloadHint` for payload metadata
  - Type-safe enums: `OperationType`, `PayloadType`, `EncodingType`
  - Exception hierarchy: `UMICPException`, `ValidationException`, `SerializationException`, etc.
  
- Matrix operations
  - `Matrix` object with comprehensive vector/matrix operations
  - Extension functions: `dot`, `cosineSim`, `magnitude()`, `normalize()`
  - Operator overloading: `+`, `-`, `*` for vectors
  - Type-safe `MatrixResult` sealed class for error handling
  
- Transport layer
  - `UMICPWebSocketClient` with auto-reconnect
  - `UMICPWebSocketServer` with multi-client support
  - `UMICPHttpClient` with HTTP/2 support ⭐ NEW
  - Coroutine-based async/await API
  - Event handlers with suspend functions
  - Message queuing and broadcasting
  
- Advanced features
  - `ServiceDiscovery` with capability-based lookup
  - `ConnectionPool` for efficient connection management
  - `Compression` with GZIP/DEFLATE support ⭐ NEW
  - Statistics tracking
  - Stale connection cleanup
  
- Developer experience
  - DSL builders for idiomatic Kotlin
  - Comprehensive documentation with KDoc
  - 120+ tests with Kotest
  - 6 working examples (including Compression and HTTP) ⭐ NEW
  - Null safety throughout
  
### Features Parity
- ✅ 100% feature parity with Java SDK Phase 4
- ✅ Production-ready quality (both Java and Kotlin)
- ✅ Full async/await with Kotlin Coroutines
- ✅ Extension functions and operator overloading
- ✅ Idiomatic Kotlin patterns
- ✅ HTTP/2 and Compression matching Java implementation

### Dependencies
- Kotlin 1.9.21
- Kotlinx Coroutines 1.7.3
- Kotlinx Serialization 1.6.2
- Java-WebSocket 1.5.5
- OkHttp 4.12.0 (HTTP/2 support)
- Apache Commons Compress 1.25.0 ⭐ NEW
- Kotest 5.8.0
- JUnit Jupiter 5.10.1

### Documentation
- Complete README with examples
- API reference with KDoc
- 4 comprehensive examples
- CHANGELOG

### Performance
- Pure Kotlin implementation
- Coroutine-based for efficiency
- Zero-copy where possible
- Optimized matrix operations

### Known Limitations
- HTTP/2 server not yet implemented (client only)
- LZ4 compression not yet implemented (GZIP/DEFLATE only)
- Not yet multiplatform (JVM only)

## [Unreleased]

### Planned
- HTTP/2 server (client already implemented ✅)
- LZ4 compression (GZIP/DEFLATE already implemented ✅)
- Kotlin Multiplatform (JS, Native)
- Kotlin Flow integration
- Metrics and monitoring
- Performance benchmarks

---

**Note**: This is the initial release. Future versions will maintain backward compatibility following semantic versioning.

