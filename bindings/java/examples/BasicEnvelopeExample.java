package com.hivellm.umicp.examples;

import com.hivellm.umicp.core.Envelope;
import com.hivellm.umicp.core.EnvelopeOptions;
import com.hivellm.umicp.core.UMICP;
import com.hivellm.umicp.types.OperationType;
import com.hivellm.umicp.types.SerializationException;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * Basic example demonstrating UMICP envelope usage in Java.
 *
 * This example shows how to:
 * - Create envelopes using builder pattern
 * - Serialize and deserialize envelopes
 * - Validate envelopes
 * - Work with capabilities
 *
 * NOTE: This is a planned example. Implementation is not yet complete.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
public class BasicEnvelopeExample {

    public static void main(String[] args) {
        System.out.println("=== UMICP Basic Envelope Example ===\n");

        try {
            // Example 1: Create simple envelope
            simpleEnvelopeExample();

            // Example 2: Create envelope with builder pattern
            builderPatternExample();

            // Example 3: Serialization and deserialization
            serializationExample();

            // Example 4: Working with capabilities
            capabilitiesExample();

            // Example 5: Envelope validation
            validationExample();

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }

    /**
     * Simple envelope creation example.
     */
    private static void simpleEnvelopeExample() {
        System.out.println("1. Simple Envelope Creation");
        System.out.println("----------------------------");

        // Create envelope using constructor
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client-001")
                   .setTo("server-001")
                   .setOperation(OperationType.DATA)
                   .setMessageId(UUID.randomUUID().toString());

            System.out.println("From: " + envelope.getFrom());
            System.out.println("To: " + envelope.getTo());
            System.out.println("Operation: " + envelope.getOperation());
            System.out.println("Message ID: " + envelope.getMessageId());
            System.out.println();
        }
    }

    /**
     * Builder pattern example.
     */
    private static void builderPatternExample() {
        System.out.println("2. Builder Pattern");
        System.out.println("------------------");

        // Create envelope using builder
        Map<String, String> capabilities = new HashMap<>();
        capabilities.put("content-type", "application/json");
        capabilities.put("priority", "high");
        capabilities.put("timestamp", String.valueOf(System.currentTimeMillis()));

        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("client-002")
            .to("server-002")
            .operation(OperationType.REQUEST)
            .messageId(UUID.randomUUID().toString())
            .capabilities(capabilities)
            .build();

        try (Envelope envelope = UMICP.createEnvelope(options)) {
            System.out.println("Envelope created with builder");
            System.out.println("Capabilities: " + envelope.getCapabilities());
            System.out.println();
        }
    }

    /**
     * Serialization and deserialization example.
     */
    private static void serializationExample() throws SerializationException {
        System.out.println("3. Serialization & Deserialization");
        System.out.println("----------------------------------");

        // Create envelope
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("sender")
                   .setTo("receiver")
                   .setOperation(OperationType.DATA)
                   .setMessageId("msg-12345");

            // Serialize to JSON
            String json = envelope.serialize();
            System.out.println("Serialized JSON:");
            System.out.println(json);
            System.out.println();

            // Deserialize from JSON
            Envelope deserialized = UMICP.deserialize(json);
            System.out.println("Deserialized envelope:");
            System.out.println("From: " + deserialized.getFrom());
            System.out.println("To: " + deserialized.getTo());
            System.out.println("Operation: " + deserialized.getOperation());
            System.out.println();
        }
    }

    /**
     * Working with capabilities example.
     */
    private static void capabilitiesExample() {
        System.out.println("4. Working with Capabilities");
        System.out.println("-----------------------------");

        try (Envelope envelope = new Envelope()) {
            // Set basic fields
            envelope.setFrom("iot-sensor-001")
                   .setTo("data-collector")
                   .setOperation(OperationType.DATA)
                   .setMessageId(UUID.randomUUID().toString());

            // Add capabilities
            Map<String, String> capabilities = new HashMap<>();
            capabilities.put("sensor-type", "temperature");
            capabilities.put("value", "23.5");
            capabilities.put("unit", "celsius");
            capabilities.put("location", "warehouse-a");
            capabilities.put("timestamp", String.valueOf(System.currentTimeMillis()));

            envelope.setCapabilities(capabilities);

            // Retrieve capabilities
            Map<String, String> retrievedCaps = envelope.getCapabilities();
            System.out.println("Sensor Data:");
            retrievedCaps.forEach((key, value) ->
                System.out.println("  " + key + ": " + value)
            );
            System.out.println();
        }
    }

    /**
     * Envelope validation example.
     */
    private static void validationExample() {
        System.out.println("5. Envelope Validation");
        System.out.println("----------------------");

        // Valid envelope
        try (Envelope validEnvelope = new Envelope()) {
            validEnvelope.setFrom("client")
                        .setTo("server")
                        .setOperation(OperationType.DATA)
                        .setMessageId("valid-001");

            boolean isValid = validEnvelope.validate();
            System.out.println("Valid envelope: " + isValid);

            // Get hash
            String hash = validEnvelope.getHash();
            System.out.println("Envelope hash: " + hash);
        }

        // Invalid envelope (missing required fields)
        try (Envelope invalidEnvelope = new Envelope()) {
            invalidEnvelope.setFrom("client");
            // Missing 'to' and 'operation'

            boolean isValid = invalidEnvelope.validate();
            System.out.println("Invalid envelope: " + isValid);
            System.out.println();
        }
    }
}

