#!/usr/bin/env python3
"""Matrix operations example."""

import numpy as np
from umicp import Matrix


def main():
    """Demonstrate matrix operations."""
    print("=== UMICP Python - Matrix Operations Example ===\n")

    matrix = Matrix()

    # Vector operations
    print("1. Vector Operations")
    v1 = np.array([1.0, 2.0, 3.0, 4.0], dtype=np.float32)
    v2 = np.array([5.0, 6.0, 7.0, 8.0], dtype=np.float32)

    print(f"   Vector 1: {v1}")
    print(f"   Vector 2: {v2}")

    # Dot product
    dot_result = matrix.dot_product(v1, v2)
    print(f"\n   Dot product: {dot_result.result}")

    # Cosine similarity
    similarity = matrix.cosine_similarity(v1, v2)
    print(f"   Cosine similarity: {similarity.similarity:.4f}")

    # Vector operations
    v_add = matrix.vector_add(v1, v2)
    print(f"   Addition: {v_add}")

    v_sub = matrix.vector_subtract(v1, v2)
    print(f"   Subtraction: {v_sub}")

    v_scale = matrix.vector_scale(v1, 2.0)
    print(f"   Scaled by 2: {v_scale}")

    v_norm = matrix.normalize(v1)
    print(f"   Normalized: {v_norm}")

    # Matrix operations
    print("\n2. Matrix Operations")
    m1 = np.array([[1, 2], [3, 4]], dtype=np.float32)
    m2 = np.array([[5, 6], [7, 8]], dtype=np.float32)

    print(f"   Matrix 1:\n{m1}")
    print(f"   Matrix 2:\n{m2}")

    # Matrix multiplication
    result = matrix.multiply(m1, m2)
    print(f"\n   Product:\n{result.result}")

    # Matrix addition
    result = matrix.add(m1, m2)
    print(f"\n   Sum:\n{result.result}")

    # Transpose
    result = matrix.transpose(m1)
    print(f"\n   Transpose of M1:\n{result.result}")

    # Determinant
    det = matrix.determinant(m1)
    print(f"\n   Determinant of M1: {det}")

    print("\n✅ Example completed successfully!")


if __name__ == "__main__":
    main()

