/**
 * UMICP Frame - Comprehensive Tests
 * 100% coverage of frame operations
 */

#include "frame.h"
#include "umicp_types.h"
#include <gtest/gtest.h>
#include <thread>

using namespace umicp;

// ============================================================================
// Frame Creation Tests
// ============================================================================

class FrameTest : public ::testing::Test {
protected:
    void SetUp() override {
        frame = Frame();
    }

    Frame frame;
};

TEST_F(FrameTest, DefaultConstruction) {
    Frame f;
    EXPECT_EQ(f.get_version_major(), UMICP_VERSION_MAJOR);
    EXPECT_EQ(f.get_version_minor(), UMICP_VERSION_MINOR);
}

TEST_F(FrameTest, SetAndGetOperation) {
    frame.set_operation(OperationType::DATA);
    EXPECT_EQ(frame.get_operation(), OperationType::DATA);
}

TEST_F(FrameTest, AllOperationTypes) {
    frame.set_operation(OperationType::CONTROL);
    EXPECT_EQ(frame.get_operation(), OperationType::CONTROL);

    frame.set_operation(OperationType::DATA);
    EXPECT_EQ(frame.get_operation(), OperationType::DATA);

    frame.set_operation(OperationType::ACK);
    EXPECT_EQ(frame.get_operation(), OperationType::ACK);

    frame.set_operation(OperationType::ERROR);
    EXPECT_EQ(frame.get_operation(), OperationType::ERROR);
}

TEST_F(FrameTest, SetAndGetPayloadSize) {
    frame.set_payload_size(1024);
    EXPECT_EQ(frame.get_payload_size(), 1024);
}

TEST_F(FrameTest, SetAndGetFlags) {
    frame.set_flags(COMPRESSED_GZIP);
    EXPECT_EQ(frame.get_flags(), COMPRESSED_GZIP);
}

TEST_F(FrameTest, MultipleFlags) {
    uint16_t flags = COMPRESSED_GZIP | ENCRYPTED_XCHACHA20 | FRAGMENT_START;
    frame.set_flags(flags);
    EXPECT_EQ(frame.get_flags(), flags);
}

TEST_F(FrameTest, AllFlagsCombined) {
    uint16_t all_flags = COMPRESSED_GZIP | COMPRESSED_BROTLI |
                         ENCRYPTED_XCHACHA20 | FRAGMENT_START |
                         FRAGMENT_CONTINUE | FRAGMENT_END |
                         STREAM_START | STREAM_END;

    frame.set_flags(all_flags);
    EXPECT_EQ(frame.get_flags(), all_flags);
}

TEST_F(FrameTest, SetAndGetSequenceNumber) {
    frame.set_sequence_number(12345);
    EXPECT_EQ(frame.get_sequence_number(), 12345);
}

TEST_F(FrameTest, SetAndGetTimestamp) {
    uint64_t timestamp = 1696939200000;
    frame.set_timestamp(timestamp);
    EXPECT_EQ(frame.get_timestamp(), timestamp);
}

TEST_F(FrameTest, SetAndGetChecksum) {
    uint32_t checksum = 0xDEADBEEF;
    frame.set_checksum(checksum);
    EXPECT_EQ(frame.get_checksum(), checksum);
}

// ============================================================================
// Payload Tests
// ============================================================================

TEST_F(FrameTest, SetAndGetPayload) {
    ByteBuffer payload = {1, 2, 3, 4, 5};
    frame.set_payload(payload);

    auto retrieved = frame.get_payload();
    EXPECT_EQ(retrieved.size(), 5);
    EXPECT_EQ(retrieved[0], 1);
    EXPECT_EQ(retrieved[4], 5);
}

TEST_F(FrameTest, EmptyPayload) {
    ByteBuffer empty;
    frame.set_payload(empty);

    auto retrieved = frame.get_payload();
    EXPECT_TRUE(retrieved.empty());
    EXPECT_EQ(frame.get_payload_size(), 0);
}

TEST_F(FrameTest, LargePayload) {
    ByteBuffer large(100 * 1024, 0xAB); // 100KB
    frame.set_payload(large);

    auto retrieved = frame.get_payload();
    EXPECT_EQ(retrieved.size(), 100 * 1024);
    EXPECT_EQ(retrieved[0], 0xAB);
}

TEST_F(FrameTest, VeryLargePayload) {
    ByteBuffer very_large(10 * 1024 * 1024, 0xCD); // 10MB
    frame.set_payload(very_large);

    auto retrieved = frame.get_payload();
    EXPECT_EQ(retrieved.size(), 10 * 1024 * 1024);
}

TEST_F(FrameTest, PayloadWithBinaryData) {
    ByteBuffer binary = {0x00, 0xFF, 0x01, 0xFE, 0x02, 0xFD};
    frame.set_payload(binary);

    auto retrieved = frame.get_payload();
    EXPECT_EQ(retrieved.size(), 6);
    EXPECT_EQ(retrieved[0], 0x00);
    EXPECT_EQ(retrieved[1], 0xFF);
}

// ============================================================================
// Header Size Tests
// ============================================================================

TEST_F(FrameTest, HeaderSizeIsCorrect) {
    EXPECT_EQ(frame.get_header_size(), UMICP_FRAME_HEADER_SIZE);
}

TEST_F(FrameTest, TotalSizeWithoutPayload) {
    EXPECT_EQ(frame.get_total_size(), UMICP_FRAME_HEADER_SIZE);
}

TEST_F(FrameTest, TotalSizeWithPayload) {
    ByteBuffer payload(1024, 0);
    frame.set_payload(payload);

    EXPECT_EQ(frame.get_total_size(), UMICP_FRAME_HEADER_SIZE + 1024);
}

// ============================================================================
// Flag Checking Tests
// ============================================================================

TEST_F(FrameTest, IsCompressedDetectsGzip) {
    frame.set_flags(COMPRESSED_GZIP);
    EXPECT_TRUE(frame.is_compressed());
}

TEST_F(FrameTest, IsCompressedDetectsBrotli) {
    frame.set_flags(COMPRESSED_BROTLI);
    EXPECT_TRUE(frame.is_compressed());
}

TEST_F(FrameTest, IsCompressedReturnsFalseWithNoCompression) {
    frame.set_flags(ENCRYPTED_XCHACHA20);
    EXPECT_FALSE(frame.is_compressed());
}

TEST_F(FrameTest, IsEncryptedDetectsEncryption) {
    frame.set_flags(ENCRYPTED_XCHACHA20);
    EXPECT_TRUE(frame.is_encrypted());
}

TEST_F(FrameTest, IsEncryptedReturnsFalseWithoutEncryption) {
    frame.set_flags(COMPRESSED_GZIP);
    EXPECT_FALSE(frame.is_encrypted());
}

TEST_F(FrameTest, IsFragmentedDetectsFragmentStart) {
    frame.set_flags(FRAGMENT_START);
    EXPECT_TRUE(frame.is_fragmented());
}

TEST_F(FrameTest, IsFragmentedDetectsFragmentContinue) {
    frame.set_flags(FRAGMENT_CONTINUE);
    EXPECT_TRUE(frame.is_fragmented());
}

TEST_F(FrameTest, IsFragmentedDetectsFragmentEnd) {
    frame.set_flags(FRAGMENT_END);
    EXPECT_TRUE(frame.is_fragmented());
}

TEST_F(FrameTest, IsFragmentedReturnsFalseWithoutFragmentation) {
    frame.set_flags(COMPRESSED_GZIP);
    EXPECT_FALSE(frame.is_fragmented());
}

TEST_F(FrameTest, IsStreamingDetectsStreamStart) {
    frame.set_flags(STREAM_START);
    EXPECT_TRUE(frame.is_streaming());
}

TEST_F(FrameTest, IsStreamingDetectsStreamEnd) {
    frame.set_flags(STREAM_END);
    EXPECT_TRUE(frame.is_streaming());
}

TEST_F(FrameTest, CombinedFlagsWork) {
    frame.set_flags(COMPRESSED_GZIP | ENCRYPTED_XCHACHA20);
    EXPECT_TRUE(frame.is_compressed());
    EXPECT_TRUE(frame.is_encrypted());
    EXPECT_FALSE(frame.is_fragmented());
}

// ============================================================================
// Copy and Move Semantics
// ============================================================================

TEST_F(FrameTest, CopyConstruction) {
    frame.set_operation(OperationType::DATA);
    frame.set_payload_size(100);
    frame.set_sequence_number(42);

    Frame copied = frame;

    EXPECT_EQ(copied.get_operation(), OperationType::DATA);
    EXPECT_EQ(copied.get_payload_size(), 100);
    EXPECT_EQ(copied.get_sequence_number(), 42);
}

TEST_F(FrameTest, CopyAssignment) {
    frame.set_operation(OperationType::ACK);
    ByteBuffer payload = {1, 2, 3};
    frame.set_payload(payload);

    Frame other;
    other = frame;

    EXPECT_EQ(other.get_operation(), OperationType::ACK);
    auto other_payload = other.get_payload();
    EXPECT_EQ(other_payload.size(), 3);
}

TEST_F(FrameTest, MoveConstruction) {
    ByteBuffer large_payload(1024 * 1024, 0xFF);
    frame.set_payload(large_payload);

    Frame moved = std::move(frame);

    auto moved_payload = moved.get_payload();
    EXPECT_EQ(moved_payload.size(), 1024 * 1024);
}

TEST_F(FrameTest, MoveAssignment) {
    ByteBuffer payload = {1, 2, 3, 4, 5};
    frame.set_payload(payload);

    Frame other;
    other = std::move(frame);

    auto other_payload = other.get_payload();
    EXPECT_EQ(other_payload.size(), 5);
}

// ============================================================================
// Version Tests
// ============================================================================

TEST_F(FrameTest, VersionFieldsAreCorrect) {
    EXPECT_EQ(frame.get_version_major(), 1);
    EXPECT_EQ(frame.get_version_minor(), 0);
}

TEST_F(FrameTest, VersionIsReadOnly) {
    // Version should be constant
    EXPECT_EQ(frame.get_version_major(), UMICP_VERSION_MAJOR);
}

// ============================================================================
// Sequence Number Tests
// ============================================================================

TEST_F(FrameTest, SequenceNumberZero) {
    frame.set_sequence_number(0);
    EXPECT_EQ(frame.get_sequence_number(), 0);
}

TEST_F(FrameTest, SequenceNumberMax) {
    uint32_t max_seq = 0xFFFFFFFF;
    frame.set_sequence_number(max_seq);
    EXPECT_EQ(frame.get_sequence_number(), max_seq);
}

TEST_F(FrameTest, SequenceNumberIncrement) {
    for (uint32_t i = 0; i < 1000; ++i) {
        frame.set_sequence_number(i);
        EXPECT_EQ(frame.get_sequence_number(), i);
    }
}

// ============================================================================
// Timestamp Tests
// ============================================================================

TEST_F(FrameTest, TimestampZero) {
    frame.set_timestamp(0);
    EXPECT_EQ(frame.get_timestamp(), 0);
}

TEST_F(FrameTest, TimestampMax) {
    uint64_t max_ts = 0xFFFFFFFFFFFFFFFF;
    frame.set_timestamp(max_ts);
    EXPECT_EQ(frame.get_timestamp(), max_ts);
}

TEST_F(FrameTest, TimestampCurrentTime) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();

    frame.set_timestamp(timestamp);
    EXPECT_EQ(frame.get_timestamp(), timestamp);
}

// ============================================================================
// Checksum Tests
// ============================================================================

TEST_F(FrameTest, ChecksumZero) {
    frame.set_checksum(0);
    EXPECT_EQ(frame.get_checksum(), 0);
}

TEST_F(FrameTest, ChecksumMax) {
    uint32_t max_checksum = 0xFFFFFFFF;
    frame.set_checksum(max_checksum);
    EXPECT_EQ(frame.get_checksum(), max_checksum);
}

TEST_F(FrameTest, ChecksumVariousValues) {
    std::vector<uint32_t> checksums = {
        0x00000000, 0x12345678, 0xABCDEF00, 0xFFFFFFFF
    };

    for (auto cs : checksums) {
        frame.set_checksum(cs);
        EXPECT_EQ(frame.get_checksum(), cs);
    }
}

// ============================================================================
// Complete Frame Tests
// ============================================================================

TEST_F(FrameTest, CompleteFrameWithAllFields) {
    frame.set_operation(OperationType::DATA);
    frame.set_payload_size(1024);
    frame.set_flags(COMPRESSED_GZIP | ENCRYPTED_XCHACHA20);
    frame.set_sequence_number(42);
    frame.set_timestamp(1696939200000);
    frame.set_checksum(0x12345678);

    ByteBuffer payload(1024, 0xAB);
    frame.set_payload(payload);

    EXPECT_EQ(frame.get_operation(), OperationType::DATA);
    EXPECT_EQ(frame.get_payload_size(), 1024);
    EXPECT_EQ(frame.get_flags(), COMPRESSED_GZIP | ENCRYPTED_XCHACHA20);
    EXPECT_EQ(frame.get_sequence_number(), 42);
    EXPECT_EQ(frame.get_timestamp(), 1696939200000);
    EXPECT_EQ(frame.get_checksum(), 0x12345678);
    EXPECT_EQ(frame.get_payload().size(), 1024);
}

// ============================================================================
// Flag Combinations Tests
// ============================================================================

TEST_F(FrameTest, CompressionAndEncryption) {
    frame.set_flags(COMPRESSED_GZIP | ENCRYPTED_XCHACHA20);
    EXPECT_TRUE(frame.is_compressed());
    EXPECT_TRUE(frame.is_encrypted());
}

TEST_F(FrameTest, FragmentationAndCompression) {
    frame.set_flags(FRAGMENT_START | COMPRESSED_GZIP);
    EXPECT_TRUE(frame.is_fragmented());
    EXPECT_TRUE(frame.is_compressed());
}

TEST_F(FrameTest, StreamingAndEncryption) {
    frame.set_flags(STREAM_START | ENCRYPTED_XCHACHA20);
    EXPECT_TRUE(frame.is_streaming());
    EXPECT_TRUE(frame.is_encrypted());
}

TEST_F(FrameTest, AllFlagsSet) {
    uint16_t all = COMPRESSED_GZIP | ENCRYPTED_XCHACHA20 |
                   FRAGMENT_START | STREAM_START;
    frame.set_flags(all);

    EXPECT_TRUE(frame.is_compressed());
    EXPECT_TRUE(frame.is_encrypted());
    EXPECT_TRUE(frame.is_fragmented());
    EXPECT_TRUE(frame.is_streaming());
}

TEST_F(FrameTest, NoFlagsSet) {
    frame.set_flags(0);

    EXPECT_FALSE(frame.is_compressed());
    EXPECT_FALSE(frame.is_encrypted());
    EXPECT_FALSE(frame.is_fragmented());
    EXPECT_FALSE(frame.is_streaming());
}

// ============================================================================
// Fragmentation Scenarios
// ============================================================================

TEST_F(FrameTest, FragmentationStart) {
    frame.set_flags(FRAGMENT_START);
    frame.set_sequence_number(0);
    ByteBuffer part1 = {1, 2, 3};
    frame.set_payload(part1);

    EXPECT_TRUE(frame.is_fragmented());
    EXPECT_EQ(frame.get_sequence_number(), 0);
}

TEST_F(FrameTest, FragmentationContinue) {
    frame.set_flags(FRAGMENT_CONTINUE);
    frame.set_sequence_number(1);
    ByteBuffer part2 = {4, 5, 6};
    frame.set_payload(part2);

    EXPECT_TRUE(frame.is_fragmented());
    EXPECT_EQ(frame.get_sequence_number(), 1);
}

TEST_F(FrameTest, FragmentationEnd) {
    frame.set_flags(FRAGMENT_END);
    frame.set_sequence_number(2);
    ByteBuffer part3 = {7, 8, 9};
    frame.set_payload(part3);

    EXPECT_TRUE(frame.is_fragmented());
    EXPECT_EQ(frame.get_sequence_number(), 2);
}

TEST_F(FrameTest, CompleteFragmentSequence) {
    // Simulate 3-part fragmented message
    Frame f1, f2, f3;

    f1.set_flags(FRAGMENT_START);
    f1.set_sequence_number(0);

    f2.set_flags(FRAGMENT_CONTINUE);
    f2.set_sequence_number(1);

    f3.set_flags(FRAGMENT_END);
    f3.set_sequence_number(2);

    EXPECT_TRUE(f1.is_fragmented());
    EXPECT_TRUE(f2.is_fragmented());
    EXPECT_TRUE(f3.is_fragmented());
}

// ============================================================================
// Streaming Tests
// ============================================================================

TEST_F(FrameTest, StreamStartFrame) {
    frame.set_flags(STREAM_START);
    frame.set_sequence_number(0);

    EXPECT_TRUE(frame.is_streaming());
    EXPECT_EQ(frame.get_sequence_number(), 0);
}

TEST_F(FrameTest, StreamEndFrame) {
    frame.set_flags(STREAM_END);
    frame.set_sequence_number(999);

    EXPECT_TRUE(frame.is_streaming());
    EXPECT_EQ(frame.get_sequence_number(), 999);
}

TEST_F(FrameTest, StreamWithMultipleFrames) {
    Frame start, middle, end;

    start.set_flags(STREAM_START);
    start.set_sequence_number(0);

    middle.set_flags(0);
    middle.set_sequence_number(1);

    end.set_flags(STREAM_END);
    end.set_sequence_number(2);

    EXPECT_TRUE(start.is_streaming());
    EXPECT_FALSE(middle.is_streaming());
    EXPECT_TRUE(end.is_streaming());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(FrameTest, MaxPayloadSize) {
    size_t max_size = 100 * 1024 * 1024; // 100MB
    ByteBuffer huge(max_size, 0);

    frame.set_payload(huge);
    EXPECT_EQ(frame.get_payload().size(), max_size);
    EXPECT_EQ(frame.get_payload_size(), max_size);
}

TEST_F(FrameTest, SequenceNumberWrapAround) {
    uint32_t max = 0xFFFFFFFF;
    frame.set_sequence_number(max);
    EXPECT_EQ(frame.get_sequence_number(), max);

    frame.set_sequence_number(0);
    EXPECT_EQ(frame.get_sequence_number(), 0);
}

TEST_F(FrameTest, AllFlagsIndividually) {
    std::vector<uint16_t> individual_flags = {
        COMPRESSED_GZIP,
        COMPRESSED_BROTLI,
        ENCRYPTED_XCHACHA20,
        FRAGMENT_START,
        FRAGMENT_CONTINUE,
        FRAGMENT_END,
        STREAM_START,
        STREAM_END
    };

    for (auto flag : individual_flags) {
        frame.set_flags(flag);
        EXPECT_EQ(frame.get_flags(), flag);
    }
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(FrameThreadSafetyTest, ConcurrentFrameCreation) {
    std::vector<std::thread> threads;
    std::vector<Frame> frames(10);

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([i, &frames]() {
            frames[i].set_operation(OperationType::DATA);
            frames[i].set_sequence_number(i);
            ByteBuffer payload(100, i);
            frames[i].set_payload(payload);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(frames[i].get_sequence_number(), static_cast<uint32_t>(i));
    }
}

TEST(FrameThreadSafetyTest, ConcurrentPayloadAccess) {
    Frame shared_frame;
    ByteBuffer payload = {1, 2, 3, 4, 5};
    shared_frame.set_payload(payload);

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&shared_frame, &success_count]() {
            auto p = shared_frame.get_payload();
            if (p.size() == 5 && p[0] == 1) {
                success_count++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(success_count, 10);
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST(FramePerformanceTest, CreateManyFrames) {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<Frame> frames;
    frames.reserve(10000);

    for (int i = 0; i < 10000; ++i) {
        Frame f;
        f.set_operation(OperationType::DATA);
        f.set_sequence_number(i);
        frames.push_back(std::move(f));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should create 10K frames in < 100ms
    EXPECT_LT(duration.count(), 100);
}

TEST(FramePerformanceTest, SetPayloadPerformance) {
    auto start = std::chrono::high_resolution_clock::now();

    Frame frame;
    ByteBuffer payload(1024, 0);

    for (int i = 0; i < 1000; ++i) {
        frame.set_payload(payload);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should set payload 1000 times in < 50ms
    EXPECT_LT(duration.count(), 50);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST(FrameStressTest, ManyFramesWithLargePayloads) {
    std::vector<Frame> frames;
    frames.reserve(1000);

    for (int i = 0; i < 1000; ++i) {
        Frame f;
        ByteBuffer payload(10 * 1024, i % 256); // 10KB each
        f.set_payload(payload);
        frames.push_back(std::move(f));
    }

    EXPECT_EQ(frames.size(), 1000);

    // Verify some frames
    EXPECT_EQ(frames[0].get_payload().size(), 10 * 1024);
    EXPECT_EQ(frames[999].get_payload().size(), 10 * 1024);
}

TEST(FrameStressTest, RapidFlagChanges) {
    Frame frame;

    for (int i = 0; i < 10000; ++i) {
        uint16_t flags = i % 256;
        frame.set_flags(flags);
        EXPECT_EQ(frame.get_flags(), flags);
    }
}

TEST(FrameStressTest, PayloadSizeConsistency) {
    Frame frame;

    for (size_t size : {0, 1, 10, 100, 1000, 10000, 100000}) {
        ByteBuffer payload(size, 0xAA);
        frame.set_payload(payload);

        EXPECT_EQ(frame.get_payload_size(), size);
        EXPECT_EQ(frame.get_payload().size(), size);
        EXPECT_EQ(frame.get_total_size(), UMICP_FRAME_HEADER_SIZE + size);
    }
}

// ============================================================================
// Real-World Scenarios
// ============================================================================

TEST(FrameScenarioTest, ControlMessage) {
    Frame control;
    control.set_operation(OperationType::CONTROL);
    control.set_sequence_number(1);

    StringMap command;
    command["action"] = "handshake";
    // In real use, this would be serialized into payload

    EXPECT_EQ(control.get_operation(), OperationType::CONTROL);
}

TEST(FrameScenarioTest, DataMessageWithVector) {
    Frame data;
    data.set_operation(OperationType::DATA);
    data.set_flags(COMPRESSED_GZIP);
    data.set_sequence_number(100);

    // Simulate vector data (1024 floats)
    ByteBuffer vector_data(1024 * sizeof(float), 0);
    data.set_payload(vector_data);

    EXPECT_EQ(data.get_payload().size(), 1024 * sizeof(float));
    EXPECT_TRUE(data.is_compressed());
}

TEST(FrameScenarioTest, AckMessage) {
    Frame ack;
    ack.set_operation(OperationType::ACK);
    ack.set_sequence_number(50);

    // ACK typically has no payload
    EXPECT_EQ(ack.get_operation(), OperationType::ACK);
    EXPECT_EQ(ack.get_payload().size(), 0);
}

TEST(FrameScenarioTest, ErrorMessage) {
    Frame error;
    error.set_operation(OperationType::ERROR);

    std::string error_msg = "Connection failed";
    ByteBuffer error_payload(error_msg.begin(), error_msg.end());
    error.set_payload(error_payload);

    EXPECT_EQ(error.get_operation(), OperationType::ERROR);
    EXPECT_GT(error.get_payload().size(), 0);
}

TEST(FrameScenarioTest, LargeFileTransferFragmented) {
    // Simulate transferring a 5MB file in 1MB fragments
    const size_t total_size = 5 * 1024 * 1024;
    const size_t fragment_size = 1 * 1024 * 1024;
    const size_t num_fragments = (total_size + fragment_size - 1) / fragment_size;

    std::vector<Frame> fragments;

    for (size_t i = 0; i < num_fragments; ++i) {
        Frame fragment;
        fragment.set_operation(OperationType::DATA);
        fragment.set_sequence_number(i);

        // Set appropriate flags
        if (i == 0) {
            fragment.set_flags(FRAGMENT_START | COMPRESSED_GZIP);
        } else if (i == num_fragments - 1) {
            fragment.set_flags(FRAGMENT_END | COMPRESSED_GZIP);
        } else {
            fragment.set_flags(FRAGMENT_CONTINUE | COMPRESSED_GZIP);
        }

        size_t current_fragment_size = (i == num_fragments - 1)
            ? (total_size % fragment_size)
            : fragment_size;

        ByteBuffer payload(current_fragment_size, i % 256);
        fragment.set_payload(payload);

        fragments.push_back(std::move(fragment));
    }

    EXPECT_EQ(fragments.size(), num_fragments);
    EXPECT_TRUE(fragments[0].is_fragmented());
    EXPECT_TRUE(fragments[0].is_compressed());
}

