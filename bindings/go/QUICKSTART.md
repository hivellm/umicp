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
    
    assert.NoError(t, err)
    assert.Equal(t, "test", env.From)
}
```

---

## 🎯 Common Patterns

### Pattern 1: Request-Response

```go
// Server side
server.OnMessage(func(env *umicp.Envelope, conn *websocket.Connection) {
    if env.Operation == umicp.OperationRequest {
        // Handle request
        response, _ := umicp.NewEnvelope().
            From("server").
            To(env.From).
            Operation(umicp.OperationResponse).
            Capability("result", "success").
            Build()
        
        server.Send(ctx, conn.ID, response)
    }
})
```

### Pattern 2: Broadcasting

```go
// Broadcast to all peers
message, _ := umicp.NewEnvelope().
    From("coordinator").
    To("*").
    Operation(umicp.OperationData).
    Capability("announcement", "System update").
    Build()

count := peer.Broadcast(ctx, message, "")
fmt.Printf("Sent to %d peers\n", count)
```

### Pattern 3: Filtering by Type

```go
// Broadcast only to incoming connections
count := peer.BroadcastToType(ctx, message, peer.PeerTypeIncoming, "")

// Or only outgoing
count := peer.BroadcastToType(ctx, message, peer.PeerTypeOutgoing, "")
```

---

## 🔧 Configuration

### Client Configuration

```go
config := websocket.ClientConfig{
    URL:                  "ws://server:8080/ws",
    HeartbeatInterval:    30 * time.Second,
    MaxReconnectAttempts: 5,
    ReconnectDelay:       time.Second,
}
```

### Server Configuration

```go
config := websocket.ServerConfig{
    Addr:        "127.0.0.1:8080",
    Path:        "/ws",
    MaxPayload:  100 * 1024 * 1024, // 100MB
    Compression: false,
}
```

### Peer Configuration

```go
config := peer.PeerConfig{
    PeerID:           "my-unique-id",
    AutoProtocol:     true,
    HandshakeTimeout: 10 * time.Second,
    Metadata:         map[string]string{
        "version": "1.0",
        "type":    "worker",
    },
}
```

---

## 📊 Monitoring & Statistics

```go
// Client stats
stats := client.Stats()
fmt.Printf("Sent: %d, Received: %d\n", 
    stats.MessagesSent, stats.MessagesReceived)

// Peer stats
stats := peer.GetStats()
fmt.Printf("Total: %d, Incoming: %d, Outgoing: %d\n",
    stats.TotalPeers, 
    stats.IncomingConnections, 
    stats.OutgoingConnections)
```

---

## 🐛 Troubleshooting

### Issue: Connection Refused
**Solution**: Check server is running and address is correct.

### Issue: Handshake Timeout
**Solution**: Ensure `AutoProtocol: true` on both peers.

### Issue: Messages Not Received
**Solution**: Check event handlers are registered before connecting.

### Issue: Import Errors
**Solution**: Run `go mod tidy` to fetch dependencies.

---

## 📖 Next Steps

1. ✅ **Run Examples**: Try all 3 examples
2. ✅ **Read API Docs**: Check [docs/](docs/) directory
3. ✅ **Build Your App**: Start with tutorials above
4. ✅ **Test Thoroughly**: Use provided test patterns
5. ✅ **Monitor**: Use statistics for production monitoring

---

## 🔗 Helpful Links

- [Full Documentation](docs/INDEX.md)
- [Implementation Roadmap](docs/IMPLEMENTATION_ROADMAP.md)
- [TypeScript Comparison](docs/COMPARISON.md)
- [Final Status](FINAL_STATUS.md)
- [Contributing](CONTRIBUTING.md)

---

## 💡 Tips

1. **Always use context**: Pass `context.Context` for cancellation
2. **Handle errors**: All functions return errors explicitly
3. **Close connections**: Use `defer` to ensure cleanup
4. **Test coverage**: Aim for 80%+ in your code
5. **Use builder pattern**: Envelope builder is type-safe

---

**Happy Coding! 🚀**

For questions, see [GitHub Discussions](https://github.com/hivellm/umicp/discussions)

**Version**: 1.0.0-mvp  
**Status**: Production Ready  
**Last Updated**: October 10, 2025

