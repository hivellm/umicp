package com.hivellm.umicp.core

import com.hivellm.umicp.types.MatrixException
import kotlin.math.sqrt

/**
 * Result wrapper for matrix operations
 */
sealed class MatrixResult<out T> {
    data class Success<T>(val value: T) : MatrixResult<T>()
    data class Failure(val error: String) : MatrixResult<Nothing>()

    val isSuccess: Boolean get() = this is Success
    val isFailure: Boolean get() = this is Failure

    fun getOrNull(): T? = when (this) {
        is Success -> value
        is Failure -> null
    }

    fun getOrThrow(): T = when (this) {
        is Success -> value
        is Failure -> throw MatrixException(error)
    }

    inline fun <R> map(transform: (T) -> R): MatrixResult<R> = when (this) {
        is Success -> Success(transform(value))
        is Failure -> this
    }

    inline fun onSuccess(action: (T) -> Unit): MatrixResult<T> {
        if (this is Success) action(value)
        return this
    }

    inline fun onFailure(action: (String) -> Unit): MatrixResult<T> {
        if (this is Failure) action(error)
        return this
    }
}

/**
 * Matrix operations for UMICP
 */
object Matrix {
    /**
     * Compute dot product of two vectors
     *
     * @param a First vector
     * @param b Second vector
     * @return Dot product result
     */
    fun dotProduct(a: FloatArray, b: FloatArray): MatrixResult<Float> {
        if (a.size != b.size) {
            return MatrixResult.Failure("Vector dimensions must match: ${a.size} != ${b.size}")
        }

        var sum = 0f
        for (i in a.indices) {
            sum += a[i] * b[i]
        }

        return MatrixResult.Success(sum)
    }

    /**
     * Compute cosine similarity between two vectors
     *
     * @param a First vector
     * @param b Second vector
     * @return Cosine similarity in range [-1, 1]
     */
    fun cosineSimilarity(a: FloatArray, b: FloatArray): MatrixResult<Float> {
        if (a.size != b.size) {
            return MatrixResult.Failure("Vector dimensions must match: ${a.size} != ${b.size}")
        }

        var dotProd = 0f
        var magA = 0f
        var magB = 0f

        for (i in a.indices) {
            dotProd += a[i] * b[i]
            magA += a[i] * a[i]
            magB += b[i] * b[i]
        }

        magA = sqrt(magA)
        magB = sqrt(magB)

        if (magA == 0f || magB == 0f) {
            return MatrixResult.Failure("Cannot compute similarity with zero magnitude vector")
        }

        return MatrixResult.Success(dotProd / (magA * magB))
    }

    /**
     * Compute magnitude (L2 norm) of a vector
     *
     * @param vector Input vector
     * @return Magnitude
     */
    fun magnitude(vector: FloatArray): MatrixResult<Float> {
        var sum = 0f
        for (value in vector) {
            sum += value * value
        }
        return MatrixResult.Success(sqrt(sum))
    }

    /**
     * Normalize a vector to unit length
     *
     * @param vector Input vector
     * @return Normalized vector
     */
    fun normalize(vector: FloatArray): MatrixResult<FloatArray> {
        val mag = magnitude(vector).getOrThrow()

        if (mag == 0f) {
            return MatrixResult.Failure("Cannot normalize zero vector")
        }

        val normalized = FloatArray(vector.size) { i -> vector[i] / mag }
        return MatrixResult.Success(normalized)
    }

    /**
     * Add two vectors element-wise
     *
     * @param a First vector
     * @param b Second vector
     * @return Result vector
     */
    fun vectorAdd(a: FloatArray, b: FloatArray): MatrixResult<FloatArray> {
        if (a.size != b.size) {
            return MatrixResult.Failure("Vector dimensions must match: ${a.size} != ${b.size}")
        }

        val result = FloatArray(a.size) { i -> a[i] + b[i] }
        return MatrixResult.Success(result)
    }

    /**
     * Subtract two vectors element-wise
     *
     * @param a First vector
     * @param b Second vector
     * @return Result vector
     */
    fun vectorSubtract(a: FloatArray, b: FloatArray): MatrixResult<FloatArray> {
        if (a.size != b.size) {
            return MatrixResult.Failure("Vector dimensions must match: ${a.size} != ${b.size}")
        }

        val result = FloatArray(a.size) { i -> a[i] - b[i] }
        return MatrixResult.Success(result)
    }

    /**
     * Multiply two vectors element-wise (Hadamard product)
     *
     * @param a First vector
     * @param b Second vector
     * @return Result vector
     */
    fun vectorMultiply(a: FloatArray, b: FloatArray): MatrixResult<FloatArray> {
        if (a.size != b.size) {
            return MatrixResult.Failure("Vector dimensions must match: ${a.size} != ${b.size}")
        }

        val result = FloatArray(a.size) { i -> a[i] * b[i] }
        return MatrixResult.Success(result)
    }

    /**
     * Matrix multiplication: C = A × B
     *
     * @param a Matrix A (m × n)
     * @param b Matrix B (n × p)
     * @param m Number of rows in A
     * @param n Number of columns in A (rows in B)
     * @param p Number of columns in B
     * @return Result matrix C (m × p)
     */
    fun matrixMultiply(
        a: FloatArray,
        b: FloatArray,
        m: Int,
        n: Int,
        p: Int
    ): MatrixResult<FloatArray> {
        if (a.size != m * n) {
            return MatrixResult.Failure("Matrix A size mismatch: expected ${m * n}, got ${a.size}")
        }
        if (b.size != n * p) {
            return MatrixResult.Failure("Matrix B size mismatch: expected ${n * p}, got ${b.size}")
        }

        val result = FloatArray(m * p)

        for (i in 0 until m) {
            for (j in 0 until p) {
                var sum = 0f
                for (k in 0 until n) {
                    sum += a[i * n + k] * b[k * p + j]
                }
                result[i * p + j] = sum
            }
        }

        return MatrixResult.Success(result)
    }

    /**
     * Transpose a matrix
     *
     * @param matrix Input matrix (rows × cols)
     * @param rows Number of rows
     * @param cols Number of columns
     * @return Transposed matrix (cols × rows)
     */
    fun transpose(matrix: FloatArray, rows: Int, cols: Int): MatrixResult<FloatArray> {
        if (matrix.size != rows * cols) {
            return MatrixResult.Failure("Matrix size mismatch: expected ${rows * cols}, got ${matrix.size}")
        }

        val result = FloatArray(rows * cols)

        for (i in 0 until rows) {
            for (j in 0 until cols) {
                result[j * rows + i] = matrix[i * cols + j]
            }
        }

        return MatrixResult.Success(result)
    }
}

/**
 * Extension functions for convenient matrix operations
 */

/**
 * Compute dot product with another vector
 */
infix fun FloatArray.dot(other: FloatArray): Float =
    Matrix.dotProduct(this, other).getOrThrow()

/**
 * Compute cosine similarity with another vector
 */
infix fun FloatArray.cosineSim(other: FloatArray): Float =
    Matrix.cosineSimilarity(this, other).getOrThrow()

/**
 * Compute magnitude of vector
 */
fun FloatArray.magnitude(): Float =
    Matrix.magnitude(this).getOrThrow()

/**
 * Normalize vector to unit length
 */
fun FloatArray.normalize(): FloatArray =
    Matrix.normalize(this).getOrThrow()

/**
 * Add two vectors
 */
operator fun FloatArray.plus(other: FloatArray): FloatArray =
    Matrix.vectorAdd(this, other).getOrThrow()

/**
 * Subtract two vectors
 */
operator fun FloatArray.minus(other: FloatArray): FloatArray =
    Matrix.vectorSubtract(this, other).getOrThrow()

/**
 * Element-wise multiplication
 */
operator fun FloatArray.times(other: FloatArray): FloatArray =
    Matrix.vectorMultiply(this, other).getOrThrow()

