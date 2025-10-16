import Foundation

#if canImport(Accelerate)
import Accelerate
#endif

/// Matrix and vector operations for UMICP
public struct MatrixOperations {
    public init() {}
    
    /// Add two vectors
    public func vectorAdd(_ a: [Double], _ b: [Double]) throws -> [Double] {
        guard a.count == b.count else {
            throw UMICPError.validationError("Vector dimensions must match")
        }
        
        #if canImport(Accelerate)
        var result = [Double](repeating: 0, count: a.count)
        var aInput = a
        var bInput = b
        vDSP_vaddD(&aInput, 1, &bInput, 1, &result, 1, vDSP_Length(a.count))
        return result
        #else
        return zip(a, b).map { $0 + $1 }
        #endif
    }
    
    /// Subtract two vectors
    public func vectorSubtract(_ a: [Double], _ b: [Double]) throws -> [Double] {
        guard a.count == b.count else {
            throw UMICPError.validationError("Vector dimensions must match")
        }
        
        #if canImport(Accelerate)
        var result = [Double](repeating: 0, count: a.count)
        var aInput = a
        var bInput = b
        vDSP_vsubD(&bInput, 1, &aInput, 1, &result, 1, vDSP_Length(a.count))
        return result
        #else
        return zip(a, b).map { $0 - $1 }
        #endif
    }
    
    /// Multiply vector by scalar
    public func vectorScale(_ vector: [Double], scalar: Double) -> [Double] {
        #if canImport(Accelerate)
        var result = [Double](repeating: 0, count: vector.count)
        var input = vector
        var scalarInput = scalar
        vDSP_vsmulD(&input, 1, &scalarInput, &result, 1, vDSP_Length(vector.count))
        return result
        #else
        return vector.map { $0 * scalar }
        #endif
    }
    
    /// Calculate dot product of two vectors
    public func dotProduct(_ a: [Double], _ b: [Double]) throws -> Double {
        guard a.count == b.count else {
            throw UMICPError.validationError("Vector dimensions must match")
        }
        
        #if canImport(Accelerate)
        var result: Double = 0
        var aInput = a
        var bInput = b
        vDSP_dotprD(&aInput, 1, &bInput, 1, &result, vDSP_Length(a.count))
        return result
        #else
        return zip(a, b).map { $0 * $1 }.reduce(0, +)
        #endif
    }
    
    /// Multiply two matrices
    /// - Parameters:
    ///   - a: First matrix (rows x cols)
    ///   - b: Second matrix (cols x k)
    ///   - rows: Number of rows in matrix A
    ///   - cols: Number of columns in A / rows in B
    ///   - k: Number of columns in matrix B
    /// - Returns: Resulting matrix (rows x k)
    public func matrixMultiply(
        _ a: [Double],
        _ b: [Double],
        rows: Int,
        cols: Int,
        k: Int
    ) throws -> [Double] {
        guard a.count == rows * cols else {
            throw UMICPError.validationError("Matrix A dimensions incorrect")
        }
        guard b.count == cols * k else {
            throw UMICPError.validationError("Matrix B dimensions incorrect")
        }
        
        #if canImport(Accelerate)
        var result = [Double](repeating: 0, count: rows * k)
        var aInput = a
        var bInput = b
        
        vDSP_mmulD(
            &aInput, 1,
            &bInput, 1,
            &result, 1,
            vDSP_Length(rows),
            vDSP_Length(k),
            vDSP_Length(cols)
        )
        return result
        #else
        // Fallback implementation
        var result = [Double](repeating: 0, count: rows * k)
        for i in 0..<rows {
            for j in 0..<k {
                var sum: Double = 0
                for m in 0..<cols {
                    sum += a[i * cols + m] * b[m * k + j]
                }
                result[i * k + j] = sum
            }
        }
        return result
        #endif
    }
    
    /// Calculate vector magnitude (Euclidean norm)
    public func vectorMagnitude(_ vector: [Double]) -> Double {
        #if canImport(Accelerate)
        var input = vector
        var result: Double = 0
        vDSP_dotprD(&input, 1, &input, 1, &result, vDSP_Length(vector.count))
        return sqrt(result)
        #else
        return sqrt(vector.map { $0 * $0 }.reduce(0, +))
        #endif
    }
    
    /// Normalize vector to unit length
    public func vectorNormalize(_ vector: [Double]) throws -> [Double] {
        let magnitude = vectorMagnitude(vector)
        guard magnitude > 0 else {
            throw UMICPError.validationError("Cannot normalize zero vector")
        }
        return vectorScale(vector, scalar: 1.0 / magnitude)
    }
}

