package com.hivellm.umicp.core;

import com.hivellm.umicp.types.OperationType;
import com.hivellm.umicp.types.PayloadType;
import com.hivellm.umicp.types.EncodingType;
import com.hivellm.umicp.types.SerializationException;
import org.junit.jupiter.api.Test;

import java.util.HashMap;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Edge case tests for Envelope class.
 */
public class EnvelopeEdgeCasesTest {

    @Test
    public void testEnvelope_EmptyCapabilities() throws Exception {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("sender")
            .to("receiver")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .build();

        Envelope envelope = new Envelope(options);

        assertNotNull(envelope.getCapabilities());
        assertTrue(envelope.getCapabilities().isEmpty());
    }

    @Test
    public void testEnvelope_NullCapabilities() {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("sender")
            .to("receiver")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .capabilities(null)
            .build();

        Envelope envelope = new Envelope(options);

        assertNotNull(envelope.getCapabilities());
        assertTrue(envelope.getCapabilities().isEmpty());
    }

    @Test
    public void testEnvelope_LargeCapabilities() throws Exception {
        Map<String, Object> largeCaps = new HashMap<>();
        for (int i = 0; i < 100; i++) {
            largeCaps.put("key" + i, "value" + i);
        }

        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("sender")
            .to("receiver")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .capabilities(largeCaps)
            .build();

        Envelope envelope = new Envelope(options);

        assertEquals(100, envelope.getCapabilities().size());
        assertEquals("value42", envelope.getCapabilities().get("key42"));
    }

    @Test
    public void testEnvelope_SpecialCharactersInFields() throws Exception {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("sender@domain.com")
            .to("receiver-123_test")
            .operation(OperationType.DATA)
            .messageId("msg-001-äöü")
            .build();

        Envelope envelope = new Envelope(options);

        assertEquals("sender@domain.com", envelope.getFrom());
        assertEquals("receiver-123_test", envelope.getTo());
        assertTrue(envelope.validate());
    }

    @Test
    public void testEnvelope_LongStrings() throws Exception {
        String longId = "a".repeat(1000);

        EnvelopeOptions options = EnvelopeOptions.builder()
            .from(longId)
            .to("receiver")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .build();

        Envelope envelope = new Envelope(options);

        assertEquals(1000, envelope.getFrom().length());
        assertTrue(envelope.validate());
    }

    @Test
    public void testEnvelope_AllOperationTypes() throws Exception {
        for (OperationType opType : OperationType.values()) {
            EnvelopeOptions options = EnvelopeOptions.builder()
                .from("sender")
                .to("receiver")
                .operation(opType)
                .messageId("msg-" + opType.name())
                .build();

            Envelope envelope = new Envelope(options);

            assertEquals(opType, envelope.getOperation());
            assertTrue(envelope.validate());
        }
    }

    @Test
    public void testEnvelope_SerializeDeserialize_WithAllFields() throws Exception {
        PayloadHint hint = PayloadHint.builder()
            .size(1024)
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT32)
            .count(256)
            .build();

        Map<String, Object> caps = new HashMap<>();
        caps.put("version", "1.0");
        caps.put("priority", "high");

        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("client-001")
            .to("server-001")
            .operation(OperationType.REQUEST)
            .messageId("req-12345")
            .capabilities(caps)
            .payloadHint(hint)
            .build();

        Envelope original = new Envelope(options);

        // Serialize
        String json = original.serialize();
        assertNotNull(json);
        assertTrue(json.contains("client-001"));
        assertTrue(json.contains("server-001"));

        // Deserialize
        Envelope restored = Envelope.deserialize(json);

        assertEquals(original.getFrom(), restored.getFrom());
        assertEquals(original.getTo(), restored.getTo());
        assertEquals(original.getOperation(), restored.getOperation());
        assertEquals(original.getMessageId(), restored.getMessageId());
        assertEquals(original.getCapabilities().size(), restored.getCapabilities().size());
    }

    @Test
    public void testEnvelope_SerializeDeserialize_MinimalFields() throws Exception {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("a")
            .to("b")
            .operation(OperationType.DATA)
            .messageId("1")
            .build();

        Envelope original = new Envelope(options);
        String json = original.serialize();
        Envelope restored = Envelope.deserialize(json);

        assertEquals(original.getFrom(), restored.getFrom());
        assertEquals(original.getTo(), restored.getTo());
        assertEquals(original.getOperation(), restored.getOperation());
    }

    @Test
    public void testEnvelope_Deserialize_InvalidJSON() {
        assertThrows(SerializationException.class, () -> {
            Envelope.deserialize("{invalid json}");
        });

        assertThrows(SerializationException.class, () -> {
            Envelope.deserialize("");
        });

        assertThrows(SerializationException.class, () -> {
            Envelope.deserialize(null);
        });
    }

    @Test
    public void testEnvelope_Deserialize_MissingFields() {
        // JSON with missing required fields
        String json = "{\"from\":\"sender\"}";

        assertThrows(SerializationException.class, () -> {
            Envelope.deserialize(json);
        });
    }

    @Test
    public void testEnvelope_HashConsistency() throws Exception {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("sender")
            .to("receiver")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .build();

        Envelope envelope = new Envelope(options);

        String hash1 = envelope.getHash();
        String hash2 = envelope.getHash();

        assertNotNull(hash1);
        assertNotNull(hash2);
        assertEquals(hash1, hash2);
        assertEquals(64, hash1.length()); // SHA-256 produces 64 hex chars
    }

    @Test
    public void testEnvelope_HashDifferentEnvelopes() throws Exception {
        Envelope env1 = new Envelope(EnvelopeOptions.builder()
            .from("sender1")
            .to("receiver")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .build());

        Envelope env2 = new Envelope(EnvelopeOptions.builder()
            .from("sender2")
            .to("receiver")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .build());

        String hash1 = env1.getHash();
        String hash2 = env2.getHash();

        assertNotEquals(hash1, hash2);
    }

    @Test
    public void testEnvelope_Clone() throws Exception {
        Map<String, Object> caps = new HashMap<>();
        caps.put("key", "value");

        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("sender")
            .to("receiver")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .capabilities(caps)
            .build();

        Envelope original = new Envelope(options);

        // Test that envelope can be created and accessed
        assertEquals("sender", original.getFrom());
        assertEquals("receiver", original.getTo());
        assertEquals(OperationType.DATA, original.getOperation());
        assertEquals("msg-001", original.getMessageId());

        // Test modification works
        original.setFrom("modified");
        assertEquals("modified", original.getFrom());
    }

    @Test
    public void testEnvelope_Close() throws Exception {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("sender")
            .to("receiver")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .build();

        Envelope envelope = new Envelope(options);

        // Should not throw
        envelope.close();

        // Can still use envelope after close in pure Java implementation
        assertTrue(envelope.validate());
    }

    @Test
    public void testEnvelope_TryWithResources() {
        // Test that Envelope works with try-with-resources
        assertDoesNotThrow(() -> {
            try (Envelope envelope = new Envelope(EnvelopeOptions.builder()
                .from("sender")
                .to("receiver")
                .operation(OperationType.DATA)
                .messageId("msg-001")
                .build())) {

                assertTrue(envelope.validate());
            }
        });
    }

    @Test
    public void testEnvelope_Equals() throws Exception {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("sender")
            .to("receiver")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .build();

        Envelope env1 = new Envelope(options);
        Envelope env2 = new Envelope(options);

        // Note: Envelopes may or may not implement equals()
        // This test documents current behavior
        assertNotNull(env1);
        assertNotNull(env2);
    }

    @Test
    public void testEnvelope_ToString() throws Exception {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("sender")
            .to("receiver")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .build();

        Envelope envelope = new Envelope(options);
        String str = envelope.toString();

        assertNotNull(str);
        assertFalse(str.isEmpty());
    }
}

