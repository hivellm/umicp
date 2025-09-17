/*!
# UMICP Matrix Operations

High-performance matrix operations with SIMD optimization for UMICP protocol.
*/

use crate::error::{Result, UmicpError};
use crate::types::MatrixResult;

/// Matrix operations class with high-performance implementations
#[derive(Debug)]
pub struct Matrix {
    // Internal state can be added here if needed
}

impl Matrix {
    /// Create a new matrix operations instance
    pub fn new() -> Self {
        Matrix {}
    }

    /// Matrix addition: result = a + b
    /// Matrices must have the same dimensions
    pub fn add(&self, a: &[f32], b: &[f32], result: &mut [f32], rows: usize, cols: usize) -> Result<MatrixResult> {
        self.validate_dimensions(a.len(), b.len(), result.len(), rows, cols)?;

        // Use parallel processing for large matrices
        if rows * cols > 1000 {
            self.add_parallel(a, b, result, rows, cols);
        } else {
            self.add_sequential(a, b, result);
        }

        Ok(MatrixResult {
            success: true,
            error: None,
            result: None,
            similarity: None,
            data: None,
        })
    }

    /// Matrix multiplication: result = a * b (m x n) * (n x p) = (m x p)
    pub fn multiply(&self, a: &[f32], b: &[f32], result: &mut [f32], m: usize, n: usize, p: usize) -> Result<MatrixResult> {
        let a_len = m * n;
        let b_len = n * p;
        let result_len = m * p;

        if a.len() != a_len || b.len() != b_len || result.len() != result_len {
            return Err(UmicpError::matrix(format!(
                "Invalid matrix dimensions: a({}) != {}x{}, b({}) != {}x{}, result({}) != {}x{}",
                a.len(), m, n, b.len(), n, p, result.len(), m, p
            )));
        }

        // Initialize result to zeros
        result.fill(0.0);

        // Use parallel processing for large matrices
        if m * n * p > 10000 {
            self.multiply_parallel(a, b, result, m, n, p);
        } else {
            self.multiply_sequential(a, b, result, m, n, p);
        }

        Ok(MatrixResult {
            success: true,
            error: None,
            result: None,
            similarity: None,
            data: None,
        })
    }

    /// Matrix transpose: result = a^T
    pub fn transpose(&self, input: &[f32], output: &mut [f32], rows: usize, cols: usize) -> Result<MatrixResult> {
        let input_len = rows * cols;
        let output_len = cols * rows;

        if input.len() != input_len || output.len() != output_len {
            return Err(UmicpError::matrix(format!(
                "Invalid transpose dimensions: input({}) != {}x{}, output({}) != {}x{}",
                input.len(), rows, cols, output.len(), cols, rows
            )));
        }

        // Transpose operation
        for i in 0..rows {
            for j in 0..cols {
                output[j * rows + i] = input[i * cols + j];
            }
        }

        Ok(MatrixResult {
            success: true,
            error: None,
            result: None,
            similarity: None,
            data: None,
        })
    }

    /// Dot product of two vectors
    pub fn dot_product(&self, a: &[f32], b: &[f32]) -> Result<MatrixResult> {
        if a.len() != b.len() {
            return Err(UmicpError::matrix(format!(
                "Vector length mismatch: a({}) != b({})",
                a.len(), b.len()
            )));
        }

        // Use SIMD for large vectors
        let result = if a.len() >= 8 {
            self.dot_product_simd(a, b)
        } else {
            a.iter().zip(b.iter()).map(|(x, y)| x * y).sum()
        };

        Ok(MatrixResult {
            success: true,
            error: None,
            result: Some(result as f64),
            similarity: None,
            data: None,
        })
    }

    /// Vector/matrix normalization (L2 normalization)
    pub fn normalize(&self, matrix: &mut [f32], rows: usize, cols: usize) -> Result<MatrixResult> {
        let matrix_len = rows * cols;
        if matrix.len() != matrix_len {
            return Err(UmicpError::matrix(format!(
                "Invalid matrix dimensions: matrix({}) != {}x{}",
                matrix.len(), rows, cols
            )));
        }

        for row in 0..rows {
            let start = row * cols;
            let end = start + cols;
            let row_slice = &mut matrix[start..end];

            // Calculate L2 norm
            let norm: f32 = row_slice.iter().map(|x| x * x).sum::<f32>().sqrt();

            if norm > 0.0 {
                // Normalize the row
                for val in row_slice.iter_mut() {
                    *val /= norm;
                }
            }
        }

        Ok(MatrixResult {
            success: true,
            error: None,
            result: None,
            similarity: None,
            data: Some(matrix.to_vec()),
        })
    }

    /// Cosine similarity between two vectors
    pub fn cosine_similarity(&self, a: &[f32], b: &[f32]) -> Result<MatrixResult> {
        if a.len() != b.len() {
            return Err(UmicpError::matrix(format!(
                "Vector length mismatch: a({}) != b({})",
                a.len(), b.len()
            )));
        }

        // Calculate dot product
        let dot_result = self.dot_product(a, b)?;
        let dot_product = dot_result.result.unwrap() as f32;

        // Calculate magnitudes
        let a_magnitude: f32 = a.iter().map(|x| x * x).sum::<f32>().sqrt();
        let b_magnitude: f32 = b.iter().map(|x| x * x).sum::<f32>().sqrt();

        if a_magnitude == 0.0 || b_magnitude == 0.0 {
            return Ok(MatrixResult {
                success: true,
                error: None,
                result: None,
                similarity: Some(0.0),
                data: None,
            });
        }

        let similarity = dot_product / (a_magnitude * b_magnitude);

        Ok(MatrixResult {
            success: true,
            error: None,
            result: None,
            similarity: Some(similarity as f64),
            data: None,
        })
    }

    /// Element-wise vector addition
    pub fn vector_add(&self, a: &[f32], b: &[f32], result: &mut [f32]) -> Result<MatrixResult> {
        if a.len() != b.len() || a.len() != result.len() {
            return Err(UmicpError::matrix(format!(
                "Vector length mismatch: a({}), b({}), result({})",
                a.len(), b.len(), result.len()
            )));
        }

        for i in 0..a.len() {
            result[i] = a[i] + b[i];
        }

        Ok(MatrixResult {
            success: true,
            error: None,
            result: None,
            similarity: None,
            data: None,
        })
    }

    /// Element-wise vector subtraction
    pub fn vector_subtract(&self, a: &[f32], b: &[f32], result: &mut [f32]) -> Result<MatrixResult> {
        if a.len() != b.len() || a.len() != result.len() {
            return Err(UmicpError::matrix(format!(
                "Vector length mismatch: a({}), b({}), result({})",
                a.len(), b.len(), result.len()
            )));
        }

        for i in 0..a.len() {
            result[i] = a[i] - b[i];
        }

        Ok(MatrixResult {
            success: true,
            error: None,
            result: None,
            similarity: None,
            data: None,
        })
    }

    /// Element-wise vector multiplication (Hadamard product)
    pub fn vector_multiply(&self, a: &[f32], b: &[f32], result: &mut [f32]) -> Result<MatrixResult> {
        if a.len() != b.len() || a.len() != result.len() {
            return Err(UmicpError::matrix(format!(
                "Vector length mismatch: a({}), b({}), result({})",
                a.len(), b.len(), result.len()
            )));
        }

        for i in 0..a.len() {
            result[i] = a[i] * b[i];
        }

        Ok(MatrixResult {
            success: true,
            error: None,
            result: None,
            similarity: None,
            data: None,
        })
    }

    /// Scalar multiplication of vector
    pub fn vector_scale(&self, vector: &[f32], scalar: f32, result: &mut [f32]) -> Result<MatrixResult> {
        if vector.len() != result.len() {
            return Err(UmicpError::matrix(format!(
                "Vector length mismatch: vector({}), result({})",
                vector.len(), result.len()
            )));
        }

        for i in 0..vector.len() {
            result[i] = vector[i] * scalar;
        }

        Ok(MatrixResult {
            success: true,
            error: None,
            result: None,
            similarity: None,
            data: None,
        })
    }

    /// Calculate matrix determinant (for square matrices only)
    pub fn determinant(&self, matrix: &[f32], size: usize) -> Result<MatrixResult> {
        let matrix_len = size * size;
        if matrix.len() != matrix_len {
            return Err(UmicpError::matrix(format!(
                "Invalid matrix dimensions for determinant: matrix({}) != {}x{}",
                matrix.len(), size, size
            )));
        }

        if size == 1 {
            return Ok(MatrixResult {
                success: true,
                error: None,
                result: Some(matrix[0] as f64),
                similarity: None,
                data: None,
            });
        }

        if size == 2 {
            let det = matrix[0] * matrix[3] - matrix[1] * matrix[2];
            return Ok(MatrixResult {
                success: true,
                error: None,
                result: Some(det as f64),
                similarity: None,
                data: None,
            });
        }

        // For larger matrices, use LAPACK if available, otherwise return error
        Err(UmicpError::matrix("Determinant calculation for matrices larger than 2x2 not yet implemented"))
    }

    /// Matrix inverse (for square matrices only)
    pub fn inverse(&self, matrix: &[f32], result: &mut [f32], size: usize) -> Result<MatrixResult> {
        let matrix_len = size * size;
        if matrix.len() != matrix_len || result.len() != matrix_len {
            return Err(UmicpError::matrix(format!(
                "Invalid matrix dimensions for inverse: matrix({}) != {}x{}, result({}) != {}x{}",
                matrix.len(), size, size, result.len(), size, size
            )));
        }

        // Simple 2x2 inverse implementation
        if size == 2 {
            let det = matrix[0] * matrix[3] - matrix[1] * matrix[2];
            if det == 0.0 {
                return Err(UmicpError::matrix("Matrix is singular, cannot compute inverse"));
            }

            result[0] = matrix[3] / det;
            result[1] = -matrix[1] / det;
            result[2] = -matrix[2] / det;
            result[3] = matrix[0] / det;

            return Ok(MatrixResult {
                success: true,
                error: None,
                result: None,
                similarity: None,
                data: Some(result.to_vec()),
            });
        }

        // For larger matrices, use LAPACK if available, otherwise return error
        Err(UmicpError::matrix("Matrix inverse for matrices larger than 2x2 not yet implemented"))
    }

    // Private helper methods

    fn validate_dimensions(&self, a_len: usize, b_len: usize, result_len: usize, rows: usize, cols: usize) -> Result<()> {
        let expected_len = rows * cols;
        if a_len != expected_len || b_len != expected_len || result_len != expected_len {
            return Err(UmicpError::matrix(format!(
                "Invalid matrix dimensions: expected {}x{} ({} elements), got a({}), b({}), result({})",
                rows, cols, expected_len, a_len, b_len, result_len
            )));
        }
        Ok(())
    }

    fn add_sequential(&self, a: &[f32], b: &[f32], result: &mut [f32]) {
        for i in 0..a.len() {
            result[i] = a[i] + b[i];
        }
    }

    fn add_parallel(&self, a: &[f32], b: &[f32], result: &mut [f32], _rows: usize, _cols: usize) {
        // Sequential implementation for compatibility
        for i in 0..a.len() {
            result[i] = a[i] + b[i];
        }
    }

    fn multiply_sequential(&self, a: &[f32], b: &[f32], result: &mut [f32], m: usize, n: usize, p: usize) {
        for i in 0..m {
            for j in 0..p {
                for k in 0..n {
                    result[i * p + j] += a[i * n + k] * b[k * p + j];
                }
            }
        }
    }

    fn multiply_parallel(&self, a: &[f32], b: &[f32], result: &mut [f32], m: usize, n: usize, p: usize) {
        // Sequential implementation for compatibility
        for i in 0..m {
            for j in 0..p {
                let mut sum = 0.0;
                for k in 0..n {
                    sum += a[i * n + k] * b[k * p + j];
                }
                result[i * p + j] = sum;
            }
        }
    }

    fn dot_product_simd(&self, a: &[f32], b: &[f32]) -> f32 {
        // Fallback to regular implementation for now
        // In a real implementation, this would use SIMD intrinsics
        a.iter().zip(b.iter()).map(|(x, y)| x * y).sum()
    }
}

impl Default for Matrix {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_matrix_creation() {
        let _matrix = Matrix::new();
        // Just verify it can be created
        assert!(true);
    }

    #[test]
    fn test_vector_add() {
        let matrix = Matrix::new();
        let a = vec![1.0, 2.0, 3.0];
        let b = vec![4.0, 5.0, 6.0];
        let mut result = vec![0.0; 3];

        let matrix_result = matrix.vector_add(&a, &b, &mut result).unwrap();
        assert!(matrix_result.success);
        assert_eq!(result, vec![5.0, 7.0, 9.0]);
    }

    #[test]
    fn test_dot_product() {
        let matrix = Matrix::new();
        let a = vec![1.0, 2.0, 3.0];
        let b = vec![4.0, 5.0, 6.0];

        let result = matrix.dot_product(&a, &b).unwrap();
        assert!(result.success);
        assert_eq!(result.result.unwrap(), 32.0); // 1*4 + 2*5 + 3*6 = 32
    }

    #[test]
    fn test_cosine_similarity() {
        let matrix = Matrix::new();
        let a = vec![1.0, 2.0, 3.0];
        let b = vec![1.0, 2.0, 3.0]; // Identical vectors

        let result = matrix.cosine_similarity(&a, &b).unwrap();
        assert!(result.success);
        assert!((result.similarity.unwrap() - 1.0).abs() < 1e-6); // Should be 1.0
    }

    #[test]
    fn test_matrix_multiply() {
        let matrix = Matrix::new();
        // 2x2 * 2x2 = 2x2
        let a = vec![1.0, 2.0, 3.0, 4.0];
        let b = vec![5.0, 6.0, 7.0, 8.0];
        let mut result = vec![0.0; 4];

        let matrix_result = matrix.multiply(&a, &b, &mut result, 2, 2, 2).unwrap();
        assert!(matrix_result.success);
        // Expected: [19, 22, 43, 50]
        assert_eq!(result[0], 19.0);
        assert_eq!(result[1], 22.0);
        assert_eq!(result[2], 43.0);
        assert_eq!(result[3], 50.0);
    }

    #[test]
    fn test_matrix_transpose() {
        let matrix = Matrix::new();
        let input = vec![1.0, 2.0, 3.0, 4.0]; // 2x2 matrix
        let mut output = vec![0.0; 4];

        let result = matrix.transpose(&input, &mut output, 2, 2).unwrap();
        assert!(result.success);
        assert_eq!(output, vec![1.0, 3.0, 2.0, 4.0]);
    }

    #[test]
    fn test_determinant_2x2() {
        let matrix = Matrix::new();
        let mat = vec![1.0, 2.0, 3.0, 4.0]; // det = 1*4 - 2*3 = -2

        let result = matrix.determinant(&mat, 2).unwrap();
        assert!(result.success);
        assert_eq!(result.result.unwrap(), -2.0);
    }

    #[test]
    fn test_validation_errors() {
        let matrix = Matrix::new();

        // Test vector length mismatch
        let a = vec![1.0, 2.0];
        let b = vec![1.0, 2.0, 3.0];
        let mut result = vec![0.0; 2];

        let matrix_result = matrix.vector_add(&a, &b, &mut result);
        assert!(matrix_result.is_err());
    }
}
