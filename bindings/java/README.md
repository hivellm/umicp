# UMICP Java SDK

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Java](https://img.shields.io/badge/Java-17%2B-orange.svg)](https://openjdk.org/)
[![Status](https://img.shields.io/badge/Phase%204-Complete-green.svg)](#)
[![Tests](https://img.shields.io/badge/Tests-380%2B%20passing-brightgreen.svg)](#)
[![Coverage](https://img.shields.io/badge/Coverage-97%25-brightgreen.svg)](#)

Java bindings for the Universal Matrix Inter-Communication Protocol (UMICP), providing high-performance communication and matrix operations for distributed systems, federated learning, and real-time applications.

## 🎉 Phase 4 Complete - Full Feature Parity!

**Status**: Phase 4 (Compression & HTTP/2) - ✅ **COMPLETE**
- ✅ 43 production classes (~9,100 LOC)
- ✅ 380+ test cases (~97% coverage)
- ✅ Full WebSocket client/server/peer implementation
- ✅ **HTTP/2 Client** - Native Java 11+ HttpClient ⭐ NEW
- ✅ **Compression** - GZIP/DEFLATE support ⭐ NEW
- ✅ **Service Discovery** - Complete with metadata and capabilities
- ✅ **Connection Pooling** - Complete with lifecycle management
- ✅ Integration tests included
- ✅ Production-ready code quality

See [IMPLEMENTATION_STATUS_UPDATE.md](./docs/IMPLEMENTATION_STATUS_UPDATE.md) for details.

## 📋 Features

### Core Features ✅
- **🔗 Type-Safe Envelopes**: Strongly-typed message serialization and validation
- **⚡ High Performance**: Optimized matrix operations (pure Java)
- **📦 Builder Pattern**: Fluent API for easy configuration
- **🛡️ Defensive Coding**: Immutability and thread-safety built-in
- **✅ Comprehensive Testing**: 274 tests with ~95% coverage
- **📚 Full Documentation**: Complete JavaDoc for all public APIs

### Transport Features ✅
- **🌐 WebSocket Client**: Auto-reconnect, heartbeat, message queuing
- **🖥️ WebSocket Server**: Multi-client, broadcast, statistics tracking
- **🔄 Multiplexed Peer**: Combined client+server, P2P communication
- **🤝 Auto-Handshake**: Automatic peer discovery and capability exchange
- **📊 Statistics**: Real-time message, byte, latency tracking
- **⚡ Event-Driven**: Non-blocking, asynchronous operations

### Advanced Features ✅
- **🔍 Service Discovery**: Capability-based service discovery with metadata
- **🏊 Connection Pooling**: Efficient connection management with min/max sizing
- **🧹 Auto-Cleanup**: Background cleanup of stale connections and services
- **📈 Pool Statistics**: Detailed metrics for connection usage
- **🗜️ Compression**: GZIP/DEFLATE compression for efficient data transfer ⭐ NEW
- **🌐 HTTP/2 Client**: Native HTTP/2 support using Java 11+ HttpClient ⭐ NEW
- **⚡ Async HTTP**: CompletableFuture-based async requests ⭐ NEW

### API Design
- Fluent builder pattern for configuration
- CompletableFuture-based async API
- Event-driven architecture
- try-with-resources support (AutoCloseable)
- Type-safe result wrappers

## 📦 Installation

### Maven
```xml
<dependency>
    <groupId>com.HiveHub</groupId>
    <artifactId>umicp-core</artifactId>
    <version>0.1.3</version>
</dependency>
```

### Gradle
```gradle
implementation 'com.HiveHub:umicp-core:0.1.3'
```

### Prerequisites
- **Java**: 17+ (Java 21+ recommended)
- **Maven**: 3.8+ or **Gradle**: 8+

## 🚀 Quick Start

### Basic Envelope Usage

```java
import com.HiveHub.umicp.core.*;
import com.HiveHub.umicp.types.*;
import java.util.UUID;

// Create envelope using fluent API
Envelope envelope = UMICP.createEnvelope(
    UMICP.envelopeOptions()
        .from("client-001")
        .to("server-001")
        .operation(OperationType.DATA)
        .messageId(UUID.randomUUID().toString())
        .capability("priority", "high")
        .build()
);

// Validate
if (UMICP.validate(envelope)) {
    System.out.println("✓ Valid envelope");
}

// Serialize to JSON
String json = UMICP.serialize(envelope);
System.out.println(json);

// Deserialize
Envelope received = UMICP.deserialize(json);

// Compute hash
String hash = UMICP.hash(envelope);
```

### Matrix Operations

```java
import com.HiveHub.umicp.core.*;

try (Matrix matrix = UMICP.createMatrix()) {
    // Vector operations
    float[] a = {1.0f, 2.0f, 3.0f};
    float[] b = {4.0f, 5.0f, 6.0f};
    
    // Dot product
    MatrixResult dotResult = matrix.dotProduct(a, b);
    System.out.println("Dot product: " + dotResult.getResult()); // 32.0
    
    // Cosine similarity
    MatrixResult simResult = matrix.cosineSimilarity(a, b);
    System.out.println("Similarity: " + simResult.getSimilarity()); // 0.9746
    
    // Vector addition
    float[] result = new float[3];
    matrix.vectorAdd(a, b, result);
    System.out.println("Sum: " + Arrays.toString(result)); // [5.0, 7.0, 9.0]
    
    // Matrix multiplication
    float[] matrixA = {1, 2, 3, 4}; // 2x2
    float[] matrixB = {5, 6, 7, 8}; // 2x2
    float[] matrixResult = new float[4];
    matrix.matrixMultiply(matrixA, matrixB, matrixResult, 2, 2, 2);
    // Result: [19, 22, 43, 50]
}
```

### Service Discovery Example

```java
import com.HiveHub.umicp.discovery.*;

// Create service discovery manager
ServiceDiscovery discovery = new ServiceDiscovery();

// Register local service
ServiceInfo localService = new ServiceInfo(
    "my-service",
    "My Application",
    "ws://localhost:8080"
);
localService.addCapability("storage");
localService.addCapability("compute");
localService.addMetadata("region", "us-east-1");

discovery.registerLocal(localService);

// Discover remote services
ServiceInfo remoteService = new ServiceInfo(
    "remote-service",
    "Remote ML Service",
    "ws://192.168.1.10:8080"
);
remoteService.addCapability("ml");
remoteService.addCapability("training");

discovery.registerService(remoteService);

// Find services by capability
List<ServiceInfo> mlServices = discovery.findByCapability("ml");
for (ServiceInfo service : mlServices) {
    System.out.println("Found ML service: " + service.getName());
}

// Find by metadata
List<ServiceInfo> eastServices = discovery.findByMetadata("region", "us-east-1");

// Cleanup stale services
int removed = discovery.cleanupStaleServices();
```

### Connection Pool Example

```java
import com.HiveHub.umicp.transport.pool.*;

// Create pool configuration
PoolConfig config = new PoolConfig()
    .setAddress("ws://localhost:8080")
    .setMinSize(2)
    .setMaxSize(10)
    .setMaxAgeSeconds(600)
    .setIdleTimeoutSeconds(300);

// Create and initialize pool
try (ConnectionPool pool = new ConnectionPool(config)) {
    pool.initialize();
    
    // Acquire connection
    PooledConnection conn = pool.acquire();
    
    try {
        // Use connection
        UMICPWebSocketClient client = conn.getClient();
        // ... send messages
        
    } finally {
        // Release connection back to pool
        pool.release(conn);
    }
    
    // Get pool statistics
    PoolStats stats = pool.getStats();
    System.out.println("Total connections: " + stats.getTotalConnections());
    System.out.println("In use: " + stats.getInUseConnections());
    
    // Start background cleanup
    pool.startCleanupTask(60); // Cleanup every 60 seconds
}
```

### Complete Example

```java
public class UMICPExample {
    public static void main(String[] args) throws SerializationException {
        // Print version
        UMICP.printVersion();
        // Output: UMICP Java SDK v1.0.0 (Protocol: 1.0, Build: java-pure)
        
        // Create envelope with payload hint
        Envelope envelope = UMICP.createEnvelope(
            UMICP.envelopeOptions()
                .from("ml-client")
                .to("ml-server")
                .operation(OperationType.DATA)
                .messageId(UUID.randomUUID().toString())
                .payloadHint(
                    UMICP.payloadHint()
                        .type(PayloadType.VECTOR)
                        .encoding(EncodingType.FLOAT32)
                        .count(768) // 768-dimensional embedding
                        .build()
                )
                .build()
        );
        
        // Process envelope
        envelope.validateOrThrow();
        String json = UMICP.serialize(envelope);
        
        // Matrix operations on embeddings
        try (Matrix matrix = UMICP.createMatrix()) {
            float[] embedding1 = new float[768]; // Your embedding
            float[] embedding2 = new float[768]; // Another embedding
            
            MatrixResult similarity = matrix.cosineSimilarity(embedding1, embedding2);
            if (similarity.isSuccess()) {
                System.out.println("Similarity: " + similarity.getSimilarity());
            }
        }
    }
}
```

## 📚 API Reference

### Core Classes

#### UMICP
Main utility class with static factory methods.

```java
// Factory methods
Envelope envelope = UMICP.createEnvelope();
Envelope envelope = UMICP.createEnvelope(options);
Matrix matrix = UMICP.createMatrix();

// Builders
EnvelopeOptions.Builder options = UMICP.envelopeOptions();
PayloadHint.Builder hint = UMICP.payloadHint();

// Serialization
String json = UMICP.serialize(envelope);
Envelope envelope = UMICP.deserialize(json);

// Validation
boolean valid = UMICP.validate(envelope);
String hash = UMICP.hash(envelope);

// Version info
String version = UMICP.getVersion();
UMICP.printVersion();
```

#### Envelope
Message envelope for protocol communication.

```java
envelope.setFrom(String from)
envelope.setTo(String to)
envelope.setOperation(OperationType operation)
envelope.setMessageId(String messageId)
envelope.addCapability(String key, String value)
envelope.setPayloadHint(PayloadHint hint)

envelope.getFrom()
envelope.getTo()
envelope.getOperation()
envelope.getMessageId()
envelope.getCapabilities()
envelope.getPayloadHint()

envelope.serialize()
envelope.validate()
envelope.validateOrThrow()
envelope.getHash()
```

#### Matrix
Matrix and vector operations.

```java
// Vector operations
matrix.vectorAdd(float[] a, float[] b, float[] result)
matrix.vectorSubtract(float[] a, float[] b, float[] result)
matrix.vectorMultiply(float[] a, float[] b, float[] result)

// Advanced operations
matrix.dotProduct(float[] a, float[] b)
matrix.cosineSimilarity(float[] a, float[] b)
matrix.normalize(float[] vector)
matrix.magnitude(float[] vector)

// Matrix operations
matrix.matrixMultiply(float[] a, float[] b, float[] result, int m, int n, int p)
matrix.transpose(float[] input, float[] output, int rows, int cols)
```

#### MatrixResult
Result wrapper for matrix operations.

```java
result.isSuccess()
result.isFailure()
result.getError()
result.getResult()          // Scalar result
result.getSimilarity()      // Similarity score
result.getData()            // Array data

result.getResultOrThrow()
result.getSimilarityOrThrow()
result.getDataOrThrow()
```

### Type Definitions

#### OperationType
```java
CONTROL, DATA, ACK, ERROR, REQUEST, RESPONSE
```

#### PayloadType
```java
VECTOR, TEXT, METADATA, BINARY
```

#### EncodingType
```java
FLOAT32, FLOAT64, INT32, INT64, UINT8, UINT16, UINT32, UINT64
```

## 🎯 Use Cases

### Federated Learning
```java
// Distribute ML model weights
Envelope weightsEnvelope = UMICP.createEnvelope(
    UMICP.envelopeOptions()
        .from("coordinator")
        .to("worker-001")
        .operation(OperationType.DATA)
        .payloadHint(
            UMICP.payloadHint()
                .type(PayloadType.VECTOR)
                .encoding(EncodingType.FLOAT32)
                .count(10000)
                .build()
        )
        .build()
);
```

### IoT Data Streaming
```java
// Sensor data
Envelope sensorData = UMICP.createEnvelope(
    UMICP.envelopeOptions()
        .from("sensor-temp-001")
        .to("data-collector")
        .operation(OperationType.DATA)
        .capability("sensor-type", "temperature")
        .capability("value", "23.5")
        .capability("unit", "celsius")
        .build()
);
```

### Microservices Communication
```java
// Service-to-service request
Envelope request = UMICP.createEnvelope(
    UMICP.envelopeOptions()
        .from("api-gateway")
        .to("auth-service")
        .operation(OperationType.REQUEST)
        .messageId(UUID.randomUUID().toString())
        .capability("action", "validate-token")
        .build()
);
```

## 🧪 Testing

```bash
# Maven
mvn test

# Gradle
gradle test

# With coverage
mvn test jacoco:report
```

Current Test Status:
- ✅ **380+ test cases** passing
- ✅ **97% coverage** of all classes
- ✅ **Zero failures**
- ✅ Service Discovery tests (28 tests)
- ✅ Connection Pool tests (28 tests)
- ✅ **NEW**: Compression tests (30 tests)
- ✅ **NEW**: HTTP/2 tests (20 tests)

## 📈 Performance

Pure Java implementation (current):
- **Envelope Creation**: ~0.5ms
- **Serialization**: ~2ms
- **Dot Product** (1000 elements): ~0.1ms
- **Matrix Multiply** (100x100): ~10ms

JNI implementation (planned):
- Expected 2-5x performance improvement
- SIMD acceleration
- Native memory management

## 🔧 Building from Source

```bash
# Clone repository
git clone https://github.com/HiveHub/umicp.git
cd umicp/bindings/java

# Build with Maven
mvn clean install

# Or with Gradle
gradle build

# Run tests
mvn test
gradle test
```

## 📖 Documentation

- [Implementation Roadmap](./docs/IMPLEMENTATION_ROADMAP.md)
- [Executive Summary](./docs/EXECUTIVE_SUMMARY.md)
- [Project Structure](./docs/PROJECT_STRUCTURE.md)
- [Phase 1 Complete](./IMPLEMENTATION_COMPLETE.md)
- [Changelog](./CHANGELOG.md)
- [Contributing](./CONTRIBUTING.md)

## 🗺️ Roadmap

### Phase 1: Foundation ✅ **COMPLETE**
- ✅ Core types and exceptions
- ✅ Envelope with JSON serialization
- ✅ Matrix operations (pure Java)
- ✅ UMICP utility class
- ✅ 139 comprehensive tests

### Phase 2: WebSocket Transport ✅ **COMPLETE**
- ✅ WebSocket client
- ✅ WebSocket server
- ✅ Multiplexed peer architecture
- ✅ Auto-handshake protocol
- ✅ Event system
- ✅ 274 tests

### Phase 3: Service Discovery & Connection Pooling ✅ **COMPLETE**
- ✅ Service Discovery with metadata and capabilities
- ✅ Connection Pooling with min/max sizing
- ✅ Auto-cleanup of stale services/connections
- ✅ Pool statistics and monitoring
- ✅ 56 additional tests
- ✅ Example programs

### Phase 4: Compression & HTTP/2 ✅ **COMPLETE**
- ✅ HTTP/2 client (Java 11+ HttpClient)
- ✅ Compression (GZIP/DEFLATE)
- ✅ Async HTTP support (CompletableFuture)
- ✅ 50 additional tests
- ✅ Example programs

### Phase 5: Advanced Features 📋 **PLANNED**
- 📋 LZ4 compression
- 📋 HTTP/2 server
- 📋 Advanced security features
- 📋 Load balancing
- 📋 JNI bindings (optional)
- [ ] Production deployment guide
- [ ] Maven Central publication
