/*!
# Load Balancing

Load balancer for distributing requests across multiple service endpoints.
Supports multiple strategies: Round Robin, Random, Least Connections, and Weighted.
*/

use crate::discovery::ServiceInfo;
use crate::error::{Result, UmicpError};
use parking_lot::RwLock;
use rand::Rng;
use std::collections::HashMap;
use std::sync::Arc;
use std::sync::atomic::{AtomicU64, AtomicUsize, Ordering};

/// Load balancing strategy
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LoadBalancingStrategy {
    /// Round-robin distribution
    RoundRobin,
    /// Random selection
    Random,
    /// Least connections (select endpoint with fewest active connections)
    LeastConnections,
    /// Weighted distribution (based on endpoint weights)
    Weighted,
}

/// Backend endpoint information
#[derive(Debug, Clone)]
pub struct BackendEndpoint {
    /// Endpoint ID
    pub id: String,
    /// Endpoint address
    pub address: String,
    /// Endpoint weight (for weighted strategy, default: 1)
    pub weight: u32,
    /// Active connections count
    active_connections: Arc<AtomicUsize>,
    /// Total requests served
    total_requests: Arc<AtomicU64>,
    /// Whether endpoint is healthy
    pub healthy: bool,
}

impl BackendEndpoint {
    /// Create new backend endpoint
    pub fn new(id: String, address: String) -> Self {
        Self {
            id,
            address,
            weight: 1,
            active_connections: Arc::new(AtomicUsize::new(0)),
            total_requests: Arc::new(AtomicU64::new(0)),
            healthy: true,
        }
    }

    /// Create with weight
    pub fn with_weight(mut self, weight: u32) -> Self {
        self.weight = weight;
        self
    }

    /// Get active connections count
    pub fn active_connections(&self) -> usize {
        self.active_connections.load(Ordering::Relaxed)
    }

    /// Get total requests served
    pub fn total_requests(&self) -> u64 {
        self.total_requests.load(Ordering::Relaxed)
    }

    /// Increment active connections
    pub(crate) fn increment_connections(&self) {
        self.active_connections.fetch_add(1, Ordering::Relaxed);
        self.total_requests.fetch_add(1, Ordering::Relaxed);
    }

    /// Decrement active connections
    pub(crate) fn decrement_connections(&self) {
        self.active_connections.fetch_sub(1, Ordering::Relaxed);
    }
}

impl From<ServiceInfo> for BackendEndpoint {
    fn from(service: ServiceInfo) -> Self {
        let weight = service
            .metadata
            .get("weight")
            .and_then(|w| w.parse::<u32>().ok())
            .unwrap_or(1);

        BackendEndpoint::new(service.service_id, service.address).with_weight(weight)
    }
}

/// Load balancer
pub struct LoadBalancer {
    /// Balancing strategy
    strategy: LoadBalancingStrategy,
    /// Backend endpoints
    endpoints: Arc<RwLock<Vec<BackendEndpoint>>>,
    /// Current index for round-robin
    current_index: Arc<AtomicUsize>,
}

impl LoadBalancer {
    /// Create new load balancer with strategy
    pub fn new(strategy: LoadBalancingStrategy) -> Self {
        Self {
            strategy,
            endpoints: Arc::new(RwLock::new(Vec::new())),
            current_index: Arc::new(AtomicUsize::new(0)),
        }
    }

    /// Add backend endpoint
    pub fn add_endpoint(&self, endpoint: BackendEndpoint) {
        self.endpoints.write().push(endpoint);
    }

    /// Remove endpoint by ID
    pub fn remove_endpoint(&self, id: &str) -> bool {
        let mut endpoints = self.endpoints.write();
        let len_before = endpoints.len();
        endpoints.retain(|e| e.id != id);
        endpoints.len() < len_before
    }

    /// Get endpoint by ID
    pub fn get_endpoint(&self, id: &str) -> Option<BackendEndpoint> {
        self.endpoints
            .read()
            .iter()
            .find(|e| e.id == id)
            .cloned()
    }

    /// Mark endpoint as healthy/unhealthy
    pub fn set_endpoint_health(&self, id: &str, healthy: bool) {
        if let Some(endpoint) = self.endpoints.write().iter_mut().find(|e| e.id == id) {
            endpoint.healthy = healthy;
        }
    }

    /// Get all endpoints
    pub fn get_endpoints(&self) -> Vec<BackendEndpoint> {
        self.endpoints.read().clone()
    }

    /// Get healthy endpoints only
    pub fn get_healthy_endpoints(&self) -> Vec<BackendEndpoint> {
        self.endpoints
            .read()
            .iter()
            .filter(|e| e.healthy)
            .cloned()
            .collect()
    }

    /// Select next endpoint based on strategy
    pub fn select(&self) -> Result<BackendEndpoint> {
        let healthy_endpoints = self.get_healthy_endpoints();

        if healthy_endpoints.is_empty() {
            return Err(UmicpError::transport("No healthy endpoints available".to_string()));
        }

        let endpoint = match self.strategy {
            LoadBalancingStrategy::RoundRobin => self.select_round_robin(&healthy_endpoints),
            LoadBalancingStrategy::Random => self.select_random(&healthy_endpoints),
            LoadBalancingStrategy::LeastConnections => self.select_least_connections(&healthy_endpoints),
            LoadBalancingStrategy::Weighted => self.select_weighted(&healthy_endpoints),
        }?;

        // Increment connection count
        endpoint.increment_connections();

        Ok(endpoint)
    }

    /// Release endpoint (decrement connection count)
    pub fn release(&self, endpoint_id: &str) {
        if let Some(endpoint) = self.endpoints.read().iter().find(|e| e.id == endpoint_id) {
            endpoint.decrement_connections();
        }
    }

    /// Round-robin selection
    fn select_round_robin(&self, endpoints: &[BackendEndpoint]) -> Result<BackendEndpoint> {
        if endpoints.is_empty() {
            return Err(UmicpError::transport("No endpoints available".to_string()));
        }

        let index = self.current_index.fetch_add(1, Ordering::Relaxed) % endpoints.len();
        Ok(endpoints[index].clone())
    }

    /// Random selection
    fn select_random(&self, endpoints: &[BackendEndpoint]) -> Result<BackendEndpoint> {
        if endpoints.is_empty() {
            return Err(UmicpError::transport("No endpoints available".to_string()));
        }

        let mut rng = rand::thread_rng();
        let index = rng.gen_range(0..endpoints.len());
        Ok(endpoints[index].clone())
    }

    /// Least connections selection
    fn select_least_connections(&self, endpoints: &[BackendEndpoint]) -> Result<BackendEndpoint> {
        endpoints
            .iter()
            .min_by_key(|e| e.active_connections())
            .cloned()
            .ok_or_else(|| UmicpError::transport("No endpoints available".to_string()))
    }

    /// Weighted selection
    fn select_weighted(&self, endpoints: &[BackendEndpoint]) -> Result<BackendEndpoint> {
        if endpoints.is_empty() {
            return Err(UmicpError::transport("No endpoints available".to_string()));
        }

        // Calculate total weight
        let total_weight: u32 = endpoints.iter().map(|e| e.weight).sum();

        if total_weight == 0 {
            // Fall back to round-robin if all weights are 0
            return self.select_round_robin(endpoints);
        }

        // Generate random number in range [0, total_weight)
        let mut rng = rand::thread_rng();
        let mut random_weight = rng.gen_range(0..total_weight);

        // Select endpoint based on weight
        for endpoint in endpoints {
            if random_weight < endpoint.weight {
                return Ok(endpoint.clone());
            }
            random_weight -= endpoint.weight;
        }

        // Fallback (should not reach here)
        Ok(endpoints[0].clone())
    }

    /// Get load balancer statistics
    pub fn get_stats(&self) -> LoadBalancerStats {
        let endpoints = self.endpoints.read();
        let total_endpoints = endpoints.len();
        let healthy_endpoints = endpoints.iter().filter(|e| e.healthy).count();
        let total_connections: usize = endpoints.iter().map(|e| e.active_connections()).sum();
        let total_requests: u64 = endpoints.iter().map(|e| e.total_requests()).sum();

        LoadBalancerStats {
            strategy: self.strategy,
            total_endpoints,
            healthy_endpoints,
            total_connections,
            total_requests,
        }
    }
}

/// Load balancer statistics
#[derive(Debug, Clone)]
pub struct LoadBalancerStats {
    pub strategy: LoadBalancingStrategy,
    pub total_endpoints: usize,
    pub healthy_endpoints: usize,
    pub total_connections: usize,
    pub total_requests: u64,
}

/// RAII guard for automatic connection release
pub struct ConnectionGuard<'a> {
    balancer: &'a LoadBalancer,
    endpoint_id: String,
}

impl<'a> ConnectionGuard<'a> {
    pub fn new(balancer: &'a LoadBalancer, endpoint_id: String) -> Self {
        Self {
            balancer,
            endpoint_id,
        }
    }

    pub fn endpoint_id(&self) -> &str {
        &self.endpoint_id
    }
}

impl<'a> Drop for ConnectionGuard<'a> {
    fn drop(&mut self) {
        self.balancer.release(&self.endpoint_id);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_backend_endpoint_creation() {
        let endpoint = BackendEndpoint::new(
            "endpoint-1".to_string(),
            "http://localhost:8080".to_string(),
        );

        assert_eq!(endpoint.id, "endpoint-1");
        assert_eq!(endpoint.address, "http://localhost:8080");
        assert_eq!(endpoint.weight, 1);
        assert_eq!(endpoint.active_connections(), 0);
        assert!(endpoint.healthy);
    }

    #[test]
    fn test_backend_endpoint_with_weight() {
        let endpoint = BackendEndpoint::new(
            "endpoint-1".to_string(),
            "http://localhost:8080".to_string(),
        )
        .with_weight(5);

        assert_eq!(endpoint.weight, 5);
    }

    #[test]
    fn test_round_robin() {
        let lb = LoadBalancer::new(LoadBalancingStrategy::RoundRobin);

        lb.add_endpoint(BackendEndpoint::new("ep1".to_string(), "addr1".to_string()));
        lb.add_endpoint(BackendEndpoint::new("ep2".to_string(), "addr2".to_string()));
        lb.add_endpoint(BackendEndpoint::new("ep3".to_string(), "addr3".to_string()));

        // Should cycle through endpoints
        let e1 = lb.select().unwrap();
        let e2 = lb.select().unwrap();
        let e3 = lb.select().unwrap();
        let e4 = lb.select().unwrap(); // Should wrap around

        assert_eq!(e1.id, "ep1");
        assert_eq!(e2.id, "ep2");
        assert_eq!(e3.id, "ep3");
        assert_eq!(e4.id, "ep1"); // Wrapped
    }

    #[test]
    fn test_random() {
        let lb = LoadBalancer::new(LoadBalancingStrategy::Random);

        lb.add_endpoint(BackendEndpoint::new("ep1".to_string(), "addr1".to_string()));
        lb.add_endpoint(BackendEndpoint::new("ep2".to_string(), "addr2".to_string()));

        // Should select valid endpoints
        for _ in 0..10 {
            let endpoint = lb.select().unwrap();
            assert!(endpoint.id == "ep1" || endpoint.id == "ep2");
        }
    }

    #[test]
    fn test_least_connections() {
        let lb = LoadBalancer::new(LoadBalancingStrategy::LeastConnections);

        let ep1 = BackendEndpoint::new("ep1".to_string(), "addr1".to_string());
        let ep2 = BackendEndpoint::new("ep2".to_string(), "addr2".to_string());

        // Simulate ep1 having more connections
        ep1.increment_connections();
        ep1.increment_connections();

        lb.add_endpoint(ep1);
        lb.add_endpoint(ep2);

        // Should select ep2 (fewer connections)
        let selected = lb.select().unwrap();
        assert_eq!(selected.id, "ep2");
    }

    #[test]
    fn test_weighted() {
        let lb = LoadBalancer::new(LoadBalancingStrategy::Weighted);

        lb.add_endpoint(BackendEndpoint::new("ep1".to_string(), "addr1".to_string()).with_weight(1));
        lb.add_endpoint(BackendEndpoint::new("ep2".to_string(), "addr2".to_string()).with_weight(9));

        // With weights 1:9, ep2 should be selected ~90% of the time
        let mut ep1_count = 0;
        let mut ep2_count = 0;

        for _ in 0..100 {
            let endpoint = lb.select().unwrap();
            if endpoint.id == "ep1" {
                ep1_count += 1;
            } else {
                ep2_count += 1;
            }
        }

        // ep2 should be selected more often (rough check)
        assert!(ep2_count > ep1_count);
    }

    #[test]
    fn test_healthy_endpoints_only() {
        let lb = LoadBalancer::new(LoadBalancingStrategy::RoundRobin);

        lb.add_endpoint(BackendEndpoint::new("ep1".to_string(), "addr1".to_string()));
        lb.add_endpoint(BackendEndpoint::new("ep2".to_string(), "addr2".to_string()));

        // Mark ep1 as unhealthy
        lb.set_endpoint_health("ep1", false);

        // Should only select ep2
        for _ in 0..5 {
            let endpoint = lb.select().unwrap();
            assert_eq!(endpoint.id, "ep2");
        }
    }

    #[test]
    fn test_no_healthy_endpoints() {
        let lb = LoadBalancer::new(LoadBalancingStrategy::RoundRobin);

        lb.add_endpoint(BackendEndpoint::new("ep1".to_string(), "addr1".to_string()));
        lb.set_endpoint_health("ep1", false);

        // Should return error
        assert!(lb.select().is_err());
    }

    #[test]
    fn test_connection_release() {
        let lb = LoadBalancer::new(LoadBalancingStrategy::LeastConnections);
        lb.add_endpoint(BackendEndpoint::new("ep1".to_string(), "addr1".to_string()));

        let endpoint = lb.select().unwrap();
        assert_eq!(endpoint.active_connections(), 1);

        lb.release(&endpoint.id);

        let endpoint_after = lb.get_endpoint("ep1").unwrap();
        assert_eq!(endpoint_after.active_connections(), 0);
    }

    #[test]
    fn test_stats() {
        let lb = LoadBalancer::new(LoadBalancingStrategy::RoundRobin);

        lb.add_endpoint(BackendEndpoint::new("ep1".to_string(), "addr1".to_string()));
        lb.add_endpoint(BackendEndpoint::new("ep2".to_string(), "addr2".to_string()));
        lb.set_endpoint_health("ep2", false);

        let _ = lb.select(); // Select once

        let stats = lb.get_stats();
        assert_eq!(stats.total_endpoints, 2);
        assert_eq!(stats.healthy_endpoints, 1);
        assert_eq!(stats.total_connections, 1);
        assert_eq!(stats.total_requests, 1);
    }
}

