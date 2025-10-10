/**
 * UMICP Buffer - Comprehensive Tests
 * 100% coverage of buffer operations
 */

#include "buffer.h"
#include "umicp_types.h"
#include <gtest/gtest.h>
#include <thread>

using namespace umicp;

// ============================================================================
// Buffer Creation Tests
// ============================================================================

class BufferTest : public ::testing::Test {
protected:
    void SetUp() override {
        buffer = std::make_unique<CircularBuffer>(1024);
    }

    std::unique_ptr<CircularBuffer> buffer;
};

TEST_F(BufferTest, DefaultConstruction) {
    CircularBuffer buf(256);
    EXPECT_EQ(buf.capacity(), 256);
    EXPECT_EQ(buf.size(), 0);
    EXPECT_TRUE(buf.empty());
}

TEST_F(BufferTest, InitialStateIsEmpty) {
    EXPECT_TRUE(buffer->empty());
    EXPECT_EQ(buffer->size(), 0);
    EXPECT_EQ(buffer->capacity(), 1024);
    EXPECT_EQ(buffer->available(), 1024);
}

TEST_F(BufferTest, CapacityIsCorrect) {
    EXPECT_EQ(buffer->capacity(), 1024);
}

TEST_F(BufferTest, AvailableSpaceInitially) {
    EXPECT_EQ(buffer->available(), 1024);
}

// ============================================================================
// Write Operations
// ============================================================================

TEST_F(BufferTest, WriteSingleByte) {
    uint8_t byte = 0x42;
    auto result = buffer->write(&byte, 1);

    EXPECT_TRUE(result.is_ok());
    EXPECT_FALSE(buffer->empty());
    EXPECT_EQ(buffer->size(), 1);
    EXPECT_EQ(buffer->available(), 1023);
}

TEST_F(BufferTest, WriteMultipleBytes) {
    ByteBuffer data = {1, 2, 3, 4, 5};
    auto result = buffer->write(data.data(), data.size());

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(buffer->size(), 5);
    EXPECT_EQ(buffer->available(), 1019);
}

TEST_F(BufferTest, WriteFullCapacity) {
    ByteBuffer data(1024, 0xAA);
    auto result = buffer->write(data.data(), data.size());

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(buffer->size(), 1024);
    EXPECT_EQ(buffer->available(), 0);
    EXPECT_TRUE(buffer->full());
}

TEST_F(BufferTest, WriteExceedingCapacityFails) {
    ByteBuffer data(2048, 0xBB);
    auto result = buffer->write(data.data(), data.size());

    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::BUFFER_OVERFLOW);
}

TEST_F(BufferTest, WriteNullPointerFails) {
    auto result = buffer->write(nullptr, 10);
    EXPECT_FALSE(result.is_ok());
}

TEST_F(BufferTest, WriteZeroByteSucceeds) {
    auto result = buffer->write(nullptr, 0);
    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(buffer->size(), 0);
}

// ============================================================================
// Read Operations
// ============================================================================

TEST_F(BufferTest, ReadSingleByte) {
    uint8_t write_byte = 0x42;
    buffer->write(&write_byte, 1);

    uint8_t read_byte = 0;
    auto result = buffer->read(&read_byte, 1);

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(read_byte, 0x42);
    EXPECT_TRUE(buffer->empty());
}

TEST_F(BufferTest, ReadMultipleBytes) {
    ByteBuffer write_data = {1, 2, 3, 4, 5};
    buffer->write(write_data.data(), write_data.size());

    ByteBuffer read_data(5);
    auto result = buffer->read(read_data.data(), read_data.size());

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(read_data, write_data);
    EXPECT_TRUE(buffer->empty());
}

TEST_F(BufferTest, ReadFromEmptyBufferFails) {
    uint8_t byte;
    auto result = buffer->read(&byte, 1);

    EXPECT_FALSE(result.is_ok());
}

TEST_F(BufferTest, ReadMoreThanAvailableFails) {
    ByteBuffer data = {1, 2, 3};
    buffer->write(data.data(), data.size());

    ByteBuffer read_buf(10);
    auto result = buffer->read(read_buf.data(), 10);

    EXPECT_FALSE(result.is_ok());
}

TEST_F(BufferTest, ReadNullPointerFails) {
    ByteBuffer data = {1, 2, 3};
    buffer->write(data.data(), data.size());

    auto result = buffer->read(nullptr, 3);
    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// Peek Operations
// ============================================================================

TEST_F(BufferTest, PeekDoesNotConsume) {
    ByteBuffer data = {1, 2, 3, 4, 5};
    buffer->write(data.data(), data.size());

    ByteBuffer peek_data(3);
    auto result = buffer->peek(peek_data.data(), 3);

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(peek_data[0], 1);
    EXPECT_EQ(peek_data[1], 2);
    EXPECT_EQ(peek_data[2], 3);
    EXPECT_EQ(buffer->size(), 5); // Still 5 bytes
}

TEST_F(BufferTest, PeekFromEmptyBufferFails) {
    uint8_t byte;
    auto result = buffer->peek(&byte, 1);

    EXPECT_FALSE(result.is_ok());
}

TEST_F(BufferTest, PeekMoreThanAvailableFails) {
    ByteBuffer data = {1, 2, 3};
    buffer->write(data.data(), data.size());

    ByteBuffer peek_buf(10);
    auto result = buffer->peek(peek_buf.data(), 10);

    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// Skip Operations
// ============================================================================

TEST_F(BufferTest, SkipBytes) {
    ByteBuffer data = {1, 2, 3, 4, 5};
    buffer->write(data.data(), data.size());

    auto result = buffer->skip(2);

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(buffer->size(), 3);

    uint8_t next_byte;
    buffer->read(&next_byte, 1);
    EXPECT_EQ(next_byte, 3); // Skipped 1 and 2
}

TEST_F(BufferTest, SkipMoreThanAvailableFails) {
    ByteBuffer data = {1, 2, 3};
    buffer->write(data.data(), data.size());

    auto result = buffer->skip(10);
    EXPECT_FALSE(result.is_ok());
}

TEST_F(BufferTest, SkipZeroBytes) {
    auto result = buffer->skip(0);
    EXPECT_TRUE(result.is_ok());
}

TEST_F(BufferTest, SkipAllBytes) {
    ByteBuffer data = {1, 2, 3, 4, 5};
    buffer->write(data.data(), data.size());

    auto result = buffer->skip(5);

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(buffer->empty());
}

// ============================================================================
// Clear and Reset
// ============================================================================

TEST_F(BufferTest, ClearEmptiesBuffer) {
    ByteBuffer data = {1, 2, 3, 4, 5};
    buffer->write(data.data(), data.size());

    buffer->clear();

    EXPECT_TRUE(buffer->empty());
    EXPECT_EQ(buffer->size(), 0);
    EXPECT_EQ(buffer->available(), 1024);
}

TEST_F(BufferTest, ClearEmptyBufferWorks) {
    buffer->clear();

    EXPECT_TRUE(buffer->empty());
    EXPECT_EQ(buffer->size(), 0);
}

TEST_F(BufferTest, ClearFullBufferWorks) {
    ByteBuffer data(1024, 0xFF);
    buffer->write(data.data(), data.size());

    buffer->clear();

    EXPECT_TRUE(buffer->empty());
    EXPECT_EQ(buffer->available(), 1024);
}

// ============================================================================
// Circular Buffer Behavior
// ============================================================================

TEST_F(BufferTest, CircularWrapAround) {
    // Fill buffer
    ByteBuffer data(1024, 0xAA);
    buffer->write(data.data(), data.size());

    // Read half
    ByteBuffer read_buf(512);
    buffer->read(read_buf.data(), 512);

    // Write more (should wrap around)
    ByteBuffer more_data(512, 0xBB);
    auto result = buffer->write(more_data.data(), more_data.size());

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(buffer->size(), 1024); // Full again
}

TEST_F(BufferTest, MultipleReadWriteCycles) {
    for (int cycle = 0; cycle < 10; ++cycle) {
        // Write
        ByteBuffer write_data(100, cycle);
        buffer->write(write_data.data(), write_data.size());

        // Read
        ByteBuffer read_data(100);
        buffer->read(read_data.data(), read_data.size());

        EXPECT_EQ(read_data[0], static_cast<uint8_t>(cycle));
    }

    EXPECT_TRUE(buffer->empty());
}

// ============================================================================
// Resize Operations
// ============================================================================

TEST_F(BufferTest, ResizeIncreasesCapacity) {
    auto result = buffer->resize(2048);

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(buffer->capacity(), 2048);
    EXPECT_EQ(buffer->available(), 2048);
}

TEST_F(BufferTest, ResizePreservesData) {
    ByteBuffer data = {1, 2, 3, 4, 5};
    buffer->write(data.data(), data.size());

    buffer->resize(2048);

    EXPECT_EQ(buffer->size(), 5);

    ByteBuffer read_data(5);
    buffer->read(read_data.data(), 5);
    EXPECT_EQ(read_data, data);
}

TEST_F(BufferTest, ResizeToSmallerSizeFails) {
    ByteBuffer data(1024, 0);
    buffer->write(data.data(), data.size());

    auto result = buffer->resize(512);

    // Should fail if data would be lost
    EXPECT_FALSE(result.is_ok());
}

TEST_F(BufferTest, ResizeEmptyBuffer) {
    auto result = buffer->resize(4096);

    EXPECT_TRUE(result.is_ok());
    EXPECT_EQ(buffer->capacity(), 4096);
    EXPECT_TRUE(buffer->empty());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(BufferTest, WriteReadAlternating) {
    for (int i = 0; i < 100; ++i) {
        uint8_t byte = i % 256;
        buffer->write(&byte, 1);

        uint8_t read_byte;
        buffer->read(&read_byte, 1);

        EXPECT_EQ(read_byte, byte);
    }
}

TEST_F(BufferTest, FillAndDrainMultipleTimes) {
    for (int round = 0; round < 5; ++round) {
        ByteBuffer data(1024, round);
        buffer->write(data.data(), data.size());
        EXPECT_TRUE(buffer->full());

        ByteBuffer read_data(1024);
        buffer->read(read_data.data(), 1024);
        EXPECT_TRUE(buffer->empty());
        EXPECT_EQ(read_data[0], static_cast<uint8_t>(round));
    }
}

TEST_F(BufferTest, VerySmallReadsAndWrites) {
    // Write 1024 bytes, 1 at a time
    for (int i = 0; i < 1024; ++i) {
        uint8_t byte = i % 256;
        buffer->write(&byte, 1);
    }

    EXPECT_TRUE(buffer->full());

    // Read 1024 bytes, 1 at a time
    for (int i = 0; i < 1024; ++i) {
        uint8_t byte;
        buffer->read(&byte, 1);
        EXPECT_EQ(byte, i % 256);
    }

    EXPECT_TRUE(buffer->empty());
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(BufferThreadSafetyTest, ConcurrentWrites) {
    CircularBuffer shared_buffer(10000);
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i, &shared_buffer, &success_count]() {
            ByteBuffer data(100, i);
            auto result = shared_buffer.write(data.data(), data.size());
            if (result.is_ok()) {
                success_count++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Some writes should succeed
    EXPECT_GT(success_count, 0);
    EXPECT_LE(shared_buffer.size(), 10000);
}

TEST(BufferThreadSafetyTest, ConcurrentReads) {
    CircularBuffer shared_buffer(10000);

    // Pre-fill buffer
    ByteBuffer data(10000, 0xAA);
    shared_buffer.write(data.data(), data.size());

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&shared_buffer, &success_count]() {
            ByteBuffer read_buf(100);
            auto result = shared_buffer.read(read_buf.data(), read_buf.size());
            if (result.is_ok()) {
                success_count++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_GT(success_count, 0);
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST(BufferPerformanceTest, SequentialWritePerformance) {
    CircularBuffer buffer(1024 * 1024); // 1MB
    ByteBuffer data(1024, 0); // 1KB

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        buffer.clear();
        buffer.write(data.data(), data.size());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete 1000 writes in < 10ms
    EXPECT_LT(duration.count(), 10);
}

TEST(BufferPerformanceTest, SequentialReadPerformance) {
    CircularBuffer buffer(1024 * 1024);
    ByteBuffer data(1024, 0xFF);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        buffer.clear();
        buffer.write(data.data(), data.size());

        ByteBuffer read_data(1024);
        buffer.read(read_data.data(), read_data.size());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete 1000 cycles in < 20ms
    EXPECT_LT(duration.count(), 20);
}

TEST(BufferPerformanceTest, LargeBlockTransfer) {
    CircularBuffer buffer(10 * 1024 * 1024); // 10MB
    ByteBuffer large_data(10 * 1024 * 1024, 0xAB);

    auto start = std::chrono::high_resolution_clock::now();

    buffer.write(large_data.data(), large_data.size());

    ByteBuffer read_data(10 * 1024 * 1024);
    buffer.read(read_data.data(), read_data.size());

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 10MB transfer should complete in < 50ms
    EXPECT_LT(duration.count(), 50);
    EXPECT_EQ(read_data, large_data);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST(BufferStressTest, RapidReadWriteCycles) {
    CircularBuffer buffer(1024);

    for (int i = 0; i < 10000; ++i) {
        uint8_t byte = i % 256;
        buffer.write(&byte, 1);

        uint8_t read_byte;
        buffer.read(&read_byte, 1);

        EXPECT_EQ(read_byte, byte);
    }
}

TEST(BufferStressTest, FragmentedWrites) {
    CircularBuffer buffer(1024);

    // Write in small random-sized chunks
    size_t total_written = 0;
    while (total_written < 1024) {
        size_t chunk_size = std::min(size_t(1 + (rand() % 64)), 1024 - total_written);
        ByteBuffer chunk(chunk_size, 0xCC);

        auto result = buffer.write(chunk.data(), chunk.size());
        if (result.is_ok()) {
            total_written += chunk_size;
        }
    }

    EXPECT_EQ(buffer.size(), 1024);
}

TEST(BufferStressTest, ManySmallBuffers) {
    std::vector<std::unique_ptr<CircularBuffer>> buffers;

    for (int i = 0; i < 1000; ++i) {
        buffers.push_back(std::make_unique<CircularBuffer>(128));

        ByteBuffer data(64, i % 256);
        buffers[i]->write(data.data(), data.size());
    }

    EXPECT_EQ(buffers.size(), 1000);

    // Verify some buffers
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(buffers[i]->size(), 64);
    }
}

// ============================================================================
// Various Buffer Sizes
// ============================================================================

TEST(BufferSizeTest, TinyBuffer) {
    CircularBuffer tiny(16);
    EXPECT_EQ(tiny.capacity(), 16);

    ByteBuffer data = {1, 2, 3, 4, 5};
    auto result = tiny.write(data.data(), data.size());
    EXPECT_TRUE(result.is_ok());
}

TEST(BufferSizeTest, SmallBuffer) {
    CircularBuffer small(256);
    EXPECT_EQ(small.capacity(), 256);
}

TEST(BufferSizeTest, MediumBuffer) {
    CircularBuffer medium(4096);
    EXPECT_EQ(medium.capacity(), 4096);
}

TEST(BufferSizeTest, LargeBuffer) {
    CircularBuffer large(1024 * 1024);
    EXPECT_EQ(large.capacity(), 1024 * 1024);
}

TEST(BufferSizeTest, HugeBuffer) {
    CircularBuffer huge(100 * 1024 * 1024); // 100MB
    EXPECT_EQ(huge.capacity(), 100 * 1024 * 1024);
}

// ============================================================================
// Data Integrity Tests
// ============================================================================

TEST(BufferIntegrityTest, ReadWriteIntegrity) {
    CircularBuffer buffer(1024);

    // Write pattern
    ByteBuffer pattern;
    for (int i = 0; i < 256; ++i) {
        pattern.push_back(i);
    }

    buffer.write(pattern.data(), pattern.size());

    // Read back
    ByteBuffer read_pattern(256);
    buffer.read(read_pattern.data(), read_pattern.size());

    // Verify integrity
    for (int i = 0; i < 256; ++i) {
        EXPECT_EQ(read_pattern[i], i);
    }
}

TEST(BufferIntegrityTest, LargeDataIntegrity) {
    CircularBuffer buffer(10 * 1024 * 1024);

    // Create unique pattern
    ByteBuffer pattern;
    for (int i = 0; i < 10000; ++i) {
        pattern.push_back((i * 17 + 42) % 256);
    }

    buffer.write(pattern.data(), pattern.size());

    ByteBuffer read_back(10000);
    buffer.read(read_back.data(), read_back.size());

    EXPECT_EQ(read_back, pattern);
}

// ============================================================================
// Boundary Conditions
// ============================================================================

TEST(BufferBoundaryTest, WriteExactlyCapacity) {
    CircularBuffer buffer(512);
    ByteBuffer data(512, 0x55);

    auto result = buffer.write(data.data(), data.size());

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(buffer.full());
    EXPECT_EQ(buffer.available(), 0);
}

TEST(BufferBoundaryTest, WriteCapacityPlusOne) {
    CircularBuffer buffer(512);
    ByteBuffer data(513, 0x66);

    auto result = buffer.write(data.data(), data.size());

    EXPECT_FALSE(result.is_ok());
}

TEST(BufferBoundaryTest, ReadExactlyAvailable) {
    CircularBuffer buffer(512);
    ByteBuffer data(100, 0x77);
    buffer.write(data.data(), data.size());

    ByteBuffer read_data(100);
    auto result = buffer.read(read_data.data(), read_data.size());

    EXPECT_TRUE(result.is_ok());
    EXPECT_TRUE(buffer.empty());
}

TEST(BufferBoundaryTest, ReadAvailablePlusOne) {
    CircularBuffer buffer(512);
    ByteBuffer data(100, 0x88);
    buffer.write(data.data(), data.size());

    ByteBuffer read_data(101);
    auto result = buffer.read(read_data.data(), read_data.size());

    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// Error Recovery Tests
// ============================================================================

TEST(BufferErrorTest, RecoverFromOverflow) {
    CircularBuffer buffer(100);

    // Try to overflow
    ByteBuffer too_much(200, 0);
    auto result1 = buffer.write(too_much.data(), too_much.size());
    EXPECT_FALSE(result1.is_ok());

    // Should still be able to write valid data
    ByteBuffer valid(50, 0x99);
    auto result2 = buffer.write(valid.data(), valid.size());
    EXPECT_TRUE(result2.is_ok());
}

TEST(BufferErrorTest, RecoverFromUnderflow) {
    CircularBuffer buffer(100);

    // Try to read from empty
    uint8_t byte;
    auto result1 = buffer.read(&byte, 1);
    EXPECT_FALSE(result1.is_ok());

    // Should still be able to write and read
    buffer.write(&byte, 1);
    auto result2 = buffer.read(&byte, 1);
    EXPECT_TRUE(result2.is_ok());
}

// ============================================================================
// Multiple Buffers
// ============================================================================

TEST(BufferMultipleTest, IndependentBuffers) {
    CircularBuffer buf1(512);
    CircularBuffer buf2(512);
    CircularBuffer buf3(512);

    ByteBuffer data1(100, 1);
    ByteBuffer data2(200, 2);
    ByteBuffer data3(300, 3);

    buf1.write(data1.data(), data1.size());
    buf2.write(data2.data(), data2.size());
    buf3.write(data3.data(), data3.size());

    EXPECT_EQ(buf1.size(), 100);
    EXPECT_EQ(buf2.size(), 200);
    EXPECT_EQ(buf3.size(), 300);
}

TEST(BufferMultipleTest, TransferBetweenBuffers) {
    CircularBuffer source(512);
    CircularBuffer dest(512);

    ByteBuffer data(256, 0xAA);
    source.write(data.data(), data.size());

    ByteBuffer transfer(256);
    source.read(transfer.data(), transfer.size());
    dest.write(transfer.data(), transfer.size());

    EXPECT_TRUE(source.empty());
    EXPECT_EQ(dest.size(), 256);
}

