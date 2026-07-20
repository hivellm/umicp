/*!
# Peer Connection

Represents an active connection to a peer.
*/

use crate::error::{Result, UmicpError};
use crate::Envelope;
use parking_lot::RwLock;
use std::sync::Arc;
use tokio::sync::mpsc;
use tokio::time::Instant;

/// Connection state
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConnectionState {
    Connecting,
    Handshaking,
    Connected,
    Disconnecting,
    Disconnected,
}

/// Peer connection
pub struct PeerConnection {
    /// Peer ID
    pub id: String,

    /// Connection state
    state: Arc<RwLock<ConnectionState>>,

    /// Message sender
    tx: mpsc::UnboundedSender<Envelope>,

    /// Connection timestamp
    connected_at: Instant,

    /// Statistics
    stats: Arc<RwLock<ConnectionStats>>,
}

/// Connection statistics
#[derive(Debug, Clone, Default)]
pub struct ConnectionStats {
    pub messages_sent: u64,
    pub messages_received: u64,
    pub bytes_sent: u64,
    pub bytes_received: u64,
}

impl PeerConnection {
    /// Create new peer connection
    pub fn new(id: impl Into<String>, tx: mpsc::UnboundedSender<Envelope>) -> Self {
        Self {
            id: id.into(),
            state: Arc::new(RwLock::new(ConnectionState::Connecting)),
            tx,
            connected_at: Instant::now(),
            stats: Arc::new(RwLock::new(ConnectionStats::default())),
        }
    }

    /// Get peer ID
    pub fn id(&self) -> &str {
        &self.id
    }

    /// Get connection state
    pub fn state(&self) -> ConnectionState {
        *self.state.read()
    }

    /// Set connection state
    pub fn set_state(&self, state: ConnectionState) {
        *self.state.write() = state;
    }

    /// Check if connected
    pub fn is_connected(&self) -> bool {
        matches!(self.state(), ConnectionState::Connected)
    }

    /// Send envelope
    pub fn send(&self, envelope: Envelope) -> Result<()> {
        if !self.is_connected() {
            return Err(UmicpError::transport(format!(
                "Peer {} is not connected",
                self.id
            )));
        }

        let size = envelope.serialize()?.len();

        self.tx
            .send(envelope)
            .map_err(|_| UmicpError::transport("Failed to send envelope".to_string()))?;

        let mut stats = self.stats.write();
        stats.messages_sent += 1;
        stats.bytes_sent += size as u64;

        Ok(())
    }

    /// Record received message
    pub fn record_received(&self, size: usize) {
        let mut stats = self.stats.write();
        stats.messages_received += 1;
        stats.bytes_received += size as u64;
    }

    /// Get statistics
    pub fn stats(&self) -> ConnectionStats {
        self.stats.read().clone()
    }

    /// Get uptime in seconds
    pub fn uptime(&self) -> u64 {
        self.connected_at.elapsed().as_secs()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_peer_connection_creation() {
        let (tx, _rx) = mpsc::unbounded_channel();
        let conn = PeerConnection::new("peer-1", tx);

        assert_eq!(conn.id(), "peer-1");
        assert_eq!(conn.state(), ConnectionState::Connecting);
        assert!(!conn.is_connected());
    }

    #[test]
    fn test_state_management() {
        let (tx, _rx) = mpsc::unbounded_channel();
        let conn = PeerConnection::new("peer-1", tx);

        conn.set_state(ConnectionState::Connected);
        assert!(conn.is_connected());

        conn.set_state(ConnectionState::Disconnected);
        assert!(!conn.is_connected());
    }

    #[test]
    fn test_stats_tracking() {
        let (tx, _rx) = mpsc::unbounded_channel();
        let conn = PeerConnection::new("peer-1", tx);

        conn.record_received(100);
        conn.record_received(200);

        let stats = conn.stats();
        assert_eq!(stats.messages_received, 2);
        assert_eq!(stats.bytes_received, 300);
    }
}

