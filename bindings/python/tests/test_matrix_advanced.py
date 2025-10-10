"""Advanced tests for matrix module."""

import pytest
import numpy as np

from umicp import Matrix, MatrixOperationError


class TestMatrixAdvanced:
    """Advanced matrix operation tests."""

    def test_vector_operations_different_dtypes(self):
        """Test vector operations with different data types."""
        matrix = Matrix()

        # float32
        v1_f32 = np.array([1.0, 2.0, 3.0], dtype=np.float32)
        v2_f32 = np.array([4.0, 5.0, 6.0], dtype=np.float32)
        result_f32 = matrix.vector_add(v1_f32, v2_f32)
        np.testing.assert_array_equal(result_f32, [5.0, 7.0, 9.0])

        # float64
        v1_f64 = np.array([1.0, 2.0, 3.0], dtype=np.float64)
        v2_f64 = np.array([4.0, 5.0, 6.0], dtype=np.float64)
        result_f64 = matrix.vector_add(v1_f64, v2_f64)
        np.testing.assert_array_equal(result_f64, [5.0, 7.0, 9.0])

    def test_matrix_operations_edge_cases(self):
        """Test matrix operations with edge cases."""
        matrix = Matrix()

        # 1x1 matrix
        m1 = np.array([[5.0]], dtype=np.float32)
        m2 = np.array([[3.0]], dtype=np.float32)
        result = matrix.multiply(m1, m2)
        assert result.result[0, 0] == 15.0

        # Large matrix
        large = np.random.rand(100, 100).astype(np.float32)
        result = matrix.transpose(large)
        assert result.result.shape == (100, 100)

    def test_incompatible_shapes_dot_product(self):
        """Test dot product with incompatible shapes."""
        matrix = Matrix()
        v1 = np.array([1.0, 2.0], dtype=np.float32)
        v2 = np.array([1.0, 2.0, 3.0], dtype=np.float32)

        with pytest.raises(MatrixOperationError):
            matrix.dot_product(v1, v2)

    def test_incompatible_shapes_cosine(self):
        """Test cosine similarity with incompatible shapes."""
        matrix = Matrix()
        v1 = np.array([1.0, 2.0], dtype=np.float32)
        v2 = np.array([1.0, 2.0, 3.0], dtype=np.float32)

        with pytest.raises(MatrixOperationError):
            matrix.cosine_similarity(v1, v2)

    def test_zero_vector_cosine(self):
        """Test cosine similarity with zero vector."""
        matrix = Matrix()
        v1 = np.array([0.0, 0.0, 0.0], dtype=np.float32)
        v2 = np.array([1.0, 2.0, 3.0], dtype=np.float32)

        with pytest.raises(MatrixOperationError):
            matrix.cosine_similarity(v1, v2)

    def test_zero_vector_normalize(self):
        """Test normalizing zero vector."""
        matrix = Matrix()
        v = np.array([0.0, 0.0], dtype=np.float32)

        with pytest.raises(MatrixOperationError):
            matrix.normalize(v)

    def test_incompatible_matrix_add(self):
        """Test matrix addition with incompatible shapes."""
        matrix = Matrix()
        m1 = np.array([[1, 2], [3, 4]], dtype=np.float32)
        m2 = np.array([[1, 2, 3]], dtype=np.float32)

        with pytest.raises(MatrixOperationError):
            matrix.add(m1, m2)

    def test_incompatible_matrix_multiply(self):
        """Test matrix multiplication with incompatible shapes."""
        matrix = Matrix()
        m1 = np.array([[1, 2], [3, 4]], dtype=np.float32)  # 2x2
        m2 = np.array([[1, 2, 3]], dtype=np.float32)  # 1x3

        with pytest.raises(MatrixOperationError):
            matrix.multiply(m1, m2)

    def test_non_square_determinant(self):
        """Test determinant of non-square matrix."""
        matrix = Matrix()
        m = np.array([[1, 2, 3], [4, 5, 6]], dtype=np.float32)  # 2x3

        with pytest.raises(MatrixOperationError):
            matrix.determinant(m)

    def test_vector_subtract(self):
        """Test vector subtraction."""
        matrix = Matrix()
        v1 = np.array([5.0, 7.0, 9.0], dtype=np.float32)
        v2 = np.array([1.0, 2.0, 3.0], dtype=np.float32)

        result = matrix.vector_subtract(v1, v2)
        np.testing.assert_array_equal(result, [4.0, 5.0, 6.0])

    def test_vector_scale_positive(self):
        """Test vector scaling with positive scalar."""
        matrix = Matrix()
        v = np.array([1.0, 2.0, 3.0], dtype=np.float32)

        result = matrix.vector_scale(v, 3.0)
        np.testing.assert_array_equal(result, [3.0, 6.0, 9.0])

    def test_vector_scale_negative(self):
        """Test vector scaling with negative scalar."""
        matrix = Matrix()
        v = np.array([1.0, 2.0, 3.0], dtype=np.float32)

        result = matrix.vector_scale(v, -2.0)
        np.testing.assert_array_equal(result, [-2.0, -4.0, -6.0])

    def test_vector_scale_zero(self):
        """Test vector scaling with zero."""
        matrix = Matrix()
        v = np.array([1.0, 2.0, 3.0], dtype=np.float32)

        result = matrix.vector_scale(v, 0.0)
        np.testing.assert_array_equal(result, [0.0, 0.0, 0.0])

    def test_matrix_inverse(self):
        """Test matrix inverse."""
        matrix = Matrix()
        m = np.array([[1, 2], [3, 4]], dtype=np.float32)

        result = matrix.inverse(m)

        # Verify A * A^-1 = I
        identity = np.matmul(m, result.result)
        expected = np.eye(2, dtype=np.float32)
        np.testing.assert_array_almost_equal(identity, expected, decimal=5)

    def test_singular_matrix_inverse(self):
        """Test inverse of singular matrix."""
        matrix = Matrix()
        # Singular matrix (determinant = 0)
        m = np.array([[1, 2], [2, 4]], dtype=np.float32)

        with pytest.raises(MatrixOperationError):
            matrix.inverse(m)

    def test_3x3_determinant(self):
        """Test determinant of 3x3 matrix."""
        matrix = Matrix()
        m = np.array([
            [1, 2, 3],
            [4, 5, 6],
            [7, 8, 9]
        ], dtype=np.float32)

        det = matrix.determinant(m)
        # This is a singular matrix, det should be close to 0
        assert abs(det) < 0.001

    def test_identity_matrix_determinant(self):
        """Test determinant of identity matrix."""
        matrix = Matrix()
        m = np.eye(3, dtype=np.float32)

        det = matrix.determinant(m)
        assert abs(det - 1.0) < 0.001

    def test_cosine_similarity_orthogonal(self):
        """Test cosine similarity of orthogonal vectors."""
        matrix = Matrix()
        v1 = np.array([1.0, 0.0], dtype=np.float32)
        v2 = np.array([0.0, 1.0], dtype=np.float32)

        result = matrix.cosine_similarity(v1, v2)
        assert abs(result.similarity) < 0.001  # Should be ~0

    def test_cosine_similarity_opposite(self):
        """Test cosine similarity of opposite vectors."""
        matrix = Matrix()
        v1 = np.array([1.0, 0.0], dtype=np.float32)
        v2 = np.array([-1.0, 0.0], dtype=np.float32)

        result = matrix.cosine_similarity(v1, v2)
        assert abs(result.similarity - (-1.0)) < 0.001  # Should be -1

    def test_normalize_various_vectors(self):
        """Test normalization of various vectors."""
        matrix = Matrix()

        # Test vector 1
        v1 = np.array([3.0, 4.0], dtype=np.float32)
        norm1 = matrix.normalize(v1)
        assert abs(np.linalg.norm(norm1) - 1.0) < 0.001

        # Test vector 2
        v2 = np.array([1.0, 1.0, 1.0], dtype=np.float32)
        norm2 = matrix.normalize(v2)
        assert abs(np.linalg.norm(norm2) - 1.0) < 0.001

