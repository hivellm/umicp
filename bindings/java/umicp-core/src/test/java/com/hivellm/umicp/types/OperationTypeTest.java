package com.hivellm.umicp.types;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link OperationType}.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 */
class OperationTypeTest {

    @Test
    @DisplayName("Should have correct numeric values")
    void testNumericValues() {
        assertThat(OperationType.CONTROL.getValue()).isEqualTo(0);
        assertThat(OperationType.DATA.getValue()).isEqualTo(1);
        assertThat(OperationType.ACK.getValue()).isEqualTo(2);
        assertThat(OperationType.ERROR.getValue()).isEqualTo(3);
        assertThat(OperationType.REQUEST.getValue()).isEqualTo(4);
        assertThat(OperationType.RESPONSE.getValue()).isEqualTo(5);
    }

    @Test
    @DisplayName("Should convert from value correctly")
    void testFromValue() {
        assertThat(OperationType.fromValue(0)).isEqualTo(OperationType.CONTROL);
        assertThat(OperationType.fromValue(1)).isEqualTo(OperationType.DATA);
        assertThat(OperationType.fromValue(2)).isEqualTo(OperationType.ACK);
        assertThat(OperationType.fromValue(3)).isEqualTo(OperationType.ERROR);
        assertThat(OperationType.fromValue(4)).isEqualTo(OperationType.REQUEST);
        assertThat(OperationType.fromValue(5)).isEqualTo(OperationType.RESPONSE);
    }

    @Test
    @DisplayName("Should throw exception for invalid value")
    void testFromValueInvalid() {
        assertThatThrownBy(() -> OperationType.fromValue(99))
            .isInstanceOf(IllegalArgumentException.class)
            .hasMessageContaining("Invalid OperationType value: 99");
    }

    @Test
    @DisplayName("Should correctly identify operations expecting response")
    void testExpectsResponse() {
        assertThat(OperationType.REQUEST.expectsResponse()).isTrue();
        assertThat(OperationType.DATA.expectsResponse()).isFalse();
        assertThat(OperationType.ACK.expectsResponse()).isFalse();
        assertThat(OperationType.RESPONSE.expectsResponse()).isFalse();
    }

    @Test
    @DisplayName("Should correctly identify response operations")
    void testIsResponse() {
        assertThat(OperationType.RESPONSE.isResponse()).isTrue();
        assertThat(OperationType.ACK.isResponse()).isTrue();
        assertThat(OperationType.REQUEST.isResponse()).isFalse();
        assertThat(OperationType.DATA.isResponse()).isFalse();
    }

    @Test
    @DisplayName("Should correctly identify error operations")
    void testIsError() {
        assertThat(OperationType.ERROR.isError()).isTrue();
        assertThat(OperationType.DATA.isError()).isFalse();
        assertThat(OperationType.CONTROL.isError()).isFalse();
    }

    @Test
    @DisplayName("Should have all enum values")
    void testAllValues() {
        OperationType[] values = OperationType.values();
        assertThat(values).hasSize(6);
        assertThat(values).containsExactly(
            OperationType.CONTROL,
            OperationType.DATA,
            OperationType.ACK,
            OperationType.ERROR,
            OperationType.REQUEST,
            OperationType.RESPONSE
        );
    }

    @Test
    @DisplayName("Should support valueOf")
    void testValueOf() {
        assertThat(OperationType.valueOf("DATA")).isEqualTo(OperationType.DATA);
        assertThat(OperationType.valueOf("REQUEST")).isEqualTo(OperationType.REQUEST);
    }
}

