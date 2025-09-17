/**
 * Simple test to verify basic compilation and GoogleTest functionality
 */

#include <gtest/gtest.h>
#include <iostream>

// Simple test to verify GoogleTest is working
TEST(BasicTest, SanityCheck) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);

    std::cout << "✅ Basic sanity check passed!" << std::endl;
}

// Test basic C++ features
TEST(BasicTest, CppFeatures) {
    // Test vectors
    std::vector<int> vec = {1, 2, 3, 4, 5};
    EXPECT_EQ(vec.size(), 5);

    // Test strings
    std::string hello = "Hello, UMICP!";
    EXPECT_EQ(hello.length(), 13);

    // Test lambdas
    auto add = [](int a, int b) { return a + b; };
    EXPECT_EQ(add(10, 20), 30);

    std::cout << "✅ C++ features working!" << std::endl;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    std::cout << "🚀 Starting UMICP Simple Test Suite" << std::endl;
    int result = RUN_ALL_TESTS();
    std::cout << "🏁 Test Suite Complete" << std::endl;
    return result;
}
