package com.hivellm.umicp.core

import io.kotest.matchers.floats.shouldBeGreaterThan
import io.kotest.matchers.shouldBe
import io.kotest.matchers.types.shouldBeInstanceOf
import org.junit.jupiter.api.Test
import kotlin.math.abs

class MatrixTest {

    @Test
    fun `should compute dot product`() {
        val a = floatArrayOf(1f, 2f, 3f)
        val b = floatArrayOf(4f, 5f, 6f)

        val result = Matrix.dotProduct(a, b)
        result.shouldBeInstanceOf<MatrixResult.Success<Float>>()
        result.getOrThrow() shouldBe 32f // 1*4 + 2*5 + 3*6
    }

    @Test
    fun `should fail dot product with mismatched dimensions`() {
        val a = floatArrayOf(1f, 2f, 3f)
        val b = floatArrayOf(4f, 5f)

        val result = Matrix.dotProduct(a, b)
        result.shouldBeInstanceOf<MatrixResult.Failure>()
    }

    @Test
    fun `should compute cosine similarity`() {
        val a = floatArrayOf(1f, 2f, 3f)
        val b = floatArrayOf(4f, 5f, 6f)

        val result = Matrix.cosineSimilarity(a, b)
        result.shouldBeInstanceOf<MatrixResult.Success<Float>>()

        val similarity = result.getOrThrow()
        similarity shouldBeGreaterThan 0.9f // Should be ~0.9746
    }

    @Test
    fun `should compute magnitude`() {
        val vector = floatArrayOf(3f, 4f) // 3-4-5 triangle

        val result = Matrix.magnitude(vector)
        result.shouldBeInstanceOf<MatrixResult.Success<Float>>()
        result.getOrThrow() shouldBe 5f
    }

    @Test
    fun `should normalize vector`() {
        val vector = floatArrayOf(3f, 4f)

        val result = Matrix.normalize(vector)
        result.shouldBeInstanceOf<MatrixResult.Success<FloatArray>>()

        val normalized = result.getOrThrow()
        normalized[0] shouldBe 0.6f
        normalized[1] shouldBe 0.8f
    }

    @Test
    fun `should add vectors`() {
        val a = floatArrayOf(1f, 2f, 3f)
        val b = floatArrayOf(4f, 5f, 6f)

        val result = Matrix.vectorAdd(a, b)
        result.shouldBeInstanceOf<MatrixResult.Success<FloatArray>>()

        val sum = result.getOrThrow()
        sum shouldBe floatArrayOf(5f, 7f, 9f)
    }

    @Test
    fun `should subtract vectors`() {
        val a = floatArrayOf(4f, 5f, 6f)
        val b = floatArrayOf(1f, 2f, 3f)

        val result = Matrix.vectorSubtract(a, b)
        result.shouldBeInstanceOf<MatrixResult.Success<FloatArray>>()

        val diff = result.getOrThrow()
        diff shouldBe floatArrayOf(3f, 3f, 3f)
    }

    @Test
    fun `should multiply vectors element-wise`() {
        val a = floatArrayOf(1f, 2f, 3f)
        val b = floatArrayOf(4f, 5f, 6f)

        val result = Matrix.vectorMultiply(a, b)
        result.shouldBeInstanceOf<MatrixResult.Success<FloatArray>>()

        val product = result.getOrThrow()
        product shouldBe floatArrayOf(4f, 10f, 18f)
    }

    @Test
    fun `should multiply matrices`() {
        // 2x2 matrices
        val a = floatArrayOf(1f, 2f, 3f, 4f)
        val b = floatArrayOf(5f, 6f, 7f, 8f)

        val result = Matrix.matrixMultiply(a, b, 2, 2, 2)
        result.shouldBeInstanceOf<MatrixResult.Success<FloatArray>>()

        val product = result.getOrThrow()
        // Expected: [1*5+2*7, 1*6+2*8, 3*5+4*7, 3*6+4*8] = [19, 22, 43, 50]
        product shouldBe floatArrayOf(19f, 22f, 43f, 50f)
    }

    @Test
    fun `should transpose matrix`() {
        // 2x3 matrix
        val matrix = floatArrayOf(1f, 2f, 3f, 4f, 5f, 6f)

        val result = Matrix.transpose(matrix, 2, 3)
        result.shouldBeInstanceOf<MatrixResult.Success<FloatArray>>()

        val transposed = result.getOrThrow()
        // Expected: 3x2 matrix [1, 4, 2, 5, 3, 6]
        transposed shouldBe floatArrayOf(1f, 4f, 2f, 5f, 3f, 6f)
    }

    @Test
    fun `should use extension function for dot product`() {
        val a = floatArrayOf(1f, 2f, 3f)
        val b = floatArrayOf(4f, 5f, 6f)

        val result = a dot b
        result shouldBe 32f
    }

    @Test
    fun `should use extension function for cosine similarity`() {
        val a = floatArrayOf(1f, 2f, 3f)
        val b = floatArrayOf(4f, 5f, 6f)

        val similarity = a cosineSim b
        similarity shouldBeGreaterThan 0.9f
    }

    @Test
    fun `should use extension function for magnitude`() {
        val vector = floatArrayOf(3f, 4f)
        vector.magnitude() shouldBe 5f
    }

    @Test
    fun `should use extension function for normalize`() {
        val vector = floatArrayOf(3f, 4f)
        val normalized = vector.normalize()

        abs(normalized[0] - 0.6f) shouldBe 0f
        abs(normalized[1] - 0.8f) shouldBe 0f
    }

    @Test
    fun `should use operator overloading for vector addition`() {
        val a = floatArrayOf(1f, 2f, 3f)
        val b = floatArrayOf(4f, 5f, 6f)

        val sum = a + b
        sum shouldBe floatArrayOf(5f, 7f, 9f)
    }

    @Test
    fun `should use operator overloading for vector subtraction`() {
        val a = floatArrayOf(4f, 5f, 6f)
        val b = floatArrayOf(1f, 2f, 3f)

        val diff = a - b
        diff shouldBe floatArrayOf(3f, 3f, 3f)
    }

    @Test
    fun `should use operator overloading for vector multiplication`() {
        val a = floatArrayOf(1f, 2f, 3f)
        val b = floatArrayOf(4f, 5f, 6f)

        val product = a * b
        product shouldBe floatArrayOf(4f, 10f, 18f)
    }
}

