# 🌐 UMICP C# SDK

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![.NET](https://img.shields.io/badge/.NET-8.0-purple.svg)](https://dotnet.microsoft.com/)
[![Version](https://img.shields.io/badge/version-0.2.0-blue.svg)](CHANGELOG.md)
[![BIP-05](https://img.shields.io/badge/BIP--05-Implementation-green.svg)](https://github.com/hivellm/hive-gov/tree/main/bips/BIP-05)

> **Production-Ready** C# bindings for the Universal Matrix Intelligent Communication Protocol (UMICP) with MCP-compatible tool discovery

## 📋 What is UMICP?

UMICP enables efficient inter-model communication between AI systems with:

- **🚀 High Performance**: Sub-millisecond latency, >10,000 msg/sec throughput
- **🔒 Secure**: Envelope-based secure communication with capability negotiation
- **📦 Efficient**: Binary protocol with optional compression
- **⚡ Real-time**: WebSocket transport with HTTP support
- **🤝 Peer-to-Peer**: True multiplexed architecture - each peer is server AND client
- **🔧 Tool Discovery**: MCP-compatible automatic tool introspection with JSON Schema

## 🛠️ Installation

### NuGet Package (Coming Soon)

```bash
dotnet add package Umicp
```

### Build from Source

```bash
git clone https://github.com/hivellm/umicp.git
cd umicp/bindings/csharp
dotnet build
dotnet test
```

## 🚀 Quick Start

### 🆕 Tool Discovery (v0.2.0)

```csharp
using Umicp.Core.ToolDiscovery;
using System.Text.Json;

// Implement discoverable service
public class MyService : IDiscoverableService
{
    public List<OperationSchema> ListOperations()
    {
        return new List<OperationSchema>
        {
            new OperationSchema(
                "search_vectors",
                JsonDocument.Parse(@"{
                    ""type"": ""object"",
                    ""properties"": {
                        ""query"": {""type"": ""string""},
                        ""limit"": {""type"": ""integer"", ""default"": 10}
                    },
                    ""required"": [""query""]
                }").RootElement
            )
            .WithTitle("Search Vectors")
            .WithDescription("Search for semantically similar content")
            .WithAnnotations(JsonDocument.Parse(@"{""read_only"": true}").RootElement)
        };
    }

    public ServerInfo GetServerInfo()
    {
        return new ServerInfo("my-service", "1.0.0", "UMICP/0.2")
            .WithFeatures(new List<string> { "discovery", "search" })
            .WithMcpCompatible(true);
    }
}

// Use discovery helpers
var service = new MyService();
var operationsResponse = DiscoveryHelpers.GenerateOperationsResponse(service);
var schemaResponse = DiscoveryHelpers.GenerateSchemaResponse(service, "search_vectors");
var infoResponse = DiscoveryHelpers.GenerateServerInfoResponse(service);
```

### Basic Envelope Creation

```csharp
using Umicp.Core;
using Umicp.Core.Types;

// Create an envelope
var envelope = new Envelope(
    fromId: "client-001",
    toId: "server-001",
    operation: OperationType.Data
);

// Serialize to JSON
var json = envelope.ToJson();
Console.WriteLine(json);

// Using builder pattern
var builtEnvelope = new EnvelopeBuilder()
    .FromId("client-001")
    .ToId("server-001")
    .Operation(OperationType.Request)
    .Capability("version", "1.0.0")
    .Build();
```

### Matrix Operations (SIMD Accelerated)

```csharp
using Umicp.Core;

// Dot product
var a = new float[] { 1.0f, 2.0f, 3.0f };
var b = new float[] { 4.0f, 5.0f, 6.0f };
var result = Matrix.DotProduct(a, b); // 32.0

// Cosine similarity
var similarity = Matrix.CosineSimilarity(a, b);

// Matrix operations
var sum = Matrix.Add(a, b);
var scaled = Matrix.Scale(a, 2.0f);
var distance = Matrix.EuclideanDistance(a, b);
```

### WebSocket Client

```csharp
using Umicp.Core.Transport;
using Umicp.Core.Events;

// Create WebSocket client
using var client = new WebSocketClient("ws://localhost:8080");

// Subscribe to events
client.Events.On(EventType.Connect, evt =>
{
    Console.WriteLine("Connected!");
});

client.Events.On(EventType.DataReceived, evt =>
{
    var data = evt.Data["data"];
    Console.WriteLine($"Received: {data}");
});

// Connect and send data
await client.ConnectAsync();
await client.SendTextAsync("Hello, UMICP!");
```

### Multiplexed Peer

```csharp
using Umicp.Core.Peer;

// Create a peer
var peer = new MultiplexedPeer(
    localId: "peer-001",
    capabilities: new Dictionary<string, object>
    {
        ["version"] = "1.0.0",
        ["features"] = new[] { "chat", "file-transfer" }
    }
);

// Add another peer
var peerInfo = new PeerInfo("peer-002", "ws://localhost:8081");
await peer.AddPeerAsync(peerInfo);

// Send message to specific peer
var message = new EnvelopeBuilder()
    .FromId(peer.LocalId)
    .ToId("peer-002")
    .Operation(OperationType.Data)
    .Capability("message", "Hello!")
    .Build();

await peer.SendToPeerAsync("peer-002", message);

// Broadcast to all peers
await peer.BroadcastAsync(message);
```

### Service Discovery

```csharp
using Umicp.Core.Discovery;

// Create service discovery
using var discovery = new ServiceDiscovery(
    heartbeatInterval: TimeSpan.FromSeconds(10),
    serviceTimeout: TimeSpan.FromSeconds(30)
);

// Register a service
var service = new ServiceInfo
{
    Id = "service-001",
    Name = "API Gateway",
    Type = "gateway",
    Endpoint = "http://localhost:3000"
};
discovery.RegisterService(service);

// Find services
var gateways = discovery.FindByType("gateway");
var healthyServices = discovery.FindHealthyServices();

// Heartbeat
discovery.Heartbeat("service-001");
```

### Connection Pool

```csharp
using Umicp.Core.Pool;

// Create connection pool
var config = new PoolConfig
{
    MinConnections = 2,
    MaxConnections = 10,
    ConnectionTimeout = TimeSpan.FromSeconds(30)
};

using var pool = new ConnectionPool("ws://localhost:8080", config);
await pool.InitializeAsync();

// Execute with pooled connection
await pool.ExecuteAsync(async connection =>
{
    await connection.SendTextAsync("Hello from pool!");
});

Console.WriteLine($"Pool stats:");
Console.WriteLine($"  Total: {pool.TotalConnections}");
Console.WriteLine($"  Available: {pool.AvailableConnections}");
Console.WriteLine($"  Active: {pool.ActiveConnections}");
```

## 📦 Features

### ✅ Core Features

- **Protocol**: Binary envelope-based communication with capability negotiation
- **Transport**: WebSocket client/server, HTTP client/server
- **Multiplexed Architecture**: Each peer functions as server AND client simultaneously
- **Message Types**: CONTROL, DATA, ACK, ERROR, REQUEST, RESPONSE operations
- **Payload Types**: Vector, Text, Metadata, Binary, JSON, Matrix data support
- **Matrix Operations**: SIMD-accelerated dot product, cosine similarity, matrix multiplication
- **Compression**: Gzip and Deflate compression with automatic detection and efficiency metrics
- **Event-Driven API**: Observer pattern with EventEmitter
- **Peer Discovery**: Automatic handshake (HELLO → ACK) with metadata exchange
- **Service Discovery**: Automatic service registration and discovery with health checks
- **Connection Pooling**: Efficient connection management with automatic scaling
- **Server Support**: Production-ready WebSocket and HTTP servers with multi-client handling

### 🎯 Feature Matrix

| Feature | Status |
|---------|--------|
| Envelope/Frame | ✅ |
| Serialization (JSON) | ✅ |
| Message Types | ✅ |
| Payload Types | ✅ |
| Matrix Operations | ✅ (SIMD) |
| WebSocket Client | ✅ |
| HTTP Client | ✅ |
| WebSocket Server | ✅ |
| HTTP Server | ✅ |
| Multiplexed Peer | ✅ |
| Event System | ✅ |
| Service Discovery | ✅ |
| Connection Pooling | ✅ |
| Compression | ✅ (Gzip/Deflate) |
| Security/Encryption | 📋 |

**Legend**: ✅ Implemented | 🚧 In Progress | 📋 Planned

## 📚 Documentation

Comprehensive documentation and review reports are available in the [docs](docs/) directory:
- **Executive Summary** - High-level overview for stakeholders
- **Technical Report** - Detailed technical implementation
- **Implementation Review** - Comprehensive code review
- **Test Coverage Report** - Testing analysis and coverage
- **Final Report** - Consolidated evaluation
- **Second Review** - Risk assessment and recommendations
- **Third Review** - Developer experience and NuGet readiness

## 📚 Examples

Check the [Examples](Umicp.Examples/) directory for comprehensive examples:

1. **01_BasicEnvelope.cs** - Envelope creation and serialization
2. **02_MatrixOperations.cs** - SIMD-accelerated matrix operations
3. **03_WebSocketClient.cs** - WebSocket client usage
4. **04_MultiplexedPeer.cs** - Peer-to-peer communication
5. **05_ServiceDiscovery.cs** - Service registration and discovery
6. **06_Compression.cs** - Data compression with Gzip and Deflate
7. **07_WebSocketServer.cs** - Multi-client WebSocket server
8. **08_HttpServer.cs** - HTTP server with routing

Run examples:

```bash
cd Umicp.Examples
dotnet run
```

## 🧪 Testing

Run unit tests:

```bash
cd Umicp.Tests
dotnet test
```

Run with coverage:

```bash
dotnet test /p:CollectCoverage=true /p:CoverletOutputFormat=opencover
```

## 📊 Performance

- **SIMD Acceleration**: Matrix operations use `System.Numerics.Vectors` for hardware acceleration
- **Zero-Copy**: Efficient memory management with minimal allocations
- **Async/Await**: Full async support throughout the API
- **Connection Pooling**: Reduces connection overhead with intelligent pooling

## 🏗️ Architecture

```
Umicp.Core/
├── Types/              # Core type definitions
│   ├── OperationType.cs
│   ├── PayloadType.cs
│   ├── EncodingType.cs
│   ├── ConnectionState.cs
│   ├── PayloadHint.cs
│   └── TransportStats.cs
├── Exceptions/         # Exception classes
│   └── UmicpException.cs
├── Events/             # Event system
│   └── EventEmitter.cs
├── Transport/          # Transport layer
│   ├── ITransport.cs
│   ├── WebSocketClient.cs
│   └── HttpClient.cs
├── Peer/               # Peer-to-peer
│   ├── MultiplexedPeer.cs
│   ├── PeerConnection.cs
│   ├── PeerInfo.cs
│   └── HandshakeProtocol.cs
├── Discovery/          # Service discovery
│   ├── ServiceDiscovery.cs
│   └── ServiceInfo.cs
├── Pool/               # Connection pooling
│   ├── ConnectionPool.cs
│   └── PoolConfig.cs
├── Envelope.cs         # Core envelope class
└── Matrix.cs           # Matrix operations
```

## 🔗 Part of HiveLLM Ecosystem

UMICP is a core component of the **HiveLLM ecosystem**, providing high-performance binary protocol for agent-to-agent communication:

- **Vectorizer**: Semantic search and vector database
- **Task Queue**: Workflow orchestration
- **Agent Framework**: Multi-language agent platform
- **Voxa**: Voice AI assistant

## 🤝 Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](../../CONTRIBUTING.md) for details.

## 📄 License

MIT License - See [LICENSE](LICENSE) file for details.

## 🔗 Links

- **Main Repository**: https://github.com/hivellm/umicp
- **Documentation**: https://github.com/hivellm/umicp/tree/main/docs
- **BIP-05 Specification**: https://github.com/hivellm/hive-gov/tree/main/bips/BIP-05
- **Issues**: https://github.com/hivellm/umicp/issues

## 📞 Support

- Open an issue on GitHub
- Join our Discord community
- Email: team@hivellm.org

---

**BIP**: BIP-05 - Universal Matrix Intelligent Communication Protocol
**Status**: 🚧 Beta Release (C# v0.1.2)
**Repository**: HiveLLM UMICP C# Implementation

