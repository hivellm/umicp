/*!
# Connection Pool

Connection pooling for efficient resource management in distributed systems.
Maintains a pool of reusable connections to reduce connection overhead.
*/

#[cfg(feature = "websocket")]
use crate::transport::WebSocketClient;
use parking_lot::RwLock;
use std::collections::{HashMap, VecDeque};
use std::sync::Arc;
use std::time::{Duration, SystemTime};

/// Connection state in pool
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PoolConnectionState {
    /// Connection is available for use
    Available,
    /// Connection is currently in use
    InUse,
    /// Connection is being validated
    Validating,
    /// Connection is closed
    Closed,
}

/// Pooled connection wrapper
#[cfg(feature = "websocket")]
pub struct PooledConnection {
    /// Unique connection ID
    pub id: String,
    /// Target address
    pub address: String,
    /// WebSocket client
    pub client: Arc<WebSocketClient>,
    /// Connection state
    pub state: PoolConnectionState,
    /// Last used timestamp
    pub last_used: SystemTime,
    /// Creation timestamp
    pub created_at: SystemTime,
    /// Use count
    pub use_count: usize,
}

#[cfg(feature = "websocket")]
impl PooledConnection {
    /// Create new pooled connection
    pub fn new(address: String) -> Self {
        let id = uuid::Uuid::new_v4().to_string();
        let client = Arc::new(WebSocketClient::new(&address));

        Self {
            id,
            address,
            client,
            state: PoolConnectionState::Available,
            last_used: SystemTime::now(),
            created_at: SystemTime::now(),
            use_count: 0,
        }
    }

    /// Mark connection as in use
    pub fn acquire(&mut self) {
        self.state = PoolConnectionState::InUse;
        self.last_used = SystemTime::now();
        self.use_count += 1;
    }

    /// Mark connection as available
    pub fn release(&mut self) {
        self.state = PoolConnectionState::Available;
        self.last_used = SystemTime::now();
    }

    /// Check if connection is stale
    pub fn is_stale(&self, max_age: Duration) -> bool {
        SystemTime::now()
            .duration_since(self.created_at)
            .unwrap_or(Duration::from_secs(0))
            > max_age
    }

    /// Check if connection is idle
    pub fn is_idle(&self, idle_timeout: Duration) -> bool {
        SystemTime::now()
            .duration_since(self.last_used)
            .unwrap_or(Duration::from_secs(0))
            > idle_timeout
    }

    /// Check if connection is connected
    pub fn is_connected(&self) -> bool {
        self.client.is_connected()
    }
}

/// Connection pool statistics
#[derive(Debug, Clone)]
pub struct PoolStats {
    /// Total connections in pool
    pub total_connections: usize,
    /// Available connections
    pub available_connections: usize,
    /// In-use connections
    pub in_use_connections: usize,
    /// Total connection requests
    pub total_requests: usize,
    /// Total connection acquisitions
    pub total_acquisitions: usize,
    /// Total connection releases
    pub total_releases: usize,
    /// Total connection creations
    pub total_creations: usize,
    /// Total connection closures
    pub total_closures: usize,
}

/// Connection pool configuration
#[derive(Debug, Clone)]
pub struct PoolConfig {
    /// Minimum pool size
    pub min_size: usize,
    /// Maximum pool size
    pub max_size: usize,
    /// Connection idle timeout
    pub idle_timeout: Duration,
    /// Maximum connection age
    pub max_age: Duration,
    /// Connection validation interval
    pub validation_interval: Duration,
}

impl Default for PoolConfig {
    fn default() -> Self {
        Self {
            min_size: 2,
            max_size: 10,
            idle_timeout: Duration::from_secs(300), // 5 minutes
            max_age: Duration::from_secs(3600),     // 1 hour
            validation_interval: Duration::from_secs(60), // 1 minute
        }
    }
}

/// Connection pool manager
#[cfg(feature = "websocket")]
pub struct ConnectionPool {
    /// Pool configuration
    config: PoolConfig,
    /// Connections by address
    pools: Arc<RwLock<HashMap<String, VecDeque<PooledConnection>>>>,
    /// Statistics
    stats: Arc<RwLock<PoolStats>>,
}

#[cfg(feature = "websocket")]
impl ConnectionPool {
    /// Create new connection pool
    pub fn new() -> Self {
        Self::with_config(PoolConfig::default())
    }

    /// Create with custom configuration
    pub fn with_config(config: PoolConfig) -> Self {
        Self {
            config,
            pools: Arc::new(RwLock::new(HashMap::new())),
            stats: Arc::new(RwLock::new(PoolStats {
                total_connections: 0,
                available_connections: 0,
                in_use_connections: 0,
                total_requests: 0,
                total_acquisitions: 0,
                total_releases: 0,
                total_creations: 0,
                total_closures: 0,
            })),
        }
    }

    /// Get connection from pool
    pub async fn acquire(&self, address: &str) -> Result<Arc<WebSocketClient>, String> {
        self.stats.write().total_requests += 1;

        let mut pools = self.pools.write();
        let pool = pools.entry(address.to_string()).or_insert_with(VecDeque::new);

        // Try to find available connection
        for conn in pool.iter_mut() {
            if conn.state == PoolConnectionState::Available && conn.is_connected() {
                conn.acquire();
                self.stats.write().total_acquisitions += 1;
                self.stats.write().available_connections -= 1;
                self.stats.write().in_use_connections += 1;
                return Ok(Arc::clone(&conn.client));
            }
        }

        // No available connection, check if we can create new one
        if pool.len() < self.config.max_size {
            let mut new_conn = PooledConnection::new(address.to_string());

            // Connect the client
            new_conn
                .client
                .connect()
                .await
                .map_err(|e| format!("Failed to connect: {:?}", e))?;

            new_conn.acquire();

            let client = Arc::clone(&new_conn.client);
            pool.push_back(new_conn);

            self.stats.write().total_connections += 1;
            self.stats.write().total_creations += 1;
            self.stats.write().in_use_connections += 1;
            self.stats.write().total_acquisitions += 1;

            Ok(client)
        } else {
            Err("Pool exhausted: maximum connections reached".to_string())
        }
    }

    /// Release connection back to pool
    pub fn release(&self, address: &str, client: &Arc<WebSocketClient>) {
        let mut pools = self.pools.write();

        if let Some(pool) = pools.get_mut(address) {
            for conn in pool.iter_mut() {
                if Arc::ptr_eq(&conn.client, client) {
                    conn.release();
                    self.stats.write().total_releases += 1;
                    self.stats.write().in_use_connections -= 1;
                    self.stats.write().available_connections += 1;
                    break;
                }
            }
        }
    }

    /// Clean up stale and idle connections
    pub async fn cleanup(&self) -> usize {
        let mut pools = self.pools.write();
        let mut total_removed = 0;

        for pool in pools.values_mut() {
            let before_size = pool.len();

            pool.retain(|conn| {
                let keep = conn.state != PoolConnectionState::Closed
                    && !conn.is_stale(self.config.max_age)
                    && (conn.state == PoolConnectionState::InUse
                        || !conn.is_idle(self.config.idle_timeout));

                if !keep {
                    self.stats.write().total_closures += 1;
                    self.stats.write().total_connections -= 1;
                }

                keep
            });

            total_removed += before_size - pool.len();
        }

        total_removed
    }

    /// Get pool statistics
    pub fn get_stats(&self) -> PoolStats {
        self.stats.read().clone()
    }

    /// Get pool size for address
    pub fn get_pool_size(&self, address: &str) -> usize {
        self.pools
            .read()
            .get(address)
            .map(|p| p.len())
            .unwrap_or(0)
    }

    /// Get available connections for address
    pub fn get_available_count(&self, address: &str) -> usize {
        self.pools
            .read()
            .get(address)
            .map(|p| {
                p.iter()
                    .filter(|c| c.state == PoolConnectionState::Available)
                    .count()
            })
            .unwrap_or(0)
    }

    /// Close all connections for address
    pub async fn close_pool(&self, address: &str) -> usize {
        let mut pools = self.pools.write();

        if let Some(pool) = pools.remove(address) {
            let count = pool.len();
            self.stats.write().total_connections -= count;
            self.stats.write().total_closures += count;
            count
        } else {
            0
        }
    }

    /// Close all connections
    pub async fn close_all(&self) -> usize {
        let mut pools = self.pools.write();
        let mut total_closed = 0;

        for (_address, pool) in pools.drain() {
            total_closed += pool.len();
        }

        self.stats.write().total_connections = 0;
        self.stats.write().total_closures += total_closed;

        total_closed
    }

    /// Warm up pool with minimum connections
    pub async fn warmup(&self, address: &str) -> Result<(), String> {
        for _ in 0..self.config.min_size {
            let client = self.acquire(address).await?;
            self.release(address, &client);
        }
        Ok(())
    }
}

#[cfg(feature = "websocket")]
impl Default for ConnectionPool {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
#[cfg(feature = "websocket")]
mod tests {
    use super::*;

    #[test]
    fn test_pooled_connection_creation() {
        let conn = PooledConnection::new("ws://localhost:8080".to_string());
        assert_eq!(conn.address, "ws://localhost:8080");
        assert_eq!(conn.state, PoolConnectionState::Available);
        assert_eq!(conn.use_count, 0);
    }

    #[test]
    fn test_pooled_connection_acquire_release() {
        let mut conn = PooledConnection::new("ws://localhost:8080".to_string());

        assert_eq!(conn.state, PoolConnectionState::Available);
        assert_eq!(conn.use_count, 0);

        conn.acquire();
        assert_eq!(conn.state, PoolConnectionState::InUse);
        assert_eq!(conn.use_count, 1);

        conn.release();
        assert_eq!(conn.state, PoolConnectionState::Available);
        assert_eq!(conn.use_count, 1);
    }

    #[test]
    fn test_connection_stale_detection() {
        let mut conn = PooledConnection::new("ws://localhost:8080".to_string());

        // Just created, not stale
        assert!(!conn.is_stale(Duration::from_secs(60)));

        // Simulate old connection
        conn.created_at = SystemTime::now()
            .checked_sub(Duration::from_secs(120))
            .unwrap();

        // Should be stale now
        assert!(conn.is_stale(Duration::from_secs(60)));
    }

    #[test]
    fn test_connection_idle_detection() {
        let mut conn = PooledConnection::new("ws://localhost:8080".to_string());

        // Just used, not idle
        assert!(!conn.is_idle(Duration::from_secs(60)));

        // Simulate idle connection
        conn.last_used = SystemTime::now()
            .checked_sub(Duration::from_secs(120))
            .unwrap();

        // Should be idle now
        assert!(conn.is_idle(Duration::from_secs(60)));
    }

    #[test]
    fn test_pool_config_default() {
        let config = PoolConfig::default();
        assert_eq!(config.min_size, 2);
        assert_eq!(config.max_size, 10);
    }

    #[test]
    fn test_connection_pool_creation() {
        let pool = ConnectionPool::new();
        let stats = pool.get_stats();
        assert_eq!(stats.total_connections, 0);
        assert_eq!(stats.available_connections, 0);
    }

    #[test]
    fn test_pool_stats() {
        let pool = ConnectionPool::new();
        let stats = pool.get_stats();

        assert_eq!(stats.total_connections, 0);
        assert_eq!(stats.total_requests, 0);
        assert_eq!(stats.total_acquisitions, 0);
        assert_eq!(stats.total_releases, 0);
    }
}

