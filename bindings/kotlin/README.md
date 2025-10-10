# UMICP Kotlin SDK

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Kotlin](https://img.shields.io/badge/Kotlin-1.9.21-blue.svg)](https://kotlinlang.org/)
[![Status](https://img.shields.io/badge/Status-Production%20Ready-green.svg)](#)

Kotlin SDK for the Universal Matrix Intelligent Communication Protocol (UMICP), providing high-performance communication and matrix operations for distributed systems, federated learning, and real-time applications.

## ✨ Features

### Core Features
- **🔗 Type-Safe Envelopes**: Data classes with null safety
- **⚡ High Performance**: Optimized matrix operations
- **🚀 Coroutines**: Full async/await support with Kotlin Coroutines
- **📦 DSL Builders**: Idiomatic Kotlin DSL for easy configuration
- **🛡️ Null Safety**: Kotlin's strong type system
- **✅ Comprehensive Testing**: 120+ tests with Kotest
- **📚 Full Documentation**: KDoc for all public APIs

### Transport Features
- **🌐 WebSocket Client**: Auto-reconnect, coroutine-based, message queuing
- **🖥️ WebSocket Server**: Multi-client, broadcast, statistics tracking
- **🌐 HTTP/2 Client**: Modern HTTP/2 transport with OkHttp ⭐ NEW
- **⚡ Event-Driven**: Suspend function handlers with coroutines
- **📊 Connection Pooling**: Efficient connection management
- **🔍 Service Discovery**: Automatic peer discovery and capability exchange
- **📦 Compression**: GZIP and DEFLATE compression support ⭐ NEW

### Kotlin-Specific Features
- **Extension Functions**: Convenient matrix operations (`a dot b`, `vector.normalize()`)
- **Operator Overloading**: Natural syntax (`a + b`, `a - b`, `a * b`)
- **Sealed Classes**: Type-safe result handling
- **Data Classes**: Immutable, equals/hashCode/copy built-in
- **Companion Objects**: Factory methods and utilities
- **Inline Functions**: Zero-overhead DSL builders

## 📦 Installation

### Gradle (Kotlin DSL)
```kotlin
dependencies {
    implementation("com.hivellm:umicp-kotlin:0.1.2")
}
```

### Gradle (Groovy)
```groovy
dependencies {
    implementation 'com.hivellm:umicp-kotlin:0.1.2'
}
```

### Maven
```xml
<dependency>
    <groupId>com.hivellm</groupId>
    <artifactId>umicp-kotlin</artifactId>
    <version>0.1.2</version>
</dependency>
```

### Prerequisites
- **Kotlin**: 1.9.21+
- **Java**: 17+
- **Gradle**: 8+ or **Maven**: 3.8+

## 🚀 Quick Start

### Basic Envelope Usage

```kotlin
import com.hivellm.umicp.core.*
import com.hivellm.umicp.types.*

// Create envelope with DSL
val envelope = Envelope.build {
    from("client-001")
    to("server-001")
    operation(OperationType.DATA)
    capability("priority", "high")
}

// Validate
if (envelope.validate()) {
    println("✓ Valid envelope")
}

// Serialize to JSON
val json = envelope.serialize()
println(json)

// Deserialize
val received = Envelope.deserialize(json)

// Compute hash
val hash = envelope.hash()
```

### Matrix Operations

```kotlin
import com.hivellm.umicp.core.*

// Using extension functions
val a = floatArrayOf(1f, 2f, 3f)
val b = floatArrayOf(4f, 5f, 6f)

// Dot product
val dot = a dot b // 32.0

// Cosine similarity
val similarity = a cosineSim b // 0.9746

// Vector operations with operators
val sum = a + b        // [5, 7, 9]
val diff = a - b       // [-3, -3, -3]
val product = a * b    // [4, 10, 18]

// Normalize
val normalized = a.normalize()
val magnitude = a.magnitude()

// Matrix multiplication
val matA = floatArrayOf(1f, 2f, 3f, 4f) // 2x2
val matB = floatArrayOf(5f, 6f, 7f, 8f) // 2x2
val result = Matrix.matrixMultiply(matA, matB, 2, 2, 2)
    .getOrThrow() // [19, 22, 43, 50]
```

### WebSocket Communication

```kotlin
import com.hivellm.umicp.transport.*
import kotlinx.coroutines.runBlocking

runBlocking {
    // Create server
    val server = UMICPWebSocketServer.create("localhost", 8080) {
        onMessage { clientId, envelope ->
            println("Received from $clientId: ${envelope.operation}")
        }
    }
    server.start()
    
    // Create client
    val client = UMICPWebSocketClient.create("ws://localhost:8080") {
        onMessage { envelope ->
            println("Received: ${envelope.from}")
        }
    }
    client.connect()
    
    // Send message
    val message = Envelope.build {
        from("client-001")
        to("server")
        operation(OperationType.DATA)
    }
    client.send(message)
    
    // Cleanup
    client.disconnect()
    server.stop()
}
```

### Service Discovery

```kotlin
import com.hivellm.umicp.discovery.*
import kotlinx.coroutines.runBlocking

runBlocking {
    val discovery = ServiceDiscovery()
    
    // Register service
    discovery.register(ServiceInfo(
        id = "api-gateway-1",
        name = "api-gateway",
        endpoint = "http://localhost:8080",
        capabilities = setOf("http", "websocket")
    ))
    
    // Find by capability
    val services = discovery.findByCapability("websocket")
    
    // Find by name pattern
    val gateways = discovery.findByName("api-*")
}
```

### Connection Pool

```kotlin
import com.hivellm.umicp.pool.*
import kotlinx.coroutines.runBlocking

runBlocking {
    val pool = ConnectionPool(
        uri = "ws://localhost:8080",
        minSize = 2,
        maxSize = 10
    )
    
    pool.initialize()
    
    // Send with pooled connection
    pool.send(envelope)
    
    // Execute with connection
    pool.withConnection { client ->
        client.send(envelope)
    }
    
    // Get statistics
    val stats = pool.getStats()
    println("Active: ${stats.activeConnections}")
    
    pool.close()
}
```

### HTTP/2 Client

```kotlin
import com.hivellm.umicp.transport.*
import kotlinx.coroutines.runBlocking

runBlocking {
    val client = UMICPHttpClient.create("http://localhost:8080")
    
    // POST request
    val response = client.post("/api/messages", envelope)
    
    // GET request
    val data = client.get("/api/data")
    
    // Check HTTP/2 support
    val isHttp2 = client.isHttp2()
    
    client.close()
}
```

### Compression

```kotlin
import com.hivellm.umicp.compression.*

// Compress data
val data = "Large message...".repeat(100).toByteArray()
val compressed = Compression.compress(data, CompressionAlgorithm.GZIP)
val decompressed = Compression.decompress(compressed, CompressionAlgorithm.GZIP)

// Extension functions
val compressed2 = data.compress(CompressionAlgorithm.GZIP)
val decompressed2 = compressed2.decompress(CompressionAlgorithm.GZIP)

// String compression
val text = "Hello, World!".repeat(100)
val compressedText = text.compress(CompressionAlgorithm.GZIP)

// Get statistics
val ratio = Compression.getCompressionRatio(data.size, compressed.size)
val savings = Compression.getSavings(data.size, compressed.size)
```

## 📚 API Reference

### Core Classes

#### Envelope
Message envelope for protocol communication.

```kotlin
data class Envelope(
    val from: String,
    val to: String,
    val operation: OperationType,
    val messageId: String = UUID.randomUUID().toString(),
    val timestamp: Long = System.currentTimeMillis(),
    val payloadHint: PayloadHint? = null,
    val capabilities: Map<String, String> = emptyMap()
)
```

**Methods:**
- `validate(): Boolean` - Validate the envelope
- `validateOrThrow()` - Validate or throw ValidationException
- `serialize(): String` - Serialize to JSON
- `hash(): String` - Compute SHA-256 hash
- `toBuilder(): Builder` - Convert to builder

#### Matrix
Matrix and vector operations.

```kotlin
object Matrix {
    fun dotProduct(a: FloatArray, b: FloatArray): MatrixResult<Float>
    fun cosineSimilarity(a: FloatArray, b: FloatArray): MatrixResult<Float>
    fun magnitude(vector: FloatArray): MatrixResult<Float>
    fun normalize(vector: FloatArray): MatrixResult<FloatArray>
    fun vectorAdd(a: FloatArray, b: FloatArray): MatrixResult<FloatArray>
    fun vectorSubtract(a: FloatArray, b: FloatArray): MatrixResult<FloatArray>
    fun vectorMultiply(a: FloatArray, b: FloatArray): MatrixResult<FloatArray>
    fun matrixMultiply(a: FloatArray, b: FloatArray, m: Int, n: Int, p: Int): MatrixResult<FloatArray>
    fun transpose(matrix: FloatArray, rows: Int, cols: Int): MatrixResult<FloatArray>
}
```

**Extension Functions:**
```kotlin
infix fun FloatArray.dot(other: FloatArray): Float
infix fun FloatArray.cosineSim(other: FloatArray): Float
fun FloatArray.magnitude(): Float
fun FloatArray.normalize(): FloatArray
operator fun FloatArray.plus(other: FloatArray): FloatArray
operator fun FloatArray.minus(other: FloatArray): FloatArray
operator fun FloatArray.times(other: FloatArray): FloatArray
```

#### MatrixResult
Sealed class for type-safe result handling.

```kotlin
sealed class MatrixResult<out T> {
    data class Success<T>(val value: T) : MatrixResult<T>()
    data class Failure(val error: String) : MatrixResult<Nothing>()
    
    fun getOrNull(): T?
    fun getOrThrow(): T
    fun <R> map(transform: (T) -> R): MatrixResult<R>
    fun onSuccess(action: (T) -> Unit): MatrixResult<T>
    fun onFailure(action: (String) -> Unit): MatrixResult<T>
}
```

### Type Definitions

#### OperationType
```kotlin
enum class OperationType {
    CONTROL, DATA, ACK, ERROR, REQUEST, RESPONSE
}
```

#### PayloadType
```kotlin
enum class PayloadType {
    VECTOR, TEXT, METADATA, BINARY
}
```

#### EncodingType
```kotlin
enum class EncodingType {
    FLOAT32, FLOAT64, INT32, INT64, UINT8, UINT16, UINT32, UINT64
}
```

## 🏆 Comparison with Other SDKs

### Kotlin vs Java

| Feature | Java | Kotlin | Advantage |
|---------|------|--------|-----------|
| **Tests** | 380+ | 120+ | Java (more tests) |
| **Coverage** | 97% | 95% | Java (slightly higher) |
| **HTTP/2** | ✅ Client | ✅ Client | = |
| **Compression** | ✅ GZIP/DEFLATE | ✅ GZIP/DEFLATE | = |
| **Null Safety** | Annotations | Built-in | ✅ **Kotlin** (compile-time) |
| **Async** | CompletableFuture | Coroutines | ✅ **Kotlin** (efficient) |
| **Extensions** | ❌ | ✅ | ✅ **Kotlin** |
| **Operators** | ❌ | ✅ | ✅ **Kotlin** |
| **DSL** | Builder | Builder + DSL | ✅ **Kotlin** |
| **Boilerplate** | High | Low (~40% less) | ✅ **Kotlin** |
| **Service Discovery** | ✅ | ✅ | = |
| **Connection Pool** | ✅ | ✅ | = |

**Both SDKs are production-ready with Phase 4 complete!**

## 🎯 Use Cases

### Federated Learning
```kotlin
val weights = Envelope.build {
    from("coordinator")
    to("worker-001")
    operation(OperationType.DATA)
    payloadHint(PayloadHint.build {
        type(PayloadType.VECTOR)
        encoding(EncodingType.FLOAT32)
        count(10000)
    })
}
```

### Microservices Communication
```kotlin
val request = Envelope.build {
    from("api-gateway")
    to("auth-service")
    operation(OperationType.REQUEST)
    capability("action", "validate-token")
}
```

### Real-time Data Processing
```kotlin
runBlocking {
    val pool = ConnectionPool("ws://processor:8080")
    pool.initialize()
    
    dataStream.collect { data ->
        val envelope = createDataEnvelope(data)
        pool.send(envelope)
    }
}
```

## 🧪 Testing

```bash
# Run all tests
./gradlew test

# Run with coverage
./gradlew test jacocoTestReport

# Run specific test class
./gradlew test --tests "com.hivellm.umicp.core.EnvelopeTest"
```

### Test Coverage
- **Enums**: OperationType, PayloadType, EncodingType
- **Core**: Envelope, PayloadHint, Matrix, UMICP facade
- **Discovery**: ServiceDiscovery, ServiceInfo
- **Transport**: WebSocketClient, WebSocketServer (integration tests)
- **Pool**: ConnectionPool

## 📈 Performance

Pure Kotlin implementation:
- **Envelope Creation**: ~0.3ms
- **Serialization**: ~1.5ms
- **Dot Product** (1000 elements): ~0.08ms
- **Matrix Multiply** (100x100): ~8ms
- **Coroutine overhead**: ~0.01ms

## 🔧 Building from Source

```bash
# Clone repository
git clone https://github.com/hivellm/umicp.git
cd umicp/bindings/kotlin

# Build
./gradlew build

# Run tests
./gradlew test

# Generate documentation
./gradlew dokkaHtml

# Publish to local Maven
./gradlew publishToMavenLocal
```

## 📖 Examples

The SDK includes comprehensive examples:

- **BasicExample.kt**: Envelope creation, serialization, validation
- **MatrixExample.kt**: Vector operations, matrix operations, ML use cases
- **WebSocketExample.kt**: Client/server communication, event handling
- **ServiceDiscoveryExample.kt**: Service registration, lookup, patterns

Run examples:
```bash
./gradlew run --args="com.hivellm.umicp.examples.BasicExample"
```

## 🗺️ Roadmap

### Phase 1: Foundation ✅ **COMPLETE**
- ✅ Core types and exceptions
- ✅ Envelope with JSON serialization
- ✅ Matrix operations (pure Kotlin)
- ✅ UMICP utility facade
- ✅ Comprehensive tests (100+ tests)

### Phase 2: Transport ✅ **COMPLETE**
- ✅ WebSocket client
- ✅ WebSocket server
- ✅ Coroutine-based async API
- ✅ Event system with suspend functions

### Phase 3: Advanced Features ✅ **COMPLETE**
- ✅ Service Discovery
- ✅ Connection Pooling
- ✅ Extension functions and operators
- ✅ DSL builders

### Phase 4: HTTP/2 & Compression ✅ **COMPLETE** ⭐ NEW
- ✅ HTTP/2 client (OkHttp)
- ✅ Compression (GZIP/DEFLATE)
- ✅ Extension functions for compression
- ✅ Comprehensive tests and examples

### Phase 5: Future Enhancements
- [ ] HTTP/2 server
- [ ] LZ4 compression
- [ ] Metrics and monitoring
- [ ] Kotlin Multiplatform (JVM, JS, Native)
- [ ] Kotlin Flow integration
- [ ] Structured concurrency patterns
