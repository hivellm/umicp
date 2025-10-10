/*!
# HTTP Client Implementation

HTTP/1.1 and HTTP/2 client using reqwest for UMICP protocol.

The client uses HTTP/2 Prior Knowledge mode for maximum performance
when communicating with known HTTP/2-capable servers.

## Features
- HTTP/2 with multiplexing
- Connection pooling
- Automatic retries
- Statistics tracking
- Health checks
*/

use crate::error::{Result, UmicpError};
use crate::Envelope;
use parking_lot::RwLock;
use reqwest::Client;
use std::sync::Arc;
use std::time::Duration;

/// HTTP client configuration
#[derive(Debug, Clone)]
pub struct HttpClientConfig {
    /// Base URL (e.g., http://localhost:3000)
    pub base_url: String,
    /// Request timeout
    pub timeout: Duration,
    /// Max retries
    pub max_retries: u32,
}

impl Default for HttpClientConfig {
    fn default() -> Self {
        Self {
            base_url: String::new(),
            timeout: Duration::from_secs(30),
            max_retries: 3,
        }
    }
}

/// HTTP client statistics
#[derive(Debug, Clone, Default)]
pub struct HttpClientStats {
    pub requests_sent: u64,
    pub responses_received: u64,
    pub bytes_sent: u64,
    pub bytes_received: u64,
    pub errors: u64,
}

/// HTTP Client
pub struct HttpClient {
    config: HttpClientConfig,
    client: Client,
    stats: Arc<RwLock<HttpClientStats>>,
}

impl HttpClient {
    /// Create new HTTP client with HTTP/2 support
    pub fn new(base_url: impl Into<String>) -> Result<Self> {
        let client = Client::builder()
            .http2_prior_knowledge()  // Force HTTP/2
            .timeout(Duration::from_secs(30))
            .pool_idle_timeout(Duration::from_secs(90))
            .pool_max_idle_per_host(10)
            .build()
            .map_err(|e| UmicpError::transport(format!("Failed to create client: {}", e)))?;

        Ok(Self {
            config: HttpClientConfig {
                base_url: base_url.into(),
                ..Default::default()
            },
            client,
            stats: Arc::new(RwLock::new(HttpClientStats::default())),
        })
    }

    /// Create with custom configuration
    pub fn with_config(config: HttpClientConfig) -> Result<Self> {
        let client = Client::builder()
            .http2_prior_knowledge()  // Force HTTP/2
            .timeout(config.timeout)
            .pool_idle_timeout(Duration::from_secs(90))
            .pool_max_idle_per_host(10)
            .build()
            .map_err(|e| UmicpError::transport(format!("Failed to create client: {}", e)))?;

        Ok(Self {
            config,
            client,
            stats: Arc::new(RwLock::new(HttpClientStats::default())),
        })
    }

    /// Send envelope
    pub async fn send(&self, envelope: Envelope) -> Result<serde_json::Value> {
        let url = format!("{}/message", self.config.base_url);

        self.stats.write().requests_sent += 1;

        // Serialize envelope to JSON string
        let json_str = envelope.serialize()?;
        let json_value: serde_json::Value = serde_json::from_str(&json_str)
            .map_err(|e| UmicpError::serialization(format!("Failed to parse JSON: {}", e)))?;

        let response = self
            .client
            .post(&url)
            .json(&json_value)
            .send()
            .await
            .map_err(|e| {
                self.stats.write().errors += 1;
                UmicpError::transport(format!("Request failed: {}", e))
            })?;

        self.stats.write().responses_received += 1;

        if !response.status().is_success() {
            self.stats.write().errors += 1;
            return Err(UmicpError::transport(format!(
                "Server returned error: {}",
                response.status()
            )));
        }

        let json = response
            .json::<serde_json::Value>()
            .await
            .map_err(|e| UmicpError::serialization(format!("Failed to parse response: {}", e)))?;

        Ok(json)
    }

    /// Send with retry
    pub async fn send_with_retry(&self, envelope: Envelope) -> Result<serde_json::Value> {
        let mut last_error = None;

        for attempt in 0..=self.config.max_retries {
            match self.send(envelope.clone()).await {
                Ok(response) => return Ok(response),
                Err(e) => {
                    last_error = Some(e);
                    if attempt < self.config.max_retries {
                        let delay = Duration::from_millis(100 * 2u64.pow(attempt));
                        tokio::time::sleep(delay).await;
                    }
                }
            }
        }

        Err(last_error.unwrap())
    }

    /// Health check
    pub async fn health_check(&self) -> Result<serde_json::Value> {
        let url = format!("{}/health", self.config.base_url);

        let response = self
            .client
            .get(&url)
            .send()
            .await
            .map_err(|e| UmicpError::transport(format!("Health check failed: {}", e)))?;

        let json = response
            .json::<serde_json::Value>()
            .await
            .map_err(|e| UmicpError::serialization(format!("Failed to parse response: {}", e)))?;

        Ok(json)
    }

    /// Get statistics
    pub fn get_stats(&self) -> HttpClientStats {
        self.stats.read().clone()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_create_client() {
        let client = HttpClient::new("http://localhost:3000");
        assert!(client.is_ok());
    }

    #[test]
    fn test_client_config() {
        let config = HttpClientConfig {
            base_url: "http://example.com".to_string(),
            timeout: Duration::from_secs(10),
            max_retries: 5,
        };

        let client = HttpClient::with_config(config);
        assert!(client.is_ok());
    }
}

