/**
 * UMICP Compression - Core Tests
 * Tests using actual CompressionManager API
 */

#include "compression.h"
#include "umicp_types.h"
#include <gtest/gtest.h>

using namespace umicp;

// ============================================================================
// Basic Compression Tests
// ============================================================================

TEST(CompressionTest, ZlibCompressionBasic) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    
    ByteBuffer data(1000, 0xAA);
    auto result = comp.compress(data);
    
    EXPECT_TRUE(result.is_success());
    EXPECT_TRUE(result.value.has_value());
    EXPECT_LT(result.value->size(), data.size());
}

TEST(CompressionTest, ZlibDecompressionBasic) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    
    ByteBuffer original(1000, 0xBB);
    
    auto compressed = comp.compress(original);
    ASSERT_TRUE(compressed.is_success());
    
    auto decompressed = comp.decompress(*compressed.value);
    ASSERT_TRUE(decompressed.is_success());
    EXPECT_EQ(*decompressed.value, original);
}

TEST(CompressionTest, GzipCompression) {
    CompressionManager comp(CompressionAlgorithm::GZIP);
    
    ByteBuffer data(1000, 0xCC);
    auto result = comp.compress(data);
    
    EXPECT_TRUE(result.is_success());
}

TEST(CompressionTest, GzipDecompression) {
    CompressionManager comp(CompressionAlgorithm::GZIP);
    
    ByteBuffer original(1000, 0xDD);
    
    auto compressed = comp.compress(original);
    ASSERT_TRUE(compressed.is_success());
    
    auto decompressed = comp.decompress(*compressed.value);
    ASSERT_TRUE(decompressed.is_success());
    EXPECT_EQ(*decompressed.value, original);
}

TEST(CompressionTest, NoCompression) {
    CompressionManager comp(CompressionAlgorithm::NONE);
    
    ByteBuffer data(100, 0xEE);
    auto result = comp.compress(data);
    
    EXPECT_TRUE(result.is_success());
    EXPECT_EQ(*result.value, data);
}

// ============================================================================
// Round-Trip Tests
// ============================================================================

TEST(CompressionTest, RoundTripZlib) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    
    ByteBuffer original;
    for (int i = 0; i < 1000; ++i) {
        original.push_back((i * 3 + 7) % 256);
    }
    
    auto compressed = comp.compress(original);
    ASSERT_TRUE(compressed.is_success());
    
    auto decompressed = comp.decompress(*compressed.value);
    ASSERT_TRUE(decompressed.is_success());
    EXPECT_EQ(*decompressed.value, original);
}

TEST(CompressionTest, RoundTripGzip) {
    CompressionManager comp(CompressionAlgorithm::GZIP);
    
    ByteBuffer original;
    for (int i = 0; i < 1000; ++i) {
        original.push_back((i * 7 + 13) % 256);
    }
    
    auto compressed = comp.compress(original);
    ASSERT_TRUE(compressed.is_success());
    
    auto decompressed = comp.decompress(*compressed.value);
    ASSERT_TRUE(decompressed.is_success());
    EXPECT_EQ(*decompressed.value, original);
}

// ============================================================================
// Compression Ratio Tests
// ============================================================================

TEST(CompressionTest, HighlyCompressibleData) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    
    ByteBuffer data(10000, 0x00); // All zeros
    
    auto result = comp.compress(data);
    ASSERT_TRUE(result.is_success());
    
    // Should achieve excellent compression
    EXPECT_LT(result.value->size(), data.size() / 10);
}

TEST(CompressionTest, RepetitivePatternCompression) {
    CompressionManager comp(CompressionAlgorithm::GZIP);
    
    ByteBuffer pattern;
    for (int i = 0; i < 100; ++i) {
        pattern.push_back(1);
        pattern.push_back(2);
        pattern.push_back(3);
    }
    
    auto result = comp.compress(pattern);
    ASSERT_TRUE(result.is_success());
    EXPECT_LT(result.value->size(), pattern.size());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(CompressionTest, EmptyData) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    
    ByteBuffer empty;
    auto result = comp.compress(empty);
    
    EXPECT_TRUE(result.is_success());
}

TEST(CompressionTest, SingleByte) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    
    ByteBuffer single = {0x42};
    auto result = comp.compress(single);
    
    EXPECT_TRUE(result.is_success());
}

TEST(CompressionTest, LargeData) {
    CompressionManager comp(CompressionAlgorithm::GZIP);
    
    ByteBuffer large(1024 * 1024, 0x55); // 1MB
    auto result = comp.compress(large);
    
    EXPECT_TRUE(result.is_success());
    EXPECT_LT(result.value->size(), large.size() / 5);
}

// ============================================================================
// Algorithm Switching
// ============================================================================

TEST(CompressionTest, SwitchAlgorithmZlibToGzip) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    
    ByteBuffer data(1000, 0xAA);
    auto zlib_result = comp.compress(data);
    
    EXPECT_TRUE(comp.set_algorithm(CompressionAlgorithm::GZIP).is_success());
    EXPECT_EQ(comp.get_algorithm(), CompressionAlgorithm::GZIP);
    
    auto gzip_result = comp.compress(data);
    EXPECT_TRUE(gzip_result.is_success());
}

TEST(CompressionTest, SwitchToNone) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    
    EXPECT_TRUE(comp.set_algorithm(CompressionAlgorithm::NONE).is_success());
    
    ByteBuffer data(100, 0xBB);
    auto result = comp.compress(data);
    
    EXPECT_TRUE(result.is_success());
    EXPECT_EQ(*result.value, data);
}

// ============================================================================
// Static Utility Tests
// ============================================================================

TEST(CompressionUtilsTest, EstimateCompressedSize) {
    ByteBuffer data(1000, 0xCC);
    
    auto estimate = CompressionManager::estimate_compressed_size(
        data, 
        CompressionAlgorithm::ZLIB
    );
    
    EXPECT_TRUE(estimate.is_success());
    EXPECT_GT(*estimate.value, 0);
}

TEST(CompressionUtilsTest, ShouldCompressLargeData) {
    ByteBuffer large(2000, 0xDD);
    size_t threshold = 1000;
    
    bool should = CompressionManager::should_compress(
        large,
        threshold,
        CompressionAlgorithm::ZLIB
    );
    
    EXPECT_TRUE(should);
}

TEST(CompressionUtilsTest, ShouldNotCompressSmallData) {
    ByteBuffer small(500, 0xEE);
    size_t threshold = 1000;
    
    bool should = CompressionManager::should_compress(
        small,
        threshold,
        CompressionAlgorithm::ZLIB
    );
    
    EXPECT_FALSE(should);
}

TEST(CompressionUtilsTest, ShouldNotCompressWithNone) {
    ByteBuffer data(2000, 0xFF);
    size_t threshold = 1000;
    
    bool should = CompressionManager::should_compress(
        data,
        threshold,
        CompressionAlgorithm::NONE
    );
    
    EXPECT_FALSE(should);
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST(CompressionPerformanceTest, CompressionSpeed) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    ByteBuffer data(10 * 1024, 0xAA);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; ++i) {
        auto result = comp.compress(data);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 200); // 100 compressions in < 200ms
}

TEST(CompressionPerformanceTest, DecompressionSpeed) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    ByteBuffer original(10 * 1024, 0xBB);
    
    auto compressed = comp.compress(original);
    ASSERT_TRUE(compressed.is_success());
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; ++i) {
        auto result = comp.decompress(*compressed.value);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 100); // 100 decompressions in < 100ms
}

// ============================================================================
// Error Handling
// ============================================================================

TEST(CompressionErrorTest, DecompressInvalidData) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    
    ByteBuffer invalid = {0xFF, 0xFF, 0xFF, 0xFF};
    auto result = comp.decompress(invalid);
    
    EXPECT_FALSE(result.is_success());
}

TEST(CompressionErrorTest, DecompressCorruptedData) {
    CompressionManager comp(CompressionAlgorithm::ZLIB);
    
    ByteBuffer original(100, 0xAA);
    auto compressed = comp.compress(original);
    ASSERT_TRUE(compressed.is_success());
    
    // Corrupt the data
    auto corrupted = *compressed.value;
    if (!corrupted.empty()) {
        corrupted[corrupted.size() / 2] ^= 0xFF;
    }
    
    auto result = comp.decompress(corrupted);
    EXPECT_FALSE(result.is_success());
}

TEST(CompressionErrorTest, InvalidAlgorithm) {
    CompressionManager comp(static_cast<CompressionAlgorithm>(999));
    
    ByteBuffer data(100, 0xCC);
    auto result = comp.compress(data);
    
    EXPECT_FALSE(result.is_success());
    EXPECT_EQ(result.code, ErrorCode::NOT_IMPLEMENTED);
}

// ============================================================================
// LZ4 Tests (if available)
// ============================================================================

TEST(CompressionLZ4Test, LZ4CompressionIfAvailable) {
    CompressionManager comp(CompressionAlgorithm::LZ4);
    
    ByteBuffer data(1000, 0xAA);
    auto result = comp.compress(data);
    
#ifdef HAVE_LZ4
    EXPECT_TRUE(result.is_success());
#else
    EXPECT_FALSE(result.is_success());
    EXPECT_EQ(result.code, ErrorCode::NOT_IMPLEMENTED);
#endif
}

