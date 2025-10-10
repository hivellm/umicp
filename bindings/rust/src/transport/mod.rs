/*!
# UMICP Transport Layer

WebSocket and HTTP/2 transport implementations for UMICP protocol.
*/

#[cfg(feature = "websocket")]
pub mod websocket_client;

#[cfg(feature = "websocket")]
pub mod websocket_server;

#[cfg(feature = "http2")]
pub mod http_client;

#[cfg(feature = "http2")]
pub mod http_server;

#[cfg(feature = "websocket")]
pub use websocket_client::WebSocketClient;

#[cfg(feature = "websocket")]
pub use websocket_server::WebSocketServer;

#[cfg(feature = "http2")]
pub use http_client::HttpClient;

#[cfg(feature = "http2")]
pub use http_server::HttpServer;

// Re-export legacy placeholder for backward compatibility
#[cfg(not(any(feature = "websocket", feature = "http2")))]
pub use crate::transport_legacy::{WebSocketTransport, Http2Transport};

// Module for legacy transport (placeholders)
#[cfg(not(any(feature = "websocket", feature = "http2")))]
#[path = "../transport_legacy.rs"]
mod transport_legacy;

/// Connection state for tracking connection lifecycle
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Disconnecting,
}

/// Transport statistics
#[derive(Debug, Clone, Default)]
pub struct TransportStats {
    pub messages_sent: u64,
    pub messages_received: u64,
    pub bytes_sent: u64,
    pub bytes_received: u64,
    pub active_connections: u32,
    pub total_connections: u64,
    pub uptime_seconds: u64,
    pub avg_latency_ms: Option<f64>,
}

