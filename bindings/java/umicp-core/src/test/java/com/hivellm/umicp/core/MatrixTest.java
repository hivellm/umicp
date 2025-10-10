package com.hivellm.umicp.core;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link Matrix}.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 */
class MatrixTest {

    private static final float DELTA = 0.0001f;

    @Test
    @DisplayName("Should add vectors")
    void testVectorAdd() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {1.0f, 2.0f, 3.0f};
            float[] b = {4.0f, 5.0f, 6.0f};
            float[] result = new float[3];

            MatrixResult opResult = matrix.vectorAdd(a, b, result);

            assertThat(opResult.isSuccess()).isTrue();
            assertThat(result).containsExactly(5.0f, 7.0f, 9.0f);
        }
    }

    @Test
    @DisplayName("Should reject vector addition with mismatched dimensions")
    void testVectorAddMismatch() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {1.0f, 2.0f};
            float[] b = {4.0f, 5.0f, 6.0f};
            float[] result = new float[3];

            MatrixResult opResult = matrix.vectorAdd(a, b, result);

            assertThat(opResult.isFailure()).isTrue();
            assertThat(opResult.getError()).contains("dimensions must match");
        }
    }

    @Test
    @DisplayName("Should subtract vectors")
    void testVectorSubtract() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {5.0f, 7.0f, 9.0f};
            float[] b = {1.0f, 2.0f, 3.0f};
            float[] result = new float[3];

            MatrixResult opResult = matrix.vectorSubtract(a, b, result);

            assertThat(opResult.isSuccess()).isTrue();
            assertThat(result).containsExactly(4.0f, 5.0f, 6.0f);
        }
    }

    @Test
    @DisplayName("Should multiply vectors element-wise")
    void testVectorMultiply() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {2.0f, 3.0f, 4.0f};
            float[] b = {5.0f, 6.0f, 7.0f};
            float[] result = new float[3];

            MatrixResult opResult = matrix.vectorMultiply(a, b, result);

            assertThat(opResult.isSuccess()).isTrue();
            assertThat(result).containsExactly(10.0f, 18.0f, 28.0f);
        }
    }

    @Test
    @DisplayName("Should compute dot product")
    void testDotProduct() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {1.0f, 2.0f, 3.0f};
            float[] b = {4.0f, 5.0f, 6.0f};

            MatrixResult result = matrix.dotProduct(a, b);

            assertThat(result.isSuccess()).isTrue();
            assertThat(result.getResult()).isEqualTo(32.0, within(DELTA));
            // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
        }
    }

    @Test
    @DisplayName("Should reject dot product with mismatched dimensions")
    void testDotProductMismatch() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {1.0f, 2.0f};
            float[] b = {4.0f, 5.0f, 6.0f};

            MatrixResult result = matrix.dotProduct(a, b);

            assertThat(result.isFailure()).isTrue();
            assertThat(result.getError()).contains("dimensions must match");
        }
    }

    @Test
    @DisplayName("Should compute cosine similarity")
    void testCosineSimilarity() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {1.0f, 2.0f, 3.0f};
            float[] b = {4.0f, 5.0f, 6.0f};

            MatrixResult result = matrix.cosineSimilarity(a, b);

            assertThat(result.isSuccess()).isTrue();
            // Cosine similarity = 32 / (sqrt(14) * sqrt(77)) ≈ 0.9746
            assertThat(result.getSimilarity()).isEqualTo(0.9746, within(0.0001));
        }
    }

    @Test
    @DisplayName("Should compute cosine similarity for identical vectors")
    void testCosineSimilarityIdentical() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {1.0f, 2.0f, 3.0f};
            float[] b = {1.0f, 2.0f, 3.0f};

            MatrixResult result = matrix.cosineSimilarity(a, b);

            assertThat(result.isSuccess()).isTrue();
            assertThat(result.getSimilarity()).isEqualTo(1.0, within(DELTA));
        }
    }

    @Test
    @DisplayName("Should compute cosine similarity for orthogonal vectors")
    void testCosineSimilarityOrthogonal() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {1.0f, 0.0f};
            float[] b = {0.0f, 1.0f};

            MatrixResult result = matrix.cosineSimilarity(a, b);

            assertThat(result.isSuccess()).isTrue();
            assertThat(result.getSimilarity()).isEqualTo(0.0, within(DELTA));
        }
    }

    @Test
    @DisplayName("Should reject cosine similarity for zero vector")
    void testCosineSimilarityZeroVector() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {1.0f, 2.0f, 3.0f};
            float[] b = {0.0f, 0.0f, 0.0f};

            MatrixResult result = matrix.cosineSimilarity(a, b);

            assertThat(result.isFailure()).isTrue();
            assertThat(result.getError()).contains("Zero-magnitude");
        }
    }

    @Test
    @DisplayName("Should normalize vector")
    void testNormalize() {
        try (Matrix matrix = new Matrix()) {
            float[] vector = {3.0f, 4.0f};

            MatrixResult result = matrix.normalize(vector);

            assertThat(result.isSuccess()).isTrue();
            assertThat(vector[0]).isEqualTo(0.6f, within(DELTA));
            assertThat(vector[1]).isEqualTo(0.8f, within(DELTA));

            // Verify magnitude is 1
            double magnitude = Math.sqrt(vector[0] * vector[0] + vector[1] * vector[1]);
            assertThat(magnitude).isEqualTo(1.0, within(DELTA));
        }
    }

    @Test
    @DisplayName("Should reject normalization of zero vector")
    void testNormalizeZeroVector() {
        try (Matrix matrix = new Matrix()) {
            float[] vector = {0.0f, 0.0f, 0.0f};

            MatrixResult result = matrix.normalize(vector);

            assertThat(result.isFailure()).isTrue();
            assertThat(result.getError()).contains("zero-magnitude");
        }
    }

    @Test
    @DisplayName("Should multiply matrices")
    void testMatrixMultiply() {
        try (Matrix matrix = new Matrix()) {
            // 2x2 matrices
            float[] a = {1.0f, 2.0f, 3.0f, 4.0f}; // [[1,2], [3,4]]
            float[] b = {5.0f, 6.0f, 7.0f, 8.0f}; // [[5,6], [7,8]]
            float[] result = new float[4];

            MatrixResult opResult = matrix.matrixMultiply(a, b, result, 2, 2, 2);

            assertThat(opResult.isSuccess()).isTrue();
            // [[1*5+2*7, 1*6+2*8], [3*5+4*7, 3*6+4*8]]
            // [[19, 22], [43, 50]]
            assertThat(result).containsExactly(19.0f, 22.0f, 43.0f, 50.0f);
        }
    }

    @Test
    @DisplayName("Should multiply matrices with different dimensions")
    void testMatrixMultiplyRectangular() {
        try (Matrix matrix = new Matrix()) {
            // A: 2x3, B: 3x2, Result: 2x2
            float[] a = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f}; // [[1,2,3], [4,5,6]]
            float[] b = {7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f}; // [[7,8], [9,10], [11,12]]
            float[] result = new float[4];

            MatrixResult opResult = matrix.matrixMultiply(a, b, result, 2, 3, 2);

            assertThat(opResult.isSuccess()).isTrue();
            // [[1*7+2*9+3*11, 1*8+2*10+3*12], [4*7+5*9+6*11, 4*8+5*10+6*12]]
            // [[58, 64], [139, 154]]
            assertThat(result).containsExactly(58.0f, 64.0f, 139.0f, 154.0f);
        }
    }

    @Test
    @DisplayName("Should reject matrix multiplication with wrong dimensions")
    void testMatrixMultiplyWrongDimensions() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {1.0f, 2.0f};
            float[] b = {3.0f, 4.0f, 5.0f};
            float[] result = new float[2];

            MatrixResult opResult = matrix.matrixMultiply(a, b, result, 2, 1, 2);

            assertThat(opResult.isFailure()).isTrue();
            assertThat(opResult.getError()).contains("dimensions don't match");
        }
    }

    @Test
    @DisplayName("Should transpose matrix")
    void testTranspose() {
        try (Matrix matrix = new Matrix()) {
            // 2x3 matrix
            float[] input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f}; // [[1,2,3], [4,5,6]]
            float[] output = new float[6];

            MatrixResult opResult = matrix.transpose(input, output, 2, 3);

            assertThat(opResult.isSuccess()).isTrue();
            // [[1,4], [2,5], [3,6]]
            assertThat(output).containsExactly(1.0f, 4.0f, 2.0f, 5.0f, 3.0f, 6.0f);
        }
    }

    @Test
    @DisplayName("Should transpose square matrix")
    void testTransposeSquare() {
        try (Matrix matrix = new Matrix()) {
            float[] input = {1.0f, 2.0f, 3.0f, 4.0f}; // [[1,2], [3,4]]
            float[] output = new float[4];

            MatrixResult opResult = matrix.transpose(input, output, 2, 2);

            assertThat(opResult.isSuccess()).isTrue();
            // [[1,3], [2,4]]
            assertThat(output).containsExactly(1.0f, 3.0f, 2.0f, 4.0f);
        }
    }

    @Test
    @DisplayName("Should compute vector magnitude")
    void testMagnitude() {
        try (Matrix matrix = new Matrix()) {
            float[] vector = {3.0f, 4.0f};

            MatrixResult result = matrix.magnitude(vector);

            assertThat(result.isSuccess()).isTrue();
            assertThat(result.getResult()).isEqualTo(5.0, within(DELTA));
        }
    }

    @Test
    @DisplayName("Should compute magnitude for zero vector")
    void testMagnitudeZero() {
        try (Matrix matrix = new Matrix()) {
            float[] vector = {0.0f, 0.0f, 0.0f};

            MatrixResult result = matrix.magnitude(vector);

            assertThat(result.isSuccess()).isTrue();
            assertThat(result.getResult()).isEqualTo(0.0, within(DELTA));
        }
    }

    @Test
    @DisplayName("Should handle large vectors")
    void testLargeVectors() {
        try (Matrix matrix = new Matrix()) {
            int size = 1000;
            float[] a = new float[size];
            float[] b = new float[size];

            for (int i = 0; i < size; i++) {
                a[i] = 1.0f;
                b[i] = 2.0f;
            }

            MatrixResult result = matrix.dotProduct(a, b);

            assertThat(result.isSuccess()).isTrue();
            assertThat(result.getResult()).isEqualTo(2000.0, within(DELTA));
        }
    }

    @Test
    @DisplayName("Should handle edge case with single element")
    void testSingleElement() {
        try (Matrix matrix = new Matrix()) {
            float[] a = {5.0f};
            float[] b = {3.0f};

            MatrixResult result = matrix.dotProduct(a, b);

            assertThat(result.isSuccess()).isTrue();
            assertThat(result.getResult()).isEqualTo(15.0, within(DELTA));
        }
    }
}

