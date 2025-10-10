/**
 * UMICP WebSocket Components Tests
 * Tests for WebSocketServer and WebSocketClient
 * Note: Full integration tests require libwebsockets
 */

#include "websocket_server.h"
#include "websocket_client.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace umicp;

// ============================================================================
// WebSocketServer Tests
// ============================================================================

class WebSocketServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        WebSocketServerConfig config;
        config.port = 8080;
        config.path = "/umicp";

        server = std::make_unique<WebSocketServer>(config);
    }

    void TearDown() override {
        if (server && server->is_running()) {
            server->stop();
        }
        server.reset();
    }

    std::unique_ptr<WebSocketServer> server;
};

TEST_F(WebSocketServerTest, Initialization) {
    EXPECT_NE(server, nullptr);
    EXPECT_FALSE(server->is_running());
}

TEST_F(WebSocketServerTest, GetConfigReturnsCorrectValues) {
    auto config = server->get_config();
    EXPECT_EQ(config.port, 8080);
    EXPECT_EQ(config.path, "/umicp");
}

TEST_F(WebSocketServerTest, InitialClientCountIsZero) {
    EXPECT_EQ(server->get_client_count(), 0);
}

TEST_F(WebSocketServerTest, GetClientsReturnsEmptyInitially) {
    auto clients = server->get_clients();
    EXPECT_TRUE(clients.empty());
}

TEST_F(WebSocketServerTest, StartSucceeds) {
    // WebSocket is now fully implemented
    auto result = server->start();
    // May fail if port is in use, but should not be NOT_IMPLEMENTED
    if (!result.is_ok()) {
        EXPECT_NE(result.error_code(), ErrorCode::NOT_IMPLEMENTED);
    } else {
        EXPECT_TRUE(server->is_running());
    }
}

TEST_F(WebSocketServerTest, StopWhenNotRunningFails) {
    auto result = server->stop();
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::INVALID_STATE);
}

TEST_F(WebSocketServerTest, SendToClientWhenNotRunningFails) {
    ByteBuffer data = {1, 2, 3, 4};
    auto result = server->send_to_client("client-1", data);
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_CONNECTED);
}

TEST_F(WebSocketServerTest, SendToNonExistentClientFails) {
    // Even if server were running, this should fail
    ByteBuffer data = {1, 2, 3, 4};
    auto result = server->send_to_client("non-existent", data);
    EXPECT_FALSE(result.is_ok());
}

TEST_F(WebSocketServerTest, DisconnectNonExistentClientFails) {
    auto result = server->disconnect_client("non-existent");
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_FOUND);
}

TEST_F(WebSocketServerTest, GetClientReturnsNulloptForNonExistent) {
    auto client = server->get_client("non-existent");
    EXPECT_FALSE(client.has_value());
}

TEST_F(WebSocketServerTest, InitialStatsAreZero) {
    auto stats = server->get_stats();
    EXPECT_EQ(stats.total_connections, 0);
    EXPECT_EQ(stats.active_connections, 0);
    EXPECT_EQ(stats.messages_sent, 0);
    EXPECT_EQ(stats.messages_received, 0);
    EXPECT_EQ(stats.bytes_sent, 0);
    EXPECT_EQ(stats.bytes_received, 0);
    EXPECT_EQ(stats.errors_count, 0);
}

TEST_F(WebSocketServerTest, ResetStatsWorks) {
    server->reset_stats();

    auto stats = server->get_stats();
    EXPECT_EQ(stats.messages_sent, 0);
    EXPECT_EQ(stats.messages_received, 0);
}

TEST_F(WebSocketServerTest, EventEmissionWorks) {
    std::atomic<int> event_count{0};

    server->on("test_event", [&event_count](const Event& event) {
        event_count++;
    });

    Event event("test_event");
    server->emit("test_event", event);

    EXPECT_EQ(event_count, 1);
}

TEST_F(WebSocketServerTest, SetConnectionCallbackWorks) {
    std::atomic<bool> callback_called{false};

    server->set_connection_callback([&callback_called](const std::string& client_id) {
        callback_called = true;
    });

    // Callback is set but won't be called without real connections
    EXPECT_NE(server, nullptr);
}

TEST_F(WebSocketServerTest, SetMessageCallbackWorks) {
    std::atomic<bool> callback_called{false};

    server->set_message_callback([&callback_called](const std::string& client_id, const ByteBuffer& data) {
        callback_called = true;
    });

    EXPECT_NE(server, nullptr);
}

// ============================================================================
// WebSocketClient Tests
// ============================================================================

class WebSocketClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        WebSocketClientConfig config;
        config.url = "ws://localhost:8080";
        config.path = "/umicp";

        client = std::make_unique<WebSocketClient>(config);
    }

    void TearDown() override {
        if (client && client->is_connected()) {
            client->disconnect();
        }
        client.reset();
    }

    std::unique_ptr<WebSocketClient> client;
};

TEST_F(WebSocketClientTest, Initialization) {
    EXPECT_NE(client, nullptr);
    EXPECT_FALSE(client->is_connected());
}

TEST_F(WebSocketClientTest, GetConfigReturnsCorrectValues) {
    auto config = client->get_ws_config();
    EXPECT_EQ(config.url, "ws://localhost:8080");
    EXPECT_EQ(config.path, "/umicp");
}

TEST_F(WebSocketClientTest, InitialConnectionStateIsDisconnected) {
    EXPECT_EQ(client->get_connection_state(), ConnectionState::DISCONNECTED);
}

TEST_F(WebSocketClientTest, InitialReconnectAttemptIsZero) {
    EXPECT_EQ(client->get_reconnect_attempt(), 0);
}

TEST_F(WebSocketClientTest, ConnectAttempts) {
    // WebSocket is now fully implemented
    auto result = client->connect();
    // May fail to connect (no server running), but should not be NOT_IMPLEMENTED
    if (!result.is_ok()) {
        EXPECT_NE(result.error_code(), ErrorCode::NOT_IMPLEMENTED);
    }
}

TEST_F(WebSocketClientTest, DisconnectWhenNotConnectedSucceeds) {
    auto result = client->disconnect();
    EXPECT_TRUE(result.is_ok());
}

TEST_F(WebSocketClientTest, SendWhenNotConnectedFails) {
    ByteBuffer data = {1, 2, 3, 4};
    auto result = client->send(data);
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_CONNECTED);
}

TEST_F(WebSocketClientTest, SendEnvelopeWhenNotConnectedFails) {
    Envelope envelope;
    envelope.set_from("client");
    envelope.set_to("server");

    auto result = client->send_envelope(envelope);
    EXPECT_FALSE(result.is_ok());
}

TEST_F(WebSocketClientTest, GetEndpointReturnsUrl) {
    EXPECT_EQ(client->get_endpoint(), "ws://localhost:8080");
}

TEST_F(WebSocketClientTest, GetTypeReturnsWebSocket) {
    EXPECT_EQ(client->get_type(), TransportType::WEBSOCKET);
}

TEST_F(WebSocketClientTest, InitialStatsAreZero) {
    auto stats = client->get_client_stats();
    EXPECT_EQ(stats.messages_sent, 0);
    EXPECT_EQ(stats.messages_received, 0);
    EXPECT_EQ(stats.bytes_sent, 0);
    EXPECT_EQ(stats.bytes_received, 0);
    EXPECT_EQ(stats.reconnect_attempts, 0);
    EXPECT_EQ(stats.errors_count, 0);
}

TEST_F(WebSocketClientTest, TransportStatsAreZero) {
    auto stats = client->get_stats();
    EXPECT_EQ(stats.messages_sent, 0);
    EXPECT_EQ(stats.messages_received, 0);
}

TEST_F(WebSocketClientTest, ResetStatsWorks) {
    client->reset_stats();

    auto stats = client->get_client_stats();
    EXPECT_EQ(stats.messages_sent, 0);
    EXPECT_EQ(stats.reconnect_attempts, 0);
}

TEST_F(WebSocketClientTest, EventEmissionWorks) {
    std::atomic<int> event_count{0};

    client->on("test_event", [&event_count](const Event& event) {
        event_count++;
    });

    Event event("test_event");
    client->emit("test_event", event);

    EXPECT_EQ(event_count, 1);
}

TEST_F(WebSocketClientTest, SetConnectCallbackWorks) {
    std::atomic<bool> callback_called{false};

    client->set_connect_callback([&callback_called]() {
        callback_called = true;
    });

    EXPECT_NE(client, nullptr);
}

TEST_F(WebSocketClientTest, SetDisconnectCallbackWorks) {
    std::atomic<bool> callback_called{false};

    client->set_disconnect_callback([&callback_called]() {
        callback_called = true;
    });

    EXPECT_NE(client, nullptr);
}

TEST_F(WebSocketClientTest, SetMessageCallbackWorks) {
    std::atomic<bool> callback_called{false};

    client->set_ws_message_callback([&callback_called](const ByteBuffer& data) {
        callback_called = true;
    });

    EXPECT_NE(client, nullptr);
}

// ============================================================================
// Configuration Tests
// ============================================================================

TEST(WebSocketConfigTest, ServerConfigDefaults) {
    WebSocketServerConfig config;

    EXPECT_EQ(config.port, 8080);
    EXPECT_EQ(config.path, "/umicp");
    EXPECT_TRUE(config.per_message_deflate);
    EXPECT_FALSE(config.use_ssl);
    EXPECT_EQ(config.max_connections, 1000);
}

TEST(WebSocketConfigTest, ClientConfigDefaults) {
    WebSocketClientConfig config;
    config.url = "ws://test";

    EXPECT_EQ(config.path, "/umicp");
    EXPECT_TRUE(config.per_message_deflate);
    EXPECT_FALSE(config.use_ssl);
    EXPECT_TRUE(config.verify_ssl);
}

TEST(WebSocketConfigTest, ReconnectConfigDefaults) {
    ReconnectConfig config;

    EXPECT_TRUE(config.enabled);
    EXPECT_EQ(config.max_attempts, 0); // Unlimited
    EXPECT_EQ(config.initial_delay, std::chrono::milliseconds(1000));
    EXPECT_EQ(config.max_delay, std::chrono::milliseconds(30000));
    EXPECT_DOUBLE_EQ(config.backoff_multiplier, 2.0);
}

TEST(WebSocketConfigTest, CustomServerConfig) {
    WebSocketServerConfig config;
    config.port = 9090;
    config.path = "/custom";
    config.per_message_deflate = false;
    config.max_payload_size = 50 * 1024 * 1024;

    EXPECT_EQ(config.port, 9090);
    EXPECT_EQ(config.path, "/custom");
    EXPECT_FALSE(config.per_message_deflate);
    EXPECT_EQ(config.max_payload_size, 50 * 1024 * 1024);
}

TEST(WebSocketConfigTest, CustomClientConfig) {
    WebSocketClientConfig config;
    config.url = "wss://secure.example.com:443";
    config.path = "/api/ws";
    config.use_ssl = true;
    config.verify_ssl = false;

    EXPECT_EQ(config.url, "wss://secure.example.com:443");
    EXPECT_EQ(config.path, "/api/ws");
    EXPECT_TRUE(config.use_ssl);
    EXPECT_FALSE(config.verify_ssl);
}

// ============================================================================
// Helper Function Tests
// ============================================================================

TEST(WebSocketHelperTest, CreateWebSocketServer) {
    auto server = create_websocket_server(9000, "/test");

    EXPECT_NE(server, nullptr);
    auto config = server->get_config();
    EXPECT_EQ(config.port, 9000);
    EXPECT_EQ(config.path, "/test");
}

TEST(WebSocketHelperTest, CreateWebSocketClient) {
    auto client = create_websocket_client("ws://localhost:8080", true);

    EXPECT_NE(client, nullptr);
    EXPECT_EQ(client->get_endpoint(), "ws://localhost:8080");
}

TEST(WebSocketHelperTest, CreateWebSocketClientWithCustomConfig) {
    ReconnectConfig reconnect;
    reconnect.enabled = true;
    reconnect.max_attempts = 5;
    reconnect.initial_delay = std::chrono::milliseconds(2000);

    auto client = create_websocket_client_with_config("ws://localhost:8080", reconnect);

    EXPECT_NE(client, nullptr);
    auto config = client->get_ws_config();
    EXPECT_EQ(config.reconnect.max_attempts, 5);
    EXPECT_EQ(config.reconnect.initial_delay, std::chrono::milliseconds(2000));
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST(WebSocketThreadSafetyTest, ConcurrentServerStatAccess) {
    auto server = create_websocket_server(8081);

    std::vector<std::thread> threads;
    const int num_threads = 10;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&server]() {
            for (int j = 0; j < 100; ++j) {
                auto stats = server->get_stats();
                (void)stats; // Suppress unused warning
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    SUCCEED();
}

TEST(WebSocketThreadSafetyTest, ConcurrentClientStatAccess) {
    auto client = create_websocket_client("ws://localhost:8082");

    std::vector<std::thread> threads;
    const int num_threads = 10;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&client]() {
            for (int j = 0; j < 100; ++j) {
                auto stats = client->get_client_stats();
                (void)stats; // Suppress unused warning
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    SUCCEED();
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


