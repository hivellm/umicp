# Changelog

All notable changes to the UMICP Kotlin SDK will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.2] - 2025-10-10

### Added - Phase 5 Complete! 🎉
- Initial release of UMICP Kotlin SDK
- **HTTP/2 transport** (Client & Server)
  - HTTP/2 client with OkHttp
  - **HTTP/2 server with Undertow** ⭐ NEW
  - POST, GET, PUT, DELETE support
  - Route DSL (get, post, put, delete, envelope)
  - Automatic protocol negotiation
  - Protocol detection
  - Coroutine-based handlers
  
- **Compression** (GZIP/DEFLATE/LZ4/LZ4_HC)
  - GZIP compression/decompression
  - DEFLATE compression/decompression
  - **LZ4 fast compression** ⭐ NEW (5x faster than GZIP)
  - **LZ4 High Compression** ⭐ NEW (better ratio)
  - Configurable compression levels
  - Extension functions for compression
  - Compression statistics (ratio, savings)
  - Performance benchmarks
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
  - `UMICPHttpClient` with HTTP/2 support
  - **`UMICPHttpServer` with HTTP/2 and route DSL** ⭐ NEW
  - Coroutine-based async/await API
  - Event handlers with suspend functions
  - Message queuing and broadcasting
  
- Advanced features
  - `ServiceDiscovery` with capability-based lookup
  - `ConnectionPool` for efficient connection management
  - **`Compression` with GZIP/DEFLATE/LZ4/LZ4_HC support** ⭐ NEW
  - Statistics tracking
  - Stale connection cleanup
  
- Developer experience
  - DSL builders for idiomatic Kotlin
  - Comprehensive documentation with KDoc
  - **140+ tests** with Kotest ⭐ (+20 new tests)
  - **7 working examples** (including HTTP Server, Compression, HTTP Client) ⭐ NEW
  - Null safety throughout
  
### Features Parity
- ✅ **EXCEEDS Java SDK** - Kotlin has HTTP/2 Server & LZ4 🏆
- ✅ Production-ready quality (both Java and Kotlin)
- ✅ Full async/await with Kotlin Coroutines
- ✅ Extension functions and operator overloading
- ✅ Idiomatic Kotlin patterns
- ✅ MORE compression algorithms than Java (LZ4/LZ4_HC)
- ✅ HTTP/2 Server (Java doesn't have this yet)

### Dependencies
- Kotlin 1.9.21
- Kotlinx Coroutines 1.7.3
- Kotlinx Serialization 1.6.2
- Java-WebSocket 1.5.5
- OkHttp 4.12.0 (HTTP/2 client)
- **Undertow 2.3.10** (HTTP/2 server) ⭐ NEW
- Apache Commons Compress 1.25.0
- **LZ4 Java 1.8.0** (LZ4 compression) ⭐ NEW
- Kotest 5.8.0
- JUnit Jupiter 5.10.1

### Documentation
- Complete README with examples
- API reference with KDoc
- **7 comprehensive examples** ⭐ (+3 new)
- CHANGELOG
- **PHASE5_COMPLETE.md** - Phase 5 summary ⭐ NEW
- Complete test documentation

### Performance
- Pure Kotlin implementation
- Coroutine-based for efficiency
- Zero-copy where possible
- Optimized matrix operations

### Known Limitations
- Not yet multiplatform (JVM only)
- gRPC transport not yet implemented
- Metrics/monitoring not yet implemented

## [Unreleased]

### Planned
- Kotlin Multiplatform (JS, Native)
- Kotlin Flow integration
- Metrics and monitoring
- Performance benchmarks
- gRPC transport
- Advanced routing patterns

---

**Note**: This is the initial release. Future versions will maintain backward compatibility following semantic versioning.

