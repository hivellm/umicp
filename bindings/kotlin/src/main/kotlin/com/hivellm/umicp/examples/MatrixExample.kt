package com.hivellm.umicp.examples

import com.hivellm.umicp.core.*

/**
 * Matrix Operations Example
 *
 * Demonstrates:
 * - Vector operations (dot product, cosine similarity)
 * - Matrix operations (multiplication, transpose)
 * - Extension functions
 * - Operator overloading
 */
fun main() {
    println("=== UMICP Kotlin SDK - Matrix Operations Example ===\n")

    // Vector operations
    println("1. Vector Operations:")
    val vecA = floatArrayOf(1f, 2f, 3f)
    val vecB = floatArrayOf(4f, 5f, 6f)

    println("  Vector A: ${vecA.contentToString()}")
    println("  Vector B: ${vecB.contentToString()}")
    println()

    // Dot product
    println("  a) Dot Product:")
    val dotResult = Matrix.dotProduct(vecA, vecB)
    dotResult.onSuccess { result ->
        println("     Result: $result")
        println("     Expected: ${1*4 + 2*5 + 3*6} ✓")
    }
    println()

    // Using extension function
    println("  b) Dot Product (extension):")
    val dotExt = vecA dot vecB
    println("     Result: $dotExt")
    println()

    // Cosine similarity
    println("  c) Cosine Similarity:")
    val simResult = Matrix.cosineSimilarity(vecA, vecB)
    simResult.onSuccess { similarity ->
        println("     Similarity: ${"%.4f".format(similarity)}")
        println("     Range: [-1, 1]")
    }
    println()

    // Magnitude
    println("  d) Magnitude:")
    val vec = floatArrayOf(3f, 4f)
    val mag = vec.magnitude()
    println("     Vector: ${vec.contentToString()}")
    println("     Magnitude: $mag")
    println("     Expected: 5.0 (3-4-5 triangle) ✓")
    println()

    // Normalization
    println("  e) Normalization:")
    val normalized = vec.normalize()
    println("     Original: ${vec.contentToString()}")
    println("     Normalized: ${normalized.contentToString()}")
    println("     New magnitude: ${"%.4f".format(normalized.magnitude())}")
    println()

    // Vector arithmetic with operator overloading
    println("2. Vector Arithmetic (Operator Overloading):")
    val v1 = floatArrayOf(1f, 2f, 3f)
    val v2 = floatArrayOf(4f, 5f, 6f)

    println("  Vector 1: ${v1.contentToString()}")
    println("  Vector 2: ${v2.contentToString()}")
    println()

    println("  a) Addition (v1 + v2):")
    val sum = v1 + v2
    println("     Result: ${sum.contentToString()}")
    println()

    println("  b) Subtraction (v2 - v1):")
    val diff = v2 - v1
    println("     Result: ${diff.contentToString()}")
    println()

    println("  c) Element-wise Multiplication (v1 * v2):")
    val product = v1 * v2
    println("     Result: ${product.contentToString()}")
    println()

    // Matrix operations
    println("3. Matrix Operations:")

    println("  a) Matrix Multiplication:")
    val matA = floatArrayOf(1f, 2f, 3f, 4f) // 2x2
    val matB = floatArrayOf(5f, 6f, 7f, 8f) // 2x2

    println("     Matrix A (2x2): ${matA.contentToString()}")
    println("     Matrix B (2x2): ${matB.contentToString()}")

    val mulResult = Matrix.matrixMultiply(matA, matB, 2, 2, 2)
    mulResult.onSuccess { result ->
        println("     Result (2x2): ${result.contentToString()}")
        println("     Expected: [19, 22, 43, 50] ✓")
    }
    println()

    println("  b) Matrix Transpose:")
    val matrix = floatArrayOf(1f, 2f, 3f, 4f, 5f, 6f) // 2x3
    println("     Original (2x3): ${matrix.contentToString()}")

    val transposeResult = Matrix.transpose(matrix, 2, 3)
    transposeResult.onSuccess { transposed ->
        println("     Transposed (3x2): ${transposed.contentToString()}")
    }
    println()

    // ML Use Case: Embedding similarity
    println("4. ML Use Case - Embedding Similarity:")

    // Simulated embeddings (768-dimensional)
    val embedding1 = FloatArray(768) { it.toFloat() / 768f }
    val embedding2 = FloatArray(768) { (it + 10).toFloat() / 768f }

    println("  Computing similarity between 768-dimensional embeddings...")

    val similarity = embedding1 cosineSim embedding2
    println("  Cosine Similarity: ${"%.6f".format(similarity)}")

    if (similarity > 0.9f) {
        println("  ✓ High similarity - documents are related")
    } else if (similarity > 0.5f) {
        println("  ~ Moderate similarity")
    } else {
        println("  ✗ Low similarity - documents are unrelated")
    }
    println()

    // Error handling
    println("5. Error Handling:")
    val shortVec = floatArrayOf(1f, 2f)
    val longVec = floatArrayOf(1f, 2f, 3f)

    val errorResult = Matrix.dotProduct(shortVec, longVec)
    errorResult.onFailure { error ->
        println("  Expected error: $error ✓")
    }
    println()

    println("=== Example Complete ===")
}

