"""Performance benchmarks for UMICP Matrix operations."""

import time
import numpy as np
from umicp.matrix import Matrix


def benchmark_dot_product(sizes=[100, 1000, 10000, 100000]):
    """Benchmark dot product operations."""
    print("\n=== Dot Product Benchmark ===")
    matrix = Matrix()

    for size in sizes:
        a = np.random.rand(size).astype(np.float32).tolist()
        b = np.random.rand(size).astype(np.float32).tolist()

        # Warmup
        for _ in range(10):
            matrix.dot_product(a, b)

        # Benchmark
        start = time.perf_counter()
        iterations = 1000
        for _ in range(iterations):
            result = matrix.dot_product(a, b)
        end = time.perf_counter()

        elapsed = end - start
        ops_per_sec = iterations / elapsed

        print(f"Size {size:>6}: {ops_per_sec:>10,.0f} ops/sec, "
              f"{elapsed/iterations*1000:.3f} ms/op")


def benchmark_cosine_similarity(sizes=[100, 1000, 10000, 100000]):
    """Benchmark cosine similarity operations."""
    print("\n=== Cosine Similarity Benchmark ===")
    matrix = Matrix()

    for size in sizes:
        a = np.random.rand(size).astype(np.float32).tolist()
        b = np.random.rand(size).astype(np.float32).tolist()

        # Warmup
        for _ in range(10):
            matrix.cosine_similarity(a, b)

        # Benchmark
        start = time.perf_counter()
        iterations = 1000
        for _ in range(iterations):
            result = matrix.cosine_similarity(a, b)
        end = time.perf_counter()

        elapsed = end - start
        ops_per_sec = iterations / elapsed

        print(f"Size {size:>6}: {ops_per_sec:>10,.0f} ops/sec, "
              f"{elapsed/iterations*1000:.3f} ms/op")


def benchmark_vector_add(sizes=[100, 1000, 10000, 100000]):
    """Benchmark vector addition operations."""
    print("\n=== Vector Addition Benchmark ===")
    matrix = Matrix()

    for size in sizes:
        a = np.random.rand(size).astype(np.float32).tolist()
        b = np.random.rand(size).astype(np.float32).tolist()

        # Warmup
        for _ in range(10):
            matrix.vector_add(a, b)

        # Benchmark
        start = time.perf_counter()
        iterations = 1000
        for _ in range(iterations):
            result = matrix.vector_add(a, b)
        end = time.perf_counter()

        elapsed = end - start
        ops_per_sec = iterations / elapsed

        print(f"Size {size:>6}: {ops_per_sec:>10,.0f} ops/sec, "
              f"{elapsed/iterations*1000:.3f} ms/op")


def benchmark_matrix_multiply():
    """Benchmark matrix multiplication operations."""
    print("\n=== Matrix Multiplication Benchmark ===")
    matrix = Matrix()

    sizes = [(10, 10, 10), (50, 50, 50), (100, 100, 100)]

    for m, n, p in sizes:
        a = np.random.rand(m * n).astype(np.float32).tolist()
        b = np.random.rand(n * p).astype(np.float32).tolist()

        # Warmup
        for _ in range(5):
            matrix.matrix_multiply(a, b, m, n, p)

        # Benchmark
        start = time.perf_counter()
        iterations = 100
        for _ in range(iterations):
            result = matrix.matrix_multiply(a, b, m, n, p)
        end = time.perf_counter()

        elapsed = end - start
        ops_per_sec = iterations / elapsed

        print(f"Size ({m}x{n}) x ({n}x{p}): {ops_per_sec:>8,.0f} ops/sec, "
              f"{elapsed/iterations*1000:.3f} ms/op")


def main():
    """Run all benchmarks."""
    print("=" * 60)
    print("UMICP Matrix Performance Benchmarks")
    print("=" * 60)

    benchmark_dot_product()
    benchmark_cosine_similarity()
    benchmark_vector_add()
    benchmark_matrix_multiply()

    print("\n" + "=" * 60)


if __name__ == "__main__":
    main()

