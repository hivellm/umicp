# UMICP Java Bindings - Project Structure

This document describes the planned structure of the UMICP Java bindings project.

## 📁 Directory Structure

```
bindings/java/
│
├── pom.xml                          # Maven parent POM
├── build.gradle.kts                 # Gradle build script (alternative)
├── README.md                        # Main project README
├── IMPLEMENTATION_ROADMAP.md        # Detailed implementation roadmap
├── CONTRIBUTING.md                  # Contribution guidelines
├── CHANGELOG.md                     # Version history
├── LICENSE                          # License file
├── .gitignore                       # Git ignore rules
│
├── docs/                            # Documentation
│   ├── INSTALLATION.md              # Installation guide
│   ├── QUICKSTART.md                # Quick start guide
│   ├── API_REFERENCE.md             # Complete API reference
│   ├── MULTIPLEXED_PEER.md          # Multiplexed peer guide
│   ├── WEBSOCKET_TRANSPORT.md       # WebSocket transport guide
│   ├── EVENT_SYSTEM.md              # Event handling patterns
│   ├── PERFORMANCE.md               # Performance tuning guide
│   ├── MIGRATION_FROM_TYPESCRIPT.md # Migration guide
│   └── PROJECT_STRUCTURE.md         # This file
│
├── native/                          # JNI native code
│   ├── CMakeLists.txt               # CMake build configuration
│   ├── src/                         # Native source files
│   │   ├── jni_envelope.cpp         # Envelope JNI bindings
│   │   ├── jni_matrix.cpp           # Matrix JNI bindings
│   │   ├── jni_frame.cpp            # Frame JNI bindings
│   │   └── jni_transport.cpp        # Transport JNI bindings
│   ├── include/                     # Native headers
│   └── lib/                         # Pre-compiled native libraries
│       ├── linux-x64/
│       ├── linux-arm64/
│       ├── macos-x64/
│       ├── macos-arm64/
│       └── windows-x64/
│
├── umicp-core/                      # Core module
│   ├── pom.xml                      # Module POM
│   └── src/
│       ├── main/
│       │   ├── java/
│       │   │   └── com/hivellm/umicp/
│       │   │       ├── core/
│       │   │       │   ├── Envelope.java
│       │   │       │   ├── EnvelopeOptions.java
│       │   │       │   ├── Frame.java
│       │   │       │   ├── Matrix.java
│       │   │       │   ├── MatrixResult.java
│       │   │       │   ├── PayloadHint.java
│       │   │       │   └── UMICP.java
│       │   │       ├── types/
│       │   │       │   ├── OperationType.java
│       │   │       │   ├── PayloadType.java
│       │   │       │   ├── EncodingType.java
│       │   │       │   ├── UMICPException.java
│       │   │       │   ├── TransportException.java
│       │   │       │   ├── SerializationException.java
│       │   │       │   └── ConnectionException.java
│       │   │       └── util/
│       │   │           ├── UUID.java
│       │   │           ├── Hash.java
│       │   │           ├── Timestamp.java
│       │   │           └── NativeLibraryLoader.java
│       │   └── resources/
│       │       └── native-lib/           # Embedded native libraries
│       └── test/
│           └── java/
│               └── com/hivellm/umicp/
│                   ├── core/
│                   │   ├── EnvelopeTest.java
│                   │   ├── MatrixTest.java
│                   │   └── FrameTest.java
│                   └── util/
│                       └── NativeLibraryLoaderTest.java
│
├── umicp-transport/                 # Transport module
│   ├── pom.xml                      # Module POM
│   └── src/
│       ├── main/
│       │   └── java/
│       │       └── com/hivellm/umicp/
│       │           ├── transport/
│       │           │   ├── UMICPWebSocketClient.java
│       │           │   ├── UMICPWebSocketServer.java
│       │           │   ├── UMICPWebSocketPeer.java
│       │           │   ├── ClientOptions.java
│       │           │   ├── ServerOptions.java
│       │           │   ├── PeerOptions.java
│       │           │   ├── ClientConnection.java
│       │           │   ├── PeerConnection.java
│       │           │   ├── PeerInfo.java
│       │           │   ├── TransportStats.java
│       │           │   ├── PeerStats.java
│       │           │   ├── ReconnectionStrategy.java
│       │           │   └── MessageQueue.java
│       │           ├── http/
│       │           │   ├── StreamableHTTPServer.java
│       │           │   ├── StreamableHTTPClient.java
│       │           │   ├── StreamableHTTPPeer.java
│       │           │   ├── HTTPServerOptions.java
│       │           │   ├── HTTPClientOptions.java
│       │           │   └── HTTPHandler.java
│       │           └── events/
│       │               ├── EventListener.java
│       │               ├── PeerEventListener.java
│       │               ├── PeerEvent.java
│       │               ├── MessageEvent.java
│       │               ├── DataEvent.java
│       │               ├── PeerConnectEvent.java
│       │               ├── PeerDisconnectEvent.java
│       │               ├── PeerReadyEvent.java
│       │               ├── HandshakeTimeoutEvent.java
│       │               ├── ErrorEvent.java
│       │               ├── ServerReadyEvent.java
│       │               └── ConnectedEvent.java
│       └── test/
│           └── java/
│               └── com/hivellm/umicp/
│                   ├── transport/
│                   │   ├── WebSocketClientTest.java
│                   │   ├── WebSocketServerTest.java
│                   │   └── WebSocketPeerTest.java
│                   ├── http/
│                   │   ├── HTTPServerTest.java
│                   │   ├── HTTPClientTest.java
│                   │   └── HTTPPeerTest.java
│                   └── integration/
│                       ├── ClientServerIntegrationTest.java
│                       ├── PeerToPeerIntegrationTest.java
│                       ├── HandshakeProtocolTest.java
│                       ├── ReconnectionTest.java
│                       ├── TwoPeerNetworkTest.java
│                       ├── ThreePeerRingTest.java
│                       ├── HubAndSpokeTest.java
│                       └── FullMeshTest.java
│
├── umicp-examples/                  # Example applications
│   ├── pom.xml                      # Module POM
│   └── src/
│       └── main/
│           └── java/
│               └── com/hivellm/umicp/examples/
│                   ├── BasicExample.java
│                   ├── SimplePeerToPeer.java
│                   ├── MultiplexedNetwork.java
│                   ├── BroadcastExample.java
│                   ├── RequestResponse.java
│                   ├── PipelineProcessing.java
│                   ├── MeshNetwork.java
│                   ├── HTTPTransport.java
│                   ├── FederatedLearning.java
│                   ├── IoTDataStreaming.java
│                   └── FinancialTransactions.java
│
└── umicp-benchmark/                 # Performance benchmarks
    ├── pom.xml                      # Module POM
    └── src/
        └── main/
            └── java/
                └── com/hivellm/umicp/benchmark/
                    ├── EnvelopeBenchmark.java
                    ├── MatrixBenchmark.java
                    ├── ThroughputBenchmark.java
                    ├── LatencyBenchmark.java
                    ├── ConcurrentConnectionsBenchmark.java
                    ├── LargePayloadBenchmark.java
                    ├── MemoryLeakTest.java
                    ├── ConnectionChurnTest.java
                    ├── MessageFloodTest.java
                    └── ErrorRecoveryTest.java
```

## 📦 Module Descriptions

### umicp-core
**Purpose**: Core UMICP functionality including Envelope, Matrix, and Frame classes.

**Dependencies**:
- Jackson (JSON serialization)
- SLF4J (logging)
- JetBrains Annotations

**Responsibilities**:
- JNI bindings to C++ core
- Envelope serialization/deserialization
- Matrix operations
- Type definitions
- Utility functions

### umicp-transport
**Purpose**: Network transport implementations (WebSocket, HTTP/2).

**Dependencies**:
- umicp-core
- Java-WebSocket or Netty
- OkHttp (for HTTP client)
- Jackson (JSON)

**Responsibilities**:
- WebSocket client/server/peer
- HTTP/2 client/server/peer
- Connection management
- Event system
- Auto-handshake protocol
- Reconnection logic

### umicp-examples
**Purpose**: Example applications demonstrating UMICP usage.

**Dependencies**:
- umicp-core
- umicp-transport

**Responsibilities**:
- Basic usage examples
- Topology examples (mesh, ring, hub-and-spoke)
- Use case examples (federated learning, IoT, financial)
- Tutorial code

### umicp-benchmark
**Purpose**: Performance benchmarks and stress tests.

**Dependencies**:
- umicp-core
- umicp-transport
- JMH (benchmarking framework)

**Responsibilities**:
- Throughput benchmarks
- Latency benchmarks
- Memory usage tests
- Stress tests
- Regression tests

## 🔧 Build Configuration

### Maven Multi-Module Structure

```xml
<modules>
    <module>umicp-core</module>
    <module>umicp-transport</module>
    <module>umicp-examples</module>
    <module>umicp-benchmark</module>
</modules>
```

### Dependency Graph

```
umicp-parent
├── umicp-core (foundation)
├── umicp-transport (depends on: umicp-core)
├── umicp-examples (depends on: umicp-core, umicp-transport)
└── umicp-benchmark (depends on: umicp-core, umicp-transport)
```

## 🎯 Package Organization

### com.hivellm.umicp.core
Core classes for envelope and matrix operations.

### com.hivellm.umicp.types
Type definitions, enums, and exceptions.

### com.hivellm.umicp.transport
WebSocket transport implementation.

### com.hivellm.umicp.http
HTTP/2 transport implementation.

### com.hivellm.umicp.events
Event system for async communication.

### com.hivellm.umicp.util
Utility classes (UUID, Hash, Timestamp, etc.).

### com.hivellm.umicp.examples
Example applications.

### com.hivellm.umicp.benchmark
Performance benchmarks.

## 📝 Naming Conventions

### Classes
- **Interfaces**: Descriptive noun (e.g., `EventListener`)
- **Implementations**: Interface name + suffix (e.g., `PeerEventListener`)
- **Exceptions**: Problem + `Exception` (e.g., `ConnectionException`)
- **Builders**: Class name + `Options` (e.g., `EnvelopeOptions`)
- **Tests**: Class name + `Test` (e.g., `EnvelopeTest`)
- **Benchmarks**: Operation + `Benchmark` (e.g., `ThroughputBenchmark`)

### Methods
- **Boolean**: `is`, `has`, `can` prefix (e.g., `isConnected`)
- **Getters**: `get` prefix (e.g., `getFrom`)
- **Setters**: `set` prefix (e.g., `setFrom`)
- **Actions**: Verb (e.g., `connect`, `send`, `broadcast`)
- **Async**: Returns `CompletableFuture<T>` (e.g., `connectAsync`)

### Constants
- **All uppercase**: `MAX_CONNECTIONS`, `DEFAULT_PORT`

## 🔗 Inter-Module Dependencies

```
┌─────────────────┐
│  umicp-parent   │
└────────┬────────┘
         │
    ┌────┴────┬────────────┬──────────────┐
    │         │            │              │
┌───▼────┐ ┌─▼──────────┐ ┌▼────────────┐ ┌▼────────────┐
│ core   │ │ transport  │ │ examples    │ │ benchmark   │
└────────┘ └──────┬─────┘ └──────┬──────┘ └──────┬──────┘
             │                  │                │
             └──────────┬───────┴────────────────┘
                        │
                   depends on core
```

## 🚀 Getting Started

1. **Clone repository**
2. **Navigate to Java bindings**: `cd bindings/java`
3. **Build all modules**: `mvn clean install`
4. **Run tests**: `mvn test`
5. **Run examples**: `mvn exec:java -pl umicp-examples -Dexec.mainClass=...`

## 📚 Additional Resources

- [Implementation Roadmap](../IMPLEMENTATION_ROADMAP.md)
- [Contributing Guidelines](../CONTRIBUTING.md)
- [TypeScript Reference](../../typescript/README.md)
- [C++ Core](../../../cpp/README.md)

---

**Status**: Planning Phase  
**Last Updated**: 2025-10-10  
**Version**: 1.0.0

