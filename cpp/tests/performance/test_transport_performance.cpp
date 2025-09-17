/**
 * UMICP Transport Performance Tests
 * Performance benchmarking for transport layer operations
 *
 * NOTE: These tests use MockTransport for performance benchmarking instead of real
 * network connections. This approach provides:
 * - Consistent and repeatable results
 * - No dependency on external infrastructure
 * - Precise control over test conditions (latency, throughput, failures)
 * - Fast execution for CI/CD pipelines
 * - Focus on measuring the performance of the UMICP protocol logic itself
 *
 * For integration testing with real transports, see the integration test suite.
 */

#include <gtest/gtest.h>
#include "transport.h"
#include "../utils/test_helpers.h"
#include <thread>
#include <future>
#include <atomic>

using namespace umicp;
using namespace umicp::testing;

class TransportPerformanceTest : public UMICPPerformanceTest {
protected:
    void SetUp() override {
        UMICPPerformanceTest::SetUp();

        // Use MockTransport for performance testing instead of real connections
        mock_config_ = TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8090);

        // Various message sizes for testing
        message_sizes_ = {64, 256, 1024, 4096, 16384, 65536}; // 64B to 64KB

        // Pre-generate test messages
        for (size_t size : message_sizes_) {
            test_messages_[size] = TestHelpers::generate_random_data(size);
        }

        test_envelope_ = TestHelpers::create_test_envelope("perf-sender", "perf-receiver");
    }

    TransportConfig mock_config_;
    std::vector<size_t> message_sizes_;
    std::map<size_t, ByteBuffer> test_messages_;
    Envelope test_envelope_;
};

// ===============================================
// S2S (Server-to-Server) Mock Performance Tests
// ===============================================

TEST_F(TransportPerformanceTest, S2S_MockCommunicationPerformance) {
    std::cout << "\n  📊 S2S Mock Communication Performance:" << std::endl;

    const int num_messages = 10000;
    const size_t message_size = 4096; // 4KB messages

    // Create two mock transports simulating S2S communication
    MockTransport server(mock_config_);
    auto client_config = mock_config_;
    client_config.port = 8091; // Different port
    MockTransport client(client_config);

    // Set up message routing between mocks
    std::atomic<size_t> server_received_count{0};
    std::atomic<size_t> client_received_count{0};

    server.set_message_callback([&](const ByteBuffer& data) {
        server_received_count++;
        // Echo back to client (simulating S2S response)
        client.simulate_receive_message(data);
    });

    client.set_message_callback([&](const ByteBuffer& data) {
        client_received_count++;
    });

    // Connect both
    ASSERT_TRUE(server.connect().is_success());
    ASSERT_TRUE(client.connect().is_success());

    auto s2s_time = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < num_messages; ++i) {
            const auto& message = test_messages_[message_size];
            server.simulate_receive_message(message);
        }
    }, 1);

    double messages_per_second = (num_messages * 1000000.0) / s2s_time;
    double latency_per_message = s2s_time / num_messages;

    PrintResults("S2S Round-trip", latency_per_message);

    std::cout << "    " << num_messages << " S2S messages (" << TestHelpers::format_bytes(message_size) << " each)" << std::endl;
    std::cout << "    Rate: " << std::fixed << std::setprecision(1) << messages_per_second << " msg/s" << std::endl;
    std::cout << "    Server received: " << server_received_count.load() << " messages" << std::endl;
    std::cout << "    Client received: " << client_received_count.load() << " messages" << std::endl;

    // Should handle high throughput with mocks
    EXPECT_GT(messages_per_second, 100000.0); // >100K msg/s for mocks
    EXPECT_LT(latency_per_message, 10.0); // <10μs per message
}

TEST_F(TransportPerformanceTest, S2S_MockLargePayloadPerformance) {
    std::cout << "\n  📊 S2S Mock Large Payload Performance:" << std::endl;

    const int num_large_messages = 100;
    const size_t large_message_size = 65536; // 64KB messages

    MockTransport server(mock_config_);
    auto client_config = mock_config_;
    client_config.port = 8092;
    MockTransport client(client_config);

    std::atomic<size_t> total_bytes_processed{0};

    server.set_message_callback([&](const ByteBuffer& data) {
        total_bytes_processed += data.size();
        // Simulate processing delay for large messages
        std::this_thread::sleep_for(std::chrono::microseconds(50)); // 50μs processing
    });

    ASSERT_TRUE(server.connect().is_success());
    ASSERT_TRUE(client.connect().is_success());

    auto large_payload_time = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < num_large_messages; ++i) {
            // Generate large message if not already in test_messages_
            if (test_messages_.find(large_message_size) == test_messages_.end()) {
                test_messages_[large_message_size] = TestHelpers::generate_random_data(large_message_size);
            }
            const auto& message = test_messages_[large_message_size];
            server.simulate_receive_message(message);
        }
    }, 1);

    double throughput_mbps = (total_bytes_processed.load() * 1000000.0) / (large_payload_time * 1000000.0); // MB/s

    PrintResults("Large S2S Payload", large_payload_time / num_large_messages);

    std::cout << "    " << num_large_messages << " large messages (" << TestHelpers::format_bytes(large_message_size) << " each)" << std::endl;
    std::cout << "    Total: " << TestHelpers::format_bytes(total_bytes_processed.load()) << " processed" << std::endl;
    std::cout << "    Throughput: " << std::fixed << std::setprecision(2) << throughput_mbps << " MB/s" << std::endl;

    // Should handle large payloads efficiently
    EXPECT_LT(large_payload_time / num_large_messages, 100.0); // <100μs per large message
}

// ===============================================
// Connection Performance Tests
// ===============================================

TEST_F(TransportPerformanceTest, Connection_EstablishmentSpeed) {
    std::cout << "\n  📊 Mock Connection Establishment Performance:" << std::endl;

    const int num_connections = 100;

    auto connection_time = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < num_connections; ++i) {
            // Use MockTransport for performance testing - no real network calls
            MockTransport transport(mock_config_);
            transport.connect();
            transport.disconnect();
        }
    }, 1);

    double connections_per_second = (num_connections * 1000000.0) / connection_time;
    double time_per_connection = connection_time / num_connections;

    PrintResults("Mock Connection Cycle", time_per_connection);

    std::cout << "    Rate: " << std::fixed << std::setprecision(1) << connections_per_second
              << " connections/s (mock)" << std::endl;

    // Should be very fast for mocks - sub-millisecond
    EXPECT_LT(time_per_connection, 100.0); // <100μs per mock connection
}

TEST_F(TransportPerformanceTest, Connection_ConcurrentConnections) {
    std::cout << "\n  📊 Mock Concurrent Connection Performance:" << std::endl;

    const int num_concurrent = 50;
    std::vector<std::unique_ptr<MockTransport>> transports;
    std::vector<std::future<void>> futures;

    // Create mock transports
    for (int i = 0; i < num_concurrent; ++i) {
        auto config = mock_config_;
        config.port = 8100 + i; // Different ports for variety
        transports.push_back(std::make_unique<MockTransport>(config));
    }

    auto connect_time = TestHelpers::benchmark_function([&]() {
        // Connect all concurrently
        for (auto& transport : transports) {
            futures.push_back(std::async(std::launch::async, [&transport]() {
                transport->connect();
            }));
        }

        // Wait for all to complete
        for (auto& future : futures) {
            future.wait();
        }
        futures.clear();
    }, 1);

    PrintResults("Mock Concurrent Connections", connect_time);

    std::cout << "    " << num_concurrent << " concurrent mock connections in "
              << TestHelpers::format_duration(connect_time) << std::endl;

    // Should be very fast for mocks
    EXPECT_LT(connect_time, 1000.0); // <1ms for 50 mock connections

    // Cleanup
    for (auto& transport : transports) {
        transport->disconnect();
    }
}

// ===============================================
// Message Sending Performance Tests
// ===============================================

TEST_F(TransportPerformanceTest, Sending_MessageSizeScaling) {
    std::cout << "\n  📊 Mock Message Size Performance Scaling:" << std::endl;
    std::cout << "    Size      |  Time (μs)  | Throughput (MB/s) | Messages/s" << std::endl;
    std::cout << "    ----------|-------------|-------------------|------------" << std::endl;

    MockTransport transport(mock_config_);
    ASSERT_TRUE(transport.connect().is_success());

    double last_throughput_mbps = 0.0;

    for (size_t size : message_sizes_) {
        const auto& message = test_messages_[size];

        auto send_time = TestHelpers::benchmark_function([&]() {
            transport.send(message);
        }, 1000);

        double throughput_mbps = (size * 1000000.0) / (send_time * 1024.0 * 1024.0);
        double messages_per_second = 1000000.0 / send_time;

        std::cout << "    " << std::setw(8) << TestHelpers::format_bytes(size)
                  << " | " << std::setw(10) << std::fixed << std::setprecision(2) << send_time
                  << " | " << std::setw(16) << std::setprecision(3) << throughput_mbps
                  << " | " << std::setw(10) << std::setprecision(1) << messages_per_second
                  << std::endl;

        last_throughput_mbps = throughput_mbps;
    }

    // Mock should achieve very high throughput (test with the last/largest message size)
    EXPECT_GT(last_throughput_mbps, 1000.0); // >1GB/s for mocks
}

TEST_F(TransportPerformanceTest, Sending_EnvelopeVsRawData) {
    std::cout << "\n  📊 Mock Envelope vs Raw Data Performance:" << std::endl;

    MockTransport transport(mock_config_);
    ASSERT_TRUE(transport.connect().is_success());

    const size_t test_size = 1024;
    const auto& raw_data = test_messages_[test_size];

    // Raw data sending
    auto raw_time = TestHelpers::benchmark_function([&]() {
        transport.send(raw_data);
    }, 1000);

    // Envelope sending
    auto envelope_time = TestHelpers::benchmark_function([&]() {
        transport.send_envelope(test_envelope_);
    }, 1000);

    PrintResults("Raw Data Send", raw_time);
    PrintResults("Envelope Send", envelope_time);

    double overhead_factor = envelope_time / raw_time;
    std::cout << "    Envelope overhead: " << std::fixed << std::setprecision(2)
              << overhead_factor << "x (" << std::setprecision(1)
              << ((overhead_factor - 1.0) * 100.0) << "% slower)" << std::endl;
}

TEST_F(TransportPerformanceTest, Sending_BinaryFrameVsJSON) {
    std::cout << "\n  📊 Mock Binary Frame vs JSON Performance:" << std::endl;

    MockTransport transport(mock_config_);
    ASSERT_TRUE(transport.connect().is_success());

    // Create test frame with payload
    ByteBuffer frame_payload = test_messages_[1024];
    Frame test_frame = TestHelpers::create_test_frame(1001, 1, frame_payload);

    // Binary frame sending
    auto frame_time = TestHelpers::benchmark_function([&]() {
        transport.send_frame(test_frame);
    }, 1000);

    // JSON envelope sending (equivalent data)
    auto json_time = TestHelpers::benchmark_function([&]() {
        transport.send_envelope(test_envelope_);
    }, 1000);

    PrintResults("Binary Frame", frame_time);
    PrintResults("JSON Envelope", json_time);

    double efficiency_factor = json_time / frame_time;
    std::cout << "    Binary efficiency: " << std::fixed << std::setprecision(2)
              << efficiency_factor << "x faster than JSON" << std::endl;
}

// ===============================================
// Throughput Tests
// ===============================================

TEST_F(TransportPerformanceTest, Throughput_SustainedSending) {
    std::cout << "\n  📊 Mock Sustained Throughput Test:" << std::endl;

    MockTransport transport(mock_config_);
    ASSERT_TRUE(transport.connect().is_success());

    const int num_messages = 1000;
    const size_t message_size = 4096; // 4KB messages
    const auto& test_message = test_messages_[message_size];

    auto total_time = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < num_messages; ++i) {
            transport.send(test_message);
        }
    }, 1);

    size_t total_bytes = num_messages * message_size;
    double throughput_mbps = (total_bytes * 1000000.0) / (total_time * 1024.0 * 1024.0);
    double messages_per_second = (num_messages * 1000000.0) / total_time;

    PrintResults("Sustained Send", total_time, num_messages);

    std::cout << "    Total: " << TestHelpers::format_bytes(total_bytes) << " in "
              << TestHelpers::format_duration(total_time) << std::endl;
    std::cout << "    Throughput: " << std::fixed << std::setprecision(2) << throughput_mbps
              << " MB/s (" << std::setprecision(1) << messages_per_second << " msg/s)" << std::endl;

    // Verify statistics
    auto stats = transport.get_stats();
    EXPECT_EQ(stats.messages_sent, num_messages);
    EXPECT_GE(stats.bytes_sent, total_bytes);
}

TEST_F(TransportPerformanceTest, Throughput_BurstSending) {
    std::cout << "\n  📊 Burst Sending Performance:" << std::endl;

    MockTransport transport(mock_config_);
    ASSERT_TRUE(transport.connect().is_success());

    std::vector<size_t> burst_sizes = {10, 50, 100, 500};
    const size_t message_size = 1024;
    const auto& test_message = test_messages_[message_size];

    for (size_t burst_size : burst_sizes) {
        auto burst_time = TestHelpers::benchmark_function([&]() {
            for (size_t i = 0; i < burst_size; ++i) {
                transport.send(test_message);
            }
        }, 10);

        double messages_per_second = (burst_size * 1000000.0) / burst_time;
        double time_per_message = burst_time / burst_size;

        std::cout << "    Burst " << std::setw(3) << burst_size
                  << ": " << std::setw(6) << std::fixed << std::setprecision(2) << time_per_message << " μs/msg"
                  << " (" << std::setw(8) << std::setprecision(1) << messages_per_second << " msg/s)" << std::endl;
    }
}

// ===============================================
// Concurrent Performance Tests
// ===============================================

TEST_F(TransportPerformanceTest, Concurrency_MultipleSenders) {
    std::cout << "\n  📊 Concurrent Senders Performance:" << std::endl;

    const int num_senders = 8;
    const int messages_per_sender = 100;
    const size_t message_size = 512;

    std::vector<std::unique_ptr<Transport>> transports;
    std::vector<std::future<void>> futures;
    std::atomic<int> total_sent{0};

    // Create transports
    for (int i = 0; i < num_senders; ++i) {
        auto config = mock_config_;
        config.port = 8200 + i;
        auto transport = TransportFactory::create_websocket(config);
        ASSERT_TRUE(transport->connect().is_success());
        transports.push_back(std::move(transport));
    }

    const auto& test_message = test_messages_[message_size];

    auto concurrent_time = TestHelpers::benchmark_function([&]() {
        // Start all senders
        for (int i = 0; i < num_senders; ++i) {
            futures.push_back(std::async(std::launch::async, [&, i]() {
                for (int j = 0; j < messages_per_sender; ++j) {
                    if (transports[i]->send(test_message).is_success()) {
                        total_sent.fetch_add(1);
                    }
                }
            }));
        }

        // Wait for all to complete
        for (auto& future : futures) {
            future.wait();
        }
    }, 1);

    int expected_total = num_senders * messages_per_sender;
    double messages_per_second = (expected_total * 1000000.0) / concurrent_time;
    size_t total_bytes = expected_total * message_size;
    double throughput_mbps = (total_bytes * 1000000.0) / (concurrent_time * 1024.0 * 1024.0);

    PrintResults("Concurrent Sending", concurrent_time);

    std::cout << "    " << num_senders << " senders × " << messages_per_sender << " messages = "
              << total_sent.load() << "/" << expected_total << " sent" << std::endl;
    std::cout << "    Rate: " << std::fixed << std::setprecision(1) << messages_per_second
              << " msg/s (" << std::setprecision(2) << throughput_mbps << " MB/s)" << std::endl;

    EXPECT_EQ(total_sent.load(), expected_total);
}

TEST_F(TransportPerformanceTest, Concurrency_SendReceiveSimulation) {
    std::cout << "\n  📊 Send/Receive Simulation Performance:" << std::endl;

    MockTransport mock_transport(mock_config_);
    ASSERT_TRUE(mock_transport.connect().is_success());

    const int num_messages = 500;
    const size_t message_size = 1024;
    const auto& test_message = test_messages_[message_size];

    std::atomic<int> messages_received{0};
    mock_transport.set_message_callback([&](const ByteBuffer& data) {
        messages_received.fetch_add(1);
        // Simulate processing time
        TestHelpers::sleep_ms(1);
    });

    auto simulation_time = TestHelpers::benchmark_function([&]() {
        // Start sender thread
        auto sender_future = std::async(std::launch::async, [&]() {
            for (int i = 0; i < num_messages; ++i) {
                mock_transport.send(test_message);
                TestHelpers::sleep_ms(2); // Simulate send interval
            }
        });

        // Start receiver simulation thread
        auto receiver_future = std::async(std::launch::async, [&]() {
            for (int i = 0; i < num_messages; ++i) {
                mock_transport.simulate_receive_message(test_message);
                TestHelpers::sleep_ms(3); // Simulate receive interval
            }
        });

        sender_future.wait();
        receiver_future.wait();
    }, 1);

    // Wait for final message processing
    TestHelpers::sleep_ms(100);

    PrintResults("Send/Receive Simulation", simulation_time);

    auto stats = mock_transport.get_stats();
    std::cout << "    Sent: " << stats.messages_sent << " messages ("
              << TestHelpers::format_bytes(stats.bytes_sent) << ")" << std::endl;
    std::cout << "    Received: " << messages_received.load() << "/" << num_messages
              << " messages processed" << std::endl;

    EXPECT_EQ(stats.messages_sent, num_messages);
    EXPECT_EQ(messages_received.load(), num_messages);
}

// ===============================================
// Memory and Resource Tests
// ===============================================

TEST_F(TransportPerformanceTest, Resource_StatisticsOverhead) {
    std::cout << "\n  📊 Statistics Tracking Overhead:" << std::endl;

    MockTransport transport(mock_config_);
    ASSERT_TRUE(transport.connect().is_success());

    const int num_operations = 10000;
    const auto& test_message = test_messages_[256];

    // Measure time with statistics
    auto time_with_stats = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < num_operations; ++i) {
            transport.send(test_message);
            if (i % 100 == 0) {
                transport.get_stats(); // Periodic stats access
            }
        }
    }, 1);

    // Reset and measure baseline (though our implementation always tracks stats)
    transport.reset_stats();
    auto time_baseline = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < num_operations; ++i) {
            transport.send(test_message);
        }
    }, 1);

    double overhead_percent = ((time_with_stats - time_baseline) / time_baseline) * 100.0;

    PrintResults("With Stats Access", time_with_stats);
    PrintResults("Baseline", time_baseline);

    std::cout << "    Statistics overhead: " << std::fixed << std::setprecision(2)
              << overhead_percent << "%" << std::endl;

    // Overhead should be minimal
    EXPECT_LT(std::abs(overhead_percent), 10.0); // Less than 10% overhead
}

TEST_F(TransportPerformanceTest, Resource_LargeMessageHandling) {
    std::cout << "\n  📊 Large Message Performance:" << std::endl;

    MockTransport transport(mock_config_);
    ASSERT_TRUE(transport.connect().is_success());

    // Test with progressively larger messages
    std::vector<size_t> large_sizes = {65536, 262144, 1048576}; // 64KB, 256KB, 1MB

    for (size_t size : large_sizes) {
        auto large_message = TestHelpers::generate_random_data(size);

        auto send_time = TestHelpers::benchmark_function([&]() {
            transport.send(large_message);
        }, 10);

        double throughput_mbps = (size * 1000000.0) / (send_time * 1024.0 * 1024.0);

        std::cout << "    " << std::setw(8) << TestHelpers::format_bytes(size)
                  << ": " << std::setw(10) << TestHelpers::format_duration(send_time)
                  << " (" << std::fixed << std::setprecision(2) << throughput_mbps << " MB/s)" << std::endl;

        // Large messages should still process in reasonable time
        EXPECT_LT(send_time, 100000.0); // 100ms for mock transport
    }
}

// ===============================================
// Transport Comparison Tests
// ===============================================

TEST_F(TransportPerformanceTest, Comparison_TransportTypes) {
    std::cout << "\n  📊 Transport Type Comparison:" << std::endl;

    const size_t test_size = 1024;
    const auto& test_message = test_messages_[test_size];
    const int num_operations = 100;

    // WebSocket transport
    auto ws_transport = TransportFactory::create_websocket(mock_config_);
    ASSERT_TRUE(ws_transport->connect().is_success());

    auto ws_time = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < num_operations; ++i) {
            ws_transport->send(test_message);
        }
    }, 1);

    // HTTP/2 transport (will show NOT_IMPLEMENTED performance)
    auto http2_config = mock_config_;
    http2_config.type = TransportType::HTTP2;
    auto http2_transport = TransportFactory::create_http2(http2_config);

    auto http2_time = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < num_operations; ++i) {
            http2_transport->send(test_message); // Will fail quickly
        }
    }, 1);

    double ws_rate = (num_operations * 1000000.0) / ws_time;
    double http2_rate = (num_operations * 1000000.0) / http2_time;

    std::cout << "    WebSocket: " << TestHelpers::format_duration(ws_time / num_operations)
              << " per operation (" << std::fixed << std::setprecision(1) << ws_rate << " ops/s)" << std::endl;
    std::cout << "    HTTP/2:    " << TestHelpers::format_duration(http2_time / num_operations)
              << " per operation (" << std::fixed << std::setprecision(1) << http2_rate << " ops/s)" << std::endl;

    // WebSocket should be much faster since HTTP/2 is not implemented
    EXPECT_GT(ws_rate, http2_rate * 10); // At least 10x faster
}

// ===============================================
// Performance Summary
// ===============================================

TEST_F(TransportPerformanceTest, Summary_OverallPerformance) {
    std::cout << "\n  📊 Transport Performance Summary:" << std::endl;

    MockTransport transport(mock_config_);
    ASSERT_TRUE(transport.connect().is_success());

    // Quick benchmark of key operations
    const auto& small_msg = test_messages_[256];
    const auto& large_msg = test_messages_[16384];

    // Small message throughput
    auto small_time = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < 1000; ++i) {
            transport.send(small_msg);
        }
    }, 1);

    // Large message throughput
    auto large_time = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < 100; ++i) {
            transport.send(large_msg);
        }
    }, 1);

    // Connection establishment
    auto connect_time = TestHelpers::benchmark_function([&]() {
        auto temp_transport = TransportFactory::create_websocket(mock_config_);
        temp_transport->connect();
        temp_transport->disconnect();
    }, 10);

    double small_rate = (1000 * 1000000.0) / small_time;
    double large_throughput = (100 * large_msg.size() * 1000000.0) / (large_time * 1024.0 * 1024.0);
    double connect_rate = (1000000.0) / connect_time;

    std::cout << "    Small messages (256B): " << std::fixed << std::setprecision(0) << small_rate << " msg/s" << std::endl;
    std::cout << "    Large messages (16KB): " << std::setprecision(2) << large_throughput << " MB/s" << std::endl;
    std::cout << "    Connection rate:        " << std::setprecision(1) << connect_rate << " conn/s" << std::endl;

    auto final_stats = transport.get_stats();
    std::cout << "    Total operations:       " << final_stats.messages_sent << " messages, "
              << TestHelpers::format_bytes(final_stats.bytes_sent) << std::endl;
}
