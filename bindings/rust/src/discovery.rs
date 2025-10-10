/*!
# Service Discovery

Basic service discovery for peer-to-peer networks.
Allows peers to discover and connect to each other based on capabilities and metadata.
*/

use crate::{Envelope, OperationType};
#[cfg(feature = "websocket")]
use crate::peer::PeerInfo;
use parking_lot::RwLock;
use std::collections::HashMap;
use std::sync::Arc;
use std::time::{Duration, SystemTime};

/// Discovered service information
#[derive(Debug, Clone)]
pub struct ServiceInfo {
    /// Unique service ID
    pub service_id: String,
    /// Service name
    pub name: String,
    /// Service address (URL)
    pub address: String,
    /// Service capabilities
    pub capabilities: Vec<String>,
    /// Service metadata
    pub metadata: HashMap<String, String>,
    /// Last seen timestamp
    pub last_seen: SystemTime,
    /// Service version
    pub version: String,
}

impl ServiceInfo {
    /// Create new service info
    pub fn new(service_id: String, name: String, address: String) -> Self {
        Self {
            service_id,
            name,
            address,
            capabilities: Vec::new(),
            metadata: HashMap::new(),
            last_seen: SystemTime::now(),
            version: "1.0.0".to_string(),
        }
    }

    /// Add capability
    pub fn add_capability(&mut self, capability: String) {
        if !self.capabilities.contains(&capability) {
            self.capabilities.push(capability);
        }
    }

    /// Add metadata
    pub fn add_metadata(&mut self, key: String, value: String) {
        self.metadata.insert(key, value);
    }

    /// Check if service has capability
    pub fn has_capability(&self, capability: &str) -> bool {
        self.capabilities.contains(&capability.to_string())
    }

    /// Get metadata value
    pub fn get_metadata(&self, key: &str) -> Option<&String> {
        self.metadata.get(key)
    }

    /// Update last seen timestamp
    pub fn update_last_seen(&mut self) {
        self.last_seen = SystemTime::now();
    }

    /// Check if service is stale (older than timeout)
    pub fn is_stale(&self, timeout: Duration) -> bool {
        SystemTime::now()
            .duration_since(self.last_seen)
            .unwrap_or(Duration::from_secs(0))
            > timeout
    }
}

/// Service discovery manager
pub struct ServiceDiscovery {
    /// Services registry
    services: Arc<RwLock<HashMap<String, ServiceInfo>>>,
    /// Service timeout (default: 60 seconds)
    timeout: Duration,
    /// Local service info
    local_service: Option<ServiceInfo>,
}

impl ServiceDiscovery {
    /// Create new service discovery manager
    pub fn new() -> Self {
        Self {
            services: Arc::new(RwLock::new(HashMap::new())),
            timeout: Duration::from_secs(60),
            local_service: None,
        }
    }

    /// Create with custom timeout
    pub fn with_timeout(timeout: Duration) -> Self {
        Self {
            services: Arc::new(RwLock::new(HashMap::new())),
            timeout,
            local_service: None,
        }
    }

    /// Register local service
    pub fn register_local(&mut self, service: ServiceInfo) {
        let service_id = service.service_id.clone();
        self.local_service = Some(service.clone());
        self.services.write().insert(service_id, service);
    }

    /// Register discovered service
    pub fn register_service(&self, service: ServiceInfo) {
        self.services
            .write()
            .insert(service.service_id.clone(), service);
    }

    /// Unregister service
    pub fn unregister_service(&self, service_id: &str) {
        self.services.write().remove(service_id);
    }

    /// Get service by ID
    pub fn get_service(&self, service_id: &str) -> Option<ServiceInfo> {
        self.services.read().get(service_id).cloned()
    }

    /// Get all services
    pub fn get_all_services(&self) -> Vec<ServiceInfo> {
        self.services.read().values().cloned().collect()
    }

    /// Find services by capability
    pub fn find_by_capability(&self, capability: &str) -> Vec<ServiceInfo> {
        self.services
            .read()
            .values()
            .filter(|s| s.has_capability(capability))
            .cloned()
            .collect()
    }

    /// Find services by metadata
    pub fn find_by_metadata(&self, key: &str, value: &str) -> Vec<ServiceInfo> {
        self.services
            .read()
            .values()
            .filter(|s| s.get_metadata(key).map(|v| v == value).unwrap_or(false))
            .cloned()
            .collect()
    }

    /// Find services by name
    pub fn find_by_name(&self, name: &str) -> Vec<ServiceInfo> {
        self.services
            .read()
            .values()
            .filter(|s| s.name == name)
            .cloned()
            .collect()
    }

    /// Update service last seen
    pub fn update_service(&self, service_id: &str) {
        if let Some(service) = self.services.write().get_mut(service_id) {
            service.update_last_seen();
        }
    }

    /// Remove stale services
    pub fn cleanup_stale(&self) -> usize {
        let timeout = self.timeout;
        let mut services = self.services.write();
        let before_count = services.len();

        services.retain(|_, service| !service.is_stale(timeout));

        before_count - services.len()
    }

    /// Get service count
    pub fn service_count(&self) -> usize {
        self.services.read().len()
    }

    /// Get local service info
    pub fn get_local_service(&self) -> Option<ServiceInfo> {
        self.local_service.clone()
    }

    /// Create discovery envelope (HELLO message)
    pub fn create_discovery_envelope(&self) -> Option<Envelope> {
        self.local_service.as_ref().map(|service| {
            let mut envelope = Envelope::builder()
                .from(&service.service_id)
                .to("broadcast")
                .operation(OperationType::Control)
                .message_id(&format!("discovery-{}", uuid::Uuid::new_v4()))
                .capability("type", "discovery")
                .capability("service_name", &service.name)
                .capability("service_address", &service.address)
                .capability("service_version", &service.version);

            // Add capabilities
            for cap in &service.capabilities {
                envelope = envelope.capability(&format!("cap:{}", cap), "true");
            }

            // Add metadata
            for (key, value) in &service.metadata {
                envelope = envelope.capability(&format!("meta:{}", key), value);
            }

            envelope.build().ok()
        })?
    }

    /// Parse discovery envelope
    pub fn parse_discovery_envelope(&self, envelope: &Envelope) -> Option<ServiceInfo> {
        let caps = envelope.capabilities()?;

        // Check if it's a discovery message
        if caps.get("type").map(|v| v == "discovery").unwrap_or(false) {
            let service_id = envelope.from().to_string();
            let name = caps.get("service_name").cloned().unwrap_or_default();
            let address = caps.get("service_address").cloned().unwrap_or_default();

            let mut service = ServiceInfo::new(service_id, name, address);

            // Parse version
            if let Some(version) = caps.get("service_version") {
                service.version = version.clone();
            }

            // Parse capabilities (cap:*)
            for (key, _value) in caps.iter() {
                if let Some(cap) = key.strip_prefix("cap:") {
                    service.add_capability(cap.to_string());
                }
            }

            // Parse metadata (meta:*)
            for (key, value) in caps.iter() {
                if let Some(meta_key) = key.strip_prefix("meta:") {
                    service.add_metadata(meta_key.to_string(), value.clone());
                }
            }

            Some(service)
        } else {
            None
        }
    }

    /// Auto-discover from peer (requires peer connection info)
    #[cfg(feature = "websocket")]
    pub fn discover_from_peer_info(&self, peer_info: &PeerInfo) -> ServiceInfo {
        let service = ServiceInfo {
            service_id: peer_info.id.clone(),
            name: peer_info
                .metadata
                .get("name")
                .cloned()
                .unwrap_or_else(|| peer_info.id.clone()),
            address: peer_info.url.clone().unwrap_or_default(),
            capabilities: peer_info.capabilities.clone(),
            metadata: peer_info.metadata.clone(),
            last_seen: SystemTime::now(),
            version: peer_info
                .metadata
                .get("version")
                .cloned()
                .unwrap_or_else(|| "1.0.0".to_string()),
        };

        self.register_service(service.clone());
        service
    }
}

impl Default for ServiceDiscovery {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_service_info_creation() {
        let service = ServiceInfo::new(
            "service-1".to_string(),
            "Test Service".to_string(),
            "ws://localhost:8080".to_string(),
        );

        assert_eq!(service.service_id, "service-1");
        assert_eq!(service.name, "Test Service");
        assert_eq!(service.address, "ws://localhost:8080");
    }

    #[test]
    fn test_service_capabilities() {
        let mut service = ServiceInfo::new(
            "service-1".to_string(),
            "Test".to_string(),
            "ws://localhost:8080".to_string(),
        );

        service.add_capability("storage".to_string());
        service.add_capability("compute".to_string());

        assert!(service.has_capability("storage"));
        assert!(service.has_capability("compute"));
        assert!(!service.has_capability("network"));
    }

    #[test]
    fn test_service_metadata() {
        let mut service = ServiceInfo::new(
            "service-1".to_string(),
            "Test".to_string(),
            "ws://localhost:8080".to_string(),
        );

        service.add_metadata("region".to_string(), "us-east-1".to_string());
        service.add_metadata("version".to_string(), "1.0.0".to_string());

        assert_eq!(service.get_metadata("region"), Some(&"us-east-1".to_string()));
        assert_eq!(service.get_metadata("version"), Some(&"1.0.0".to_string()));
        assert_eq!(service.get_metadata("unknown"), None);
    }

    #[test]
    fn test_service_discovery_creation() {
        let discovery = ServiceDiscovery::new();
        assert_eq!(discovery.service_count(), 0);
    }

    #[test]
    fn test_register_service() {
        let discovery = ServiceDiscovery::new();
        let service = ServiceInfo::new(
            "service-1".to_string(),
            "Test".to_string(),
            "ws://localhost:8080".to_string(),
        );

        discovery.register_service(service.clone());
        assert_eq!(discovery.service_count(), 1);

        let retrieved = discovery.get_service("service-1");
        assert!(retrieved.is_some());
        assert_eq!(retrieved.unwrap().name, "Test");
    }

    #[test]
    fn test_find_by_capability() {
        let discovery = ServiceDiscovery::new();

        let mut service1 = ServiceInfo::new(
            "service-1".to_string(),
            "Storage".to_string(),
            "ws://localhost:8080".to_string(),
        );
        service1.add_capability("storage".to_string());

        let mut service2 = ServiceInfo::new(
            "service-2".to_string(),
            "Compute".to_string(),
            "ws://localhost:8081".to_string(),
        );
        service2.add_capability("compute".to_string());

        discovery.register_service(service1);
        discovery.register_service(service2);

        let storage_services = discovery.find_by_capability("storage");
        assert_eq!(storage_services.len(), 1);
        assert_eq!(storage_services[0].name, "Storage");

        let compute_services = discovery.find_by_capability("compute");
        assert_eq!(compute_services.len(), 1);
        assert_eq!(compute_services[0].name, "Compute");
    }

    #[test]
    fn test_find_by_metadata() {
        let discovery = ServiceDiscovery::new();

        let mut service1 = ServiceInfo::new(
            "service-1".to_string(),
            "US Service".to_string(),
            "ws://localhost:8080".to_string(),
        );
        service1.add_metadata("region".to_string(), "us-east-1".to_string());

        let mut service2 = ServiceInfo::new(
            "service-2".to_string(),
            "EU Service".to_string(),
            "ws://localhost:8081".to_string(),
        );
        service2.add_metadata("region".to_string(), "eu-west-1".to_string());

        discovery.register_service(service1);
        discovery.register_service(service2);

        let us_services = discovery.find_by_metadata("region", "us-east-1");
        assert_eq!(us_services.len(), 1);
        assert_eq!(us_services[0].name, "US Service");
    }

    #[test]
    fn test_unregister_service() {
        let discovery = ServiceDiscovery::new();
        let service = ServiceInfo::new(
            "service-1".to_string(),
            "Test".to_string(),
            "ws://localhost:8080".to_string(),
        );

        discovery.register_service(service);
        assert_eq!(discovery.service_count(), 1);

        discovery.unregister_service("service-1");
        assert_eq!(discovery.service_count(), 0);
    }

    #[test]
    fn test_service_stale_detection() {
        let mut service = ServiceInfo::new(
            "service-1".to_string(),
            "Test".to_string(),
            "ws://localhost:8080".to_string(),
        );

        // Service just created is not stale
        assert!(!service.is_stale(Duration::from_secs(60)));

        // Manually set old timestamp (in real scenario this would be old)
        service.last_seen = SystemTime::now()
            .checked_sub(Duration::from_secs(120))
            .unwrap();

        // Now it should be stale (older than 60 seconds)
        assert!(service.is_stale(Duration::from_secs(60)));
    }

    #[test]
    fn test_cleanup_stale() {
        let discovery = ServiceDiscovery::with_timeout(Duration::from_millis(100));

        let mut service = ServiceInfo::new(
            "service-1".to_string(),
            "Test".to_string(),
            "ws://localhost:8080".to_string(),
        );

        // Make service stale
        service.last_seen = SystemTime::now()
            .checked_sub(Duration::from_secs(120))
            .unwrap();

        discovery.register_service(service);
        assert_eq!(discovery.service_count(), 1);

        // Cleanup should remove stale service
        let removed = discovery.cleanup_stale();
        assert_eq!(removed, 1);
        assert_eq!(discovery.service_count(), 0);
    }
}

