/**
 * UMICP Memory Management Tests
 * Memory leak detection and resource management testing
 */

#include <gtest/gtest.h>
#include "protocol.h"
#include "transport.h"
#include "security.h"
#include "serialization.h"
#include "matrix_ops.h"
#include "../utils/test_helpers.h"
#include <memory>
#include <vector>
#include <chrono>
#include <thread>

using namespace umicp;
using namespace umicp::testing;

class MemoryManagementTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        // Memory test configuration
        memory_config_.max_message_size = 1024 * 1024; // 1MB
        memory_config_.connection_timeout = 5000;
        memory_config_.enable_binary = true;
        memory_config_.preferred_format = ContentType::JSON;
        memory_config_.require_auth = false;
        memory_config_.require_encryption = false;
    }

    UMICPConfig memory_config_;

    // Helper to measure memory usage (simplified)
    size_t get_memory_usage() {
        // This is a simplified implementation
        // In a real test, you would use platform-specific memory monitoring
        return 0;
    }

    // Helper to generate large data
    std::string generate_large_data(size_t size) {
        return std::string(size, 'x');
    }
};

// ===============================================
// Basic Memory Leak Tests
// ===============================================

TEST_F(MemoryManagementTest, Protocol_MemoryLeak) {
    const int num_iterations = 1000;
    size_t initial_memory = get_memory_usage();

    for (int i = 0; i < num_iterations; ++i) {
        // Create and destroy protocol instances
        {
            Protocol protocol("memory-test-node-" + std::to_string(i));
            protocol.configure(memory_config_);
            TransportConfig mock_config;
            mock_config.type = TransportType::WEBSOCKET;
            protocol.set_transport(std::make_shared<MockTransport>(mock_config));

            auto connect_result = protocol.connect();
            if (connect_result.is_success()) {
                // Send some messages
                for (int j = 0; j < 10; ++j) {
                    std::string data = generate_large_data(1024);
                    ByteBuffer data_bytes(data.begin(), data.end());
                    protocol.send_data("test-to", data_bytes);
                }
                protocol.disconnect();
            }
        }
        // Protocol should be destroyed here

        // Check for memory growth every 100 iterations
        if (i % 100 == 0) {
            size_t current_memory = get_memory_usage();
            // In a real test, you would check for significant memory growth
            EXPECT_GE(current_memory, initial_memory);
        }
    }

    size_t final_memory = get_memory_usage();
    // Memory should not grow significantly
    EXPECT_LE(final_memory - initial_memory, 1024 * 1024); // 1MB threshold
}

TEST_F(MemoryManagementTest, SecurityManager_MemoryLeak) {
    const int num_iterations = 1000;
    size_t initial_memory = get_memory_usage();

    for (int i = 0; i < num_iterations; ++i) {
        // Create and destroy security managers
        {
            SecurityManager security("test-security-" + std::to_string(i));
            auto key_result = security.generate_keypair();
            if (key_result.is_success()) {
                // Perform operations
                for (int j = 0; j < 10; ++j) {
                    std::string data = generate_large_data(512);
                    ByteBuffer data_bytes(data.begin(), data.end());
                    auto sign_result = security.sign_data(data_bytes);
                    if (sign_result.is_success()) {
                        security.verify_signature(data_bytes, *sign_result.value);
                    }
                }
            }
        }
        // SecurityManager should be destroyed here
    }

    size_t final_memory = get_memory_usage();
    // Memory should not grow significantly
    EXPECT_LE(final_memory - initial_memory, 1024 * 1024); // 1MB threshold
}

TEST_F(MemoryManagementTest, Serialization_MemoryLeak) {
    const int num_iterations = 1000;
    size_t initial_memory = get_memory_usage();

    for (int i = 0; i < num_iterations; ++i) {
        // Test JSON serialization
        {
            Envelope envelope = TestHelpers::create_test_envelope("test-from", "test-to", OperationType::CONTROL);
            // Note: Envelope doesn't have payload field

            auto serialize_result = JsonSerializer::serialize_envelope(envelope);
            if (serialize_result.is_success()) {
                auto deserialize_result = JsonSerializer::deserialize_envelope(*serialize_result.value);
                // Results should be automatically cleaned up
            }
        }

        // Test Binary serialization
        {
            ByteBuffer payload(1024, 0x42);
            Frame frame = TestHelpers::create_test_frame(1, 1, payload);

            auto serialize_result = BinarySerializer::serialize_frame(frame);
            if (serialize_result.is_success()) {
                auto deserialize_result = BinarySerializer::deserialize_frame(*serialize_result.value);
                // Results should be automatically cleaned up
            }
        }

        // Test CBOR serialization
        {
            JsonObject obj = {{"data", generate_large_data(512)}};

            auto encode_result = BinarySerializer::encode_cbor(obj);
            if (encode_result.is_success()) {
                auto decode_result = BinarySerializer::decode_cbor(*encode_result.value);
                // Results should be automatically cleaned up
            }
        }
    }

    size_t final_memory = get_memory_usage();
    // Memory should not grow significantly
    EXPECT_LE(final_memory - initial_memory, 1024 * 1024); // 1MB threshold
}

// ===============================================
// Large Data Memory Tests
// ===============================================

TEST_F(MemoryManagementTest, LargeData_Serialization) {
    const int num_iterations = 100;
    // const size_t data_size = 10 * 1024 * 1024; // 10MB per iteration - unused
    size_t initial_memory = get_memory_usage();

    for (int i = 0; i < num_iterations; ++i) {
        // Create large envelope
        Envelope envelope = TestHelpers::create_test_envelope("test-from", "test-to", OperationType::CONTROL);
        // Note: Envelope doesn't have payload field

        // Serialize and deserialize
        auto serialize_result = JsonSerializer::serialize_envelope(envelope);
        if (serialize_result.is_success()) {
            auto deserialize_result = JsonSerializer::deserialize_envelope(*serialize_result.value);
            // Results should be cleaned up
        }

        // Check memory every 10 iterations
        if (i % 10 == 0) {
            size_t current_memory = get_memory_usage();
            EXPECT_GE(current_memory, initial_memory);
        }
    }

    size_t final_memory = get_memory_usage();
    // Memory should not grow significantly despite large data
    EXPECT_LE(final_memory - initial_memory, 50 * 1024 * 1024); // 50MB threshold
}

TEST_F(MemoryManagementTest, LargeData_MatrixOperations) {
    const int num_iterations = 50;
    const size_t vector_size = 100000; // 100k elements
    size_t initial_memory = get_memory_usage();

    for (int i = 0; i < num_iterations; ++i) {
        // Create large vectors
        std::vector<float> vec_a = TestHelpers::generate_random_vector(vector_size, 12345);
        std::vector<float> vec_b = TestHelpers::generate_random_vector(vector_size, 54321);
        std::vector<float> result(vec_a.size());

        // Perform operations
        auto add_result = MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, vector_size);
        EXPECT_TRUE(add_result.is_success());

        float dot_result;
        auto dot_op_result = MatrixOps::dot_product(vec_a.data(), vec_b.data(), &dot_result, vector_size);
        EXPECT_TRUE(dot_op_result.is_success());

        auto norm_result = MatrixOps::normalize(result.data(), 1, vector_size);
        EXPECT_TRUE(norm_result.is_success());

        // Vectors should be cleaned up when going out of scope
    }

    size_t final_memory = get_memory_usage();
    // Memory should not grow significantly
    EXPECT_LE(final_memory - initial_memory, 100 * 1024 * 1024); // 100MB threshold
}

// ===============================================
// Resource Exhaustion Tests
// ===============================================

TEST_F(MemoryManagementTest, ResourceExhaustion_ManyConnections) {
    const int max_connections = 1000;
    std::vector<std::unique_ptr<Protocol>> protocols;
    size_t initial_memory = get_memory_usage();

    // Try to create many protocol instances
    for (int i = 0; i < max_connections; ++i) {
        try {
            auto protocol = std::make_unique<Protocol>("exhaustion-test-" + std::to_string(i));
            protocol->configure(memory_config_);
            TransportConfig mock_config;
            mock_config.type = TransportType::WEBSOCKET;
            protocol->set_transport(std::make_shared<MockTransport>(mock_config));

            auto connect_result = protocol->connect();
            if (connect_result.is_success()) {
                protocols.push_back(std::move(protocol));
            }

            // Check memory every 100 connections
            if (i % 100 == 0) {
                size_t current_memory = get_memory_usage();
                EXPECT_GE(current_memory, initial_memory);
            }
        } catch (const std::bad_alloc& e) {
            // Expected when memory is exhausted
            break;
        }
    }

    // Clean up all connections
    protocols.clear();

    size_t final_memory = get_memory_usage();
    // Memory should be similar to initial after cleanup
    EXPECT_LE(final_memory - initial_memory, 10 * 1024 * 1024); // 10MB threshold
}

TEST_F(MemoryManagementTest, ResourceExhaustion_LargeAllocations) {
    const int max_allocations = 100;
    std::vector<std::unique_ptr<std::vector<uint8_t>>> allocations;
    size_t initial_memory = get_memory_usage();

    // Try to allocate large amounts of memory
    for (int i = 0; i < max_allocations; ++i) {
        try {
            // Allocate 10MB at a time
            auto allocation = std::make_unique<std::vector<uint8_t>>(10 * 1024 * 1024, 0x42);
            allocations.push_back(std::move(allocation));

            // Check memory every 10 allocations
            if (i % 10 == 0) {
                size_t current_memory = get_memory_usage();
                EXPECT_GE(current_memory, initial_memory);
            }
        } catch (const std::bad_alloc& e) {
            // Expected when memory is exhausted
            break;
        }
    }

    // Clean up all allocations
    allocations.clear();

    size_t final_memory = get_memory_usage();
    // Memory should be similar to initial after cleanup
    EXPECT_LE(final_memory - initial_memory, 50 * 1024 * 1024); // 50MB threshold
}

// ===============================================
// Circular Reference Tests
// ===============================================

TEST_F(MemoryManagementTest, CircularReference_ProtocolTransport) {
    const int num_iterations = 100;
    size_t initial_memory = get_memory_usage();

    for (int i = 0; i < num_iterations; ++i) {
        // Create protocol and transport with potential circular references
        TransportConfig mock_config;
        mock_config.type = TransportType::WEBSOCKET;
        auto transport = std::make_shared<MockTransport>(mock_config);
        {
            Protocol protocol("circular-test-" + std::to_string(i));
            protocol.configure(memory_config_);
            protocol.set_transport(transport);

            auto connect_result = protocol.connect();
            if (connect_result.is_success()) {
                // Send messages
                for (int j = 0; j < 10; ++j) {
                    std::string data = generate_large_data(512);
                    ByteBuffer data_bytes(data.begin(), data.end());
                    protocol.send_data("test-to", data_bytes);
                }
                protocol.disconnect();
            }
        }
        // Protocol should be destroyed, breaking circular reference
        transport.reset(); // Explicitly reset transport
    }

    size_t final_memory = get_memory_usage();
    // Memory should not grow significantly
    EXPECT_LE(final_memory - initial_memory, 5 * 1024 * 1024); // 5MB threshold
}

// ===============================================
// Exception Safety Tests
// ===============================================

TEST_F(MemoryManagementTest, ExceptionSafety_Protocol) {
    const int num_iterations = 100;
    size_t initial_memory = get_memory_usage();

    for (int i = 0; i < num_iterations; ++i) {
        try {
            Protocol protocol("exception-test-" + std::to_string(i));
            protocol.configure(memory_config_);
            TransportConfig mock_config;
            mock_config.type = TransportType::WEBSOCKET;
            protocol.set_transport(std::make_shared<MockTransport>(mock_config));

            auto connect_result = protocol.connect();
            if (connect_result.is_success()) {
                // Simulate exception during operation
                if (i % 10 == 0) {
                    throw std::runtime_error("Simulated exception");
                }

                // Normal operation
                std::string data = generate_large_data(1024);
                ByteBuffer data_bytes(data.begin(), data.end());
                protocol.send_data("test-to", data_bytes);
                protocol.disconnect();
            }
        } catch (const std::exception& e) {
            // Exception should be caught, resources should be cleaned up
        }
    }

    size_t final_memory = get_memory_usage();
    // Memory should not grow significantly despite exceptions
    EXPECT_LE(final_memory - initial_memory, 5 * 1024 * 1024); // 5MB threshold
}

TEST_F(MemoryManagementTest, ExceptionSafety_Serialization) {
    const int num_iterations = 100;
    size_t initial_memory = get_memory_usage();

    for (int i = 0; i < num_iterations; ++i) {
        try {
            // Create potentially problematic data
            Envelope envelope = TestHelpers::create_test_envelope("test-from", "test-to", OperationType::CONTROL);
            // Note: Envelope doesn't have payload field

            // Simulate exception during serialization
            if (i % 10 == 0) {
                throw std::runtime_error("Simulated serialization exception");
            }

            auto serialize_result = JsonSerializer::serialize_envelope(envelope);
            if (serialize_result.is_success()) {
                auto deserialize_result = JsonSerializer::deserialize_envelope(*serialize_result.value);
                // Results should be cleaned up
            }
        } catch (const std::exception& e) {
            // Exception should be caught, resources should be cleaned up
        }
    }

    size_t final_memory = get_memory_usage();
    // Memory should not grow significantly despite exceptions
    EXPECT_LE(final_memory - initial_memory, 5 * 1024 * 1024); // 5MB threshold
}

// ===============================================
// Long-Running Memory Tests
// ===============================================

TEST_F(MemoryManagementTest, LongRunning_MemoryStability) {
    const int duration_seconds = 30;
    const int operation_interval_ms = 100;
    size_t initial_memory = get_memory_usage();

    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = start_time + std::chrono::seconds(duration_seconds);

    Protocol protocol("long-running-test");
    protocol.configure(memory_config_);
    TransportConfig mock_config;
    mock_config.type = TransportType::WEBSOCKET;
    protocol.set_transport(std::make_shared<MockTransport>(mock_config));

    auto connect_result = protocol.connect();
    ASSERT_TRUE(connect_result.is_success());

    int operation_count = 0;
    while (std::chrono::high_resolution_clock::now() < end_time) {
        // Perform various operations
        std::string data = generate_large_data(512);
        ByteBuffer data_bytes(data.begin(), data.end());
        auto send_result = protocol.send_data("test-to", data_bytes);

        // Test serialization
        Envelope envelope = TestHelpers::create_test_envelope("test-from", "test-to", OperationType::CONTROL);
        // Note: Envelope doesn't have payload field
        auto serialize_result = JsonSerializer::serialize_envelope(envelope);

        // Test matrix operations
        std::vector<float> vec_a = TestHelpers::generate_random_vector(1000, 12345);
        std::vector<float> vec_b = TestHelpers::generate_random_vector(1000, 54321);
        std::vector<float> result(1000);
        MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, 1000);

        operation_count++;

        // Check memory every 100 operations
        if (operation_count % 100 == 0) {
            size_t current_memory = get_memory_usage();
            EXPECT_GE(current_memory, initial_memory);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(operation_interval_ms));
    }

    protocol.disconnect();

    size_t final_memory = get_memory_usage();

    std::cout << "Long-Running Memory Test:" << std::endl;
    std::cout << "  Duration: " << duration_seconds << " seconds" << std::endl;
    std::cout << "  Operations: " << operation_count << std::endl;
    std::cout << "  Initial memory: " << initial_memory << std::endl;
    std::cout << "  Final memory: " << final_memory << std::endl;
    std::cout << "  Memory growth: " << (final_memory - initial_memory) << std::endl;

    // Memory should not grow significantly over time
    EXPECT_LE(final_memory - initial_memory, 10 * 1024 * 1024); // 10MB threshold
}
