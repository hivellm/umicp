import Foundation
import UMICP

func matrixOperationsExample() throws {
    print("=== Matrix Operations Example ===\n")

    let ops = UMICP.matrixOps()

    // Vector addition
    print("Vector Addition:")
    let v1 = [1.0, 2.0, 3.0, 4.0]
    let v2 = [5.0, 6.0, 7.0, 8.0]
    let sum = try ops.vectorAdd(v1, v2)
    print("  \(v1) + \(v2) = \(sum)")

    // Vector subtraction
    print("\nVector Subtraction:")
    let diff = try ops.vectorSubtract(v2, v1)
    print("  \(v2) - \(v1) = \(diff)")

    // Scalar multiplication
    print("\nScalar Multiplication:")
    let scaled = ops.vectorScale(v1, scalar: 2.0)
    print("  \(v1) * 2.0 = \(scaled)")

    // Dot product
    print("\nDot Product:")
    let dot = try ops.dotProduct(v1, v2)
    print("  \(v1) · \(v2) = \(dot)")

    // Vector magnitude
    print("\nVector Magnitude:")
    let testVector = [3.0, 4.0]
    let magnitude = ops.vectorMagnitude(testVector)
    print("  ||\(testVector)|| = \(magnitude)")

    // Vector normalization
    print("\nVector Normalization:")
    let normalized = try ops.vectorNormalize(testVector)
    print("  normalize(\(testVector)) = \(normalized)")
    print("  magnitude of normalized = \(ops.vectorMagnitude(normalized))")

    // Matrix multiplication
    print("\nMatrix Multiplication (2x2):")
    let m1 = [1.0, 2.0, 3.0, 4.0]
    let m2 = [5.0, 6.0, 7.0, 8.0]
    print("  [[1, 2],     [[5, 6],")
    print("   [3, 4]]  *   [7, 8]]")

    let result = try ops.matrixMultiply(m1, m2, rows: 2, cols: 2, k: 2)
    print("  = [[%g, %g],", result[0], result[1])
    print("     [%g, %g]]", result[2], result[3])

    // Larger matrix multiplication
    print("\nMatrix Multiplication (3x3):")
    let a = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0]
    let b = [9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0]
    let matResult = try ops.matrixMultiply(a, b, rows: 3, cols: 3, k: 3)
    print("  Result: \(matResult)")
}

// Run example
do {
    try matrixOperationsExample()
    print("\n✅ Matrix operations example completed!")
} catch {
    print("❌ Error: \(error)")
}

