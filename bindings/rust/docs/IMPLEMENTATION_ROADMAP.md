# UMICP Rust Bindings - Implementation Roadmap

## 📊 Current Status

**Foundation Layer**: ✅ **100% Complete**
- ✅ Envelope system with builder pattern
- ✅ Matrix operations with parallel processing
- ✅ Type definitions (OperationType, PayloadType, EncodingType)
- ✅ Error handling
- ✅ Utilities (UUID, hashing, timestamps)

**Transport Layer**: ❌ **0% Implemented** (Only placeholders)
- ❌ WebSocket Client
- ❌ WebSocket Server
- ❌ Multiplexed Peer Architecture
- ❌ Streaming HTTP Transport

**Overall Gap**: **~75-80%** of TypeScript SDK functionality missing

---

## 🎯 Implementation Checklist

### Phase 1: Basic WebSocket Transport (CRITICAL - ~3-5 days)

**Goal**: Enable basic network communication between Rust peers

#### Dependencies to Add
```toml
[dependencies]
tokio = { version = "1.35", features = ["full"] }
tokio-tungstenite = "0.21"
tungstenite = "0.21"
futures = "0.3"
async-trait = "0.1"
```

#### Files to Create/Modify
- [ ] `src/transport/mod.rs` - Transport module organization
- [ ] `src/transport/websocket_client.rs` - WebSocket client implementation
- [ ] `src/transport/websocket_server.rs` - WebSocket server implementation
- [ ] `src/transport/connection.rs` - Connection state management

#### WebSocket Client Implementation
- [ ] **Struct**: `WebSocketClient`
  ```rust
  pub struct WebSocketClient {
      url: String,
      ws_stream: Option<WebSocketStream<MaybeTlsStream<TcpStream>>>,
      message_tx: mpsc::UnboundedSender<Envelope>,
      state: Arc<RwLock<ClientState>>,
  }
  ```

- [ ] **Methods**:
  - [ ] `async fn new(url: &str) -> Result<Self>`
  - [ ] `async fn connect(&mut self) -> Result<()>`
  - [ ] `async fn disconnect(&mut self) -> Result<()>`
  - [ ] `async fn send(&self, envelope: Envelope) -> Result<()>`
  - [ ] `fn is_connected(&self) -> bool`
  - [ ] `fn get_stats(&self) -> TransportStats`
  - [ ] `async fn set_message_handler<F>(&mut self, handler: F)` where F: Fn(Envelope) + Send + Sync

- [ ] **Features**:
  - [ ] Auto-reconnection with exponential backoff
  - [ ] Heartbeat/ping handling (30s interval)
  - [ ] Message queue for offline buffering
  - [ ] Connection timeout (10s default)
  - [ ] Graceful shutdown

#### WebSocket Server Implementation
- [ ] **Struct**: `WebSocketServer`
  ```rust
  pub struct WebSocketServer {
      addr: SocketAddr,
      clients: Arc<RwLock<HashMap<String, ClientConnection>>>,
      listener: Option<TcpListener>,
      state: Arc<RwLock<ServerState>>,
  }
  ```

- [ ] **Methods**:
  - [ ] `async fn new(addr: &str) -> Result<Self>`
  - [ ] `async fn start(&mut self) -> Result<()>`
  - [ ] `async fn stop(&mut self) -> Result<()>`
  - [ ] `async fn send(&self, envelope: Envelope, client_id: &str) -> Result<()>`
  - [ ] `async fn broadcast(&self, envelope: Envelope) -> Result<usize>`
  - [ ] `fn get_clients(&self) -> Vec<ClientConnection>`
  - [ ] `fn get_stats(&self) -> TransportStats`
  - [ ] `async fn set_message_handler<F>(&mut self, handler: F)` where F: Fn(Envelope, String) + Send + Sync
  - [ ] `async fn set_connection_handler<F>(&mut self, handler: F)`

- [ ] **Features**:
  - [ ] Accept multiple client connections
  - [ ] Per-client message queue
  - [ ] Connection tracking (HashMap<String, ClientConnection>)
  - [ ] Compression support (per-message deflate)
  - [ ] Max payload size limit (100MB default)
  - [ ] Ping/pong with configurable interval

#### Testing
- [ ] `tests/websocket_client_tests.rs` - Client unit tests
- [ ] `tests/websocket_server_tests.rs` - Server unit tests
- [ ] `tests/websocket_integration_tests.rs` - Client-server integration
- [ ] Performance benchmarks

---

### Phase 2: Multiplexed Peer Architecture (CRITICAL - ~5-7 days)

**Goal**: Implement TypeScript `UMICPWebSocketPeer` equivalent - each peer can be server AND multiple clients

#### Files to Create
- [ ] `src/peer/mod.rs` - Peer module
- [ ] `src/peer/websocket_peer.rs` - Main multiplexed peer implementation
- [ ] `src/peer/connection.rs` - PeerConnection and PeerInfo structs
- [ ] `src/peer/handshake.rs` - Auto-handshake protocol logic

#### Core Structs
- [ ] **PeerConnection**:
  ```rust
  pub struct PeerConnection {
      pub id: String,
      pub peer_type: PeerType, // Incoming or Outgoing
      pub url: Option<String>,
      pub client: WebSocketClient, // or connection handle
      pub metadata: HashMap<String, String>,
      pub connected_at: DateTime<Utc>,
      pub handshake_complete: bool,
      pub peer_info: Option<PeerInfo>,
  }
  
  pub enum PeerType {
      Incoming,
      Outgoing,
  }
  ```

- [ ] **PeerInfo**:
  ```rust
  pub struct PeerInfo {
      pub peer_id: String,
      pub metadata: HashMap<String, String>,
      pub capabilities: HashMap<String, String>,
      pub handshake_completed_at: DateTime<Utc>,
  }
  ```

- [ ] **WebSocketPeer**:
  ```rust
  pub struct WebSocketPeer {
      peer_id: String,
      server: Option<WebSocketServer>,
      peers: Arc<RwLock<HashMap<String, PeerConnection>>>,
      metadata: HashMap<String, String>,
      options: PeerOptions,
      event_tx: broadcast::Sender<PeerEvent>,
  }
  
  pub struct PeerOptions {
      pub peer_id: String,
      pub auto_protocol: bool,
      pub handshake_timeout: Duration,
  }
  ```

#### Methods to Implement
- [ ] **Constructor & Initialization**:
  - [ ] `async fn new(options: PeerOptions) -> Result<Self>`
  - [ ] `async fn with_server(options: PeerOptions, server_addr: &str) -> Result<Self>`
  - [ ] `fn initialize_server(&mut self, addr: &str) -> Result<()>`

- [ ] **Connection Management**:
  - [ ] `async fn connect_to_peer(&mut self, url: &str, metadata: Option<HashMap<String, String>>) -> Result<String>`
  - [ ] `async fn disconnect_peer(&mut self, peer_id: &str) -> Result<bool>`
  - [ ] `async fn disconnect_peer_by_url(&mut self, url: &str) -> Result<bool>`

- [ ] **Message Sending**:
  - [ ] `async fn send_to_peer(&self, peer_id: &str, envelope: Envelope) -> Result<bool>`
  - [ ] `async fn send_to_peer_by_url(&self, url: &str, envelope: Envelope) -> Result<bool>`
  - [ ] `async fn broadcast(&self, envelope: Envelope, exclude_peer_id: Option<&str>) -> Result<usize>`
  - [ ] `async fn broadcast_to_type(&self, envelope: Envelope, peer_type: PeerType, exclude: Option<&str>) -> Result<usize>`
  - [ ] `async fn send_and_wait(&self, peer_id: &str, envelope: Envelope, timeout: Duration) -> Result<Envelope>`

- [ ] **Peer Management**:
  - [ ] `fn get_peers(&self) -> Vec<PeerConnection>`
  - [ ] `fn get_peers_by_type(&self, peer_type: PeerType) -> Vec<PeerConnection>`
  - [ ] `fn get_peer(&self, peer_id: &str) -> Option<PeerConnection>`
  - [ ] `fn find_peer_by_metadata(&self, key: &str, value: &str) -> Option<PeerConnection>`

- [ ] **Statistics & Control**:
  - [ ] `fn get_stats(&self) -> PeerStats`
  - [ ] `async fn shutdown(&mut self) -> Result<()>`

#### Auto-Handshake Protocol
- [ ] **Send HELLO on connection**:
  - [ ] Create CONTROL envelope with type='hello'
  - [ ] Include peer_id, version, metadata in capabilities
  - [ ] Start handshake timeout timer

- [ ] **Handle HELLO messages**:
  - [ ] Extract peer info from capabilities
  - [ ] Send ACK response
  - [ ] Store peer info
  - [ ] Clear handshake timeout
  - [ ] Emit peer:ready event

- [ ] **Handle ACK messages**:
  - [ ] Extract peer info
  - [ ] Mark handshake as complete
  - [ ] Emit peer:ready event

- [ ] **Handshake Timeout**:
  - [ ] Default: 10 seconds
  - [ ] Emit timeout event
  - [ ] Optionally disconnect peer

#### Testing
- [ ] `tests/multiplexed_peer_tests.rs` - Peer unit tests
- [ ] `tests/peer_handshake_tests.rs` - Handshake protocol tests
- [ ] `tests/peer_integration_tests.rs` - Multi-peer integration
- [ ] `tests/peer_topologies_tests.rs` - Test different network topologies

---

### Phase 3: Event System (IMPORTANT - ~2-3 days)

**Goal**: Ergonomic API with event handling similar to TypeScript EventEmitter

#### Options for Implementation

**Option A: Tokio Channels** (Recommended)
```rust
use tokio::sync::broadcast;

pub enum PeerEvent {
    Message(Envelope, PeerConnection),
    Data(Envelope, PeerConnection),
    PeerConnect(PeerConnection),
    PeerDisconnect(PeerConnection),
    PeerReady(PeerConnection, PeerInfo),
    HandshakeTimeout(PeerConnection),
    Error(String, Option<PeerConnection>),
    ServerReady,
    Connected(String, String),
}

impl WebSocketPeer {
    pub fn subscribe(&self) -> broadcast::Receiver<PeerEvent> {
        self.event_tx.subscribe()
    }
}
```

**Option B: Callback Registry**
```rust
use std::sync::Arc;

type EventCallback = Arc<dyn Fn(PeerEvent) + Send + Sync>;

pub struct EventRegistry {
    handlers: HashMap<EventType, Vec<EventCallback>>,
}

impl WebSocketPeer {
    pub fn on<F>(&mut self, event_type: EventType, handler: F) 
    where F: Fn(PeerEvent) + Send + Sync + 'static
    {
        // Register handler
    }
}
```

**Option C: Trait-based** (Most Rusty)
```rust
#[async_trait]
pub trait PeerEventHandler: Send + Sync {
    async fn on_message(&self, envelope: Envelope, peer: PeerConnection) {}
    async fn on_peer_connect(&self, peer: PeerConnection) {}
    async fn on_peer_disconnect(&self, peer: PeerConnection) {}
    async fn on_error(&self, error: String, peer: Option<PeerConnection>) {}
}
```

#### Implementation Tasks
- [ ] Choose event system approach (recommend: Tokio channels + trait)
- [ ] Implement `PeerEvent` enum
- [ ] Implement event subscription mechanism
- [ ] Implement event emission in all relevant methods
- [ ] Add examples for each event type
- [ ] Test event propagation

---

### Phase 4: Streaming HTTP Transport (MEDIUM - ~3-4 days)

**Goal**: HTTP-based transport as alternative to WebSocket

#### Dependencies to Add
```toml
[dependencies]
axum = "0.7"
hyper = "1.0"
tower = "0.4"
reqwest = { version = "0.11", features = ["json", "stream"] }
```

#### Files to Create
- [ ] `src/transport/http_server.rs` - HTTP streaming server
- [ ] `src/transport/http_client.rs` - HTTP client
- [ ] `src/transport/http_peer.rs` - Multiplexed HTTP peer

#### StreamableHttpServer
- [ ] **Struct**: `StreamableHttpServer`
- [ ] **Framework**: Use `axum` for HTTP server
- [ ] **Methods**:
  - [ ] `async fn new(addr: &str) -> Result<Self>`
  - [ ] `async fn start(&mut self) -> Result<()>`
  - [ ] `async fn handle_request(envelope: Envelope) -> Response`
  - [ ] Streaming response support
  - [ ] CORS handling

#### StreamableHttpClient
- [ ] **Struct**: `StreamableHttpClient`
- [ ] **Framework**: Use `reqwest` for HTTP client
- [ ] **Methods**:
  - [ ] `async fn new(base_url: &str) -> Result<Self>`
  - [ ] `async fn send(&self, envelope: Envelope) -> Result<Envelope>`
  - [ ] `async fn send_streaming(&self, envelope: Envelope) -> Result<Stream>`
  - [ ] Timeout handling
  - [ ] Custom headers support

#### StreamableHttpPeer
- [ ] Multiplexed HTTP peer (similar to WebSocketPeer)
- [ ] Server + multiple client architecture
- [ ] Event-driven API

---

### Phase 5: Advanced Features (HIGH - ~2-3 days)

#### Auto-Reconnection
- [ ] Exponential backoff algorithm
- [ ] Configurable max retry attempts (default: 5)
- [ ] Configurable retry delay (default: 1s, max: 30s)
- [ ] Connection state tracking
- [ ] Reconnection events

#### Compression Support
- [ ] Per-message deflate for WebSocket
- [ ] LZ4 compression (optional)
- [ ] Zstandard compression (optional)
- [ ] Configurable compression level
- [ ] Automatic compression for large messages (>1KB)

#### Message Queuing
- [ ] Offline message queue when disconnected
- [ ] Configurable queue size (default: 1000 messages)
- [ ] Queue persistence (optional)
- [ ] Automatic flush on reconnection
- [ ] Queue overflow handling

#### Statistics & Monitoring
- [ ] **TransportStats enhancement**:
  - [ ] Real-time message counting
  - [ ] Bytes sent/received tracking
  - [ ] Average latency calculation
  - [ ] Connection uptime
  - [ ] Error rate tracking
  - [ ] Peak throughput metrics

---

### Phase 6: Testing Suite (HIGH - ~4-5 days)

#### Unit Tests
- [ ] `tests/envelope_tests.rs` - Envelope operations
- [ ] `tests/matrix_tests.rs` - Matrix operations
- [ ] `tests/transport_client_tests.rs` - WebSocket client
- [ ] `tests/transport_server_tests.rs` - WebSocket server
- [ ] `tests/peer_tests.rs` - Multiplexed peer

#### Integration Tests
- [ ] `tests/client_server_integration.rs` - Client-server communication
- [ ] `tests/peer_to_peer_integration.rs` - Peer-to-peer scenarios
- [ ] `tests/handshake_protocol_tests.rs` - Auto-handshake validation
- [ ] `tests/reconnection_tests.rs` - Auto-reconnection scenarios

#### E2E Tests
- [ ] `tests/e2e/two_peer_network.rs` - Simple 2-peer network
- [ ] `tests/e2e/three_peer_ring.rs` - Ring topology (A→B→C→A)
- [ ] `tests/e2e/hub_and_spoke.rs` - Hub-and-spoke topology
- [ ] `tests/e2e/full_mesh.rs` - Full mesh network

#### Performance Tests
- [ ] `tests/performance/throughput_benchmark.rs` - Message throughput
- [ ] `tests/performance/latency_benchmark.rs` - End-to-end latency
- [ ] `tests/performance/concurrent_connections.rs` - 100+ peers
- [ ] `tests/performance/large_payload_tests.rs` - Large message handling

#### Stress Tests
- [ ] `tests/stress/memory_leak_tests.rs` - Long-running stability
- [ ] `tests/stress/connection_churn.rs` - Rapid connect/disconnect
- [ ] `tests/stress/message_flood.rs` - High message volume
- [ ] `tests/stress/error_recovery.rs` - Error handling under load

**Coverage Target**: 90%+ overall, 95%+ for transport layer

---

### Phase 7: Examples & Documentation (MEDIUM - ~2-3 days)

#### Examples to Create
- [ ] `examples/simple_peer_to_peer.rs` - Basic 2-peer communication
- [ ] `examples/multiplexed_network.rs` - 3+ peers with multiplexed architecture
- [ ] `examples/broadcast_example.rs` - Hub broadcasting to multiple peers
- [ ] `examples/request_response.rs` - Request-response pattern with sendAndWait
- [ ] `examples/pipeline_processing.rs` - Sequential pipeline topology
- [ ] `examples/mesh_network.rs` - Full mesh topology
- [ ] `examples/http_transport.rs` - HTTP-based communication
- [ ] `examples/federated_learning.rs` - Federated learning use case (update existing)

#### Documentation
- [ ] **README.md Enhancement**:
  - [ ] Quick start with multiplexed peer
  - [ ] Feature comparison with TypeScript
  - [ ] Performance benchmarks
  - [ ] Topology examples

- [ ] **API Documentation**:
  - [ ] Rustdoc comments for all public APIs
  - [ ] Usage examples in doc comments
  - [ ] Link to TypeScript docs for reference

- [ ] **Guides**:
  - [ ] `docs/MULTIPLEXED_PEER.md` - Multiplexed architecture guide
  - [ ] `docs/WEBSOCKET_TRANSPORT.md` - WebSocket transport guide
  - [ ] `docs/EVENT_SYSTEM.md` - Event handling patterns
  - [ ] `docs/MIGRATION_FROM_TYPESCRIPT.md` - Migration guide

---

### Phase 8: Production Readiness (MEDIUM - ~2-3 days)

#### Error Handling Enhancement
- [ ] Detailed error messages with context
- [ ] Error recovery strategies
- [ ] Graceful degradation
- [ ] Error rate limiting
- [ ] Structured error logging

#### Logging
- [ ] Integration with `tracing` crate
- [ ] Structured logging for all operations
- [ ] Configurable log levels
- [ ] Performance-sensitive logging (avoid in hot paths)

#### Configuration
- [ ] Configuration file support (YAML/TOML)
- [ ] Environment variable support
- [ ] Builder pattern for all options
- [ ] Validation for all config values
- [ ] Default values clearly documented

#### Security
- [ ] TLS/SSL support for WebSocket (wss://)
- [ ] Certificate validation
- [ ] Mutual TLS (mTLS) option
- [ ] Peer authentication (pre-shared keys or JWT)
- [ ] Rate limiting per peer
- [ ] Message size limits

---

## 📋 Detailed Implementation Checklist

### Core Transport Implementation

#### WebSocket Client
- [ ] Connection state machine (Disconnected → Connecting → Connected → Disconnected)
- [ ] URL parsing and validation
- [ ] TLS support (ws:// and wss://)
- [ ] Message serialization/deserialization
- [ ] Send queue with backpressure
- [ ] Receive loop with async stream
- [ ] Ping/pong handling
- [ ] Close frame handling
- [ ] Error propagation
- [ ] Metrics collection

#### WebSocket Server
- [ ] TCP listener on specified address
- [ ] WebSocket upgrade handshake
- [ ] Client connection tracking
- [ ] Per-client send/receive loops
- [ ] Broadcast to all clients
- [ ] Broadcast to filtered clients
- [ ] Client disconnection handling
- [ ] Server shutdown (graceful close)
- [ ] Connection limits (max clients)
- [ ] Per-client rate limiting

#### Multiplexed Peer
- [ ] Unified peer map (HashMap<String, PeerConnection>)
- [ ] Server component initialization (optional)
- [ ] Multiple outgoing connections support
- [ ] Incoming connection acceptance
- [ ] Unified message handler for all connections
- [ ] Auto-handshake on new connection
  - [ ] Send HELLO message
  - [ ] Wait for ACK response
  - [ ] Extract peer info from ACK
  - [ ] Store peer info
  - [ ] Timeout handling (10s default)
- [ ] Message routing by peer ID
- [ ] Message routing by URL
- [ ] Broadcast to all peers
- [ ] Broadcast to incoming peers only
- [ ] Broadcast to outgoing peers only
- [ ] Peer discovery and metadata
- [ ] Connection lifecycle management
- [ ] Graceful shutdown of all connections

#### Event System
- [ ] Event enum definition (Message, Data, PeerConnect, etc.)
- [ ] Event emission mechanism
- [ ] Event subscription (broadcast channel)
- [ ] Multiple subscribers support
- [ ] Event filtering by type
- [ ] Async event handlers
- [ ] Event handler error handling
- [ ] Unsubscribe mechanism

---

## 🔧 Technical Implementation Details

### Message Flow (Multiplexed Peer)

```rust
// Incoming connection flow:
1. Server accepts connection
2. Create PeerConnection { type: Incoming, ... }
3. Add to peers map
4. Emit PeerConnect event
5. Send HELLO message (if autoProtocol)
6. Wait for ACK
7. Extract peer info from ACK
8. Mark handshake complete
9. Emit PeerReady event

// Outgoing connection flow:
1. connectToPeer(url) called
2. Create WebSocketClient
3. Connect to URL
4. Create PeerConnection { type: Outgoing, url, ... }
5. Add to peers map
6. Emit PeerConnect event
7. Send HELLO message (if autoProtocol)
8. Wait for ACK response
9. Extract peer info
10. Mark handshake complete
11. Emit PeerReady and Connected events

// Message handling:
1. Receive envelope
2. Find peer in map (by client ID or connection)
3. Emit Message event (raw)
4. Check operation type:
   - CONTROL → handleControlMessage (protocol logic)
   - ACK → handleAckMessage (protocol logic)
   - DATA → Emit Data event (application logic)
   - ERROR → Emit Error event
```

### Topology Support

#### Full Mesh (All-to-All)
```rust
// Each peer connects to all others
let peer_a = WebSocketPeer::with_server(opts, "127.0.0.1:20081").await?;
peer_a.connect_to_peer("ws://localhost:20082").await?;
peer_a.connect_to_peer("ws://localhost:20083").await?;
// Each peer does the same
```

#### Hub-and-Spoke
```rust
// Hub accepts connections, spokes connect
let hub = WebSocketPeer::with_server(opts, "127.0.0.1:20080").await?;
// Spokes connect to hub
let spoke = WebSocketPeer::new(opts).await?;
spoke.connect_to_peer("ws://localhost:20080").await?;
```

#### Ring Topology
```rust
// A → B → C → A
peer_a.connect_to_peer("ws://localhost:20082").await?; // A → B
peer_b.connect_to_peer("ws://localhost:20083").await?; // B → C
peer_c.connect_to_peer("ws://localhost:20081").await?; // C → A
```

---

## 🎯 Priority Matrix

| Feature | Priority | Effort | Blockers | Dependencies |
|---------|----------|--------|----------|--------------|
| WebSocket Client | 🔴 CRITICAL | Medium (3d) | None | tokio-tungstenite |
| WebSocket Server | 🔴 CRITICAL | Medium (3d) | None | tokio-tungstenite |
| Multiplexed Peer | 🔴 CRITICAL | High (5d) | WebSocket Client/Server | Phase 1 |
| Auto-Handshake | 🔴 CRITICAL | Low (2d) | Multiplexed Peer | Phase 2 |
| Event System | 🟡 HIGH | Medium (3d) | None | None |
| Broadcast Methods | 🟡 HIGH | Low (1d) | Multiplexed Peer | Phase 2 |
| Peer Management | 🟡 HIGH | Low (2d) | Multiplexed Peer | Phase 2 |
| send_and_wait | 🟡 HIGH | Medium (2d) | Event System | Phase 3 |
| Streaming HTTP | 🟢 MEDIUM | Medium (3d) | None | axum |
| Auto-Reconnection | 🟢 MEDIUM | Low (1d) | WebSocket Client | Phase 1 |
| Testing Suite | 🟡 HIGH | High (4d) | All above | Phases 1-4 |
| Documentation | 🟢 MEDIUM | Medium (2d) | All above | Phases 1-4 |

---

## 📈 Estimated Timeline

### Fast Track (Focus on Core Features)
- **Week 1**: WebSocket Client + Server (Phase 1)
- **Week 2**: Multiplexed Peer + Auto-Handshake (Phase 2)
- **Week 3**: Event System + Testing (Phases 3 & 6)
- **Week 4**: Examples + Documentation (Phase 7)
- **Total**: ~4 weeks to feature parity with TypeScript core

### Complete Implementation
- **Weeks 1-2**: Phases 1-2 (Transport + Peer)
- **Week 3**: Phase 3 (Event System)
- **Week 4**: Phase 4 (HTTP Transport)
- **Week 5**: Phase 5 (Advanced Features)
- **Week 6**: Phases 6-7 (Testing + Documentation)
- **Week 7**: Phase 8 (Production Readiness)
- **Total**: ~7 weeks to full production readiness

---

## 🚀 Quick Wins (Start Here)

### Week 1 - Make It Work
1. ✅ Install tokio + tokio-tungstenite
2. ✅ Implement basic WebSocket client (connect, send, receive)
3. ✅ Implement basic WebSocket server (accept, send, receive)
4. ✅ Simple client-server example working
5. ✅ Basic tests passing

### Week 2 - Add Multiplexing
1. ✅ Create PeerConnection and PeerInfo structs
2. ✅ Implement WebSocketPeer with server component
3. ✅ Implement connect_to_peer method
4. ✅ Implement send_to_peer and broadcast
5. ✅ Auto-handshake protocol (HELLO → ACK)
6. ✅ 3-peer ring example working

### Week 3 - Polish & Test
1. ✅ Add event system
2. ✅ Add peer management methods
3. ✅ Add comprehensive tests
4. ✅ Add examples for common topologies
5. ✅ Update documentation

---

## 📚 Reference Implementation

**Primary Reference**: TypeScript SDK
- `bindings/typescript/src/transports/websocket-transport.ts` (1257 lines)
  - Lines 142-341: `UMICPWebSocketClient`
  - Lines 343-543: `UMICPWebSocketServer`
  - Lines 695-1257: `UMICPWebSocketPeer` ⭐ **KEY REFERENCE**

**Key Files to Study**:
- `bindings/typescript/src/transports/websocket-transport.ts` - Multiplexed peer
- `bindings/typescript/src/transports/streamable-http.ts` - HTTP transport
- `bindings/typescript/test/e2e.test.ts` - E2E test patterns
- `bindings/typescript/test/websocket-transport.test.ts` - Transport tests

---

## 🎓 Recommended Approach

### Step-by-Step Implementation

1. **Start Simple**:
   - Implement basic WebSocket client with tokio-tungstenite
   - Get connection working
   - Send/receive envelopes
   - Basic error handling

2. **Add Server**:
   - Implement WebSocket server
   - Accept connections
   - Track clients
   - Broadcast support

3. **Build Multiplexed Peer**:
   - Combine server + multiple clients
   - Unified peer map
   - Study TypeScript implementation closely
   - Implement method by method

4. **Add Protocol**:
   - HELLO message on connect
   - ACK response handling
   - Peer info extraction
   - Handshake timeout

5. **Add Events**:
   - Choose event system (channels recommended)
   - Emit events in all relevant places
   - Add subscription mechanism

6. **Test Everything**:
   - Write tests for each component
   - E2E tests for topologies
   - Performance benchmarks

7. **Document**:
   - API documentation
   - Usage examples
   - Migration guide from TypeScript

---

## 🔍 Code Comparison Examples

### TypeScript UMICPWebSocketPeer
```typescript
// ~960 lines of implementation
export class UMICPWebSocketPeer extends EventEmitter {
  private peers: Map<string, PeerConnection> = new Map();
  private server: UMICPWebSocketServer | null = null;
  
  async connectToPeer(url: string, metadata?: object): Promise<string | null> {
    // Creates client, connects, sends HELLO, waits for ACK
  }
  
  broadcast(envelope: Envelope, excludePeerId?: string): number {
    // Sends to all peers in map
  }
}
```

### Rust Target Implementation
```rust
// To be implemented (~800-1000 lines estimated)
pub struct WebSocketPeer {
    peer_id: String,
    peers: Arc<RwLock<HashMap<String, PeerConnection>>>,
    server: Option<WebSocketServer>,
    event_tx: broadcast::Sender<PeerEvent>,
}

impl WebSocketPeer {
    pub async fn connect_to_peer(&mut self, url: &str, metadata: Option<HashMap<String, String>>) -> Result<String> {
        // TODO: Implement
    }
    
    pub async fn broadcast(&self, envelope: Envelope, exclude_peer_id: Option<&str>) -> Result<usize> {
        // TODO: Implement
    }
}
```

---

## ⚠️ Critical Gaps Summary

**What's Missing (High Impact)**:
1. 🔴 **WebSocket Transport** - Without this, Rust SDK cannot communicate over network
2. 🔴 **Multiplexed Peer** - Core feature of UMICP 2.0, enables true P2P
3. 🔴 **Auto-Handshake** - Peer discovery and metadata exchange
4. 🔴 **Event System** - Ergonomic API for users
5. 🔴 **Testing** - Only 1 test file, no transport tests

**What's Working Well**:
- ✅ Envelope system is complete and matches TypeScript
- ✅ Matrix operations are complete with parallel processing
- ✅ Type system is well-designed
- ✅ Error handling is comprehensive
- ✅ Code quality is good

**Bottom Line**:
The Rust SDK has a **solid foundation** but is missing the **entire networking layer**. It's like having a great car engine (Envelope + Matrix) but no transmission or wheels (Transport + Peer). To reach parity with TypeScript, the focus should be:

1. **Week 1**: WebSocket transport (Client + Server)
2. **Week 2**: Multiplexed Peer architecture
3. **Week 3**: Event system + Testing
4. **Week 4**: Documentation + Examples

After these 4 weeks, Rust SDK would be **functionally equivalent** to TypeScript SDK.

---

## 📞 Next Steps

1. Review this roadmap
2. Prioritize phases based on project needs
3. Set up Rust project with required dependencies
4. Start with Phase 1 (WebSocket Transport)
5. Use TypeScript implementation as reference
6. Implement incrementally with tests at each step

**Note**: This roadmap is based on detailed analysis comparing TypeScript SDK (100% functional) with Rust SDK (foundation only). Total estimated effort: **~22-32 days** for full parity.

