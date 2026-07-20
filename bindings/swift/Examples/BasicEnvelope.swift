import Foundation
import UMICP

// Example 1: Basic Envelope Creation
func basicEnvelopeExample() throws {
    print("=== Basic Envelope Example ===\n")

    // Create a simple envelope
    let envelope = try UMICP.builder()
        .from("client-001")
        .to("server-001")
        .operation(.data)
        .messageId("msg-\(UUID().uuidString)")
        .capabilityString("content-type", value: "application/json")
        .buildWithHash()

    print("Envelope created:")
    print("  From: \(envelope.from)")
    print("  To: \(envelope.to)")
    print("  Operation: \(envelope.operation.description)")
    print("  Message ID: \(envelope.messageId)")
    print("  Hash: \(envelope.hash ?? "none")")

    // Serialize to JSON
    let json = try envelope.serialize()
    print("\nSerialized JSON:")
    print(json)

    // Deserialize back
    let restored = try Envelope.deserialize(from: json)
    print("\nDeserialized envelope:")
    print("  From: \(restored.from)")
    print("  Hash matches: \(try restored.verifyHash())")
}

// Example 2: Native JSON Types in Capabilities
func nativeTypesExample() throws {
    print("\n=== Native Types Example ===\n")

    let envelope = try UMICP.builder()
        .from("client-002")
        .to("server-002")
        .operation(.request)
        .messageId("req-\(UUID().uuidString)")
        .capabilityString("action", value: "search")
        .capabilityInt("max_results", value: 100)
        .capabilityBool("streaming", value: true)
        .capabilityDouble("temperature", value: 0.7)
        .capability("filters", value: ["category": "tech", "year": 2024])
        .capability("tags", value: ["ai", "ml", "llm"])
        .buildWithHash()

    print("Envelope with native types:")
    print("  String capability: \(envelope.getCapability("action") ?? "none")")
    print("  Int capability: \(envelope.getCapability("max_results") ?? 0)")
    print("  Bool capability: \(envelope.getCapability("streaming") ?? false)")
    print("  Double capability: \(envelope.getCapability("temperature") ?? 0.0)")
    print("  Dict capability: \(envelope.getCapability("filters") ?? [:])")
    print("  Array capability: \(envelope.getCapability("tags") ?? [])")
}

// Example 3: Envelope with Payload
func payloadExample() throws {
    print("\n=== Payload Example ===\n")

    let payloadData = "Hello UMICP from Swift!".data(using: .utf8)!

    let envelope = try UMICP.builder()
        .from("client-003")
        .to("server-003")
        .operation(.data)
        .messageId("data-\(UUID().uuidString)")
        .payload(payloadData)
        .buildWithHash()

    print("Envelope with payload:")
    print("  Payload size: \(envelope.payload?.count ?? 0) bytes")
    if let data = envelope.payload, let text = String(data: data, encoding: .utf8) {
        print("  Payload content: \(text)")
    }
}

// Run all examples
do {
    try basicEnvelopeExample()
    try nativeTypesExample()
    try payloadExample()
    print("\n✅ All examples completed successfully!")
} catch {
    print("❌ Error: \(error)")
}

