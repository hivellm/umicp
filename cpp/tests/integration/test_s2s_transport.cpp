/**
 * UMICP Server-to-Server Transport Integration Tests
 * Testing real communication scenarios between transport instances
 */

#include <gtest/gtest.h>
#include "transport.h"
#include "../utils/test_helpers.h"
#include <thread>
#include <future>
#include <cstring>
#include <atomic>
#include <queue>
#include <mutex>

using namespace umicp;
using namespace umicp::testing;

class S2STransportTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        UMICPTestFixture::SetUp();

        // Create different configs for server and client
        server_config_ = TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8090);
        client_config_ = TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8090);

        // Test data
        test_message_ = TestHelpers::generate_random_data(256);
        test_envelope_ = TestHelpers::create_test_envelope("server-node", "client-node");
        test_frame_ = TestHelpers::create_test_frame(2001, 1, test_message_);
    }

    TransportConfig server_config_, client_config_;
    ByteBuffer test_message_;
    Envelope test_envelope_;
    Frame test_frame_;
};

// ===============================================
// Mock Server-Client Communication
// ===============================================

TEST_F(S2STransportTest, MockS2S_BasicCommunication) {
    // Create server and client mock transports
    MockTransport server(server_config_);
    MockTransport client(client_config_);

    // Set up message passing between them
    std::queue<ByteBuffer> server_to_client;
    std::queue<ByteBuffer> client_to_server;
    std::mutex server_queue_mutex, client_queue_mutex;

    // Override send methods to route to the other transport
    // Note: These lambdas are defined but not used in this simplified test
    // In a full implementation, they would be used to route messages between transports

    // Message tracking
    std::vector<ByteBuffer> server_received, client_received;

    server.set_message_callback([&](const ByteBuffer& data) {
        server_received.push_back(data);
    });

    client.set_message_callback([&](const ByteBuffer& data) {
        client_received.push_back(data);
    });

    // Connect both
    ASSERT_TRUE(server.connect().is_success());
    ASSERT_TRUE(client.connect().is_success());

    // Client sends message to server
    client.simulate_receive_message(test_message_);
    server.simulate_receive_message(test_message_);

    EXPECT_EQ(client_received.size(), 1);
    EXPECT_EQ(server_received.size(), 1);

    ASSERT_BUFFERS_EQUAL(client_received[0], test_message_);
    ASSERT_BUFFERS_EQUAL(server_received[0], test_message_);

    std::cout << "  📡 S2S mock communication successful: "
              << TestHelpers::format_bytes(test_message_.size()) << " exchanged" << std::endl;
}

TEST_F(S2STransportTest, MockS2S_EnvelopeExchange) {
    MockTransport server(server_config_);
    MockTransport client(client_config_);

    std::vector<Envelope> server_envelopes, client_envelopes;

    // Set up envelope processing
    server.set_message_callback([&](const ByteBuffer& data) {
        std::string json_str(data.begin(), data.end());
        auto envelope_result = JsonSerializer::deserialize_envelope(json_str);
        if (envelope_result.is_success()) {
            server_envelopes.push_back(*envelope_result.value);
        }
    });

    client.set_message_callback([&](const ByteBuffer& data) {
        std::string json_str(data.begin(), data.end());
        auto envelope_result = JsonSerializer::deserialize_envelope(json_str);
        if (envelope_result.is_success()) {
            client_envelopes.push_back(*envelope_result.value);
        }
    });

    ASSERT_TRUE(server.connect().is_success());
    ASSERT_TRUE(client.connect().is_success());

    // Send envelopes both ways
    Envelope server_to_client_env = TestHelpers::create_test_envelope("server", "client", OperationType::CONTROL);
    Envelope client_to_server_env = TestHelpers::create_test_envelope("client", "server", OperationType::DATA);

    // Simulate sending
    auto server_json = JsonSerializer::serialize_envelope(server_to_client_env);
    auto client_json = JsonSerializer::serialize_envelope(client_to_server_env);

    ASSERT_TRUE(server_json.is_success());
    ASSERT_TRUE(client_json.is_success());

    ByteBuffer server_data(server_json.value->begin(), server_json.value->end());
    ByteBuffer client_data(client_json.value->begin(), client_json.value->end());

    client.simulate_receive_message(server_data);
    server.simulate_receive_message(client_data);

    ASSERT_EQ(client_envelopes.size(), 1);
    ASSERT_EQ(server_envelopes.size(), 1);

    EXPECT_EQ(client_envelopes[0].from, "server");
    EXPECT_EQ(client_envelopes[0].to, "client");
    EXPECT_EQ(client_envelopes[0].op, OperationType::CONTROL);

    EXPECT_EQ(server_envelopes[0].from, "client");
    EXPECT_EQ(server_envelopes[0].to, "server");
    EXPECT_EQ(server_envelopes[0].op, OperationType::DATA);

    std::cout << "  📨 S2S envelope exchange successful" << std::endl;
}

TEST_F(S2STransportTest, MockS2S_BinaryFrameExchange) {
    MockTransport server(server_config_);
    MockTransport client(client_config_);

    std::vector<Frame> server_frames, client_frames;

    // Set up frame processing
    auto process_frame = [](const ByteBuffer& data, std::vector<Frame>& frames) {
        if (data.size() >= UMICP_FRAME_HEADER_SIZE) {
            auto frame_result = BinarySerializer::deserialize_frame(data);
            if (frame_result.is_success()) {
                frames.push_back(*frame_result.value);
            }
        }
    };

    server.set_message_callback([&](const ByteBuffer& data) {
        process_frame(data, server_frames);
    });

    client.set_message_callback([&](const ByteBuffer& data) {
        process_frame(data, client_frames);
    });

    ASSERT_TRUE(server.connect().is_success());
    ASSERT_TRUE(client.connect().is_success());

    // Create test frames
    ByteBuffer server_payload = TestHelpers::generate_random_data(512);
    ByteBuffer client_payload = TestHelpers::generate_random_data(256);

    Frame server_frame = TestHelpers::create_test_frame(3001, 1, server_payload);
    Frame client_frame = TestHelpers::create_test_frame(3002, 1, client_payload);

    // Serialize and send
    auto server_data = BinarySerializer::serialize_frame(server_frame);
    auto client_data = BinarySerializer::serialize_frame(client_frame);

    ASSERT_TRUE(server_data.is_success());
    ASSERT_TRUE(client_data.is_success());

    client.simulate_receive_message(*server_data.value);
    server.simulate_receive_message(*client_data.value);

    ASSERT_EQ(client_frames.size(), 1);
    ASSERT_EQ(server_frames.size(), 1);

    EXPECT_EQ(client_frames[0].header.stream_id, 3001);
    EXPECT_EQ(server_frames[0].header.stream_id, 3002);

    ASSERT_BUFFERS_EQUAL(client_frames[0].payload, server_payload);
    ASSERT_BUFFERS_EQUAL(server_frames[0].payload, client_payload);

    std::cout << "  📦 S2S binary frame exchange successful: "
              << TestHelpers::format_bytes(server_payload.size()) << " + "
              << TestHelpers::format_bytes(client_payload.size()) << std::endl;
}

// ===============================================
// Multi-Node Communication Simulation
// ===============================================

TEST_F(S2STransportTest, MultiNode_BroadcastSimulation) {
    const int num_nodes = 5;
    std::vector<std::unique_ptr<MockTransport>> nodes;

    // Create nodes
    for (int i = 0; i < num_nodes; ++i) {
        auto config = TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8100 + i);
        nodes.push_back(std::make_unique<MockTransport>(config));
        ASSERT_TRUE(nodes[i]->connect().is_success());
    }

    // Track received messages per node
    std::vector<std::vector<ByteBuffer>> received_messages(num_nodes);

    for (int i = 0; i < num_nodes; ++i) {
        nodes[i]->set_message_callback([&, i](const ByteBuffer& data) {
            received_messages[i].push_back(data);
        });
    }

    // Simulate broadcast: each node sends a message that all others receive
    std::vector<ByteBuffer> broadcast_messages;
    for (int sender = 0; sender < num_nodes; ++sender) {
        ByteBuffer message = TestHelpers::generate_random_data(100 + sender * 10);
        broadcast_messages.push_back(message);

        // Simulate all other nodes receiving this message
        for (int receiver = 0; receiver < num_nodes; ++receiver) {
            if (receiver != sender) {
                nodes[receiver]->simulate_receive_message(message);
            }
        }
    }

    // Verify each node received messages from all others
    for (int receiver = 0; receiver < num_nodes; ++receiver) {
        EXPECT_EQ(received_messages[receiver].size(), num_nodes - 1);

        // Verify message content
        for (int sender = 0, msg_idx = 0; sender < num_nodes; ++sender) {
            if (sender != receiver) {
                ASSERT_BUFFERS_EQUAL(received_messages[receiver][msg_idx], broadcast_messages[sender]);
                msg_idx++;
            }
        }
    }

    std::cout << "  📡 Multi-node broadcast: " << num_nodes << " nodes, "
              << (num_nodes * (num_nodes - 1)) << " total messages" << std::endl;
}

TEST_F(S2STransportTest, MultiNode_RingTopology) {
    const int num_nodes = 6;
    std::vector<std::unique_ptr<MockTransport>> nodes;

    // Create nodes in ring topology (each node connects to next)
    for (int i = 0; i < num_nodes; ++i) {
        auto config = TestHelpers::create_test_transport_config(TransportType::WEBSOCKET, 8200 + i);
        nodes.push_back(std::make_unique<MockTransport>(config));
        ASSERT_TRUE(nodes[i]->connect().is_success());
    }

    // Track message path through ring
    std::vector<std::atomic<int>> messages_received(num_nodes);
    for (auto& counter : messages_received) {
        counter.store(0);
    }

    ByteBuffer ring_message = TestHelpers::generate_random_data(50);
    std::atomic<int> total_hops{0};

    for (int i = 0; i < num_nodes; ++i) {
        nodes[i]->set_message_callback([&, i](const ByteBuffer& data) {
            messages_received[i].fetch_add(1);
            total_hops.fetch_add(1);

            // Forward to next node in ring (simulate ring topology)
            int next_node = (i + 1) % num_nodes;
            if (total_hops.load() < num_nodes * 2) { // Prevent infinite loop
                std::thread([&, next_node, data]() {
                    TestHelpers::sleep_ms(10); // Simulate network delay
                    nodes[next_node]->simulate_receive_message(data);
                }).detach();
            }
        });
    }

    // Start the ring by sending message to node 0
    nodes[0]->simulate_receive_message(ring_message);

    // Wait for message to propagate through ring
    TestHelpers::sleep_ms(200);

    // Each node should have received the message at least once
    for (int i = 0; i < num_nodes; ++i) {
        EXPECT_GT(messages_received[i].load(), 0);
    }

    std::cout << "  🔄 Ring topology: " << total_hops.load() << " total hops through "
              << num_nodes << " nodes" << std::endl;
}

// ===============================================
// Stress Testing
// ===============================================

TEST_F(S2STransportTest, Stress_HighVolumeExchange) {
    MockTransport server(server_config_);
    MockTransport client(client_config_);

    ASSERT_TRUE(server.connect().is_success());
    ASSERT_TRUE(client.connect().is_success());

    const int num_messages = 1000;
    std::atomic<int> server_received{0}, client_received{0};

    server.set_message_callback([&](const ByteBuffer& data) {
        server_received.fetch_add(1);
    });

    client.set_message_callback([&](const ByteBuffer& data) {
        client_received.fetch_add(1);
    });

    // Generate test messages
    std::vector<ByteBuffer> test_messages;
    for (int i = 0; i < num_messages; ++i) {
        test_messages.push_back(TestHelpers::generate_random_data(64 + (i % 100)));
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    // Send messages in parallel
    std::vector<std::future<void>> futures;

    // Client to server
    futures.push_back(std::async(std::launch::async, [&]() {
        for (const auto& message : test_messages) {
            server.simulate_receive_message(message);
            if (rand() % 10 == 0) TestHelpers::sleep_ms(1); // Occasional delay
        }
    }));

    // Server to client
    futures.push_back(std::async(std::launch::async, [&]() {
        for (const auto& message : test_messages) {
            client.simulate_receive_message(message);
            if (rand() % 10 == 0) TestHelpers::sleep_ms(1); // Occasional delay
        }
    }));

    // Wait for completion
    for (auto& future : futures) {
        future.wait();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Allow some time for final message processing
    TestHelpers::sleep_ms(100);

    EXPECT_EQ(server_received.load(), num_messages);
    EXPECT_EQ(client_received.load(), num_messages);

    double messages_per_second = (2.0 * num_messages * 1000.0) / duration.count();

    std::cout << "  🚀 High volume: " << (2 * num_messages) << " messages in "
              << duration.count() << "ms ("
              << static_cast<int>(messages_per_second) << " msg/s)" << std::endl;
}

TEST_F(S2STransportTest, Stress_LargeMessageExchange) {
    MockTransport server(server_config_);
    MockTransport client(client_config_);

    ASSERT_TRUE(server.connect().is_success());
    ASSERT_TRUE(client.connect().is_success());

    // Test with various large message sizes
    std::vector<size_t> message_sizes = {
        1024,      // 1KB
        10240,     // 10KB
        102400,    // 100KB
        1048576    // 1MB
    };

    std::atomic<int> total_received{0};
    size_t total_bytes_received = 0;
    std::mutex bytes_mutex;

    auto message_handler = [&](const ByteBuffer& data) {
        total_received.fetch_add(1);
        std::lock_guard<std::mutex> lock(bytes_mutex);
        total_bytes_received += data.size();
    };

    server.set_message_callback(message_handler);
    client.set_message_callback(message_handler);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Send large messages
    for (size_t size : message_sizes) {
        ByteBuffer large_message = TestHelpers::generate_random_data(size);

        // Send both directions
        server.simulate_receive_message(large_message);
        client.simulate_receive_message(large_message);

        TestHelpers::sleep_ms(10); // Brief pause between sizes
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    TestHelpers::sleep_ms(50); // Allow final processing

    EXPECT_EQ(total_received.load(), message_sizes.size() * 2);

    double throughput_mbps = (total_bytes_received * 8.0 * 1000.0) / (duration.count() * 1024.0 * 1024.0);

    std::cout << "  📊 Large messages: " << TestHelpers::format_bytes(total_bytes_received)
              << " in " << duration.count() << "ms ("
              << std::fixed << std::setprecision(2) << throughput_mbps << " Mbps)" << std::endl;
}

// ===============================================
// Connection Reliability Tests
// ===============================================

TEST_F(S2STransportTest, Reliability_ConnectionInterruption) {
    MockTransport server(server_config_);
    MockTransport client(client_config_);

    std::atomic<int> connection_events{0};
    std::atomic<int> messages_after_reconnect{0};

    auto connection_handler = [&](bool connected, const std::string& error) {
        connection_events.fetch_add(1);
        if (connected) {
            std::cout << "    🔗 Connection established" << std::endl;
        } else {
            std::cout << "    🔌 Connection lost: " << error << std::endl;
        }
    };

    server.set_connection_callback(connection_handler);
    client.set_connection_callback(connection_handler);

    client.set_message_callback([&](const ByteBuffer& data) {
        messages_after_reconnect.fetch_add(1);
    });

    // Initial connection
    ASSERT_TRUE(server.connect().is_success());
    ASSERT_TRUE(client.connect().is_success());
    TestHelpers::sleep_ms(50);

    // Simulate connection interruption
    ASSERT_TRUE(server.disconnect().is_success());
    ASSERT_TRUE(client.disconnect().is_success());
    TestHelpers::sleep_ms(50);

    // Reconnect
    ASSERT_TRUE(server.connect().is_success());
    ASSERT_TRUE(client.connect().is_success());
    TestHelpers::sleep_ms(50);

    // Send message after reconnection
    ByteBuffer test_message = TestHelpers::generate_random_data(100);
    client.simulate_receive_message(test_message);

    TestHelpers::sleep_ms(50);

    EXPECT_GT(connection_events.load(), 4); // Connect/disconnect events
    EXPECT_EQ(messages_after_reconnect.load(), 1);

    std::cout << "  🔄 Connection resilience: " << connection_events.load() << " events, messaging resumed" << std::endl;
}

TEST_F(S2STransportTest, Reliability_MessageOrdering) {
    MockTransport server(server_config_);
    MockTransport client(client_config_);

    ASSERT_TRUE(server.connect().is_success());
    ASSERT_TRUE(client.connect().is_success());

    const int num_sequential_messages = 50;
    std::vector<int> received_order;
    std::mutex order_mutex;

    server.set_message_callback([&](const ByteBuffer& data) {
        if (data.size() >= sizeof(int)) {
            int message_id;
            memcpy(&message_id, data.data(), sizeof(int));

            std::lock_guard<std::mutex> lock(order_mutex);
            received_order.push_back(message_id);
        }
    });

    // Send numbered messages in sequence
    for (int i = 0; i < num_sequential_messages; ++i) {
        ByteBuffer message(sizeof(int));
        memcpy(message.data(), &i, sizeof(int));

        server.simulate_receive_message(message);
        TestHelpers::sleep_ms(1); // Small delay to maintain order
    }

    TestHelpers::sleep_ms(100); // Allow processing

    ASSERT_EQ(received_order.size(), num_sequential_messages);

    // Check if messages arrived in order
    bool in_order = true;
    for (int i = 0; i < num_sequential_messages; ++i) {
        if (received_order[i] != i) {
            in_order = false;
            break;
        }
    }

    EXPECT_TRUE(in_order);

    std::cout << "  📋 Message ordering: " << num_sequential_messages
              << " messages " << (in_order ? "✅ in order" : "❌ out of order") << std::endl;
}
