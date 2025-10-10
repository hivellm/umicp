package com.hivellm.umicp.core;

import com.hivellm.umicp.types.OperationType;
import com.hivellm.umicp.types.SerializationException;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link UMICP}.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 */
class UMICPTest {

    @Test
    @DisplayName("Should have version information")
    void testVersionInfo() {
        assertThat(UMICP.VERSION).isNotNull().isNotEmpty();
        assertThat(UMICP.PROTOCOL_VERSION).isNotNull().isNotEmpty();
        assertThat(UMICP.BUILD).isNotNull().isNotEmpty();

        assertThat(UMICP.getVersion()).isEqualTo(UMICP.VERSION);
        assertThat(UMICP.getProtocolVersion()).isEqualTo(UMICP.PROTOCOL_VERSION);
        assertThat(UMICP.getBuild()).isEqualTo(UMICP.BUILD);
    }

    @Test
    @DisplayName("Should format version info")
    void testVersionInfoString() {
        String versionInfo = UMICP.getVersionInfo();

        assertThat(versionInfo).contains("UMICP");
        assertThat(versionInfo).contains(UMICP.VERSION);
        assertThat(versionInfo).contains(UMICP.PROTOCOL_VERSION);
        assertThat(versionInfo).contains(UMICP.BUILD);
    }

    @Test
    @DisplayName("Should print version")
    void testPrintVersion() {
        assertThatCode(() -> UMICP.printVersion()).doesNotThrowAnyException();
    }

    @Test
    @DisplayName("Should report implementation type")
    void testImplementationType() {
        // Pure Java implementation
        assertThat(UMICP.isNativeAvailable()).isFalse();
        assertThat(UMICP.getImplementationType()).isEqualTo("pure-java");
    }

    @Test
    @DisplayName("Should create empty envelope")
    void testCreateEmptyEnvelope() {
        Envelope envelope = UMICP.createEnvelope();

        assertThat(envelope).isNotNull();
        assertThat(envelope.getFrom()).isNull();
        assertThat(envelope.getTo()).isNull();
    }

    @Test
    @DisplayName("Should create envelope with options")
    void testCreateEnvelopeWithOptions() {
        EnvelopeOptions options = UMICP.envelopeOptions()
            .from("client")
            .to("server")
            .operation(OperationType.DATA)
            .build();

        Envelope envelope = UMICP.createEnvelope(options);

        assertThat(envelope).isNotNull();
        assertThat(envelope.getFrom()).isEqualTo("client");
        assertThat(envelope.getTo()).isEqualTo("server");
        assertThat(envelope.getOperation()).isEqualTo(OperationType.DATA);
    }

    @Test
    @DisplayName("Should create envelope options builder")
    void testEnvelopeOptionsBuilder() {
        EnvelopeOptions.Builder builder = UMICP.envelopeOptions();

        assertThat(builder).isNotNull();

        EnvelopeOptions options = builder
            .from("test")
            .build();

        assertThat(options.getFrom()).isEqualTo("test");
    }

    @Test
    @DisplayName("Should create matrix")
    void testCreateMatrix() {
        try (Matrix matrix = UMICP.createMatrix()) {
            assertThat(matrix).isNotNull();

            // Test that it works
            float[] a = {1.0f, 2.0f};
            float[] b = {3.0f, 4.0f};
            MatrixResult result = matrix.dotProduct(a, b);

            assertThat(result.isSuccess()).isTrue();
        }
    }

    @Test
    @DisplayName("Should create payload hint builder")
    void testPayloadHintBuilder() {
        PayloadHint.Builder builder = UMICP.payloadHint();

        assertThat(builder).isNotNull();

        PayloadHint hint = builder.size(100).build();

        assertThat(hint.getSize()).isEqualTo(100);
    }

    @Test
    @DisplayName("Should serialize envelope")
    void testSerialize() throws SerializationException {
        Envelope envelope = UMICP.createEnvelope(
            UMICP.envelopeOptions()
                .from("client")
                .to("server")
                .operation(OperationType.DATA)
                .build()
        );

        String json = UMICP.serialize(envelope);

        assertThat(json).isNotNull().isNotEmpty();
        assertThat(json).contains("\"from\":\"client\"");
        assertThat(json).contains("\"to\":\"server\"");
    }

    @Test
    @DisplayName("Should deserialize envelope")
    void testDeserialize() throws SerializationException {
        String json = "{\"from\":\"client\",\"to\":\"server\",\"operation\":1,\"messageId\":\"test\"}";

        Envelope envelope = UMICP.deserialize(json);

        assertThat(envelope).isNotNull();
        assertThat(envelope.getFrom()).isEqualTo("client");
        assertThat(envelope.getTo()).isEqualTo("server");
        assertThat(envelope.getOperation()).isEqualTo(OperationType.DATA);
    }

    @Test
    @DisplayName("Should round-trip serialize and deserialize")
    void testRoundTrip() throws SerializationException {
        Envelope original = UMICP.createEnvelope(
            UMICP.envelopeOptions()
                .from("client")
                .to("server")
                .operation(OperationType.REQUEST)
                .messageId("msg-001")
                .capability("test", "value")
                .build()
        );

        String json = UMICP.serialize(original);
        Envelope deserialized = UMICP.deserialize(json);

        assertThat(deserialized.getFrom()).isEqualTo(original.getFrom());
        assertThat(deserialized.getTo()).isEqualTo(original.getTo());
        assertThat(deserialized.getOperation()).isEqualTo(original.getOperation());
        assertThat(deserialized.getMessageId()).isEqualTo(original.getMessageId());
    }

    @Test
    @DisplayName("Should validate envelope")
    void testValidate() {
        Envelope valid = UMICP.createEnvelope(
            UMICP.envelopeOptions()
                .from("client")
                .to("server")
                .operation(OperationType.DATA)
                .build()
        );

        assertThat(UMICP.validate(valid)).isTrue();

        Envelope invalid = UMICP.createEnvelope();
        assertThat(UMICP.validate(invalid)).isFalse();
    }

    @Test
    @DisplayName("Should compute hash")
    void testHash() throws SerializationException {
        Envelope envelope = UMICP.createEnvelope(
            UMICP.envelopeOptions()
                .from("client")
                .to("server")
                .operation(OperationType.DATA)
                .build()
        );

        String hash = UMICP.hash(envelope);

        assertThat(hash).isNotNull();
        assertThat(hash).hasSize(64); // SHA-256
        assertThat(hash).matches("[0-9a-f]{64}");
    }

    @Test
    @DisplayName("Should not allow instantiation")
    void testNoInstantiation() {
        assertThatThrownBy(() -> {
            // Use reflection to try to instantiate
            var constructor = UMICP.class.getDeclaredConstructor();
            constructor.setAccessible(true);
            constructor.newInstance();
        })
        .hasCauseInstanceOf(UnsupportedOperationException.class)
        .getCause()
        .hasMessageContaining("utility class");
    }

    @Test
    @DisplayName("Should provide fluent API")
    void testFluentAPI() throws SerializationException {
        // Complete workflow in fluent style
        String json = UMICP.serialize(
            UMICP.createEnvelope(
                UMICP.envelopeOptions()
                    .from("client")
                    .to("server")
                    .operation(OperationType.DATA)
                    .messageId("msg-001")
                    .capability("priority", "high")
                    .payloadHint(
                        UMICP.payloadHint()
                            .size(1024)
                            .build()
                    )
                    .build()
            )
        );

        assertThat(json).isNotEmpty();

        Envelope envelope = UMICP.deserialize(json);
        assertThat(UMICP.validate(envelope)).isTrue();
    }
}

