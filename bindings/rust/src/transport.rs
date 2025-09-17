/*!
# UMICP Transport Layer

WebSocket and HTTP/2 transport implementations for UMICP protocol.
*/

use crate::error::{Result, UmicpError};
use crate::types::*;

/// Message handler type for incoming messages
pub type MessageHandler = Box<dyn Fn(crate::Envelope, String) -> Result<()> + Send + Sync>;

/// Connection handler type for connection events
pub type ConnectionHandler = Box<dyn Fn(bool, String) -> Result<()> + Send + Sync>;

/// Placeholder WebSocket transport implementation
pub struct WebSocketTransport;

impl WebSocketTransport {
    /// Create a new WebSocket server transport
    pub fn new_server(_addr: &str) -> Result<Self> {
        Err(UmicpError::generic("WebSocket transport not implemented in this build"))
    }

    /// Create a new WebSocket client transport
    pub fn new_client(_url: &str) -> Result<Self> {
        Err(UmicpError::generic("WebSocket transport not implemented in this build"))
    }

    /// Set message handler for incoming messages
    pub fn set_message_handler<F>(&mut self, _handler: F)
    where
        F: Fn(crate::Envelope, String) -> Result<()> + Send + Sync + 'static,
    {
        // Placeholder implementation
    }

    /// Set connection handler for connection events
    pub fn set_connection_handler<F>(&mut self, _handler: F)
    where
        F: Fn(bool, String) -> Result<()> + Send + Sync + 'static,
    {
        // Placeholder implementation
    }

    /// Send message to a specific connection (server mode)
    pub fn send(&self, _envelope: crate::Envelope, _connection_id: &str) -> Result<()> {
        Err(UmicpError::generic("WebSocket transport not implemented in this build"))
    }

    /// Send message to server (client mode)
    pub fn send_to_server(&self, _envelope: crate::Envelope) -> Result<()> {
        Err(UmicpError::generic("WebSocket transport not implemented in this build"))
    }

    /// Get transport statistics
    pub fn get_stats(&self) -> TransportStats {
        TransportStats::default()
    }

    /// Shutdown the transport
    pub fn shutdown(&self) -> Result<()> {
        Ok(())
    }
}

/// Placeholder HTTP/2 transport implementation
pub struct Http2Transport;

impl Http2Transport {
    pub fn new(_url: &str) -> Self {
        Http2Transport
    }

    pub fn connect(&self) -> Result<()> {
        Err(UmicpError::generic("HTTP/2 transport not implemented in this build"))
    }

    pub fn send(&self, _message: &str) -> Result<()> {
        Err(UmicpError::generic("HTTP/2 transport not implemented in this build"))
    }

    pub fn is_connected(&self) -> bool {
        false
    }

    pub fn get_stats(&self) -> TransportStats {
        TransportStats::default()
    }
}

impl Default for TransportStats {
    fn default() -> Self {
        TransportStats {
            messages_sent: 0,
            messages_received: 0,
            bytes_sent: 0,
            bytes_received: 0,
            active_connections: 0,
            total_connections: 0,
            uptime_seconds: 0,
            avg_latency_ms: None,
        }
    }
}
