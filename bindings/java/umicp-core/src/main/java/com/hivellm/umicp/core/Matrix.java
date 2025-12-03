package com.hivellm.umicp.core;

import org.jetbrains.annotations.NotNull;

/**
 * High-performance matrix and vector operations.
 *
 * <p>This class provides optimized operations for matrix and vector computations,
 * commonly used in machine learning and data processing pipelines. Operations include:
 * <ul>
 *   <li>Vector addition, subtraction, multiplication</li>
 *   <li>Dot product</li>
 *   <li>Cosine similarity</li>
 *   <li>Matrix multiplication</li>
 *   <li>Matrix transpose</li>
 *   <li>Vector normalization</li>
 * </ul>
 *
 * <p>Example usage:
 * <pre>{@code
 * try (Matrix matrix = new Matrix()) {
 *     // Dot product
 *     float[] a = {1.0f, 2.0f, 3.0f};
 *     float[] b = {4.0f, 5.0f, 6.0f};
 *     MatrixResult result = matrix.dotProduct(a, b);
 *     System.out.println("Dot product: " + result.getResult()); // 32.0
 *
 *     // Cosine similarity
 *     MatrixResult sim = matrix.cosineSimilarity(a, b);
 *     System.out.println("Similarity: " + sim.getSimilarity()); // 0.9746...
 *
 *     // Vector addition
 *     float[] sum = new float[3];
 *     matrix.vectorAdd(a, b, sum);
 *     System.out.println("Sum: " + Arrays.toString(sum)); // [5.0, 7.0, 9.0]
 * }
 * }</pre>
 *
 * <p>This is a pure Java implementation. For better performance, consider using
 * the JNI-backed version which leverages native SIMD acceleration.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see MatrixResult
 */
public class Matrix implements AutoCloseable {

    private static final double EPSILON = 1e-10;

    /**
     * Creates a new Matrix instance.
     */
    public Matrix() {
        // Pure Java implementation - no native resources
    }

    /**
     * Adds two vectors element-wise: result = a + b
     *
     * @param a the first vector
     * @param b the second vector
     * @param result the output vector
     * @return the operation result
     */
    @NotNull
    public MatrixResult vectorAdd(@NotNull float[] a, @NotNull float[] b, @NotNull float[] result) {
        if (a.length != b.length || a.length != result.length) {
            return MatrixResult.failure("Vector dimensions must match");
        }

        try {
            for (int i = 0; i < a.length; i++) {
                result[i] = a[i] + b[i];
            }
            return MatrixResult.success();
        } catch (Exception e) {
            return MatrixResult.failure("Vector addition failed: " + e.getMessage());
        }
    }

    /**
     * Subtracts two vectors element-wise: result = a - b
     *
     * @param a the first vector
     * @param b the second vector
     * @param result the output vector
     * @return the operation result
     */
    @NotNull
    public MatrixResult vectorSubtract(@NotNull float[] a, @NotNull float[] b, @NotNull float[] result) {
        if (a.length != b.length || a.length != result.length) {
            return MatrixResult.failure("Vector dimensions must match");
        }

        try {
            for (int i = 0; i < a.length; i++) {
                result[i] = a[i] - b[i];
            }
            return MatrixResult.success();
        } catch (Exception e) {
            return MatrixResult.failure("Vector subtraction failed: " + e.getMessage());
        }
    }

    /**
     * Multiplies two vectors element-wise: result = a * b
     *
     * @param a the first vector
     * @param b the second vector
     * @param result the output vector
     * @return the operation result
     */
    @NotNull
    public MatrixResult vectorMultiply(@NotNull float[] a, @NotNull float[] b, @NotNull float[] result) {
        if (a.length != b.length || a.length != result.length) {
            return MatrixResult.failure("Vector dimensions must match");
        }

        try {
            for (int i = 0; i < a.length; i++) {
                result[i] = a[i] * b[i];
            }
            return MatrixResult.success();
        } catch (Exception e) {
            return MatrixResult.failure("Vector multiplication failed: " + e.getMessage());
        }
    }

    /**
     * Computes the dot product of two vectors: a · b
     *
     * @param a the first vector
     * @param b the second vector
     * @return the operation result containing the dot product
     */
    @NotNull
    public MatrixResult dotProduct(@NotNull float[] a, @NotNull float[] b) {
        if (a.length != b.length) {
            return MatrixResult.failure("Vector dimensions must match");
        }

        try {
            double sum = 0.0;
            for (int i = 0; i < a.length; i++) {
                sum += a[i] * b[i];
            }
            return MatrixResult.success(sum);
        } catch (Exception e) {
            return MatrixResult.failure("Dot product failed: " + e.getMessage());
        }
    }

    /**
     * Computes the cosine similarity between two vectors: (a · b) / (||a|| * ||b||)
     *
     * <p>The result is in the range [-1, 1], where:
     * <ul>
     *   <li>1 indicates identical direction</li>
     *   <li>0 indicates orthogonal vectors</li>
     *   <li>-1 indicates opposite direction</li>
     * </ul>
     *
     * @param a the first vector
     * @param b the second vector
     * @return the operation result containing the similarity score
     */
    @NotNull
    public MatrixResult cosineSimilarity(@NotNull float[] a, @NotNull float[] b) {
        if (a.length != b.length) {
            return MatrixResult.failure("Vector dimensions must match");
        }

        try {
            double dotProduct = 0.0;
            double normA = 0.0;
            double normB = 0.0;

            for (int i = 0; i < a.length; i++) {
                dotProduct += a[i] * b[i];
                normA += a[i] * a[i];
                normB += b[i] * b[i];
            }

            normA = Math.sqrt(normA);
            normB = Math.sqrt(normB);

            if (normA < EPSILON || normB < EPSILON) {
                return MatrixResult.failure("Zero-magnitude vector encountered");
            }

            double similarity = dotProduct / (normA * normB);
            return MatrixResult.successWithSimilarity(similarity);
        } catch (Exception e) {
            return MatrixResult.failure("Cosine similarity failed: " + e.getMessage());
        }
    }

    /**
     * Normalizes a vector to unit length (L2 normalization).
     *
     * @param vector the vector to normalize (modified in-place)
     * @return the operation result
     */
    @NotNull
    public MatrixResult normalize(@NotNull float[] vector) {
        try {
            double norm = 0.0;
            for (float v : vector) {
                norm += v * v;
            }
            norm = Math.sqrt(norm);

            if (norm < EPSILON) {
                return MatrixResult.failure("Cannot normalize zero-magnitude vector");
            }

            for (int i = 0; i < vector.length; i++) {
                vector[i] = (float) (vector[i] / norm);
            }

            return MatrixResult.success();
        } catch (Exception e) {
            return MatrixResult.failure("Normalization failed: " + e.getMessage());
        }
    }

    /**
     * Multiplies two matrices: result = A * B
     *
     * <p>Matrix A is m×n, matrix B is n×p, result is m×p.
     *
     * @param a the first matrix (row-major order)
     * @param b the second matrix (row-major order)
     * @param result the output matrix (row-major order)
     * @param m number of rows in A
     * @param n number of columns in A (rows in B)
     * @param p number of columns in B
     * @return the operation result
     */
    @NotNull
    public MatrixResult matrixMultiply(@NotNull float[] a, @NotNull float[] b,
                                      @NotNull float[] result, int m, int n, int p) {
        if (a.length != m * n) {
            return MatrixResult.failure("Matrix A dimensions don't match: expected " + (m*n) + ", got " + a.length);
        }
        if (b.length != n * p) {
            return MatrixResult.failure("Matrix B dimensions don't match: expected " + (n*p) + ", got " + b.length);
        }
        if (result.length != m * p) {
            return MatrixResult.failure("Result dimensions don't match: expected " + (m*p) + ", got " + result.length);
        }

        try {
            // Initialize result to zero
            for (int i = 0; i < result.length; i++) {
                result[i] = 0.0f;
            }

            // Perform matrix multiplication
            for (int i = 0; i < m; i++) {
                for (int j = 0; j < p; j++) {
                    for (int k = 0; k < n; k++) {
                        result[i * p + j] += a[i * n + k] * b[k * p + j];
                    }
                }
            }

            return MatrixResult.success();
        } catch (Exception e) {
            return MatrixResult.failure("Matrix multiplication failed: " + e.getMessage());
        }
    }

    /**
     * Transposes a matrix: result = A^T
     *
     * @param input the input matrix (row-major order)
     * @param output the output matrix (row-major order)
     * @param rows number of rows in input
     * @param cols number of columns in input
     * @return the operation result
     */
    @NotNull
    public MatrixResult transpose(@NotNull float[] input, @NotNull float[] output,
                                  int rows, int cols) {
        if (input.length != rows * cols) {
            return MatrixResult.failure("Input dimensions don't match: expected " + (rows*cols) + ", got " + input.length);
        }
        if (output.length != rows * cols) {
            return MatrixResult.failure("Output dimensions don't match: expected " + (rows*cols) + ", got " + output.length);
        }

        try {
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    output[j * rows + i] = input[i * cols + j];
                }
            }
            return MatrixResult.success();
        } catch (Exception e) {
            return MatrixResult.failure("Matrix transpose failed: " + e.getMessage());
        }
    }

    /**
     * Computes the magnitude (L2 norm) of a vector.
     *
     * @param vector the input vector
     * @return the operation result containing the magnitude
     */
    @NotNull
    public MatrixResult magnitude(@NotNull float[] vector) {
        try {
            double sum = 0.0;
            for (float v : vector) {
                sum += v * v;
            }
            return MatrixResult.success(Math.sqrt(sum));
        } catch (Exception e) {
            return MatrixResult.failure("Magnitude calculation failed: " + e.getMessage());
        }
    }

    /**
     * Closes this matrix instance and releases any resources.
     *
     * <p>In the pure Java implementation, this method does nothing.
     * It's provided for compatibility with JNI-based implementations.
     */
    @Override
    public void close() {
        // No resources to release in pure Java implementation
    }
}

