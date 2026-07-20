/*!
# HTTP Server Implementation

Streaming HTTP/1.1 and HTTP/2 server using axum for UMICP protocol.

The server automatically negotiates HTTP/2 via ALPN when available,
falling back to HTTP/1.1 for compatibility.

## Features
- HTTP/2 multiplexing (automatic via hyper 1.x)
- Streaming responses
- JSON API for envelopes
- Statistics tracking
- Graceful shutdown
*/

use crate::error::{Result, UmicpError};
use crate::Envelope;
use axum::{
    extract::State,
    http::StatusCode,
    response::IntoResponse,
    routing::{get, post},
    Json, Router,
};
use parking_lot::RwLock;
use std::net::SocketAddr;
use std::sync::Arc;
use tokio::net::TcpListener;
use tokio::sync::mpsc;
use tokio::task::JoinHandle;
use tokio::time::Instant;

/// HTTP server message handler
pub type HttpMessageHandler = Arc<dyn Fn(Envelope, String) + Send + Sync>;

/// HTTP server configuration
#[derive(Debug, Clone)]
pub struct HttpServerConfig {
    /// Address to bind to
    pub addr: SocketAddr,
    /// Max request size in bytes
    pub max_request_size: usize,
}

impl Default for HttpServerConfig {
    fn default() -> Self {
        Self {
            addr: "127.0.0.1:3000".parse().unwrap(),
            max_request_size: 10 * 1024 * 1024, // 10MB
        }
    }
}

/// HTTP server statistics
#[derive(Debug, Clone, Default)]
pub struct HttpServerStats {
    pub requests_received: u64,
    pub responses_sent: u64,
    pub bytes_received: u64,
    pub bytes_sent: u64,
    pub errors: u64,
}

/// Shared server state
#[derive(Clone)]
struct ServerState {
    stats: Arc<RwLock<HttpServerStats>>,
    message_handler: Option<HttpMessageHandler>,
    started_at: Instant,
}

/// HTTP Server
pub struct HttpServer {
    config: HttpServerConfig,
    state: Arc<RwLock<ServerState>>,
    shutdown_tx: mpsc::UnboundedSender<()>,
    shutdown_rx: Arc<RwLock<Option<mpsc::UnboundedReceiver<()>>>>,
}

impl HttpServer {
    /// Create new HTTP server
    pub fn new(addr: impl Into<String>) -> Result<Self> {
        let addr: SocketAddr = addr
            .into()
            .parse()
            .map_err(|e| UmicpError::validation(format!("Invalid address: {}", e)))?;

        let (shutdown_tx, shutdown_rx) = mpsc::unbounded_channel();

        Ok(Self {
            config: HttpServerConfig {
                addr,
                ..Default::default()
            },
            state: Arc::new(RwLock::new(ServerState {
                stats: Arc::new(RwLock::new(HttpServerStats::default())),
                message_handler: None,
                started_at: Instant::now(),
            })),
            shutdown_tx,
            shutdown_rx: Arc::new(RwLock::new(Some(shutdown_rx))),
        })
    }

    /// Set message handler
    pub fn set_message_handler(&mut self, handler: HttpMessageHandler) {
        self.state.write().message_handler = Some(handler);
    }

    /// Start server (non-blocking)
    pub async fn start(&mut self) -> Result<JoinHandle<()>> {
        let state = self.state.read().clone();
        let addr = self.config.addr;

        // Build router
        let app = Router::new()
            .route("/", get(root_handler))
            .route("/health", get(health_handler))
            .route("/message", post(message_handler))
            .with_state(state.clone());

        let listener = TcpListener::bind(&addr)
            .await
            .map_err(|e| UmicpError::transport(format!("Failed to bind: {}", e)))?;

        tracing::info!("HTTP server listening on {}", addr);

        // Take shutdown receiver
        let mut shutdown_rx = self
            .shutdown_rx
            .write()
            .take()
            .ok_or_else(|| UmicpError::transport("Server already started".to_string()))?;

        // Spawn server task with HTTP/2 support (axum 0.8+)
        let handle = tokio::spawn(async move {
            // Serve with graceful shutdown
            axum::serve(listener, app)
                .with_graceful_shutdown(async move {
                    shutdown_rx.recv().await;
                    tracing::info!("HTTP server shutdown signal received");
                })
                .await
                .unwrap_or_else(|e| {
                    tracing::error!("HTTP server error: {}", e);
                });

            tracing::info!("HTTP server stopped");
        });

        Ok(handle)
    }

    /// Shutdown server
    pub fn shutdown(&self) -> Result<()> {
        self.shutdown_tx
            .send(())
            .map_err(|_| UmicpError::transport("Failed to send shutdown signal".to_string()))
    }

    /// Get statistics
    pub fn get_stats(&self) -> HttpServerStats {
        self.state.read().stats.read().clone()
    }
}

/// Root handler
async fn root_handler() -> &'static str {
    "UMICP HTTP Server"
}

/// Health check handler
async fn health_handler(State(state): State<ServerState>) -> impl IntoResponse {
    let stats = state.stats.read().clone();
    let uptime = state.started_at.elapsed().as_secs();

    let response = serde_json::json!({
        "status": "ok",
        "uptime_seconds": uptime,
        "requests": stats.requests_received,
        "responses": stats.responses_sent,
    });

    Json(response)
}

/// Message handler
async fn message_handler(
    State(state): State<ServerState>,
    Json(json_value): Json<serde_json::Value>,
) -> impl IntoResponse {
    state.stats.write().requests_received += 1;

    // Deserialize JSON value to envelope
    let json_str = match serde_json::to_string(&json_value) {
        Ok(s) => s,
        Err(e) => {
            state.stats.write().errors += 1;
            return (
                StatusCode::BAD_REQUEST,
                Json(serde_json::json!({
                    "status": "error",
                    "message": format!("Invalid JSON: {}", e),
                })),
            );
        }
    };

    let envelope = match Envelope::deserialize(&json_str) {
        Ok(env) => env,
        Err(e) => {
            state.stats.write().errors += 1;
            return (
                StatusCode::BAD_REQUEST,
                Json(serde_json::json!({
                    "status": "error",
                    "message": format!("Invalid envelope: {}", e),
                })),
            );
        }
    };

    // Call message handler
    if let Some(handler) = &state.message_handler {
        let client_id = "http-client".to_string(); // TODO: Extract from headers
        handler(envelope.clone(), client_id);
    }

    state.stats.write().responses_sent += 1;

    // Return acknowledgment
    let response = serde_json::json!({
        "status": "ok",
        "message_id": envelope.message_id(),
    });

    (StatusCode::OK, Json(response))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_create_server() {
        let server = HttpServer::new("127.0.0.1:3001");
        assert!(server.is_ok());
    }

    #[test]
    fn test_server_config() {
        let server = HttpServer::new("127.0.0.1:3002").unwrap();
        assert_eq!(server.config.addr.port(), 3002);
    }
}

