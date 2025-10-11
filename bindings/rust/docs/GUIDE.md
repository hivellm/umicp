# UMICP Rust Bindings - Technical Guide

> **📝 TECHNICAL GUIDE**

**Version**: 0.1.2  
**Last Updated**: October 11, 2025

---

## 🚀 Quick Start

### Installation

```toml
[dependencies]
umicp-core = "0.1.2"
tokio = { version = "1.42", features = ["full"] }
```

### Basic Usage

```rust
use umicp_core::{Envelope, OperationType};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Create envelope
    let envelope = Envelope::builder()
        .from("client-001")
        .to("server-001")
        .operation(OperationType::Data)
        .payload(b"Hello!".to_vec())
        .build()?;
    
    // Serialize
    let json = serde_json::to_string(&envelope)?;
    
    Ok(())
}
```

---

## 📝 Core API

### Envelope
```rust
use umicp_core::{Envelope, OperationType, PayloadType};

let envelope = Envelope::builder()
    .from("client")
    .to("server")
    .operation(OperationType::Data)
    .payload(b"data".to_vec())
    .payload_type(PayloadType::Binary)
    .build()?;
```

### Matrix
```rust
use umicp_core::Matrix;

let a = vec![1.0, 2.0, 3.0];
let b = vec![4.0, 5.0, 6.0];

let dot = Matrix::dot_product(&a, &b)?;
let cos = Matrix::cosine_similarity(&a, &b)?;
let norm = Matrix::normalize(&a)?;
```

---

## 🌐 WebSocket Transport

### Client
```rust
use umicp_core::transport::WebSocketClient;

let client = WebSocketClient::new("ws://localhost:8080").await?;
client.send(&envelope).await?;
client.close().await?;
```

### Server
```rust
use umicp_core::transport::WebSocketServer;

let server = WebSocketServer::new("127.0.0.1:8080").await?;
server.broadcast(&envelope).await?;
```

---

## 🔥 HTTP/2 Transport

### Client
```rust
use umicp_core::transport::HttpClient;

let client = HttpClient::new("http://localhost:8080");
let response = client.post("/api", &envelope).await?;
```

### Server
```rust
use umicp_core::transport::HttpServer;

let server = HttpServer::new("127.0.0.1:8080").await?;
server.start().await?;
```

---

## ⚡ Advanced Features

### Service Discovery
```rust
use umicp_core::ServiceDiscovery;

let discovery = ServiceDiscovery::new();
discovery.register("my-service", "ws://localhost:8080")?;
let services = discovery.find_by_name("my-service")?;
```

### Connection Pooling
```rust
use umicp_core::ConnectionPool;

let pool = ConnectionPool::new(2, 10);
let conn = pool.acquire().await?;
conn.send(&envelope).await?;
pool.release(conn).await?;
```

---

## 💡 Best Practices

### Error Handling
```rust
use umicp_core::error::Result;

fn send_message() -> Result<()> {
    // Use ? operator
    let envelope = Envelope::builder().build()?;
    Ok(())
}
```

### Async/Await
```rust
#[tokio::main]
async fn main() -> Result<()> {
    client.connect().await?;
    Ok(())
}
```

---

## 📚 Resources

- [README.md](./README.md)
- [STATUS.md](./STATUS.md)
- [REVIEWS.md](./REVIEWS.md)
- [Examples](../examples/)

---

*Last Updated: October 11, 2025*

