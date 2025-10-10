package com.hivellm.umicp.core;

import com.hivellm.umicp.types.EncodingType;
import com.hivellm.umicp.types.OperationType;
import com.hivellm.umicp.types.PayloadType;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link EnvelopeOptions}.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 */
class EnvelopeOptionsTest {

    @Test
    @DisplayName("Should create empty options")
    void testEmptyOptions() {
        EnvelopeOptions options = EnvelopeOptions.builder().build();

        assertThat(options.getFrom()).isNull();
        assertThat(options.getTo()).isNull();
        assertThat(options.getOperation()).isNull();
        assertThat(options.getMessageId()).isNull();
        assertThat(options.getCapabilities()).isEmpty();
        assertThat(options.getPayloadHint()).isNull();
    }

    @Test
    @DisplayName("Should create options with all fields")
    void testCompleteOptions() {
        String messageId = UUID.randomUUID().toString();
        PayloadHint hint = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT32)
            .count(768)
            .build();

        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("client-001")
            .to("server-001")
            .operation(OperationType.DATA)
            .messageId(messageId)
            .capability("priority", "high")
            .capability("content-type", "application/json")
            .payloadHint(hint)
            .build();

        assertThat(options.getFrom()).isEqualTo("client-001");
        assertThat(options.getTo()).isEqualTo("server-001");
        assertThat(options.getOperation()).isEqualTo(OperationType.DATA);
        assertThat(options.getMessageId()).isEqualTo(messageId);
        assertThat(options.getCapabilities())
            .hasSize(2)
            .containsEntry("priority", "high")
            .containsEntry("content-type", "application/json");
        assertThat(options.getPayloadHint()).isEqualTo(hint);
    }

    @Test
    @DisplayName("Should add capabilities individually")
    void testIndividualCapabilities() {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .capability("key1", "value1")
            .capability("key2", "value2")
            .capability("key3", "value3")
            .build();

        assertThat(options.getCapabilities())
            .hasSize(3)
            .containsEntry("key1", "value1")
            .containsEntry("key2", "value2")
            .containsEntry("key3", "value3");
    }

    @Test
    @DisplayName("Should set capabilities as map")
    void testCapabilitiesMap() {
        Map<String, String> caps = new HashMap<>();
        caps.put("version", "1.0");
        caps.put("encoding", "utf-8");

        EnvelopeOptions options = EnvelopeOptions.builder()
            .capabilities(caps)
            .build();

        assertThat(options.getCapabilities()).isEqualTo(caps);
    }

    @Test
    @DisplayName("Should combine map and individual capabilities")
    void testMixedCapabilities() {
        Map<String, String> caps = new HashMap<>();
        caps.put("key1", "value1");

        EnvelopeOptions options = EnvelopeOptions.builder()
            .capabilities(caps)
            .capability("key2", "value2")
            .capability("key3", "value3")
            .build();

        assertThat(options.getCapabilities())
            .hasSize(3)
            .containsEntry("key1", "value1")
            .containsEntry("key2", "value2")
            .containsEntry("key3", "value3");
    }

    @Test
    @DisplayName("Should return defensive copy of capabilities")
    void testDefensiveCopy() {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .capability("key", "value")
            .build();

        Map<String, String> caps = options.getCapabilities();
        caps.put("new-key", "new-value");

        // Original should not be affected
        assertThat(options.getCapabilities()).hasSize(1);
        assertThat(options.getCapabilities()).doesNotContainKey("new-key");
    }

    @Test
    @DisplayName("Should support fluent API")
    void testFluentAPI() {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("client")
            .to("server")
            .operation(OperationType.REQUEST)
            .messageId("msg-001")
            .capability("test", "value")
            .build();

        assertThat(options.getFrom()).isEqualTo("client");
        assertThat(options.getTo()).isEqualTo("server");
        assertThat(options.getOperation()).isEqualTo(OperationType.REQUEST);
    }

    @Test
    @DisplayName("Should support equals and hashCode")
    void testEqualsAndHashCode() {
        EnvelopeOptions options1 = EnvelopeOptions.builder()
            .from("client")
            .to("server")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .build();

        EnvelopeOptions options2 = EnvelopeOptions.builder()
            .from("client")
            .to("server")
            .operation(OperationType.DATA)
            .messageId("msg-001")
            .build();

        EnvelopeOptions options3 = EnvelopeOptions.builder()
            .from("client")
            .to("server")
            .operation(OperationType.REQUEST)
            .messageId("msg-002")
            .build();

        assertThat(options1).isEqualTo(options2);
        assertThat(options1.hashCode()).isEqualTo(options2.hashCode());
        assertThat(options1).isNotEqualTo(options3);
    }

    @Test
    @DisplayName("Should have meaningful toString")
    void testToString() {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .from("client")
            .to("server")
            .operation(OperationType.DATA)
            .build();

        String str = options.toString();
        assertThat(str).contains("EnvelopeOptions");
        assertThat(str).contains("client");
        assertThat(str).contains("server");
        assertThat(str).contains("DATA");
    }

    @Test
    @DisplayName("Should handle null capabilities map")
    void testNullCapabilities() {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .capabilities(null)
            .build();

        assertThat(options.getCapabilities()).isEmpty();
    }

    @Test
    @DisplayName("Should allow overwriting capabilities")
    void testOverwriteCapabilities() {
        EnvelopeOptions options = EnvelopeOptions.builder()
            .capability("key", "value1")
            .capability("key", "value2")
            .build();

        assertThat(options.getCapabilities())
            .hasSize(1)
            .containsEntry("key", "value2");
    }
}

