/**
 * UMICP HTTP Integration Tests
 * Tests for StreamableHTTPClient and StreamableHTTPServer working together
 */

#include "http_client.h"
#include "http_server.h"
#include "envelope.h"
#include "serialization.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace umicp;

// ============================================================================
// Test Helpers
// ============================================================================

class HTTPIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Configure server
        HTTPServerConfig server_config;
        server_config.port = test_port_;
        server_config.host = "127.0.0.1";
        server_config.path = "/umicp";
        server_config.worker_threads = 2;

        server_ = std::make_unique<StreamableHTTPServer>(server_config);

        // Configure client
        HTTPClientConfig client_config;
        client_config.base_url = "http://127.0.0.1:" + std::to_string(test_port_);
        client_config.path = "/umicp";
        client_config.timeout = std::chrono::milliseconds(5000);
        client_config.connect_timeout = std::chrono::milliseconds(2000);

        client_ = std::make_unique<StreamableHTTPClient>(client_config);
    }

    void TearDown() override {
        if (client_) {
            client_->disconnect();
        }
        client_.reset();

        if (server_ && server_->is_running()) {
            server_->stop();
        }
        server_.reset();

        // Give time for cleanup
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    bool StartServer() {
        auto result = server_->start();
        if (result.is_ok()) {
            // Give server time to start
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return true;
        }
        return false;
    }

    static constexpr uint16_t test_port_ = 19080;
    std::unique_ptr<StreamableHTTPServer> server_;
    std::unique_ptr<StreamableHTTPClient> client_;
};

// ============================================================================
// Basic Integration Tests
// ============================================================================

TEST_F(HTTPIntegrationTest, ServerStartsSuccessfully) {
    EXPECT_TRUE(StartServer());
    EXPECT_TRUE(server_->is_running());
}

TEST_F(HTTPIntegrationTest, ClientConnectsToServer) {
    ASSERT_TRUE(StartServer());

    auto result = client_->connect();
    EXPECT_TRUE(result.is_ok()) << "Connect failed: " << result.error();
    EXPECT_TRUE(client_->is_connected());
}

TEST_F(HTTPIntegrationTest, ClientHealthCheckSucceeds) {
    ASSERT_TRUE(StartServer());
    ASSERT_TRUE(client_->connect().is_ok());

    auto result = client_->health_check();
    EXPECT_TRUE(result.is_ok()) << "Health check failed: " << result.error();
}

// ============================================================================
// Envelope Exchange Tests
// ============================================================================

TEST_F(HTTPIntegrationTest, SendReceiveEnvelope) {
    ASSERT_TRUE(StartServer());

    // Set up server handler
    std::atomic<bool> server_received{false};
    server_->on_message([&server_received](const Envelope& envelope, const HTTPRequest& req) -> Result<Envelope> {
        server_received = true;

        // Create response
        Envelope response;
        response.version = "1.0";
        response.from = "server";
        response.to = envelope.from;
        response.op = OperationType::ACK;

        // Echo message via msg_id
        response.msg_id = "Echo: " + envelope.msg_id;

        return Result<Envelope>::ok(response);
    });

    ASSERT_TRUE(client_->connect().is_ok());

    // Create request envelope
    Envelope request;
    request.version = "1.0";
    request.from = "client";
    request.to = "server";
    request.op = OperationType::DATA;
    request.msg_id = "Hello, Server!";

    // Send and receive
    auto result = client_->send(request);
    ASSERT_TRUE(result.is_ok()) << "Send failed: " << result.error();

    auto response = result.unwrap();
    EXPECT_EQ(response.from, "server");
    EXPECT_EQ(response.to, "client");
    EXPECT_EQ(response.op, OperationType::ACK);
    EXPECT_EQ(response.msg_id, "Echo: Hello, Server!");

    EXPECT_TRUE(server_received);
}

TEST_F(HTTPIntegrationTest, MultipleRequests) {
    ASSERT_TRUE(StartServer());

    std::atomic<int> request_count{0};
    server_->on_message([&request_count](const Envelope& envelope, const HTTPRequest& req) -> Result<Envelope> {
        request_count++;

        Envelope response;
        response.version = "1.0";
        response.from = "server";
        response.to = envelope.from;
        response.op = OperationType::ACK;
        response.msg_id = "Response " + std::to_string(request_count.load());

        return Result<Envelope>::ok(response);
    });

    ASSERT_TRUE(client_->connect().is_ok());

    const int num_requests = 5;
    for (int i = 0; i < num_requests; ++i) {
        Envelope request;
        request.version = "1.0";
        request.from = "client";
        request.to = "server";
        request.op = OperationType::DATA;
        request.msg_id = "Request " + std::to_string(i + 1);

        auto result = client_->send(request);
        ASSERT_TRUE(result.is_ok()) << "Request " << i << " failed: " << result.error();

        auto response = result.unwrap();
        EXPECT_EQ(response.msg_id, "Response " + std::to_string(i + 1));
    }

    EXPECT_EQ(request_count.load(), num_requests);
}

TEST_F(HTTPIntegrationTest, LargeMessage) {
    ASSERT_TRUE(StartServer());

    server_->on_message([](const Envelope& envelope, const HTTPRequest& req) -> Result<Envelope> {
        // Echo back
        Envelope response;
        response.version = "1.0";
        response.from = "server";
        response.to = envelope.from;
        response.op = OperationType::ACK;
        response.msg_id = envelope.msg_id;

        return Result<Envelope>::ok(response);
    });

    ASSERT_TRUE(client_->connect().is_ok());

    // Create large message (100KB of text)
    std::string large_msg(100 * 1024, 'X');

    Envelope request;
    request.version = "1.0";
    request.from = "client";
    request.to = "server";
    request.op = OperationType::DATA;
    request.msg_id = large_msg;

    auto result = client_->send(request);
    ASSERT_TRUE(result.is_ok()) << "Large message send failed: " << result.error();

    auto response = result.unwrap();
    EXPECT_EQ(response.msg_id.size(), large_msg.size());
    EXPECT_EQ(response.msg_id, large_msg);
}

// ============================================================================
// Custom Endpoint Tests
// ============================================================================

TEST_F(HTTPIntegrationTest, CustomEndpoint) {
    ASSERT_TRUE(StartServer());

    std::atomic<bool> custom_handler_called{false};
    server_->on_request("/custom", [&custom_handler_called](const HTTPRequest& req) -> HTTPResponseBuilder {
        custom_handler_called = true;
        HTTPResponseBuilder builder;
        builder.status(200).json("{\"message\": \"custom endpoint\"}");
        return builder;
    });

    ASSERT_TRUE(client_->connect().is_ok());

    auto result = client_->get("/custom");
    ASSERT_TRUE(result.is_ok()) << "Custom endpoint failed: " << result.error();

    auto response = result.unwrap();
    EXPECT_EQ(response.status_code, 200);
    EXPECT_TRUE(response.success);

    std::string body(response.body.begin(), response.body.end());
    EXPECT_NE(body.find("custom endpoint"), std::string::npos);

    EXPECT_TRUE(custom_handler_called);
}

TEST_F(HTTPIntegrationTest, InfoEndpoint) {
    ASSERT_TRUE(StartServer());

    server_->on_request("/info", [this](const HTTPRequest& req) -> HTTPResponseBuilder {
        auto stats = server_->get_statistics();

        std::ostringstream json;
        json << "{"
             << "\"connections\": " << stats.connections_active << ", "
             << "\"requests\": " << stats.requests_handled
             << "}";

        HTTPResponseBuilder builder;
        builder.status(200).json(json.str());
        return builder;
    });

    ASSERT_TRUE(client_->connect().is_ok());

    auto result = client_->get("/info");
    ASSERT_TRUE(result.is_ok());

    auto response = result.unwrap();
    EXPECT_EQ(response.status_code, 200);

    std::string body(response.body.begin(), response.body.end());
    EXPECT_NE(body.find("connections"), std::string::npos);
    EXPECT_NE(body.find("requests"), std::string::npos);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(HTTPIntegrationTest, ServerReturnsErrorWhenNoHandler) {
    ASSERT_TRUE(StartServer());
    // Don't set message handler

    ASSERT_TRUE(client_->connect().is_ok());

    Envelope request;
    request.version = "1.0";
    request.from = "client";
    request.to = "server";
    request.op = OperationType::DATA;
    std::string msg = "test";
    request.payload.assign(msg.begin(), msg.end());

    auto result = client_->send(request);
    // Should get error response from server (501 Not Implemented)
    EXPECT_FALSE(result.is_ok());
}

TEST_F(HTTPIntegrationTest, InvalidEnvelopeReturns400) {
    ASSERT_TRUE(StartServer());

    server_->on_message([](const Envelope& envelope, const HTTPRequest& req) -> Result<Envelope> {
        Envelope response;
        response.version = "1.0";
        response.from = "server";
        response.to = envelope.from;
        response.op = OperationType::ACK;
        return Result<Envelope>::ok(response);
    });

    ASSERT_TRUE(client_->connect().is_ok());

    // Send invalid data
    std::vector<uint8_t> invalid_data = {0xFF, 0xFE, 0xFD, 0xFC};
    auto result = client_->post(invalid_data);

    if (result.is_ok()) {
        auto response = result.unwrap();
        EXPECT_GE(response.status_code, 400); // Should be 4xx or 5xx
    }
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST_F(HTTPIntegrationTest, ServerStatisticsUpdate) {
    ASSERT_TRUE(StartServer());

    server_->on_message([](const Envelope& envelope, const HTTPRequest& req) -> Result<Envelope> {
        Envelope response;
        response.version = "1.0";
        response.from = "server";
        response.to = envelope.from;
        response.op = OperationType::ACK;
        return Result<Envelope>::ok(response);
    });

    auto initial_stats = server_->get_statistics();
    EXPECT_EQ(initial_stats.requests_handled, 0);

    ASSERT_TRUE(client_->connect().is_ok());

    Envelope request;
    request.version = "1.0";
    request.from = "client";
    request.to = "server";
    request.op = OperationType::DATA;
    request.msg_id = "test";

    client_->send(request);

    // Give server time to process
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto final_stats = server_->get_statistics();
    EXPECT_GT(final_stats.requests_handled, 0);
    EXPECT_GT(final_stats.bytes_received, 0);
    EXPECT_GT(final_stats.bytes_sent, 0);
}

TEST_F(HTTPIntegrationTest, ClientStatisticsUpdate) {
    ASSERT_TRUE(StartServer());

    server_->on_message([](const Envelope& envelope, const HTTPRequest& req) -> Result<Envelope> {
        Envelope response;
        response.version = "1.0";
        response.from = "server";
        response.to = envelope.from;
        response.op = OperationType::ACK;
        return Result<Envelope>::ok(response);
    });

    ASSERT_TRUE(client_->connect().is_ok());

    auto initial_stats = client_->get_statistics();
    EXPECT_EQ(initial_stats.requests_sent, 0);

    Envelope request;
    request.version = "1.0";
    request.from = "client";
    request.to = "server";
    request.op = OperationType::DATA;
    std::string msg = "test";
    request.payload.assign(msg.begin(), msg.end());

    client_->send(request);

    auto final_stats = client_->get_statistics();
    EXPECT_GT(final_stats.requests_sent, initial_stats.requests_sent);
    EXPECT_GT(final_stats.bytes_sent, 0);
    EXPECT_GT(final_stats.bytes_received, 0);
}

// ============================================================================
// Concurrent Requests Tests
// ============================================================================

TEST_F(HTTPIntegrationTest, ConcurrentRequests) {
    ASSERT_TRUE(StartServer());

    std::atomic<int> requests_received{0};
    server_->on_message([&requests_received](const Envelope& envelope, const HTTPRequest& req) -> Result<Envelope> {
        requests_received++;

        // Simulate some processing
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        Envelope response;
        response.version = "1.0";
        response.from = "server";
        response.to = envelope.from;
        response.op = OperationType::ACK;
        return Result<Envelope>::ok(response);
    });

    const int num_threads = 5;
    const int requests_per_thread = 3;
    std::vector<std::thread> threads;
    std::atomic<int> successes{0};

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&, t]() {
            HTTPClientConfig config;
            config.base_url = "http://127.0.0.1:" + std::to_string(test_port_);
            config.path = "/umicp";
            config.timeout = std::chrono::milliseconds(10000);

            StreamableHTTPClient thread_client(config);
            if (!thread_client.connect().is_ok()) {
                return;
            }

            for (int i = 0; i < requests_per_thread; ++i) {
                Envelope request;
                request.version = "1.0";
                request.from = "client-" + std::to_string(t);
                request.to = "server";
                request.op = OperationType::DATA;
                std::string msg = "Thread " + std::to_string(t) + " Request " + std::to_string(i);
                request.payload.assign(msg.begin(), msg.end());

                if (thread_client.send(request).is_ok()) {
                    successes++;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(successes.load(), num_threads * requests_per_thread);
    EXPECT_EQ(requests_received.load(), num_threads * requests_per_thread);
}

// ============================================================================
// Keep-Alive Tests
// ============================================================================

TEST_F(HTTPIntegrationTest, KeepAliveConnection) {
    ASSERT_TRUE(StartServer());

    server_->on_message([](const Envelope& envelope, const HTTPRequest& req) -> Result<Envelope> {
        Envelope response;
        response.version = "1.0";
        response.from = "server";
        response.to = envelope.from;
        response.op = OperationType::ACK;
        return Result<Envelope>::ok(response);
    });

    ASSERT_TRUE(client_->connect().is_ok());

    // Send multiple requests on same connection
    for (int i = 0; i < 3; ++i) {
        Envelope request;
        request.version = "1.0";
        request.from = "client";
        request.to = "server";
        request.op = OperationType::DATA;
        std::string msg = "Keep-alive request " + std::to_string(i);
        request.payload.assign(msg.begin(), msg.end());

        auto result = client_->send(request);
        EXPECT_TRUE(result.is_ok()) << "Request " << i << " failed";

        // Connection should remain open
        EXPECT_TRUE(client_->is_connected());
    }
}

// ============================================================================
// Server Shutdown Tests
// ============================================================================

TEST_F(HTTPIntegrationTest, GracefulShutdown) {
    ASSERT_TRUE(StartServer());
    ASSERT_TRUE(client_->connect().is_ok());

    // Server is running
    EXPECT_TRUE(server_->is_running());

    // Stop server
    server_->stop();

    // Server is stopped
    EXPECT_FALSE(server_->is_running());

    // Subsequent requests should fail
    Envelope request;
    request.version = "1.0";
    request.from = "client";
    request.to = "server";
    request.op = OperationType::DATA;

    auto result = client_->send(request);
    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// Main Test Entry Point
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

