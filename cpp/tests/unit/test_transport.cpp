/**
 * UMICP Transport Unit Tests
 * Testing transport layer functionality
 */

#include <gtest/gtest.h>
#include "transport.h"
#include "../utils/test_helpers.h"
#include <atomic>
#include <future>

using namespace umicp;
using namespace umicp::testing;

class TransportTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        // Create test configurations
        ws_config_ = TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8080);
        http2_config_ = TestHelpers::create_test_transport_config(TransportType::HTTP2, 8081);

        // Test data
        test_envelope_ = TestHelpers::create_test_envelope("transport-test", "target");
        test_data_ = TestHelpers::generate_random_data(1024);
        test_frame_ = TestHelpers::create_test_frame(1001, 1, test_data_);
    }

    TransportConfig ws_config_, http2_config_;
    Envelope test_envelope_;
    ByteBuffer test_data_;
    Frame test_frame_;
};

// ===============================================
// Transport Factory Tests
// ===============================================

TEST_F(TransportTest, Factory_CreateWebSocket) {
    auto transport = TransportFactory::create_websocket(ws_config_);

    ASSERT_NE(transport, nullptr);
    EXPECT_EQ(transport->get_type(), TransportType::WEBSOCKET);
    EXPECT_FALSE(transport->is_connected());

    auto config = transport->get_config();
    EXPECT_EQ(config.type, TransportType::WEBSOCKET);
    EXPECT_EQ(config.host, ws_config_.host);
    EXPECT_EQ(config.port, ws_config_.port);
}

TEST_F(TransportTest, Factory_CreateHTTP2) {
    auto transport = TransportFactory::create_http2(http2_config_);

    ASSERT_NE(transport, nullptr);
    EXPECT_EQ(transport->get_type(), TransportType::HTTP2);
    EXPECT_FALSE(transport->is_connected());
}

TEST_F(TransportTest, Factory_CreateGeneric) {
    auto ws_transport = TransportFactory::create(TransportType::WEBSOCKET, ws_config_);
    auto http2_transport = TransportFactory::create(TransportType::HTTP2, http2_config_);

    ASSERT_NE(ws_transport, nullptr);
    ASSERT_NE(http2_transport, nullptr);

    EXPECT_EQ(ws_transport->get_type(), TransportType::WEBSOCKET);
    EXPECT_EQ(http2_transport->get_type(), TransportType::HTTP2);
}

// ===============================================
// WebSocket Transport Tests
// ===============================================

TEST_F(TransportTest, WebSocket_BasicConnection) {
    auto transport = TransportFactory::create_websocket(ws_config_);

    EXPECT_FALSE(transport->is_connected());

    auto connect_result = transport->connect();
    ASSERT_TRUE(connect_result.is_success());
    EXPECT_TRUE(transport->is_connected());

    auto disconnect_result = transport->disconnect();
    ASSERT_TRUE(disconnect_result.is_success());
    EXPECT_FALSE(transport->is_connected());
}

TEST_F(TransportTest, WebSocket_DoubleConnection) {
    auto transport = TransportFactory::create_websocket(ws_config_);

    // First connection should succeed
    auto result1 = transport->connect();
    ASSERT_TRUE(result1.is_success());

    // Second connection should succeed (no-op)
    auto result2 = transport->connect();
    EXPECT_TRUE(result2.is_success());

    EXPECT_TRUE(transport->is_connected());
}

TEST_F(TransportTest, WebSocket_SendData) {
    auto transport = TransportFactory::create_websocket(ws_config_);

    // Should fail when not connected
    auto send_result = transport->send(test_data_);
    EXPECT_FALSE(send_result.is_success());
    EXPECT_EQ(send_result.code, ErrorCode::NETWORK_ERROR);

    // Connect and try again
    ASSERT_TRUE(transport->connect().is_success());

    send_result = transport->send(test_data_);
    EXPECT_TRUE(send_result.is_success());

    // Check statistics
    auto stats = transport->get_stats();
    EXPECT_GT(stats.bytes_sent, 0);
    EXPECT_GT(stats.messages_sent, 0);
}

TEST_F(TransportTest, WebSocket_SendEnvelope) {
    auto transport = TransportFactory::create_websocket(ws_config_);
    ASSERT_TRUE(transport->connect().is_success());

    auto result = transport->send_envelope(test_envelope_);
    ASSERT_TRUE(result.is_success());

    auto stats = transport->get_stats();
    EXPECT_GT(stats.bytes_sent, 0);
    EXPECT_GT(stats.messages_sent, 0);
}

TEST_F(TransportTest, WebSocket_SendFrame) {
    auto transport = TransportFactory::create_websocket(ws_config_);
    ASSERT_TRUE(transport->connect().is_success());

    auto result = transport->send_frame(test_frame_);
    ASSERT_TRUE(result.is_success());

    auto stats = transport->get_stats();
    EXPECT_GT(stats.bytes_sent, 0);
    EXPECT_EQ(stats.messages_sent, 1);
}

TEST_F(TransportTest, WebSocket_CallbacksConnection) {
    auto transport = TransportFactory::create_websocket(ws_config_);

    bool connected_called = false;
    bool disconnected_called = false;
    std::string connection_error;

    transport->set_connection_callback([&](bool connected, const std::string& error) {
        if (connected) {
            connected_called = true;
        } else {
            disconnected_called = true;
            connection_error = error;
        }
    });

    // Test connection callback
    ASSERT_TRUE(transport->connect().is_success());
    TestHelpers::sleep_ms(50); // Allow callback to execute

    EXPECT_TRUE(connected_called);

    // Test disconnection callback
    ASSERT_TRUE(transport->disconnect().is_success());
    TestHelpers::sleep_ms(50); // Allow callback to execute

    EXPECT_TRUE(disconnected_called);
}

TEST_F(TransportTest, WebSocket_CallbacksMessage) {
    auto transport = TransportFactory::create_websocket(ws_config_);

    std::vector<ByteBuffer> received_messages;
    transport->set_message_callback([&](const ByteBuffer& data) {
        received_messages.push_back(data);
    });

    ASSERT_TRUE(transport->connect().is_success());

    // Wait for heartbeat messages (simulated in MockTransport)
    TestHelpers::sleep_ms(1100); // Wait for at least one heartbeat

    EXPECT_GT(received_messages.size(), 0);

    auto stats = transport->get_stats();
    EXPECT_GT(stats.messages_received, 0);
    EXPECT_GT(stats.bytes_received, 0);
}

TEST_F(TransportTest, WebSocket_Statistics) {
    auto transport = TransportFactory::create_websocket(ws_config_);
    ASSERT_TRUE(transport->connect().is_success());

    // Initial stats should be mostly zero
    auto initial_stats = transport->get_stats();
    size_t initial_sent = initial_stats.messages_sent;

    // Send some data
    for (int i = 0; i < 5; ++i) {
        ASSERT_TRUE(transport->send(test_data_).is_success());
    }

    auto final_stats = transport->get_stats();
    EXPECT_EQ(final_stats.messages_sent, initial_sent + 5);
    EXPECT_GT(final_stats.bytes_sent, initial_stats.bytes_sent);
    EXPECT_EQ(final_stats.connection_count, 1);

    // Test stats reset
    transport->reset_stats();
    auto reset_stats = transport->get_stats();
    EXPECT_EQ(reset_stats.messages_sent, 0);
    EXPECT_EQ(reset_stats.bytes_sent, 0);
}

TEST_F(TransportTest, WebSocket_Configuration) {
    auto transport = TransportFactory::create_websocket(ws_config_);

    // Should not be able to configure while connected
    ASSERT_TRUE(transport->connect().is_success());

    TransportConfig new_config = ws_config_;
    new_config.port = 9999;

    auto config_result = transport->configure(new_config);
    EXPECT_FALSE(config_result.is_success());
    EXPECT_EQ(config_result.code, ErrorCode::NETWORK_ERROR);

    // Should be able to configure when disconnected
    ASSERT_TRUE(transport->disconnect().is_success());

    config_result = transport->configure(new_config);
    EXPECT_TRUE(config_result.is_success());

    auto current_config = transport->get_config();
    EXPECT_EQ(current_config.port, 9999);
}

TEST_F(TransportTest, WebSocket_Endpoint) {
    auto transport = TransportFactory::create_websocket(ws_config_);

    std::string endpoint = transport->get_endpoint();
    EXPECT_NE(endpoint.find("ws://"), std::string::npos);
    EXPECT_NE(endpoint.find(ws_config_.host), std::string::npos);
    EXPECT_NE(endpoint.find(std::to_string(ws_config_.port)), std::string::npos);

    std::cout << "  🌐 WebSocket endpoint: " << endpoint << std::endl;
}

// ===============================================
// HTTP/2 Transport Tests (Stubs)
// ===============================================

TEST_F(TransportTest, HTTP2_NotImplementedYet) {
    auto transport = TransportFactory::create_http2(http2_config_);

    // All operations should return NOT_IMPLEMENTED
    auto connect_result = transport->connect();
    auto send_result = transport->send(test_data_);
    auto send_envelope_result = transport->send_envelope(test_envelope_);
    auto send_frame_result = transport->send_frame(test_frame_);

    EXPECT_FALSE(connect_result.is_success());
    EXPECT_FALSE(send_result.is_success());
    EXPECT_FALSE(send_envelope_result.is_success());
    EXPECT_FALSE(send_frame_result.is_success());

    EXPECT_EQ(connect_result.code, ErrorCode::NOT_IMPLEMENTED);
    EXPECT_EQ(send_result.code, ErrorCode::NOT_IMPLEMENTED);

    EXPECT_FALSE(transport->is_connected());
}

TEST_F(TransportTest, HTTP2_ConfigurationWorks) {
    auto transport = TransportFactory::create_http2(http2_config_);

    // Configuration should work even if not implemented
    TransportConfig new_config = http2_config_;
    new_config.port = 8443;

    auto result = transport->configure(new_config);
    EXPECT_TRUE(result.is_success());

    auto config = transport->get_config();
    EXPECT_EQ(config.port, 8443);
}

TEST_F(TransportTest, HTTP2_Endpoint) {
    auto transport = TransportFactory::create_http2(http2_config_);

    std::string endpoint = transport->get_endpoint();
    EXPECT_NE(endpoint.find("https://"), std::string::npos);
    EXPECT_NE(endpoint.find(http2_config_.host), std::string::npos);

    std::cout << "  🌐 HTTP/2 endpoint: " << endpoint << std::endl;
}

// ===============================================
// Mock Transport Tests
// ===============================================

TEST_F(TransportTest, MockTransport_BasicFunctionality) {
    MockTransport mock_transport(ws_config_);

    EXPECT_FALSE(mock_transport.is_connected());

    auto connect_result = mock_transport.connect();
    ASSERT_TRUE(connect_result.is_success());
    EXPECT_TRUE(mock_transport.is_connected());

    auto send_result = mock_transport.send(test_data_);
    ASSERT_TRUE(send_result.is_success());

    // Check sent data
    const auto& sent_data = mock_transport.get_sent_data();
    ASSERT_EQ(sent_data.size(), 1);
    ASSERT_BUFFERS_EQUAL(sent_data[0], test_data_);
}

TEST_F(TransportTest, MockTransport_MessageSimulation) {
    MockTransport mock_transport(ws_config_);

    ByteBuffer received_message;
    bool message_received = false;

    mock_transport.set_message_callback([&](const ByteBuffer& data) {
        received_message = data;
        message_received = true;
    });

    ASSERT_TRUE(mock_transport.connect().is_success());

    // Simulate receiving a message
    ByteBuffer test_message = TestHelpers::generate_random_data(50);
    mock_transport.simulate_receive_message(test_message);

    EXPECT_TRUE(message_received);
    ASSERT_BUFFERS_EQUAL(received_message, test_message);

    auto stats = mock_transport.get_stats();
    EXPECT_EQ(stats.messages_received, 1);
    EXPECT_EQ(stats.bytes_received, 50);
}

// ===============================================
// Error Handling Tests
// ===============================================

TEST_F(TransportTest, ErrorHandling_SendWithoutConnection) {
    auto transport = TransportFactory::create_websocket(ws_config_);

    auto send_result = transport->send(test_data_);
    auto envelope_result = transport->send_envelope(test_envelope_);
    auto frame_result = transport->send_frame(test_frame_);

    EXPECT_FALSE(send_result.is_success());
    EXPECT_FALSE(envelope_result.is_success());
    EXPECT_FALSE(frame_result.is_success());

    EXPECT_EQ(send_result.code, ErrorCode::NETWORK_ERROR);
}

TEST_F(TransportTest, ErrorHandling_DoubleDisconnect) {
    auto transport = TransportFactory::create_websocket(ws_config_);

    ASSERT_TRUE(transport->connect().is_success());

    auto disconnect1 = transport->disconnect();
    auto disconnect2 = transport->disconnect();

    EXPECT_TRUE(disconnect1.is_success());
    EXPECT_TRUE(disconnect2.is_success()); // Should handle gracefully
}

// ===============================================
// Performance Tests
// ===============================================

TEST_F(TransportTest, Performance_ConnectionSpeed) {
    auto transport = TransportFactory::create_websocket(ws_config_);

    auto connect_time = TestHelpers::benchmark_function([&]() {
        transport->connect();
        transport->disconnect();
    }, 10);

    std::cout << "  📊 Connection cycle: " << TestHelpers::format_duration(connect_time) << std::endl;

    // Connection should be reasonably fast (under 100ms for mock)
    EXPECT_LT(connect_time, 100000.0); // 100ms in microseconds
}

TEST_F(TransportTest, Performance_SendThroughput) {
    auto transport = TransportFactory::create_websocket(ws_config_);
    ASSERT_TRUE(transport->connect().is_success());

    const int num_messages = 100;
    ByteBuffer small_message = TestHelpers::generate_random_data(100);

    auto send_time = TestHelpers::benchmark_function([&]() {
        for (int i = 0; i < num_messages; ++i) {
            transport->send(small_message);
        }
    }, 1);

    double messages_per_second = (num_messages * 1000000.0) / send_time;
    double throughput_mbps = (messages_per_second * small_message.size() * 8) / (1024 * 1024);

    std::cout << "  📊 Send throughput: " << static_cast<int>(messages_per_second)
              << " msg/s (" << std::fixed << std::setprecision(2) << throughput_mbps << " Mbps)" << std::endl;
}

TEST_F(TransportTest, Performance_LargeMessage) {
    auto transport = TransportFactory::create_websocket(ws_config_);
    ASSERT_TRUE(transport->connect().is_success());

    // Test with 1MB message
    ByteBuffer large_message = TestHelpers::generate_random_data(1024 * 1024);

    auto send_time = TestHelpers::benchmark_function([&]() {
        transport->send(large_message);
    }, 10);

    double throughput_mbps = (large_message.size() * 8 * 1000000.0) / (send_time * 1024 * 1024);

    std::cout << "  📊 Large message (" << TestHelpers::format_bytes(large_message.size())
              << "): " << TestHelpers::format_duration(send_time)
              << " (" << std::fixed << std::setprecision(2) << throughput_mbps << " Mbps)" << std::endl;
}

// ===============================================
// Concurrent Access Tests
// ===============================================

TEST_F(TransportTest, Concurrency_MultipleSenders) {
    auto transport = TransportFactory::create_websocket(ws_config_);
    ASSERT_TRUE(transport->connect().is_success());

    const int num_threads = 4;
    const int messages_per_thread = 25;
    std::vector<std::future<void>> futures;
    std::atomic<int> successful_sends{0};

    ByteBuffer test_message = TestHelpers::generate_random_data(50);

    for (int t = 0; t < num_threads; ++t) {
        futures.push_back(std::async(std::launch::async, [&]() {
            for (int i = 0; i < messages_per_thread; ++i) {
                if (transport->send(test_message).is_success()) {
                    successful_sends.fetch_add(1);
                }
                TestHelpers::sleep_ms(1); // Small delay to interleave
            }
        }));
    }

    // Wait for all threads to complete
    for (auto& future : futures) {
        future.wait();
    }

    EXPECT_EQ(successful_sends.load(), num_threads * messages_per_thread);

    auto stats = transport->get_stats();
    EXPECT_EQ(stats.messages_sent, num_threads * messages_per_thread);

    std::cout << "  📊 Concurrent sends: " << successful_sends.load() << " messages from "
              << num_threads << " threads" << std::endl;
}

TEST_F(TransportTest, Concurrency_SendReceive) {
    MockTransport mock_transport(ws_config_);
    ASSERT_TRUE(mock_transport.connect().is_success());

    std::atomic<int> messages_received{0};
    mock_transport.set_message_callback([&](const ByteBuffer& data) {
        messages_received.fetch_add(1);
    });

    const int num_messages = 50;
    ByteBuffer test_message = TestHelpers::generate_random_data(100);

    // Start sender thread
    auto sender_future = std::async(std::launch::async, [&]() {
        for (int i = 0; i < num_messages; ++i) {
            mock_transport.send(test_message);
            TestHelpers::sleep_ms(10);
        }
    });

    // Start receiver simulation thread
    auto receiver_future = std::async(std::launch::async, [&]() {
        for (int i = 0; i < num_messages; ++i) {
            mock_transport.simulate_receive_message(test_message);
            TestHelpers::sleep_ms(12);
        }
    });

    sender_future.wait();
    receiver_future.wait();

    auto stats = mock_transport.get_stats();
    EXPECT_EQ(stats.messages_sent, num_messages);
    EXPECT_EQ(messages_received.load(), num_messages);

    std::cout << "  📊 Concurrent send/receive: " << num_messages << " messages each direction" << std::endl;
}
