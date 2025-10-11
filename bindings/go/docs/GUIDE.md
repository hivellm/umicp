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


---

# UMICP Go Bindings - Quick Start Guide

**Status**: ✅ Production Ready (MVP)  
**Version**: 1.0.0-mvp  
**Last Updated**: October 10, 2025

---

## 📦 Installation

```bash
go get github.com/hivellm/umicp-go
```

**Requirements**: Go 1.21+

---

## 🏃 Quick Examples

### 1. Basic Envelope (30 seconds)

```bash
# Run the basic example
cd umicp/bindings/go
go run examples/basic/main.go
```

**What it does**: Creates envelopes, serializes, validates, and hashes them.

---

### 2. WebSocket Communication (1 minute)

```bash
# Run WebSocket client-server example
go run examples/websocket_example/main.go
```

**What it does**: 
- Starts WebSocket server on port 8080
- Connects client
- Sends 5 messages
- Shows acknowledgments
- Displays statistics

---

### 3. Multiplexed Peer Network (2 minutes)

```bash
# Run 3-peer network example
go run examples/peer_example/main.go
```

**What it does**:
- Creates 3 interconnected peers
- Peer A: Server on port 20081
- Peer B: Server on port 20082 + connects to A
- Peer C: Client-only, connects to A and B
- Demonstrates handshake, broadcast, statistics

---

## 📚 Step-by-Step Tutorials

### Tutorial 1: Create Your First Envelope

```go
package main

import (
    "fmt"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
    // Step 1: Create envelope
    envelope, err := umicp.NewEnvelope().
        From("my-app").
        To("server").
        Operation(umicp.OperationData).
        Capability("message", "Hello World!").
        Build()
    
    if err != nil {
        panic(err)
    }
    
    // Step 2: Serialize
    data, _ := envelope.Serialize()
    fmt.Printf("Size: %d bytes\n", len(data))
    
    // Step 3: Deserialize
    received, _ := umicp.DeserializeEnvelope(data)
    fmt.Printf("Message: %s\n", received.Capabilities["message"])
}
```

**Save as**: `my_first_envelope.go`  
**Run**: `go run my_first_envelope.go`

---

### Tutorial 2: WebSocket Client

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
    
    // Create client
    config := websocket.DefaultClientConfig()
    config.URL = "ws://your-server:8080/ws"
    
    client := websocket.NewClient(*config)
    
    // Handle messages
    client.OnMessage(func(env *umicp.Envelope) {
        fmt.Printf("Received: %v\n", env.Capabilities)
    })
    
    // Connect
    if err := client.Connect(ctx); err != nil {
        panic(err)
    }
    defer client.Disconnect(ctx)
    
    // Send message
    env, _ := umicp.NewEnvelope().
        From("my-client").
        To("server").
        Operation(umicp.OperationData).
        Capability("action", "ping").
        Build()
    
    client.Send(ctx, env)
    
    time.Sleep(5 * time.Second)
}
```

---

### Tutorial 3: Create a Peer

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
    
    // Create peer with server
    myPeer, err := peer.NewPeerWithServer(
        peer.PeerConfig{
            PeerID: "my-peer",
            AutoProtocol: true,  // Enable auto-handshake
        },
        peer.ServerConfig{
            Addr: "127.0.0.1:9000",
            Path: "/umicp",
        },
    )
    if err != nil {
        panic(err)
    }
    
    // Handle events
    myPeer.On(peer.EventPeerReady, func(e *peer.Event) {
        fmt.Printf("Peer ready: %s\n", e.PeerInfo.PeerID)
    })
    
    myPeer.On(peer.EventData, func(e *peer.Event) {
        fmt.Printf("Data: %v\n", e.Envelope.Capabilities)
    })
    
    // Start
    myPeer.Start(ctx)
    
    // Connect to other peer
    peerID, _ := myPeer.ConnectToPeer(ctx, "ws://other-peer:9001/umicp", nil)
    
    // Send message
    env, _ := umicp.NewEnvelope().
        From("my-peer").
        To("other-peer").
        Operation(umicp.OperationData).
        Capability("message", "Hi!").
        Build()
    
    myPeer.SendToPeer(ctx, peerID, env)
    
    time.Sleep(10 * time.Second)
    myPeer.Shutdown(ctx)
}
```

---

## 🧪 Testing Your Code

### Run Tests
```bash
# Test everything
go test ./...

# Test with coverage
go test -cover ./...

# Verbose output
go test -v ./pkg/umicp/

# Run benchmarks
go test -bench=. ./pkg/umicp/matrix/
```

### Write Your Own Tests
```go
package myapp

import (
    "testing"
    "github.com/stretchr/testify/assert"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func TestMyEnvelope(t *testing.T) {
    env, err := umicp.NewEnvelope().
        From("test").
        To("dest").
        Build()
