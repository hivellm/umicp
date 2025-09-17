/*!
# Basic Envelope Example

This example demonstrates basic usage of UMICP envelopes for message creation,
serialization, and deserialization.
*/

use umicp_core::{Envelope, OperationType};
use uuid;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("UMICP Rust Bindings - Basic Envelope Example");
    println!("=============================================\n");

    // Create a basic envelope
    println!("1. Creating a basic envelope...");
    let envelope = Envelope::builder()
        .from("client-001")
        .to("server-001")
        .operation(OperationType::Data)
        .message_id(&uuid::Uuid::new_v4().to_string())
        .capability("content-type", "application/json")
        .capability("priority", "high")
        .build()?;

    println!("   From: {}", envelope.from());
    println!("   To: {}", envelope.to());
    println!("   Operation: {:?}", envelope.operation());
    println!("   Message ID: {}", envelope.message_id());
    println!("   Timestamp: {}", envelope.timestamp());

    if let Some(capabilities) = envelope.capabilities() {
        println!("   Capabilities:");
        for (key, value) in capabilities {
            println!("     {}: {}", key, value);
        }
    }

    println!();

    // Serialize the envelope
    println!("2. Serializing envelope...");
    let serialized = envelope.serialize()?;
    println!("   Serialized JSON (first 200 chars): {}...",
             &serialized[..std::cmp::min(200, serialized.len())]);
    println!("   Total length: {} bytes", serialized.len());
    println!();

    // Deserialize the envelope
    println!("3. Deserializing envelope...");
    let deserialized = Envelope::deserialize(&serialized)?;
    println!("   Successfully deserialized!");
    println!("   From: {}", deserialized.from());
    println!("   To: {}", deserialized.to());
    println!("   Operation: {:?}", deserialized.operation());
    println!();

    // Generate envelope hash
    println!("4. Generating envelope hash...");
    let hash = envelope.hash()?;
    println!("   Hash: {}", hash);
    println!();

    // Validate envelope
    println!("5. Validating envelope...");
    match envelope.validate() {
        Ok(_) => println!("   ✓ Envelope is valid"),
        Err(e) => println!("   ✗ Envelope validation failed: {}", e),
    }
    println!();

    // Create envelope with different operation types
    println!("6. Creating envelopes with different operations...");
    let operations = vec![
        OperationType::Control,
        OperationType::Data,
        OperationType::Ack,
        OperationType::Error,
        OperationType::Request,
        OperationType::Response,
    ];

    for op in operations {
        let op_envelope = Envelope::builder()
            .from("example-client")
            .to("example-server")
            .operation(op)
            .message_id(&uuid::Uuid::new_v4().to_string())
            .capability("example", "true")
            .build()?;

        println!("   Operation: {:?} -> Hash: {}",
                op, &op_envelope.hash()?[..16]);
    }

    println!("\n✓ Basic envelope example completed successfully!");

    Ok(())
}
