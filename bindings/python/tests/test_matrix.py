"""Tests for Matrix module."""

import pytest
import numpy as np

from umicp import Matrix, MatrixOperationError


class TestMatrix:
    """Test Matrix class."""

    def test_dot_product(self):
        """Test vector dot product."""
        matrix = Matrix()
        v1 = np.array([1.0, 2.0, 3.0], dtype=np.float32)
        v2 = np.array([4.0, 5.0, 6.0], dtype=np.float32)

        result = matrix.dot_product(v1, v2)
        assert result.result == 32.0  # 1*4 + 2*5 + 3*6

    def test_cosine_similarity(self):
        """Test cosine similarity."""
        matrix = Matrix()
        v1 = np.array([1.0, 0.0], dtype=np.float32)
        v2 = np.array([1.0, 0.0], dtype=np.float32)

        result = matrix.cosine_similarity(v1, v2)
        assert abs(result.similarity - 1.0) < 0.001  # Identical vectors

    def test_matrix_multiply(self):
        """Test matrix multiplication."""
        matrix = Matrix()
        m1 = np.array([[1, 2], [3, 4]], dtype=np.float32)
        m2 = np.array([[5, 6], [7, 8]], dtype=np.float32)

        result = matrix.multiply(m1, m2)
        expected = np.array([[19, 22], [43, 50]], dtype=np.float32)

        np.testing.assert_array_equal(result.result, expected)

    def test_matrix_add(self):
        """Test matrix addition."""
        matrix = Matrix()
        m1 = np.array([[1, 2], [3, 4]], dtype=np.float32)
        m2 = np.array([[5, 6], [7, 8]], dtype=np.float32)

        result = matrix.add(m1, m2)
        expected = np.array([[6, 8], [10, 12]], dtype=np.float32)

        np.testing.assert_array_equal(result.result, expected)

    def test_transpose(self):
        """Test matrix transpose."""
        matrix = Matrix()
        m = np.array([[1, 2, 3], [4, 5, 6]], dtype=np.float32)

        result = matrix.transpose(m)
        assert result.result.shape == (3, 2)

    def test_normalize(self):
        """Test vector normalization."""
        matrix = Matrix()
        v = np.array([3.0, 4.0], dtype=np.float32)

        result = matrix.normalize(v)
        norm = np.linalg.norm(result)

        assert abs(norm - 1.0) < 0.001  # Unit vector

    def test_determinant(self):
        """Test matrix determinant."""
        matrix = Matrix()
        m = np.array([[1, 2], [3, 4]], dtype=np.float32)

        det = matrix.determinant(m)
        assert abs(det - (-2.0)) < 0.001  # 1*4 - 2*3 = -2

