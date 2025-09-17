/**
 * UMICP Comprehensive Stress Tests
 * High-load testing scenarios for production readiness
 */

#include <gtest/gtest.h>
#include "protocol.h"
#include "transport.h"
#include "security.h"
#include "serialization.h"
#include "matrix_ops.h"
#include "../utils/test_helpers.h"
#include <thread>
#include <chrono>
#include <atomic>
#include <random>
#include <future>

using namespace umicp;
using namespace umicp::testing;

class StressTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        // Stress test configuration
        stress_config_.max_message_size = 1024 * 1024; // 1MB
        stress_config_.connection_timeout = 5000;
        stress_config_.enable_binary = true;
        stress_config_.preferred_format = ContentType::JSON;
        stress_config_.require_auth = false;
        stress_config_.require_encryption = false;

        // Initialize random number generator
        rng_.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    UMICPConfig stress_config_;
    std::mt19937 rng_;

    std::string generate_random_data(size_t size) {
        std::uniform_int_distribution<char> dist(32, 126);
        std::string data(size, ' ');
        for (char& c : data) {
            c = dist(rng_);
        }
        return data;
    }

    std::vector<float> generate_random_vector(size_t size) {
        std::uniform_real_distribution<float> dist(-1000.0f, 1000.0f);
        std::vector<float> vec(size);
        for (float& f : vec) {
            f = dist(rng_);
        }
        return vec;
    }
};

// ===============================================
// High-Volume Message Tests
// ===============================================

TEST_F(StressTest, HighVolume_MessagesPerSecond) {
    const int num_messages = 10000;
    const int num_threads = 10;
    const std::string message_data = generate_random_data(1024);

    Protocol protocol("stress-test-node");
    protocol.configure(stress_config_);
    TransportConfig transport_config;
    transport_config.type = TransportType::WEBSOCKET;
    transport_config.host = "localhost";
    transport_config.port = 8080;
    protocol.set_transport(std::make_shared<MockTransport>(transport_config));

    auto connect_result = protocol.connect();
    ASSERT_TRUE(connect_result.is_success());

    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};
    std::vector<std::thread> threads;

    auto start_time = std::chrono::high_resolution_clock::now();

    // Launch multiple threads sending messages
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&]() {
            for (int i = 0; i < num_messages / num_threads; ++i) {
                ByteBuffer data_bytes(message_data.begin(), message_data.end());
                auto result = protocol.send_data("test-to", data_bytes);
                if (result.is_success()) {
                    success_count++;
                } else {
                    failure_count++;
                }
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    double messages_per_second = (double)success_count.load() / (duration.count() / 1000.0);

    std::cout << "High Volume Test Results:" << std::endl;
    std::cout << "  Messages sent: " << success_count.load() << std::endl;
    std::cout << "  Messages failed: " << failure_count.load() << std::endl;
    std::cout << "  Duration: " << duration.count() << " ms" << std::endl;
    std::cout << "  Messages/sec: " << messages_per_second << std::endl;

    EXPECT_GT(success_count.load(), num_messages * 0.95); // 95% success rate
    EXPECT_GT(messages_per_second, 1000); // At least 1000 msg/s
}

TEST_F(StressTest, HighVolume_LargeMessages) {
    const int num_messages = 100;
    const size_t message_size = 1024 * 1024; // 1MB per message
    const std::string large_message = generate_random_data(message_size);

    Protocol protocol("stress-test-node");
    protocol.configure(stress_config_);
    TransportConfig transport_config;
    transport_config.type = TransportType::WEBSOCKET;
    transport_config.host = "localhost";
    transport_config.port = 8080;
    protocol.set_transport(std::make_shared<MockTransport>(transport_config));

    auto connect_result = protocol.connect();
    ASSERT_TRUE(connect_result.is_success());

    auto start_time = std::chrono::high_resolution_clock::now();

    int success_count = 0;
    for (int i = 0; i < num_messages; ++i) {
        ByteBuffer data_bytes(large_message.begin(), large_message.end());
        auto result = protocol.send_data("test-to", data_bytes);
        if (result.is_success()) {
            success_count++;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    double throughput_mbps = (double)(success_count * message_size) / (duration.count() / 1000.0) / (1024 * 1024);

    std::cout << "Large Message Test Results:" << std::endl;
    std::cout << "  Messages sent: " << success_count << "/" << num_messages << std::endl;
    std::cout << "  Duration: " << duration.count() << " ms" << std::endl;
    std::cout << "  Throughput: " << throughput_mbps << " MB/s" << std::endl;

    EXPECT_GT(success_count, num_messages * 0.9); // 90% success rate
    EXPECT_GT(throughput_mbps, 10.0); // At least 10 MB/s
}

// ===============================================
// Matrix Operations Stress Tests
// ===============================================

TEST_F(StressTest, MatrixOps_LargeMatrices) {
    const size_t matrix_size = 1000; // 1000x1000 matrix
    const int num_operations = 100;

    std::vector<float> matrix_a = generate_random_vector(matrix_size * matrix_size);
    std::vector<float> matrix_b = generate_random_vector(matrix_size * matrix_size);
    std::vector<float> result(matrix_size * matrix_size);

    auto start_time = std::chrono::high_resolution_clock::now();

    int success_count = 0;
    for (int i = 0; i < num_operations; ++i) {
        auto op_result = MatrixOps::multiply(matrix_a.data(), matrix_b.data(),
                                            result.data(), matrix_size, matrix_size, matrix_size);
        if (op_result.is_success()) {
            success_count++;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    double operations_per_second = (double)success_count / (duration.count() / 1000.0);

    std::cout << "Matrix Operations Stress Test:" << std::endl;
    std::cout << "  Operations completed: " << success_count << "/" << num_operations << std::endl;
    std::cout << "  Duration: " << duration.count() << " ms" << std::endl;
    std::cout << "  Operations/sec: " << operations_per_second << std::endl;

    EXPECT_GT(success_count, num_operations * 0.95); // 95% success rate
}

TEST_F(StressTest, MatrixOps_VeryLargeVectors) {
    const size_t vector_size = 100000; // 100k elements
    const int num_operations = 1000;

    std::vector<float> vec_a = generate_random_vector(vector_size);
    std::vector<float> vec_b = generate_random_vector(vector_size);
    std::vector<float> result(vector_size);

    auto start_time = std::chrono::high_resolution_clock::now();

    int success_count = 0;
    for (int i = 0; i < num_operations; ++i) {
        auto op_result = MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, vector_size);
        if (op_result.is_success()) {
            success_count++;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    double operations_per_second = (double)success_count / (duration.count() / 1000.0);

    std::cout << "Large Vector Operations Test:" << std::endl;
    std::cout << "  Operations completed: " << success_count << "/" << num_operations << std::endl;
    std::cout << "  Duration: " << duration.count() << " ms" << std::endl;
    std::cout << "  Operations/sec: " << operations_per_second << std::endl;

    EXPECT_GT(success_count, num_operations * 0.95); // 95% success rate
}

// ===============================================
// Serialization Stress Tests
// ===============================================

TEST_F(StressTest, Serialization_HighFrequency) {
    const int num_operations = 10000;
    // const size_t envelope_size = 1024; // unused

    Envelope test_envelope = TestHelpers::create_test_envelope("test-from", "test-to", OperationType::CONTROL);
    // Note: Envelope doesn't have payload field

    auto start_time = std::chrono::high_resolution_clock::now();

    int success_count = 0;
    for (int i = 0; i < num_operations; ++i) {
        auto serialize_result = JsonSerializer::serialize_envelope(test_envelope);
        if (serialize_result.is_success()) {
            auto deserialize_result = JsonSerializer::deserialize_envelope(*serialize_result.value);
            if (deserialize_result.is_success()) {
                success_count++;
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    double operations_per_second = (double)success_count / (duration.count() / 1000.0);

    std::cout << "Serialization Stress Test:" << std::endl;
    std::cout << "  Operations completed: " << success_count << "/" << num_operations << std::endl;
    std::cout << "  Duration: " << duration.count() << " ms" << std::endl;
    std::cout << "  Operations/sec: " << operations_per_second << std::endl;

    EXPECT_GT(success_count, num_operations * 0.95); // 95% success rate
    EXPECT_GT(operations_per_second, 1000); // At least 1000 ops/s
}

TEST_F(StressTest, Serialization_LargeData) {
    const int num_operations = 100;
    const size_t data_size = 10 * 1024 * 1024; // 10MB

    std::string large_data = generate_random_data(data_size);
    JsonObject test_obj = {{"data", large_data}};

    auto start_time = std::chrono::high_resolution_clock::now();

    int success_count = 0;
    for (int i = 0; i < num_operations; ++i) {
        auto encode_result = BinarySerializer::encode_cbor(test_obj);
        if (encode_result.is_success()) {
            auto decode_result = BinarySerializer::decode_cbor(*encode_result.value);
            if (decode_result.is_success()) {
                success_count++;
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    double throughput_mbps = (double)(success_count * data_size) / (duration.count() / 1000.0) / (1024 * 1024);

    std::cout << "Large Data Serialization Test:" << std::endl;
    std::cout << "  Operations completed: " << success_count << "/" << num_operations << std::endl;
    std::cout << "  Duration: " << duration.count() << " ms" << std::endl;
    std::cout << "  Throughput: " << throughput_mbps << " MB/s" << std::endl;

    EXPECT_GT(success_count, num_operations * 0.9); // 90% success rate
}

// ===============================================
// Security Stress Tests
// ===============================================

TEST_F(StressTest, Security_HighFrequencySigning) {
    const int num_operations = 10000;
    const std::string test_data = generate_random_data(1024);

    SecurityManager security("test-security");
    auto key_result = security.generate_keypair();
    ASSERT_TRUE(key_result.is_success());

    auto start_time = std::chrono::high_resolution_clock::now();

    int success_count = 0;
    for (int i = 0; i < num_operations; ++i) {
        ByteBuffer data_bytes(test_data.begin(), test_data.end());
        auto sign_result = security.sign_data(data_bytes);
        if (sign_result.is_success()) {
            auto verify_result = security.verify_signature(data_bytes, *sign_result.value);
            if (verify_result.is_success() && *verify_result.value) {
                success_count++;
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    double operations_per_second = (double)success_count / (duration.count() / 1000.0);

    std::cout << "Security Signing Stress Test:" << std::endl;
    std::cout << "  Operations completed: " << success_count << "/" << num_operations << std::endl;
    std::cout << "  Duration: " << duration.count() << " ms" << std::endl;
    std::cout << "  Operations/sec: " << operations_per_second << std::endl;

    EXPECT_GT(success_count, num_operations * 0.95); // 95% success rate
}

TEST_F(StressTest, Security_ConcurrentSessions) {
    const int num_sessions = 100;
    const int operations_per_session = 100;

    std::vector<std::unique_ptr<SecurityManager>> security_managers;
    std::vector<std::future<bool>> futures;

    // Create multiple security managers
    for (int i = 0; i < num_sessions; ++i) {
        auto security = std::make_unique<SecurityManager>("test-session-" + std::to_string(i));
        auto key_result = security->generate_keypair();
        if (key_result.is_success()) {
            security_managers.push_back(std::move(security));
        }
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // Launch concurrent operations
    for (auto& security : security_managers) {
        futures.push_back(std::async(std::launch::async, [&]() {
            int success_count = 0;
            for (int i = 0; i < operations_per_session; ++i) {
                std::string data = generate_random_data(512);
                ByteBuffer data_buffer(data.begin(), data.end());
                auto sign_result = security->sign_data(data_buffer);
                if (sign_result.is_success()) {
                    auto verify_result = security->verify_signature(data_buffer, *sign_result.value);
                    if (verify_result.is_success() && *verify_result.value) {
                        success_count++;
                    }
                }
            }
            return success_count == operations_per_session;
        }));
    }

    // Wait for all operations to complete
    int successful_sessions = 0;
    for (auto& future : futures) {
        if (future.get()) {
            successful_sessions++;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Concurrent Security Sessions Test:" << std::endl;
    std::cout << "  Sessions: " << security_managers.size() << std::endl;
    std::cout << "  Successful sessions: " << successful_sessions << std::endl;
    std::cout << "  Duration: " << duration.count() << " ms" << std::endl;

    EXPECT_GT(successful_sessions, num_sessions * 0.9); // 90% success rate
}

// ===============================================
// Memory Stress Tests
// ===============================================

TEST_F(StressTest, Memory_AllocationPatterns) {
    const int num_iterations = 1000;
    const size_t allocation_size = 1024 * 1024; // 1MB

    std::vector<std::unique_ptr<std::vector<float>>> allocations;

    auto start_time = std::chrono::high_resolution_clock::now();

    // Allocate and deallocate memory in patterns
    for (int i = 0; i < num_iterations; ++i) {
        // Allocate
        auto vec = std::make_unique<std::vector<float>>(allocation_size);
        std::fill(vec->begin(), vec->end(), 1.0f);
        allocations.push_back(std::move(vec));

        // Periodically deallocate to simulate real usage
        if (i % 10 == 0 && !allocations.empty()) {
            allocations.erase(allocations.begin());
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Memory Allocation Stress Test:" << std::endl;
    std::cout << "  Iterations: " << num_iterations << std::endl;
    std::cout << "  Allocations remaining: " << allocations.size() << std::endl;
    std::cout << "  Duration: " << duration.count() << " ms" << std::endl;

    // Clean up
    allocations.clear();

    EXPECT_LT(duration.count(), 10000); // Should complete within 10 seconds
}

// ===============================================
// Long-Running Stability Tests
// ===============================================

TEST_F(StressTest, Stability_LongRunning) {
    const int duration_seconds = 30;
    const int message_interval_ms = 100;

    Protocol protocol("stability-test-node");
    protocol.configure(stress_config_);
    TransportConfig transport_config;
    transport_config.type = TransportType::WEBSOCKET;
    transport_config.host = "localhost";
    transport_config.port = 8080;
    protocol.set_transport(std::make_shared<MockTransport>(transport_config));

    auto connect_result = protocol.connect();
    ASSERT_TRUE(connect_result.is_success());

    std::atomic<bool> running{true};
    std::atomic<int> message_count{0};
    std::atomic<int> error_count{0};

    // Launch message sending thread
    std::thread sender_thread([&]() {
        while (running) {
            std::string data = generate_random_data(1024);
            ByteBuffer data_bytes(data.begin(), data.end());
            auto result = protocol.send_data("test-to", data_bytes);

            if (result.is_success()) {
                message_count++;
            } else {
                error_count++;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(message_interval_ms));
        }
    });

    // Run for specified duration
    std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));
    running = false;
    sender_thread.join();

    double messages_per_second = (double)message_count.load() / duration_seconds;
    double error_rate = (double)error_count.load() / (message_count.load() + error_count.load());

    std::cout << "Long-Running Stability Test:" << std::endl;
    std::cout << "  Duration: " << duration_seconds << " seconds" << std::endl;
    std::cout << "  Messages sent: " << message_count.load() << std::endl;
    std::cout << "  Errors: " << error_count.load() << std::endl;
    std::cout << "  Messages/sec: " << messages_per_second << std::endl;
    std::cout << "  Error rate: " << (error_rate * 100) << "%" << std::endl;

    EXPECT_GT(message_count.load(), 100); // Should send at least 100 messages
    EXPECT_LT(error_rate, 0.05); // Error rate should be less than 5%
}
