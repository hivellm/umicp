/**
 * Simple Matrix Operations Test
 */

#include <gtest/gtest.h>
#include <iostream>
#include <vector>

// Include UMICP headers
#include "umicp_types.h"
#include "matrix_ops.h"

using namespace umicp;

TEST(MatrixOpsSimple, VectorAddition) {
    std::vector<float> a = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> b = {5.0f, 6.0f, 7.0f, 8.0f};
    std::vector<float> result(4);

    auto op_result = MatrixOps::add(a.data(), b.data(), result.data(), 1, 4);

    ASSERT_TRUE(op_result.is_success());
    EXPECT_FLOAT_EQ(result[0], 6.0f);
    EXPECT_FLOAT_EQ(result[1], 8.0f);
    EXPECT_FLOAT_EQ(result[2], 10.0f);
    EXPECT_FLOAT_EQ(result[3], 12.0f);

    std::cout << "✅ Vector addition test passed!" << std::endl;
}

TEST(MatrixOpsSimple, DotProduct) {
    std::vector<float> a = {1.0f, 2.0f, 3.0f, 4.0f};
    std::vector<float> b = {5.0f, 6.0f, 7.0f, 8.0f};
    float result = 0.0f;

    auto op_result = MatrixOps::dot_product(a.data(), b.data(), &result, 4);

    ASSERT_TRUE(op_result.is_success());
    // Expected: 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70
    EXPECT_FLOAT_EQ(result, 70.0f);

    std::cout << "✅ Dot product test passed!" << std::endl;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "🧮 Starting Matrix Operations Test" << std::endl;
    int result = RUN_ALL_TESTS();
    std::cout << "🏁 Matrix Tests Complete" << std::endl;
    return result;
}
