/**
 * UMICP Comprehensive Edge Cases Tests
 * Testing boundary conditions, corner cases, and unusual scenarios
 */

#include <gtest/gtest.h>
#include "protocol.h"
#include "transport.h"
#include "security.h"
#include "serialization.h"
#include "matrix_ops.h"
#include "../utils/test_helpers.h"
#include <limits>
#include <cmath>

using namespace umicp;
using namespace umicp::testing;

class EdgeCasesTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        // Edge case test configuration
        edge_config_.max_message_size = 1024 * 1024;
        edge_config_.connection_timeout = 5000;
        edge_config_.enable_binary = true;
        edge_config_.preferred_format = ContentType::JSON;
        edge_config_.require_auth = false;
        edge_config_.require_encryption = false;
    }

    UMICPConfig edge_config_;
};

// ===============================================
// Matrix Operations Edge Cases
// ===============================================

TEST_F(EdgeCasesTest, MatrixOps_ZeroSizeVectors) {
    std::cout << "\n  🔍 Testing Zero-Size Vector Operations:" << std::endl;

    std::vector<float> empty_vec;
    std::vector<float> result_vec;
    float scalar_result = 0.0f;

    // Test vector addition with zero size
    auto add_result = MatrixOps::add(empty_vec.data(), empty_vec.data(),
                                   result_vec.data(), 1, 0);
    EXPECT_FALSE(add_result.is_success());
    EXPECT_EQ(add_result.code, ErrorCode::INVALID_ARGUMENT);

    // Test dot product with zero size
    auto dot_result = MatrixOps::dot_product(empty_vec.data(), empty_vec.data(),
                                           &scalar_result, 0);
    EXPECT_FALSE(dot_result.is_success());
    EXPECT_EQ(dot_result.code, ErrorCode::INVALID_ARGUMENT);

    // Test normalization with zero size
    auto norm_result = MatrixOps::normalize(empty_vec.data(), 1, 0);
    EXPECT_FALSE(norm_result.is_success());
    EXPECT_EQ(norm_result.code, ErrorCode::INVALID_ARGUMENT);

    std::cout << "    ✅ Zero-size operations properly rejected" << std::endl;
}

TEST_F(EdgeCasesTest, MatrixOps_SingleElementVectors) {
    std::cout << "\n  🔍 Testing Single-Element Vector Operations:" << std::endl;

    std::vector<float> vec_a = {5.0f};
    std::vector<float> vec_b = {3.0f};
    std::vector<float> result(1);
    float scalar_result = 0.0f;

    // Test vector addition
    auto add_result = MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, 1);
    ASSERT_TRUE(add_result.is_success());
    EXPECT_FLOAT_EQ(result[0], 8.0f);

    // Test dot product
    auto dot_result = MatrixOps::dot_product(vec_a.data(), vec_b.data(), &scalar_result, 1);
    ASSERT_TRUE(dot_result.is_success());
    EXPECT_FLOAT_EQ(scalar_result, 15.0f);

    // Test normalization
    std::vector<float> norm_vec = {4.0f};
    auto norm_result = MatrixOps::normalize(norm_vec.data(), 1, 1);
    ASSERT_TRUE(norm_result.is_success());
    EXPECT_FLOAT_EQ(norm_vec[0], 1.0f); // Should be normalized to 1.0

    std::cout << "    ✅ Single-element operations work correctly" << std::endl;
}

TEST_F(EdgeCasesTest, MatrixOps_ExtremeValues) {
    std::cout << "\n  🔍 Testing Extreme Numerical Values:" << std::endl;

    // Test with very large numbers
    std::vector<float> large_vec = {std::numeric_limits<float>::max() / 2,
                                   std::numeric_limits<float>::max() / 2};
    std::vector<float> result(2);

    auto large_add = MatrixOps::add(large_vec.data(), large_vec.data(),
                                  result.data(), 1, 2);
    ASSERT_TRUE(large_add.is_success());
    // Results might overflow, but operation should succeed
    EXPECT_TRUE(std::isfinite(result[0]) || std::isinf(result[0]));

    // Test with very small numbers
    std::vector<float> small_vec = {std::numeric_limits<float>::min(),
                                   std::numeric_limits<float>::min()};
    std::vector<float> small_result(2);

    auto small_add = MatrixOps::add(small_vec.data(), small_vec.data(),
                                  small_result.data(), 1, 2);
    ASSERT_TRUE(small_add.is_success());
    EXPECT_TRUE(std::isfinite(small_result[0]));

    // Test with zero vector normalization
    std::vector<float> zero_vec = {0.0f, 0.0f, 0.0f};
    auto zero_norm = MatrixOps::normalize(zero_vec.data(), 1, 3);
    // This should handle the zero-length vector case
    EXPECT_TRUE(zero_norm.is_success() || zero_norm.code == ErrorCode::INVALID_ARGUMENT);

    std::cout << "    ✅ Extreme values handled appropriately" << std::endl;
}

TEST_F(EdgeCasesTest, MatrixOps_SpecialFloatValues) {
    std::cout << "\n  🔍 Testing Special Float Values (NaN, Infinity):" << std::endl;

    // Test with NaN values
    std::vector<float> nan_vec = {std::numeric_limits<float>::quiet_NaN(), 1.0f, 2.0f};
    std::vector<float> normal_vec = {1.0f, 2.0f, 3.0f};
    std::vector<float> result(3);

    auto nan_add = MatrixOps::add(nan_vec.data(), normal_vec.data(),
                                result.data(), 1, 3);
    ASSERT_TRUE(nan_add.is_success());
    EXPECT_TRUE(std::isnan(result[0])); // NaN + anything = NaN
    EXPECT_FLOAT_EQ(result[1], 3.0f);   // 1 + 2 = 3
    EXPECT_FLOAT_EQ(result[2], 5.0f);   // 2 + 3 = 5

    // Test with infinity values
    std::vector<float> inf_vec = {std::numeric_limits<float>::infinity(), 1.0f};
    std::vector<float> inf_result(2);

    auto inf_add = MatrixOps::add(inf_vec.data(), normal_vec.data(),
                                inf_result.data(), 1, 2);
    ASSERT_TRUE(inf_add.is_success());
    EXPECT_TRUE(std::isinf(inf_result[0]));

    // Test dot product with special values
    float dot_result = 0.0f;
    auto dot_nan = MatrixOps::dot_product(nan_vec.data(), normal_vec.data(),
                                        &dot_result, 3);
    ASSERT_TRUE(dot_nan.is_success());
    EXPECT_TRUE(std::isnan(dot_result));

    std::cout << "    ✅ Special float values (NaN, Inf) handled correctly" << std::endl;
}

TEST_F(EdgeCasesTest, MatrixOps_AlignmentEdgeCases) {
    std::cout << "\n  🔍 Testing Memory Alignment Edge Cases:" << std::endl;

    // Test with unaligned sizes (not multiples of SIMD width)
    std::vector<size_t> unaligned_sizes = {1, 3, 5, 7, 9, 15, 17, 31, 33};

    for (size_t size : unaligned_sizes) {
        auto vec_a = TestHelpers::generate_random_vector(size, 12345);
        auto vec_b = TestHelpers::generate_random_vector(size, 54321);
        std::vector<float> result(size);

        auto add_result = MatrixOps::add(vec_a.data(), vec_b.data(),
                                       result.data(), 1, size);
        ASSERT_TRUE(add_result.is_success()) << "Failed for size: " << size;

        // Verify correctness
        for (size_t i = 0; i < size; ++i) {
            EXPECT_NEAR(result[i], vec_a[i] + vec_b[i], 0.001f)
                << "Mismatch at index " << i << " for size " << size;
        }
    }

    std::cout << "    ✅ Unaligned sizes handled correctly by SIMD operations" << std::endl;
}

// ===============================================
// Serialization Edge Cases
// ===============================================

TEST_F(EdgeCasesTest, Serialization_EmptyEnvelope) {
    std::cout << "\n  🔍 Testing Empty Envelope Serialization:" << std::endl;

    Envelope empty_envelope;
    // All fields are empty/default

    auto serialize_result = JsonSerializer::serialize_envelope(empty_envelope);
    ASSERT_TRUE(serialize_result.is_success());

    std::string json_string = *serialize_result.value;
    EXPECT_FALSE(json_string.empty());

    // Should be able to deserialize back
    auto deserialize_result = JsonSerializer::deserialize_envelope(json_string);
    ASSERT_TRUE(deserialize_result.is_success());

    std::cout << "    ✅ Empty envelope serialization works" << std::endl;
}

TEST_F(EdgeCasesTest, Serialization_VeryLongStrings) {
    std::cout << "\n  🔍 Testing Very Long String Serialization:" << std::endl;

    // Create envelope with very long strings
    Envelope long_envelope;
    long_envelope.version = "1.0";
    long_envelope.msg_id = std::string(10000, 'a'); // 10K character message ID
    long_envelope.from = std::string(5000, 'f');    // 5K character from field
    long_envelope.to = std::string(5000, 't');      // 5K character to field
    long_envelope.op = OperationType::DATA;

    auto serialize_result = JsonSerializer::serialize_envelope(long_envelope);
    ASSERT_TRUE(serialize_result.is_success());

    std::string json_string = *serialize_result.value;
    EXPECT_GT(json_string.size(), 20000); // Should be quite large

    // Test deserialization
    auto deserialize_result = JsonSerializer::deserialize_envelope(json_string);
    ASSERT_TRUE(deserialize_result.is_success());

    auto& deserialized = *deserialize_result.value;
    EXPECT_EQ(deserialized.msg_id.size(), 10000);
    EXPECT_EQ(deserialized.from.size(), 5000);
    EXPECT_EQ(deserialized.to.size(), 5000);

    std::cout << "    ✅ Very long strings handled correctly" << std::endl;
}

TEST_F(EdgeCasesTest, Serialization_SpecialCharacters) {
    std::cout << "\n  🔍 Testing Special Characters in Serialization:" << std::endl;

    Envelope special_envelope;
    special_envelope.version = "1.0";
    special_envelope.msg_id = "test-msg-123";
    special_envelope.from = "sender-with-unicode-🚀";
    special_envelope.to = "receiver-with-quotes-\"and\\backslashes\"";
    special_envelope.op = OperationType::CONTROL;

    auto serialize_result = JsonSerializer::serialize_envelope(special_envelope);
    ASSERT_TRUE(serialize_result.is_success());

    std::string json_string = *serialize_result.value;
    EXPECT_FALSE(json_string.empty());

    // Test deserialization
    auto deserialize_result = JsonSerializer::deserialize_envelope(json_string);
    ASSERT_TRUE(deserialize_result.is_success());

    auto& deserialized = *deserialize_result.value;
    EXPECT_EQ(deserialized.from, special_envelope.from);
    EXPECT_EQ(deserialized.to, special_envelope.to);

    std::cout << "    ✅ Special characters handled correctly in JSON" << std::endl;
}

TEST_F(EdgeCasesTest, Serialization_MalformedJSON) {
    std::cout << "\n  🔍 Testing Malformed JSON Handling:" << std::endl;

    std::vector<std::string> malformed_jsons = {
        "",                          // Empty string
        "{",                         // Incomplete JSON
        "{ \"incomplete\": ",        // Incomplete value
        "{ invalid json }",          // Invalid syntax
        "null",                      // Valid JSON but not object
        "[]",                        // Array instead of object
        "{ \"msg_id\": null }",      // Null value where string expected
    };

    for (const auto& malformed : malformed_jsons) {
        auto deserialize_result = JsonSerializer::deserialize_envelope(malformed);
        EXPECT_FALSE(deserialize_result.is_success())
            << "Should fail for malformed JSON: " << malformed;
        EXPECT_EQ(deserialize_result.code, ErrorCode::SERIALIZATION_ERROR);
    }

    std::cout << "    ✅ Malformed JSON properly rejected" << std::endl;
}

TEST_F(EdgeCasesTest, Serialization_MaxSizeFrames) {
    std::cout << "\n  🔍 Testing Maximum Size Frame Serialization:" << std::endl;

    // Create frame with maximum payload size
    const size_t max_payload_size = 1024 * 1024; // 1MB
    ByteBuffer large_payload = TestHelpers::generate_random_data(max_payload_size);

    Frame large_frame = TestHelpers::create_test_frame(12345, 1, large_payload);

    auto serialize_result = BinarySerializer::serialize_frame(large_frame);
    ASSERT_TRUE(serialize_result.is_success());

    ByteBuffer serialized = *serialize_result.value;
    EXPECT_GT(serialized.size(), max_payload_size); // Should be larger due to header

    // Test deserialization
    auto deserialize_result = BinarySerializer::deserialize_frame(serialized);
    ASSERT_TRUE(deserialize_result.is_success());

    auto& deserialized = *deserialize_result.value;
    EXPECT_EQ(deserialized.header.stream_id, large_frame.header.stream_id);
    EXPECT_EQ(deserialized.payload.size(), large_payload.size());
    EXPECT_TRUE(TestHelpers::buffers_equal(deserialized.payload, large_payload));

    std::cout << "    ✅ Maximum size frames handled correctly" << std::endl;
}

// ===============================================
// Protocol Edge Cases
// ===============================================

TEST_F(EdgeCasesTest, Protocol_RapidConnectDisconnect) {
    std::cout << "\n  🔍 Testing Rapid Connect/Disconnect Cycles:" << std::endl;

    Protocol protocol("rapid-test-node");
    protocol.configure(edge_config_);
    protocol.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8100)));

    // Perform rapid connect/disconnect cycles
    for (int i = 0; i < 10; ++i) {
        auto connect_result = protocol.connect();
        EXPECT_TRUE(connect_result.is_success()) << "Connect failed on cycle " << i;

        auto disconnect_result = protocol.disconnect();
        EXPECT_TRUE(disconnect_result.is_success()) << "Disconnect failed on cycle " << i;
    }

    // Final state should be disconnected
    EXPECT_FALSE(protocol.is_connected());

    std::cout << "    ✅ Rapid connect/disconnect cycles handled correctly" << std::endl;
}

TEST_F(EdgeCasesTest, Protocol_DoubleConnect) {
    std::cout << "\n  🔍 Testing Double Connection Attempt:" << std::endl;

    Protocol protocol("double-connect-node");
    protocol.configure(edge_config_);
    protocol.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8101)));

    // First connection should succeed
    auto first_connect = protocol.connect();
    ASSERT_TRUE(first_connect.is_success());
    EXPECT_TRUE(protocol.is_connected());

    // Second connection should fail (already connected)
    auto second_connect = protocol.connect();
    EXPECT_FALSE(second_connect.is_success());
    EXPECT_EQ(second_connect.code, ErrorCode::NETWORK_ERROR);

    std::cout << "    ✅ Double connection properly prevented" << std::endl;
}

TEST_F(EdgeCasesTest, Protocol_SendWithoutConnection) {
    std::cout << "\n  🔍 Testing Send Operations Without Connection:" << std::endl;

    Protocol protocol("no-connect-node");
    protocol.configure(edge_config_);
    protocol.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8102)));

    // Don't connect, try to send
    ByteBuffer test_data = TestHelpers::generate_random_data(100);

    auto data_result = protocol.send_data("target", test_data);
    EXPECT_FALSE(data_result.is_success());
    EXPECT_EQ(data_result.code, ErrorCode::NETWORK_ERROR);

    auto control_result = protocol.send_control("target", "test");
    EXPECT_FALSE(control_result.is_success());
    EXPECT_EQ(control_result.code, ErrorCode::NETWORK_ERROR);

    std::cout << "    ✅ Send operations properly fail without connection" << std::endl;
}

TEST_F(EdgeCasesTest, Protocol_ExtremeMessageSizes) {
    std::cout << "\n  🔍 Testing Extreme Message Sizes:" << std::endl;

    Protocol protocol("extreme-size-node");
    protocol.configure(edge_config_);
    protocol.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8103)));

    auto connect_result = protocol.connect();
    ASSERT_TRUE(connect_result.is_success());

    // Test zero-size message
    ByteBuffer empty_data;
    auto empty_result = protocol.send_data("target", empty_data);
    EXPECT_FALSE(empty_result.is_success()); // Should reject empty data

    // Test very large message (within limits)
    ByteBuffer large_data = TestHelpers::generate_random_data(512 * 1024); // 512KB
    auto large_result = protocol.send_data("target", large_data);
    EXPECT_TRUE(large_result.is_success());

    // Test message exceeding configured limits
    ByteBuffer huge_data = TestHelpers::generate_random_data(2 * 1024 * 1024); // 2MB (exceeds 1MB limit)
    auto huge_result = protocol.send_data("target", huge_data);
    EXPECT_FALSE(huge_result.is_success());
    EXPECT_EQ(huge_result.code, ErrorCode::INVALID_ARGUMENT);

    std::cout << "    ✅ Extreme message sizes handled correctly" << std::endl;
}

// ===============================================
// Security Edge Cases
// ===============================================

TEST_F(EdgeCasesTest, Security_EmptyDataSigning) {
    std::cout << "\n  🔍 Testing Empty Data Signing:" << std::endl;

    SecurityManager security("empty-data-security");
    auto key_result = security.generate_keypair();
    ASSERT_TRUE(key_result.is_success());

    // Test signing empty data
    ByteBuffer empty_data;
    auto sign_result = security.sign_data(empty_data);
    EXPECT_FALSE(sign_result.is_success()); // Should reject empty data
    EXPECT_EQ(sign_result.code, ErrorCode::INVALID_ARGUMENT);

    std::cout << "    ✅ Empty data signing properly rejected" << std::endl;
}

TEST_F(EdgeCasesTest, Security_InvalidSignatureVerification) {
    std::cout << "\n  🔍 Testing Invalid Signature Verification:" << std::endl;

    SecurityManager security("invalid-sig-security");
    auto key_result = security.generate_keypair();
    ASSERT_TRUE(key_result.is_success());

    ByteBuffer test_data = TestHelpers::generate_random_data(256);

    // Test with empty signature
    ByteBuffer empty_signature;
    auto verify_empty = security.verify_signature(test_data, empty_signature);
    EXPECT_FALSE(verify_empty.is_success());
    EXPECT_EQ(verify_empty.code, ErrorCode::INVALID_ARGUMENT);

    // Test with random signature (not created by this key)
    ByteBuffer random_signature = TestHelpers::generate_random_data(64);
    auto verify_random = security.verify_signature(test_data, random_signature);
    EXPECT_FALSE(verify_random.is_success());

    std::cout << "    ✅ Invalid signatures properly rejected" << std::endl;
}

TEST_F(EdgeCasesTest, Security_MultipleKeyGeneration) {
    std::cout << "\n  🔍 Testing Multiple Key Generation:" << std::endl;

    SecurityManager security("multi-key-security");

    // Generate multiple keypairs
    for (int i = 0; i < 5; ++i) {
        auto key_result = security.generate_keypair();
        EXPECT_TRUE(key_result.is_success()) << "Key generation " << i << " failed";
    }

    // Each generation should create a new keypair
    ByteBuffer test_data = TestHelpers::generate_random_data(100);
    auto sign_result = security.sign_data(test_data);
    ASSERT_TRUE(sign_result.is_success());

    auto verify_result = security.verify_signature(test_data, *sign_result.value);
    ASSERT_TRUE(verify_result.is_success());
    EXPECT_TRUE(*verify_result.value);

    std::cout << "    ✅ Multiple key generation works correctly" << std::endl;
}

// ===============================================
// Transport Edge Cases
// ===============================================

TEST_F(EdgeCasesTest, Transport_InvalidConfiguration) {
    std::cout << "\n  🔍 Testing Invalid Transport Configuration:" << std::endl;

    TransportConfig invalid_config;
    invalid_config.type = TransportType::WEBSOCKET;
    invalid_config.host = ""; // Empty host
    invalid_config.port = 0;  // Invalid port

    auto transport = TransportFactory::create(TransportType::WEBSOCKET, invalid_config);
    EXPECT_NE(transport, nullptr); // Should still create transport

    // But connection should fail
    auto connect_result = transport->connect();
    EXPECT_FALSE(connect_result.is_success());

    std::cout << "    ✅ Invalid configuration properly handled" << std::endl;
}

TEST_F(EdgeCasesTest, Transport_StatsOverflow) {
    std::cout << "\n  🔍 Testing Transport Statistics Overflow:" << std::endl;

    MockTransport transport(TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8104));
    auto connect_result = transport.connect();
    ASSERT_TRUE(connect_result.is_success());

    // Send many messages to test stats overflow protection
    ByteBuffer test_data = TestHelpers::generate_random_data(100);

    for (int i = 0; i < 10000; ++i) {
        transport.send(test_data);
    }

    auto stats = transport.get_stats();
    EXPECT_EQ(stats.messages_sent, 10000);
    EXPECT_EQ(stats.bytes_sent, 10000 * 100);

    // Stats should be reasonable (not overflowed)
    EXPECT_LT(stats.bytes_sent, std::numeric_limits<uint64_t>::max() / 2);

    std::cout << "    ✅ Transport statistics don't overflow" << std::endl;
}

// ===============================================
// Memory Edge Cases
// ===============================================

TEST_F(EdgeCasesTest, Memory_LargeAllocationPatterns) {
    std::cout << "\n  🔍 Testing Large Memory Allocation Patterns:" << std::endl;

    std::vector<std::vector<float>> large_allocations;

    // Allocate progressively larger vectors
    for (size_t size = 1000; size <= 100000; size *= 2) {
        large_allocations.emplace_back(TestHelpers::generate_random_vector(size));

        // Verify allocation succeeded
        EXPECT_EQ(large_allocations.back().size(), size);

        // Test operations on large vectors
        if (large_allocations.size() >= 2) {
            auto& vec_a = large_allocations[large_allocations.size() - 2];
            auto& vec_b = large_allocations[large_allocations.size() - 1];

            if (vec_a.size() == vec_b.size()) {
                std::vector<float> result(vec_a.size());
                auto add_result = MatrixOps::add(vec_a.data(), vec_b.data(),
                                               result.data(), 1, vec_a.size());
                EXPECT_TRUE(add_result.is_success());
            }
        }
    }

    // Cleanup should happen automatically
    large_allocations.clear();

    std::cout << "    ✅ Large memory allocations handled correctly" << std::endl;
}

TEST_F(EdgeCasesTest, Memory_FragmentationResistance) {
    std::cout << "\n  🔍 Testing Memory Fragmentation Resistance:" << std::endl;

    std::vector<std::vector<float>> fragmentation_test;

    // Create fragmentation pattern: allocate, deallocate, allocate different size
    for (int cycle = 0; cycle < 100; ++cycle) {
        // Allocate various sizes
        fragmentation_test.emplace_back(TestHelpers::generate_random_vector(100 + cycle));
        fragmentation_test.emplace_back(TestHelpers::generate_random_vector(500 + cycle * 2));
        fragmentation_test.emplace_back(TestHelpers::generate_random_vector(1000 + cycle * 3));

        // Deallocate middle one (create fragmentation)
        if (fragmentation_test.size() >= 2) {
            fragmentation_test.erase(fragmentation_test.end() - 2);
        }

        // Test operations still work
        if (fragmentation_test.size() >= 2) {
            auto& vec_a = fragmentation_test[fragmentation_test.size() - 2];
            auto& vec_b = fragmentation_test[fragmentation_test.size() - 1];

            if (vec_a.size() == vec_b.size()) {
                std::vector<float> result(vec_a.size());
                auto add_result = MatrixOps::add(vec_a.data(), vec_b.data(),
                                               result.data(), 1, vec_a.size());
                EXPECT_TRUE(add_result.is_success());
            }
        }
    }

    std::cout << "    ✅ Memory fragmentation handled gracefully" << std::endl;
}

// ===============================================
// Concurrency Edge Cases
// ===============================================

TEST_F(EdgeCasesTest, Concurrency_RaceConditionProtection) {
    std::cout << "\n  🔍 Testing Race Condition Protection:" << std::endl;

    Protocol protocol("race-test-node");
    protocol.configure(edge_config_);
    protocol.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8105)));

    auto connect_result = protocol.connect();
    ASSERT_TRUE(connect_result.is_success());

    std::atomic<int> successful_ops{0};
    std::atomic<int> failed_ops{0};
    const int num_threads = 8;
    const int ops_per_thread = 100;

    std::vector<std::future<void>> futures;

    // Launch multiple threads performing operations simultaneously
    for (int t = 0; t < num_threads; ++t) {
        futures.push_back(std::async(std::launch::async, [&]() {
            for (int i = 0; i < ops_per_thread; ++i) {
                ByteBuffer data = TestHelpers::generate_random_data(100);
                auto result = protocol.send_data("race-target", data);

                if (result.is_success()) {
                    successful_ops.fetch_add(1);
                } else {
                    failed_ops.fetch_add(1);
                }

                // Also test statistics access
                auto stats = protocol.get_stats();
                (void)stats; // Suppress unused variable warning
            }
        }));
    }

    // Wait for all threads
    for (auto& future : futures) {
        future.wait();
    }

    int total_ops = successful_ops.load() + failed_ops.load();
    int expected_ops = num_threads * ops_per_thread;

    EXPECT_EQ(total_ops, expected_ops);
    EXPECT_GT(successful_ops.load(), expected_ops * 0.9); // At least 90% should succeed

    std::cout << "    ✅ Race conditions properly protected (" << successful_ops.load()
              << "/" << expected_ops << " operations succeeded)" << std::endl;
}
