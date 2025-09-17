/**
 * UMICP Matrix Operations Edge Cases Tests
 * Comprehensive testing of edge cases and boundary conditions
 */

#include <gtest/gtest.h>
#include "matrix_ops.h"
#include "../utils/test_helpers.h"
#include <cmath>
#include <limits>
#include <random>

using namespace umicp;
using namespace umicp::testing;

class MatrixOpsEdgeCaseTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        // Initialize random number generator
        rng_.seed(42);

        // Edge case vectors
        zero_vector_ = std::vector<float>(8, 0.0f);
        one_vector_ = std::vector<float>(8, 1.0f);
        negative_vector_ = {-1.0f, -2.0f, -3.0f, -4.0f, -5.0f, -6.0f, -7.0f, -8.0f};

        // Extreme values
        max_float_ = std::numeric_limits<float>::max();
        min_float_ = std::numeric_limits<float>::min();
        epsilon_ = std::numeric_limits<float>::epsilon();

        // Very large vectors for stress testing
        very_large_vector_ = TestHelpers::generate_random_vector(10000, 12345);
        very_large_vector_b_ = TestHelpers::generate_random_vector(10000, 54321);
    }

    std::mt19937 rng_;
    std::vector<float> zero_vector_, one_vector_, negative_vector_;
    std::vector<float> very_large_vector_, very_large_vector_b_;
    float max_float_, min_float_, epsilon_;
};

// ===============================================
// Zero Vector Tests
// ===============================================

TEST_F(MatrixOpsEdgeCaseTest, ZeroVector_Addition) {
    std::vector<float> result(8);

    auto op_result = MatrixOps::add(zero_vector_.data(), zero_vector_.data(), result.data(), 1, 8);

    ASSERT_TRUE(op_result.is_success());
    for (size_t i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(result[i], 0.0f);
    }
}

TEST_F(MatrixOpsEdgeCaseTest, ZeroVector_DotProduct) {
    float result;

    auto op_result = MatrixOps::dot_product(zero_vector_.data(), one_vector_.data(), &result, 8);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST_F(MatrixOpsEdgeCaseTest, ZeroVector_Normalization) {
    std::vector<float> result = zero_vector_;

    auto op_result = MatrixOps::normalize(result.data(), 1, 8);

    ASSERT_TRUE(op_result.is_success());
    // Zero vector normalization should result in zero vector
    for (size_t i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(result[i], 0.0f);
    }
}

// ===============================================
// Extreme Value Tests
// ===============================================

TEST_F(MatrixOpsEdgeCaseTest, ExtremeValues_Addition) {
    std::vector<float> vec_a = {max_float_, min_float_, epsilon_, -epsilon_};
    std::vector<float> vec_b = {1.0f, 1.0f, 1.0f, 1.0f};
    std::vector<float> result(4);

    auto op_result = MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, 4);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_FLOAT_EQ(result[0], max_float_ + 1.0f);
    EXPECT_FLOAT_EQ(result[1], min_float_ + 1.0f);
}

TEST_F(MatrixOpsEdgeCaseTest, ExtremeValues_DotProduct) {
    std::vector<float> vec_a = {max_float_, min_float_};
    std::vector<float> vec_b = {1.0f, 1.0f};
    float result;

    auto op_result = MatrixOps::dot_product(vec_a.data(), vec_b.data(), &result, 2);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_FLOAT_EQ(result, max_float_ + min_float_);
}

// ===============================================
// Negative Value Tests
// ===============================================

TEST_F(MatrixOpsEdgeCaseTest, NegativeValues_Addition) {
    std::vector<float> result(8);

    auto op_result = MatrixOps::add(negative_vector_.data(), negative_vector_.data(), result.data(), 1, 8);

    ASSERT_TRUE(op_result.is_success());
    for (size_t i = 0; i < 8; ++i) {
        EXPECT_FLOAT_EQ(result[i], negative_vector_[i] * 2.0f);
    }
}

TEST_F(MatrixOpsEdgeCaseTest, NegativeValues_DotProduct) {
    std::vector<float> vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> vec_b = {-1.0f, -2.0f, -3.0f, -4.0f};
    float result;

    auto op_result = MatrixOps::dot_product(vec_a.data(), vec_b.data(), &result, 4);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_FLOAT_EQ(result, -30.0f); // 1*(-1) + 2*(-2) + 3*(-3) + 4*(-4)
}

TEST_F(MatrixOpsEdgeCaseTest, NegativeValues_Normalization) {
    std::vector<float> vec = {-3.0f, -4.0f};
    std::vector<float> result = vec;

    auto op_result = MatrixOps::normalize(result.data(), 1, 2);

    ASSERT_TRUE(op_result.is_success());
    float magnitude = std::sqrt(result[0] * result[0] + result[1] * result[1]);
    EXPECT_NEAR(magnitude, 1.0f, epsilon_);
}

// ===============================================
// Very Large Vector Tests (Stress Testing)
// ===============================================

TEST_F(MatrixOpsEdgeCaseTest, VeryLargeVectors_Addition) {
    std::vector<float> result(10000);

    auto op_result = MatrixOps::add(very_large_vector_.data(), very_large_vector_b_.data(),
                                   result.data(), 1, 10000);

    ASSERT_TRUE(op_result.is_success());

    // Verify first few elements
    for (size_t i = 0; i < 10; ++i) {
        EXPECT_FLOAT_EQ(result[i], very_large_vector_[i] + very_large_vector_b_[i]);
    }
}

TEST_F(MatrixOpsEdgeCaseTest, VeryLargeVectors_DotProduct) {
    float result;

    auto op_result = MatrixOps::dot_product(very_large_vector_.data(), very_large_vector_b_.data(),
                                           &result, 10000);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_TRUE(std::isfinite(result));
}

TEST_F(MatrixOpsEdgeCaseTest, VeryLargeVectors_Normalization) {
    std::vector<float> result = very_large_vector_;

    auto op_result = MatrixOps::normalize(result.data(), 1, 10000);

    ASSERT_TRUE(op_result.is_success());

    // Check that the result is normalized
    float magnitude = 0.0f;
    for (size_t i = 0; i < 10000; ++i) {
        magnitude += result[i] * result[i];
    }
    magnitude = std::sqrt(magnitude);
    EXPECT_NEAR(magnitude, 1.0f, epsilon_);
}

// ===============================================
// Precision and Accuracy Tests
// ===============================================

TEST_F(MatrixOpsEdgeCaseTest, Precision_Addition) {
    std::vector<float> vec_a = {1.0f + epsilon_, 2.0f + epsilon_};
    std::vector<float> vec_b = {1.0f - epsilon_, 2.0f - epsilon_};
    std::vector<float> result(2);

    auto op_result = MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, 2);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_FLOAT_EQ(result[0], 2.0f);
    EXPECT_FLOAT_EQ(result[1], 4.0f);
}

TEST_F(MatrixOpsEdgeCaseTest, Precision_DotProduct) {
    std::vector<float> vec_a = {1.0f, 1.0f};
    std::vector<float> vec_b = {1.0f + epsilon_, 1.0f - epsilon_};
    float result;

    auto op_result = MatrixOps::dot_product(vec_a.data(), vec_b.data(), &result, 2);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_NEAR(result, 2.0f, epsilon_ * 10);
}

// ===============================================
// Matrix Edge Cases
// ===============================================

TEST_F(MatrixOpsEdgeCaseTest, MatrixMultiplication_ZeroMatrix) {
    std::vector<float> matrix_a = {0.0f, 0.0f, 0.0f, 0.0f}; // 2x2 zero matrix
    std::vector<float> matrix_b = {1.0f, 2.0f, 3.0f, 4.0f}; // 2x2 matrix
    std::vector<float> result(4);

    auto op_result = MatrixOps::multiply(matrix_a.data(), matrix_b.data(), result.data(), 2, 2, 2);

    ASSERT_TRUE(op_result.is_success());
    for (size_t i = 0; i < 4; ++i) {
        EXPECT_FLOAT_EQ(result[i], 0.0f);
    }
}

TEST_F(MatrixOpsEdgeCaseTest, MatrixMultiplication_IdentityMatrix) {
    std::vector<float> matrix_a = {1.0f, 0.0f, 0.0f, 1.0f}; // 2x2 identity
    std::vector<float> matrix_b = {5.0f, 6.0f, 7.0f, 8.0f}; // 2x2 matrix
    std::vector<float> result(4);

    auto op_result = MatrixOps::multiply(matrix_a.data(), matrix_b.data(), result.data(), 2, 2, 2);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_FLOAT_EQ(result[0], 5.0f);
    EXPECT_FLOAT_EQ(result[1], 6.0f);
    EXPECT_FLOAT_EQ(result[2], 7.0f);
    EXPECT_FLOAT_EQ(result[3], 8.0f);
}

TEST_F(MatrixOpsEdgeCaseTest, MatrixTranspose_SingleElement) {
    std::vector<float> matrix = {42.0f}; // 1x1 matrix
    std::vector<float> result(1);

    auto op_result = MatrixOps::transpose(matrix.data(), result.data(), 1, 1);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_FLOAT_EQ(result[0], 42.0f);
}

// ===============================================
// Cosine Similarity Edge Cases
// ===============================================

TEST_F(MatrixOpsEdgeCaseTest, CosineSimilarity_IdenticalVectors) {
    std::vector<float> vec = {1.0f, 2.0f, 3.0f, 4.0f};
    float result;

    auto op_result = MatrixOps::cosine_similarity(vec.data(), vec.data(), &result, 4);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_NEAR(result, 1.0f, epsilon_);
}

TEST_F(MatrixOpsEdgeCaseTest, CosineSimilarity_OppositeVectors) {
    std::vector<float> vec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> vec_b = {-1.0f, -2.0f, -3.0f, -4.0f};
    float result;

    auto op_result = MatrixOps::cosine_similarity(vec_a.data(), vec_b.data(), &result, 4);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_NEAR(result, -1.0f, epsilon_);
}

TEST_F(MatrixOpsEdgeCaseTest, CosineSimilarity_OrthogonalVectors) {
    std::vector<float> vec_a = {1.0f, 0.0f};
    std::vector<float> vec_b = {0.0f, 1.0f};
    float result;

    auto op_result = MatrixOps::cosine_similarity(vec_a.data(), vec_b.data(), &result, 2);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_NEAR(result, 0.0f, epsilon_);
}

// ===============================================
// Random Data Tests
// ===============================================

TEST_F(MatrixOpsEdgeCaseTest, RandomData_Consistency) {
    const size_t num_tests = 100;
    const size_t vector_size = 16;

    for (size_t test = 0; test < num_tests; ++test) {
        std::vector<float> vec_a = TestHelpers::generate_random_vector(vector_size, rng_());
        std::vector<float> vec_b = TestHelpers::generate_random_vector(vector_size, rng_());
        std::vector<float> result_add(vector_size);
        std::vector<float> result_dot(1);
        float dot_result;

        // Test addition
        auto add_result = MatrixOps::add(vec_a.data(), vec_b.data(), result_add.data(), 1, vector_size);
        ASSERT_TRUE(add_result.is_success());

        // Test dot product
        auto dot_op_result = MatrixOps::dot_product(vec_a.data(), vec_b.data(), &dot_result, vector_size);
        ASSERT_TRUE(dot_op_result.is_success());

        // Verify results are finite
        for (size_t i = 0; i < vector_size; ++i) {
            EXPECT_TRUE(std::isfinite(result_add[i]));
        }
        EXPECT_TRUE(std::isfinite(dot_result));
    }
}

// ===============================================
// Memory Alignment Tests
// ===============================================

TEST_F(MatrixOpsEdgeCaseTest, MemoryAlignment_AlignedVectors) {
    // Test with properly aligned vectors
    const size_t size = 16;
    std::vector<float> vec_a(size, 1.0f);
    std::vector<float> vec_b(size, 2.0f);
    std::vector<float> result(size);

    auto op_result = MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, size);

    ASSERT_TRUE(op_result.is_success());
    for (size_t i = 0; i < size; ++i) {
        EXPECT_FLOAT_EQ(result[i], 3.0f);
    }
}

// ===============================================
// Performance Under Load Tests
// ===============================================

TEST_F(MatrixOpsEdgeCaseTest, PerformanceUnderLoad_MultipleOperations) {
    const size_t num_operations = 1000;
    const size_t vector_size = 64;

    std::vector<float> vec_a = TestHelpers::generate_random_vector(vector_size, 12345);
    std::vector<float> vec_b = TestHelpers::generate_random_vector(vector_size, 54321);
    std::vector<float> result(vector_size);

    for (size_t i = 0; i < num_operations; ++i) {
        auto op_result = MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, vector_size);
        ASSERT_TRUE(op_result.is_success());

        // Rotate vectors slightly to create variation
        std::rotate(vec_a.begin(), vec_a.begin() + 1, vec_a.end());
    }
}
