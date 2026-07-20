/**
 * UMICP Compression - Simple Working Tests
 * Tests adapted to correct CompressionManager API
 */

#include "compression.h"
#include "umicp_types.h"
#include <gtest/gtest.h>

using namespace umicp;

// ============================================================================
// Basic Compression Tests
// ============================================================================

class CompressionSimpleTest : public ::testing::Test {
protected:
    void SetUp() override {
        zlib_comp = std::make_unique<CompressionManager>(CompressionAlgorithm::ZLIB);
        gzip_comp = std::make_unique<CompressionManager>(CompressionAlgorithm::GZIP);
    }

    std::unique_ptr<CompressionManager> zlib_comp;
    std::unique_ptr<CompressionManager> gzip_comp;
};

TEST_F(CompressionSimpleTest, ZlibCompressBasic) {
    ByteBuffer data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto result = zlib_comp->compress(data);

    EXPECT_TRUE(result.is_ok());
    if (result.value.has_value()) {
        EXPECT_FALSE(result.value->empty());
    }
}

TEST_F(CompressionSimpleTest, ZlibCompressDecompress) {
    ByteBuffer original = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto compressed = zlib_comp->compress(original);
    ASSERT_TRUE(compressed.is_ok());
    ASSERT_TRUE(compressed.value.has_value());

    auto decompressed = zlib_comp->decompress(*compressed.value);

    ASSERT_TRUE(decompressed.is_ok());
    ASSERT_TRUE(decompressed.value.has_value());
    EXPECT_EQ(*decompressed.value, original);
}

TEST_F(CompressionSimpleTest, GzipCompressBasic) {
    ByteBuffer data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto result = gzip_comp->compress(data);

    EXPECT_TRUE(result.is_ok());
    if (result.value.has_value()) {
        EXPECT_FALSE(result.value->empty());
    }
}

TEST_F(CompressionSimpleTest, GzipCompressDecompress) {
    ByteBuffer original(1000, 42); // 1KB of same byte

    auto compressed = gzip_comp->compress(original);
    ASSERT_TRUE(compressed.is_ok());
    ASSERT_TRUE(compressed.value.has_value());

    // Should compress well
    EXPECT_LT(compressed.value->size(), original.size());

    auto decompressed = gzip_comp->decompress(*compressed.value);

    ASSERT_TRUE(decompressed.is_ok());
    ASSERT_TRUE(decompressed.value.has_value());
    EXPECT_EQ(*decompressed.value, original);
}

TEST_F(CompressionSimpleTest, CompressLargeData) {
    ByteBuffer original(10000, 'A'); // 10KB of same character

    auto compressed = zlib_comp->compress(original);
    ASSERT_TRUE(compressed.is_ok());
    ASSERT_TRUE(compressed.value.has_value());

    // Should compress significantly
    EXPECT_LT(compressed.value->size(), original.size() / 10);

    auto decompressed = zlib_comp->decompress(*compressed.value);

    ASSERT_TRUE(decompressed.is_ok());
    ASSERT_TRUE(decompressed.value.has_value());
    EXPECT_EQ(*decompressed.value, original);
}

TEST_F(CompressionSimpleTest, CompressRandomData) {
    ByteBuffer original;
    for (size_t i = 0; i < 1000; ++i) {
        original.push_back(static_cast<uint8_t>(i % 256));
    }

    auto compressed = zlib_comp->compress(original);
    ASSERT_TRUE(compressed.is_ok());
    ASSERT_TRUE(compressed.value.has_value());

    auto decompressed = zlib_comp->decompress(*compressed.value);

    ASSERT_TRUE(decompressed.is_ok());
    ASSERT_TRUE(decompressed.value.has_value());
    EXPECT_EQ(*decompressed.value, original);
}

TEST_F(CompressionSimpleTest, ChangeAlgorithm) {
    ByteBuffer data(1000, 'X');

    // Start with ZLIB
    EXPECT_EQ(zlib_comp->get_algorithm(), CompressionAlgorithm::ZLIB);

    auto result1 = zlib_comp->compress(data);
    ASSERT_TRUE(result1.is_ok());

    // Change to GZIP
    auto set_result = zlib_comp->set_algorithm(CompressionAlgorithm::GZIP);
    EXPECT_TRUE(set_result.is_ok());
    EXPECT_EQ(zlib_comp->get_algorithm(), CompressionAlgorithm::GZIP);

    auto result2 = zlib_comp->compress(data);
    ASSERT_TRUE(result2.is_ok());

    // Results might be different
    // (GZIP has different header than ZLIB)
}

TEST_F(CompressionSimpleTest, CompressEmptyData) {
    ByteBuffer empty;

    auto result = zlib_comp->compress(empty);

    // Should handle empty data gracefully
    EXPECT_TRUE(result.is_ok() || result.code == ErrorCode::INVALID_ARGUMENT);
}

TEST_F(CompressionSimpleTest, DecompressInvalidData) {
    ByteBuffer invalid = {0xFF, 0xFE, 0xFD, 0xFC};

    auto result = zlib_comp->decompress(invalid);

    // Should fail gracefully
    EXPECT_FALSE(result.is_ok());
}

TEST_F(CompressionSimpleTest, CompressionLevel) {
    ByteBuffer data(5000, 'T');

    // Default compression
    auto default_comp = zlib_comp->compress(data);
    ASSERT_TRUE(default_comp.is_ok());

    // Best compression (9)
    auto best_comp = zlib_comp->compress(data, 9);
    ASSERT_TRUE(best_comp.is_ok());

    // Fast compression (1)
    auto fast_comp = zlib_comp->compress(data, 1);
    ASSERT_TRUE(fast_comp.is_ok());

    // All should decompress correctly
    auto decomp1 = zlib_comp->decompress(*default_comp.value);
    auto decomp2 = zlib_comp->decompress(*best_comp.value);
    auto decomp3 = zlib_comp->decompress(*fast_comp.value);

    ASSERT_TRUE(decomp1.is_ok());
    ASSERT_TRUE(decomp2.is_ok());
    ASSERT_TRUE(decomp3.is_ok());

    EXPECT_EQ(*decomp1.value, data);
    EXPECT_EQ(*decomp2.value, data);
    EXPECT_EQ(*decomp3.value, data);
}

// ============================================================================
// Utility Function Tests
// ============================================================================

TEST(CompressionUtilityTest, EstimateCompressedSize) {
    ByteBuffer data(1000, 'A');

    auto estimate = CompressionManager::estimate_compressed_size(data, CompressionAlgorithm::ZLIB);

    if (estimate.is_ok() && estimate.value.has_value()) {
        EXPECT_GT(*estimate.value, 0);
        // Estimate should be reasonable
        EXPECT_LT(*estimate.value, data.size() * 2);
    }
}

TEST(CompressionUtilityTest, ShouldCompressSmallData) {
    ByteBuffer small = {1, 2, 3, 4, 5};

    bool should_compress = CompressionManager::should_compress(
        small, 100, CompressionAlgorithm::ZLIB);

    // Small data below threshold shouldn't be compressed
    EXPECT_FALSE(should_compress);
}

TEST(CompressionUtilityTest, ShouldCompressLargeData) {
    ByteBuffer large(10000, 'X');

    bool should_compress = CompressionManager::should_compress(
        large, 1000, CompressionAlgorithm::ZLIB);

    // Large data above threshold should be compressed
    EXPECT_TRUE(should_compress);
}

// ============================================================================
// Main Test Entry Point
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

