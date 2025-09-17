/*!
# UMICP Rust Bindings

High-performance Rust bindings for the Universal Matrix Inter-Communication Protocol (UMICP).

UMICP provides a universal communication framework for distributed systems, federated learning,
and real-time applications with built-in matrix operations and type-safe messaging.

## Features

- **🔗 Universal Communication**: WebSocket and HTTP/2 transport layers
- **📦 Type-Safe Envelopes**: Strongly-typed message serialization and validation
- **⚡ High Performance**: SIMD-optimized matrix operations with parallel processing
- **🔄 Federated Learning**: Built-in support for ML model distribution and aggregation
- **🛡️ Security First**: Input validation, authentication, and encrypted communication
- **📊 Real-time**: Low-latency communication for IoT and financial applications
- **🧪 Well Tested**: Comprehensive test suite with async testing support

## Quick Start

### Basic Envelope Usage

```rust,no_run
use umicp_core::{Envelope, OperationType};

# fn example() -> Result<(), Box<dyn std::error::Error>> {
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
# Ok(())
# }
```

### WebSocket Transport

```rust,no_run
use umicp_core::{WebSocketTransport, Envelope, OperationType};

# fn example() -> Result<(), Box<dyn std::error::Error>> {
// Server setup (requires websocket feature)
#[cfg(feature = "websocket")]
let server = WebSocketTransport::new_server("127.0.0.1:8080")?;

// Client setup (requires websocket feature)
#[cfg(feature = "websocket")]
let client = WebSocketTransport::new_client("ws://127.0.0.1:8080")?;

// Message handling
#[cfg(feature = "websocket")]
server.set_message_handler(|envelope, conn_id| async move {
    println!("Received: {:?}", envelope.capabilities());
    // Echo response
    let response = Envelope::builder()
        .from("server")
        .to(envelope.from())
        .operation(OperationType::Ack)
        .message_id(&format!("response-{}", uuid::Uuid::new_v4()))
        .build()?;

    server.send(response, &conn_id).await?;
    Ok(())
});

// Send message
let message = Envelope::builder()
    .from("client")
    .to("server")
    .operation(OperationType::Data)
    .message_id("hello-001")
    .capability("message", "Hello UMICP!")
    .build()?;

#[cfg(feature = "websocket")]
client.send(message, "")?;
# Ok(())
# }
```

### Matrix Operations

```rust,no_run
use umicp_core::Matrix;

# fn example() -> Result<(), Box<dyn std::error::Error>> {
// Create matrix instance
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
println!("Dot product: {:?}", dot_product); // 70.0

// Matrix multiplication (2x2 * 2x2 = 2x2)
let matrix_a = vec![1.0, 2.0, 3.0, 4.0]; // 2x2 matrix
let matrix_b = vec![5.0, 6.0, 7.0, 8.0]; // 2x2 matrix
let mut matrix_result = vec![0.0; 4]; // 2x2 result

matrix.multiply(&matrix_a, &matrix_b, &mut matrix_result, 2, 2, 2)?;
println!("Matrix multiplication: {:?}", matrix_result);
# Ok(())
# }
```
*/

pub mod envelope;
pub mod matrix;
pub mod transport;
pub mod types;
pub mod error;
pub mod utils;

pub use envelope::Envelope;
pub use matrix::Matrix;
pub use transport::{WebSocketTransport, Http2Transport};
pub use types::*;
pub use error::*;

/// Version information
pub const VERSION: &str = env!("CARGO_PKG_VERSION");
pub const UMICP_VERSION: &str = "1.0";

/// Utility functions and constants
pub mod umicp {
    use super::*;

    /// Create a new envelope with default settings
    pub fn create_envelope() -> envelope::EnvelopeBuilder {
        Envelope::builder()
    }

    /// Create a new matrix instance
    pub fn create_matrix() -> Matrix {
        Matrix::new()
    }

    /// Check if WebSocket transport is available
    pub fn has_websocket_transport() -> bool {
        cfg!(feature = "websocket")
    }

    /// Check if HTTP/2 transport is available
    pub fn has_http2_transport() -> bool {
        cfg!(feature = "http2")
    }

    /// Get version information
    pub fn version() -> &'static str {
        VERSION
    }

    /// Get UMICP protocol version
    pub fn umicp_version() -> &'static str {
        UMICP_VERSION
    }
}
