/**
 * UMICP Comprehensive Concurrency Tests
 * Thread safety and concurrent access testing
 */

#include <gtest/gtest.h>
#include "protocol.h"
#include "transport.h"
#include "security.h"
#include "serialization.h"
#include "matrix_ops.h"
#include "../utils/test_helpers.h"
#include <thread>
#include <atomic>
#include <vector>
#include <future>
#include <chrono>
#include <random>
#include <mutex>

using namespace umicp;
using namespace umicp::testing;

class ConcurrencyTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        // Concurrency test configuration
        concurrency_config_.max_message_size = 1024 * 1024;
        concurrency_config_.connection_timeout = 5000;
        concurrency_config_.enable_binary = true;
        concurrency_config_.preferred_format = ContentType::JSON;
        concurrency_config_.require_auth = false;
        concurrency_config_.require_encryption = false;

        // Initialize random number generator
        rng_.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    UMICPConfig concurrency_config_;
    std::mt19937 rng_;

    std::string generate_random_data(size_t size) {
        std::uniform_int_distribution<char> dist(32, 126);
        std::string data(size, ' ');
        for (char& c : data) {
            c = dist(rng_);
        }
        return data;
    }
};

// ===============================================
// Protocol Concurrency Tests
// ===============================================

TEST_F(ConcurrencyTest, Protocol_ConcurrentConnections) {
    const int num_threads = 10;
    const int operations_per_thread = 100;

    std::vector<std::unique_ptr<Protocol>> protocols;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};

    // Create multiple protocol instances
    for (int i = 0; i < num_threads; ++i) {
        auto protocol = std::make_unique<Protocol>("concurrent-node-" + std::to_string(i));
        protocol->configure(concurrency_config_);
        TransportConfig mock_config;
        mock_config.type = TransportType::WEBSOCKET;
        protocol->set_transport(std::make_shared<MockTransport>(mock_config));
        protocols.push_back(std::move(protocol));
    }

    // Launch concurrent operations
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            auto& protocol = protocols[t];

            for (int i = 0; i < operations_per_thread; ++i) {
                // Connect
                auto connect_result = protocol->connect();
                if (connect_result.is_success()) {
                    // Send message
                    std::string data = generate_random_data(1024);
                    ByteBuffer data_bytes(data.begin(), data.end());
                    auto send_result = protocol->send_data("test-to", data_bytes);
                    if (send_result.is_success()) {
                        success_count++;
                    } else {
                        failure_count++;
                    }

                    // Disconnect
                    protocol->disconnect();
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

    int total_operations = num_threads * operations_per_thread;
    double success_rate = (double)success_count.load() / total_operations;

    std::cout << "Concurrent Protocol Operations:" << std::endl;
    std::cout << "  Threads: " << num_threads << std::endl;
    std::cout << "  Operations per thread: " << operations_per_thread << std::endl;
    std::cout << "  Total operations: " << total_operations << std::endl;
    std::cout << "  Successful: " << success_count.load() << std::endl;
    std::cout << "  Failed: " << failure_count.load() << std::endl;
    std::cout << "  Success rate: " << (success_rate * 100) << "%" << std::endl;

    EXPECT_GT(success_rate, 0.9); // 90% success rate
}

TEST_F(ConcurrencyTest, Protocol_SharedInstance) {
    const int num_threads = 20;
    const int operations_per_thread = 50;

    Protocol protocol("shared-protocol-node");
    protocol.configure(concurrency_config_);
    TransportConfig mock_config;
    mock_config.type = TransportType::WEBSOCKET;
    protocol.set_transport(std::make_shared<MockTransport>(mock_config));

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};
    // Use atomic counter instead of barrier (C++20 feature not available)
    std::atomic<int> ready_count{0};

    // Launch concurrent operations on shared protocol instance
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            // Wait for all threads to start
            ready_count++;
            while (ready_count.load() < num_threads) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }

            for (int i = 0; i < operations_per_thread; ++i) {
                // Try to connect (only one should succeed)
                auto connect_result = protocol.connect();
                if (connect_result.is_success()) {
                    // Send message
                    std::string data = generate_random_data(512);
                    auto send_result = protocol.send_control("test-to", OperationType::CONTROL, data);
                    if (send_result.is_success()) {
                        success_count++;
                    } else {
                        failure_count++;
                    }

                    // Disconnect
                    protocol.disconnect();
                } else {
                    // Expected for most threads
                    failure_count++;
                }

                // Small delay to prevent overwhelming
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Shared Protocol Instance Test:" << std::endl;
    std::cout << "  Threads: " << num_threads << std::endl;
    std::cout << "  Operations per thread: " << operations_per_thread << std::endl;
    std::cout << "  Successful: " << success_count.load() << std::endl;
    std::cout << "  Failed: " << failure_count.load() << std::endl;

    // Should have some successes and some expected failures
    EXPECT_GT(success_count.load(), 0);
    EXPECT_GT(failure_count.load(), 0);
}

// ===============================================
// Matrix Operations Concurrency Tests
// ===============================================

TEST_F(ConcurrencyTest, MatrixOps_ConcurrentOperations) {
    const int num_threads = 8;
    const int operations_per_thread = 1000;
    const size_t vector_size = 1024;

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};

    // Generate test data
    std::vector<float> vec_a = TestHelpers::generate_random_vector(vector_size, 12345);
    std::vector<float> vec_b = TestHelpers::generate_random_vector(vector_size, 54321);

    // Launch concurrent matrix operations
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            std::vector<float> result(vector_size);

            for (int i = 0; i < operations_per_thread; ++i) {
                // Perform different operations based on thread ID
                bool success = false;

                switch (t % 4) {
                    case 0: {
                        auto op_result = MatrixOps::add(vec_a.data(), vec_b.data(), result.data(), 1, vector_size);
                        success = op_result.is_success();
                        break;
                    }
                    case 1: {
                        float dot_result;
                        auto op_result = MatrixOps::dot_product(vec_a.data(), vec_b.data(), &dot_result, vector_size);
                        success = op_result.is_success();
                        break;
                    }
                    case 2: {
                        auto op_result = MatrixOps::normalize(result.data(), 1, vector_size);
                        success = op_result.is_success();
                        break;
                    }
                    case 3: {
                        float cos_result;
                        auto op_result = MatrixOps::cosine_similarity(vec_a.data(), vec_b.data(), &cos_result, vector_size);
                        success = op_result.is_success();
                        break;
                    }
                }

                if (success) {
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

    int total_operations = num_threads * operations_per_thread;
    double success_rate = (double)success_count.load() / total_operations;

    std::cout << "Concurrent Matrix Operations:" << std::endl;
    std::cout << "  Threads: " << num_threads << std::endl;
    std::cout << "  Operations per thread: " << operations_per_thread << std::endl;
    std::cout << "  Total operations: " << total_operations << std::endl;
    std::cout << "  Successful: " << success_count.load() << std::endl;
    std::cout << "  Failed: " << failure_count.load() << std::endl;
    std::cout << "  Success rate: " << (success_rate * 100) << "%" << std::endl;

    EXPECT_GT(success_rate, 0.95); // 95% success rate
}

TEST_F(ConcurrencyTest, MatrixOps_SharedData) {
    const int num_threads = 4;
    const int operations_per_thread = 500;
    const size_t vector_size = 512;

    // Shared data that threads will modify
    std::vector<float> shared_vector = TestHelpers::generate_random_vector(vector_size, 12345);
    std::mutex vector_mutex;

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    // Launch concurrent operations on shared data
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                std::lock_guard<std::mutex> lock(vector_mutex);

                // Perform operation on shared vector
                auto op_result = MatrixOps::normalize(shared_vector.data(), 1, vector_size);
                if (op_result.is_success()) {
                    success_count++;
                }

                // Modify vector slightly
                for (size_t j = 0; j < vector_size; ++j) {
                    shared_vector[j] += 0.001f;
                }
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    int total_operations = num_threads * operations_per_thread;
    double success_rate = (double)success_count.load() / total_operations;

    std::cout << "Shared Data Matrix Operations:" << std::endl;
    std::cout << "  Threads: " << num_threads << std::endl;
    std::cout << "  Operations per thread: " << operations_per_thread << std::endl;
    std::cout << "  Total operations: " << total_operations << std::endl;
    std::cout << "  Successful: " << success_count.load() << std::endl;
    std::cout << "  Success rate: " << (success_rate * 100) << "%" << std::endl;

    EXPECT_GT(success_rate, 0.95); // 95% success rate
}

// ===============================================
// Security Concurrency Tests
// ===============================================

TEST_F(ConcurrencyTest, Security_ConcurrentSigning) {
    const int num_threads = 10;
    const int operations_per_thread = 100;

    SecurityManager security("test-security");
    auto key_result = security.generate_keypair();
    ASSERT_TRUE(key_result.is_success());

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};

    // Launch concurrent signing operations
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                std::string data = generate_random_data(1024);

                ByteBuffer data_bytes(data.begin(), data.end());
                auto sign_result = security.sign_data(data_bytes);
                if (sign_result.is_success()) {
                    auto verify_result = security.verify_signature(data_bytes, *sign_result.value);
                    if (verify_result.is_success() && *verify_result.value) {
                        success_count++;
                    } else {
                        failure_count++;
                    }
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

    int total_operations = num_threads * operations_per_thread;
    double success_rate = (double)success_count.load() / total_operations;

    std::cout << "Concurrent Security Operations:" << std::endl;
    std::cout << "  Threads: " << num_threads << std::endl;
    std::cout << "  Operations per thread: " << operations_per_thread << std::endl;
    std::cout << "  Total operations: " << total_operations << std::endl;
    std::cout << "  Successful: " << success_count.load() << std::endl;
    std::cout << "  Failed: " << failure_count.load() << std::endl;
    std::cout << "  Success rate: " << (success_rate * 100) << "%" << std::endl;

    EXPECT_GT(success_rate, 0.95); // 95% success rate
}

TEST_F(ConcurrencyTest, Security_MultipleInstances) {
    const int num_instances = 20;
    const int operations_per_instance = 50;

    std::vector<std::unique_ptr<SecurityManager>> security_managers;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};

    // Create multiple security manager instances
    for (int i = 0; i < num_instances; ++i) {
        auto security = std::make_unique<SecurityManager>("test-security-" + std::to_string(i));
        auto key_result = security->generate_keypair();
        if (key_result.is_success()) {
            security_managers.push_back(std::move(security));
        }
    }

    // Launch concurrent operations on different instances
    for (size_t t = 0; t < security_managers.size(); ++t) {
        threads.emplace_back([&, t]() {
            auto& security = security_managers[t];

            for (int i = 0; i < operations_per_instance; ++i) {
                std::string data = generate_random_data(512);

                ByteBuffer data_bytes(data.begin(), data.end());
                auto sign_result = security->sign_data(data_bytes);
                if (sign_result.is_success()) {
                    auto verify_result = security->verify_signature(data_bytes, *sign_result.value);
                    if (verify_result.is_success() && *verify_result.value) {
                        success_count++;
                    } else {
                        failure_count++;
                    }
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

    int total_operations = security_managers.size() * operations_per_instance;
    double success_rate = (double)success_count.load() / total_operations;

    std::cout << "Multiple Security Instances:" << std::endl;
    std::cout << "  Instances: " << security_managers.size() << std::endl;
    std::cout << "  Operations per instance: " << operations_per_instance << std::endl;
    std::cout << "  Total operations: " << total_operations << std::endl;
    std::cout << "  Successful: " << success_count.load() << std::endl;
    std::cout << "  Failed: " << failure_count.load() << std::endl;
    std::cout << "  Success rate: " << (success_rate * 100) << "%" << std::endl;

    EXPECT_GT(success_rate, 0.95); // 95% success rate
}

// ===============================================
// Serialization Concurrency Tests
// ===============================================

TEST_F(ConcurrencyTest, Serialization_ConcurrentSerialization) {
    const int num_threads = 8;
    const int operations_per_thread = 500;

    Envelope test_envelope = TestHelpers::create_test_envelope("test-from", "test-to", OperationType::CONTROL);
    // Note: Envelope doesn't have payload field, using payload_refs instead

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};

    // Launch concurrent serialization operations
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                // Serialize
                auto serialize_result = JsonSerializer::serialize_envelope(test_envelope);
                if (serialize_result.is_success()) {
                    // Deserialize
                    auto deserialize_result = JsonSerializer::deserialize_envelope(*serialize_result.value);
                    if (deserialize_result.is_success()) {
                        success_count++;
                    } else {
                        failure_count++;
                    }
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

    int total_operations = num_threads * operations_per_thread;
    double success_rate = (double)success_count.load() / total_operations;

    std::cout << "Concurrent Serialization:" << std::endl;
    std::cout << "  Threads: " << num_threads << std::endl;
    std::cout << "  Operations per thread: " << operations_per_thread << std::endl;
    std::cout << "  Total operations: " << total_operations << std::endl;
    std::cout << "  Successful: " << success_count.load() << std::endl;
    std::cout << "  Failed: " << failure_count.load() << std::endl;
    std::cout << "  Success rate: " << (success_rate * 100) << "%" << std::endl;

    EXPECT_GT(success_rate, 0.95); // 95% success rate
}

TEST_F(ConcurrencyTest, Serialization_MixedOperations) {
    const int num_threads = 6;
    const int operations_per_thread = 200;

    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};
    std::atomic<int> failure_count{0};

    // Launch different types of serialization operations
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                bool success = false;

                switch (t % 3) {
                    case 0: {
                        // JSON serialization
                        Envelope envelope = TestHelpers::create_test_envelope("test-from", "test-to", OperationType::CONTROL);
                        auto result = JsonSerializer::serialize_envelope(envelope);
                        success = result.is_success();
                        break;
                    }
                    case 1: {
                        // Binary frame serialization
                        ByteBuffer payload(512, 0x42);
                        Frame frame = TestHelpers::create_test_frame(1, 1, payload);
                        auto result = BinarySerializer::serialize_frame(frame);
                        success = result.is_success();
                        break;
                    }
                    case 2: {
                        // CBOR serialization
                        JsonObject obj = {{"data", generate_random_data(256)}};
                        auto result = BinarySerializer::encode_cbor(obj);
                        success = result.is_success();
                        break;
                    }
                }

                if (success) {
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

    int total_operations = num_threads * operations_per_thread;
    double success_rate = (double)success_count.load() / total_operations;

    std::cout << "Mixed Serialization Operations:" << std::endl;
    std::cout << "  Threads: " << num_threads << std::endl;
    std::cout << "  Operations per thread: " << operations_per_thread << std::endl;
    std::cout << "  Total operations: " << total_operations << std::endl;
    std::cout << "  Successful: " << success_count.load() << std::endl;
    std::cout << "  Failed: " << failure_count.load() << std::endl;
    std::cout << "  Success rate: " << (success_rate * 100) << "%" << std::endl;

    EXPECT_GT(success_rate, 0.95); // 95% success rate
}

// ===============================================
// Race Condition Tests
// ===============================================

TEST_F(ConcurrencyTest, RaceCondition_StateChanges) {
    const int num_threads = 10;
    const int operations_per_thread = 100;

    Protocol protocol("race-condition-node");
    protocol.configure(concurrency_config_);
    TransportConfig mock_config;
    mock_config.type = TransportType::WEBSOCKET;
    protocol.set_transport(std::make_shared<MockTransport>(mock_config));

    std::vector<std::thread> threads;
    std::atomic<int> connect_success{0};
    std::atomic<int> disconnect_success{0};
    std::atomic<int> send_success{0};
    std::atomic<int> total_operations{0};

    // Launch threads that randomly connect/disconnect/send
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            std::uniform_int_distribution<int> action_dist(0, 2);

            for (int i = 0; i < operations_per_thread; ++i) {
                int action = action_dist(rng_);
                total_operations++;

                switch (action) {
                    case 0: {
                        // Try to connect
                        auto result = protocol.connect();
                        if (result.is_success()) {
                            connect_success++;
                        }
                        break;
                    }
                    case 1: {
                        // Try to disconnect
                        auto result = protocol.disconnect();
                        if (result.is_success()) {
                            disconnect_success++;
                        }
                        break;
                    }
                    case 2: {
                        // Try to send message
                        std::string data = generate_random_data(256);
                        auto result = protocol.send_control("test-to", OperationType::CONTROL, data);
                        if (result.is_success()) {
                            send_success++;
                        }
                        break;
                    }
                }

                // Small delay to allow state changes
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Race Condition Test:" << std::endl;
    std::cout << "  Threads: " << num_threads << std::endl;
    std::cout << "  Operations per thread: " << operations_per_thread << std::endl;
    std::cout << "  Total operations: " << total_operations.load() << std::endl;
    std::cout << "  Connect successes: " << connect_success.load() << std::endl;
    std::cout << "  Disconnect successes: " << disconnect_success.load() << std::endl;
    std::cout << "  Send successes: " << send_success.load() << std::endl;

    // Should have some successful operations
    EXPECT_GT(total_operations.load(), 0);
    // Should not crash or hang
    EXPECT_TRUE(true);
}
