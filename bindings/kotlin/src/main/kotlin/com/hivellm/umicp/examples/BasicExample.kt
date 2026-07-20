package com.hivellm.umicp.examples

import com.hivellm.umicp.core.Envelope
import com.hivellm.umicp.core.PayloadHint
import com.hivellm.umicp.core.UMICP
import com.hivellm.umicp.types.EncodingType
import com.hivellm.umicp.types.OperationType
import com.hivellm.umicp.types.PayloadType

/**
 * Basic UMICP Example
 *
 * Demonstrates:
 * - Creating envelopes
 * - Serialization/deserialization
 * - Validation
 * - Hash computation
 */
fun main() {
    println("=== UMICP Kotlin SDK - Basic Example ===\n")

    // Print version information
    UMICP.printVersion()
    println()

    // Create an envelope with builder pattern
    println("1. Creating envelope with builder pattern:")
    val envelope = Envelope.builder()
        .from("client-001")
        .to("server-001")
        .operation(OperationType.DATA)
        .capability("priority", "high")
        .capability("version", "1.0")
        .build()

    println("  From: ${envelope.from}")
    println("  To: ${envelope.to}")
    println("  Operation: ${envelope.operation}")
    println("  Message ID: ${envelope.messageId}")
    println("  Capabilities: ${envelope.capabilities}")
    println()

    // Create envelope with DSL
    println("2. Creating envelope with DSL:")
    val dslEnvelope = Envelope.build {
        from("ml-client")
        to("ml-server")
        operation(OperationType.REQUEST)
        capability("model", "gpt-4")
        payloadHint(
            PayloadHint.build {
                type(PayloadType.VECTOR)
                encoding(EncodingType.FLOAT32)
                count(768)
            }
        )
    }
    println("  Envelope created with payload hint")
    println("  Payload type: ${dslEnvelope.payloadHint?.type}")
    println("  Payload count: ${dslEnvelope.payloadHint?.count}")
    println()

    // Validate envelope
    println("3. Validating envelope:")
    val isValid = envelope.validate()
    println("  Valid: $isValid")

    try {
        envelope.validateOrThrow()
        println("  Validation passed!")
    } catch (e: Exception) {
        println("  Validation failed: ${e.message}")
    }
    println()

    // Serialize to JSON
    println("4. Serializing to JSON:")
    val json = envelope.serialize()
    println("  JSON: ${json.take(100)}...")
    println()

    // Deserialize from JSON
    println("5. Deserializing from JSON:")
    val deserialized = Envelope.deserialize(json)
    println("  From: ${deserialized.from}")
    println("  To: ${deserialized.to}")
    println("  Match: ${deserialized.from == envelope.from}")
    println()

    // Compute hash
    println("6. Computing hash:")
    val hash = envelope.hash()
    println("  Hash: $hash")
    println("  Hash length: ${hash.length} (SHA-256)")
    println()

    // Using UMICP facade
    println("7. Using UMICP facade:")
    val facadeEnvelope = UMICP.createEnvelope {
        from("facade-client")
        to("facade-server")
        operation(OperationType.DATA)
    }

    println("  Envelope created via facade")
    println("  Serialized: ${UMICP.serialize(facadeEnvelope).take(50)}...")
    println("  Valid: ${UMICP.validate(facadeEnvelope)}")
    println("  Hash: ${UMICP.hash(facadeEnvelope).take(16)}...")
    println()

    println("=== Example Complete ===")
}

