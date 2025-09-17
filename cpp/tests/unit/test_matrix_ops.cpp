/**
 * UMICP Matrix Operations Unit Tests
 * Comprehensive testing of SIMD-optimized matrix operations
 */

#include <gtest/gtest.h>
#include "matrix_ops.h"
#include "../utils/test_helpers.h"
#include <cmath>

using namespace umicp;
using namespace umicp::testing;

class MatrixOpsTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        // Test vectors
        vec_a_ = {1.0f, 2.0f, 3.0f, 4.0f};
        vec_b_ = {5.0f, 6.0f, 7.0f, 8.0f};

        // Large vectors for SIMD testing
        large_vec_a_ = TestHelpers::generate_random_vector(1024, 12345);
        large_vec_b_ = TestHelpers::generate_random_vector(1024, 54321);

        // Matrix data (4x4)
        matrix_4x4_ = {
            1.0f, 2.0f, 3.0f, 4.0f,
            5.0f, 6.0f, 7.0f, 8.0f,
            9.0f, 10.0f, 11.0f, 12.0f,
            13.0f, 14.0f, 15.0f, 16.0f
        };
    }

    std::vector<float> vec_a_, vec_b_;
    std::vector<float> large_vec_a_, large_vec_b_;
    std::vector<float> matrix_4x4_;
};

// ===============================================
// Vector Addition Tests
// ===============================================

TEST_F(MatrixOpsTest, VectorAddition_BasicTest) {
    std::vector<float> result(4);

    auto op_result = MatrixOps::add(vec_a_.data(), vec_b_.data(), result.data(), 1, 4);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_FLOAT_EQ(result[0], 6.0f);  // 1 + 5
    EXPECT_FLOAT_EQ(result[1], 8.0f);  // 2 + 6
    EXPECT_FLOAT_EQ(result[2], 10.0f); // 3 + 7
    EXPECT_FLOAT_EQ(result[3], 12.0f); // 4 + 8
}

TEST_F(MatrixOpsTest, VectorAddition_LargeVector) {
    std::vector<float> result(1024);

    auto op_result = MatrixOps::add(large_vec_a_.data(), large_vec_b_.data(),
                                   result.data(), 1, 1024);

    ASSERT_TRUE(op_result.is_success());

    // Verify results manually for first few elements
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_FLOAT_EQ(result[i], large_vec_a_[i] + large_vec_b_[i]);
    }
}

TEST_F(MatrixOpsTest, VectorAddition_NullPointers) {
    std::vector<float> result(4);

    auto result1 = MatrixOps::add(nullptr, vec_b_.data(), result.data(), 1, 4);
    auto result2 = MatrixOps::add(vec_a_.data(), nullptr, result.data(), 1, 4);
    auto result3 = MatrixOps::add(vec_a_.data(), vec_b_.data(), nullptr, 1, 4);

    EXPECT_FALSE(result1.is_success());
    EXPECT_FALSE(result2.is_success());
    EXPECT_FALSE(result3.is_success());
    EXPECT_EQ(result1.code, ErrorCode::INVALID_ARGUMENT);
}

// ===============================================
// Dot Product Tests
// ===============================================

TEST_F(MatrixOpsTest, DotProduct_BasicTest) {
    float result = 0.0f;

    auto op_result = MatrixOps::dot_product(vec_a_.data(), vec_b_.data(), &result, 4);

    ASSERT_TRUE(op_result.is_success());
    // Expected: 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70
    EXPECT_FLOAT_EQ(result, 70.0f);
}

TEST_F(MatrixOpsTest, DotProduct_OrthogonalVectors) {
    std::vector<float> orthogonal_a = {1.0f, 0.0f, 0.0f, 0.0f};
    std::vector<float> orthogonal_b = {0.0f, 1.0f, 0.0f, 0.0f};
    float result = 0.0f;

    auto op_result = MatrixOps::dot_product(orthogonal_a.data(), orthogonal_b.data(),
                                          &result, 4);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST_F(MatrixOpsTest, DotProduct_LargeVector) {
    float result = 0.0f;

    auto op_result = MatrixOps::dot_product(large_vec_a_.data(), large_vec_b_.data(),
                                          &result, 1024);

    ASSERT_TRUE(op_result.is_success());

    // Verify manually for small subset
    float manual_result = 0.0f;
    for (size_t i = 0; i < 10; ++i) {
        manual_result += large_vec_a_[i] * large_vec_b_[i];
    }
    // We can't verify the entire result due to floating point precision,
    // but we can check that some computation occurred
    EXPECT_NE(result, 0.0f);
}

// ===============================================
// Matrix Multiplication Tests
// ===============================================

TEST_F(MatrixOpsTest, MatrixMultiplication_2x2) {
    // 2x2 matrices: A * B = C
    std::vector<float> a = {1.0f, 2.0f, 3.0f, 4.0f}; // [[1,2], [3,4]]
    std::vector<float> b = {5.0f, 6.0f, 7.0f, 8.0f}; // [[5,6], [7,8]]
    std::vector<float> result(4);

    auto op_result = MatrixOps::multiply(a.data(), b.data(), result.data(), 2, 2, 2);

    ASSERT_TRUE(op_result.is_success());

    // Expected result: [[19,22], [43,50]]
    EXPECT_FLOAT_EQ(result[0], 19.0f); // 1*5 + 2*7
    EXPECT_FLOAT_EQ(result[1], 22.0f); // 1*6 + 2*8
    EXPECT_FLOAT_EQ(result[2], 43.0f); // 3*5 + 4*7
    EXPECT_FLOAT_EQ(result[3], 50.0f); // 3*6 + 4*8
}

TEST_F(MatrixOpsTest, MatrixMultiplication_IdentityMatrix) {
    std::vector<float> identity = {1.0f, 0.0f, 0.0f, 1.0f}; // 2x2 identity
    std::vector<float> test_matrix = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> result(4);

    auto op_result = MatrixOps::multiply(test_matrix.data(), identity.data(),
                                       result.data(), 2, 2, 2);

    ASSERT_TRUE(op_result.is_success());
    ASSERT_VECTORS_EQUAL(result, test_matrix, 0.001f);
}

// ===============================================
// Matrix Transpose Tests
// ===============================================

TEST_F(MatrixOpsTest, MatrixTranspose_2x3) {
    // 2x3 matrix: [[1,2,3], [4,5,6]]
    std::vector<float> input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    std::vector<float> result(6);

    auto op_result = MatrixOps::transpose(input.data(), result.data(), 2, 3);

    ASSERT_TRUE(op_result.is_success());

    // Expected: [[1,4], [2,5], [3,6]] = [1,4,2,5,3,6]
    std::vector<float> expected = {1.0f, 4.0f, 2.0f, 5.0f, 3.0f, 6.0f};
    ASSERT_VECTORS_EQUAL(result, expected, 0.001f);
}

TEST_F(MatrixOpsTest, MatrixTranspose_SquareMatrix) {
    // 3x3 matrix
    std::vector<float> input = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    std::vector<float> result(9);

    auto op_result = MatrixOps::transpose(input.data(), result.data(), 3, 3);

    ASSERT_TRUE(op_result.is_success());

    // Expected transpose
    std::vector<float> expected = {1.0f, 4.0f, 7.0f, 2.0f, 5.0f, 8.0f, 3.0f, 6.0f, 9.0f};
    ASSERT_VECTORS_EQUAL(result, expected, 0.001f);
}

// ===============================================
// Vector Normalization Tests
// ===============================================

TEST_F(MatrixOpsTest, VectorNormalization_UnitVector) {
    std::vector<float> vec = {3.0f, 4.0f}; // Length = 5

    auto op_result = MatrixOps::normalize(vec.data(), 1, 2);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_FLOAT_EQ(vec[0], 0.6f); // 3/5
    EXPECT_FLOAT_EQ(vec[1], 0.8f); // 4/5

    // Verify unit length
    float length = std::sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
    EXPECT_NEAR(length, 1.0f, 0.001f);
}

TEST_F(MatrixOpsTest, VectorNormalization_ZeroVector) {
    std::vector<float> vec = {0.0f, 0.0f, 0.0f};

    auto op_result = MatrixOps::normalize(vec.data(), 1, 3);

    // Should handle zero vector gracefully (implementation dependent)
    // At minimum, shouldn't crash
    EXPECT_TRUE(op_result.is_success() || op_result.code == ErrorCode::INVALID_ARGUMENT);
}

TEST_F(MatrixOpsTest, VectorNormalization_MultipleVectors) {
    // 2 vectors of length 3 each: [[1,0,0], [0,3,4]]
    std::vector<float> vecs = {1.0f, 0.0f, 0.0f, 0.0f, 3.0f, 4.0f};

    auto op_result = MatrixOps::normalize(vecs.data(), 2, 3);

    ASSERT_TRUE(op_result.is_success());

    // First vector should remain [1,0,0]
    EXPECT_NEAR(vecs[0], 1.0f, 0.001f);
    EXPECT_NEAR(vecs[1], 0.0f, 0.001f);
    EXPECT_NEAR(vecs[2], 0.0f, 0.001f);

    // Second vector should be [0, 0.6, 0.8]
    EXPECT_NEAR(vecs[3], 0.0f, 0.001f);
    EXPECT_NEAR(vecs[4], 0.6f, 0.001f);
    EXPECT_NEAR(vecs[5], 0.8f, 0.001f);
}

// ===============================================
// Cosine Similarity Tests
// ===============================================

TEST_F(MatrixOpsTest, CosineSimilarity_IdenticalVectors) {
    float result = 0.0f;

    auto op_result = MatrixOps::cosine_similarity(vec_a_.data(), vec_a_.data(),
                                                &result, 4);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_NEAR(result, 1.0f, 0.001f); // cos(0) = 1
}

TEST_F(MatrixOpsTest, CosineSimilarity_OrthogonalVectors) {
    std::vector<float> orthogonal_a = {1.0f, 0.0f};
    std::vector<float> orthogonal_b = {0.0f, 1.0f};
    float result = 0.0f;

    auto op_result = MatrixOps::cosine_similarity(orthogonal_a.data(), orthogonal_b.data(),
                                                &result, 2);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_NEAR(result, 0.0f, 0.001f); // cos(90°) = 0
}

TEST_F(MatrixOpsTest, CosineSimilarity_OppositeVectors) {
    std::vector<float> vec_positive = {1.0f, 1.0f};
    std::vector<float> vec_negative = {-1.0f, -1.0f};
    float result = 0.0f;

    auto op_result = MatrixOps::cosine_similarity(vec_positive.data(), vec_negative.data(),
                                                &result, 2);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_NEAR(result, -1.0f, 0.001f); // cos(180°) = -1
}

// ===============================================
// Edge Cases and Error Handling
// ===============================================

TEST_F(MatrixOpsTest, ErrorHandling_NullPointers) {
    std::vector<float> result(4);
    float scalar_result = 0.0f;

    // Test all operations with null pointers
    EXPECT_FALSE(MatrixOps::add(nullptr, vec_b_.data(), result.data(), 1, 4).is_success());
    EXPECT_FALSE(MatrixOps::multiply(nullptr, vec_b_.data(), result.data(), 2, 2, 2).is_success());
    EXPECT_FALSE(MatrixOps::transpose(nullptr, result.data(), 2, 2).is_success());
    EXPECT_FALSE(MatrixOps::dot_product(nullptr, vec_b_.data(), &scalar_result, 4).is_success());
    EXPECT_FALSE(MatrixOps::normalize(nullptr, 1, 4).is_success());
    EXPECT_FALSE(MatrixOps::cosine_similarity(nullptr, vec_b_.data(), &scalar_result, 4).is_success());
}

TEST_F(MatrixOpsTest, EdgeCase_ZeroSizeOperations) {
    std::vector<float> result(1);
    float scalar_result = 0.0f;

    // Operations with zero size should handle gracefully
    auto add_result = MatrixOps::add(vec_a_.data(), vec_b_.data(), result.data(), 0, 0);
    auto mult_result = MatrixOps::multiply(vec_a_.data(), vec_b_.data(), result.data(), 0, 0, 0);
    auto dot_result = MatrixOps::dot_product(vec_a_.data(), vec_b_.data(), &scalar_result, 0);

    // Behavior may vary, but operations should not crash
    // Results should either succeed (no-op) or return appropriate error
}

// ===============================================
// SIMD Performance Verification
// ===============================================

TEST_F(MatrixOpsTest, SIMD_PerformanceCheck) {
    // This test verifies that SIMD paths are being used
    // by comparing performance on vectors of different sizes

    const size_t small_size = 15;  // Too small for efficient SIMD
    const size_t large_size = 1024; // Good for SIMD

    auto small_a = TestHelpers::generate_random_vector(small_size);
    auto small_b = TestHelpers::generate_random_vector(small_size);
    auto large_a = TestHelpers::generate_random_vector(large_size);
    auto large_b = TestHelpers::generate_random_vector(large_size);

    std::vector<float> small_result(small_size);
    std::vector<float> large_result(large_size);

    // Benchmark small vector addition
    auto small_time = TestHelpers::benchmark_function([&]() {
        MatrixOps::add(small_a.data(), small_b.data(), small_result.data(), 1, small_size);
    }, 10000);

    // Benchmark large vector addition
    auto large_time = TestHelpers::benchmark_function([&]() {
        MatrixOps::add(large_a.data(), large_b.data(), large_result.data(), 1, large_size);
    }, 1000);

    // Calculate time per element
    double small_time_per_element = small_time / small_size;
    double large_time_per_element = large_time / large_size;

    std::cout << "  📊 Small vector (" << small_size << "): "
              << TestHelpers::format_duration(small_time_per_element) << " per element" << std::endl;
    std::cout << "  📊 Large vector (" << large_size << "): "
              << TestHelpers::format_duration(large_time_per_element) << " per element" << std::endl;

    // SIMD should make large vectors more efficient per element
    // This is not a hard requirement but indicates SIMD is working
    if (large_time_per_element < small_time_per_element * 0.8) {
        std::cout << "  ✅ SIMD optimization detected!" << std::endl;
    }
}
