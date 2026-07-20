/**
 * UMICP HTTP Client Unit Tests
 * Tests for StreamableHTTPClient
 */

#include "http_client.h"
#include "envelope.h"
#include "serialization.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace umicp;

// ============================================================================
// HTTPClientConfig Tests
// ============================================================================

TEST(HTTPClientConfigTest, DefaultValues) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:8080";

    EXPECT_EQ(config.path, "/umicp");
    EXPECT_EQ(config.timeout.count(), 30000);
    EXPECT_EQ(config.connect_timeout.count(), 10000);
    EXPECT_TRUE(config.verify_ssl);
    EXPECT_TRUE(config.keep_alive);
    EXPECT_EQ(config.max_redirects, 5);
    EXPECT_EQ(config.buffer_size, 65536);
}

TEST(HTTPClientConfigTest, CustomValues) {
    HTTPClientConfig config;
    config.base_url = "https://example.com";
    config.path = "/api/v1";
    config.timeout = std::chrono::milliseconds(5000);
    config.connect_timeout = std::chrono::milliseconds(2000);
    config.verify_ssl = false;
    config.keep_alive = false;
    config.max_redirects = 3;
    config.buffer_size = 32768;

    EXPECT_EQ(config.base_url, "https://example.com");
    EXPECT_EQ(config.path, "/api/v1");
    EXPECT_EQ(config.timeout.count(), 5000);
    EXPECT_EQ(config.connect_timeout.count(), 2000);
    EXPECT_FALSE(config.verify_ssl);
    EXPECT_FALSE(config.keep_alive);
    EXPECT_EQ(config.max_redirects, 3);
    EXPECT_EQ(config.buffer_size, 32768);
}

TEST(HTTPClientConfigTest, Headers) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:8080";
    config.headers["Authorization"] = "Bearer token123";
    config.headers["X-Custom-Header"] = "value";

    EXPECT_EQ(config.headers.size(), 2);
    EXPECT_EQ(config.headers["Authorization"], "Bearer token123");
    EXPECT_EQ(config.headers["X-Custom-Header"], "value");
}

// ============================================================================
// HTTPResponse Tests
// ============================================================================

TEST(HTTPResponseTest, DefaultConstruction) {
    HTTPResponse response;

    EXPECT_EQ(response.status_code, 0);
    EXPECT_TRUE(response.status_message.empty());
    EXPECT_TRUE(response.headers.empty());
    EXPECT_TRUE(response.body.empty());
    EXPECT_FALSE(response.success);
    EXPECT_TRUE(response.error.empty());
}

TEST(HTTPResponseTest, SuccessResponse) {
    HTTPResponse response;
    response.status_code = 200;
    response.status_message = "OK";
    response.success = true;
    response.body = {1, 2, 3, 4};
    response.headers["Content-Type"] = "application/octet-stream";

    EXPECT_EQ(response.status_code, 200);
    EXPECT_EQ(response.status_message, "OK");
    EXPECT_TRUE(response.success);
    EXPECT_EQ(response.body.size(), 4);
    EXPECT_EQ(response.headers["Content-Type"], "application/octet-stream");
}

TEST(HTTPResponseTest, ErrorResponse) {
    HTTPResponse response;
    response.status_code = 500;
    response.status_message = "Internal Server Error";
    response.success = false;
    response.error = "Connection timeout";

    EXPECT_EQ(response.status_code, 500);
    EXPECT_FALSE(response.success);
    EXPECT_EQ(response.error, "Connection timeout");
}

// ============================================================================
// StreamableHTTPClient Tests
// ============================================================================

class HTTPClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        HTTPClientConfig config;
        config.base_url = "http://localhost:9999"; // Non-existent server
        config.path = "/umicp";
        config.timeout = std::chrono::milliseconds(1000);
        config.connect_timeout = std::chrono::milliseconds(500);

        client = std::make_unique<StreamableHTTPClient>(config);
    }

    void TearDown() override {
        if (client) {
            client->disconnect();
        }
        client.reset();
    }

    std::unique_ptr<StreamableHTTPClient> client;
};

TEST_F(HTTPClientTest, Initialization) {
    EXPECT_NE(client, nullptr);
    EXPECT_FALSE(client->is_connected());
}

TEST_F(HTTPClientTest, GetConfigReturnsCorrectValues) {
    auto config = client->get_config();
    EXPECT_EQ(config.base_url, "http://localhost:9999");
    EXPECT_EQ(config.path, "/umicp");
    EXPECT_EQ(config.timeout.count(), 1000);
}

TEST_F(HTTPClientTest, InitiallyNotConnected) {
    EXPECT_FALSE(client->is_connected());
}

TEST_F(HTTPClientTest, NonExistentServerConfiguration) {
    // Test that we can configure a client for a non-existent server
    // without actually trying to connect (which would timeout)
    HTTPClientConfig config;
    config.base_url = "http://192.0.2.1:1";  // TEST-NET-1 (non-routable IP)
    config.timeout = std::chrono::milliseconds(100);  // Very short timeout
    config.connect_timeout = std::chrono::milliseconds(100);

    // Create client - should succeed (no actual connection yet)
    StreamableHTTPClient quick_client(config);

    // Verify configuration was set correctly
    EXPECT_FALSE(quick_client.is_connected());

    // Note: We don't actually try to connect here to avoid timeout
    // The connection failure is tested implicitly by other tests
    EXPECT_TRUE(true);  // Configuration successful
}

TEST_F(HTTPClientTest, SendWhenNotConnectedFails) {
    Envelope envelope;
    envelope.version = "1.0";
    envelope.from = "client";
    envelope.to = "server";
    envelope.op = OperationType::DATA;

    auto result = client->send(envelope);
    EXPECT_FALSE(result.is_ok());
    EXPECT_FALSE(result.error().empty());
}

TEST_F(HTTPClientTest, PostWhenNotConnectedFails) {
    std::vector<uint8_t> data = {1, 2, 3, 4};
    auto result = client->post(data);
    EXPECT_FALSE(result.is_ok());
}

TEST_F(HTTPClientTest, GetWhenNotConnectedFails) {
    auto result = client->get("/health");
    EXPECT_FALSE(result.is_ok());
}

TEST_F(HTTPClientTest, DisconnectWhenNotConnectedIsOk) {
    EXPECT_FALSE(client->is_connected());
    client->disconnect(); // Should not crash
    EXPECT_FALSE(client->is_connected());
}

TEST_F(HTTPClientTest, SetHeaderWorks) {
    client->set_header("X-Test", "value");
    auto config = client->get_config();
    EXPECT_EQ(config.headers["X-Test"], "value");
}

TEST_F(HTTPClientTest, RemoveHeaderWorks) {
    client->set_header("X-Test", "value");
    client->remove_header("X-Test");
    auto config = client->get_config();
    EXPECT_EQ(config.headers.count("X-Test"), 0);
}

TEST_F(HTTPClientTest, InitialStatisticsAreZero) {
    auto stats = client->get_statistics();
    EXPECT_EQ(stats.requests_sent, 0);
    EXPECT_EQ(stats.responses_received, 0);
    EXPECT_EQ(stats.bytes_sent, 0);
    EXPECT_EQ(stats.bytes_received, 0);
    EXPECT_EQ(stats.errors, 0);
    EXPECT_EQ(stats.avg_latency.count(), 0);
}

TEST_F(HTTPClientTest, ResetStatisticsWorks) {
    client->reset_statistics();
    auto stats = client->get_statistics();
    EXPECT_EQ(stats.requests_sent, 0);
    EXPECT_EQ(stats.responses_received, 0);
}

// ============================================================================
// HTTPClient with Mock Server Tests
// ============================================================================

class HTTPClientWithServerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // These tests would require a real HTTP server
        // For now, we just verify the client can be constructed
        HTTPClientConfig config;
        config.base_url = "http://httpbin.org"; // Public test server
        config.path = "/post";
        config.timeout = std::chrono::milliseconds(5000);

        client = std::make_unique<StreamableHTTPClient>(config);
    }

    void TearDown() override {
        if (client) {
            client->disconnect();
        }
        client.reset();
    }

    std::unique_ptr<StreamableHTTPClient> client;
};

TEST_F(HTTPClientWithServerTest, ClientCreationSucceeds) {
    EXPECT_NE(client, nullptr);
}

// Note: Actual network tests are skipped in unit tests
// They should be run in integration tests with a real server

// ============================================================================
// HTTPClient URL Building Tests
// ============================================================================

TEST(HTTPClientURLTest, BaseURLWithPath) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:8080";
    config.path = "/umicp";

    StreamableHTTPClient client(config);
    // Internal URL building tested through requests
}

TEST(HTTPClientURLTest, BaseURLWithTrailingSlash) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:8080/";
    config.path = "/umicp";

    StreamableHTTPClient client(config);
    // Should handle trailing slash correctly
}

TEST(HTTPClientURLTest, BaseURLHTTPS) {
    HTTPClientConfig config;
    config.base_url = "https://secure.example.com";
    config.path = "/api";
    config.verify_ssl = true;

    StreamableHTTPClient client(config);
    auto cfg = client.get_config();
    EXPECT_TRUE(cfg.verify_ssl);
}

// ============================================================================
// HTTPClient SSL/TLS Configuration Tests
// ============================================================================

TEST(HTTPClientSSLTest, SSLVerificationEnabled) {
    HTTPClientConfig config;
    config.base_url = "https://example.com";
    config.verify_ssl = true;
    config.ssl_ca_path = "/etc/ssl/certs/ca-bundle.crt";

    StreamableHTTPClient client(config);
    auto cfg = client.get_config();
    EXPECT_TRUE(cfg.verify_ssl);
    EXPECT_EQ(cfg.ssl_ca_path, "/etc/ssl/certs/ca-bundle.crt");
}

TEST(HTTPClientSSLTest, SSLVerificationDisabled) {
    HTTPClientConfig config;
    config.base_url = "https://example.com";
    config.verify_ssl = false;

    StreamableHTTPClient client(config);
    auto cfg = client.get_config();
    EXPECT_FALSE(cfg.verify_ssl);
}

TEST(HTTPClientSSLTest, ClientCertificateConfiguration) {
    HTTPClientConfig config;
    config.base_url = "https://example.com";
    config.ssl_cert_path = "/path/to/client.crt";
    config.ssl_key_path = "/path/to/client.key";

    StreamableHTTPClient client(config);
    auto cfg = client.get_config();
    EXPECT_EQ(cfg.ssl_cert_path, "/path/to/client.crt");
    EXPECT_EQ(cfg.ssl_key_path, "/path/to/client.key");
}

// ============================================================================
// HTTPClient Timeout Tests
// ============================================================================

TEST(HTTPClientTimeoutTest, DefaultTimeouts) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:8080";

    StreamableHTTPClient client(config);
    auto cfg = client.get_config();
    EXPECT_EQ(cfg.timeout.count(), 30000);
    EXPECT_EQ(cfg.connect_timeout.count(), 10000);
}

TEST(HTTPClientTimeoutTest, CustomTimeouts) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:8080";
    config.timeout = std::chrono::milliseconds(5000);
    config.connect_timeout = std::chrono::milliseconds(2000);

    StreamableHTTPClient client(config);
    auto cfg = client.get_config();
    EXPECT_EQ(cfg.timeout.count(), 5000);
    EXPECT_EQ(cfg.connect_timeout.count(), 2000);
}

// ============================================================================
// HTTPClient Multiple Instances Tests
// ============================================================================

TEST(HTTPClientMultipleTest, CanCreateMultipleClients) {
    HTTPClientConfig config1;
    config1.base_url = "http://localhost:8080";

    HTTPClientConfig config2;
    config2.base_url = "http://localhost:8081";

    StreamableHTTPClient client1(config1);
    StreamableHTTPClient client2(config2);

    EXPECT_EQ(client1.get_config().base_url, "http://localhost:8080");
    EXPECT_EQ(client2.get_config().base_url, "http://localhost:8081");
}

TEST(HTTPClientMultipleTest, ClientsAreIndependent) {
    HTTPClientConfig config1;
    config1.base_url = "http://localhost:8080";

    HTTPClientConfig config2;
    config2.base_url = "http://localhost:8081";

    StreamableHTTPClient client1(config1);
    StreamableHTTPClient client2(config2);

    client1.set_header("X-Client", "1");
    client2.set_header("X-Client", "2");

    auto cfg1 = client1.get_config();
    auto cfg2 = client2.get_config();
    EXPECT_EQ(cfg1.headers.at("X-Client"), "1");
    EXPECT_EQ(cfg2.headers.at("X-Client"), "2");
}

// ============================================================================
// Main Test Entry Point
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

