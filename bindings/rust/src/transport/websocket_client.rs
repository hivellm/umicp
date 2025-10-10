/*!
# WebSocket Client Implementation

Async WebSocket client with auto-reconnection and message handling.
*/

use crate::error::{Result, UmicpError};
use crate::Envelope;
use futures::{SinkExt, StreamExt};
use parking_lot::RwLock;
use std::sync::Arc;
use std::time::Duration;
use tokio::net::TcpStream;
use tokio::sync::mpsc;
use tokio::time::{sleep, Instant};
use tokio_tungstenite::{
    connect_async, tungstenite::protocol::Message, MaybeTlsStream, WebSocketStream,
};

/// Connection state
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Disconnecting,
}

/// WebSocket client configuration
#[derive(Debug, Clone)]
pub struct WebSocketClientConfig {
    /// WebSocket URL (ws:// or wss://)
    pub url: String,
    /// Connection timeout in milliseconds
    pub connection_timeout: Duration,
    /// Auto-reconnect on disconnect
    pub auto_reconnect: bool,
    /// Max reconnection attempts (0 = infinite)
    pub max_reconnect_attempts: u32,
    /// Initial reconnect delay
    pub reconnect_delay: Duration,
    /// Max reconnect delay
    pub max_reconnect_delay: Duration,
    /// Ping interval
    pub ping_interval: Duration,
}

impl Default for WebSocketClientConfig {
    fn default() -> Self {
        Self {
            url: String::new(),
            connection_timeout: Duration::from_secs(10),
            auto_reconnect: true,
            max_reconnect_attempts: 5,
            reconnect_delay: Duration::from_secs(1),
            max_reconnect_delay: Duration::from_secs(30),
            ping_interval: Duration::from_secs(30),
        }
    }
}

/// WebSocket client statistics
#[derive(Debug, Clone, Default)]
pub struct ClientStats {
    pub messages_sent: u64,
    pub messages_received: u64,
    pub bytes_sent: u64,
    pub bytes_received: u64,
    pub connect_count: u64,
    pub disconnect_count: u64,
    pub reconnect_count: u64,
    pub uptime_seconds: u64,
}

/// WebSocket client state
struct ClientState {
    state: ConnectionState,
    stats: ClientStats,
    connected_at: Option<Instant>,
    reconnect_attempts: u32,
}

/// Message handler callback type
pub type ClientMessageHandler = Arc<dyn Fn(Envelope) + Send + Sync>;

/// WebSocket Client
pub struct WebSocketClient {
    config: WebSocketClientConfig,
    state: Arc<RwLock<ClientState>>,
    message_tx: mpsc::UnboundedSender<Envelope>,
    message_rx: Arc<RwLock<Option<mpsc::UnboundedReceiver<Envelope>>>>,
    shutdown_tx: mpsc::UnboundedSender<()>,
    shutdown_rx: Arc<RwLock<Option<mpsc::UnboundedReceiver<()>>>>,
    message_handler: Arc<RwLock<Option<ClientMessageHandler>>>,
}

impl WebSocketClient {
    /// Create a new WebSocket client
    pub fn new(url: impl Into<String>) -> Self {
        let (message_tx, message_rx) = mpsc::unbounded_channel();
        let (shutdown_tx, shutdown_rx) = mpsc::unbounded_channel();

        Self {
            config: WebSocketClientConfig {
                url: url.into(),
                ..Default::default()
            },
            state: Arc::new(RwLock::new(ClientState {
                state: ConnectionState::Disconnected,
                stats: ClientStats::default(),
                connected_at: None,
                reconnect_attempts: 0,
            })),
            message_tx,
            message_rx: Arc::new(RwLock::new(Some(message_rx))),
            shutdown_tx,
            shutdown_rx: Arc::new(RwLock::new(Some(shutdown_rx))),
            message_handler: Arc::new(RwLock::new(None)),
        }
    }

    /// Set message handler for received messages
    pub fn set_message_handler(&self, handler: ClientMessageHandler) {
        *self.message_handler.write() = Some(handler);
    }

    /// Create with custom configuration
    pub fn with_config(config: WebSocketClientConfig) -> Self {
        let (message_tx, message_rx) = mpsc::unbounded_channel();
        let (shutdown_tx, shutdown_rx) = mpsc::unbounded_channel();

        Self {
            config,
            state: Arc::new(RwLock::new(ClientState {
                state: ConnectionState::Disconnected,
                stats: ClientStats::default(),
                connected_at: None,
                reconnect_attempts: 0,
            })),
            message_tx,
            message_rx: Arc::new(RwLock::new(Some(message_rx))),
            shutdown_tx,
            shutdown_rx: Arc::new(RwLock::new(Some(shutdown_rx))),
            message_handler: Arc::new(RwLock::new(None)),
        }
    }

    /// Connect to WebSocket server
    pub async fn connect(&self) -> Result<()> {
        self.state.write().state = ConnectionState::Connecting;

        match connect_async(&self.config.url).await {
            Ok((ws_stream, _response)) => {
                self.state.write().state = ConnectionState::Connected;
                self.state.write().stats.connect_count += 1;
                self.state.write().connected_at = Some(Instant::now());
                self.state.write().reconnect_attempts = 0;

                // Start message loop
                self.start_message_loop(ws_stream).await;

                Ok(())
            }
            Err(e) => {
                self.state.write().state = ConnectionState::Disconnected;
                Err(UmicpError::transport(format!("Failed to connect: {}", e)))
            }
        }
    }

    /// Connect with auto-reconnection
    pub async fn connect_with_retry(&self) -> Result<()> {
        let mut attempts = 0;
        let max_attempts = self.config.max_reconnect_attempts;

        loop {
            match self.connect().await {
                Ok(()) => return Ok(()),
                Err(e) => {
                    attempts += 1;

                    if max_attempts > 0 && attempts >= max_attempts {
                        return Err(UmicpError::transport(format!(
                            "Failed to connect after {} attempts: {}",
                            attempts, e
                        )));
                    }

                    // Calculate backoff delay
                    let delay = std::cmp::min(
                        self.config.reconnect_delay * attempts,
                        self.config.max_reconnect_delay,
                    );

                    tracing::warn!(
                        "Connection attempt {} failed, retrying in {:?}...",
                        attempts,
                        delay
                    );

                    sleep(delay).await;
                }
            }
        }
    }

    /// Send envelope to server
    pub async fn send(&self, envelope: Envelope) -> Result<()> {
        if !self.is_connected() {
            return Err(UmicpError::transport("Not connected"));
        }

        self.message_tx
            .send(envelope)
            .map_err(|e| UmicpError::transport(format!("Failed to send message: {}", e)))?;

        self.state.write().stats.messages_sent += 1;

        Ok(())
    }

    /// Check if connected
    pub fn is_connected(&self) -> bool {
        self.state.read().state == ConnectionState::Connected
    }

    /// Get connection state
    pub fn get_state(&self) -> ConnectionState {
        self.state.read().state
    }

    /// Get statistics
    pub fn get_stats(&self) -> ClientStats {
        let mut stats = self.state.read().stats.clone();

        if let Some(connected_at) = self.state.read().connected_at {
            stats.uptime_seconds = connected_at.elapsed().as_secs();
        }

        stats
    }

    /// Disconnect from server
    pub async fn disconnect(&self) -> Result<()> {
        self.state.write().state = ConnectionState::Disconnecting;

        // Send shutdown signal
        let _ = self.shutdown_tx.send(());

        // Wait a bit for graceful shutdown
        sleep(Duration::from_millis(100)).await;

        self.state.write().state = ConnectionState::Disconnected;
        self.state.write().stats.disconnect_count += 1;
        self.state.write().connected_at = None;

        Ok(())
    }

    /// Start message send/receive loop
    async fn start_message_loop(&self, ws_stream: WebSocketStream<MaybeTlsStream<TcpStream>>) {
        let (mut write, mut read) = ws_stream.split();

        let state_send = Arc::clone(&self.state);
        let state_recv = Arc::clone(&self.state);
        let message_rx = self.message_rx.write().take();
        let shutdown_rx = self.shutdown_rx.write().take();
        let message_handler = Arc::clone(&self.message_handler);

        if message_rx.is_none() || shutdown_rx.is_none() {
            tracing::error!("Message loop already started");
            return;
        }

        let mut message_rx = message_rx.unwrap();
        let mut shutdown_rx = shutdown_rx.unwrap();

        // Spawn send loop
        let send_task = tokio::spawn(async move {
            loop {
                tokio::select! {
                    Some(envelope) = message_rx.recv() => {
                        // Serialize envelope
                        match envelope.serialize() {
                            Ok(json) => {
                                let size = json.len();
                                if let Err(e) = write.send(Message::Text(json.into())).await {
                                    tracing::error!("Failed to send message: {}", e);
                                    break;
                                }
                                state_send.write().stats.bytes_sent += size as u64;
                            }
                            Err(e) => {
                                tracing::error!("Failed to serialize envelope: {}", e);
                            }
                        }
                    }
                    Some(_) = shutdown_rx.recv() => {
                        tracing::info!("Shutting down send loop");
                        let _ = write.close().await;
                        break;
                    }
                }
            }
        });

        // Receive loop (in current task)
        while let Some(msg_result) = read.next().await {
            match msg_result {
                Ok(msg) => {
                    if let Message::Text(text) = msg {
                        state_recv.write().stats.messages_received += 1;
                        state_recv.write().stats.bytes_received += text.len() as u64;

                        // Deserialize envelope
                        match Envelope::deserialize(&text) {
                            Ok(envelope) => {
                                tracing::debug!("Received envelope from {}", envelope.from());

                                // Call message handler if set
                                if let Some(handler) = message_handler.read().as_ref() {
                                    handler(envelope);
                                }
                            }
                            Err(e) => {
                                tracing::error!("Failed to deserialize envelope: {}", e);
                            }
                        }
                    } else if let Message::Close(_) = msg {
                        tracing::info!("Server closed connection");
                        break;
                    }
                }
                Err(e) => {
                    tracing::error!("WebSocket error: {}", e);
                    break;
                }
            }
        }

        // Cleanup
        send_task.abort();
        state_recv.write().state = ConnectionState::Disconnected;

        tracing::info!("Message loop ended");

        // Note: Auto-reconnection should be handled externally by the user
        // Calling self.connect_with_retry() here would cause infinite recursion
    }
}

impl Drop for WebSocketClient {
    fn drop(&mut self) {
        // Best effort disconnect
        let _ = self.shutdown_tx.send(());
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[tokio::test]
    async fn test_create_client() {
        let client = WebSocketClient::new("ws://localhost:8080");
        assert_eq!(client.get_state(), ConnectionState::Disconnected);
    }

    #[tokio::test]
    async fn test_client_config() {
        let config = WebSocketClientConfig {
            url: "ws://localhost:8080".to_string(),
            auto_reconnect: false,
            max_reconnect_attempts: 3,
            ..Default::default()
        };

        let client = WebSocketClient::with_config(config);
        assert!(!client.config.auto_reconnect);
        assert_eq!(client.config.max_reconnect_attempts, 3);
    }

    #[tokio::test]
    async fn test_stats_tracking() {
        let client = WebSocketClient::new("ws://localhost:8080");
        let stats = client.get_stats();

        assert_eq!(stats.messages_sent, 0);
        assert_eq!(stats.messages_received, 0);
        assert_eq!(stats.connect_count, 0);
    }
}

