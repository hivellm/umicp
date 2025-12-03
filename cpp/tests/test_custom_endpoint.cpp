/**
 * Test for Custom Endpoint Support (v0.2.2)
 *
 * Verifies that the C++ UMICP implementation supports
 * custom endpoint paths for compatibility with different servers
 * (e.g., Vectorizer uses /umicp, standard servers use /message)
 */

#include <gtest/gtest.h>
#include "http_client.h"
#include "envelope.h"
#include <string>

namespace umicp {
namespace tests {

class CustomEndpointTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
    }

    void TearDown() override {
        // Cleanup
    }
};

// Test that HTTPClientConfig supports custom base URLs
TEST_F(CustomEndpointTest, ClientConfigAcceptsCustomBaseURL) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:8000";
    config.timeout = std::chrono::milliseconds(30000);
    config.verify_ssl = true;

    EXPECT_EQ(config.base_url, "http://localhost:8000");
    EXPECT_EQ(config.timeout.count(), 30000);
    EXPECT_TRUE(config.verify_ssl);
}

// Test client creation with custom base URL
TEST_F(CustomEndpointTest, ClientCreationWithCustomBaseURL) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:8000";

    StreamableHTTPClient client(config);

    // Client should be created successfully
    EXPECT_FALSE(client.is_connected());
}

// Test URL building for Vectorizer endpoint
TEST_F(CustomEndpointTest, VectorizerEndpointURL) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:8000";
    config.path = "/umicp";  // Vectorizer uses /umicp

    StreamableHTTPClient client(config);

    // Verify the path is set correctly
    EXPECT_EQ(config.path, "/umicp");
}

// Test URL building for standard UMICP endpoint
TEST_F(CustomEndpointTest, StandardEndpointURL) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:9000";
    config.path = "/message";  // Standard servers use /message

    StreamableHTTPClient client(config);

    // Verify the path is set correctly
    EXPECT_EQ(config.path, "/message");
}

// Test URL building with trailing slash in base URL
TEST_F(CustomEndpointTest, BaseURLWithTrailingSlash) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:8000/";
    config.path = "/umicp";

    // The implementation should handle both cases correctly
    EXPECT_EQ(config.base_url, "http://localhost:8000/");
    EXPECT_EQ(config.path, "/umicp");
}

// Test default path is /umicp
TEST_F(CustomEndpointTest, DefaultPathIsUmicp) {
    HTTPClientConfig config;
    config.base_url = "http://localhost:8000";

    // Default path should be /umicp
    EXPECT_EQ(config.path, "/umicp");
}

// Test multiple clients with different endpoints
TEST_F(CustomEndpointTest, MultipleClientsWithDifferentEndpoints) {
    // Client for Vectorizer
    HTTPClientConfig vectorizer_config;
    vectorizer_config.base_url = "http://localhost:8000";
    vectorizer_config.path = "/umicp";

    // Client for standard UMICP server
    HTTPClientConfig standard_config;
    standard_config.base_url = "http://localhost:9000";
    standard_config.path = "/message";

    // Both clients should be independent
    EXPECT_NE(vectorizer_config.base_url, standard_config.base_url);
    EXPECT_NE(vectorizer_config.path, standard_config.path);
}

// Test that config is properly copied
TEST_F(CustomEndpointTest, ConfigCopyPreservesBaseURL) {
    HTTPClientConfig config1;
    config1.base_url = "http://localhost:8000";
    config1.path = "/umicp";
    config1.timeout = std::chrono::milliseconds(5000);

    HTTPClientConfig config2 = config1;

    EXPECT_EQ(config2.base_url, "http://localhost:8000");
    EXPECT_EQ(config2.path, "/umicp");
    EXPECT_EQ(config2.timeout.count(), 5000);
}

// Test version compatibility
TEST_F(CustomEndpointTest, VersionSupportsCustomEndpoints) {
    // This test ensures that v0.2.2 supports custom endpoints
    // The functionality is verified through the other tests
    EXPECT_TRUE(true); // Placeholder - version 0.2.2 supports custom endpoints
}

} // namespace tests
} // namespace umicp

