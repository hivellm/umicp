# UMICP Rust Bindings

[![Crates.io](https://img.shields.io/crates/v/umicp-core.svg)](https://crates.io/crates/umicp-core)
[![Documentation](https://docs.rs/umicp-core/badge.svg)](https://docs.rs/umicp-core)
[![License: CC0-1.0](https://img.shields.io/badge/License-CC0_1.0-lightgrey.svg)](https://creativecommons.org/publicdomain/zero/1.0/)

High-performance Rust bindings for the Universal Matrix Inter-Communication Protocol (UMICP), providing a complete implementation of the protocol with async WebSocket transport, type-safe envelope handling, and SIMD-optimized matrix operations.

## 🚀 Features

- **🔗 Universal Communication**: Async WebSocket transport with automatic reconnection
- **📦 Type-Safe Envelopes**: Strongly-typed message serialization and validation with JSON canonical format
- **⚡ High Performance**: SIMD-optimized matrix operations with parallel processing support
- **🔄 Federated Learning**: Built-in support for ML model distribution and aggregation
- **🛡️ Security First**: Input validation, rate limiting, and error handling
- **📊 Real-time**: Low-latency communication for IoT and financial applications
- **🧪 Well Tested**: Comprehensive test suite with async testing support
- **📈 Parallel Processing**: Rayon-based parallel matrix operations for large datasets

## 📦 Installation

Add this to your `Cargo.toml`:

```toml
[dependencies]
umicp-core = "1.0"
```

### Feature Flags

- `websocket` (default): Enable WebSocket transport
- `http2`: Enable HTTP/2 transport (future use)
- `full`: Enable all transports

```toml
# Enable all features
umicp-core = { version = "1.0", features = ["full"] }

# Minimal installation (no transports)
umicp-core = { version = "1.0", default-features = false }
```

## 🏃 Quick Start

### Basic Envelope Usage

```rust,no_run
use umicp_core::{Envelope, OperationType};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Create a UMICP envelope
    let envelope = Envelope::builder()
        .from("client-001")
        .to("server-001")
        .operation(OperationType::Data)
        .message_id("msg-12345")
        .capability("content-type", "application/json")
        .capability("priority", "high")
        .build()?;

    // Serialize for transmission
    let serialized = envelope.serialize()?;

    // Deserialize received data
    let received: Envelope = Envelope::deserialize(&serialized)?;

    println!("From: {}", received.from());
    println!("Capabilities: {:?}", received.capabilities());

    Ok(())
}
```

### WebSocket Transport

```rust,no_run
use umicp_core::{WebSocketTransport, Envelope, OperationType};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Server setup
    let mut server = WebSocketTransport::new_server("127.0.0.1:8080").await?;

    // Message handling
    server.set_message_handler(|envelope, conn_id| async move {
        println!("Received: {:?}", envelope.capabilities());

        let response = Envelope::builder()
            .from("server")
            .to(envelope.from())
            .operation(OperationType::Ack)
            .message_id(format!("response-{}", uuid::Uuid::new_v4()))
            .build()?;

        server.send(response, &conn_id).await?;
        Ok(())
    });

    // Client setup
    let mut client = WebSocketTransport::new_client("ws://127.0.0.1:8080").await?;

    // Start communication
    tokio::spawn(async move { server.run().await });
    tokio::spawn(async move { client.run().await });

    // Send message
    let message = Envelope::builder()
        .from("client")
        .to("server")
        .operation(OperationType::Data)
        .message_id("hello-001")
        .capability("message", "Hello UMICP!")
        .build()?;

    client.send_to_server(message).await?;

    Ok(())
}
```

### Matrix Operations

```rust,no_run
use umicp_core::Matrix;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let matrix = Matrix::new();

    // Vector operations
    let vector1 = vec![1.0f32, 2.0, 3.0, 4.0];
    let vector2 = vec![5.0f32, 6.0, 7.0, 8.0];
    let mut result = vec![0.0f32; 4];

    // Vector addition
    matrix.vector_add(&vector1, &vector2, &mut result)?;
    println!("Addition result: {:?}", result); // [6.0, 8.0, 10.0, 12.0]

    // Dot product
    let dot_product = matrix.dot_product(&vector1, &vector2)?;
    println!("Dot product: {}", dot_product.result.unwrap()); // 70.0

    // Matrix multiplication
    let matrix_a = vec![1.0, 2.0, 3.0, 4.0]; // 2x2 matrix
    let matrix_b = vec![5.0, 6.0, 7.0, 8.0]; // 2x2 matrix
    let mut matrix_result = vec![0.0; 4];

    matrix.matrix_multiply(&matrix_a, &matrix_b, &mut matrix_result, 2, 2, 2)?;
    println!("Matrix multiplication: {:?}", matrix_result);

    Ok(())
}
```

## 🧪 Testing

Run the comprehensive test suite:

```bash
cargo test
```

### Test Coverage

The Rust binding provides comprehensive test coverage equivalent to TypeScript and C++ implementations:

- **Unit Tests**: 12 core functionality tests
- **Integration Tests**: 9 cross-component tests
- **Total Coverage**: 21 tests covering all major functionality

Run specific test categories:

```bash
# Unit tests (core functionality)
cargo test --lib

# Integration tests (cross-component)
cargo test --test integration_tests

# Run all tests with verbose output
cargo test -- --nocapture
```

### Coverage Areas

✅ **Envelope Operations**
- Creation and validation
- JSON serialization/deserialization
- Hash generation and integrity
- Builder pattern functionality
- Edge cases and error handling

✅ **Matrix Operations**
- Vector addition and operations
- Matrix multiplication and transpose
- Dot product and cosine similarity
- Normalization and determinant
- SIMD optimization validation

✅ **Security & Validation**
- Input sanitization
- UUID validation
- Memory safety checks
- Error handling robustness

✅ **Performance & Stress**
- Large data handling
- Concurrent operations
- Memory efficiency
- Edge case performance

### Test Results

```bash
running 12 tests
test result: ok. 12 passed; 0 failed; 0 ignored; 0 measured

running 9 tests
test result: ok. 9 passed; 0 failed; 0 ignored; 0 measured
```

### Continuous Integration

All tests pass successfully, ensuring:
- Memory safety (no Rust panics or undefined behavior)
- Thread safety for concurrent operations
- Data integrity across serialization cycles
- Performance consistency under load

## 📚 API Reference

### Core Types

#### `Envelope`
Message container with metadata and capabilities.

```rust
// Builder pattern
let envelope = Envelope::builder()
    .from("sender")
    .to("recipient")
    .operation(OperationType::Data)
    .capability("key", "value")
    .build()?;

// Serialization
let json = envelope.serialize()?;
let deserialized = Envelope::deserialize(&json)?;

// Validation
envelope.validate()?;

// Hash generation
let hash = envelope.hash()?;
```

#### `Matrix`
High-performance matrix operations.

```rust
let matrix = Matrix::new();

// Vector operations
matrix.vector_add(&a, &b, &mut result)?;
matrix.dot_product(&a, &b)?;

// Matrix operations
matrix.matrix_multiply(&a, &b, &mut result, m, n, p)?;
matrix.transpose(&input, &mut output, rows, cols)?;

// Similarity
matrix.cosine_similarity(&a, &b)?;
```

#### `WebSocketTransport`
Async WebSocket transport layer.

```rust
// Server
let mut server = WebSocketTransport::new_server("127.0.0.1:8080").await?;
server.set_message_handler(|envelope, conn_id| async move {
    // Handle message
    Ok(())
});
server.run().await?;

// Client
let mut client = WebSocketTransport::new_client("ws://127.0.0.1:8080").await?;
client.send_to_server(envelope).await?;
```

### Operation Types

```rust
pub enum OperationType {
    Control,   // Protocol control messages
    Data,      // Regular data messages
    Ack,       // Acknowledgments
    Error,     // Error messages
    Request,   // Request messages
    Response,  // Response messages
}
```

### Payload Types

```rust
pub enum PayloadType {
    Vector,   // Numeric arrays
    Text,     // Text data
    Metadata, // Metadata information
    Binary,   // Binary data
}
```

## 🎯 Use Cases

### Federated Learning

```rust
// Model weight distribution
let weights_envelope = Envelope::builder()
    .from("coordinator")
    .to("worker-001")
    .operation(OperationType::Data)
    .capability("model-version", "1.0")
    .capability("layer-count", "12")
    .capability("weights", &weights_json)
    .build()?;

transport.send(weights_envelope, &worker_id).await?;
```

### IoT Data Streaming

```rust
// Sensor data collection
let sensor_data = Envelope::builder()
    .from("sensor-temp-001")
    .to("data-collector")
    .operation(OperationType::Data)
    .capability("sensor-type", "temperature")
    .capability("value", &temperature.to_string())
    .capability("unit", "celsius")
    .capability("timestamp", &timestamp)
    .build()?;

transport.send(sensor_data, "").await?;
```

### Real-time Analytics

```rust
// Matrix-based similarity computation
let matrix = Matrix::new();
let similarity = matrix.cosine_similarity(&user_vector, &item_vector)?;

// Send recommendation
let recommendation = Envelope::builder()
    .from("recommendation-engine")
    .to("user-interface")
    .operation(OperationType::Data)
    .capability("similarity", &similarity.similarity.unwrap().to_string())
    .capability("item-id", &item_id)
    .build()?;
```

## 📈 Performance

### Benchmarks

Based on comprehensive testing:

- **Envelope Creation**: ~5μs per envelope (validated in stress tests)
- **JSON Serialization**: ~15μs for complex envelopes
- **Validation**: Sub-microsecond field validation
- **Hash Generation**: SHA-256 integrity verification
- **Matrix Operations**: SIMD-optimized for vector operations
- **Memory Usage**: Efficient allocation with automatic cleanup

### Test Performance Results

```bash
# Stress test results (10,000 envelopes)
Created 10000 envelopes in 0.049s (~4.9μs per envelope)

# Matrix operations (large vectors)
Size 1000: 0.001ms, 1000K elements/ms throughput
Size 10000: 0.015ms, 666K elements/ms throughput

# Serialization stress test (5,000 envelopes)
Serialized 5000 envelopes (1.2 MB) in 0.023s
```

### Optimization Features

✅ **Memory Safety**: Rust's compile-time guarantees
- No buffer overflows or null pointer dereferences
- Automatic memory management with RAII
- Thread-safe concurrent operations

✅ **Performance Optimizations**:
- SIMD operations for vector calculations
- Efficient JSON serialization with serde
- Optimized matrix operations for numerical computing
- Minimal allocation overhead

✅ **Reliability Features**:
- Comprehensive error handling
- Input validation at all boundaries
- Graceful degradation under load
- Resource cleanup guarantees

## 🔧 Configuration

### Environment Variables

```bash
# WebSocket configuration
UMICP_WS_MAX_PAYLOAD=1048576
UMICP_WS_HEARTBEAT_INTERVAL=30
UMICP_WS_MAX_RECONNECT_ATTEMPTS=3

# Performance tuning
UMICP_MATRIX_PARALLEL_THRESHOLD=1000
UMICP_ENVELOPE_CACHE_SIZE=1000
```

### Transport Configuration

```rust
use umicp_core::TransportConfig;

let config = TransportConfig {
    max_payload_size: 1024 * 1024, // 1MB
    heartbeat_interval: 30,
    max_reconnect_attempts: 3,
    connection_timeout: 10,
    compression_enabled: true,
    ..Default::default()
};
```

## 🛠️ Development

### Building from Source

```bash
git clone https://github.com/cmmv-hive/umicp.git
cd umicp/bindings/rust

# Build with default features
cargo build

# Build with all features
cargo build --features full

# Run tests
cargo test

# Generate documentation
cargo doc --open
```

### Development Scripts

```bash
# Development build with debug symbols
cargo build --debug

# Run benchmarks
cargo bench

# Check code quality
cargo clippy

# Format code
cargo fmt

# Run specific example
cargo run --example basic_envelope
```

## 📖 Documentation

- [API Reference](https://docs.rs/umicp-core) - Complete API documentation
- [Examples](./examples/) - Usage examples and tutorials
- [UMICP Protocol Specification](../docs/) - Protocol specification
- [Performance Guide](./docs/performance.md) - Performance tuning guide

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](../CONTRIBUTING.md) for details.

### Development Setup

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass: `cargo test`
6. Check code quality: `cargo clippy`
7. Format code: `cargo fmt`
8. Submit a pull request

### Code Standards

- Use `cargo fmt` for consistent formatting
- Run `cargo clippy` to catch common mistakes
- Add comprehensive tests for new features
- Document public APIs with examples
- Follow Rust naming conventions

## 📄 License

This project is licensed under the CC0-1.0 License - see the [LICENSE](../LICENSE) file for details.

## 🔗 Links

- [GitHub Repository](https://github.com/cmmv-hive/umicp)
- [Crates.io Package](https://crates.io/crates/umicp-core)
- [Documentation](https://docs.rs/umicp-core)
- [Issue Tracker](https://github.com/cmmv-hive/umicp/issues)
- [UMICP Protocol](https://github.com/cmmv-hive/umicp/tree/main/docs)

## 🙏 Acknowledgments

- CMMV Hive community for protocol design and testing
- Contributors and maintainers
- Rust ecosystem for excellent async and performance tools
- Open source dependencies and libraries

---

**UMICP Rust Bindings** - High-performance communication for distributed systems.
