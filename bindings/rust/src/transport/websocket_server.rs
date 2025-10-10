/*!
# WebSocket Server Implementation

Async WebSocket server for accepting multiple client connections.
*/

use crate::error::{Result, UmicpError};
use crate::Envelope;
use futures::{SinkExt, StreamExt};
use parking_lot::RwLock;
use std::collections::HashMap;
use std::net::SocketAddr;
use std::sync::Arc;
use std::time::Duration;
use tokio::net::{TcpListener, TcpStream};
use tokio::sync::mpsc;
use tokio::task::JoinHandle;
use tokio::time::Instant;
use tokio_tungstenite::{accept_async, tungstenite::protocol::Message};

/// Message handler callback type
pub type MessageHandler = Arc<dyn Fn(Envelope, String) + Send + Sync>;

/// Connection event handler callback type
pub type ConnectionHandler = Arc<dyn Fn(String, SocketAddr) + Send + Sync>;

/// Client connection information
#[derive(Debug, Clone)]
pub struct ClientConnection {
    pub id: String,
    pub addr: SocketAddr,
    pub connected_at: Instant,
    pub metadata: HashMap<String, String>,
}

/// WebSocket server configuration
#[derive(Debug, Clone)]
pub struct WebSocketServerConfig {
    /// Address to bind to
    pub addr: SocketAddr,
    /// Ping interval
    pub ping_interval: Duration,
    /// Max payload size in bytes
    pub max_payload: usize,
    /// Enable compression
    pub compression: bool,
}

impl Default for WebSocketServerConfig {
    fn default() -> Self {
        Self {
            addr: "127.0.0.1:8080".parse().unwrap(),
            ping_interval: Duration::from_secs(30),
            max_payload: 100 * 1024 * 1024, // 100MB
            compression: true,
        }
    }
}

/// WebSocket server statistics
#[derive(Debug, Clone, Default)]
pub struct ServerStats {
    pub messages_sent: u64,
    pub messages_received: u64,
    pub bytes_sent: u64,
    pub bytes_received: u64,
    pub active_connections: u32,
    pub total_connections: u64,
    pub uptime_seconds: u64,
}

/// WebSocket Server
pub struct WebSocketServer {
    config: WebSocketServerConfig,
    clients: Arc<RwLock<HashMap<String, ClientConnection>>>,
    client_senders: Arc<RwLock<HashMap<String, mpsc::UnboundedSender<Envelope>>>>,
    stats: Arc<RwLock<ServerStats>>,
    started_at: Option<Instant>,
    shutdown_tx: mpsc::UnboundedSender<()>,
    shutdown_rx: Arc<RwLock<Option<mpsc::UnboundedReceiver<()>>>>,
    message_handler: Option<MessageHandler>,
    connection_handler: Option<ConnectionHandler>,
    disconnection_handler: Option<ConnectionHandler>,
}

impl WebSocketServer {
    /// Create a new WebSocket server
    pub fn new(addr: impl Into<String>) -> Result<Self> {
        let addr: SocketAddr = addr
            .into()
            .parse()
            .map_err(|e| UmicpError::validation(format!("Invalid address: {}", e)))?;

        let (shutdown_tx, shutdown_rx) = mpsc::unbounded_channel();

        Ok(Self {
            config: WebSocketServerConfig {
                addr,
                ..Default::default()
            },
            clients: Arc::new(RwLock::new(HashMap::new())),
            client_senders: Arc::new(RwLock::new(HashMap::new())),
            stats: Arc::new(RwLock::new(ServerStats::default())),
            started_at: None,
            shutdown_tx,
            shutdown_rx: Arc::new(RwLock::new(Some(shutdown_rx))),
            message_handler: None,
            connection_handler: None,
            disconnection_handler: None,
        })
    }

    /// Create with custom configuration
    pub fn with_config(config: WebSocketServerConfig) -> Self {
        let (shutdown_tx, shutdown_rx) = mpsc::unbounded_channel();

        Self {
            config,
            clients: Arc::new(RwLock::new(HashMap::new())),
            client_senders: Arc::new(RwLock::new(HashMap::new())),
            stats: Arc::new(RwLock::new(ServerStats::default())),
            started_at: None,
            shutdown_tx,
            shutdown_rx: Arc::new(RwLock::new(Some(shutdown_rx))),
            message_handler: None,
            connection_handler: None,
            disconnection_handler: None,
        }
    }

    /// Set message handler callback
    pub fn set_message_handler(&mut self, handler: MessageHandler) {
        self.message_handler = Some(handler);
    }

    /// Set connection handler callback
    pub fn set_connection_handler(&mut self, handler: ConnectionHandler) {
        self.connection_handler = Some(handler);
    }

    /// Set disconnection handler callback
    pub fn set_disconnection_handler(&mut self, handler: ConnectionHandler) {
        self.disconnection_handler = Some(handler);
    }

    /// Shutdown the server gracefully
    pub fn shutdown(&self) -> Result<()> {
        self.shutdown_tx
            .send(())
            .map_err(|_| UmicpError::transport("Failed to send shutdown signal".to_string()))
    }

    /// Start the server and accept connections (non-blocking)
    /// Returns a JoinHandle that can be awaited or used to check server status
    pub async fn start(&mut self) -> Result<JoinHandle<()>> {
        let listener = TcpListener::bind(&self.config.addr)
            .await
            .map_err(|e| UmicpError::transport(format!("Failed to bind: {}", e)))?;

        self.started_at = Some(Instant::now());

        tracing::info!("WebSocket server listening on {}", self.config.addr);

        let clients = Arc::clone(&self.clients);
        let client_senders = Arc::clone(&self.client_senders);
        let stats = Arc::clone(&self.stats);
        let message_handler = self.message_handler.clone();
        let connection_handler = self.connection_handler.clone();
        let disconnection_handler = self.disconnection_handler.clone();

        // Take shutdown receiver (can only start once)
        let shutdown_rx = self.shutdown_rx.write().take()
            .ok_or_else(|| UmicpError::transport("Server already started".to_string()))?;

        // Spawn accept loop in background
        let handle = tokio::spawn(async move {
            let mut shutdown_rx = shutdown_rx;

            loop {
                tokio::select! {
                    result = listener.accept() => {
                        match result {
                            Ok((stream, addr)) => {
                                tracing::info!("New connection from {}", addr);

                                let clients_clone = Arc::clone(&clients);
                                let client_senders_clone = Arc::clone(&client_senders);
                                let stats_clone = Arc::clone(&stats);
                                let message_handler_clone = message_handler.clone();
                                let connection_handler_clone = connection_handler.clone();
                                let disconnection_handler_clone = disconnection_handler.clone();

                                // Spawn handler for this client
                                tokio::spawn(async move {
                                    if let Err(e) = Self::handle_client(
                                        stream,
                                        addr,
                                        clients_clone,
                                        client_senders_clone,
                                        stats_clone,
                                        message_handler_clone,
                                        connection_handler_clone,
                                        disconnection_handler_clone,
                                    ).await {
                                        tracing::error!("Error handling client {}: {}", addr, e);
                                    }
                                });
                            }
                            Err(e) => {
                                tracing::error!("Failed to accept connection: {}", e);
                            }
                        }
                    }
                    _ = shutdown_rx.recv() => {
                        tracing::info!("Shutdown signal received, stopping server");
                        break;
                    }
                }
            }

            tracing::info!("Server stopped");
        });

        Ok(handle)
    }

    /// Handle individual client connection
    async fn handle_client(
        stream: TcpStream,
        addr: SocketAddr,
        clients: Arc<RwLock<HashMap<String, ClientConnection>>>,
        client_senders: Arc<RwLock<HashMap<String, mpsc::UnboundedSender<Envelope>>>>,
        stats: Arc<RwLock<ServerStats>>,
        message_handler: Option<MessageHandler>,
        connection_handler: Option<ConnectionHandler>,
        disconnection_handler: Option<ConnectionHandler>,
    ) -> Result<()> {
        // Upgrade to WebSocket
        let ws_stream = accept_async(stream)
            .await
            .map_err(|e| UmicpError::transport(format!("WebSocket handshake failed: {}", e)))?;

        let client_id = uuid::Uuid::new_v4().to_string();

        // Create client connection
        let client_conn = ClientConnection {
            id: client_id.clone(),
            addr,
            connected_at: Instant::now(),
            metadata: HashMap::new(),
        };

        // Register client
        clients.write().insert(client_id.clone(), client_conn);
        stats.write().active_connections += 1;
        stats.write().total_connections += 1;

        tracing::info!("Client {} connected", client_id);

        // Call connection handler
        if let Some(handler) = &connection_handler {
            handler(client_id.clone(), addr);
        }

        // Create channel for sending messages to this client
        let (tx, mut rx) = mpsc::unbounded_channel::<Envelope>();
        client_senders.write().insert(client_id.clone(), tx);

        // Split stream
        let (mut write, mut read) = ws_stream.split();

        // Create channel for control messages (ping/pong)
        let (control_tx, mut control_rx) = mpsc::unbounded_channel::<Message>();

        // Spawn send loop
        let client_id_send = client_id.clone();
        let stats_send = Arc::clone(&stats);
        let send_task = tokio::spawn(async move {
            loop {
                tokio::select! {
                    Some(envelope) = rx.recv() => {
                        match envelope.serialize() {
                            Ok(json) => {
                                let size = json.len();
                                if let Err(e) = write.send(Message::Text(json.into())).await {
                                    tracing::error!("Failed to send to {}: {}", client_id_send, e);
                                    break;
                                }
                                stats_send.write().messages_sent += 1;
                                stats_send.write().bytes_sent += size as u64;
                            }
                            Err(e) => {
                                tracing::error!("Failed to serialize envelope: {}", e);
                            }
                        }
                    }
                    Some(msg) = control_rx.recv() => {
                        if let Err(e) = write.send(msg).await {
                            tracing::error!("Failed to send control message: {}", e);
                            break;
                        }
                    }
                }
            }
        });

        // Receive loop
        while let Some(msg_result) = read.next().await {
            match msg_result {
                Ok(msg) => {
                    if let Message::Text(text) = msg {
                        stats.write().messages_received += 1;
                        stats.write().bytes_received += text.len() as u64;

                        match Envelope::deserialize(&text) {
                            Ok(envelope) => {
                                tracing::debug!("Received envelope from client {}", client_id);
                                tracing::debug!("From: {}, To: {}", envelope.from(), envelope.to());

                                // Call message handler
                                if let Some(handler) = &message_handler {
                                    handler(envelope, client_id.clone());
                                }
                            }
                            Err(e) => {
                                tracing::error!("Failed to deserialize envelope: {}", e);
                            }
                        }
                    } else if let Message::Close(_) = msg {
                        tracing::info!("Client {} closed connection", client_id);
                        break;
                    } else if let Message::Ping(data) = msg {
                        // Send pong through control channel
                        let _ = control_tx.send(Message::Pong(data));
                    }
                }
                Err(e) => {
                    tracing::error!("WebSocket error from {}: {}", client_id, e);
                    break;
                }
            }
        }

        // Cleanup
        send_task.abort();
        clients.write().remove(&client_id);
        client_senders.write().remove(&client_id);
        stats.write().active_connections -= 1;

        tracing::info!("Client {} disconnected", client_id);

        // Call disconnection handler
        if let Some(handler) = &disconnection_handler {
            handler(client_id, addr);
        }

        Ok(())
    }

    /// Send envelope to specific client
    pub async fn send_to_client(&self, client_id: &str, envelope: Envelope) -> Result<()> {
        let senders = self.client_senders.read();

        if let Some(tx) = senders.get(client_id) {
            tx.send(envelope)
                .map_err(|e| UmicpError::transport(format!("Failed to send: {}", e)))?;
            Ok(())
        } else {
            Err(UmicpError::transport(format!("Client {} not found", client_id)))
        }
    }

    /// Broadcast envelope to all connected clients
    pub async fn broadcast(&self, envelope: Envelope) -> Result<usize> {
        let senders = self.client_senders.read();
        let mut sent_count = 0;

        for (client_id, tx) in senders.iter() {
            if let Err(e) = tx.send(envelope.clone()) {
                tracing::error!("Failed to broadcast to {}: {}", client_id, e);
            } else {
                sent_count += 1;
            }
        }

        Ok(sent_count)
    }

    /// Broadcast to all except specified client
    pub async fn broadcast_except(&self, envelope: Envelope, exclude_client_id: &str) -> Result<usize> {
        let senders = self.client_senders.read();
        let mut sent_count = 0;

        for (client_id, tx) in senders.iter() {
            if client_id != exclude_client_id {
                if let Err(e) = tx.send(envelope.clone()) {
                    tracing::error!("Failed to broadcast to {}: {}", client_id, e);
                } else {
                    sent_count += 1;
                }
            }
        }

        Ok(sent_count)
    }

    /// Get all connected clients
    pub fn get_clients(&self) -> Vec<ClientConnection> {
        self.clients.read().values().cloned().collect()
    }

    /// Get specific client
    pub fn get_client(&self, client_id: &str) -> Option<ClientConnection> {
        self.clients.read().get(client_id).cloned()
    }

    /// Get server statistics
    pub fn get_stats(&self) -> ServerStats {
        let mut stats = self.stats.read().clone();

        if let Some(started_at) = self.started_at {
            stats.uptime_seconds = started_at.elapsed().as_secs();
        }

        stats
    }

}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn test_create_server() {
        let server = WebSocketServer::new("127.0.0.1:8080").unwrap();
        let stats = server.get_stats();

        assert_eq!(stats.active_connections, 0);
        assert_eq!(stats.total_connections, 0);
    }

    #[tokio::test]
    async fn test_server_config() {
        let config = WebSocketServerConfig {
            addr: "127.0.0.1:9090".parse().unwrap(),
            compression: false,
            ..Default::default()
        };

        let server = WebSocketServer::with_config(config);
        assert!(!server.config.compression);
        assert_eq!(server.config.addr.port(), 9090);
    }
}

