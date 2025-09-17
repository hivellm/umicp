/**
 * UMICP Matrix Operations Performance Tests
 * Comprehensive performance benchmarking for SIMD operations
 */

#include <gtest/gtest.h>
#include "matrix_ops.h"
#include "../utils/test_helpers.h"
#include <vector>
#include <random>
#include <iomanip>

using namespace umicp;
using namespace umicp::testing;

class MatrixPerformanceTest : public UMICPPerformanceTest {
protected:
    void SetUp() override {
        UMICPPerformanceTest::SetUp();

        // Various test sizes for scalability testing
        test_sizes_ = {16, 64, 256, 1024, 4096, 16384};

        // Pre-generate test data for consistency
        for (size_t size : test_sizes_) {
            vec_a_[size] = TestHelpers::generate_random_vector(size, 12345);
            vec_b_[size] = TestHelpers::generate_random_vector(size, 54321);
            results_[size] = std::vector<float>(size);
        }

        // Large matrices for multiplication tests
        matrix_sizes_ = {32, 64, 128, 256};
        for (size_t size : matrix_sizes_) {
            size_t total_elements = size * size;
            matrix_a_[size] = TestHelpers::generate_random_vector(total_elements, 11111);
            matrix_b_[size] = TestHelpers::generate_random_vector(total_elements, 22222);
            matrix_results_[size] = std::vector<float>(total_elements);
        }
    }

    std::vector<size_t> test_sizes_;
    std::vector<size_t> matrix_sizes_;

    std::map<size_t, std::vector<float>> vec_a_, vec_b_, results_;
    std::map<size_t, std::vector<float>> matrix_a_, matrix_b_, matrix_results_;
};

// ===============================================
// Vector Addition Performance Tests
// ===============================================

TEST_F(MatrixPerformanceTest, VectorAddition_ScalabilityTest) {
    std::cout << "\n  📊 Vector Addition Scalability:" << std::endl;
    std::cout << "    Size      |  Time (μs)  | Throughput (GB/s) | Elements/s" << std::endl;
    std::cout << "    ----------|-------------|-------------------|------------" << std::endl;

    for (size_t size : test_sizes_) {
        auto& a = vec_a_[size];
        auto& b = vec_b_[size];
        auto& result = results_[size];

        auto time_us = TestHelpers::benchmark_function([&]() {
            MatrixOps::add(a.data(), b.data(), result.data(), 1, size);
        }, 1000);

        // Calculate throughput (reading 2 arrays + writing 1 array = 3 * size * 4 bytes)
        size_t bytes_processed = 3 * size * sizeof(float);
        double throughput_gbps = (bytes_processed * 1000000.0) / (time_us * 1024.0 * 1024.0 * 1024.0);
        double elements_per_second = (size * 1000000.0) / time_us;

        std::cout << "    " << std::setw(8) << size
                  << " | " << std::setw(10) << std::fixed << std::setprecision(2) << time_us
                  << " | " << std::setw(16) << std::setprecision(3) << throughput_gbps
                  << " | " << std::setw(10) << std::scientific << std::setprecision(2) << elements_per_second
                  << std::endl;

        // Performance should scale reasonably (not linearly due to cache effects)
        EXPECT_LT(time_us, size * 0.01); // Very loose upper bound
    }
}

TEST_F(MatrixPerformanceTest, VectorAddition_SIMDEffectiveness) {
    std::cout << "\n  📊 SIMD Effectiveness Analysis:" << std::endl;

    // Test performance with different vector sizes to see SIMD benefits
    std::vector<size_t> simd_test_sizes = {15, 16, 31, 32, 63, 64, 127, 128, 255, 256, 511, 512};

    for (size_t size : simd_test_sizes) {
        auto vec_a = TestHelpers::generate_random_vector(size);
        auto vec_b = TestHelpers::generate_random_vector(size);
        std::vector<float> result(size);

        auto time_us = TestHelpers::benchmark_function([&]() {
            MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, size);
        }, 10000);

        double time_per_element = time_us / size;

        std::cout << "    Size " << std::setw(3) << size
                  << ": " << std::setw(6) << std::fixed << std::setprecision(3) << time_per_element
                  << " μs/element";

        // Highlight SIMD-friendly sizes
        if (size % 16 == 0 || size % 8 == 0) {
            std::cout << " ✨ (SIMD-aligned)";
        }
        std::cout << std::endl;
    }
}

// ===============================================
// Dot Product Performance Tests
// ===============================================

TEST_F(MatrixPerformanceTest, DotProduct_ScalabilityTest) {
    std::cout << "\n  📊 Dot Product Scalability:" << std::endl;
    std::cout << "    Size      |  Time (μs)  | GFLOPS | Elements/s" << std::endl;
    std::cout << "    ----------|-------------|--------|------------" << std::endl;

    for (size_t size : test_sizes_) {
        auto& a = vec_a_[size];
        auto& b = vec_b_[size];
        float result = 0.0f;

        auto time_us = TestHelpers::benchmark_function([&]() {
            MatrixOps::dot_product(a.data(), b.data(), &result, size);
        }, 1000);

        // Dot product performs size multiply-add operations = size FLOPs
        double gflops = (size * 1000000.0) / (time_us * 1000000000.0);
        double elements_per_second = (size * 1000000.0) / time_us;

        std::cout << "    " << std::setw(8) << size
                  << " | " << std::setw(10) << std::fixed << std::setprecision(2) << time_us
                  << " | " << std::setw(6) << std::setprecision(3) << gflops
                  << " | " << std::setw(10) << std::scientific << std::setprecision(2) << elements_per_second
                  << std::endl;
    }
}

TEST_F(MatrixPerformanceTest, DotProduct_CompareWithNaive) {
    std::cout << "\n  📊 Dot Product: SIMD vs Naive Implementation:" << std::endl;

    // Naive implementation for comparison
    auto naive_dot_product = [](const float* a, const float* b, size_t size) -> float {
        float result = 0.0f;
        for (size_t i = 0; i < size; ++i) {
            result += a[i] * b[i];
        }
        return result;
    };

    for (size_t size : {256, 1024, 4096}) {
        auto& a = vec_a_[size];
        auto& b = vec_b_[size];

        // UMICP optimized version
        float result_optimized = 0.0f;
        auto time_optimized = TestHelpers::benchmark_function([&]() {
            MatrixOps::dot_product(a.data(), b.data(), &result_optimized, size);
        }, 1000);

        // Naive version
        float result_naive = 0.0f;
        auto time_naive = TestHelpers::benchmark_function([&]() {
            result_naive = naive_dot_product(a.data(), b.data(), size);
        }, 1000);

        double speedup = time_naive / time_optimized;

        std::cout << "    Size " << std::setw(4) << size
                  << ": Optimized " << std::setw(8) << std::fixed << std::setprecision(2) << time_optimized << " μs"
                  << ", Naive " << std::setw(8) << time_naive << " μs"
                  << ", Speedup: " << std::setw(5) << std::setprecision(2) << speedup << "x" << std::endl;

        // Results should be approximately equal
        EXPECT_NEAR(result_optimized, result_naive, std::abs(result_naive) * 0.001f);

        // Optimized version should be faster (for larger sizes)
        if (size >= 1024) {
            EXPECT_GT(speedup, 1.0);
        }
    }
}

// ===============================================
// Matrix Multiplication Performance Tests
// ===============================================

TEST_F(MatrixPerformanceTest, MatrixMultiplication_ScalabilityTest) {
    std::cout << "\n  📊 Matrix Multiplication Scalability:" << std::endl;
    std::cout << "    Size      |  Time (ms)  | GFLOPS | Operations/s" << std::endl;
    std::cout << "    ----------|-------------|--------|-------------" << std::endl;

    for (size_t size : matrix_sizes_) {
        auto& a = matrix_a_[size];
        auto& b = matrix_b_[size];
        auto& result = matrix_results_[size];

        auto time_us = TestHelpers::benchmark_function([&]() {
            MatrixOps::multiply(a.data(), b.data(), result.data(), size, size, size);
        }, 10); // Fewer iterations for expensive operation

        double time_ms = time_us / 1000.0;

        // Matrix multiplication: size^3 multiply-add operations = 2 * size^3 FLOPs
        uint64_t flops = 2ULL * size * size * size;
        double gflops = (flops * 1000.0) / (time_us * 1000000.0);
        double ops_per_second = (flops * 1000000.0) / time_us;

        std::cout << "    " << std::setw(3) << size << "x" << size
                  << " | " << std::setw(10) << std::fixed << std::setprecision(2) << time_ms
                  << " | " << std::setw(6) << std::setprecision(3) << gflops
                  << " | " << std::setw(11) << std::scientific << std::setprecision(2) << ops_per_second
                  << std::endl;

        // Matrix multiplication should have cubic time complexity
        // For reasonable performance, should handle 128x128 in under 100ms
        if (size <= 128) {
            EXPECT_LT(time_ms, 100.0);
        }
    }
}

// ===============================================
// Matrix Transpose Performance Tests
// ===============================================

TEST_F(MatrixPerformanceTest, MatrixTranspose_ScalabilityTest) {
    std::cout << "\n  📊 Matrix Transpose Scalability:" << std::endl;
    std::cout << "    Size      |  Time (μs)  | Throughput (GB/s) | Elements/s" << std::endl;
    std::cout << "    ----------|-------------|-------------------|------------" << std::endl;

    for (size_t size : matrix_sizes_) {
        auto& input = matrix_a_[size];
        auto& output = matrix_results_[size];

        auto time_us = TestHelpers::benchmark_function([&]() {
            MatrixOps::transpose(input.data(), output.data(), size, size);
        }, 100);

        // Transpose reads and writes each element once
        size_t bytes_processed = 2 * size * size * sizeof(float);
        double throughput_gbps = (bytes_processed * 1000000.0) / (time_us * 1024.0 * 1024.0 * 1024.0);
        double elements_per_second = (size * size * 1000000.0) / time_us;

        std::cout << "    " << std::setw(3) << size << "x" << size
                  << " | " << std::setw(10) << std::fixed << std::setprecision(2) << time_us
                  << " | " << std::setw(16) << std::setprecision(3) << throughput_gbps
                  << " | " << std::setw(10) << std::scientific << std::setprecision(2) << elements_per_second
                  << std::endl;
    }
}

// ===============================================
// Vector Normalization Performance Tests
// ===============================================

TEST_F(MatrixPerformanceTest, VectorNormalization_ScalabilityTest) {
    std::cout << "\n  📊 Vector Normalization Scalability:" << std::endl;
    std::cout << "    Size      |  Time (μs)  | Elements/s | Vectors/s" << std::endl;
    std::cout << "    ----------|-------------|------------|----------" << std::endl;

    for (size_t size : test_sizes_) {
        auto test_vector = vec_a_[size]; // Copy so we can modify it

        auto time_us = TestHelpers::benchmark_function([&]() {
            MatrixOps::normalize(test_vector.data(), 1, size);
        }, 1000);

        double elements_per_second = (size * 1000000.0) / time_us;
        double vectors_per_second = 1000000.0 / time_us;

        std::cout << "    " << std::setw(8) << size
                  << " | " << std::setw(10) << std::fixed << std::setprecision(2) << time_us
                  << " | " << std::setw(10) << std::scientific << std::setprecision(2) << elements_per_second
                  << " | " << std::setw(8) << std::fixed << std::setprecision(1) << vectors_per_second
                  << std::endl;
    }
}

TEST_F(MatrixPerformanceTest, VectorNormalization_BatchProcessing) {
    std::cout << "\n  📊 Batch Vector Normalization:" << std::endl;

    const size_t vector_size = 256;
    const std::vector<size_t> batch_sizes = {1, 10, 100, 1000};

    for (size_t batch_size : batch_sizes) {
        // Create batch of vectors
        size_t total_elements = batch_size * vector_size;
        auto batch_vectors = TestHelpers::generate_random_vector(total_elements);

        auto time_us = TestHelpers::benchmark_function([&]() {
            MatrixOps::normalize(batch_vectors.data(), batch_size, vector_size);
        }, 100);

        double vectors_per_second = (batch_size * 1000000.0) / time_us;
        double time_per_vector = time_us / batch_size;

        std::cout << "    Batch " << std::setw(4) << batch_size
                  << ": " << std::setw(8) << std::fixed << std::setprecision(2) << time_per_vector << " μs/vector"
                  << ", " << std::setw(8) << std::setprecision(1) << vectors_per_second << " vectors/s" << std::endl;
    }
}

// ===============================================
// Cosine Similarity Performance Tests
// ===============================================

TEST_F(MatrixPerformanceTest, CosineSimilarity_ScalabilityTest) {
    std::cout << "\n  📊 Cosine Similarity Scalability:" << std::endl;
    std::cout << "    Size      |  Time (μs)  | Comparisons/s" << std::endl;
    std::cout << "    ----------|-------------|---------------" << std::endl;

    for (size_t size : test_sizes_) {
        auto& a = vec_a_[size];
        auto& b = vec_b_[size];
        float similarity = 0.0f;

        auto time_us = TestHelpers::benchmark_function([&]() {
            MatrixOps::cosine_similarity(a.data(), b.data(), &similarity, size);
        }, 1000);

        double comparisons_per_second = 1000000.0 / time_us;

        std::cout << "    " << std::setw(8) << size
                  << " | " << std::setw(10) << std::fixed << std::setprecision(2) << time_us
                  << " | " << std::setw(13) << std::setprecision(1) << comparisons_per_second
                  << std::endl;
    }
}

// ===============================================
// Memory Access Pattern Tests
// ===============================================

TEST_F(MatrixPerformanceTest, MemoryAccess_CacheEfficiency) {
    std::cout << "\n  📊 Cache Efficiency Analysis:" << std::endl;

    // Test with different stride patterns to analyze cache performance
    const size_t base_size = 1024;
    std::vector<size_t> strides = {1, 2, 4, 8, 16, 32, 64};

    for (size_t stride : strides) {
        size_t actual_size = base_size / stride;
        if (actual_size < 16) continue;

        auto vec_a = TestHelpers::generate_random_vector(base_size);
        auto vec_b = TestHelpers::generate_random_vector(base_size);
        std::vector<float> result(actual_size);

        auto time_us = TestHelpers::benchmark_function([&]() {
            // Access every stride-th element
            for (size_t i = 0; i < actual_size; ++i) {
                result[i] = vec_a[i * stride] + vec_b[i * stride];
            }
        }, 1000);

        double time_per_element = time_us / actual_size;

        std::cout << "    Stride " << std::setw(2) << stride
                  << ": " << std::setw(6) << std::fixed << std::setprecision(3) << time_per_element
                  << " μs/element (" << actual_size << " elements)" << std::endl;
    }
}

// ===============================================
// Stress Tests
// ===============================================

TEST_F(MatrixPerformanceTest, StressTest_LargeOperations) {
    std::cout << "\n  📊 Large Scale Operations Stress Test:" << std::endl;

    // Test with very large vectors
    const std::vector<size_t> large_sizes = {65536, 262144, 1048576}; // 64K, 256K, 1M elements

    for (size_t size : large_sizes) {
        std::cout << "    Testing size: " << TestHelpers::format_bytes(size * sizeof(float)) << std::endl;

        auto vec_a = TestHelpers::generate_random_vector(size, 12345);
        auto vec_b = TestHelpers::generate_random_vector(size, 54321);
        std::vector<float> result(size);

        // Vector addition
        auto add_time = TestHelpers::benchmark_function([&]() {
            MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, size);
        }, 10);

        // Dot product
        float dot_result = 0.0f;
        auto dot_time = TestHelpers::benchmark_function([&]() {
            MatrixOps::dot_product(vec_a.data(), vec_b.data(), &dot_result, size);
        }, 10);

        double add_throughput = (3 * size * sizeof(float) * 1000000.0) / (add_time * 1024.0 * 1024.0);
        double dot_throughput = (2 * size * sizeof(float) * 1000000.0) / (dot_time * 1024.0 * 1024.0);

        std::cout << "      Add: " << TestHelpers::format_duration(add_time)
                  << " (" << std::fixed << std::setprecision(2) << add_throughput << " MB/s)" << std::endl;
        std::cout << "      Dot: " << TestHelpers::format_duration(dot_time)
                  << " (" << std::fixed << std::setprecision(2) << dot_throughput << " MB/s)" << std::endl;

        // Should complete in reasonable time even for large vectors
        EXPECT_LT(add_time, 100000.0); // 100ms
        EXPECT_LT(dot_time, 100000.0);  // 100ms
    }
}

// ===============================================
// Overall Performance Summary
// ===============================================

TEST_F(MatrixPerformanceTest, PerformanceSummary_AllOperations) {
    std::cout << "\n  📊 Performance Summary (1K elements):" << std::endl;
    std::cout << "    Operation          |  Time (μs)  | Rate" << std::endl;
    std::cout << "    -------------------|-------------|----------------" << std::endl;

    const size_t test_size = 1024;
    auto& a = vec_a_[test_size];
    auto& b = vec_b_[test_size];
    std::vector<float> result(test_size);
    float scalar_result = 0.0f;

    // Vector Addition
    auto add_time = TestHelpers::benchmark_function([&]() {
        MatrixOps::add(a.data(), b.data(), result.data(), 1, test_size);
    }, 1000);

    // Dot Product
    auto dot_time = TestHelpers::benchmark_function([&]() {
        MatrixOps::dot_product(a.data(), b.data(), &scalar_result, test_size);
    }, 1000);

    // Matrix Transpose (32x32)
    const size_t matrix_size = 32;
    auto transpose_time = TestHelpers::benchmark_function([&]() {
        MatrixOps::transpose(matrix_a_[matrix_size].data(), matrix_results_[matrix_size].data(),
                           matrix_size, matrix_size);
    }, 1000);

    // Vector Normalization
    auto normalize_vec = a; // Copy
    auto normalize_time = TestHelpers::benchmark_function([&]() {
        MatrixOps::normalize(normalize_vec.data(), 1, test_size);
    }, 1000);

    // Cosine Similarity
    auto cosine_time = TestHelpers::benchmark_function([&]() {
        MatrixOps::cosine_similarity(a.data(), b.data(), &scalar_result, test_size);
    }, 1000);

    std::cout << "    Vector Addition    | " << std::setw(10) << std::fixed << std::setprecision(2) << add_time
              << " | " << std::setw(8) << std::setprecision(1) << (test_size * 1000000.0 / add_time) << " elem/s" << std::endl;

    std::cout << "    Dot Product        | " << std::setw(10) << dot_time
              << " | " << std::setw(8) << std::setprecision(1) << (test_size * 1000000.0 / dot_time) << " elem/s" << std::endl;

    std::cout << "    Matrix Transpose   | " << std::setw(10) << transpose_time
              << " | " << std::setw(8) << std::setprecision(1) << (matrix_size * matrix_size * 1000000.0 / transpose_time) << " elem/s" << std::endl;

    std::cout << "    Vector Normalize   | " << std::setw(10) << normalize_time
              << " | " << std::setw(8) << std::setprecision(1) << (1000000.0 / normalize_time) << " vec/s" << std::endl;

    std::cout << "    Cosine Similarity  | " << std::setw(10) << cosine_time
              << " | " << std::setw(8) << std::setprecision(1) << (1000000.0 / cosine_time) << " comp/s" << std::endl;
}
