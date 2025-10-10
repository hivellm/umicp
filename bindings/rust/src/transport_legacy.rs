/*!
# UMICP Transport Layer - Legacy Placeholders

This file contains placeholder implementations for when WebSocket feature is not enabled.
*/

use crate::error::{Result, UmicpError};
use crate::types::TransportStats;

/// Message handler type for incoming messages
pub type MessageHandler = Box<dyn Fn(crate::Envelope, String) -> Result<()> + Send + Sync>;

/// Connection handler type for connection events
pub type ConnectionHandler = Box<dyn Fn(bool, String) -> Result<()> + Send + Sync>;

/// Placeholder WebSocket transport implementation
pub struct WebSocketTransport;

impl WebSocketTransport {
    /// Create a new WebSocket server transport
    pub fn new_server(_addr: &str) -> Result<Self> {
        Err(UmicpError::generic("WebSocket transport not implemented. Enable 'websocket' feature."))
    }

    /// Create a new WebSocket client transport
    pub fn new_client(_url: &str) -> Result<Self> {
        Err(UmicpError::generic("WebSocket transport not implemented. Enable 'websocket' feature."))
    }

    /// Get transport statistics
    pub fn get_stats(&self) -> TransportStats {
        TransportStats::default()
    }
}

/// Placeholder HTTP/2 transport implementation
pub struct Http2Transport;

impl Http2Transport {
    pub fn new(_url: &str) -> Self {
        Http2Transport
    }

    pub fn connect(&self) -> Result<()> {
        Err(UmicpError::generic("HTTP/2 transport not implemented. Enable 'http2' feature."))
    }

    pub fn get_stats(&self) -> TransportStats {
        TransportStats::default()
    }
}
