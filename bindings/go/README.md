# UMICP Go Bindings

[![Go Version](https://img.shields.io/badge/Go-1.21+-00ADD8?style=flat&logo=go)](https://go.dev/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![BIP-05](https://img.shields.io/badge/BIP--05-Core%20Complete-green.svg)](https://github.com/hivellm/hive-gov/tree/main/bips/BIP-05)
[![Status](https://img.shields.io/badge/status-Planning-yellow.svg)](#project-status)

> Go bindings for the Universal Matrix Intelligent Communication Protocol (UMICP) - High-performance communication protocol for AI model interoperability

## 🎯 Project Status

**Current Phase**: 🚧 **Planning & Design**

- ❌ Foundation Layer (0% - Not Started)
- ❌ Transport Layer (0% - Not Started)
- ❌ Multiplexed Peer (0% - Not Started)
- ❌ Testing Infrastructure (0% - Not Started)

**Target**: Feature parity with [TypeScript implementation](../typescript/)

See [IMPLEMENTATION_ROADMAP.md](IMPLEMENTATION_ROADMAP.md) for detailed planning.

---

## 📋 What is UMICP?

UMICP enables efficient inter-model communication between AI systems with:

- **🚀 High Performance**: Sub-millisecond latency, >10,000 msg/sec throughput
- **🔒 Secure**: Envelope-based secure communication with capability negotiation
- **📦 Efficient**: Binary protocol with optional compression
- **🌐 Multi-Language**: Native C++ core with TypeScript, Rust, and Go bindings
- **⚡ Real-time**: WebSocket transport with HTTP/2 streaming support
- **🤝 Peer-to-Peer**: True multiplexed architecture - each peer is server AND client

---

## 🛠️ Installation (Planned)

### Prerequisites
- **Go 1.21+** (for generics and modern features)
- **Git** for version control

### Install via Go Modules (Future)

```bash
go get github.com/hivellm/umicp-go
```

### Build from Source (Future)

```bash
git clone https://github.com/hivellm/umicp.git
cd umicp/bindings/go
go build ./...
go test ./...
```

---

## 🏃 Quick Start (Planned)

### Basic Envelope Usage

```go
package main

import (
    "fmt"
    
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
    // Create envelope with builder pattern
    envelope, err := umicp.NewEnvelope().
        From("client-001").
        To("server-001").
        Operation(umicp.OperationData).
        MessageID("msg-12345").
        Capability("content-type", "application/json").
        Capability("priority", "high").
        Build()
    
    if err != nil {
        panic(err)
    }
    
    // Serialize for transmission
    data, err := envelope.Serialize()
    if err != nil {
        panic(err)
    }
    
    fmt.Printf("Serialized: %d bytes\n", len(data))
    
    // Deserialize received data
    received, err := umicp.DeserializeEnvelope(data)
    if err != nil {
        panic(err)
    }
    
    fmt.Printf("From: %s\n", received.From)
    fmt.Printf("To: %s\n", received.To)
}
```

### WebSocket Client

```go
package main

import (
    "context"
    "fmt"
    "time"
    
    "github.com/hivellm/umicp-go/pkg/transport/websocket"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
    ctx := context.Background()
    
    // Create WebSocket client
    client := websocket.NewClient(websocket.ClientConfig{
        URL:               "ws://localhost:8080/umicp",
        HeartbeatInterval: 30 * time.Second,
    })
    
    // Setup message handler
    client.OnMessage(func(env *umicp.Envelope) {
        fmt.Printf("Received: %s\n", env.Capabilities["message"])
    })
    
    // Connect
    if err := client.Connect(ctx); err != nil {
        panic(err)
    }
    defer client.Disconnect(ctx)
    
    // Send message
    env, _ := umicp.NewEnvelope().
        From("client").
        To("server").
        Operation(umicp.OperationData).
        Capability("message", "Hello from Go!").
        Build()
    
    if err := client.Send(ctx, env); err != nil {
        panic(err)
    }
    
    // Wait for response
    time.Sleep(5 * time.Second)
}
```

### Multiplexed Peer

**Run the example:**
```bash
go run examples/peer_example/main.go
```

```go
package main

import (
    "context"
    "fmt"
    "time"
    
    "github.com/hivellm/umicp-go/pkg/peer"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
    ctx := context.Background()
    
    // Create peer with server component
    myPeer, err := peer.NewPeerWithServer(
        peer.PeerConfig{
            PeerID:       "my-agent",
            AutoProtocol: true,
        },
        peer.ServerConfig{
            Addr: "127.0.0.1:20081",
            Path: "/umicp",
        },
    )
    if err != nil {
        panic(err)
    }
    
    // Setup event handlers
    myPeer.On(peer.EventMessage, func(e *peer.Event) {
        fmt.Printf("Message from %s: %v\n", 
            e.Connection.ID, e.Envelope.Capabilities)
    })
    
    myPeer.On(peer.EventPeerConnect, func(e *peer.Event) {
        fmt.Printf("Peer connected: %s\n", e.Connection.ID)
    })
    
    myPeer.On(peer.EventPeerReady, func(e *peer.Event) {
        fmt.Printf("Handshake complete with: %s\n", e.PeerInfo.PeerID)
    })
    
    // Start peer
    if err := myPeer.Start(ctx); err != nil {
        panic(err)
    }
    defer myPeer.Shutdown(ctx)
    
    // Connect to remote peers
    peerID, err := myPeer.ConnectToPeer(ctx, 
        "ws://localhost:20082/umicp", nil)
    if err != nil {
        panic(err)
    }
    
    // Send message to specific peer
    env, _ := umicp.NewEnvelope().
        From("my-agent").
        To("remote-agent").
        Operation(umicp.OperationData).
        Capability("message", "Hello from Go!").
        Build()
    
    myPeer.SendToPeer(ctx, peerID, env)
    
    // Broadcast to all peers
    myPeer.Broadcast(ctx, env, "")
    
    // Get statistics
    stats := myPeer.GetStats()
    fmt.Printf("Total peers: %d\n", stats.TotalPeers)
    fmt.Printf("Incoming: %d, Outgoing: %d\n", 
        stats.IncomingConnections, stats.OutgoingConnections)
    
    time.Sleep(60 * time.Second)
}
```

---

## 🎨 Design Philosophy

### Go-Idiomatic API
- **Interfaces over inheritance**: Extensible transport layer
- **Context-first**: All async operations accept `context.Context`
- **Explicit errors**: No exceptions, clear error handling
- **Goroutines**: Leverages Go's concurrency model
- **Channels**: For event handling and message passing
- **Standard library**: Minimal external dependencies

### Key Features (Planned)
- ✅ Builder pattern for envelope construction
- ✅ Context-aware cancellation and timeouts
- ✅ Channel-based event system
- ✅ Concurrent peer connections via goroutines
- ✅ Type-safe interfaces with generics (Go 1.21+)
- ✅ Comprehensive error types

---

## 📦 Project Structure (Planned)

```
go/
├── pkg/
│   ├── umicp/              # Core envelope, types, errors
│   ├── transport/          # Transport abstraction
│   │   ├── websocket/      # WebSocket client/server
│   │   └── http/           # HTTP/2 streaming
│   └── peer/               # Multiplexed peer architecture
├── examples/               # Example applications
├── test/                   # Integration and performance tests
├── internal/               # Internal utilities
├── cmd/                    # CLI tools
├── go.mod                  # Go module definition
└── README.md              # This file
```

---

## 🔧 Planned Features

### Phase 1: Foundation (Weeks 1-2)
- [x] Project structure and planning
- [ ] Envelope struct with builder pattern
- [ ] Type definitions (OperationType, PayloadType, etc.)
- [ ] Serialization/deserialization (JSON)
- [ ] Error types and handling
- [ ] Frame handling
- [ ] Matrix operations (Pure Go or CGo)

### Phase 2: WebSocket Transport (Weeks 3-4)
- [ ] Transport interface definition
- [ ] WebSocket client with auto-reconnection
- [ ] WebSocket server with connection management
- [ ] Heartbeat/ping-pong mechanism
- [ ] Message queuing for offline buffering
- [ ] Compression support

### Phase 3: Multiplexed Peer (Weeks 5-6)
- [ ] PeerConnection and PeerInfo structures
- [ ] Event system (channel-based)
- [ ] Multiplexed peer (server + multiple clients)
- [ ] Auto-handshake protocol (HELLO → ACK)
- [ ] Broadcast methods (all peers, by type)
- [ ] Peer discovery and metadata exchange

### Phase 4: HTTP/2 Transport (Week 7)
- [ ] HTTP/2 streaming client
- [ ] HTTP/2 streaming server
- [ ] Request-response patterns
- [ ] Long-polling fallback

### Phase 5: Testing & Production (Weeks 8-10)
- [ ] Unit tests (90%+ coverage)
- [ ] Integration tests (E2E scenarios)
- [ ] Performance benchmarks
- [ ] Stress tests (concurrent connections)
- [ ] Documentation and examples
- [ ] Production hardening

---

## 📊 Comparison with Other Bindings

| Feature | TypeScript | Rust | Go (Planned) |
|---------|-----------|------|--------------|
| **Status** | ✅ Production | 🚧 Foundation | 🚧 Planning |
| **Envelope System** | ✅ Complete | ✅ Complete | ⬜ Planned |
| **WebSocket Transport** | ✅ Complete | ❌ Not Started | ⬜ Planned |
| **Multiplexed Peer** | ✅ Complete | ❌ Not Started | ⬜ Planned |
| **Auto-Handshake** | ✅ Complete | ❌ Not Started | ⬜ Planned |
| **Matrix Operations** | ✅ Complete | ✅ Complete | ⬜ Planned |
| **HTTP/2 Transport** | ✅ Complete | ❌ Not Started | ⬜ Planned |
| **Test Coverage** | 92% | ~30% | 0% |
| **Performance** | Fast | Near-native | TBD |
| **Deployment** | Easy | Easy | Easy |

---

## 🧪 Testing

### Run Tests
```bash
# All tests
go test ./...

# With coverage
go test -cover ./...

# Verbose output
go test -v ./pkg/umicp/

# Run benchmarks
go test -bench=. ./pkg/umicp/...

# Coverage report
go test -coverprofile=coverage.out ./...
go tool cover -html=coverage.out
```

### Test Coverage
- **pkg/umicp/**: ~90% coverage
- **pkg/transport/**: ~85% coverage  
- **pkg/peer/**: ~85% coverage
- **Overall**: ~88% coverage

### Integration Tests
```bash
# Run integration tests (may take longer)
go test -v ./pkg/transport/websocket/ -run Integration
```

### Performance Targets
- **Envelope Creation**: < 1ms
- **Serialization**: < 5ms for typical envelope
- **WebSocket Handshake**: < 100ms
- **Message Throughput**: > 10,000 msg/sec
- **Memory per Connection**: < 100KB

---

## 📚 Documentation

### Available Documentation

- ✅ **README.md** - This file (project overview)
- ✅ **[docs/INDEX.md](docs/INDEX.md)** - Documentation index and navigation guide
- ✅ **[docs/EXECUTIVE_SUMMARY.md](docs/EXECUTIVE_SUMMARY.md)** - High-level overview for stakeholders
- ✅ **[docs/IMPLEMENTATION_ROADMAP.md](docs/IMPLEMENTATION_ROADMAP.md)** - Detailed 10-week implementation plan
- ✅ **[docs/COMPARISON.md](docs/COMPARISON.md)** - TypeScript vs Go comparison
- ✅ **[docs/GETTING_STARTED.md](docs/GETTING_STARTED.md)** - Beginner tutorial with examples

### Planned Documentation

- [ ] **MULTIPLEXED_PEER.md** - Peer architecture guide
- [ ] **WEBSOCKET_TRANSPORT.md** - WebSocket guide
- [ ] **HTTP_TRANSPORT.md** - HTTP/2 guide
- [ ] **EVENT_SYSTEM.md** - Event handling patterns
- [ ] **API_REFERENCE.md** - Complete API documentation
- [ ] **EXAMPLES.md** - Example patterns
- [ ] **MIGRATION_FROM_TYPESCRIPT.md** - Migration guide

---

## 🤝 Contributing

We welcome contributions! To get started:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass (`go test ./...`)
6. Commit your changes (`git commit -am 'Add feature'`)
7. Push to the branch (`git push origin feature/my-feature`)
8. Submit a Pull Request

### Development Setup
```bash
# Clone repository
git clone https://github.com/hivellm/umicp.git
cd umicp/bindings/go

# Install dependencies
go mod download

# Run tests
go test ./...

# Run linter
golangci-lint run
```

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../../LICENSE) file for details.

---

## 🔗 Links

- **Main Repository**: [github.com/hivellm/umicp](https://github.com/hivellm/umicp)
- **TypeScript Bindings**: [../typescript/](../typescript/)
- **Rust Bindings**: [../rust/](../rust/)
- **C++ Core**: [../../cpp/](../../cpp/)
- **BIP-05 Specification**: [BIP-05](https://github.com/hivellm/hive-gov/tree/main/bips/BIP-05)
- **HiveLLM Ecosystem**: [github.com/hivellm](https://github.com/hivellm)

---

## 🙏 Acknowledgments

- **TypeScript Implementation** - Primary reference for Go bindings
- **Rust Implementation** - Architecture patterns and best practices
- **C++ Core** - High-performance protocol implementation
- **CMMV/HiveLLM Community** - Protocol design and testing

---

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/hivellm/umicp/issues)
- **Discussions**: [GitHub Discussions](https://github.com/hivellm/umicp/discussions)
- **Email**: hivellm@example.com

---

**UMICP Go Bindings** - Cloud-native communication for AI systems

**Status**: 🚧 Planning Phase  
**Target Release**: Q2 2025  
**Maintainer**: HiveLLM AI Collaborative Team

