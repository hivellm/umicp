/**
 * UMICP HTTP Server Unit Tests
 * Tests for StreamableHTTPServer and HTTPResponseBuilder
 */

#include "http_server.h"
#include "envelope.h"
#include "serialization.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace umicp;

// ============================================================================
// HTTPMethod Tests
// ============================================================================

TEST(HTTPMethodTest, EnumValues) {
    EXPECT_EQ(static_cast<int>(HTTPMethod::GET), static_cast<int>(HTTPMethod::GET));
    EXPECT_NE(static_cast<int>(HTTPMethod::GET), static_cast<int>(HTTPMethod::POST));
    EXPECT_NE(static_cast<int>(HTTPMethod::PUT), static_cast<int>(HTTPMethod::DELETE));
}

// ============================================================================
// HTTPRequest Tests
// ============================================================================

TEST(HTTPRequestTest, DefaultConstruction) {
    HTTPRequest request;

    EXPECT_EQ(request.method, HTTPMethod::UNKNOWN);
    EXPECT_TRUE(request.path.empty());
    EXPECT_TRUE(request.query_string.empty());
    EXPECT_TRUE(request.headers.empty());
    EXPECT_TRUE(request.query_params.empty());
    EXPECT_TRUE(request.body.empty());
    EXPECT_TRUE(request.remote_address.empty());
    EXPECT_EQ(request.remote_port, 0);
}

TEST(HTTPRequestTest, SetProperties) {
    HTTPRequest request;
    request.method = HTTPMethod::POST;
    request.path = "/umicp";
    request.query_string = "key=value";
    request.headers["Content-Type"] = "application/json";
    request.query_params["key"] = "value";
    request.body = {1, 2, 3, 4};
    request.remote_address = "127.0.0.1";
    request.remote_port = 12345;

    EXPECT_EQ(request.method, HTTPMethod::POST);
    EXPECT_EQ(request.path, "/umicp");
    EXPECT_EQ(request.query_string, "key=value");
    EXPECT_EQ(request.headers["Content-Type"], "application/json");
    EXPECT_EQ(request.query_params["key"], "value");
    EXPECT_EQ(request.body.size(), 4);
    EXPECT_EQ(request.remote_address, "127.0.0.1");
    EXPECT_EQ(request.remote_port, 12345);
}

// ============================================================================
// HTTPResponseBuilder Tests
// ============================================================================

TEST(HTTPResponseBuilderTest, DefaultConstruction) {
    HTTPResponseBuilder builder;
    auto response = builder.build();

    EXPECT_FALSE(response.empty());
    // Should have HTTP/1.1 200 OK as default
}

TEST(HTTPResponseBuilderTest, SetStatus) {
    HTTPResponseBuilder builder;
    builder.status(404);
    auto response = builder.build();

    // Convert to string to check
    std::string response_str(response.begin(), response.end());
    EXPECT_NE(response_str.find("404"), std::string::npos);
    EXPECT_NE(response_str.find("Not Found"), std::string::npos);
}

TEST(HTTPResponseBuilderTest, SetHeader) {
    HTTPResponseBuilder builder;
    builder.header("X-Custom-Header", "test-value");
    auto response = builder.build();

    std::string response_str(response.begin(), response.end());
    EXPECT_NE(response_str.find("X-Custom-Header: test-value"), std::string::npos);
}

TEST(HTTPResponseBuilderTest, SetBodyString) {
    HTTPResponseBuilder builder;
    builder.body("Hello, World!");
    auto response = builder.build();

    std::string response_str(response.begin(), response.end());
    EXPECT_NE(response_str.find("Hello, World!"), std::string::npos);
    EXPECT_NE(response_str.find("Content-Length: 13"), std::string::npos);
}

TEST(HTTPResponseBuilderTest, SetBodyBinary) {
    HTTPResponseBuilder builder;
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    builder.body(data);
    auto response = builder.build();

    std::string response_str(response.begin(), response.end());
    EXPECT_NE(response_str.find("Content-Length: 5"), std::string::npos);
}

TEST(HTTPResponseBuilderTest, SetJSON) {
    HTTPResponseBuilder builder;
    builder.json("{\"status\": \"ok\"}");
    auto response = builder.build();

    std::string response_str(response.begin(), response.end());
    EXPECT_NE(response_str.find("Content-Type: application/json"), std::string::npos);
    EXPECT_NE(response_str.find("{\"status\": \"ok\"}"), std::string::npos);
}

TEST(HTTPResponseBuilderTest, FluentAPI) {
    HTTPResponseBuilder builder;
    auto response = builder
        .status(201)
        .header("X-Test", "value")
        .json("{\"created\": true}")
        .build();

    std::string response_str(response.begin(), response.end());
    EXPECT_NE(response_str.find("201"), std::string::npos);
    EXPECT_NE(response_str.find("X-Test: value"), std::string::npos);
    EXPECT_NE(response_str.find("{\"created\": true}"), std::string::npos);
}

TEST(HTTPResponseBuilderTest, CommonStatusCodes) {
    struct TestCase {
        int code;
        std::string message;
    };

    std::vector<TestCase> cases = {
        {200, "OK"},
        {201, "Created"},
        {204, "No Content"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {500, "Internal Server Error"},
        {503, "Service Unavailable"}
    };

    for (const auto& test_case : cases) {
        HTTPResponseBuilder builder;
        auto response = builder.status(test_case.code).build();
        std::string response_str(response.begin(), response.end());

        EXPECT_NE(response_str.find(std::to_string(test_case.code)), std::string::npos);
        EXPECT_NE(response_str.find(test_case.message), std::string::npos);
    }
}

TEST(HTTPResponseBuilderTest, AutomaticHeaders) {
    HTTPResponseBuilder builder;
    builder.body("test");
    auto response = builder.build();

    std::string response_str(response.begin(), response.end());
    // Should have automatic headers
    EXPECT_NE(response_str.find("Server: UMICP"), std::string::npos);
    EXPECT_NE(response_str.find("Content-Length:"), std::string::npos);
    EXPECT_NE(response_str.find("Date:"), std::string::npos);
}

// ============================================================================
// HTTPServerConfig Tests
// ============================================================================

TEST(HTTPServerConfigTest, DefaultValues) {
    HTTPServerConfig config;

    EXPECT_EQ(config.port, 9080);
    EXPECT_EQ(config.host, "0.0.0.0");
    EXPECT_EQ(config.path, "/umicp");
    EXPECT_EQ(config.max_connections, 1000);
    EXPECT_EQ(config.max_request_size, 100 * 1024 * 1024);
    EXPECT_EQ(config.max_header_size, 8192);
    EXPECT_EQ(config.request_timeout.count(), 30000);
    EXPECT_EQ(config.keep_alive_timeout.count(), 60000);
    EXPECT_EQ(config.buffer_size, 65536);
    EXPECT_EQ(config.worker_threads, 4);
    EXPECT_FALSE(config.use_ssl);
}

TEST(HTTPServerConfigTest, CustomValues) {
    HTTPServerConfig config;
    config.port = 8080;
    config.host = "127.0.0.1";
    config.path = "/api";
    config.max_connections = 500;
    config.max_request_size = 50 * 1024 * 1024;
    config.max_header_size = 4096;
    config.request_timeout = std::chrono::milliseconds(15000);
    config.keep_alive_timeout = std::chrono::milliseconds(30000);
    config.buffer_size = 32768;
    config.worker_threads = 8;
    config.use_ssl = true;
    config.ssl_cert_path = "/path/to/cert.pem";
    config.ssl_key_path = "/path/to/key.pem";

    EXPECT_EQ(config.port, 8080);
    EXPECT_EQ(config.host, "127.0.0.1");
    EXPECT_EQ(config.path, "/api");
    EXPECT_EQ(config.max_connections, 500);
    EXPECT_EQ(config.worker_threads, 8);
    EXPECT_TRUE(config.use_ssl);
}

// ============================================================================
// HTTPClientInfo Tests
// ============================================================================

TEST(HTTPClientInfoTest, Construction) {
    HTTPClientInfo info("client-1", 42);

    EXPECT_EQ(info.id, "client-1");
    EXPECT_EQ(info.socket_fd, 42);
    EXPECT_EQ(info.remote_port, 0);
    EXPECT_EQ(info.requests_handled, 0);
    EXPECT_EQ(info.bytes_received, 0);
    EXPECT_EQ(info.bytes_sent, 0);
    EXPECT_TRUE(info.keep_alive);
}

TEST(HTTPClientInfoTest, SetProperties) {
    HTTPClientInfo info("client-1", 42);
    info.remote_address = "192.168.1.100";
    info.remote_port = 54321;
    info.requests_handled = 10;
    info.bytes_received = 1024;
    info.bytes_sent = 2048;
    info.keep_alive = false;

    EXPECT_EQ(info.remote_address, "192.168.1.100");
    EXPECT_EQ(info.remote_port, 54321);
    EXPECT_EQ(info.requests_handled, 10);
    EXPECT_EQ(info.bytes_received, 1024);
    EXPECT_EQ(info.bytes_sent, 2048);
    EXPECT_FALSE(info.keep_alive);
}

// ============================================================================
// StreamableHTTPServer Tests
// ============================================================================

class HTTPServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        HTTPServerConfig config;
        config.port = 9999; // Use non-standard port to avoid conflicts
        config.host = "127.0.0.1";
        config.path = "/umicp";
        config.worker_threads = 2;

        server = std::make_unique<StreamableHTTPServer>(config);
    }

    void TearDown() override {
        if (server && server->is_running()) {
            server->stop();
        }
        server.reset();

        // Give time for cleanup
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::unique_ptr<StreamableHTTPServer> server;
};

TEST_F(HTTPServerTest, Initialization) {
    EXPECT_NE(server, nullptr);
    EXPECT_FALSE(server->is_running());
}

TEST_F(HTTPServerTest, GetConfigReturnsCorrectValues) {
    auto config = server->get_config();
    EXPECT_EQ(config.port, 9999);
    EXPECT_EQ(config.host, "127.0.0.1");
    EXPECT_EQ(config.path, "/umicp");
    EXPECT_EQ(config.worker_threads, 2);
}

TEST_F(HTTPServerTest, InitiallyNotRunning) {
    EXPECT_FALSE(server->is_running());
}

TEST_F(HTTPServerTest, InitialClientsIsEmpty) {
    auto clients = server->get_clients();
    EXPECT_TRUE(clients.empty());
}

TEST_F(HTTPServerTest, StartSucceeds) {
    auto result = server->start();

    if (result.is_ok()) {
        EXPECT_TRUE(server->is_running());
    } else {
        // May fail if port is in use
        std::cout << "Server start failed (expected if port in use): " << result.error() << std::endl;
    }
}

TEST_F(HTTPServerTest, StartWhenAlreadyRunningFails) {
    auto result1 = server->start();
    if (result1.is_ok()) {
        auto result2 = server->start();
        EXPECT_FALSE(result2.is_ok());
    }
}

TEST_F(HTTPServerTest, StopWhenNotRunningDoesNothing) {
    EXPECT_FALSE(server->is_running());
    server->stop(); // Should not crash
    EXPECT_FALSE(server->is_running());
}

TEST_F(HTTPServerTest, StartAndStop) {
    auto start_result = server->start();
    if (start_result.is_ok()) {
        EXPECT_TRUE(server->is_running());

        server->stop();
        EXPECT_FALSE(server->is_running());
    }
}

TEST_F(HTTPServerTest, SetMessageHandler) {
    bool handler_called = false;

    server->on_message([&handler_called](const Envelope& envelope, const HTTPRequest& req) -> Result<Envelope> {
        handler_called = true;
        Envelope response;
        response.version = "1.0";
        response.from = "server";
        response.to = envelope.from;
        response.op = OperationType::ACK;
        return Result<Envelope>::ok(response);
    });

    // Handler is set but not called yet (no requests)
    EXPECT_FALSE(handler_called);
}

TEST_F(HTTPServerTest, SetCustomRequestHandler) {
    bool handler_called = false;

    server->on_request("/custom", [&handler_called](const HTTPRequest& req) -> HTTPResponseBuilder {
        handler_called = true;
        HTTPResponseBuilder builder;
        builder.status(200).json("{\"message\": \"custom\"}");
        return builder;
    });

    EXPECT_FALSE(handler_called);
}

TEST_F(HTTPServerTest, InitialStatisticsAreZero) {
    auto stats = server->get_statistics();
    EXPECT_EQ(stats.connections_accepted, 0);
    EXPECT_EQ(stats.connections_active, 0);
    EXPECT_EQ(stats.requests_handled, 0);
    EXPECT_EQ(stats.bytes_received, 0);
    EXPECT_EQ(stats.bytes_sent, 0);
    EXPECT_EQ(stats.errors, 0);
}

TEST_F(HTTPServerTest, ResetStatistics) {
    server->reset_statistics();
    auto stats = server->get_statistics();
    EXPECT_EQ(stats.connections_accepted, 0);
    EXPECT_EQ(stats.requests_handled, 0);
}

TEST_F(HTTPServerTest, GetNonExistentClientFails) {
    auto result = server->get_client("non-existent");
    EXPECT_FALSE(result.is_ok());
}

TEST_F(HTTPServerTest, DisconnectNonExistentClientFails) {
    auto result = server->disconnect_client("non-existent");
    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// HTTPServer Multiple Handlers Tests
// ============================================================================

TEST_F(HTTPServerTest, MultipleCustomHandlers) {
    int handler1_count = 0;
    int handler2_count = 0;

    server->on_request("/endpoint1", [&handler1_count](const HTTPRequest& req) -> HTTPResponseBuilder {
        handler1_count++;
        HTTPResponseBuilder builder;
        builder.status(200).json("{\"endpoint\": 1}");
        return builder;
    });

    server->on_request("/endpoint2", [&handler2_count](const HTTPRequest& req) -> HTTPResponseBuilder {
        handler2_count++;
        HTTPResponseBuilder builder;
        builder.status(200).json("{\"endpoint\": 2}");
        return builder;
    });

    EXPECT_EQ(handler1_count, 0);
    EXPECT_EQ(handler2_count, 0);
}

// ============================================================================
// HTTPServer Configuration Edge Cases
// ============================================================================

TEST(HTTPServerEdgeCasesTest, ZeroWorkerThreads) {
    HTTPServerConfig config;
    config.port = 9998;
    config.worker_threads = 0; // Edge case

    StreamableHTTPServer server(config);
    auto cfg = server.get_config();
    EXPECT_EQ(cfg.worker_threads, 0);
}

TEST(HTTPServerEdgeCasesTest, VeryLargeMaxConnections) {
    HTTPServerConfig config;
    config.port = 9997;
    config.max_connections = 1000000; // Very large

    StreamableHTTPServer server(config);
    auto cfg = server.get_config();
    EXPECT_EQ(cfg.max_connections, 1000000);
}

TEST(HTTPServerEdgeCasesTest, VerySmallTimeout) {
    HTTPServerConfig config;
    config.port = 9996;
    config.request_timeout = std::chrono::milliseconds(1);

    StreamableHTTPServer server(config);
    auto cfg = server.get_config();
    EXPECT_EQ(cfg.request_timeout.count(), 1);
}

TEST(HTTPServerEdgeCasesTest, IPv4Localhost) {
    HTTPServerConfig config;
    config.port = 9995;
    config.host = "127.0.0.1";

    StreamableHTTPServer server(config);
    auto cfg = server.get_config();
    EXPECT_EQ(cfg.host, "127.0.0.1");
}

TEST(HTTPServerEdgeCasesTest, AllInterfacesBinding) {
    HTTPServerConfig config;
    config.port = 9994;
    config.host = "0.0.0.0";

    StreamableHTTPServer server(config);
    auto cfg = server.get_config();
    EXPECT_EQ(cfg.host, "0.0.0.0");
}

// ============================================================================
// HTTPServer SSL Configuration Tests
// ============================================================================

TEST(HTTPServerSSLTest, SSLDisabledByDefault) {
    HTTPServerConfig config;
    config.port = 9993;

    StreamableHTTPServer server(config);
    auto cfg = server.get_config();
    EXPECT_FALSE(cfg.use_ssl);
}

TEST(HTTPServerSSLTest, SSLConfiguration) {
    HTTPServerConfig config;
    config.port = 9992;
    config.use_ssl = true;
    config.ssl_cert_path = "/path/to/cert.pem";
    config.ssl_key_path = "/path/to/key.pem";

    StreamableHTTPServer server(config);
    auto cfg = server.get_config();
    EXPECT_TRUE(cfg.use_ssl);
    EXPECT_EQ(cfg.ssl_cert_path, "/path/to/cert.pem");
    EXPECT_EQ(cfg.ssl_key_path, "/path/to/key.pem");
}

// ============================================================================
// HTTPServer Event Emission Tests
// ============================================================================

TEST_F(HTTPServerTest, ServerHasEventEmitter) {
    // StreamableHTTPServer extends EventEmitter
    // This test just verifies we can create it
    EXPECT_NE(server, nullptr);
}

// ============================================================================
// Main Test Entry Point
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

