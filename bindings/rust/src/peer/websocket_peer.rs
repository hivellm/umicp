/*!
# WebSocket Peer

Multiplexed peer that can act as both server and client simultaneously.
*/

use crate::error::{Result, UmicpError};
use crate::peer::{ConnectionState, PeerConnection, PeerInfo};
use crate::transport::{WebSocketClient, WebSocketServer};
use crate::Envelope;
use parking_lot::RwLock;
use std::collections::HashMap;
use std::net::SocketAddr;
use std::sync::Arc;
use tokio::sync::mpsc;
use tokio::task::JoinHandle;

/// Message handler for peers
pub type PeerMessageHandler = Arc<dyn Fn(Envelope, String) + Send + Sync>;

/// Peer event handler
pub type PeerEventHandler = Arc<dyn Fn(String, PeerInfo) + Send + Sync>;

/// WebSocket Peer configuration
#[derive(Debug, Clone)]
pub struct WebSocketPeerConfig {
    /// Local peer ID
    pub peer_id: String,

    /// Server bind address (optional)
    pub server_addr: Option<SocketAddr>,

    /// Enable auto-handshake
    pub auto_handshake: bool,

    /// Handshake timeout in seconds
    pub handshake_timeout: u64,
}

impl Default for WebSocketPeerConfig {
    fn default() -> Self {
        Self {
            peer_id: uuid::Uuid::new_v4().to_string(),
            server_addr: Some("127.0.0.1:8080".parse().unwrap()),
            auto_handshake: true,
            handshake_timeout: 10,
        }
    }
}

/// WebSocket Peer
pub struct WebSocketPeer {
    config: WebSocketPeerConfig,

    /// Server instance (if listening)
    server: Option<WebSocketServer>,
    server_handle: Option<JoinHandle<()>>,

    /// Connected peers (both incoming and outgoing)
    peers: Arc<RwLock<HashMap<String, PeerConnection>>>,

    /// Peer information
    peer_info: Arc<RwLock<HashMap<String, PeerInfo>>>,

    /// Outgoing clients
    clients: Arc<RwLock<HashMap<String, Arc<WebSocketClient>>>>,

    /// Message handler
    message_handler: Option<PeerMessageHandler>,

    /// Peer connect handler
    connect_handler: Option<PeerEventHandler>,

    /// Peer disconnect handler
    disconnect_handler: Option<PeerEventHandler>,
}

impl WebSocketPeer {
    /// Create new WebSocket peer
    pub fn new(config: WebSocketPeerConfig) -> Self {
        Self {
            config,
            server: None,
            server_handle: None,
            peers: Arc::new(RwLock::new(HashMap::new())),
            peer_info: Arc::new(RwLock::new(HashMap::new())),
            clients: Arc::new(RwLock::new(HashMap::new())),
            message_handler: None,
            connect_handler: None,
            disconnect_handler: None,
        }
    }

    /// Create with default configuration
    pub fn with_defaults() -> Self {
        Self::new(WebSocketPeerConfig::default())
    }

    /// Set message handler
    pub fn set_message_handler(&mut self, handler: PeerMessageHandler) {
        self.message_handler = Some(handler);
    }

    /// Set peer connect handler
    pub fn set_connect_handler(&mut self, handler: PeerEventHandler) {
        self.connect_handler = Some(handler);
    }

    /// Set peer disconnect handler
    pub fn set_disconnect_handler(&mut self, handler: PeerEventHandler) {
        self.disconnect_handler = Some(handler);
    }

    /// Start server (if configured)
    pub async fn start_server(&mut self) -> Result<()> {
        let addr = self.config.server_addr.ok_or_else(|| {
            UmicpError::validation("No server address configured".to_string())
        })?;

        let mut server = WebSocketServer::new(addr.to_string())?;

        // Set up server handlers
        let peers_msg = Arc::clone(&self.peers);
        let message_handler = self.message_handler.clone();

        server.set_message_handler(Arc::new(move |envelope, client_id| {
            peers_msg.read().get(&client_id).map(|conn| {
                conn.record_received(envelope.serialize().map(|s| s.len()).unwrap_or(0));
            });

            if let Some(handler) = &message_handler {
                handler(envelope, client_id);
            }
        }));

        let peer_info_conn = Arc::clone(&self.peer_info);
        let connect_handler = self.connect_handler.clone();

        server.set_connection_handler(Arc::new(move |client_id, addr| {
            let info = PeerInfo::server(&client_id, addr);
            peer_info_conn.write().insert(client_id.clone(), info.clone());

            if let Some(handler) = &connect_handler {
                handler(client_id, info);
            }
        }));

        let peer_info_disc = Arc::clone(&self.peer_info);
        let peers_disc = Arc::clone(&self.peers);
        let disconnect_handler = self.disconnect_handler.clone();

        server.set_disconnection_handler(Arc::new(move |client_id, _addr| {
            if let Some(info) = peer_info_disc.write().remove(&client_id) {
                peers_disc.write().remove(&client_id);

                if let Some(handler) = &disconnect_handler {
                    handler(client_id, info);
                }
            }
        }));

        let handle = server.start().await?;
        self.server_handle = Some(handle);
        self.server = Some(server);

        Ok(())
    }

    /// Connect to remote peer
    pub async fn connect_to_peer(&mut self, url: impl Into<String>) -> Result<String> {
        let url = url.into();
        let peer_id = uuid::Uuid::new_v4().to_string();

        let client = Arc::new(WebSocketClient::new(&url));
        client.connect().await?;

        let info = PeerInfo::client(&peer_id, &url);
        self.peer_info.write().insert(peer_id.clone(), info.clone());

        // Create message channel
        let (tx, mut rx) = mpsc::unbounded_channel();
        let conn = PeerConnection::new(&peer_id, tx);
        conn.set_state(ConnectionState::Connected);

        self.peers.write().insert(peer_id.clone(), conn);
        self.clients.write().insert(peer_id.clone(), Arc::clone(&client));

        // Spawn send loop
        let client_clone = Arc::clone(&client);
        tokio::spawn(async move {
            while let Some(envelope) = rx.recv().await {
                if let Err(e) = client_clone.send(envelope).await {
                    tracing::error!("Failed to send to peer: {}", e);
                    break;
                }
            }
        });

        if let Some(handler) = &self.connect_handler {
            handler(peer_id.clone(), info);
        }

        Ok(peer_id)
    }

    /// Send message to specific peer
    pub async fn send_to_peer(&self, peer_id: &str, envelope: Envelope) -> Result<()> {
        let peers = self.peers.read();
        let conn = peers
            .get(peer_id)
            .ok_or_else(|| UmicpError::transport(format!("Peer {} not found", peer_id)))?;

        conn.send(envelope)
    }

    /// Broadcast to all peers
    pub async fn broadcast(&self, envelope: Envelope) -> Result<()> {
        let peers = self.peers.read();

        for (peer_id, conn) in peers.iter() {
            if let Err(e) = conn.send(envelope.clone()) {
                tracing::error!("Failed to send to peer {}: {}", peer_id, e);
            }
        }

        Ok(())
    }

    /// Broadcast to all except one
    pub async fn broadcast_except(&self, except_peer_id: &str, envelope: Envelope) -> Result<()> {
        let peers = self.peers.read();

        for (peer_id, conn) in peers.iter() {
            if peer_id != except_peer_id {
                if let Err(e) = conn.send(envelope.clone()) {
                    tracing::error!("Failed to send to peer {}: {}", peer_id, e);
                }
            }
        }

        Ok(())
    }

    /// Get all peer IDs
    pub fn get_peers(&self) -> Vec<String> {
        self.peers.read().keys().cloned().collect()
    }

    /// Get peer info
    pub fn get_peer_info(&self, peer_id: &str) -> Option<PeerInfo> {
        self.peer_info.read().get(peer_id).cloned()
    }

    /// Get all peer info
    pub fn get_all_peer_info(&self) -> Vec<PeerInfo> {
        self.peer_info.read().values().cloned().collect()
    }

    /// Find peers by metadata
    pub fn find_by_metadata(&self, key: &str, value: &str) -> Vec<String> {
        self.peer_info
            .read()
            .iter()
            .filter(|(_, info)| info.get_metadata(key) == Some(&value.to_string()))
            .map(|(id, _)| id.clone())
            .collect()
    }

    /// Find peers by capability
    pub fn find_by_capability(&self, capability: &str) -> Vec<String> {
        self.peer_info
            .read()
            .iter()
            .filter(|(_, info)| info.has_capability(capability))
            .map(|(id, _)| id.clone())
            .collect()
    }

    /// Get peer count
    pub fn peer_count(&self) -> usize {
        self.peers.read().len()
    }

    /// Disconnect peer
    pub async fn disconnect_peer(&mut self, peer_id: &str) -> Result<()> {
        if let Some(client) = self.clients.write().remove(peer_id) {
            client.disconnect().await?;
        }

        if let Some(info) = self.peer_info.write().remove(peer_id) {
            self.peers.write().remove(peer_id);

            if let Some(handler) = &self.disconnect_handler {
                handler(peer_id.to_string(), info);
            }
        }

        Ok(())
    }

    /// Shutdown peer
    pub async fn shutdown(&mut self) -> Result<()> {
        // Disconnect all clients
        let client_ids: Vec<_> = self.clients.read().keys().cloned().collect();
        for peer_id in client_ids {
            let _ = self.disconnect_peer(&peer_id).await;
        }

        // Shutdown server
        if let Some(server) = &self.server {
            server.shutdown()?;
        }

        if let Some(handle) = self.server_handle.take() {
            handle.await.map_err(|e| UmicpError::transport(format!("Server shutdown error: {}", e)))?;
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_peer_creation() {
        let config = WebSocketPeerConfig::default();
        let peer = WebSocketPeer::new(config);

        assert_eq!(peer.peer_count(), 0);
    }

    #[test]
    fn test_peer_config() {
        let mut config = WebSocketPeerConfig::default();
        config.peer_id = "test-peer".to_string();
        config.auto_handshake = false;

        let peer = WebSocketPeer::new(config);
        assert_eq!(peer.config.peer_id, "test-peer");
        assert!(!peer.config.auto_handshake);
    }
}

