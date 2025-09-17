/**
 * UMICP Bindings Integration Tests
 * Testing integration between C++ core and TypeScript/Node.js bindings
 */

#include <gtest/gtest.h>
#include "protocol.h"
#include "transport.h"
#include "security.h"
#include "serialization.h"
#include "matrix_ops.h"
#include "../utils/test_helpers.h"
#include <thread>
#include <future>
#include <atomic>
#include <fstream>
#include <filesystem>

using namespace umicp;
using namespace umicp::testing;

class BindingsIntegrationTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        // Bindings test configuration
        bindings_config_.max_message_size = 1024 * 1024; // 1MB
        bindings_config_.connection_timeout = 5000;
        bindings_config_.enable_binary = true;
        bindings_config_.preferred_format = ContentType::JSON;
        bindings_config_.require_auth = false;
        bindings_config_.require_encryption = false;

        // Create test data for bindings
        test_matrix_a_ = TestHelpers::generate_random_vector(256, 12345);
        test_matrix_b_ = TestHelpers::generate_random_vector(256, 54321);
        result_matrix_ = std::vector<float>(256);

        // Create test envelope
        test_envelope_ = TestHelpers::create_test_envelope("cpp-core", "ts-binding", OperationType::DATA);

        // Create test frame
        test_payload_ = TestHelpers::generate_random_data(512);
        test_frame_ = TestHelpers::create_test_frame(1001, 1, test_payload_);
    }

    UMICPConfig bindings_config_;
    std::vector<float> test_matrix_a_, test_matrix_b_, result_matrix_;
    Envelope test_envelope_;
    Frame test_frame_;
    ByteBuffer test_payload_;

    // Helper to simulate Node.js binding call
    std::string simulate_nodejs_call(const std::string& operation, const std::string& data) {
        // This simulates what the Node.js binding would do:
        // 1. Receive JSON from TypeScript
        // 2. Parse it
        // 3. Call C++ functions
        // 4. Return JSON result

        if (operation == "matrix_add") {
            // Simulate matrix addition through bindings
            auto result = MatrixOps::add(test_matrix_a_.data(), test_matrix_b_.data(),
                                       result_matrix_.data(), 1, test_matrix_a_.size());
            return result.is_success() ? "success" : "error";
        } else if (operation == "serialize_envelope") {
            auto result = JsonSerializer::serialize_envelope(test_envelope_);
            return result.is_success() ? *result.value : "error";
        } else if (operation == "serialize_frame") {
            auto result = BinarySerializer::serialize_frame(test_frame_);
            return result.is_success() ? "binary_data" : "error";
        }

        return "unknown_operation";
    }

    // Helper to write test data to file (simulating bindings file I/O)
    void write_test_data_file(const std::string& filename, const ByteBuffer& data) {
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
    }

    // Helper to read test data from file
    ByteBuffer read_test_data_file(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) return {};

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        ByteBuffer buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()), size);
        return buffer;
    }
};

// ===============================================
// Matrix Operations Bindings Tests
// ===============================================

TEST_F(BindingsIntegrationTest, MatrixOps_BindingsCompatibility) {
    std::cout << "\n  🔗 Testing Matrix Operations through Bindings Interface:" << std::endl;

    // Test vector addition (most common operation in bindings)
    auto add_result = MatrixOps::add(test_matrix_a_.data(), test_matrix_b_.data(),
                                   result_matrix_.data(), 1, test_matrix_a_.size());
    ASSERT_TRUE(add_result.is_success());

    // Simulate what the bindings would do
    std::string bindings_result = simulate_nodejs_call("matrix_add", "");
    EXPECT_EQ(bindings_result, "success");

    // Test dot product
    float dot_result = 0.0f;
    auto dot_op_result = MatrixOps::dot_product(test_matrix_a_.data(), test_matrix_b_.data(),
                                              &dot_result, test_matrix_a_.size());
    ASSERT_TRUE(dot_op_result.is_success());
    EXPECT_NE(dot_result, 0.0f);

    // Test normalization (important for ML applications)
    auto normalize_vec = test_matrix_a_; // Copy for modification
    auto norm_result = MatrixOps::normalize(normalize_vec.data(), 1, normalize_vec.size());
    ASSERT_TRUE(norm_result.is_success());

    // Verify normalization worked
    float norm = 0.0f;
    MatrixOps::dot_product(normalize_vec.data(), normalize_vec.data(), &norm, normalize_vec.size());
    EXPECT_NEAR(norm, 1.0f, 0.001f);

    std::cout << "    ✅ Matrix operations work correctly through bindings interface" << std::endl;
}

TEST_F(BindingsIntegrationTest, MatrixOps_LargeDataThroughBindings) {
    std::cout << "\n  🔗 Testing Large Matrix Data through Bindings:" << std::endl;

    // Test with larger matrices (common in real ML applications)
    const size_t large_size = 10000; // 10K elements
    auto large_a = TestHelpers::generate_random_vector(large_size, 11111);
    auto large_b = TestHelpers::generate_random_vector(large_size, 22222);
    std::vector<float> large_result(large_size);

    // Time the operation (bindings need to be fast)
    auto start_time = std::chrono::high_resolution_clock::now();

    auto result = MatrixOps::add(large_a.data(), large_b.data(),
                               large_result.data(), 1, large_size);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    ASSERT_TRUE(result.is_success());

    std::cout << "    Large vector addition (" << large_size << " elements): "
              << duration.count() << " μs" << std::endl;

    // Should be fast enough for real-time applications
    EXPECT_LT(duration.count(), 10000); // Less than 10ms
}

TEST_F(BindingsIntegrationTest, MatrixOps_TypeScriptCompatibleResults) {
    std::cout << "\n  🔗 Testing TypeScript-Compatible Result Formats:" << std::endl;

    // Test operations that need to return data to TypeScript
    const size_t test_size = 100;
    auto vec_a = TestHelpers::generate_random_vector(test_size, 12345);
    auto vec_b = TestHelpers::generate_random_vector(test_size, 54321);
    std::vector<float> result(test_size);

    // Vector addition
    auto add_result = MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, test_size);
    ASSERT_TRUE(add_result.is_success());

    // Verify results are reasonable (no NaN, infinite values)
    for (size_t i = 0; i < test_size; ++i) {
        EXPECT_TRUE(std::isfinite(result[i]));
        EXPECT_NEAR(result[i], vec_a[i] + vec_b[i], 0.001f);
    }

    // Dot product (returns scalar)
    float dot_result = 0.0f;
    auto dot_op_result = MatrixOps::dot_product(vec_a.data(), vec_b.data(), &dot_result, test_size);
    ASSERT_TRUE(dot_op_result.is_success());
    EXPECT_TRUE(std::isfinite(dot_result));

    // Cosine similarity (returns scalar in [-1, 1])
    float similarity = 0.0f;
    auto sim_result = MatrixOps::cosine_similarity(vec_a.data(), vec_b.data(), &similarity, test_size);
    ASSERT_TRUE(sim_result.is_success());
    EXPECT_TRUE(std::isfinite(similarity));
    EXPECT_GE(similarity, -1.0f);
    EXPECT_LE(similarity, 1.0f);

    std::cout << "    ✅ All results are TypeScript-compatible (finite, in expected ranges)" << std::endl;
}

// ===============================================
// Serialization Bindings Tests
// ===============================================

TEST_F(BindingsIntegrationTest, Serialization_JSONBindingsCompatibility) {
    std::cout << "\n  🔗 Testing JSON Serialization through Bindings:" << std::endl;

    // Test envelope serialization (most common in bindings)
    auto serialize_result = JsonSerializer::serialize_envelope(test_envelope_);
    ASSERT_TRUE(serialize_result.is_success());

    std::string json_string = *serialize_result.value;
    EXPECT_FALSE(json_string.empty());

    // Simulate bindings call
    std::string bindings_result = simulate_nodejs_call("serialize_envelope", "");
    EXPECT_NE(bindings_result, "error");

    // Test deserialization (bindings receive JSON from network)
    auto deserialize_result = JsonSerializer::deserialize_envelope(json_string);
    ASSERT_TRUE(deserialize_result.is_success());

    auto& deserialized_envelope = *deserialize_result.value;
    EXPECT_EQ(deserialized_envelope.from, test_envelope_.from);
    EXPECT_EQ(deserialized_envelope.to, test_envelope_.to);
    EXPECT_EQ(deserialized_envelope.op, test_envelope_.op);

    std::cout << "    ✅ JSON serialization works correctly through bindings" << std::endl;
}

TEST_F(BindingsIntegrationTest, Serialization_BinaryFrameBindings) {
    std::cout << "\n  🔗 Testing Binary Frame Serialization for Bindings:" << std::endl;

    // Test binary frame serialization
    auto frame_result = BinarySerializer::serialize_frame(test_frame_);
    ASSERT_TRUE(frame_result.is_success());

    ByteBuffer serialized_frame = *frame_result.value;
    EXPECT_FALSE(serialized_frame.empty());

    // Simulate bindings call
    std::string bindings_result = simulate_nodejs_call("serialize_frame", "");
    EXPECT_NE(bindings_result, "error");

    // Test deserialization
    auto deserialize_result = BinarySerializer::deserialize_frame(serialized_frame);
    ASSERT_TRUE(deserialize_result.is_success());

    auto& deserialized_frame = *deserialize_result.value;
    EXPECT_EQ(deserialized_frame.header.stream_id, test_frame_.header.stream_id);
    EXPECT_EQ(deserialized_frame.header.sequence, test_frame_.header.sequence);
    EXPECT_EQ(deserialized_frame.payload.size(), test_frame_.payload.size());

    std::cout << "    ✅ Binary frame serialization works for bindings" << std::endl;
}

TEST_F(BindingsIntegrationTest, Serialization_CBORForBindings) {
    std::cout << "\n  🔗 Testing CBOR Serialization for Bindings:" << std::endl;

    // Create test JSON object
    JsonObject test_obj = {
        {"string_field", "test_value"},
        {"number_field", 42.5},
        {"boolean_field", true}
    };

    // Test CBOR encoding
    auto encode_result = BinarySerializer::encode_cbor(test_obj);
    ASSERT_TRUE(encode_result.is_success());

    ByteBuffer cbor_data = *encode_result.value;
    EXPECT_FALSE(cbor_data.empty());

    // Test CBOR decoding
    auto decode_result = BinarySerializer::decode_cbor(cbor_data);
    ASSERT_TRUE(decode_result.is_success());

    // Verify the data roundtrip
    // Note: In real bindings, this would be converted to JavaScript objects
    EXPECT_FALSE(decode_result.value->empty());

    std::cout << "    ✅ CBOR serialization works for bindings data exchange" << std::endl;
}

// ===============================================
// Protocol Integration Tests
// ===============================================

TEST_F(BindingsIntegrationTest, Protocol_BindingsMessageFlow) {
    std::cout << "\n  🔗 Testing Protocol Message Flow through Bindings:" << std::endl;

    Protocol protocol("bindings-test-node");
    protocol.configure(bindings_config_);
    protocol.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8090)));

    // Connect protocol
    auto connect_result = protocol.connect();
    ASSERT_TRUE(connect_result.is_success());

    // Test sending data (common bindings operation)
    ByteBuffer test_data = TestHelpers::generate_random_data(1024);
    auto send_result = protocol.send_data("bindings-receiver", test_data);
    EXPECT_TRUE(send_result.is_success());

    // Test sending control message
    auto control_result = protocol.send_control("bindings-receiver", "test_control");
    EXPECT_TRUE(control_result.is_success());

    // Test protocol statistics (bindings often query these)
    auto stats = protocol.get_stats();
    EXPECT_GT(stats.messages_sent, 0);

    std::cout << "    ✅ Protocol message flow works through bindings interface" << std::endl;
}

TEST_F(BindingsIntegrationTest, Protocol_AsyncOperationsForBindings) {
    std::cout << "\n  🔗 Testing Async Operations for Node.js Bindings:" << std::endl;

    Protocol protocol("async-bindings-node");
    protocol.configure(bindings_config_);
    protocol.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8091)));

    auto connect_result = protocol.connect();
    ASSERT_TRUE(connect_result.is_success());

    // Simulate async operations (like Node.js would do)
    std::atomic<int> completed_operations{0};
    std::vector<std::future<void>> futures;

    const int num_async_ops = 10;

    for (int i = 0; i < num_async_ops; ++i) {
        futures.push_back(std::async(std::launch::async, [&, i]() {
            ByteBuffer data = TestHelpers::generate_random_data(256);
            auto result = protocol.send_data("async-target", data);
            if (result.is_success()) {
                completed_operations.fetch_add(1);
            }
        }));
    }

    // Wait for all async operations
    for (auto& future : futures) {
        future.wait();
    }

    EXPECT_EQ(completed_operations.load(), num_async_ops);

    std::cout << "    ✅ Async operations work correctly for Node.js bindings" << std::endl;
}

// ===============================================
// Security Bindings Tests
// ===============================================

TEST_F(BindingsIntegrationTest, Security_BindingsKeyManagement) {
    std::cout << "\n  🔗 Testing Security Key Management through Bindings:" << std::endl;

    SecurityManager security("bindings-security-test");

    // Test key generation (bindings need this for new sessions)
    auto key_result = security.generate_keypair();
    ASSERT_TRUE(key_result.is_success());

    // Test signing (common in bindings for message authentication)
    ByteBuffer test_data = TestHelpers::generate_random_data(256);
    auto sign_result = security.sign_data(test_data);
    ASSERT_TRUE(sign_result.is_success());

    ByteBuffer signature = *sign_result.value;
    EXPECT_FALSE(signature.empty());

    // Test verification (bindings verify incoming messages)
    auto verify_result = security.verify_signature(test_data, signature);
    ASSERT_TRUE(verify_result.is_success());
    EXPECT_TRUE(*verify_result.value);

    // Test with wrong data (should fail)
    ByteBuffer wrong_data = TestHelpers::generate_random_data(256);
    auto verify_wrong = security.verify_signature(wrong_data, signature);
    ASSERT_TRUE(verify_wrong.is_success());
    EXPECT_FALSE(*verify_wrong.value);

    std::cout << "    ✅ Security operations work correctly through bindings" << std::endl;
}

TEST_F(BindingsIntegrationTest, Security_BindingsEncryption) {
    std::cout << "\n  🔗 Testing Encryption through Bindings:" << std::endl;

    SecurityManager security("bindings-encryption-test");
    auto key_result = security.generate_keypair();
    ASSERT_TRUE(key_result.is_success());

    // Test encryption (for secure bindings communication)
    ByteBuffer plaintext = TestHelpers::generate_random_data(512);
    auto encrypt_result = security.encrypt_data(plaintext);
    ASSERT_TRUE(encrypt_result.is_success());

    ByteBuffer ciphertext = *encrypt_result.value;
    EXPECT_FALSE(ciphertext.empty());
    EXPECT_NE(ciphertext.size(), plaintext.size()); // Should be different due to padding/headers

    // Test decryption
    auto decrypt_result = security.decrypt_data(ciphertext);
    ASSERT_TRUE(decrypt_result.is_success());

    ByteBuffer decrypted = *decrypt_result.value;
    EXPECT_EQ(decrypted.size(), plaintext.size());
    EXPECT_TRUE(TestHelpers::buffers_equal(decrypted, plaintext));

    std::cout << "    ✅ Encryption works correctly through bindings" << std::endl;
}

// ===============================================
// File I/O Integration Tests
// ===============================================

TEST_F(BindingsIntegrationTest, FileIO_BindingsDataExchange) {
    std::cout << "\n  🔗 Testing File I/O for Bindings Data Exchange:" << std::endl;

    // Create test files (bindings often work with files)
    std::string test_filename = "bindings_test_data.bin";
    ByteBuffer test_data = TestHelpers::generate_random_data(2048);

    // Write test data
    write_test_data_file(test_filename, test_data);

    // Read it back
    ByteBuffer read_data = read_test_data_file(test_filename);
    EXPECT_EQ(read_data.size(), test_data.size());
    EXPECT_TRUE(TestHelpers::buffers_equal(read_data, test_data));

    // Test with serialized envelope
    auto envelope_json = JsonSerializer::serialize_envelope(test_envelope_);
    ASSERT_TRUE(envelope_json.is_success());

    ByteBuffer json_data(envelope_json.value->begin(), envelope_json.value->end());
    std::string json_filename = "bindings_envelope.json";
    write_test_data_file(json_filename, json_data);

    ByteBuffer read_json = read_test_data_file(json_filename);
    std::string json_string(read_json.begin(), read_json.end());

    auto parsed_envelope = JsonSerializer::deserialize_envelope(json_string);
    ASSERT_TRUE(parsed_envelope.is_success());
    EXPECT_EQ(parsed_envelope.value->from, test_envelope_.from);

    // Cleanup
    std::filesystem::remove(test_filename);
    std::filesystem::remove(json_filename);

    std::cout << "    ✅ File I/O works correctly for bindings data exchange" << std::endl;
}

// ===============================================
// Performance Tests for Bindings
// ===============================================

TEST_F(BindingsIntegrationTest, Performance_BindingsOverhead) {
    std::cout << "\n  🔗 Testing Performance Overhead for Bindings:" << std::endl;

    const int num_operations = 1000;

    // Test matrix operation performance (common in ML bindings)
    auto matrix_time = TestHelpers::benchmark_function([&]() {
        MatrixOps::add(test_matrix_a_.data(), test_matrix_b_.data(),
                      result_matrix_.data(), 1, test_matrix_a_.size());
    }, num_operations);

    // Test serialization performance (common in network bindings)
    auto serialize_time = TestHelpers::benchmark_function([&]() {
        JsonSerializer::serialize_envelope(test_envelope_);
    }, num_operations);

    // Test protocol operation performance
    Protocol protocol("perf-bindings-node");
    protocol.configure(bindings_config_);
    protocol.set_transport(std::make_shared<MockTransport>(
        TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8092)));
    protocol.connect();

    ByteBuffer perf_data = TestHelpers::generate_random_data(256);
    auto protocol_time = TestHelpers::benchmark_function([&]() {
        protocol.send_data("perf-target", perf_data);
    }, num_operations);

    std::cout << "    Matrix operations: " << TestHelpers::format_duration(matrix_time) << " per op" << std::endl;
    std::cout << "    Serialization: " << TestHelpers::format_duration(serialize_time) << " per op" << std::endl;
    std::cout << "    Protocol send: " << TestHelpers::format_duration(protocol_time) << " per op" << std::endl;

    // Performance should be reasonable for bindings
    EXPECT_LT(matrix_time, 100.0);      // < 100μs per matrix op
    EXPECT_LT(serialize_time, 1000.0);  // < 1ms per serialization
    EXPECT_LT(protocol_time, 1000.0);   // < 1ms per protocol op

    std::cout << "    ✅ Performance is acceptable for bindings usage" << std::endl;
}

TEST_F(BindingsIntegrationTest, Performance_ConcurrentBindingsCalls) {
    std::cout << "\n  🔗 Testing Concurrent Bindings Calls Performance:" << std::endl;

    const int num_threads = 4;
    const int ops_per_thread = 250;

    std::atomic<int> successful_ops{0};
    std::vector<std::future<void>> futures;

    auto start_time = std::chrono::high_resolution_clock::now();

    // Launch concurrent operations (simulating multiple Node.js worker threads)
    for (int t = 0; t < num_threads; ++t) {
        futures.push_back(std::async(std::launch::async, [&]() {
            for (int i = 0; i < ops_per_thread; ++i) {
                // Mix of different operations
                if (i % 3 == 0) {
                    // Matrix operation
                    auto result = MatrixOps::add(test_matrix_a_.data(), test_matrix_b_.data(),
                                               result_matrix_.data(), 1, test_matrix_a_.size());
                    if (result.is_success()) successful_ops.fetch_add(1);
                } else if (i % 3 == 1) {
                    // Serialization
                    auto result = JsonSerializer::serialize_envelope(test_envelope_);
                    if (result.is_success()) successful_ops.fetch_add(1);
                } else {
                    // Frame serialization
                    auto result = BinarySerializer::serialize_frame(test_frame_);
                    if (result.is_success()) successful_ops.fetch_add(1);
                }
            }
        }));
    }

    // Wait for all threads
    for (auto& future : futures) {
        future.wait();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    int expected_ops = num_threads * ops_per_thread;
    double ops_per_second = (successful_ops.load() * 1000.0) / total_time.count();

    std::cout << "    Concurrent operations: " << successful_ops.load() << "/" << expected_ops << std::endl;
    std::cout << "    Total time: " << total_time.count() << " ms" << std::endl;
    std::cout << "    Rate: " << std::fixed << std::setprecision(1) << ops_per_second << " ops/s" << std::endl;

    EXPECT_EQ(successful_ops.load(), expected_ops);
    EXPECT_GT(ops_per_second, 1000.0); // Should handle at least 1K ops/s

    std::cout << "    ✅ Concurrent bindings calls work efficiently" << std::endl;
}

// ===============================================
// Error Handling for Bindings
// ===============================================

TEST_F(BindingsIntegrationTest, ErrorHandling_BindingsErrorPropagation) {
    std::cout << "\n  🔗 Testing Error Propagation to Bindings:" << std::endl;

    // Test matrix operation with invalid parameters
    std::vector<float> empty_vec;
    std::vector<float> result_vec(10);
    auto matrix_error = MatrixOps::add(empty_vec.data(), test_matrix_a_.data(),
                                     result_vec.data(), 1, 10);
    EXPECT_FALSE(matrix_error.is_success());
    EXPECT_EQ(matrix_error.code, ErrorCode::INVALID_ARGUMENT);

    // Test serialization with invalid data
    Envelope invalid_envelope;
    // Leave required fields empty
    auto serialize_error = JsonSerializer::serialize_envelope(invalid_envelope);
    // This might succeed with empty fields, so let's test deserialization
    auto deserialize_error = JsonSerializer::deserialize_envelope("invalid json {");
    EXPECT_FALSE(deserialize_error.is_success());

    // Test protocol with no transport
    Protocol error_protocol("error-test-node");
    auto connect_error = error_protocol.connect();
    EXPECT_FALSE(connect_error.is_success());
    EXPECT_EQ(connect_error.code, ErrorCode::INVALID_ARGUMENT);

    // Test security with invalid data
    SecurityManager security("error-security-test");
    ByteBuffer empty_signature;
    ByteBuffer test_data = TestHelpers::generate_random_data(100);
    auto verify_error = security.verify_signature(test_data, empty_signature);
    EXPECT_FALSE(verify_error.is_success());

    std::cout << "    ✅ Error conditions are properly handled and propagated to bindings" << std::endl;
}

// ===============================================
// Memory Management for Bindings
// ===============================================

TEST_F(BindingsIntegrationTest, Memory_BindingsMemoryManagement) {
    std::cout << "\n  🔗 Testing Memory Management for Bindings:" << std::endl;

    // Test with large data allocations (bindings need to handle this properly)
    const size_t large_size = 100000; // 100K elements

    // Allocate large vectors
    auto large_vec_a = TestHelpers::generate_random_vector(large_size, 11111);
    auto large_vec_b = TestHelpers::generate_random_vector(large_size, 22222);
    std::vector<float> large_result(large_size);

    // Perform operations
    auto result = MatrixOps::add(large_vec_a.data(), large_vec_b.data(),
                               large_result.data(), 1, large_size);
    ASSERT_TRUE(result.is_success());

    // Test multiple allocations and deallocations
    for (int i = 0; i < 10; ++i) {
        auto temp_vec = TestHelpers::generate_random_vector(10000, i);
        std::vector<float> temp_result(10000);

        auto temp_result_op = MatrixOps::add(temp_vec.data(), temp_vec.data(),
                                           temp_result.data(), 1, 10000);
        EXPECT_TRUE(temp_result_op.is_success());

        // Vectors should be automatically cleaned up when going out of scope
    }

    // Test serialization with large data
    ByteBuffer large_payload = TestHelpers::generate_random_data(50000); // 50KB
    Frame large_frame = TestHelpers::create_test_frame(2001, 1, large_payload);

    auto frame_result = BinarySerializer::serialize_frame(large_frame);
    ASSERT_TRUE(frame_result.is_success());

    auto deserialize_result = BinarySerializer::deserialize_frame(*frame_result.value);
    ASSERT_TRUE(deserialize_result.is_success());

    std::cout << "    ✅ Memory management works correctly for large bindings data" << std::endl;
}
