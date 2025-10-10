package com.hivellm.umicp.core;

import com.hivellm.umicp.types.EncodingType;
import com.hivellm.umicp.types.OperationType;
import com.hivellm.umicp.types.PayloadType;
import com.hivellm.umicp.types.SerializationException;
import com.hivellm.umicp.types.ValidationException;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link Envelope}.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 */
class EnvelopeTest {

    @Test
    @DisplayName("Should create empty envelope")
    void testEmptyEnvelope() {
        try (Envelope envelope = new Envelope()) {
            assertThat(envelope.getFrom()).isNull();
            assertThat(envelope.getTo()).isNull();
            assertThat(envelope.getOperation()).isNull();
            assertThat(envelope.getMessageId()).isNull();
            assertThat(envelope.getCapabilities()).isEmpty();
            assertThat(envelope.getPayloadHint()).isNull();
        }
    }

    @Test
    @DisplayName("Should create envelope from options")
    void testEnvelopeFromOptions() {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("client-001")
            .to("server-001")
            .operation(OperationType.DATA)
            .messageId("msg-12345")
            .capability("priority", "high")
            .build();

        try (Envelope envelope = new Envelope(options)) {
            assertThat(envelope.getFrom()).isEqualTo("client-001");
            assertThat(envelope.getTo()).isEqualTo("server-001");
            assertThat(envelope.getOperation()).isEqualTo(OperationType.DATA);
            assertThat(envelope.getMessageId()).isEqualTo("msg-12345");
            assertThat(envelope.getCapabilities()).containsEntry("priority", "high");
        }
    }

    @Test
    @DisplayName("Should support fluent setter API")
    void testFluentSetters() {
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client")
                   .setTo("server")
                   .setOperation(OperationType.REQUEST)
                   .setMessageId("msg-001")
                   .addCapability("test", "value");

            assertThat(envelope.getFrom()).isEqualTo("client");
            assertThat(envelope.getTo()).isEqualTo("server");
            assertThat(envelope.getOperation()).isEqualTo(OperationType.REQUEST);
            assertThat(envelope.getMessageId()).isEqualTo("msg-001");
            assertThat(envelope.getCapabilities()).containsEntry("test", "value");
        }
    }

    @Test
    @DisplayName("Should serialize to JSON")
    void testSerialization() throws SerializationException {
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client")
                   .setTo("server")
                   .setOperation(OperationType.DATA)
                   .setMessageId("msg-001");

            String json = envelope.serialize();

            assertThat(json).isNotNull();
            assertThat(json).contains("\"from\":\"client\"");
            assertThat(json).contains("\"to\":\"server\"");
            assertThat(json).contains("\"operation\":1"); // DATA = 1
            assertThat(json).contains("\"messageId\":\"msg-001\"");
        }
    }

    @Test
    @DisplayName("Should deserialize from JSON")
    void testDeserialization() throws SerializationException {
        String json = "{\"from\":\"client\",\"to\":\"server\",\"operation\":1,\"messageId\":\"msg-001\"}";

        Envelope envelope = Envelope.deserialize(json);

        assertThat(envelope.getFrom()).isEqualTo("client");
        assertThat(envelope.getTo()).isEqualTo("server");
        assertThat(envelope.getOperation()).isEqualTo(OperationType.DATA);
        assertThat(envelope.getMessageId()).isEqualTo("msg-001");
    }

    @Test
    @DisplayName("Should round-trip serialize and deserialize")
    void testRoundTrip() throws SerializationException {
        try (Envelope original = new Envelope()) {
            original.setFrom("client-001")
                   .setTo("server-001")
                   .setOperation(OperationType.REQUEST)
                   .setMessageId(UUID.randomUUID().toString())
                   .addCapability("version", "1.0")
                   .addCapability("encoding", "utf-8");

            String json = original.serialize();
            Envelope deserialized = Envelope.deserialize(json);

            assertThat(deserialized.getFrom()).isEqualTo(original.getFrom());
            assertThat(deserialized.getTo()).isEqualTo(original.getTo());
            assertThat(deserialized.getOperation()).isEqualTo(original.getOperation());
            assertThat(deserialized.getMessageId()).isEqualTo(original.getMessageId());
            assertThat(deserialized.getCapabilities()).isEqualTo(original.getCapabilities());
        }
    }

    @Test
    @DisplayName("Should validate valid envelope")
    void testValidation() {
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client")
                   .setTo("server")
                   .setOperation(OperationType.DATA);

            assertThat(envelope.validate()).isTrue();
        }
    }

    @Test
    @DisplayName("Should reject envelope without from")
    void testValidationMissingFrom() {
        try (Envelope envelope = new Envelope()) {
            envelope.setTo("server")
                   .setOperation(OperationType.DATA);

            assertThat(envelope.validate()).isFalse();
        }
    }

    @Test
    @DisplayName("Should reject envelope without to")
    void testValidationMissingTo() {
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client")
                   .setOperation(OperationType.DATA);

            assertThat(envelope.validate()).isFalse();
        }
    }

    @Test
    @DisplayName("Should reject envelope without operation")
    void testValidationMissingOperation() {
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client")
                   .setTo("server");

            assertThat(envelope.validate()).isFalse();
        }
    }

    @Test
    @DisplayName("Should reject envelope with empty from")
    void testValidationEmptyFrom() {
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("")
                   .setTo("server")
                   .setOperation(OperationType.DATA);

            assertThat(envelope.validate()).isFalse();
        }
    }

    @Test
    @DisplayName("Should reject envelope with whitespace-only from")
    void testValidationWhitespaceFrom() {
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("   ")
                   .setTo("server")
                   .setOperation(OperationType.DATA);

            assertThat(envelope.validate()).isFalse();
        }
    }

    @Test
    @DisplayName("Should validate or throw exception")
    void testValidateOrThrow() {
        try (Envelope validEnvelope = new Envelope()) {
            validEnvelope.setFrom("client")
                        .setTo("server")
                        .setOperation(OperationType.DATA);

            assertThatCode(() -> validEnvelope.validateOrThrow())
                .doesNotThrowAnyException();
        }

        try (Envelope invalidEnvelope = new Envelope()) {
            invalidEnvelope.setFrom("client");
            // Missing 'to' and 'operation'

            assertThatThrownBy(() -> invalidEnvelope.validateOrThrow())
                .isInstanceOf(ValidationException.class)
                .hasMessageContaining("'to' field is required");
        }
    }

    @Test
    @DisplayName("Should compute hash")
    void testHash() throws SerializationException {
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client")
                   .setTo("server")
                   .setOperation(OperationType.DATA)
                   .setMessageId("msg-001");

            String hash = envelope.getHash();

            assertThat(hash).isNotNull();
            assertThat(hash).hasSize(64); // SHA-256 produces 64 hex characters
            assertThat(hash).matches("[0-9a-f]{64}");
        }
    }

    @Test
    @DisplayName("Should cache hash when envelope unchanged")
    void testHashCaching() throws SerializationException {
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client")
                   .setTo("server")
                   .setOperation(OperationType.DATA);

            String hash1 = envelope.getHash();
            String hash2 = envelope.getHash();

            assertThat(hash1).isEqualTo(hash2);
        }
    }

    @Test
    @DisplayName("Should invalidate hash cache when envelope modified")
    void testHashInvalidation() throws SerializationException {
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client")
                   .setTo("server")
                   .setOperation(OperationType.DATA);

            String hash1 = envelope.getHash();

            envelope.setMessageId("msg-001");

            String hash2 = envelope.getHash();

            assertThat(hash1).isNotEqualTo(hash2);
        }
    }

    @Test
    @DisplayName("Should manage capabilities")
    void testCapabilitiesManagement() {
        try (Envelope envelope = new Envelope()) {
            // Add capabilities
            envelope.addCapability("key1", "value1");
            envelope.addCapability("key2", "value2");

            assertThat(envelope.getCapabilities())
                .hasSize(2)
                .containsEntry("key1", "value1")
                .containsEntry("key2", "value2");

            // Remove capability
            envelope.removeCapability("key1");

            assertThat(envelope.getCapabilities())
                .hasSize(1)
                .doesNotContainKey("key1");

            // Set all capabilities
            Map<String, String> newCaps = new HashMap<>();
            newCaps.put("key3", "value3");
            envelope.setCapabilities(newCaps);

            assertThat(envelope.getCapabilities())
                .hasSize(1)
                .containsEntry("key3", "value3");
        }
    }

    @Test
    @DisplayName("Should handle payload hint")
    void testPayloadHint() throws SerializationException {
        PayloadHint hint = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT32)
            .count(768)
            .build();

        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client")
                   .setTo("server")
                   .setOperation(OperationType.DATA)
                   .setPayloadHint(hint);

            assertThat(envelope.getPayloadHint()).isEqualTo(hint);

            // Serialize and deserialize
            String json = envelope.serialize();
            Envelope deserialized = Envelope.deserialize(json);

            assertThat(deserialized.getPayloadHint()).isNotNull();
            assertThat(deserialized.getPayloadHint().getType()).isEqualTo(PayloadType.VECTOR);
            assertThat(deserialized.getPayloadHint().getEncoding()).isEqualTo(EncodingType.FLOAT32);
            assertThat(deserialized.getPayloadHint().getCount()).isEqualTo(768);
        }
    }

    @Test
    @DisplayName("Should validate payload hint")
    void testPayloadHintValidation() {
        // Invalid payload hint
        PayloadHint invalidHint = PayloadHint.builder()
            .type(PayloadType.TEXT)
            .encoding(EncodingType.FLOAT32) // Encoding with non-VECTOR type
            .build();

        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client")
                   .setTo("server")
                   .setOperation(OperationType.DATA)
                   .setPayloadHint(invalidHint);

            assertThat(envelope.validate()).isFalse();
        }
    }

    @Test
    @DisplayName("Should support equals and hashCode")
    void testEqualsAndHashCode() {
        try (Envelope envelope1 = new Envelope();
             Envelope envelope2 = new Envelope();
             Envelope envelope3 = new Envelope()) {

            envelope1.setFrom("client")
                    .setTo("server")
                    .setOperation(OperationType.DATA)
                    .setMessageId("msg-001");

            envelope2.setFrom("client")
                    .setTo("server")
                    .setOperation(OperationType.DATA)
                    .setMessageId("msg-001");

            envelope3.setFrom("client")
                    .setTo("server")
                    .setOperation(OperationType.REQUEST)
                    .setMessageId("msg-002");

            assertThat(envelope1).isEqualTo(envelope2);
            assertThat(envelope1.hashCode()).isEqualTo(envelope2.hashCode());
            assertThat(envelope1).isNotEqualTo(envelope3);
        }
    }

    @Test
    @DisplayName("Should have meaningful toString")
    void testToString() {
        try (Envelope envelope = new Envelope()) {
            envelope.setFrom("client")
                   .setTo("server")
                   .setOperation(OperationType.DATA);

            String str = envelope.toString();
            assertThat(str).contains("Envelope");
            assertThat(str).contains("client");
            assertThat(str).contains("server");
            assertThat(str).contains("DATA");
        }
    }

    @Test
    @DisplayName("Should handle all operation types")
    void testAllOperationTypes() throws SerializationException {
        for (OperationType opType : OperationType.values()) {
            try (Envelope envelope = new Envelope()) {
                envelope.setFrom("client")
                       .setTo("server")
                       .setOperation(opType);

                // Serialize and deserialize
                String json = envelope.serialize();
                Envelope deserialized = Envelope.deserialize(json);

                assertThat(deserialized.getOperation()).isEqualTo(opType);
            }
        }
    }

    @Test
    @DisplayName("Should throw SerializationException on invalid JSON")
    void testDeserializationError() {
        String invalidJson = "{invalid json}";

        assertThatThrownBy(() -> Envelope.deserialize(invalidJson))
            .isInstanceOf(SerializationException.class)
            .hasMessageContaining("Failed to deserialize envelope");
    }

    @Test
    @DisplayName("Should return defensive copy of capabilities")
    void testDefensiveCopy() {
        try (Envelope envelope = new Envelope()) {
            envelope.addCapability("key", "value");

            Map<String, String> caps = envelope.getCapabilities();
            caps.put("new-key", "new-value");

            // Original should not be affected
            assertThat(envelope.getCapabilities()).hasSize(1);
            assertThat(envelope.getCapabilities()).doesNotContainKey("new-key");
        }
    }
}

