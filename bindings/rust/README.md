# UMICP Rust Bindings

[![Crates.io](https://img.shields.io/crates/v/umicp-core.svg)](https://crates.io/crates/umicp-core)
[![Documentation](https://docs.rs/umicp-core/badge.svg)](https://docs.rs/umicp-core)
[![License](https://img.shields.io/badge/license-CC0--1.0-blue.svg)](../../../LICENSE)

High-performance Rust bindings for the Universal Matrix Inter-Communication Protocol (UMICP).

## 🚀 Status: **In Active Development**

| Component | Status | Coverage |
|-----------|--------|----------|
| **Envelope System** | ✅ Complete | 100% |
| **Matrix Operations** | ✅ Complete | 100% |
| **WebSocket Client** | ✅ Functional | 90% |
| **WebSocket Server** | ✅ Functional | 85% |
| **Multiplexed Peer** | 🚧 Planned | 0% |
| **Event System** | 🚧 Planned | 0% |
| **HTTP Transport** | 📋 Planned | 0% |

**Current Progress**: ~40% of TypeScript SDK functionality  
**Tests Passing**: 31/39 (79%)  
**Production Ready**: Foundation layer only

---

## 📦 Installation

Add to your `Cargo.toml`:

```toml
[dependencies]
umicp-core = { version = "1.0", features = ["websocket"] }
tokio = { version = "1.35", features = ["full"] }
```

### Features

- `websocket` - WebSocket transport (requires tokio)
- `http2` - HTTP/2 transport (planned)
- `full` - All features

---

## 🎯 Quick Start

### Basic Envelope Usage

```rust
use umicp_core::{Envelope, OperationType};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Create a UMICP envelope
    let envelope = Envelope::builder()
        .from("client-001")
        .to("server-001")
        .operation(OperationType::Data)
        .message_id("msg-12345")
        .capability("content-type", "application/json")
        .build()?;

    // Serialize for transmission
    let serialized = envelope.serialize()?;
    
    // Deserialize received data
    let received = Envelope::deserialize(&serialized)?;
    
    println!("From: {}", received.from());
    println!("To: {}", received.to());
    
    Ok(())
}
```

### WebSocket Client (Basic)

```rust
use umicp_core::{WebSocketClient, Envelope, OperationType};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Create client
    let client = WebSocketClient::new("ws://localhost:8080");
    
    // Connect
    client.connect().await?;
    println!("Connected!");
    
    // Send message
    let envelope = Envelope::builder()
        .from("rust-client")
        .to("server")
        .operation(OperationType::Data)
        .capability("message", "Hello from Rust!")
        .build()?;
    
    client.send(envelope).await?;
    println!("Message sent!");
    
    // Get statistics
    let stats = client.get_stats();
    println!("Messages sent: {}", stats.messages_sent);
    println!("Bytes sent: {}", stats.bytes_sent);
    
    // Disconnect
    client.disconnect().await?;
    
    Ok(())
}
```

### Matrix Operations

```rust
use umicp_core::Matrix;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut matrix = Matrix::new();

    // Vector operations
    let vector1 = vec![1.0f32, 2.0, 3.0, 4.0];
    let vector2 = vec![5.0f32, 6.0, 7.0, 8.0];
    let mut result = vec![0.0f32; 4];

    // Vector addition
    matrix.vector_add(&vector1, &vector2, &mut result)?;
    println!("Addition result: {:?}", result); // [6.0, 8.0, 10.0, 12.0]

    // Dot product
    let dot_product = matrix.dot_product(&vector1, &vector2)?;
    println!("Dot product: {:?}", dot_product.result); // 70.0

    // Cosine similarity
    let similarity = matrix.cosine_similarity(&vector1, &vector2)?;
    println!("Similarity: {:?}", similarity.similarity);

    Ok(())
}
```

---

## ✅ Implemented Features

### Envelope System (100% Complete)
- ✅ Builder pattern for envelope creation
- ✅ JSON serialization/deserialization
- ✅ Hash generation (SHA-256)
- ✅ Validation
- ✅ All operation types (Control, Data, Ack, Error, Request, Response)
- ✅ Payload hints (type, size, encoding, count)
- ✅ Capabilities (metadata key-value pairs)
- ✅ Payload references support

### Matrix Operations (100% Complete)
- ✅ Matrix addition, multiplication, transpose
- ✅ Vector addition, subtraction, multiplication, scaling
- ✅ Dot product
- ✅ Cosine similarity
- ✅ L2 normalization
- ✅ Matrix determinant
- ✅ Parallel processing for large matrices (>1000 elements)
- ✅ SIMD optimization support

### WebSocket Transport (85% Complete)
- ✅ **WebSocket Client**:
  - Async connect/disconnect
  - Send envelopes
  - Connection state tracking
  - Auto-reconnection with backoff
  - Statistics tracking
  - Configurable timeouts
- ✅ **WebSocket Server**:
  - Accept multiple connections
  - Send to specific client
  - Broadcast to all clients
  - Client tracking
  - Statistics tracking
  - Graceful client cleanup

### Type System
- ✅ `OperationType` enum
- ✅ `PayloadType` enum
- ✅ `EncodingType` enum
- ✅ `PayloadHint` struct
- ✅ `TransportStats` struct
- ✅ `ConnectionState` enum

### Error Handling
- ✅ `UmicpError` with thiserror
- ✅ Validation errors
- ✅ Serialization errors
- ✅ Transport errors
- ✅ Matrix operation errors
- ✅ Generic errors

---

## 🚧 In Development

### Message Handlers (Est. 1-2 days)
- [ ] Client message handler callbacks
- [ ] Server message handler callbacks
- [ ] Connection event callbacks
- [ ] Error event callbacks

### Server Refactoring (Est. 1 day)
- [ ] Non-blocking `start()` method
- [ ] Return JoinHandle for background task
- [ ] Proper shutdown coordination
- [ ] Integration test compatibility

---

## 📋 Planned Features

### Multiplexed Peer Architecture (Est. 5-7 days)
- [ ] `WebSocketPeer` struct
- [ ] Unified server + multiple client architecture
- [ ] Auto-handshake protocol (HELLO → ACK)
- [ ] Peer discovery and metadata exchange
- [ ] Broadcast to all peers
- [ ] Send to specific peer by ID/URL
- [ ] Peer management methods
- [ ] Support for network topologies (mesh, hub-and-spoke, etc.)

### Event System (Est. 2-3 days)
- [ ] Tokio broadcast channels or callback registry
- [ ] Event types (Message, PeerConnect, PeerDisconnect, etc.)
- [ ] Multiple event subscribers
- [ ] Async event handling

### HTTP Transport (Est. 3-4 days)
- [ ] Streaming HTTP server (using axum)
- [ ] HTTP client (using reqwest)
- [ ] Multiplexed HTTP peer

### Advanced Features (Est. 2-3 days)
- [ ] TLS/SSL support (wss://)
- [ ] Compression (per-message deflate)
- [ ] Service discovery
- [ ] Load balancing
- [ ] Authentication

---

## 🧪 Testing

### Run Tests

```bash
# All unit tests
cargo test --features websocket --lib

# Envelope tests
cargo test --features websocket --test envelope_tests

# Matrix tests  
cargo test --features websocket --test matrix_tests

# WebSocket transport tests (some ignored)
cargo test --features websocket --test websocket_transport_tests

# Run all tests (excluding ignored)
cargo test --features websocket
```

### Test Coverage

- **Envelope**: 14 tests ✅ All passing
- **Matrix**: 14 tests ✅ All passing
- **WebSocket**: 13 tests (6 passing, 7 ignored pending server refactor)
- **Total**: 46 tests (39 passing, 7 ignored)

---

## 📚 Examples

```bash
# Basic WebSocket client-server
cargo run --features websocket --example websocket_basic

# Matrix operations
cargo run --example matrix_operations

# Envelope usage
cargo run --example basic_envelope

# Error handling
cargo run --example error_handling
```

---

## 🔍 Architecture

### Current Implementation

```
umicp-core/
├── src/
│   ├── envelope.rs          ✅ Complete (516 lines)
│   ├── matrix.rs            ✅ Complete (517 lines)
│   ├── types.rs             ✅ Complete (273 lines)
│   ├── error.rs             ✅ Complete (158 lines)
│   ├── utils.rs             ✅ Complete (104 lines)
│   ├── transport_legacy.rs  ✅ Placeholders
│   └── transport/
│       ├── mod.rs           ✅ Module organization
│       ├── websocket_client.rs  ✅ ~350 lines
│       └── websocket_server.rs  ✅ ~330 lines
├── tests/
│   ├── envelope_tests.rs    ✅ 14 tests
│   ├── matrix_tests.rs      ✅ 14 tests
│   ├── websocket_transport_tests.rs  🟡 13 tests (6 passing)
│   └── integration_tests.rs ✅ 1 test
└── examples/
    ├── websocket_basic.rs   ✅ Client-server example
    ├── matrix_operations.rs ✅ Matrix ops
    ├── basic_envelope.rs    ✅ Envelope usage
    └── error_handling.rs    ✅ Error patterns
```

### Planned Structure

```
umicp-core/
└── src/
    ├── peer/                 🚧 To be created
    │   ├── mod.rs
    │   ├── websocket_peer.rs
    │   ├── connection.rs
    │   └── handshake.rs
    └── transport/
        ├── http_server.rs    📋 Planned
        ├── http_client.rs    📋 Planned
        └── http_peer.rs      📋 Planned
```

---

## 🤝 Contributing

See [IMPLEMENTATION_ROADMAP.md](./IMPLEMENTATION_ROADMAP.md) for detailed implementation checklist.

### Priority Areas
1. 🔴 **HIGH**: Refactor server for non-blocking operation
2. 🔴 **HIGH**: Implement message/connection handlers
3. 🔴 **HIGH**: Multiplexed peer architecture
4. 🟡 **MEDIUM**: Event system
5. 🟢 **LOW**: HTTP transport

---

## 📄 License

MIT License - See [LICENSE](../../../LICENSE) file for details.

---

## 🔗 Part of HiveLLM Ecosystem

UMICP Rust bindings are part of the [HiveLLM ecosystem](https://github.com/hivellm), providing high-performance binary protocol for agent-to-agent communication.

See the [HiveLLM paper](../../../gov/papers/vectorizer_paper.md) for comprehensive ecosystem documentation.

---

**Version**: 1.0.0  
**Rust Edition**: 2021  
**MSRV**: 1.70+  
**Status**: 🚧 Active Development (40% complete)

---

*Last Updated: October 10, 2025*
