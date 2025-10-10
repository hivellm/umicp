/*!
# Peer Information

Metadata about a connected peer.
*/

use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::net::SocketAddr;
use tokio::time::Instant;

/// Peer information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PeerInfo {
    /// Unique peer ID
    pub id: String,

    /// Peer URL (ws://... or wss://...)
    pub url: Option<String>,

    /// Socket address (for incoming connections)
    pub addr: Option<SocketAddr>,

    /// Peer type (client or server)
    pub peer_type: PeerType,

    /// Connection timestamp
    #[serde(skip)]
    pub connected_at: Option<Instant>,

    /// Custom metadata
    pub metadata: HashMap<String, String>,

    /// Peer capabilities
    pub capabilities: Vec<String>,

    /// Peer version
    pub version: Option<String>,
}

/// Peer type
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum PeerType {
    /// Client connection (outgoing)
    Client,

    /// Server connection (incoming)
    Server,
}

impl PeerInfo {
    /// Create new peer info
    pub fn new(id: impl Into<String>, peer_type: PeerType) -> Self {
        Self {
            id: id.into(),
            url: None,
            addr: None,
            peer_type,
            connected_at: Some(Instant::now()),
            metadata: HashMap::new(),
            capabilities: Vec::new(),
            version: None,
        }
    }

    /// Create client peer info
    pub fn client(id: impl Into<String>, url: impl Into<String>) -> Self {
        let mut info = Self::new(id, PeerType::Client);
        info.url = Some(url.into());
        info
    }

    /// Create server peer info
    pub fn server(id: impl Into<String>, addr: SocketAddr) -> Self {
        let mut info = Self::new(id, PeerType::Server);
        info.addr = Some(addr);
        info
    }

    /// Add metadata
    pub fn with_metadata(mut self, key: impl Into<String>, value: impl Into<String>) -> Self {
        self.metadata.insert(key.into(), value.into());
        self
    }

    /// Add capability
    pub fn with_capability(mut self, capability: impl Into<String>) -> Self {
        self.capabilities.push(capability.into());
        self
    }

    /// Set version
    pub fn with_version(mut self, version: impl Into<String>) -> Self {
        self.version = Some(version.into());
        self
    }

    /// Get metadata value
    pub fn get_metadata(&self, key: &str) -> Option<&String> {
        self.metadata.get(key)
    }

    /// Check if has capability
    pub fn has_capability(&self, capability: &str) -> bool {
        self.capabilities.contains(&capability.to_string())
    }

    /// Get connection uptime in seconds
    pub fn uptime(&self) -> u64 {
        self.connected_at
            .map(|t| t.elapsed().as_secs())
            .unwrap_or(0)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_peer_info_creation() {
        let peer = PeerInfo::new("peer-1", PeerType::Client);
        assert_eq!(peer.id, "peer-1");
        assert_eq!(peer.peer_type, PeerType::Client);
    }

    #[test]
    fn test_client_peer() {
        let peer = PeerInfo::client("peer-1", "ws://localhost:8080");
        assert_eq!(peer.peer_type, PeerType::Client);
        assert_eq!(peer.url, Some("ws://localhost:8080".to_string()));
    }

    #[test]
    fn test_with_metadata() {
        let peer = PeerInfo::new("peer-1", PeerType::Client)
            .with_metadata("app", "test")
            .with_capability("sync")
            .with_version("1.0.0");

        assert_eq!(peer.get_metadata("app"), Some(&"test".to_string()));
        assert!(peer.has_capability("sync"));
        assert_eq!(peer.version, Some("1.0.0".to_string()));
    }
}

