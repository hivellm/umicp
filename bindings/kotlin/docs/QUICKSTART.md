# UMICP Kotlin SDK - Quick Start Guide

Get started with the UMICP Kotlin SDK in 5 minutes!

## Installation

### Gradle (Kotlin DSL) - Recommended
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

## Basic Usage

### 1. Create an Envelope

```kotlin
import com.hivellm.umicp.core.*
import com.hivellm.umicp.types.*

// Using DSL builder (recommended)
val envelope = Envelope.build {
    from("client-001")
    to("server-001")
    operation(OperationType.DATA)
    capability("priority", "high")
}

// Or traditional builder
val envelope2 = Envelope.builder()
    .from("client-001")
    .to("server-001")
    .operation(OperationType.DATA)
    .build()
```

### 2. Serialize and Validate

```kotlin
// Validate
if (envelope.validate()) {
    println("✓ Valid!")
}

// Or throw exception
envelope.validateOrThrow()

// Serialize
val json = envelope.serialize()

// Deserialize
val received = Envelope.deserialize(json)

// Hash
val hash = envelope.hash()
```

### 3. Matrix Operations

```kotlin
val a = floatArrayOf(1f, 2f, 3f)
val b = floatArrayOf(4f, 5f, 6f)

// Extension functions
val dot = a dot b                    // 32.0
val similarity = a cosineSim b       // 0.9746
val mag = a.magnitude()             // 3.7416
val normalized = a.normalize()      // [0.267, 0.534, 0.801]

// Operators
val sum = a + b                     // [5, 7, 9]
val diff = a - b                    // [-3, -3, -3]
val product = a * b                 // [4, 10, 18]
```

### 4. WebSocket Client

```kotlin
import com.hivellm.umicp.transport.*
import kotlinx.coroutines.runBlocking

runBlocking {
    // Create client
    val client = UMICPWebSocketClient.create("ws://localhost:8080") {
        onConnect {
            println("Connected!")
        }
        
        onMessage { envelope ->
            println("Received: ${envelope.from}")
        }
        
        onDisconnect {
            println("Disconnected")
        }
    }
    
    // Connect
    client.connect()
    
    // Send message
    val msg = Envelope.build {
        from("client")
        to("server")
        operation(OperationType.DATA)
    }
    client.send(msg)
    
    // Disconnect
    client.disconnect()
}
```

### 5. WebSocket Server

```kotlin
runBlocking {
    val server = UMICPWebSocketServer.create("localhost", 8080) {
        onClientConnect { clientId ->
            println("Client connected: $clientId")
        }
        
        onMessage { clientId, envelope ->
            println("Received from $clientId: ${envelope.operation}")
            
            // Echo back
            send(clientId, envelope)
        }
        
        onClientDisconnect { clientId ->
            println("Client disconnected: $clientId")
        }
    }
    
    server.start()
    
    // Broadcast to all clients
    val broadcast = Envelope.build {
        from("server")
        to("all")
        operation(OperationType.DATA)
    }
    server.broadcast(broadcast)
    
    // Stop server
    server.stop()
}
```

### 6. Service Discovery

```kotlin
import com.hivellm.umicp.discovery.*

runBlocking {
    val discovery = ServiceDiscovery()
    
    // Register service
    discovery.register(ServiceInfo(
        id = "api-1",
        name = "api-gateway",
        endpoint = "http://localhost:8080",
        capabilities = setOf("http", "websocket")
    ))
    
    // Find by capability
    val services = discovery.findByCapability("websocket")
    
    // Find by name pattern
    val apis = discovery.findByName("api-*")
    
    // Lookup by ID
    val service = discovery.lookup("api-1")
}
```

### 7. Connection Pool

```kotlin
import com.hivellm.umicp.pool.*

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
    
    // Get stats
    val stats = pool.getStats()
    println("Active: ${stats.activeConnections}")
    
    pool.close()
}
```

## Why Choose Kotlin?

### 1. Null Safety
```kotlin
// Compile-time null safety
val envelope: Envelope  // Cannot be null
val hint: PayloadHint?  // Nullable

// No more NullPointerException!
```

### 2. Coroutines
```kotlin
// Lightweight async/await
launch {
    client.connect()  // Suspend function
    client.send(msg)  // Non-blocking
}
```

### 3. Extension Functions
```kotlin
// Natural syntax
val result = vector1 dot vector2
val normalized = vector.normalize()
```

### 4. Operator Overloading
```kotlin
// Math-like syntax
val sum = vector1 + vector2
val diff = vector1 - vector2
```

### 5. DSL Builders
```kotlin
// Type-safe configuration
val envelope = Envelope.build {
    from("client")
    to("server")
    operation(OperationType.DATA)
}
```

### 6. Data Classes
```kotlin
// Automatic equals, hashCode, copy, toString
data class ServiceInfo(
    val id: String,
    val name: String,
    val endpoint: String
)
```

### 7. Sealed Classes
```kotlin
// Type-safe result handling
when (val result = Matrix.dotProduct(a, b)) {
    is MatrixResult.Success -> println(result.value)
    is MatrixResult.Failure -> println(result.error)
}
```

## Examples

The SDK includes 4 complete examples:

```bash
# Basic usage
./gradlew run --args="com.hivellm.umicp.examples.BasicExample"

# Matrix operations
./gradlew run --args="com.hivellm.umicp.examples.MatrixExample"

# WebSocket communication
./gradlew run --args="com.hivellm.umicp.examples.WebSocketExample"

# Service discovery
./gradlew run --args="com.hivellm.umicp.examples.ServiceDiscoveryExample"
```

## Testing

```bash
# Run all tests
./gradlew test

# Run with coverage
./gradlew test jacocoTestReport

# Run specific test
./gradlew test --tests "com.hivellm.umicp.core.EnvelopeTest"
```

## Documentation

- **README**: Complete API reference
- **Examples**: 4 working examples
- **KDoc**: All public APIs documented
- **Tests**: 100+ tests as documentation

## Next Steps

1. Read the [README](README.md) for complete API reference
2. Check out the [examples](src/main/kotlin/com/hivellm/umicp/examples/)
3. Review the [tests](src/test/kotlin/com/hivellm/umicp/) for usage patterns
4. See [CONTRIBUTING](CONTRIBUTING.md) to contribute

## Need Help?

- **GitHub Issues**: [Report bugs](https://github.com/hivellm/umicp/issues)
- **Discussions**: [Ask questions](https://github.com/hivellm/umicp/discussions)
- **Documentation**: [Complete docs](../../docs/)

---

**Version**: 0.1.2  
**License**: MIT  
**Status**: ✅ Production Ready

