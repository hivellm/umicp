/**
 * UMICP Test Suite - Main Entry Point
 * Comprehensive testing for all UMICP components
 */

#include <gtest/gtest.h>
#include <iostream>
#include <chrono>

// Test configuration
class UMICPTestEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        std::cout << "🚀 UMICP Test Suite Starting" << std::endl;
        std::cout << "==============================" << std::endl;
        start_time_ = std::chrono::high_resolution_clock::now();
    }

    void TearDown() override {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time_);

        std::cout << "\n==============================" << std::endl;
        std::cout << "🏁 Test Suite Complete" << std::endl;
        std::cout << "Total Execution Time: " << duration.count() << " ms" << std::endl;
        std::cout << "==============================" << std::endl;
    }

private:
    std::chrono::high_resolution_clock::time_point start_time_;
};

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Add global test environment
    ::testing::AddGlobalTestEnvironment(new UMICPTestEnvironment);

    // Custom test output format
    ::testing::GTEST_FLAG(color) = "yes";
    ::testing::GTEST_FLAG(print_time) = true;

    return RUN_ALL_TESTS();
}
