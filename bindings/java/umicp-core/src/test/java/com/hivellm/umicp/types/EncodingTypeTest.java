package com.hivellm.umicp.types;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link EncodingType}.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 */
class EncodingTypeTest {

    @Test
    @DisplayName("Should have correct numeric values")
    void testNumericValues() {
        assertThat(EncodingType.FLOAT32.getValue()).isEqualTo(0);
        assertThat(EncodingType.FLOAT64.getValue()).isEqualTo(1);
        assertThat(EncodingType.INT32.getValue()).isEqualTo(2);
        assertThat(EncodingType.INT64.getValue()).isEqualTo(3);
        assertThat(EncodingType.UINT8.getValue()).isEqualTo(4);
        assertThat(EncodingType.UINT16.getValue()).isEqualTo(5);
        assertThat(EncodingType.UINT32.getValue()).isEqualTo(6);
        assertThat(EncodingType.UINT64.getValue()).isEqualTo(7);
    }

    @Test
    @DisplayName("Should have correct bytes per value")
    void testBytesPerValue() {
        assertThat(EncodingType.FLOAT32.getBytesPerValue()).isEqualTo(4);
        assertThat(EncodingType.FLOAT64.getBytesPerValue()).isEqualTo(8);
        assertThat(EncodingType.INT32.getBytesPerValue()).isEqualTo(4);
        assertThat(EncodingType.INT64.getBytesPerValue()).isEqualTo(8);
        assertThat(EncodingType.UINT8.getBytesPerValue()).isEqualTo(1);
        assertThat(EncodingType.UINT16.getBytesPerValue()).isEqualTo(2);
        assertThat(EncodingType.UINT32.getBytesPerValue()).isEqualTo(4);
        assertThat(EncodingType.UINT64.getBytesPerValue()).isEqualTo(8);
    }

    @Test
    @DisplayName("Should calculate total bytes correctly")
    void testCalculateTotalBytes() {
        assertThat(EncodingType.FLOAT32.calculateTotalBytes(100)).isEqualTo(400);
        assertThat(EncodingType.FLOAT64.calculateTotalBytes(100)).isEqualTo(800);
        assertThat(EncodingType.UINT8.calculateTotalBytes(100)).isEqualTo(100);
        assertThat(EncodingType.UINT16.calculateTotalBytes(100)).isEqualTo(200);
        assertThat(EncodingType.INT64.calculateTotalBytes(768)).isEqualTo(6144);
    }

    @Test
    @DisplayName("Should convert from value correctly")
    void testFromValue() {
        assertThat(EncodingType.fromValue(0)).isEqualTo(EncodingType.FLOAT32);
        assertThat(EncodingType.fromValue(1)).isEqualTo(EncodingType.FLOAT64);
        assertThat(EncodingType.fromValue(7)).isEqualTo(EncodingType.UINT64);
    }

    @Test
    @DisplayName("Should throw exception for invalid value")
    void testFromValueInvalid() {
        assertThatThrownBy(() -> EncodingType.fromValue(99))
            .isInstanceOf(IllegalArgumentException.class)
            .hasMessageContaining("Invalid EncodingType value: 99");
    }

    @Test
    @DisplayName("Should correctly identify floating point types")
    void testIsFloatingPoint() {
        assertThat(EncodingType.FLOAT32.isFloatingPoint()).isTrue();
        assertThat(EncodingType.FLOAT64.isFloatingPoint()).isTrue();
        assertThat(EncodingType.INT32.isFloatingPoint()).isFalse();
        assertThat(EncodingType.UINT8.isFloatingPoint()).isFalse();
    }

    @Test
    @DisplayName("Should correctly identify integer types")
    void testIsInteger() {
        assertThat(EncodingType.INT32.isInteger()).isTrue();
        assertThat(EncodingType.INT64.isInteger()).isTrue();
        assertThat(EncodingType.UINT8.isInteger()).isTrue();
        assertThat(EncodingType.FLOAT32.isInteger()).isFalse();
        assertThat(EncodingType.FLOAT64.isInteger()).isFalse();
    }

    @Test
    @DisplayName("Should correctly identify signed types")
    void testIsSigned() {
        assertThat(EncodingType.INT32.isSigned()).isTrue();
        assertThat(EncodingType.INT64.isSigned()).isTrue();
        assertThat(EncodingType.UINT8.isSigned()).isFalse();
        assertThat(EncodingType.UINT32.isSigned()).isFalse();
    }

    @Test
    @DisplayName("Should correctly identify unsigned types")
    void testIsUnsigned() {
        assertThat(EncodingType.UINT8.isUnsigned()).isTrue();
        assertThat(EncodingType.UINT16.isUnsigned()).isTrue();
        assertThat(EncodingType.UINT32.isUnsigned()).isTrue();
        assertThat(EncodingType.UINT64.isUnsigned()).isTrue();
        assertThat(EncodingType.INT32.isUnsigned()).isFalse();
        assertThat(EncodingType.FLOAT32.isUnsigned()).isFalse();
    }

    @Test
    @DisplayName("Should have all enum values")
    void testAllValues() {
        EncodingType[] values = EncodingType.values();
        assertThat(values).hasSize(8);
    }

    @Test
    @DisplayName("Should handle edge cases for byte calculation")
    void testCalculateTotalBytesEdgeCases() {
        assertThat(EncodingType.UINT8.calculateTotalBytes(0)).isEqualTo(0);
        assertThat(EncodingType.FLOAT32.calculateTotalBytes(1)).isEqualTo(4);
        assertThat(EncodingType.UINT64.calculateTotalBytes(1000000)).isEqualTo(8000000);
    }
}

