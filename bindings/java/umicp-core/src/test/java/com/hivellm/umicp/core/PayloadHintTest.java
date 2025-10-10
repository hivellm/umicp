package com.hivellm.umicp.core;

import com.hivellm.umicp.types.EncodingType;
import com.hivellm.umicp.types.PayloadType;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link PayloadHint}.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 */
class PayloadHintTest {

    @Test
    @DisplayName("Should create empty payload hint")
    void testEmptyHint() {
        PayloadHint hint = PayloadHint.builder().build();

        assertThat(hint.getType()).isNull();
        assertThat(hint.getSize()).isNull();
        assertThat(hint.getEncoding()).isNull();
        assertThat(hint.getCount()).isNull();
        assertThat(hint.validate()).isTrue();
    }

    @Test
    @DisplayName("Should create hint with type only")
    void testTypeOnly() {
        PayloadHint hint = PayloadHint.builder()
            .type(PayloadType.TEXT)
            .build();

        assertThat(hint.getType()).isEqualTo(PayloadType.TEXT);
        assertThat(hint.validate()).isTrue();
    }

    @Test
    @DisplayName("Should create hint for vector with auto-calculated size")
    void testVectorWithAutoSize() {
        PayloadHint hint = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT32)
            .count(768)
            .build();

        assertThat(hint.getType()).isEqualTo(PayloadType.VECTOR);
        assertThat(hint.getEncoding()).isEqualTo(EncodingType.FLOAT32);
        assertThat(hint.getCount()).isEqualTo(768);
        assertThat(hint.getSize()).isEqualTo(3072); // 768 * 4 bytes
        assertThat(hint.validate()).isTrue();
    }

    @Test
    @DisplayName("Should create hint with explicit size")
    void testExplicitSize() {
        PayloadHint hint = PayloadHint.builder()
            .type(PayloadType.BINARY)
            .size(1024)
            .build();

        assertThat(hint.getType()).isEqualTo(PayloadType.BINARY);
        assertThat(hint.getSize()).isEqualTo(1024);
        assertThat(hint.validate()).isTrue();
    }

    @Test
    @DisplayName("Should validate consistency between encoding, count, and size")
    void testConsistencyValidation() {
        // Consistent values
        PayloadHint validHint = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT64)
            .count(100)
            .size(800) // 100 * 8 bytes
            .build();

        assertThat(validHint.validate()).isTrue();

        // Inconsistent size
        PayloadHint invalidHint = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT32)
            .count(100)
            .size(500) // Should be 400
            .build();

        assertThat(invalidHint.validate()).isFalse();
    }

    @Test
    @DisplayName("Should reject encoding without VECTOR type")
    void testEncodingRequiresVectorType() {
        PayloadHint hint = PayloadHint.builder()
            .type(PayloadType.TEXT)
            .encoding(EncodingType.FLOAT32)
            .build();

        assertThat(hint.validate()).isFalse();
    }

    @Test
    @DisplayName("Should reject negative or zero size")
    void testInvalidSize() {
        assertThatThrownBy(() ->
            PayloadHint.builder().size(0)
        ).isInstanceOf(IllegalArgumentException.class);

        assertThatThrownBy(() ->
            PayloadHint.builder().size(-100)
        ).isInstanceOf(IllegalArgumentException.class);
    }

    @Test
    @DisplayName("Should reject negative or zero count")
    void testInvalidCount() {
        assertThatThrownBy(() ->
            PayloadHint.builder().count(0)
        ).isInstanceOf(IllegalArgumentException.class);

        assertThatThrownBy(() ->
            PayloadHint.builder().count(-10)
        ).isInstanceOf(IllegalArgumentException.class);
    }

    @Test
    @DisplayName("Should support equals and hashCode")
    void testEqualsAndHashCode() {
        PayloadHint hint1 = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT32)
            .count(768)
            .build();

        PayloadHint hint2 = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT32)
            .count(768)
            .build();

        PayloadHint hint3 = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT64)
            .count(768)
            .build();

        assertThat(hint1).isEqualTo(hint2);
        assertThat(hint1.hashCode()).isEqualTo(hint2.hashCode());
        assertThat(hint1).isNotEqualTo(hint3);
    }

    @Test
    @DisplayName("Should have meaningful toString")
    void testToString() {
        PayloadHint hint = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT32)
            .count(768)
            .build();

        String str = hint.toString();
        assertThat(str).contains("PayloadHint");
        assertThat(str).contains("VECTOR");
        assertThat(str).contains("FLOAT32");
        assertThat(str).contains("768");
    }

    @Test
    @DisplayName("Should handle different encoding types")
    void testDifferentEncodings() {
        // FLOAT32: 4 bytes per value
        PayloadHint float32 = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT32)
            .count(100)
            .build();
        assertThat(float32.getSize()).isEqualTo(400);

        // FLOAT64: 8 bytes per value
        PayloadHint float64 = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.FLOAT64)
            .count(100)
            .build();
        assertThat(float64.getSize()).isEqualTo(800);

        // UINT8: 1 byte per value
        PayloadHint uint8 = PayloadHint.builder()
            .type(PayloadType.VECTOR)
            .encoding(EncodingType.UINT8)
            .count(100)
            .build();
        assertThat(uint8.getSize()).isEqualTo(100);
    }
}

