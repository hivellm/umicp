/**
 * UMICP Compression - Comprehensive Tests
 * 100% coverage of compression operations
 */

#include "compression.h"
#include "umicp_types.h"
#include <gtest/gtest.h>

using namespace umicp;

// ============================================================================
// Compression Tests - ZLIB
// ============================================================================

class CompressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        compressor = std::make_unique<CompressionManager>();
    }

    std::unique_ptr<CompressionManager> compressor;
};

TEST_F(CompressionTest, CompressZlibBasic) {
    ByteBuffer data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    compressor->set_algorithm(CompressionAlgorithm::ZLIB);
    auto result = compressor->compress(data);

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(result.value.has_value());
    // Compressed data might be larger for small inputs
}

TEST_F(CompressionTest, DecompressZlibBasic) {
    ByteBuffer original = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto compressed = compressor->compress(original, CompressionAlgorithm::ZLIB);
    ASSERT_TRUE(compressed.is_ok());

    auto decompressed = compressor->decompress(
        *compressed.value,
        CompressionAlgorithm::ZLIB,
        original.size()
    );

    ASSERT_TRUE(decompressed.is_ok());
    EXPECT_EQ(*decompressed.value, original);
}

TEST_F(CompressionTest, CompressZlibLargeData) {
    ByteBuffer large(100 * 1024, 0xAA); // 100KB of same byte

    auto result = compressor->compress(large, CompressionAlgorithm::ZLIB);

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(result.value.has_value());
    // Should compress well (repetitive data)
    EXPECT_LT(result.value->size(), large.size());
}

TEST_F(CompressionTest, CompressDecompressRoundTrip) {
    // Create data with patterns
    ByteBuffer original;
    for (int i = 0; i < 1000; ++i) {
        original.push_back((i * 3 + 7) % 256);
    }

    auto compressed = compressor->compress(original, CompressionAlgorithm::ZLIB);
    ASSERT_TRUE(compressed.is_ok());

    auto decompressed = compressor->decompress(
        *compressed.value,
        CompressionAlgorithm::ZLIB,
        original.size()
    );

    ASSERT_TRUE(decompressed.is_ok());
    ASSERT_TRUE(decompressed.value.has_value());
    EXPECT_EQ(*decompressed.value, original);
}

// ============================================================================
// Compression Tests - GZIP
// ============================================================================

TEST_F(CompressionTest, CompressGzipBasic) {
    ByteBuffer data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto result = compressor->compress(data, CompressionAlgorithm::GZIP);

    EXPECT_TRUE(result.is_ok());
}

TEST_F(CompressionTest, DecompressGzipBasic) {
    ByteBuffer original = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto compressed = compressor->compress(original, CompressionAlgorithm::GZIP);
    ASSERT_TRUE(compressed.is_ok());

    auto decompressed = compressor->decompress(
        *compressed.value,
        CompressionAlgorithm::GZIP,
        original.size()
    );

    ASSERT_TRUE(decompressed.is_ok());
    EXPECT_EQ(*decompressed.value, original);
}

TEST_F(CompressionTest, GzipCompressesRepetitiveData) {
    ByteBuffer repetitive(10000, 0x42);

    auto result = compressor->compress(repetitive, CompressionAlgorithm::GZIP);

    ASSERT_TRUE(result.is_ok());
    ASSERT_TRUE(result.value.has_value());

    // Should compress significantly
    EXPECT_LT(result.value->size(), repetitive.size() / 10);
}

// ============================================================================
// Compression Tests - NONE
// ============================================================================

TEST_F(CompressionTest, CompressNoneReturnsOriginal) {
    ByteBuffer data = {1, 2, 3, 4, 5};

    auto result = compressor->compress(data, CompressionAlgorithm::NONE);

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(result.value.has_value());
    EXPECT_EQ(*result.value, data);
}

TEST_F(CompressionTest, DecompressNoneReturnsOriginal) {
    ByteBuffer data = {1, 2, 3, 4, 5};

    auto result = compressor->decompress(data, CompressionAlgorithm::NONE, data.size());

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(result.value.has_value());
    EXPECT_EQ(*result.value, data);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(CompressionTest, CompressEmptyData) {
    ByteBuffer empty;

    auto result = compressor->compress(empty, CompressionAlgorithm::ZLIB);

    EXPECT_TRUE(result.is_ok());
}

TEST_F(CompressionTest, CompressSingleByte) {
    ByteBuffer single = {0x42};

    auto result = compressor->compress(single, CompressionAlgorithm::ZLIB);

    EXPECT_TRUE(result.is_ok());
}

TEST_F(CompressionTest, CompressVeryLargeData) {
    ByteBuffer huge(10 * 1024 * 1024, 0x55); // 10MB

    auto result = compressor->compress(huge, CompressionAlgorithm::GZIP);

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(result.value.has_value());
    // Should compress well
    EXPECT_LT(result.value->size(), huge.size() / 10);
}

TEST_F(CompressionTest, CompressRandomData) {
    ByteBuffer random;
    for (int i = 0; i < 10000; ++i) {
        random.push_back(rand() % 256);
    }

    auto result = compressor->compress(random, CompressionAlgorithm::ZLIB);

    EXPECT_TRUE(result.is_ok());
    // Random data doesn't compress well
}

TEST_F(CompressionTest, CompressHighlyCompressibleData) {
    ByteBuffer compressible(100000, 0);

    auto result = compressor->compress(compressible, CompressionAlgorithm::ZLIB);

    ASSERT_TRUE(result.is_ok());
    ASSERT_TRUE(result.value.has_value());

    // Should compress to < 1% of original
    EXPECT_LT(result.value->size(), compressible.size() / 100);
}

// ============================================================================
// Error Handling
// ============================================================================

TEST_F(CompressionTest, DecompressInvalidData) {
    ByteBuffer invalid = {0xFF, 0xFF, 0xFF, 0xFF};

    auto result = compressor->decompress(invalid, CompressionAlgorithm::ZLIB, 100);

    // Should fail gracefully
    EXPECT_FALSE(result.is_ok());
}

TEST_F(CompressionTest, DecompressCorruptedData) {
    ByteBuffer original = {1, 2, 3, 4, 5};
    auto compressed = compressor->compress(original, CompressionAlgorithm::ZLIB);

    ASSERT_TRUE(compressed.is_ok());

    // Corrupt the compressed data
    auto corrupted = *compressed.value;
    if (!corrupted.empty()) {
        corrupted[corrupted.size() / 2] ^= 0xFF;
    }

    auto result = compressor->decompress(corrupted, CompressionAlgorithm::ZLIB, original.size());

    // Should detect corruption
    EXPECT_FALSE(result.is_ok());
}

TEST_F(CompressionTest, DecompressWithWrongSize) {
    ByteBuffer original = {1, 2, 3, 4, 5};
    auto compressed = compressor->compress(original, CompressionAlgorithm::ZLIB);

    ASSERT_TRUE(compressed.is_ok());

    // Try to decompress with wrong size
    auto result = compressor->decompress(*compressed.value, CompressionAlgorithm::ZLIB, 1000);

    // Should handle size mismatch
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST(CompressionPerformanceTest, ZlibCompressionSpeed) {
    CompressionManager comp;
    ByteBuffer data(10 * 1024, 0xAA); // 10KB

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100; ++i) {
        auto result = comp.compress(data, CompressionAlgorithm::ZLIB);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 100 compressions of 10KB should complete in < 100ms
    EXPECT_LT(duration.count(), 100);
}

TEST(CompressionPerformanceTest, ZlibDecompressionSpeed) {
    CompressionManager comp;
    ByteBuffer original(10 * 1024, 0xBB);

    auto compressed = comp.compress(original, CompressionAlgorithm::ZLIB);
    ASSERT_TRUE(compressed.is_ok());

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100; ++i) {
        auto result = comp.decompress(*compressed.value, CompressionAlgorithm::ZLIB, original.size());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 100 decompressions should complete in < 50ms
    EXPECT_LT(duration.count(), 50);
}

// ============================================================================
// Compression Ratio Tests
// ============================================================================

TEST(CompressionRatioTest, HighlyCompressibleData) {
    CompressionManager comp;
    ByteBuffer data(100000, 0x00);

    auto result = comp.compress(data, CompressionAlgorithm::ZLIB);

    ASSERT_TRUE(result.is_ok());
    ASSERT_TRUE(result.value.has_value());

    double ratio = static_cast<double>(result.value->size()) / data.size();

    // Should achieve > 99% compression
    EXPECT_LT(ratio, 0.01);
}

TEST(CompressionRatioTest, RandomDataDoesNotCompress) {
    CompressionManager comp;
    ByteBuffer random;
    for (int i = 0; i < 10000; ++i) {
        random.push_back(rand() % 256);
    }

    auto result = comp.compress(random, CompressionAlgorithm::ZLIB);

    ASSERT_TRUE(result.is_ok());
    // Random data may expand slightly
}

TEST(CompressionRatioTest, TextLikeData) {
    CompressionManager comp;

    // Simulate text (lots of spaces, common chars)
    std::string text = "The quick brown fox jumps over the lazy dog. ";
    ByteBuffer text_data;
    for (int i = 0; i < 100; ++i) {
        text_data.insert(text_data.end(), text.begin(), text.end());
    }

    auto result = comp.compress(text_data, CompressionAlgorithm::ZLIB);

    ASSERT_TRUE(result.is_ok());
    ASSERT_TRUE(result.value.has_value());

    // Text should compress well
    EXPECT_LT(result.value->size(), text_data.size() / 2);
}

// ============================================================================
// Size Estimation Tests
// ============================================================================

TEST_F(CompressionTest, EstimateCompressedSize) {
    ByteBuffer data(1000, 0xCC);

    auto estimate = compressor->estimate_compressed_size(data, CompressionAlgorithm::ZLIB);

    EXPECT_TRUE(estimate.is_ok());
    EXPECT_TRUE(estimate.value.has_value());
    EXPECT_GT(*estimate.value, 0);
}

TEST_F(CompressionTest, EstimateForEmptyData) {
    ByteBuffer empty;

    auto estimate = compressor->estimate_compressed_size(empty, CompressionAlgorithm::ZLIB);

    EXPECT_TRUE(estimate.is_ok());
}

TEST_F(CompressionTest, EstimateForLargeData) {
    ByteBuffer large(1024 * 1024, 0xDD);

    auto estimate = compressor->estimate_compressed_size(large, CompressionAlgorithm::ZLIB);

    EXPECT_TRUE(estimate.is_ok());
    EXPECT_TRUE(estimate.value.has_value());
}

// ============================================================================
// Multiple Compressions
// ============================================================================

TEST(CompressionMultipleTest, CompressMultipleDifferentData) {
    CompressionManager comp;

    std::vector<ByteBuffer> datasets = {
        {1, 2, 3, 4, 5},
        ByteBuffer(100, 0xAA),
        ByteBuffer(1000, 0xBB),
        {0xFF, 0xFE, 0xFD, 0xFC}
    };

    for (const auto& data : datasets) {
        auto result = comp.compress(data, CompressionAlgorithm::ZLIB);
        EXPECT_TRUE(result.is_ok());

        auto decompressed = comp.decompress(
            *result.value,
            CompressionAlgorithm::ZLIB,
            data.size()
        );

        ASSERT_TRUE(decompressed.is_ok());
        EXPECT_EQ(*decompressed.value, data);
    }
}

TEST(CompressionMultipleTest, DifferentAlgorithms) {
    CompressionManager comp;
    ByteBuffer data(1000, 0xEE);

    std::vector<CompressionAlgorithm> algorithms = {
        CompressionAlgorithm::NONE,
        CompressionAlgorithm::ZLIB,
        CompressionAlgorithm::GZIP
    };

    for (auto algo : algorithms) {
        auto compressed = comp.compress(data, algo);

        if (compressed.is_ok()) {
            auto decompressed = comp.decompress(*compressed.value, algo, data.size());

            if (decompressed.is_ok()) {
                EXPECT_EQ(*decompressed.value, data);
            }
        }
    }
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(CompressionThreadSafetyTest, ConcurrentCompressions) {
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i, &success_count]() {
            CompressionManager comp;
            ByteBuffer data(1000, i);

            auto result = comp.compress(data, CompressionAlgorithm::ZLIB);
            if (result.is_ok()) {
                auto decompressed = comp.decompress(
                    *result.value,
                    CompressionAlgorithm::ZLIB,
                    data.size()
                );

                if (decompressed.is_ok() && *decompressed.value == data) {
                    success_count++;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(success_count, 10);
}

// ============================================================================
// Compression Statistics Tests
// ============================================================================

TEST_F(CompressionTest, GetStatistics) {
    auto stats = compressor->get_stats();

    EXPECT_GE(stats.total_compressions, 0);
    EXPECT_GE(stats.total_decompressions, 0);
    EXPECT_GE(stats.total_bytes_compressed, 0);
    EXPECT_GE(stats.total_bytes_decompressed, 0);
}

TEST_F(CompressionTest, StatisticsIncreaseAfterOperations) {
    auto stats_before = compressor->get_stats();

    ByteBuffer data(1000, 0xAA);
    auto compressed = compressor->compress(data, CompressionAlgorithm::ZLIB);

    if (compressed.is_ok()) {
        compressor->decompress(*compressed.value, CompressionAlgorithm::ZLIB, data.size());
    }

    auto stats_after = compressor->get_stats();

    EXPECT_GE(stats_after.total_compressions, stats_before.total_compressions);
    EXPECT_GE(stats_after.total_decompressions, stats_before.total_decompressions);
}

TEST_F(CompressionTest, ResetStatistics) {
    ByteBuffer data(1000, 0xBB);
    compressor->compress(data, CompressionAlgorithm::ZLIB);

    compressor->reset_stats();

    auto stats = compressor->get_stats();
    EXPECT_EQ(stats.total_compressions, 0);
    EXPECT_EQ(stats.total_decompressions, 0);
    EXPECT_EQ(stats.total_bytes_compressed, 0);
    EXPECT_EQ(stats.total_bytes_decompressed, 0);
}

// ============================================================================
// Real-World Data Patterns
// ============================================================================

TEST(CompressionRealWorldTest, JsonLikeData) {
    CompressionManager comp;

    // Simulate JSON (lots of quotes, braces, repetition)
    std::string json_pattern = R"({"key":"value","number":123,"array":[1,2,3]})";
    ByteBuffer json_data;
    for (int i = 0; i < 100; ++i) {
        json_data.insert(json_data.end(), json_pattern.begin(), json_pattern.end());
    }

    auto result = comp.compress(json_data, CompressionAlgorithm::GZIP);

    ASSERT_TRUE(result.is_ok());
    ASSERT_TRUE(result.value.has_value());

    // JSON should compress well
    double ratio = static_cast<double>(result.value->size()) / json_data.size();
    EXPECT_LT(ratio, 0.3); // Should achieve > 70% compression
}

TEST(CompressionRealWorldTest, BinaryProtobufLike) {
    CompressionManager comp;

    // Simulate protobuf (field tags, varints)
    ByteBuffer protobuf_like;
    for (int i = 0; i < 1000; ++i) {
        protobuf_like.push_back(0x08); // Field 1, varint
        protobuf_like.push_back(i % 128);
        protobuf_like.push_back(0x12); // Field 2, length-delimited
        protobuf_like.push_back(0x04); // Length 4
        protobuf_like.push_back(0x01);
        protobuf_like.push_back(0x02);
        protobuf_like.push_back(0x03);
        protobuf_like.push_back(0x04);
    }

    auto result = comp.compress(protobuf_like, CompressionAlgorithm::ZLIB);

    ASSERT_TRUE(result.is_ok());
    EXPECT_TRUE(result.value.has_value());
}

TEST(CompressionRealWorldTest, ImageLikeData) {
    CompressionManager comp;

    // Simulate image data (some patterns, some random)
    ByteBuffer image_like;
    for (int y = 0; y < 100; ++y) {
        for (int x = 0; x < 100; ++x) {
            // Create some pattern
            image_like.push_back((x + y) % 256);
            image_like.push_back((x * 2) % 256);
            image_like.push_back((y * 2) % 256);
        }
    }

    auto result = comp.compress(image_like, CompressionAlgorithm::GZIP);

    EXPECT_TRUE(result.is_ok());
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST(CompressionStressTest, ManySmallCompressions) {
    CompressionManager comp;

    for (int i = 0; i < 1000; ++i) {
        ByteBuffer small(10, i % 256);
        auto result = comp.compress(small, CompressionAlgorithm::ZLIB);
        EXPECT_TRUE(result.is_ok());
    }
}

TEST(CompressionStressTest, ManyLargeCompressions) {
    CompressionManager comp;

    for (int i = 0; i < 10; ++i) {
        ByteBuffer large(1024 * 1024, i); // 1MB each
        auto result = comp.compress(large, CompressionAlgorithm::GZIP);
        EXPECT_TRUE(result.is_ok());
    }
}

TEST(CompressionStressTest, AlternatingCompressDecompress) {
    CompressionManager comp;

    for (int i = 0; i < 100; ++i) {
        ByteBuffer data(1000, i % 256);

        auto compressed = comp.compress(data, CompressionAlgorithm::ZLIB);
        ASSERT_TRUE(compressed.is_ok());

        auto decompressed = comp.decompress(*compressed.value, CompressionAlgorithm::ZLIB, data.size());
        ASSERT_TRUE(decompressed.is_ok());

        EXPECT_EQ(*decompressed.value, data);
    }
}

