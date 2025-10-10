/*!
# Service Discovery Example

Demonstrates automatic service discovery in a peer network.
*/

use umicp_core::{Envelope, OperationType, ServiceDiscovery, ServiceInfo};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("=== UMICP Service Discovery Example ===\n");

    // Create service discovery manager
    let discovery = ServiceDiscovery::new();

    // Register local service
    let mut local_service = ServiceInfo::new(
        "service-001".to_string(),
        "Storage Service".to_string(),
        "ws://localhost:8080".to_string(),
    );

    local_service.add_capability("storage".to_string());
    local_service.add_capability("cache".to_string());
    local_service.add_metadata("region".to_string(), "us-east-1".to_string());
    local_service.add_metadata("version".to_string(), "1.0.0".to_string());

    discovery.register_service(local_service.clone());
    println!("✓ Registered local service: {}", local_service.name);

    // Register more services
    let mut compute_service = ServiceInfo::new(
        "service-002".to_string(),
        "Compute Service".to_string(),
        "ws://localhost:8081".to_string(),
    );
    compute_service.add_capability("compute".to_string());
    compute_service.add_capability("ml".to_string());
    compute_service.add_metadata("region".to_string(), "us-west-1".to_string());

    discovery.register_service(compute_service.clone());
    println!("✓ Registered compute service: {}", compute_service.name);

    // Find services by capability
    println!("\n--- Find by Capability ---");
    let storage_services = discovery.find_by_capability("storage");
    println!("Services with 'storage' capability: {}", storage_services.len());
    for service in storage_services {
        println!("  - {} at {}", service.name, service.address);
    }

    let compute_services = discovery.find_by_capability("compute");
    println!("Services with 'compute' capability: {}", compute_services.len());
    for service in compute_services {
        println!("  - {} at {}", service.name, service.address);
    }

    // Find services by metadata
    println!("\n--- Find by Metadata ---");
    let us_east_services = discovery.find_by_metadata("region", "us-east-1");
    println!("Services in us-east-1: {}", us_east_services.len());
    for service in us_east_services {
        println!("  - {} ({:?})", service.name, service.capabilities);
    }

    // Create discovery envelope
    println!("\n--- Discovery Protocol ---");
    if let Some(envelope) = discovery.create_discovery_envelope() {
        println!("Discovery envelope created:");
        println!("  From: {}", envelope.from());
        println!("  To: {}", envelope.to());
        println!("  Type: {:?}", envelope.operation());
        println!("  Capabilities: {}", envelope.capabilities().len());
    }

    // Simulate receiving a discovery message
    let discovery_envelope = Envelope::builder()
        .from("service-003")
        .to("broadcast")
        .operation(OperationType::Control)
        .message_id("discovery-123")
        .capability("type", "discovery")
        .capability("service_name", "Network Service")
        .capability("service_address", "ws://localhost:8082")
        .capability("service_version", "1.0.0")
        .capability("cap:network", "true")
        .capability("cap:routing", "true")
        .capability("meta:region", "eu-west-1")
        .build()?;

    if let Some(discovered) = discovery.parse_discovery_envelope(&discovery_envelope) {
        println!("\n✓ Discovered new service:");
        println!("  Service ID: {}", discovered.service_id);
        println!("  Name: {}", discovered.name);
        println!("  Address: {}", discovered.address);
        println!("  Capabilities: {:?}", discovered.capabilities);
        println!("  Metadata: {:?}", discovered.metadata);

        discovery.register_service(discovered);
    }

    // Statistics
    println!("\n--- Statistics ---");
    println!("Total services: {}", discovery.service_count());
    println!("All services:");
    for service in discovery.get_all_services() {
        println!("  - {} ({}) - {:?}", service.name, service.address, service.capabilities);
    }

    Ok(())
}

