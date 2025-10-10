# Getting Started with UMICP Go Bindings

This guide will help you get started with the UMICP Go bindings once they are implemented.

## 🚧 Current Status

**Important**: The Go bindings are currently in the **planning phase**. This document describes the planned API and usage patterns.

- [ ] Phase 1: Foundation Layer (Envelope, Types, Errors)
- [ ] Phase 2: WebSocket Transport
- [ ] Phase 3: Multiplexed Peer Architecture
- [ ] Phase 4: HTTP/2 Transport
- [ ] Phase 5: Testing & Documentation

See [IMPLEMENTATION_ROADMAP.md](IMPLEMENTATION_ROADMAP.md) for the detailed plan.

---

## 📋 Prerequisites (Future)

- **Go 1.21+** (for generics and modern features)
- **Git** for version control
- Basic understanding of Go programming

---

## 🛠️ Installation (Planned)

### Option 1: Go Modules (Recommended)

```bash
go get github.com/hivellm/umicp-go
```

### Option 2: Build from Source

```bash
git clone https://github.com/hivellm/umicp.git
cd umicp/bindings/go
go build ./...
go test ./...
```

---

## 🏃 Quick Start Examples

### Example 1: Basic Envelope

Create, serialize, and deserialize an envelope.

```go
package main

import (
    "fmt"
    "log"
    
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
    // Create envelope
    envelope, err := umicp.NewEnvelope().
        From("client-001").
        To("server-001").
        Operation(umicp.OperationData).
        Capability("message", "Hello UMICP!").
        Build()
    
    if err != nil {
        log.Fatal(err)
    }
    
    // Serialize
    data, err := envelope.Serialize()
    if err != nil {
        log.Fatal(err)
    }
    
    fmt.Printf("Serialized: %d bytes\n", len(data))
    
    // Deserialize
    received, err := umicp.DeserializeEnvelope(data)
    if err != nil {
        log.Fatal(err)
    }
    
    fmt.Printf("From: %s\n", received.From)
    fmt.Printf("Message: %s\n", received.Capabilities["message"])
}
```

**Run:**
```bash
go run examples/basic/main.go
```

---

### Example 2: WebSocket Client

Connect to a WebSocket server and send messages.

```go
package main

import (
    "context"
    "fmt"
    "log"
    "time"
    
    "github.com/hivellm/umicp-go/pkg/transport/websocket"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
    ctx := context.Background()
    
    // Create client
    client := websocket.NewClient(websocket.ClientConfig{
        URL:               "ws://localhost:8080/umicp",
        HeartbeatInterval: 30 * time.Second,
    })
    
    // Setup message handler
    client.OnMessage(func(env *umicp.Envelope) {
        fmt.Printf("Received from %s: %v\n", 
            env.From, env.Capabilities)
    })
    
    // Setup connection handlers
    client.OnConnect(func() {
        fmt.Println("Connected to server")
    })
    
    client.OnDisconnect(func(err error) {
        if err != nil {
            fmt.Printf("Disconnected with error: %v\n", err)
        } else {
            fmt.Println("Disconnected")
        }
    })
    
    // Connect
    if err := client.Connect(ctx); err != nil {
        log.Fatal(err)
    }
    defer client.Disconnect(ctx)
    
    // Send messages
    for i := 0; i < 5; i++ {
        env, _ := umicp.NewEnvelope().
            From("client").
            To("server").
            Operation(umicp.OperationData).
            Capability("index", fmt.Sprintf("%d", i)).
            Capability("message", fmt.Sprintf("Message #%d", i)).
            Build()
        
        if err := client.Send(ctx, env); err != nil {
            log.Printf("Send error: %v", err)
        }
        
        time.Sleep(time.Second)
    }
    
    // Wait for responses
    time.Sleep(5 * time.Second)
}
```

---

### Example 3: WebSocket Server

Create a WebSocket server to handle connections.

```go
package main

import (
    "context"
    "fmt"
    "log"
    "os"
    "os/signal"
    "syscall"
    
    "github.com/hivellm/umicp-go/pkg/transport/websocket"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
    ctx, cancel := context.WithCancel(context.Background())
    defer cancel()
    
    // Create server
    server := websocket.NewServer(websocket.ServerConfig{
        Addr:        "127.0.0.1:8080",
        Path:        "/umicp",
        Compression: true,
    })
    
    // Setup message handler
    server.OnMessage(func(env *umicp.Envelope, conn *websocket.Connection) {
        fmt.Printf("Message from %s: %v\n", 
            conn.ID, env.Capabilities)
        
        // Send acknowledgment
        ack, _ := umicp.NewEnvelope().
            From("server").
            To(env.From).
            Operation(umicp.OperationAck).
            Capability("status", "received").
            Capability("original_id", env.MessageID).
            Build()
        
        server.Send(ctx, conn.ID, ack)
    })
    
    // Setup connection handlers
    server.OnConnect(func(conn *websocket.Connection) {
        fmt.Printf("Client connected: %s\n", conn.ID)
    })
    
    server.OnDisconnect(func(conn *websocket.Connection) {
        fmt.Printf("Client disconnected: %s\n", conn.ID)
    })
    
    // Start server
    go func() {
        fmt.Println("Server listening on 127.0.0.1:8080/umicp")
        if err := server.Start(ctx); err != nil {
            log.Printf("Server error: %v", err)
        }
    }()
    
    // Wait for interrupt
    sigCh := make(chan os.Signal, 1)
    signal.Notify(sigCh, os.Interrupt, syscall.SIGTERM)
    <-sigCh
    
    fmt.Println("\nShutting down server...")
}
```

---

### Example 4: Multiplexed Peer

Create a peer that can both accept connections and connect to others.

```go
package main

import (
    "context"
    "fmt"
    "log"
    "time"
    
    "github.com/hivellm/umicp-go/pkg/peer"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
    ctx := context.Background()
    
    // Create peer with server
    myPeer, err := peer.NewPeerWithServer(
        peer.PeerConfig{
            PeerID:           "agent-001",
            AutoProtocol:     true,
            HandshakeTimeout: 10 * time.Second,
        },
        peer.ServerConfig{
            Addr: "127.0.0.1:20081",
            Path: "/umicp",
        },
    )
    if err != nil {
        log.Fatal(err)
    }
    
    // Setup event handlers
    myPeer.On(peer.EventMessage, func(e *peer.Event) {
        fmt.Printf("[MESSAGE] From %s (%s): %v\n",
            e.Connection.ID,
            e.Connection.Type,
            e.Envelope.Capabilities)
    })
    
    myPeer.On(peer.EventData, func(e *peer.Event) {
        fmt.Printf("[DATA] From %s: %s\n",
            e.Envelope.From,
            e.Envelope.Capabilities["message"])
    })
    
    myPeer.On(peer.EventPeerConnect, func(e *peer.Event) {
        fmt.Printf("[CONNECT] Peer connected: %s (%s)\n",
            e.Connection.ID, e.Connection.Type)
    })
    
    myPeer.On(peer.EventPeerReady, func(e *peer.Event) {
        fmt.Printf("[READY] Handshake complete: %s\n",
            e.PeerInfo.PeerID)
    })
    
    myPeer.On(peer.EventPeerDisconnect, func(e *peer.Event) {
        fmt.Printf("[DISCONNECT] Peer disconnected: %s\n",
            e.Connection.ID)
    })
    
    // Start peer
    if err := myPeer.Start(ctx); err != nil {
        log.Fatal(err)
    }
    defer myPeer.Shutdown(ctx)
    
    fmt.Println("Peer started on 127.0.0.1:20081")
    
    // Connect to remote peers
    peerID, err := myPeer.ConnectToPeer(ctx,
        "ws://localhost:20082/umicp", nil)
    if err != nil {
        log.Printf("Failed to connect: %v", err)
    } else {
        fmt.Printf("Connected to peer: %s\n", peerID)
    }
    
    // Send message to specific peer
    time.Sleep(2 * time.Second)
    
    env, _ := umicp.NewEnvelope().
        From("agent-001").
        To("remote-agent").
        Operation(umicp.OperationData).
        Capability("message", "Hello from Go!").
        Build()
    
    if err := myPeer.SendToPeer(ctx, peerID, env); err != nil {
        log.Printf("Send error: %v", err)
    }
    
    // Broadcast to all peers
    broadcast, _ := umicp.NewEnvelope().
        From("agent-001").
        To("*").
        Operation(umicp.OperationData).
        Capability("message", "Broadcast message").
        Build()
    
    count := myPeer.Broadcast(ctx, broadcast, "")
    fmt.Printf("Broadcast sent to %d peers\n", count)
    
    // Get statistics
    stats := myPeer.GetStats()
    fmt.Printf("\nPeer Statistics:\n")
    fmt.Printf("  Total Peers: %d\n", stats.TotalPeers)
    fmt.Printf("  Incoming: %d\n", stats.IncomingConnections)
    fmt.Printf("  Outgoing: %d\n", stats.OutgoingConnections)
    fmt.Printf("  Handshakes Complete: %d\n", stats.HandshakesComplete)
    
    // Keep running
    time.Sleep(60 * time.Second)
}
```

---

### Example 5: Full Mesh Network

Create a full mesh network with 3 peers.

```go
package main

import (
    "context"
    "fmt"
    "log"
    "sync"
    "time"
    
    "github.com/hivellm/umicp-go/pkg/peer"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
    ctx := context.Background()
    var wg sync.WaitGroup
    
    // Create 3 peers
    peers := make([]*peer.Peer, 3)
    ports := []int{20081, 20082, 20083}
    
    // Start each peer
    for i := 0; i < 3; i++ {
        peerID := fmt.Sprintf("peer-%d", i+1)
        addr := fmt.Sprintf("127.0.0.1:%d", ports[i])
        
        p, err := peer.NewPeerWithServer(
            peer.PeerConfig{
                PeerID:       peerID,
                AutoProtocol: true,
            },
            peer.ServerConfig{
                Addr: addr,
                Path: "/umicp",
            },
        )
        if err != nil {
            log.Fatal(err)
        }
        
        // Setup handlers
        p.On(peer.EventPeerReady, func(e *peer.Event) {
            fmt.Printf("[%s] Connected to %s\n",
                peerID, e.PeerInfo.PeerID)
        })
        
        p.Start(ctx)
        peers[i] = p
        
        fmt.Printf("Started %s on %s\n", peerID, addr)
    }
    
    // Wait for servers to start
    time.Sleep(time.Second)
    
    // Connect peers in full mesh (each connects to all others)
    for i, p := range peers {
        for j, port := range ports {
            if i != j {
                url := fmt.Sprintf("ws://127.0.0.1:%d/umicp", port)
                go func(peer *peer.Peer, url string) {
                    if _, err := peer.ConnectToPeer(ctx, url, nil); err != nil {
                        log.Printf("Connect error: %v", err)
                    }
                }(p, url)
            }
        }
    }
    
    // Wait for connections
    time.Sleep(3 * time.Second)
    
    // Send test messages
    for i, p := range peers {
        env, _ := umicp.NewEnvelope().
            From(fmt.Sprintf("peer-%d", i+1)).
            To("*").
            Operation(umicp.OperationData).
            Capability("message", fmt.Sprintf("Hello from peer-%d", i+1)).
            Build()
        
        count := p.Broadcast(ctx, env, "")
        fmt.Printf("Peer-%d broadcast to %d peers\n", i+1, count)
    }
    
    // Print statistics
    fmt.Println("\nNetwork Statistics:")
    for i, p := range peers {
        stats := p.GetStats()
        fmt.Printf("Peer-%d: %d total connections (%d in, %d out)\n",
            i+1, stats.TotalPeers,
            stats.IncomingConnections,
            stats.OutgoingConnections)
    }
    
    // Keep running
    fmt.Println("\nMesh network running. Press Ctrl+C to stop.")
    time.Sleep(60 * time.Second)
    
    // Cleanup
    for _, p := range peers {
        p.Shutdown(ctx)
    }
}
```

---

## 📚 Next Steps

1. **Read the Documentation**:
   - [README.md](README.md) - Overview
   - [IMPLEMENTATION_ROADMAP.md](IMPLEMENTATION_ROADMAP.md) - Detailed plan
   - [COMPARISON.md](COMPARISON.md) - TypeScript vs Go

2. **Explore Examples**:
   - `examples/basic/` - Basic envelope usage
   - `examples/websocket/` - WebSocket client/server
   - `examples/peer/` - Multiplexed peer
   - `examples/mesh/` - Full mesh network

3. **Run Tests** (when available):
   ```bash
   go test ./...
   go test -v ./pkg/umicp/
   go test -bench=. ./...
   ```

4. **Check Integration** (when available):
   ```bash
   go test ./test/integration/
   ```

---

## 🔧 Configuration

### Environment Variables

```bash
# WebSocket configuration
export UMICP_WS_PORT=8080
export UMICP_WS_HOST=localhost

# Peer configuration
export UMICP_PEER_ID=my-agent
export UMICP_HANDSHAKE_TIMEOUT=10s

# Logging
export UMICP_LOG_LEVEL=info
```

### Configuration File (Future)

```yaml
# config.yml
peer:
  id: my-agent
  auto_protocol: true
  handshake_timeout: 10s

server:
  addr: 127.0.0.1:20081
  path: /umicp
  compression: true

client:
  heartbeat_interval: 30s
  max_reconnect_attempts: 5
  reconnect_delay: 1s
```

---

## 🐛 Troubleshooting

### Common Issues

**1. Connection Refused**
```
Error: transport: connection failed
```
**Solution**: Ensure the server is running and the address is correct.

**2. Handshake Timeout**
```
Error: peer: handshake timeout
```
**Solution**: Check network connectivity and ensure auto-protocol is enabled.

**3. Context Canceled**
```
Error: context canceled
```
**Solution**: Don't cancel the context while operations are in progress.

---

## 📞 Getting Help

- **Issues**: [GitHub Issues](https://github.com/hivellm/umicp/issues)
- **Discussions**: [GitHub Discussions](https://github.com/hivellm/umicp/discussions)
- **Documentation**: [docs/](../../docs/)

---

## 🤝 Contributing

We welcome contributions! See [CONTRIBUTING.md](../../CONTRIBUTING.md) for guidelines.

---

**Status**: 🚧 Planning Phase  
**Last Updated**: 2025-10-10  
**Maintainer**: HiveLLM AI Collaborative Team

