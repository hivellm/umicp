# UMICP Kotlin SDK - Technical Guide

> **📝 COMPREHENSIVE TECHNICAL GUIDE**

**Version**: 0.1.2  
**Last Updated**: October 11, 2025  
**Target Audience**: Kotlin Developers  

---

## 🚀 Quick Start

### Add Dependency
```kotlin
// build.gradle.kts
dependencies {
    implementation("com.hivellm:umicp-kotlin:0.1.2")
}
```

### Basic Usage
```kotlin
import com.hivellm.umicp.core.*
import com.hivellm.umicp.types.*

fun main() {
    // Create envelope with DSL
    val envelope = Envelope.build {
        from("client-001")
        to("server-001")
        operation(OperationType.DATA)
        payload("Hello UMICP!".toByteArray())
    }
    
    // Serialize
    val json = envelope.toJson()
    println(json)
    
    // Deserialize
    val received = Envelope.fromJson(json)
    println("From: ${received.from}")
}
```

---

## 📝 Core Concepts

### Envelope

```kotlin
// DSL Builder (Recommended)
val envelope = Envelope.build {
    from("client")
    to("server")
    operation(OperationType.DATA)
    payload("data".toByteArray())
    payloadType(PayloadType.BINARY)
    encoding(EncodingType.RAW)
    capability("priority", "high")
}

// Traditional Builder
val envelope = Envelope.Builder()
    .from("client")
    .to("server")
    .build()
```

### Matrix Operations

```kotlin
import com.hivellm.umicp.core.*

// Extension functions
val a = floatArrayOf(1f, 2f, 3f)
val b = floatArrayOf(4f, 5f, 6f)

val dot = a dot b  // 32.0
val cos = a.cosineSimilarity(b)  // 0.974
val norm = a.normalize()

// Operators
val sum = a + b
val diff = a - b
val scaled = a * 2.0f

// Matrix operations
val matrix = arrayOf(
    floatArrayOf(1f, 2f),
    floatArrayOf(3f, 4f)
)
val vector = floatArrayOf(5f, 6f)
val result = matrix * vector  // [17, 39]
```

---

## 🌐 WebSocket Transport

### Client

```kotlin
import com.hivellm.umicp.transport.*
import kotlinx.coroutines.*

suspend fun main() {
    val client = UMICPWebSocketClient("ws://localhost:8080")
    
    // Event handlers
    client.onConnected {
        println("Connected!")
    }
    
    client.onMessage { envelope ->
        println("Received: ${envelope.from}")
    }
    
    client.onDisconnected { reason ->
        println("Disconnected: $reason")
    }
    
    // Connect
    client.connect()
    
    // Send
    val envelope = Envelope.build {
        from("client")
        to("server")
        operation(OperationType.DATA)
    }
    client.send(envelope)
    
    // Disconnect
    client.disconnect()
}
```

### Server

```kotlin
val server = UMICPWebSocketServer(8080)

server.onClientConnected { clientId ->
    println("Client $clientId connected")
}

server.onClientMessage { clientId, envelope ->
    println("Message from $clientId")
    
    // Echo back
    val response = Envelope.build {
        from("server")
        to(envelope.from)
        operation(OperationType.RESPONSE)
    }
    server.sendToClient(clientId, response)
}

// Start
runBlocking {
    server.start()
}

// Stop
server.stop()
```

---

## 🔥 HTTP/2 Transport

### HTTP/2 Client

```kotlin
import com.hivellm.umicp.transport.*

val client = UMICPHttpClient.create("http://localhost:8080")

// POST request
val envelope = Envelope.build { /* ... */ }
val response = client.post("/api/messages", envelope)

// GET request
val data = client.get("/api/messages/123")

// Check HTTP/2
val isHttp2 = client.isHttp2()
println("Using HTTP/2: $isHttp2")

// Close
client.close()
```

### HTTP/2 Server 🥇 (Unique Feature)

```kotlin
import com.hivellm.umicp.transport.*

// Create server with handler
val server = UMICPHttpServer.create(8080) { envelope ->
    // Process request
    println("Request from: ${envelope.from}")
    
    // Return response
    Envelope.build {
        from("server")
        to(envelope.from)
        operation(OperationType.RESPONSE)
        payload("Processed".toByteArray())
    }
}

// Start
runBlocking {
    server.start()
}

// Stop
server.stop()
```

---

## ⚡ Compression

### Basic Compression

```kotlin
import com.hivellm.umicp.compression.*

val data = "Large text...".repeat(100).toByteArray()

// GZIP
val gzipCompressed = Compression.compress(data, CompressionAlgorithm.GZIP)
val gzipDecompressed = Compression.decompress(gzipCompressed, CompressionAlgorithm.GZIP)

// DEFLATE
val deflateCompressed = Compression.compress(data, CompressionAlgorithm.DEFLATE)

// LZ4 🥇 (5x faster!)
val lz4Compressed = Compression.compress(data, CompressionAlgorithm.LZ4)
val lz4Decompressed = Compression.decompress(lz4Compressed, CompressionAlgorithm.LZ4)

// LZ4_HC 🥇 (high compression)
val lz4hcCompressed = Compression.compress(data, CompressionAlgorithm.LZ4_HC)
```

### Extension Functions

```kotlin
// Compress
val compressed = data.compress(CompressionAlgorithm.LZ4)

// Decompress
val decompressed = compressed.decompress(CompressionAlgorithm.LZ4)

// String compression
val compressedString = "Large text...".compress(CompressionAlgorithm.GZIP)

// Statistics
val ratio = Compression.getCompressionRatio(data.size, compressed.size)
val savings = Compression.getSavings(data.size, compressed.size)
println("Compression ratio: $ratio")
println("Savings: $savings%")
```

---

## 🔍 Service Discovery

```kotlin
import com.hivellm.umicp.discovery.*

val discovery = ServiceDiscovery.getInstance()

// Register service
val service = ServiceInfo(
    name = "my-service",
    url = "ws://localhost:8080",
    type = "websocket",
    metadata = mapOf(
        "version" to "1.0",
        "region" to "us-east"
    )
)
discovery.register(service)

// Find services
val services = discovery.findByName("my-service")
val byType = discovery.findByType("websocket")
val byMetadata = discovery.findByMetadata("region", "us-east")

// Unregister
discovery.unregister("my-service")
```

---

## 🏊 Connection Pooling

```kotlin
import com.hivellm.umicp.pool.*

val pool = ConnectionPool(
    minSize = 2,
    maxSize = 10,
    connectionFactory = {
        UMICPWebSocketClient("ws://localhost:8080")
    }
)

// Get connection
val conn = pool.acquire()

// Use connection
conn.send(envelope)

// Return to pool
pool.release(conn)

// Statistics
println("Active: ${pool.activeCount}")
println("Idle: ${pool.idleCount}")

// Shutdown
pool.shutdown()
```

---

## 💡 Best Practices

### 1. Use DSL Builders
```kotlin
// Good ✅
val envelope = Envelope.build {
    from("client")
    to("server")
}

// OK (but verbose)
val envelope = Envelope.Builder()
    .from("client")
    .to("server")
    .build()
```

### 2. Use Extension Functions
```kotlin
// Good ✅
val result = a dot b
val normalized = vector.normalize()

// OK (but verbose)
val result = Matrix.dotProduct(a, b)
val normalized = Matrix.normalize(vector)
```

### 3. Use Coroutines
```kotlin
// Good ✅
suspend fun connect() {
    client.connect()
}

// OK (but blocks)
fun connect() = runBlocking {
    client.connect()
}
```

### 4. Use Null Safety
```kotlin
// Good ✅
val envelope: Envelope  // Cannot be null
val hint: PayloadHint?  // Nullable, explicit

// Avoid
var envelope: Envelope? = null  // Unnecessary nullable
```

### 5. Use LZ4 for Performance 🥇
```kotlin
// Best performance ✅
val compressed = data.compress(CompressionAlgorithm.LZ4)

// Good (smaller size)
val compressed = data.compress(CompressionAlgorithm.GZIP)
```

---

## 🐛 Troubleshooting

### Connection Issues
```kotlin
// Enable auto-reconnect
val client = UMICPWebSocketClient(
    url = "ws://localhost:8080",
    autoReconnect = true,
    maxReconnectAttempts = 10,
    reconnectDelay = 5000
)
```

### Performance Issues
```kotlin
// Use LZ4 for speed
val compressed = data.compress(CompressionAlgorithm.LZ4)  // 5x faster!

// Use connection pooling
val pool = ConnectionPool(minSize = 5, maxSize = 20)

// Use coroutines
launch(Dispatchers.IO) {
    client.connect()
}
```

### Memory Issues
```kotlin
// Close resources
client.close()
server.stop()
pool.shutdown()

// Use sequences for large data
val data = generateSequence { /* ... */ }
    .take(1000)
    .toList()
```

---

## 📚 Additional Resources

- [README.md](./README.md) - Overview
- [STATUS.md](./STATUS.md) - Status
- [ROADMAP.md](./ROADMAP.md) - Timeline
- [REVIEWS.md](./REVIEWS.md) - Quality assessment
- [Examples](../src/main/kotlin/com/hivellm/umicp/examples/) - Working examples

---

**Version**: 0.1.2  
**Last Updated**: October 11, 2025  
**Maintainer**: HiveLLM Contributors

