# UMICP Java Bindings - Technical Guide

> **📝 COMPLETE TECHNICAL GUIDE**
>
> This guide provides comprehensive technical information for developers using UMICP Java bindings.

**Version**: 0.2.0  
**Last Updated**: October 11, 2025  
**Target Audience**: Java Developers  

---

## 📚 Table of Contents

1. [Quick Start](#quick-start)
2. [Core Concepts](#core-concepts)
3. [Maven Setup](#maven-setup)
4. [Basic Usage](#basic-usage)
5. [WebSocket Transport](#websocket-transport)
6. [HTTP/2 Transport](#http2-transport)
7. [Advanced Features](#advanced-features)
8. [Best Practices](#best-practices)
9. [Troubleshooting](#troubleshooting)

---

## 🚀 Quick Start

### 1. Add Dependency

```xml
<dependencies>
    <dependency>
        <groupId>com.hivellm</groupId>
        <artifactId>umicp-core</artifactId>
        <version>0.2.0</version>
    </dependency>
    
    <dependency>
        <groupId>com.hivellm</groupId>
        <artifactId>umicp-transport</artifactId>
        <version>0.2.0</version>
    </dependency>
</dependencies>
```

### 2. Create First Envelope

```java
import com.hivellm.umicp.core.*;
import com.hivellm.umicp.types.*;

public class QuickStart {
    public static void main(String[] args) {
        // Create envelope
        Envelope envelope = new Envelope.Builder()
            .from("client-001")
            .to("server-001")
            .operation(OperationType.DATA)
            .payload("Hello UMICP!".getBytes())
            .payloadType(PayloadType.TEXT)
            .build();
        
        // Serialize to JSON
        String json = envelope.toJson();
        System.out.println(json);
        
        // Deserialize
        Envelope received = Envelope.fromJson(json);
        System.out.println("Received: " + new String(received.getPayload()));
    }
}
```

---

## 🎯 Core Concepts

### Envelope

The `Envelope` is the core message container in UMICP.

**Structure**:
```java
public class Envelope {
    private String from;                      // Sender ID
    private String to;                        // Recipient ID
    private OperationType operation;          // Operation type
    private String messageId;                 // Unique message ID
    private long timestamp;                   // Unix timestamp (ms)
    private Map<String, String> capabilities; // Key-value metadata
    private byte[] payload;                   // Binary payload
    private PayloadType payloadType;          // Payload type hint
    private EncodingType encoding;            // Encoding type
}
```

**Operations**:
```java
public enum OperationType {
    DATA,       // Regular data message
    CONTROL,    // Control message (handshake, etc)
    ACK,        // Acknowledgment
    ERROR,      // Error message
    REQUEST,    // Request message
    RESPONSE,   // Response message
    BROADCAST,  // Broadcast message
    PING,       // Ping message
    PONG,       // Pong response
    SUBSCRIBE,  // Subscribe to topic
    UNSUBSCRIBE,// Unsubscribe from topic
    PUBLISH,    // Publish to topic
    HEARTBEAT,  // Heartbeat
    SHUTDOWN    // Shutdown signal
}
```

---

## 🔧 Maven Setup

### Parent POM

```xml
<project>
    <groupId>com.hivellm</groupId>
    <artifactId>umicp-java-parent</artifactId>
    <version>0.2.0</version>
    <packaging>pom</packaging>
    
    <modules>
        <module>umicp-core</module>
        <module>umicp-transport</module>
        <module>umicp-examples</module>
    </modules>
    
    <properties>
        <java.version>17</java.version>
        <maven.compiler.source>17</maven.compiler.source>
        <maven.compiler.target>17</maven.compiler.target>
    </properties>
</project>
```

### Build Commands

```bash
# Build all modules
mvn clean install

# Run tests
mvn test

# Generate JavaDoc
mvn javadoc:javadoc

# Generate coverage report
mvn jacoco:report

# Run specific example
mvn exec:java -pl umicp-examples \
  -Dexec.mainClass="com.hivellm.umicp.examples.BasicEnvelopeExample"
```

---

## 📝 Basic Usage

### Creating Envelopes

**Simple Envelope**:
```java
Envelope envelope = new Envelope.Builder()
    .from("client-001")
    .to("server-001")
    .operation(OperationType.DATA)
    .build();
```

**With Payload**:
```java
String message = "Hello UMICP!";
Envelope envelope = new Envelope.Builder()
    .from("client-001")
    .to("server-001")
    .operation(OperationType.DATA)
    .payload(message.getBytes(StandardCharsets.UTF_8))
    .payloadType(PayloadType.TEXT)
    .build();
```

**With Metadata**:
```java
Envelope envelope = new Envelope.Builder()
    .from("client-001")
    .to("server-001")
    .operation(OperationType.DATA)
    .addCapability("priority", "high")
    .addCapability("ttl", "60000")
    .addCapability("retry", "3")
    .build();
```

### Matrix Operations

```java
import com.hivellm.umicp.core.Matrix;

// Dot product
float[] a = {1.0f, 2.0f, 3.0f};
float[] b = {4.0f, 5.0f, 6.0f};
float dotProduct = Matrix.dotProduct(a, b); // 32.0

// Cosine similarity
float similarity = Matrix.cosineSimilarity(a, b); // 0.974

// Matrix multiplication
float[][] matrix = {{1, 2}, {3, 4}};
float[] vector = {5, 6};
float[] result = Matrix.matrixVectorMultiply(matrix, vector); // [17, 39]

// Normalize vector
float[] normalized = Matrix.normalize(a);
```

---

## 🌐 WebSocket Transport

### WebSocket Client

**Basic Client**:
```java
import com.hivellm.umicp.transport.*;

ClientOptions options = ClientOptions.builder()
    .url("ws://localhost:8080/umicp")
    .autoReconnect(true)
    .maxReconnectAttempts(5)
    .reconnectDelay(Duration.ofSeconds(2))
    .heartbeatInterval(Duration.ofSeconds(30))
    .build();

UMICPWebSocketClient client = new UMICPWebSocketClient(options);

// Set up event listeners
client.setEventListener(new TransportEventListener() {
    @Override
    public void onConnected() {
        System.out.println("Connected!");
    }
    
    @Override
    public void onMessage(Envelope envelope) {
        System.out.println("Received: " + envelope.getFrom());
    }
    
    @Override
    public void onDisconnected(String reason) {
        System.out.println("Disconnected: " + reason);
    }
    
    @Override
    public void onError(Throwable error) {
        System.err.println("Error: " + error.getMessage());
    }
});

// Connect
client.connect();

// Send envelope
Envelope envelope = new Envelope.Builder()
    .from("client-001")
    .to("server-001")
    .operation(OperationType.DATA)
    .payload("Hello!".getBytes())
    .build();
    
client.send(envelope);

// Disconnect
client.disconnect();
```

### WebSocket Server

**Basic Server**:
```java
ServerOptions options = ServerOptions.builder()
    .host("0.0.0.0")
    .port(8080)
    .path("/umicp")
    .maxConnections(1000)
    .build();

UMICPWebSocketServer server = new UMICPWebSocketServer(options);

// Set up event listeners
server.setEventListener(new ServerEventListener() {
    @Override
    public void onClientConnected(String clientId) {
        System.out.println("Client connected: " + clientId);
    }
    
    @Override
    public void onClientMessage(String clientId, Envelope envelope) {
        System.out.println("Message from " + clientId);
        
        // Echo back
        Envelope response = new Envelope.Builder()
            .from("server-001")
            .to(envelope.getFrom())
            .operation(OperationType.RESPONSE)
            .payload("Echo: ".getBytes())
            .build();
        
        server.sendToClient(clientId, response);
    }
    
    @Override
    public void onClientDisconnected(String clientId, String reason) {
        System.out.println("Client disconnected: " + clientId);
    }
});

// Start server
server.start();

// Broadcast to all clients
Envelope broadcast = new Envelope.Builder()
    .from("server-001")
    .to("*")
    .operation(OperationType.BROADCAST)
    .payload("Announcement".getBytes())
    .build();
    
server.broadcast(broadcast);

// Stop server
server.stop();
```

### Multiplexed Peer (Client + Server)

**Multiplexed Peer**:
```java
import com.hivellm.umicp.transport.*;

PeerOptions options = PeerOptions.builder()
    .peerId("peer-001")
    .serverHost("0.0.0.0")
    .serverPort(8080)
    .serverPath("/umicp")
    .autoHandshake(true)
    .build();

UMICPWebSocketPeer peer = new UMICPWebSocketPeer(options);

// Set up event listener
peer.setEventListener(new PeerEventListener() {
    @Override
    public void onPeerConnected(String peerId, boolean isIncoming) {
        System.out.println("Peer connected: " + peerId + 
            " (incoming=" + isIncoming + ")");
    }
    
    @Override
    public void onPeerReady(String peerId, PeerInfo info) {
        System.out.println("Peer ready: " + peerId);
        System.out.println("Handshake complete!");
    }
    
    @Override
    public void onPeerMessage(String peerId, Envelope envelope) {
        System.out.println("Message from peer: " + peerId);
    }
    
    @Override
    public void onPeerDisconnected(String peerId) {
        System.out.println("Peer disconnected: " + peerId);
    }
});

// Start peer (starts internal server)
peer.start();

// Connect to remote peer (outgoing connection)
peer.connectToPeer("ws://localhost:8081/umicp");

// Send to specific peer
peer.sendToPeer("peer-002", envelope);

// Broadcast to all peers
peer.broadcastToAll(envelope);

// Get peer statistics
PeerStats stats = peer.getStats();
System.out.println("Total peers: " + stats.getTotalPeers());
System.out.println("Incoming: " + stats.getIncomingConnections());
System.out.println("Outgoing: " + stats.getOutgoingConnections());

// Shutdown
peer.shutdown();
```

---

## 🔥 HTTP/2 Transport

### HTTP/2 Client

```java
import com.hivellm.umicp.transport.http.*;

HttpClientOptions options = HttpClientOptions.builder()
    .baseUrl("http://localhost:8080")
    .connectTimeout(Duration.ofSeconds(10))
    .readTimeout(Duration.ofSeconds(30))
    .build();

UMICPHttpClient client = new UMICPHttpClient(options);

// Send request
Envelope request = new Envelope.Builder()
    .from("client-001")
    .to("server-001")
    .operation(OperationType.REQUEST)
    .payload("data".getBytes())
    .build();

Envelope response = client.send(request);
System.out.println("Response: " + new String(response.getPayload()));

// Close client
client.close();
```

### HTTP/2 Server

```java
import com.hivellm.umicp.transport.http.*;

HttpServerOptions options = HttpServerOptions.builder()
    .port(8080)
    .contextPath("/umicp")
    .maxConcurrentStreams(100)
    .build();

UMICPHttpServer server = new UMICPHttpServer(options);

// Set request handler
server.setRequestHandler((request) -> {
    // Process request
    System.out.println("Received request from: " + request.getFrom());
    
    // Create response
    return new Envelope.Builder()
        .from("server-001")
        .to(request.getFrom())
        .operation(OperationType.RESPONSE)
        .payload("Processed".getBytes())
        .build();
});

// Start server
server.start();

// Stop server
server.stop();
```

---

## ⚡ Advanced Features

### Compression

```java
import com.hivellm.umicp.compression.*;

// Compress data
byte[] data = "Large payload...".getBytes();
byte[] compressed = Compression.compress(data, CompressionType.GZIP);

// Decompress data
byte[] decompressed = Compression.decompress(compressed, CompressionType.GZIP);

// Auto-detect compression
CompressionType type = Compression.detectCompression(compressed);

// Using CompressionManager
CompressionManager manager = new CompressionManager();
manager.setDefaultCompression(CompressionType.GZIP);
manager.setCompressionThreshold(1024); // Compress if > 1KB

byte[] result = manager.compressIfNeeded(data);
```

### Service Discovery

```java
import com.hivellm.umicp.discovery.*;

ServiceDiscovery discovery = ServiceDiscovery.getInstance();

// Register service
ServiceInfo service = new ServiceInfo(
    "my-service",
    "ws://localhost:8080/umicp",
    "websocket",
    Map.of("version", "1.0", "region", "us-east")
);
discovery.register(service);

// Find services
List<ServiceInfo> services = discovery.findByName("my-service");
List<ServiceInfo> byType = discovery.findByType("websocket");
List<ServiceInfo> byTag = discovery.findByTag("region", "us-east");

// Unregister
discovery.unregister("my-service");
```

### Connection Pooling

```java
import com.hivellm.umicp.transport.pool.*;

PoolConfig config = PoolConfig.builder()
    .maxPoolSize(10)
    .minIdleConnections(2)
    .maxIdleTime(Duration.ofMinutes(5))
    .validationInterval(Duration.ofSeconds(30))
    .build();

ConnectionPool pool = new ConnectionPool(config);

// Get connection from pool
PooledConnection conn = pool.acquire();

// Use connection
conn.send(envelope);

// Return to pool
pool.release(conn);

// Get statistics
PoolStats stats = pool.getStats();
System.out.println("Active: " + stats.getActiveConnections());
System.out.println("Idle: " + stats.getIdleConnections());

// Shutdown pool
pool.shutdown();
```

### Event System

```java
import com.hivellm.umicp.events.*;

EventEmitter emitter = new EventEmitter();

// Register event handler
emitter.on("custom-event", (data) -> {
    System.out.println("Event received: " + data);
});

// Emit event
emitter.emit("custom-event", "Hello Events!");

// Remove handler
emitter.off("custom-event", handler);

// Type-safe events
emitter.on("envelope-event", new TypedEventListener<Envelope>() {
    @Override
    public void handle(Envelope envelope) {
        System.out.println("Envelope: " + envelope.getMessageId());
    }
});
```

---

## 💡 Best Practices

### 1. Resource Management

**Always close resources**:
```java
// Use try-with-resources
try (UMICPWebSocketClient client = new UMICPWebSocketClient(options)) {
    client.connect();
    client.send(envelope);
} // Automatically closed

// Or explicit close
UMICPWebSocketClient client = new UMICPWebSocketClient(options);
try {
    client.connect();
    client.send(envelope);
} finally {
    client.disconnect();
}
```

### 2. Error Handling

**Handle all exceptions**:
```java
try {
    client.send(envelope);
} catch (TransportException e) {
    logger.error("Transport error: " + e.getMessage(), e);
    // Retry or failover
} catch (ValidationException e) {
    logger.error("Validation error: " + e.getMessage(), e);
    // Fix envelope and retry
} catch (UMICPException e) {
    logger.error("UMICP error: " + e.getMessage(), e);
    // Generic error handling
}
```

### 3. Thread Safety

**Use concurrent data structures**:
```java
// Thread-safe storage
private final Map<String, Envelope> messages = new ConcurrentHashMap<>();

// Atomic operations
private final AtomicInteger messageCount = new AtomicInteger(0);
messageCount.incrementAndGet();

// Synchronization when needed
private final Object lock = new Object();
synchronized(lock) {
    // Critical section
}
```

### 4. Connection Pooling

**Use pools for high-throughput applications**:
```java
// Create pool once
ConnectionPool pool = new ConnectionPool(config);

// Reuse connections
for (int i = 0; i < 1000; i++) {
    PooledConnection conn = pool.acquire();
    try {
        conn.send(envelope);
    } finally {
        pool.release(conn);
    }
}
```

### 5. Logging

**Use SLF4J**:
```java
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

private static final Logger logger = LoggerFactory.getLogger(MyClass.class);

logger.debug("Debug message");
logger.info("Info message");
logger.warn("Warning message");
logger.error("Error message", exception);
```

---

## 🐛 Troubleshooting

### Connection Issues

**Problem**: Client won't connect
```java
// Check URL format
String url = "ws://localhost:8080/umicp"; // Correct
String url = "ws://localhost:8080";       // Missing path - wrong

// Enable auto-reconnect
ClientOptions options = ClientOptions.builder()
    .url("ws://localhost:8080/umicp")
    .autoReconnect(true)
    .maxReconnectAttempts(10)
    .reconnectDelay(Duration.ofSeconds(5))
    .build();
```

### Performance Issues

**Problem**: Low throughput
```java
// 1. Use connection pooling
ConnectionPool pool = new ConnectionPool(config);

// 2. Increase buffer sizes
ClientOptions options = ClientOptions.builder()
    .url("ws://localhost:8080/umicp")
    .sendBufferSize(64 * 1024)
    .receiveBufferSize(64 * 1024)
    .build();

// 3. Enable compression
CompressionManager manager = new CompressionManager();
manager.setDefaultCompression(CompressionType.GZIP);
```

### Memory Leaks

**Problem**: Memory usage grows
```java
// 1. Always close resources
client.disconnect();
server.stop();
pool.shutdown();

// 2. Clear collections
messages.clear();
cache.invalidateAll();

// 3. Remove event listeners
emitter.off("event", handler);

// 4. Use weak references for caches
private final Map<String, WeakReference<Envelope>> cache = new ConcurrentHashMap<>();
```

### Debugging

**Enable debug logging**:
```xml
<!-- logback.xml -->
<configuration>
    <appender name="STDOUT" class="ch.qos.logback.core.ConsoleAppender">
        <encoder>
            <pattern>%d{HH:mm:ss.SSS} [%thread] %-5level %logger{36} - %msg%n</pattern>
        </encoder>
    </appender>
    
    <logger name="com.hivellm.umicp" level="DEBUG"/>
    
    <root level="INFO">
        <appender-ref ref="STDOUT"/>
    </root>
</configuration>
```

---

## 📚 Additional Resources

### Documentation
- [README.md](./README.md) - Project overview
- [STATUS.md](./STATUS.md) - Implementation status
- [ROADMAP.md](./ROADMAP.md) - Development plan
- [REVIEWS.md](./REVIEWS.md) - Quality reviews

### Examples
- [BasicEnvelopeExample](../examples/BasicEnvelopeExample.java)
- [MultiplexedPeerExample](../examples/MultiplexedPeerExample.java)
- [More examples](../umicp-examples/src/main/java/com/hivellm/umicp/examples/)

### External Links
- [Project Repository](https://github.com/hivellm/umicp)
- [Issue Tracker](https://github.com/hivellm/umicp/issues)
- [TypeScript Bindings](../typescript/)

---

**Version**: 0.2.0  
**Last Updated**: October 11, 2025  
**Maintainer**: HiveLLM Contributors

---

*For questions or issues, please open a GitHub issue or contact the development team.*

