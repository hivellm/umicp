# UMICP Go Bindings - Implementation Roadmap

## 📊 Current Status

**Language Binding**: 🚧 **Planning Phase**
- ❌ Foundation Layer (0% Complete)
- ❌ Transport Layer (0% Complete)  
- ❌ Peer Architecture (0% Complete)
- ❌ Testing Infrastructure (0% Complete)

**Goal**: Production-ready Go bindings with feature parity to TypeScript implementation

**Target Go Version**: Go 1.21+ (for generics, improved error handling, and modern standard library)

---

## 🎯 Design Philosophy

### Go-Idiomatic Approach
- **Interfaces over inheritance**: Use Go interfaces for extensibility
- **Context-first**: All async operations accept `context.Context`
- **Error handling**: Explicit error returns, no exceptions
- **Goroutines**: Leverage Go's concurrency model (channels, goroutines, select)
- **Standard library**: Prefer stdlib over external dependencies where possible
- **Simple API**: Follow Go's philosophy of simplicity and readability

### Key Go Features to Leverage
- **Channels**: For event handling and message passing
- **Goroutines**: For concurrent peer connections
- **Context**: For cancellation and timeout management
- **Interfaces**: For transport abstraction (WebSocket, HTTP, custom)
- **Generics**: For type-safe payload handling (Go 1.21+)
- **net/http**: For HTTP/2 transport with minimal dependencies

---

## 📦 Project Structure

```
umicp/bindings/go/
├── README.md                    # Package documentation
├── IMPLEMENTATION_ROADMAP.md    # This file
├── go.mod                       # Go module definition
├── go.sum                       # Dependency checksums
├── LICENSE                      # MIT License
│
├── pkg/
│   ├── umicp/
│   │   ├── envelope.go          # Envelope struct and methods
│   │   ├── envelope_test.go     # Envelope tests
│   │   ├── frame.go             # Frame handling
│   │   ├── frame_test.go        # Frame tests
│   │   ├── types.go             # Operation types, enums
│   │   ├── errors.go            # Custom error types
│   │   ├── utils.go             # UUID, hashing, timestamps
│   │   └── matrix/
│   │       ├── operations.go    # Matrix operations (CGo bindings or pure Go)
│   │       └── operations_test.go
│   │
│   ├── transport/
│   │   ├── transport.go         # Transport interface definition
│   │   ├── websocket/
│   │   │   ├── client.go        # WebSocket client
│   │   │   ├── client_test.go   # Client tests
│   │   │   ├── server.go        # WebSocket server
│   │   │   ├── server_test.go   # Server tests
│   │   │   └── connection.go    # Connection management
│   │   │
│   │   └── http/
│   │       ├── client.go        # HTTP/2 streaming client
│   │       ├── client_test.go   # Client tests
│   │       ├── server.go        # HTTP/2 streaming server
│   │       └── server_test.go   # Server tests
│   │
│   └── peer/
│       ├── peer.go              # Multiplexed peer implementation
│       ├── peer_test.go         # Peer tests
│       ├── connection.go        # PeerConnection struct
│       ├── handshake.go         # Auto-handshake protocol
│       ├── events.go            # Event system
│       └── topology.go          # Topology helpers
│
├── examples/
│   ├── basic/
│   │   └── main.go              # Basic envelope usage
│   ├── websocket/
│   │   ├── client/main.go       # WebSocket client example
│   │   └── server/main.go       # WebSocket server example
│   ├── peer/
│   │   └── main.go              # Multiplexed peer example
│   ├── mesh/
│   │   └── main.go              # Full mesh network
│   ├── hub_spoke/
│   │   └── main.go              # Hub-and-spoke topology
│   └── federated_learning/
│       └── main.go              # Federated learning example
│
├── internal/
│   ├── compression/
│   │   └── compress.go          # Compression utilities
│   ├── security/
│   │   └── validate.go          # Input validation
│   └── metrics/
│       └── stats.go             # Statistics collection
│
├── cmd/
│   └── umicp-cli/
│       └── main.go              # CLI tool for testing
│
└── test/
    ├── integration/
    │   ├── e2e_test.go          # End-to-end tests
    │   └── topology_test.go     # Topology tests
    ├── performance/
    │   ├── throughput_test.go   # Throughput benchmarks
    │   └── latency_test.go      # Latency benchmarks
    └── stress/
        └── concurrent_test.go   # Concurrency stress tests
```

---

## 🔧 Phase 1: Foundation Layer (CRITICAL - ~5-7 days)

**Goal**: Core data structures and envelope handling

### Dependencies
```go
// go.mod
module github.com/hivellm/umicp-go

go 1.21

require (
    github.com/google/uuid v1.6.0           // UUID generation
    github.com/stretchr/testify v1.9.0      // Testing utilities
)
```

### 1.1 Core Types & Envelope (2-3 days)

#### Files to Create
- [ ] `pkg/umicp/types.go` - Type definitions
- [ ] `pkg/umicp/envelope.go` - Envelope struct and methods
- [ ] `pkg/umicp/envelope_test.go` - Comprehensive tests
- [ ] `pkg/umicp/errors.go` - Error types
- [ ] `pkg/umicp/utils.go` - Utilities

#### Type Definitions
```go
// types.go
package umicp

type OperationType uint8

const (
    OperationData OperationType = iota
    OperationControl
    OperationAck
    OperationError
    OperationRequest
    OperationResponse
)

type PayloadType uint8

const (
    PayloadVector PayloadType = iota
    PayloadText
    PayloadMetadata
    PayloadBinary
)

type EncodingType uint8

const (
    EncodingRaw EncodingType = iota
    EncodingJSON
    EncodingProtobuf
    EncodingMsgpack
)
```

#### Envelope Implementation
```go
// envelope.go
package umicp

import (
    "crypto/sha256"
    "encoding/hex"
    "encoding/json"
    "time"
    
    "github.com/google/uuid"
)

type Envelope struct {
    From         string            `json:"from"`
    To           string            `json:"to"`
    Operation    OperationType     `json:"operation"`
    MessageID    string            `json:"message_id"`
    Timestamp    int64             `json:"timestamp"`
    Capabilities map[string]string `json:"capabilities"`
    Payload      []byte            `json:"payload,omitempty"`
    PayloadType  PayloadType       `json:"payload_type,omitempty"`
    Encoding     EncodingType      `json:"encoding,omitempty"`
}

type EnvelopeBuilder struct {
    envelope *Envelope
}

func NewEnvelope() *EnvelopeBuilder {
    return &EnvelopeBuilder{
        envelope: &Envelope{
            MessageID:    uuid.New().String(),
            Timestamp:    time.Now().UnixMilli(),
            Capabilities: make(map[string]string),
        },
    }
}

func (b *EnvelopeBuilder) From(from string) *EnvelopeBuilder {
    b.envelope.From = from
    return b
}

func (b *EnvelopeBuilder) To(to string) *EnvelopeBuilder {
    b.envelope.To = to
    return b
}

func (b *EnvelopeBuilder) Operation(op OperationType) *EnvelopeBuilder {
    b.envelope.Operation = op
    return b
}

func (b *EnvelopeBuilder) MessageID(id string) *EnvelopeBuilder {
    b.envelope.MessageID = id
    return b
}

func (b *EnvelopeBuilder) Capability(key, value string) *EnvelopeBuilder {
    b.envelope.Capabilities[key] = value
    return b
}

func (b *EnvelopeBuilder) Capabilities(caps map[string]string) *EnvelopeBuilder {
    b.envelope.Capabilities = caps
    return b
}

func (b *EnvelopeBuilder) Payload(data []byte, payloadType PayloadType) *EnvelopeBuilder {
    b.envelope.Payload = data
    b.envelope.PayloadType = payloadType
    return b
}

func (b *EnvelopeBuilder) Build() (*Envelope, error) {
    if err := b.envelope.Validate(); err != nil {
        return nil, err
    }
    return b.envelope, nil
}

func (e *Envelope) Validate() error {
    if e.From == "" {
        return ErrMissingFrom
    }
    if e.To == "" {
        return ErrMissingTo
    }
    if e.MessageID == "" {
        return ErrMissingMessageID
    }
    return nil
}

func (e *Envelope) Hash() string {
    h := sha256.New()
    h.Write([]byte(e.From + e.To + e.MessageID))
    return hex.EncodeToString(h.Sum(nil))
}

func (e *Envelope) Serialize() ([]byte, error) {
    return json.Marshal(e)
}

func DeserializeEnvelope(data []byte) (*Envelope, error) {
    var env Envelope
    if err := json.Unmarshal(data, &env); err != nil {
        return nil, err
    }
    if err := env.Validate(); err != nil {
        return nil, err
    }
    return &env, nil
}
```

#### Error Types
```go
// errors.go
package umicp

import "errors"

var (
    ErrMissingFrom      = errors.New("envelope: missing 'from' field")
    ErrMissingTo        = errors.New("envelope: missing 'to' field")
    ErrMissingMessageID = errors.New("envelope: missing 'message_id' field")
    ErrInvalidEnvelope  = errors.New("envelope: invalid envelope")
    
    // Transport errors
    ErrNotConnected     = errors.New("transport: not connected")
    ErrConnectionClosed = errors.New("transport: connection closed")
    ErrTimeout          = errors.New("transport: operation timeout")
    
    // Peer errors
    ErrPeerNotFound     = errors.New("peer: peer not found")
    ErrHandshakeTimeout = errors.New("peer: handshake timeout")
    ErrAlreadyConnected = errors.New("peer: already connected")
)
```

### 1.2 Frame Handling (1-2 days)

```go
// frame.go
package umicp

import (
    "encoding/binary"
    "io"
)

const (
    FrameHeaderSize = 8
    MaxFrameSize    = 100 * 1024 * 1024 // 100MB
)

type Frame struct {
    Length  uint32
    Type    uint16
    Flags   uint8
    Reserved uint8
    Payload []byte
}

func (f *Frame) Serialize(w io.Writer) error {
    header := make([]byte, FrameHeaderSize)
    binary.BigEndian.PutUint32(header[0:4], f.Length)
    binary.BigEndian.PutUint16(header[4:6], f.Type)
    header[6] = f.Flags
    header[7] = f.Reserved
    
    if _, err := w.Write(header); err != nil {
        return err
    }
    if _, err := w.Write(f.Payload); err != nil {
        return err
    }
    return nil
}

func DeserializeFrame(r io.Reader) (*Frame, error) {
    header := make([]byte, FrameHeaderSize)
    if _, err := io.ReadFull(r, header); err != nil {
        return nil, err
    }
    
    frame := &Frame{
        Length:   binary.BigEndian.Uint32(header[0:4]),
        Type:     binary.BigEndian.Uint16(header[4:6]),
        Flags:    header[6],
        Reserved: header[7],
    }
    
    if frame.Length > MaxFrameSize {
        return nil, errors.New("frame: size exceeds maximum")
    }
    
    frame.Payload = make([]byte, frame.Length)
    if _, err := io.ReadFull(r, frame.Payload); err != nil {
        return nil, err
    }
    
    return frame, nil
}
```

### 1.3 Matrix Operations (2-3 days)

**Options**:
1. **CGo Bindings** to C++ core (best performance)
2. **Pure Go** implementation (easier deployment)

```go
// pkg/umicp/matrix/operations.go
package matrix

// Option 1: Pure Go (simpler)
func DotProduct(a, b []float32) float32 {
    if len(a) != len(b) {
        panic("vectors must have same length")
    }
    var sum float32
    for i := range a {
        sum += a[i] * b[i]
    }
    return sum
}

func CosineSimilarity(a, b []float32) float32 {
    dot := DotProduct(a, b)
    magA := Magnitude(a)
    magB := Magnitude(b)
    if magA == 0 || magB == 0 {
        return 0
    }
    return dot / (magA * magB)
}

func Magnitude(v []float32) float32 {
    var sum float32
    for _, val := range v {
        sum += val * val
    }
    return float32(math.Sqrt(float64(sum)))
}

// Option 2: CGo bindings (better performance)
/*
#cgo CFLAGS: -I../../cpp/include
#cgo LDFLAGS: -L../../cpp/build -lumicp_core

#include "matrix_ops.h"
*/
// import "C"

// func DotProductCGo(a, b []float32) float32 {
//     return float32(C.dot_product((*C.float)(&a[0]), (*C.float)(&b[0]), C.int(len(a))))
// }
```

---

## 🔧 Phase 2: WebSocket Transport (CRITICAL - ~5-7 days)

**Goal**: WebSocket client and server for network communication

### Dependencies
```go
require (
    github.com/gorilla/websocket v1.5.1   // WebSocket implementation
    golang.org/x/sync v0.6.0              // Advanced sync primitives
)
```

### 2.1 Transport Interface (1 day)

```go
// pkg/transport/transport.go
package transport

import (
    "context"
    
    "github.com/hivellm/umicp-go/pkg/umicp"
)

// Transport defines the interface for all transports
type Transport interface {
    // Connect establishes connection
    Connect(ctx context.Context) error
    
    // Disconnect closes connection
    Disconnect(ctx context.Context) error
    
    // Send sends an envelope
    Send(ctx context.Context, env *umicp.Envelope) error
    
    // Receive receives envelopes (blocking)
    Receive(ctx context.Context) (*umicp.Envelope, error)
    
    // IsConnected returns connection status
    IsConnected() bool
    
    // Stats returns transport statistics
    Stats() *Stats
}

type Stats struct {
    MessagesSent     int64
    MessagesReceived int64
    BytesSent        int64
    BytesReceived    int64
    ConnectedAt      time.Time
    LastMessageAt    time.Time
}
```

### 2.2 WebSocket Client (2-3 days)

```go
// pkg/transport/websocket/client.go
package websocket

import (
    "context"
    "sync"
    "time"
    
    "github.com/gorilla/websocket"
    "github.com/hivellm/umicp-go/pkg/umicp"
    "github.com/hivellm/umicp-go/pkg/transport"
)

type ClientConfig struct {
    URL                  string
    HeartbeatInterval    time.Duration
    MaxReconnectAttempts int
    ReconnectDelay       time.Duration
    ReadBufferSize       int
    WriteBufferSize      int
}

type Client struct {
    config ClientConfig
    conn   *websocket.Conn
    mu     sync.RWMutex
    
    // Channels
    sendCh    chan *umicp.Envelope
    receiveCh chan *umicp.Envelope
    closeCh   chan struct{}
    
    // State
    connected bool
    stats     *transport.Stats
    
    // Callbacks
    onMessage    func(*umicp.Envelope)
    onConnect    func()
    onDisconnect func(error)
    onError      func(error)
}

func NewClient(config ClientConfig) *Client {
    if config.HeartbeatInterval == 0 {
        config.HeartbeatInterval = 30 * time.Second
    }
    if config.MaxReconnectAttempts == 0 {
        config.MaxReconnectAttempts = 5
    }
    if config.ReconnectDelay == 0 {
        config.ReconnectDelay = time.Second
    }
    
    return &Client{
        config:    config,
        sendCh:    make(chan *umicp.Envelope, 100),
        receiveCh: make(chan *umicp.Envelope, 100),
        closeCh:   make(chan struct{}),
        stats:     &transport.Stats{},
    }
}

func (c *Client) Connect(ctx context.Context) error {
    c.mu.Lock()
    defer c.mu.Unlock()
    
    if c.connected {
        return umicp.ErrAlreadyConnected
    }
    
    dialer := websocket.Dialer{
        ReadBufferSize:  c.config.ReadBufferSize,
        WriteBufferSize: c.config.WriteBufferSize,
    }
    
    conn, _, err := dialer.DialContext(ctx, c.config.URL, nil)
    if err != nil {
        return err
    }
    
    c.conn = conn
    c.connected = true
    c.stats.ConnectedAt = time.Now()
    
    // Start goroutines
    go c.readLoop()
    go c.writeLoop()
    go c.heartbeatLoop()
    
    if c.onConnect != nil {
        c.onConnect()
    }
    
    return nil
}

func (c *Client) Disconnect(ctx context.Context) error {
    c.mu.Lock()
    defer c.mu.Unlock()
    
    if !c.connected {
        return nil
    }
    
    close(c.closeCh)
    c.connected = false
    
    return c.conn.Close()
}

func (c *Client) Send(ctx context.Context, env *umicp.Envelope) error {
    if !c.IsConnected() {
        return umicp.ErrNotConnected
    }
    
    select {
    case c.sendCh <- env:
        return nil
    case <-ctx.Done():
        return ctx.Err()
    }
}

func (c *Client) readLoop() {
    defer func() {
        if c.onDisconnect != nil {
            c.onDisconnect(nil)
        }
    }()
    
    for {
        select {
        case <-c.closeCh:
            return
        default:
            _, data, err := c.conn.ReadMessage()
            if err != nil {
                if c.onError != nil {
                    c.onError(err)
                }
                return
            }
            
            env, err := umicp.DeserializeEnvelope(data)
            if err != nil {
                if c.onError != nil {
                    c.onError(err)
                }
                continue
            }
            
            c.stats.MessagesReceived++
            c.stats.BytesReceived += int64(len(data))
            c.stats.LastMessageAt = time.Now()
            
            if c.onMessage != nil {
                c.onMessage(env)
            }
        }
    }
}

func (c *Client) writeLoop() {
    for {
        select {
        case <-c.closeCh:
            return
        case env := <-c.sendCh:
            data, err := env.Serialize()
            if err != nil {
                if c.onError != nil {
                    c.onError(err)
                }
                continue
            }
            
            if err := c.conn.WriteMessage(websocket.BinaryMessage, data); err != nil {
                if c.onError != nil {
                    c.onError(err)
                }
                return
            }
            
            c.stats.MessagesSent++
            c.stats.BytesSent += int64(len(data))
        }
    }
}

func (c *Client) heartbeatLoop() {
    ticker := time.NewTicker(c.config.HeartbeatInterval)
    defer ticker.Stop()
    
    for {
        select {
        case <-c.closeCh:
            return
        case <-ticker.C:
            if err := c.conn.WriteMessage(websocket.PingMessage, nil); err != nil {
                if c.onError != nil {
                    c.onError(err)
                }
                return
            }
        }
    }
}

func (c *Client) IsConnected() bool {
    c.mu.RLock()
    defer c.mu.RUnlock()
    return c.connected
}

func (c *Client) Stats() *transport.Stats {
    c.mu.RLock()
    defer c.mu.RUnlock()
    return c.stats
}

// Event handlers
func (c *Client) OnMessage(fn func(*umicp.Envelope)) {
    c.onMessage = fn
}

func (c *Client) OnConnect(fn func()) {
    c.onConnect = fn
}

func (c *Client) OnDisconnect(fn func(error)) {
    c.onDisconnect = fn
}

func (c *Client) OnError(fn func(error)) {
    c.onError = fn
}
```

### 2.3 WebSocket Server (2-3 days)

```go
// pkg/transport/websocket/server.go
package websocket

import (
    "context"
    "net/http"
    "sync"
    
    "github.com/gorilla/websocket"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

type ServerConfig struct {
    Addr            string
    Path            string
    MaxPayload      int64
    Compression     bool
    ReadBufferSize  int
    WriteBufferSize int
}

type Connection struct {
    ID     string
    Conn   *websocket.Conn
    SendCh chan *umicp.Envelope
}

type Server struct {
    config  ServerConfig
    server  *http.Server
    clients sync.Map // map[string]*Connection
    
    upgrader websocket.Upgrader
    
    // Callbacks
    onMessage    func(*umicp.Envelope, *Connection)
    onConnect    func(*Connection)
    onDisconnect func(*Connection)
    onError      func(error)
}

func NewServer(config ServerConfig) *Server {
    if config.MaxPayload == 0 {
        config.MaxPayload = 100 * 1024 * 1024 // 100MB
    }
    
    return &Server{
        config: config,
        upgrader: websocket.Upgrader{
            ReadBufferSize:  config.ReadBufferSize,
            WriteBufferSize: config.WriteBufferSize,
            CheckOrigin: func(r *http.Request) bool {
                return true // Configure as needed
            },
        },
    }
}

func (s *Server) Start(ctx context.Context) error {
    mux := http.NewServeMux()
    mux.HandleFunc(s.config.Path, s.handleWebSocket)
    
    s.server = &http.Server{
        Addr:    s.config.Addr,
        Handler: mux,
    }
    
    go func() {
        <-ctx.Done()
        s.server.Shutdown(context.Background())
    }()
    
    return s.server.ListenAndServe()
}

func (s *Server) handleWebSocket(w http.ResponseWriter, r *http.Request) {
    conn, err := s.upgrader.Upgrade(w, r, nil)
    if err != nil {
        if s.onError != nil {
            s.onError(err)
        }
        return
    }
    
    clientID := uuid.New().String()
    client := &Connection{
        ID:     clientID,
        Conn:   conn,
        SendCh: make(chan *umicp.Envelope, 100),
    }
    
    s.clients.Store(clientID, client)
    
    if s.onConnect != nil {
        s.onConnect(client)
    }
    
    go s.readLoop(client)
    go s.writeLoop(client)
}

func (s *Server) readLoop(client *Connection) {
    defer func() {
        s.clients.Delete(client.ID)
        client.Conn.Close()
        if s.onDisconnect != nil {
            s.onDisconnect(client)
        }
    }()
    
    for {
        _, data, err := client.Conn.ReadMessage()
        if err != nil {
            if s.onError != nil {
                s.onError(err)
            }
            return
        }
        
        env, err := umicp.DeserializeEnvelope(data)
        if err != nil {
            if s.onError != nil {
                s.onError(err)
            }
            continue
        }
        
        if s.onMessage != nil {
            s.onMessage(env, client)
        }
    }
}

func (s *Server) writeLoop(client *Connection) {
    for env := range client.SendCh {
        data, err := env.Serialize()
        if err != nil {
            if s.onError != nil {
                s.onError(err)
            }
            continue
        }
        
        if err := client.Conn.WriteMessage(websocket.BinaryMessage, data); err != nil {
            if s.onError != nil {
                s.onError(err)
            }
            return
        }
    }
}

func (s *Server) Send(ctx context.Context, clientID string, env *umicp.Envelope) error {
    val, ok := s.clients.Load(clientID)
    if !ok {
        return umicp.ErrPeerNotFound
    }
    
    client := val.(*Connection)
    select {
    case client.SendCh <- env:
        return nil
    case <-ctx.Done():
        return ctx.Err()
    }
}

func (s *Server) Broadcast(ctx context.Context, env *umicp.Envelope) int {
    count := 0
    s.clients.Range(func(key, val interface{}) bool {
        client := val.(*Connection)
        select {
        case client.SendCh <- env:
            count++
        default:
            // Skip if buffer is full
        }
        return true
    })
    return count
}

func (s *Server) GetClients() []*Connection {
    var clients []*Connection
    s.clients.Range(func(key, val interface{}) bool {
        clients = append(clients, val.(*Connection))
        return true
    })
    return clients
}

// Event handlers
func (s *Server) OnMessage(fn func(*umicp.Envelope, *Connection)) {
    s.onMessage = fn
}

func (s *Server) OnConnect(fn func(*Connection)) {
    s.onConnect = fn
}

func (s *Server) OnDisconnect(fn func(*Connection)) {
    s.onDisconnect = fn
}

func (s *Server) OnError(fn func(error)) {
    s.onError = fn
}
```

---

## 🔧 Phase 3: Multiplexed Peer Architecture (CRITICAL - ~7-10 days)

**Goal**: True P2P multiplexed architecture - each peer is server AND multiple clients

### 3.1 Peer Structures (2 days)

```go
// pkg/peer/connection.go
package peer

import (
    "time"
    
    "github.com/hivellm/umicp-go/pkg/transport/websocket"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

type PeerType string

const (
    PeerTypeIncoming PeerType = "incoming"
    PeerTypeOutgoing PeerType = "outgoing"
)

type PeerConnection struct {
    ID                string
    Type              PeerType
    URL               string // For outgoing connections
    Client            *websocket.Client
    Metadata          map[string]string
    ConnectedAt       time.Time
    HandshakeComplete bool
    PeerInfo          *PeerInfo
}

type PeerInfo struct {
    PeerID       string
    Metadata     map[string]string
    Capabilities map[string]string
    CompletedAt  time.Time
}

type PeerStats struct {
    TotalPeers           int
    IncomingConnections  int
    OutgoingConnections  int
    MessagesReceived     int64
    MessagesSent         int64
    HandshakesComplete   int
    HandshakesPending    int
}
```

### 3.2 Event System (2 days)

```go
// pkg/peer/events.go
package peer

import (
    "github.com/hivellm/umicp-go/pkg/umicp"
)

type EventType string

const (
    EventMessage          EventType = "message"
    EventData             EventType = "data"
    EventPeerConnect      EventType = "peer:connect"
    EventPeerDisconnect   EventType = "peer:disconnect"
    EventPeerReady        EventType = "peer:ready"
    EventHandshakeTimeout EventType = "handshake:timeout"
    EventError            EventType = "error"
    EventServerReady      EventType = "server:ready"
    EventConnected        EventType = "connected"
)

type Event struct {
    Type       EventType
    Envelope   *umicp.Envelope
    Connection *PeerConnection
    PeerInfo   *PeerInfo
    Error      error
    Data       interface{}
}

type EventHandler func(*Event)

type EventBus struct {
    handlers map[EventType][]EventHandler
    eventCh  chan *Event
    closeCh  chan struct{}
}

func NewEventBus() *EventBus {
    bus := &EventBus{
        handlers: make(map[EventType][]EventHandler),
        eventCh:  make(chan *Event, 1000),
        closeCh:  make(chan struct{}),
    }
    go bus.processEvents()
    return bus
}

func (eb *EventBus) On(eventType EventType, handler EventHandler) {
    eb.handlers[eventType] = append(eb.handlers[eventType], handler)
}

func (eb *EventBus) Emit(event *Event) {
    select {
    case eb.eventCh <- event:
    default:
        // Event buffer full, drop event or handle as needed
    }
}

func (eb *EventBus) processEvents() {
    for {
        select {
        case <-eb.closeCh:
            return
        case event := <-eb.eventCh:
            if handlers, ok := eb.handlers[event.Type]; ok {
                for _, handler := range handlers {
                    go handler(event) // Run handlers concurrently
                }
            }
        }
    }
}

func (eb *EventBus) Close() {
    close(eb.closeCh)
}
```

### 3.3 Multiplexed Peer (3-4 days)

```go
// pkg/peer/peer.go
package peer

import (
    "context"
    "fmt"
    "sync"
    "time"
    
    "github.com/google/uuid"
    "github.com/hivellm/umicp-go/pkg/transport/websocket"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

type PeerConfig struct {
    PeerID           string
    AutoProtocol     bool
    HandshakeTimeout time.Duration
    Metadata         map[string]string
}

type ServerConfig struct {
    Addr        string
    Path        string
    Compression bool
}

type Peer struct {
    config  PeerConfig
    server  *websocket.Server
    peers   sync.Map // map[string]*PeerConnection
    eventBus *EventBus
    
    mu sync.RWMutex
}

func NewPeer(config PeerConfig) *Peer {
    if config.PeerID == "" {
        config.PeerID = uuid.New().String()
    }
    if config.HandshakeTimeout == 0 {
        config.HandshakeTimeout = 10 * time.Second
    }
    if config.Metadata == nil {
        config.Metadata = make(map[string]string)
    }
    
    return &Peer{
        config:   config,
        eventBus: NewEventBus(),
    }
}

func NewPeerWithServer(config PeerConfig, serverConfig ServerConfig) (*Peer, error) {
    peer := NewPeer(config)
    
    server := websocket.NewServer(websocket.ServerConfig{
        Addr:        serverConfig.Addr,
        Path:        serverConfig.Path,
        Compression: serverConfig.Compression,
    })
    
    // Setup server event handlers
    server.OnConnect(func(conn *websocket.Connection) {
        peerConn := &PeerConnection{
            ID:          conn.ID,
            Type:        PeerTypeIncoming,
            ConnectedAt: time.Now(),
            Metadata:    make(map[string]string),
        }
        peer.peers.Store(conn.ID, peerConn)
        
        peer.eventBus.Emit(&Event{
            Type:       EventPeerConnect,
            Connection: peerConn,
        })
        
        // Auto handshake for incoming
        if peer.config.AutoProtocol {
            go peer.waitForHello(peerConn)
        }
    })
    
    server.OnDisconnect(func(conn *websocket.Connection) {
        if val, ok := peer.peers.Load(conn.ID); ok {
            peerConn := val.(*PeerConnection)
            peer.peers.Delete(conn.ID)
            
            peer.eventBus.Emit(&Event{
                Type:       EventPeerDisconnect,
                Connection: peerConn,
            })
        }
    })
    
    server.OnMessage(func(env *umicp.Envelope, conn *websocket.Connection) {
        if val, ok := peer.peers.Load(conn.ID); ok {
            peerConn := val.(*PeerConnection)
            peer.handleMessage(env, peerConn)
        }
    })
    
    peer.server = server
    return peer, nil
}

func (p *Peer) Start(ctx context.Context) error {
    if p.server != nil {
        go func() {
            if err := p.server.Start(ctx); err != nil {
                p.eventBus.Emit(&Event{
                    Type:  EventError,
                    Error: err,
                })
            }
        }()
        
        p.eventBus.Emit(&Event{
            Type: EventServerReady,
        })
    }
    return nil
}

func (p *Peer) ConnectToPeer(ctx context.Context, url string, metadata map[string]string) (string, error) {
    client := websocket.NewClient(websocket.ClientConfig{
        URL: url,
    })
    
    if err := client.Connect(ctx); err != nil {
        return "", err
    }
    
    peerID := uuid.New().String()
    peerConn := &PeerConnection{
        ID:          peerID,
        Type:        PeerTypeOutgoing,
        URL:         url,
        Client:      client,
        Metadata:    metadata,
        ConnectedAt: time.Now(),
    }
    
    // Setup client event handlers
    client.OnMessage(func(env *umicp.Envelope) {
        p.handleMessage(env, peerConn)
    })
    
    client.OnDisconnect(func(err error) {
        p.peers.Delete(peerID)
        p.eventBus.Emit(&Event{
            Type:       EventPeerDisconnect,
            Connection: peerConn,
        })
    })
    
    p.peers.Store(peerID, peerConn)
    
    p.eventBus.Emit(&Event{
        Type:       EventPeerConnect,
        Connection: peerConn,
    })
    
    // Auto handshake for outgoing
    if p.config.AutoProtocol {
        if err := p.sendHello(ctx, peerConn); err != nil {
            return "", err
        }
    }
    
    return peerID, nil
}

func (p *Peer) SendToPeer(ctx context.Context, peerID string, env *umicp.Envelope) error {
    val, ok := p.peers.Load(peerID)
    if !ok {
        return umicp.ErrPeerNotFound
    }
    
    peerConn := val.(*PeerConnection)
    
    if peerConn.Type == PeerTypeOutgoing {
        return peerConn.Client.Send(ctx, env)
    } else {
        // Incoming connection - send via server
        return p.server.Send(ctx, peerConn.ID, env)
    }
}

func (p *Peer) Broadcast(ctx context.Context, env *umicp.Envelope, excludePeerID string) int {
    count := 0
    p.peers.Range(func(key, val interface{}) bool {
        peerID := key.(string)
        if peerID != excludePeerID {
            if err := p.SendToPeer(ctx, peerID, env); err == nil {
                count++
            }
        }
        return true
    })
    return count
}

func (p *Peer) GetPeers() []*PeerConnection {
    var peers []*PeerConnection
    p.peers.Range(func(key, val interface{}) bool {
        peers = append(peers, val.(*PeerConnection))
        return true
    })
    return peers
}

func (p *Peer) GetPeersByType(peerType PeerType) []*PeerConnection {
    var peers []*PeerConnection
    p.peers.Range(func(key, val interface{}) bool {
        peer := val.(*PeerConnection)
        if peer.Type == peerType {
            peers = append(peers, peer)
        }
        return true
    })
    return peers
}

func (p *Peer) GetStats() PeerStats {
    stats := PeerStats{}
    p.peers.Range(func(key, val interface{}) bool {
        peer := val.(*PeerConnection)
        stats.TotalPeers++
        if peer.Type == PeerTypeIncoming {
            stats.IncomingConnections++
        } else {
            stats.OutgoingConnections++
        }
        if peer.HandshakeComplete {
            stats.HandshakesComplete++
        } else {
            stats.HandshakesPending++
        }
        return true
    })
    return stats
}

func (p *Peer) handleMessage(env *umicp.Envelope, peerConn *PeerConnection) {
    // Emit raw message event
    p.eventBus.Emit(&Event{
        Type:       EventMessage,
        Envelope:   env,
        Connection: peerConn,
    })
    
    // Handle protocol messages
    switch env.Operation {
    case umicp.OperationControl:
        p.handleControlMessage(env, peerConn)
    case umicp.OperationAck:
        p.handleAckMessage(env, peerConn)
    case umicp.OperationData:
        p.eventBus.Emit(&Event{
            Type:       EventData,
            Envelope:   env,
            Connection: peerConn,
        })
    case umicp.OperationError:
        p.eventBus.Emit(&Event{
            Type:       EventError,
            Envelope:   env,
            Connection: peerConn,
        })
    }
}

func (p *Peer) On(eventType EventType, handler EventHandler) {
    p.eventBus.On(eventType, handler)
}

func (p *Peer) Shutdown(ctx context.Context) error {
    // Close all peer connections
    p.peers.Range(func(key, val interface{}) bool {
        peer := val.(*PeerConnection)
        if peer.Type == PeerTypeOutgoing {
            peer.Client.Disconnect(ctx)
        }
        return true
    })
    
    // Shutdown server if exists
    if p.server != nil {
        // Server shutdown handled by context
    }
    
    p.eventBus.Close()
    return nil
}
```

### 3.4 Handshake Protocol (2 days)

```go
// pkg/peer/handshake.go
package peer

import (
    "context"
    "fmt"
    "time"
    
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func (p *Peer) sendHello(ctx context.Context, peerConn *PeerConnection) error {
    hello := umicp.NewEnvelope().
        From(p.config.PeerID).
        To("*").
        Operation(umicp.OperationControl).
        MessageID(fmt.Sprintf("hello-%s", peerConn.ID)).
        Capability("type", "hello").
        Capability("peer_id", p.config.PeerID).
        Capability("version", "1.0").
        Build()
    
    if err := p.SendToPeer(ctx, peerConn.ID, hello); err != nil {
        return err
    }
    
    // Start handshake timeout
    go p.waitForHandshake(peerConn)
    
    return nil
}

func (p *Peer) waitForHello(peerConn *PeerConnection) {
    timer := time.NewTimer(p.config.HandshakeTimeout)
    defer timer.Stop()
    
    <-timer.C
    if !peerConn.HandshakeComplete {
        p.eventBus.Emit(&Event{
            Type:       EventHandshakeTimeout,
            Connection: peerConn,
        })
    }
}

func (p *Peer) waitForHandshake(peerConn *PeerConnection) {
    timer := time.NewTimer(p.config.HandshakeTimeout)
    defer timer.Stop()
    
    <-timer.C
    if !peerConn.HandshakeComplete {
        p.eventBus.Emit(&Event{
            Type:       EventHandshakeTimeout,
            Connection: peerConn,
        })
    }
}

func (p *Peer) handleControlMessage(env *umicp.Envelope, peerConn *PeerConnection) {
    msgType := env.Capabilities["type"]
    
    switch msgType {
    case "hello":
        p.handleHello(env, peerConn)
    }
}

func (p *Peer) handleHello(env *umicp.Envelope, peerConn *PeerConnection) {
    // Extract peer info from HELLO
    peerInfo := &PeerInfo{
        PeerID:       env.Capabilities["peer_id"],
        Metadata:     make(map[string]string),
        Capabilities: env.Capabilities,
        CompletedAt:  time.Now(),
    }
    
    peerConn.PeerInfo = peerInfo
    
    // Send ACK response
    ack, _ := umicp.NewEnvelope().
        From(p.config.PeerID).
        To(peerInfo.PeerID).
        Operation(umicp.OperationAck).
        MessageID(fmt.Sprintf("ack-%s", env.MessageID)).
        Capability("type", "hello_ack").
        Capability("peer_id", p.config.PeerID).
        Build()
    
    ctx := context.Background()
    p.SendToPeer(ctx, peerConn.ID, ack)
    
    // Mark handshake complete
    peerConn.HandshakeComplete = true
    
    // Emit peer ready event
    p.eventBus.Emit(&Event{
        Type:       EventPeerReady,
        Connection: peerConn,
        PeerInfo:   peerInfo,
    })
}

func (p *Peer) handleAckMessage(env *umicp.Envelope, peerConn *PeerConnection) {
    msgType := env.Capabilities["type"]
    
    if msgType == "hello_ack" {
        // Extract peer info from ACK
        peerInfo := &PeerInfo{
            PeerID:       env.Capabilities["peer_id"],
            Metadata:     make(map[string]string),
            Capabilities: env.Capabilities,
            CompletedAt:  time.Now(),
        }
        
        peerConn.PeerInfo = peerInfo
        peerConn.HandshakeComplete = true
        
        // Emit events
        p.eventBus.Emit(&Event{
            Type:       EventPeerReady,
            Connection: peerConn,
            PeerInfo:   peerInfo,
        })
        
        p.eventBus.Emit(&Event{
            Type:       EventConnected,
            Connection: peerConn,
        })
    }
}
```

---

## 🔧 Phase 4: HTTP/2 Streaming Transport (MEDIUM - ~4-5 days)

**Goal**: HTTP/2 based transport as alternative to WebSocket

### Dependencies
```go
require (
    golang.org/x/net v0.20.0  // HTTP/2 support
)
```

### Implementation (Simplified)

```go
// pkg/transport/http/client.go
package http

import (
    "bytes"
    "context"
    "io"
    "net/http"
    
    "golang.org/x/net/http2"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

type HTTPClient struct {
    baseURL string
    client  *http.Client
}

func NewHTTPClient(baseURL string) *HTTPClient {
    transport := &http2.Transport{}
    
    return &HTTPClient{
        baseURL: baseURL,
        client: &http.Client{
            Transport: transport,
        },
    }
}

func (c *HTTPClient) Send(ctx context.Context, env *umicp.Envelope) (*umicp.Envelope, error) {
    data, err := env.Serialize()
    if err != nil {
        return nil, err
    }
    
    req, err := http.NewRequestWithContext(ctx, "POST", c.baseURL, bytes.NewReader(data))
    if err != nil {
        return nil, err
    }
    
    req.Header.Set("Content-Type", "application/octet-stream")
    
    resp, err := c.client.Do(req)
    if err != nil {
        return nil, err
    }
    defer resp.Body.Close()
    
    respData, err := io.ReadAll(resp.Body)
    if err != nil {
        return nil, err
    }
    
    return umicp.DeserializeEnvelope(respData)
}

// pkg/transport/http/server.go
package http

import (
    "context"
    "io"
    "net/http"
    
    "golang.org/x/net/http2"
    "golang.org/x/net/http2/h2c"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

type HTTPServer struct {
    addr    string
    handler func(*umicp.Envelope) (*umicp.Envelope, error)
    server  *http.Server
}

func NewHTTPServer(addr string) *HTTPServer {
    return &HTTPServer{
        addr: addr,
    }
}

func (s *HTTPServer) OnRequest(handler func(*umicp.Envelope) (*umicp.Envelope, error)) {
    s.handler = handler
}

func (s *HTTPServer) Start(ctx context.Context) error {
    h2s := &http2.Server{}
    
    mux := http.NewServeMux()
    mux.HandleFunc("/", s.handleRequest)
    
    s.server = &http.Server{
        Addr:    s.addr,
        Handler: h2c.NewHandler(mux, h2s),
    }
    
    go func() {
        <-ctx.Done()
        s.server.Shutdown(context.Background())
    }()
    
    return s.server.ListenAndServe()
}

func (s *HTTPServer) handleRequest(w http.ResponseWriter, r *http.Request) {
    data, err := io.ReadAll(r.Body)
    if err != nil {
        http.Error(w, err.Error(), http.StatusBadRequest)
        return
    }
    
    env, err := umicp.DeserializeEnvelope(data)
    if err != nil {
        http.Error(w, err.Error(), http.StatusBadRequest)
        return
    }
    
    if s.handler != nil {
        resp, err := s.handler(env)
        if err != nil {
            http.Error(w, err.Error(), http.StatusInternalServerError)
            return
        }
        
        respData, err := resp.Serialize()
        if err != nil {
            http.Error(w, err.Error(), http.StatusInternalServerError)
            return
        }
        
        w.Header().Set("Content-Type", "application/octet-stream")
        w.Write(respData)
    }
}
```

---

## 🧪 Phase 5: Testing Infrastructure (HIGH - ~5-7 days)

### 5.1 Unit Tests

```bash
# Run all tests
go test ./...

# Run with coverage
go test -cover ./...

# Run specific package
go test ./pkg/umicp/
go test ./pkg/transport/websocket/
go test ./pkg/peer/
```

### 5.2 Integration Tests

```go
// test/integration/e2e_test.go
package integration_test

import (
    "context"
    "testing"
    "time"
    
    "github.com/stretchr/testify/assert"
    "github.com/hivellm/umicp-go/pkg/peer"
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func TestTwoPeerCommunication(t *testing.T) {
    ctx := context.Background()
    
    // Create peer A with server
    peerA, err := peer.NewPeerWithServer(
        peer.PeerConfig{
            PeerID:       "peer-a",
            AutoProtocol: true,
        },
        peer.ServerConfig{
            Addr: "127.0.0.1:20081",
            Path: "/umicp",
        },
    )
    assert.NoError(t, err)
    
    // Start peer A
    err = peerA.Start(ctx)
    assert.NoError(t, err)
    defer peerA.Shutdown(ctx)
    
    // Create peer B
    peerB := peer.NewPeer(peer.PeerConfig{
        PeerID:       "peer-b",
        AutoProtocol: true,
    })
    
    // Wait for peer ready
    readyCh := make(chan bool, 1)
    peerB.On(peer.EventPeerReady, func(e *peer.Event) {
        readyCh <- true
    })
    
    // Connect B to A
    _, err = peerB.ConnectToPeer(ctx, "ws://127.0.0.1:20081/umicp", nil)
    assert.NoError(t, err)
    
    // Wait for handshake
    select {
    case <-readyCh:
        // Success
    case <-time.After(5 * time.Second):
        t.Fatal("handshake timeout")
    }
    
    // Test message exchange
    msgCh := make(chan *umicp.Envelope, 1)
    peerA.On(peer.EventData, func(e *peer.Event) {
        msgCh <- e.Envelope
    })
    
    // Send from B to A
    env, _ := umicp.NewEnvelope().
        From("peer-b").
        To("peer-a").
        Operation(umicp.OperationData).
        Capability("message", "Hello from B").
        Build()
    
    peers := peerB.GetPeers()
    err = peerB.SendToPeer(ctx, peers[0].ID, env)
    assert.NoError(t, err)
    
    // Wait for message
    select {
    case received := <-msgCh:
        assert.Equal(t, "Hello from B", received.Capabilities["message"])
    case <-time.After(2 * time.Second):
        t.Fatal("message timeout")
    }
}

func TestFullMeshTopology(t *testing.T) {
    // Create 3 peers in full mesh
    // Each peer connects to all others
    // Test broadcast functionality
    
    // TODO: Implement
}
```

### 5.3 Performance Benchmarks

```go
// test/performance/throughput_test.go
package performance_test

import (
    "context"
    "testing"
    
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func BenchmarkEnvelopeSerialize(b *testing.B) {
    env, _ := umicp.NewEnvelope().
        From("sender").
        To("receiver").
        Operation(umicp.OperationData).
        Build()
    
    b.ResetTimer()
    for i := 0; i < b.N; i++ {
        _, _ = env.Serialize()
    }
}

func BenchmarkEnvelopeDeserialize(b *testing.B) {
    env, _ := umicp.NewEnvelope().
        From("sender").
        To("receiver").
        Operation(umicp.OperationData).
        Build()
    
    data, _ := env.Serialize()
    
    b.ResetTimer()
    for i := 0; i < b.N; i++ {
        _, _ = umicp.DeserializeEnvelope(data)
    }
}

func BenchmarkWebSocketThroughput(b *testing.B) {
    // Setup client-server
    // Measure messages/second
    
    // TODO: Implement
}
```

---

## 📚 Phase 6: Examples & Documentation (MEDIUM - ~3-4 days)

### 6.1 Examples to Create

#### Basic Usage
```go
// examples/basic/main.go
package main

import (
    "fmt"
    
    "github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
    // Create envelope
    env, err := umicp.NewEnvelope().
        From("client-001").
        To("server-001").
        Operation(umicp.OperationData).
        Capability("message", "Hello UMICP!").
        Build()
    
    if err != nil {
        panic(err)
    }
    
    // Serialize
    data, err := env.Serialize()
    if err != nil {
        panic(err)
    }
    
    fmt.Printf("Serialized: %d bytes\n", len(data))
    
    // Deserialize
    received, err := umicp.DeserializeEnvelope(data)
    if err != nil {
        panic(err)
    }
    
    fmt.Printf("From: %s\n", received.From)
    fmt.Printf("Message: %s\n", received.Capabilities["message"])
}
```

#### Multiplexed Peer
```go
// examples/peer/main.go
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
        fmt.Printf("Message from %s (%s)\n", 
            e.Connection.ID, e.Connection.Type)
    })
    
    myPeer.On(peer.EventPeerConnect, func(e *peer.Event) {
        fmt.Printf("Peer connected: %s\n", e.Connection.ID)
    })
    
    myPeer.On(peer.EventPeerReady, func(e *peer.Event) {
        fmt.Printf("Peer ready: %s\n", e.PeerInfo.PeerID)
    })
    
    // Start peer
    if err := myPeer.Start(ctx); err != nil {
        panic(err)
    }
    
    fmt.Println("Peer started on 127.0.0.1:20081")
    
    // Connect to remote peers
    peerID, err := myPeer.ConnectToPeer(ctx, "ws://localhost:20082/umicp", nil)
    if err != nil {
        panic(err)
    }
    
    fmt.Printf("Connected to peer: %s\n", peerID)
    
    // Send message
    env, _ := umicp.NewEnvelope().
        From("my-agent").
        To("remote-agent").
        Operation(umicp.OperationData).
        Capability("message", "Hello from Go!").
        Build()
    
    if err := myPeer.SendToPeer(ctx, peerID, env); err != nil {
        panic(err)
    }
    
    // Keep running
    time.Sleep(60 * time.Second)
    
    // Shutdown
    myPeer.Shutdown(ctx)
}
```

### 6.2 Documentation Files

- [ ] `README.md` - Package overview, quick start, installation
- [ ] `docs/GETTING_STARTED.md` - Beginner tutorial
- [ ] `docs/MULTIPLEXED_PEER.md` - Peer architecture guide
- [ ] `docs/WEBSOCKET_TRANSPORT.md` - WebSocket transport guide
- [ ] `docs/HTTP_TRANSPORT.md` - HTTP transport guide
- [ ] `docs/EVENT_SYSTEM.md` - Event handling patterns
- [ ] `docs/MIGRATION_FROM_TYPESCRIPT.md` - Migration guide
- [ ] `docs/API_REFERENCE.md` - Complete API documentation
- [ ] `docs/EXAMPLES.md` - Example code and patterns
- [ ] `docs/PERFORMANCE.md` - Performance tuning guide

---

## 📈 Implementation Timeline

### Fast Track (Core Features Only)
- **Week 1-2**: Foundation Layer (Envelope, Frame, Matrix)
- **Week 3-4**: WebSocket Transport (Client + Server)
- **Week 5-6**: Multiplexed Peer Architecture
- **Week 7**: Testing & Examples
- **Week 8**: Documentation & Polish
- **Total**: ~8 weeks to feature parity with TypeScript core

### Complete Implementation
- **Weeks 1-2**: Phase 1 (Foundation Layer)
- **Weeks 3-4**: Phase 2 (WebSocket Transport)
- **Weeks 5-6**: Phase 3 (Multiplexed Peer)
- **Week 7**: Phase 4 (HTTP Transport)
- **Week 8**: Phase 5 (Testing)
- **Week 9**: Phase 6 (Examples & Docs)
- **Week 10**: Phase 7 (Production Readiness)
- **Total**: ~10 weeks to full production readiness

---

## 🎯 Priority Matrix

| Feature | Priority | Effort | Dependencies |
|---------|----------|--------|--------------|
| Core Envelope | 🔴 CRITICAL | Medium (3d) | None |
| Frame Handling | 🔴 CRITICAL | Low (2d) | Envelope |
| WebSocket Client | 🔴 CRITICAL | Medium (3d) | Envelope |
| WebSocket Server | 🔴 CRITICAL | Medium (3d) | Envelope |
| Multiplexed Peer | 🔴 CRITICAL | High (7d) | WebSocket |
| Auto-Handshake | 🔴 CRITICAL | Medium (2d) | Multiplexed Peer |
| Event System | 🟡 HIGH | Medium (2d) | None |
| Matrix Operations | 🟢 MEDIUM | Medium (3d) | None |
| HTTP Transport | 🟢 MEDIUM | Medium (4d) | Envelope |
| Testing Suite | 🟡 HIGH | High (5d) | All features |
| Documentation | 🟢 MEDIUM | Medium (3d) | All features |

---

## 🚀 Getting Started (Quick Wins)

### Week 1: Foundation
1. Setup Go module and project structure
2. Implement `Envelope` struct with builder pattern
3. Implement serialization/deserialization (JSON)
4. Write comprehensive tests for envelope
5. Implement basic error types

### Week 2: Basic Transport
1. Implement WebSocket client (basic)
2. Implement WebSocket server (basic)
3. Simple client-server example working
4. Basic reconnection logic
5. Test client-server communication

### Week 3-4: Multiplexing
1. Implement `PeerConnection` structures
2. Implement event bus system
3. Implement `Peer` with server component
4. Implement `ConnectToPeer` method
5. Implement handshake protocol
6. 2-peer example working
7. 3-peer mesh example working

---

## 📚 Reference Implementation

**Primary Reference**: TypeScript SDK
- `bindings/typescript/src/index.ts` - Core envelope
- `bindings/typescript/src/transports/websocket-transport.ts` - WebSocket + Peer
- `bindings/typescript/src/transports/streamable-http.ts` - HTTP transport
- `bindings/typescript/test/e2e.test.ts` - E2E patterns

**Secondary Reference**: Rust Roadmap
- `bindings/rust/IMPLEMENTATION_ROADMAP.md` - Architecture patterns

---

## 🔍 Go-Specific Considerations

### Concurrency Patterns
- Use **goroutines** for peer connections (one per connection)
- Use **channels** for message passing between goroutines
- Use **sync.Map** for concurrent peer storage
- Use **context.Context** for cancellation propagation

### Error Handling
- Return explicit errors (no exceptions)
- Use sentinel errors for common cases (`ErrNotConnected`, etc.)
- Wrap errors with context using `fmt.Errorf("context: %w", err)`

### Performance
- Minimize allocations in hot paths
- Reuse buffers where possible
- Use `sync.Pool` for frequently allocated objects
- Profile with `pprof` to identify bottlenecks

### Testing
- Use `testing` package (standard library)
- Use `testify/assert` for assertions
- Write table-driven tests
- Use `t.Parallel()` for concurrent tests
- Use benchmarks (`go test -bench=.`)

---

## ⚠️ Critical Decisions

### 1. Matrix Operations: CGo vs Pure Go?
**Recommendation**: Start with **Pure Go**, optionally add CGo later

**Pros of Pure Go**:
- Simpler deployment (no C++ dependencies)
- Cross-platform compatibility
- Easier debugging

**Pros of CGo**:
- Better performance (SIMD acceleration)
- Code reuse from C++ core

### 2. Event System: Channels vs Callbacks?
**Recommendation**: **Channels** (more idiomatic Go)

```go
// Option A: Channels (recommended)
events := peer.Subscribe()
for event := range events {
    // Handle event
}

// Option B: Callbacks (TypeScript-like)
peer.On("message", func(e *Event) {
    // Handle event
})
```

### 3. JSON vs Binary Serialization?
**Recommendation**: Start with **JSON**, add binary later

- JSON is easier to debug
- JSON is compatible with TypeScript
- Can add Protobuf/MessagePack later for performance

---

## 📞 Next Steps

1. ✅ Review this roadmap
2. ⬜ Setup Go project structure
3. ⬜ Implement Phase 1 (Foundation Layer)
4. ⬜ Implement Phase 2 (WebSocket Transport)
5. ⬜ Implement Phase 3 (Multiplexed Peer)
6. ⬜ Add comprehensive tests
7. ⬜ Write documentation and examples
8. ⬜ Performance benchmarking
9. ⬜ Production readiness review

---

## 🎓 Resources

### Go Learning Resources
- [Effective Go](https://go.dev/doc/effective_go)
- [Go by Example](https://gobyexample.com/)
- [Gorilla WebSocket Tutorial](https://github.com/gorilla/websocket)
- [Go Concurrency Patterns](https://go.dev/blog/pipelines)

### UMICP Resources
- TypeScript Implementation (primary reference)
- Rust Roadmap (architecture patterns)
- BIP-05 Specification
- C++ Core Implementation

---

**Total Estimated Effort**: ~40-50 days for full production parity with TypeScript SDK

**Minimum Viable Product**: ~20-25 days for core functionality (Envelope + WebSocket + Peer)

**Maintainer**: HiveLLM AI Collaborative Team
**Status**: 🚧 Planning Complete, Implementation Pending
**Last Updated**: 2025-10-10

