#!/usr/bin/env python3
"""Basic envelope usage example."""

from umicp import Envelope, EnvelopeBuilder, OperationType


def main():
    """Demonstrate basic envelope operations."""
    print("=== UMICP Python - Basic Envelope Example ===\n")

    # Create envelope using builder
    print("1. Creating envelope...")
    envelope = EnvelopeBuilder() \
        .from_id("client-001") \
        .to_id("server-001") \
        .operation(OperationType.DATA) \
        .message_id("msg-12345") \
        .capability("content-type", "application/json") \
        .capability("priority", "high") \
        .build()

    print(f"   Created: {envelope}")
    print(f"   Message ID: {envelope.message_id}")
    print(f"   Hash: {envelope.hash_value}")

    # Serialize to JSON
    print("\n2. Serializing...")
    json_str = envelope.to_json()
    print(f"   JSON length: {len(json_str)} bytes")
    print(f"   JSON preview: {json_str[:100]}...")

    # Deserialize from JSON
    print("\n3. Deserializing...")
    received = Envelope.from_json(json_str)
    print(f"   From: {received.from_id}")
    print(f"   To: {received.to_id}")
    print(f"   Operation: {received.operation.value}")
    print(f"   Capabilities: {received.capabilities}")

    # Verify hash
    print("\n4. Verifying hash...")
    computed_hash = received.compute_hash()
    print(f"   Original hash: {envelope.hash_value}")
    print(f"   Computed hash: {computed_hash}")
    print(f"   Valid: {envelope.hash_value == computed_hash}")

    print("\n✅ Example completed successfully!")


if __name__ == "__main__":
    main()

