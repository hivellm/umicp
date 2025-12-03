package com.hivellm.umicp.core;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link MatrixResult}.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 */
class MatrixResultTest {

    @Test
    @DisplayName("Should create successful result with scalar value")
    void testSuccessWithScalar() {
        MatrixResult result = MatrixResult.success(42.5);

        assertThat(result.isSuccess()).isTrue();
        assertThat(result.isFailure()).isFalse();
        assertThat(result.getResult()).isEqualTo(42.5);
        assertThat(result.getError()).isNull();
        assertThat(result.getSimilarity()).isNull();
        assertThat(result.getData()).isNull();
    }

    @Test
    @DisplayName("Should create successful result with similarity")
    void testSuccessWithSimilarity() {
        MatrixResult result = MatrixResult.successWithSimilarity(0.95);

        assertThat(result.isSuccess()).isTrue();
        assertThat(result.getSimilarity()).isEqualTo(0.95);
        assertThat(result.getResult()).isNull();
        assertThat(result.getData()).isNull();
    }

    @Test
    @DisplayName("Should create successful result with data")
    void testSuccessWithData() {
        float[] data = {1.0f, 2.0f, 3.0f};
        MatrixResult result = MatrixResult.successWithData(data);

        assertThat(result.isSuccess()).isTrue();
        assertThat(result.getData()).isEqualTo(new float[]{1.0f, 2.0f, 3.0f});
        assertThat(result.getResult()).isNull();
        assertThat(result.getSimilarity()).isNull();
    }

    @Test
    @DisplayName("Should create void success result")
    void testVoidSuccess() {
        MatrixResult result = MatrixResult.success();

        assertThat(result.isSuccess()).isTrue();
        assertThat(result.getResult()).isNull();
        assertThat(result.getSimilarity()).isNull();
        assertThat(result.getData()).isNull();
        assertThat(result.getError()).isNull();
    }

    @Test
    @DisplayName("Should create failure result")
    void testFailure() {
        MatrixResult result = MatrixResult.failure("Something went wrong");

        assertThat(result.isSuccess()).isFalse();
        assertThat(result.isFailure()).isTrue();
        assertThat(result.getError()).isEqualTo("Something went wrong");
        assertThat(result.getResult()).isNull();
        assertThat(result.getSimilarity()).isNull();
        assertThat(result.getData()).isNull();
    }

    @Test
    @DisplayName("Should return defensive copy of data")
    void testDefensiveCopy() {
        float[] original = {1.0f, 2.0f, 3.0f};
        MatrixResult result = MatrixResult.successWithData(original);

        // Modify original
        original[0] = 999.0f;

        // Result should not be affected
        assertThat(result.getData()[0]).isEqualTo(1.0f);

        // Modify returned data
        float[] returned = result.getData();
        returned[1] = 888.0f;

        // Result should not be affected
        assertThat(result.getData()[1]).isEqualTo(2.0f);
    }

    @Test
    @DisplayName("Should get result or throw")
    void testGetResultOrThrow() {
        // Success case
        MatrixResult success = MatrixResult.success(42.0);
        assertThat(success.getResultOrThrow()).isEqualTo(42.0);

        // Failure case
        MatrixResult failure = MatrixResult.failure("Error");
        assertThatThrownBy(() -> failure.getResultOrThrow())
            .isInstanceOf(IllegalStateException.class)
            .hasMessageContaining("Operation failed");

        // No result available
        MatrixResult noResult = MatrixResult.successWithSimilarity(0.9);
        assertThatThrownBy(() -> noResult.getResultOrThrow())
            .isInstanceOf(IllegalStateException.class)
            .hasMessageContaining("No scalar result available");
    }

    @Test
    @DisplayName("Should get similarity or throw")
    void testGetSimilarityOrThrow() {
        // Success case
        MatrixResult success = MatrixResult.successWithSimilarity(0.95);
        assertThat(success.getSimilarityOrThrow()).isEqualTo(0.95);

        // Failure case
        MatrixResult failure = MatrixResult.failure("Error");
        assertThatThrownBy(() -> failure.getSimilarityOrThrow())
            .isInstanceOf(IllegalStateException.class)
            .hasMessageContaining("Operation failed");

        // No similarity available
        MatrixResult noSimilarity = MatrixResult.success(42.0);
        assertThatThrownBy(() -> noSimilarity.getSimilarityOrThrow())
            .isInstanceOf(IllegalStateException.class)
            .hasMessageContaining("No similarity value available");
    }

    @Test
    @DisplayName("Should get data or throw")
    void testGetDataOrThrow() {
        // Success case
        float[] data = {1.0f, 2.0f};
        MatrixResult success = MatrixResult.successWithData(data);
        assertThat(success.getDataOrThrow()).isEqualTo(data);

        // Failure case
        MatrixResult failure = MatrixResult.failure("Error");
        assertThatThrownBy(() -> failure.getDataOrThrow())
            .isInstanceOf(IllegalStateException.class)
            .hasMessageContaining("Operation failed");

        // No data available
        MatrixResult noData = MatrixResult.success(42.0);
        assertThatThrownBy(() -> noData.getDataOrThrow())
            .isInstanceOf(IllegalStateException.class)
            .hasMessageContaining("No data array available");
    }

    @Test
    @DisplayName("Should support equals and hashCode")
    void testEqualsAndHashCode() {
        MatrixResult result1 = MatrixResult.success(42.0);
        MatrixResult result2 = MatrixResult.success(42.0);
        MatrixResult result3 = MatrixResult.success(43.0);

        assertThat(result1).isEqualTo(result2);
        assertThat(result1.hashCode()).isEqualTo(result2.hashCode());
        assertThat(result1).isNotEqualTo(result3);

        // Test with data
        float[] data = {1.0f, 2.0f};
        MatrixResult withData1 = MatrixResult.successWithData(data);
        MatrixResult withData2 = MatrixResult.successWithData(data);
        assertThat(withData1).isEqualTo(withData2);
    }

    @Test
    @DisplayName("Should have meaningful toString for success")
    void testToStringSuccess() {
        MatrixResult result = MatrixResult.success(42.5);
        String str = result.toString();

        assertThat(str).contains("MatrixResult");
        assertThat(str).contains("success=true");
        assertThat(str).contains("result=42.5");
    }

    @Test
    @DisplayName("Should have meaningful toString for failure")
    void testToStringFailure() {
        MatrixResult result = MatrixResult.failure("Test error");
        String str = result.toString();

        assertThat(str).contains("MatrixResult");
        assertThat(str).contains("success=false");
        assertThat(str).contains("error='Test error'");
    }

    @Test
    @DisplayName("Should have meaningful toString with similarity")
    void testToStringWithSimilarity() {
        MatrixResult result = MatrixResult.successWithSimilarity(0.95);
        String str = result.toString();

        assertThat(str).contains("MatrixResult");
        assertThat(str).contains("success=true");
        assertThat(str).contains("similarity=0.95");
    }

    @Test
    @DisplayName("Should have meaningful toString with data")
    void testToStringWithData() {
        float[] data = {1.0f, 2.0f, 3.0f};
        MatrixResult result = MatrixResult.successWithData(data);
        String str = result.toString();

        assertThat(str).contains("MatrixResult");
        assertThat(str).contains("success=true");
        assertThat(str).contains("data.length=3");
    }
}

