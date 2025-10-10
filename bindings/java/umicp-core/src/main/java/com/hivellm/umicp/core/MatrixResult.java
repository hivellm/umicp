package com.hivellm.umicp.core;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.Arrays;
import java.util.Objects;

/**
 * Result wrapper for matrix operations.
 *
 * <p>This class encapsulates the result of matrix/vector operations, including
 * success status, error messages, and various result types (scalar, array, similarity).
 *
 * <p>Example usage:
 * <pre>{@code
 * try (Matrix matrix = new Matrix()) {
 *     float[] a = {1.0f, 2.0f, 3.0f};
 *     float[] b = {4.0f, 5.0f, 6.0f};
 *
 *     MatrixResult result = matrix.dotProduct(a, b);
 *
 *     if (result.isSuccess()) {
 *         double dotProduct = result.getResult();
 *         System.out.println("Dot product: " + dotProduct);
 *     } else {
 *         System.err.println("Error: " + result.getError());
 *     }
 * }
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see Matrix
 */
public class MatrixResult {

    private final boolean success;
    private final String error;
    private final Double result;
    private final Double similarity;
    private final float[] data;

    /**
     * Private constructor. Use static factory methods to create instances.
     */
    private MatrixResult(boolean success, String error, Double result,
                        Double similarity, float[] data) {
        this.success = success;
        this.error = error;
        this.result = result;
        this.similarity = similarity;
        this.data = data;
    }

    /**
     * Creates a successful result with a scalar value.
     *
     * @param result the result value
     * @return a successful MatrixResult
     */
    @NotNull
    public static MatrixResult success(double result) {
        return new MatrixResult(true, null, result, null, null);
    }

    /**
     * Creates a successful result with similarity score.
     *
     * @param similarity the similarity value (typically 0.0 to 1.0)
     * @return a successful MatrixResult
     */
    @NotNull
    public static MatrixResult successWithSimilarity(double similarity) {
        return new MatrixResult(true, null, null, similarity, null);
    }

    /**
     * Creates a successful result with array data.
     *
     * @param data the result data array
     * @return a successful MatrixResult
     */
    @NotNull
    public static MatrixResult successWithData(@NotNull float[] data) {
        return new MatrixResult(true, null, null, null, Arrays.copyOf(data, data.length));
    }

    /**
     * Creates a successful result with no data (void operation).
     *
     * @return a successful MatrixResult
     */
    @NotNull
    public static MatrixResult success() {
        return new MatrixResult(true, null, null, null, null);
    }

    /**
     * Creates a failed result with an error message.
     *
     * @param error the error message
     * @return a failed MatrixResult
     */
    @NotNull
    public static MatrixResult failure(@NotNull String error) {
        return new MatrixResult(false, error, null, null, null);
    }

    /**
     * Returns whether the operation was successful.
     *
     * @return true if successful, false otherwise
     */
    public boolean isSuccess() {
        return success;
    }

    /**
     * Returns whether the operation failed.
     *
     * @return true if failed, false otherwise
     */
    public boolean isFailure() {
        return !success;
    }

    /**
     * Returns the error message if the operation failed.
     *
     * @return the error message, or null if successful
     */
    @Nullable
    public String getError() {
        return error;
    }

    /**
     * Returns the scalar result value.
     *
     * @return the result value, or null if not applicable
     */
    @Nullable
    public Double getResult() {
        return result;
    }

    /**
     * Returns the similarity score.
     *
     * @return the similarity value, or null if not applicable
     */
    @Nullable
    public Double getSimilarity() {
        return similarity;
    }

    /**
     * Returns the result data array.
     *
     * @return a copy of the data array, or null if not applicable
     */
    @Nullable
    public float[] getData() {
        return data != null ? Arrays.copyOf(data, data.length) : null;
    }

    /**
     * Returns the result value or throws if the operation failed.
     *
     * @return the result value
     * @throws IllegalStateException if the operation failed or result is not available
     */
    public double getResultOrThrow() {
        if (!success) {
            throw new IllegalStateException("Operation failed: " + error);
        }
        if (result == null) {
            throw new IllegalStateException("No scalar result available");
        }
        return result;
    }

    /**
     * Returns the similarity value or throws if the operation failed.
     *
     * @return the similarity value
     * @throws IllegalStateException if the operation failed or similarity is not available
     */
    public double getSimilarityOrThrow() {
        if (!success) {
            throw new IllegalStateException("Operation failed: " + error);
        }
        if (similarity == null) {
            throw new IllegalStateException("No similarity value available");
        }
        return similarity;
    }

    /**
     * Returns the data array or throws if the operation failed.
     *
     * @return a copy of the data array
     * @throws IllegalStateException if the operation failed or data is not available
     */
    @NotNull
    public float[] getDataOrThrow() {
        if (!success) {
            throw new IllegalStateException("Operation failed: " + error);
        }
        if (data == null) {
            throw new IllegalStateException("No data array available");
        }
        return Arrays.copyOf(data, data.length);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        MatrixResult that = (MatrixResult) o;
        return success == that.success &&
               Objects.equals(error, that.error) &&
               Objects.equals(result, that.result) &&
               Objects.equals(similarity, that.similarity) &&
               Arrays.equals(data, that.data);
    }

    @Override
    public int hashCode() {
        int hash = Objects.hash(success, error, result, similarity);
        hash = 31 * hash + Arrays.hashCode(data);
        return hash;
    }

    @Override
    public String toString() {
        if (!success) {
            return "MatrixResult{success=false, error='" + error + "'}";
        }

        StringBuilder sb = new StringBuilder("MatrixResult{success=true");
        if (result != null) {
            sb.append(", result=").append(result);
        }
        if (similarity != null) {
            sb.append(", similarity=").append(similarity);
        }
        if (data != null) {
            sb.append(", data.length=").append(data.length);
        }
        sb.append('}');
        return sb.toString();
    }
}

