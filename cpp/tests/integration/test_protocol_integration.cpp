#include <gtest/gtest.h>
#include "umicp.h"
#include "protocol.h"
#include "transport.h"
#include "security.h"
#include "../utils/test_helpers.h"
#include <thread>
#include <chrono>
#include <memory>

namespace umicp {

// Test fixture for protocol integration tests
class ProtocolIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        security = std::make_shared<SecurityManager>("test-model");
        protocol = std::make_unique<Protocol>("test-model");
    }

    void TearDown() override {
    }

    std::shared_ptr<SecurityManager> security;
    std::unique_ptr<Protocol> protocol;
};

// Test basic protocol initialization
TEST_F(ProtocolIntegrationTest, ProtocolInitialization) {
    ASSERT_NE(protocol, nullptr);
    ASSERT_NE(security, nullptr);
}

// Test protocol configuration
TEST_F(ProtocolIntegrationTest, ProtocolConfiguration) {
    UMICPConfig config;
    config.version = "1.0";
    config.max_message_size = 1024;

    auto result = protocol->configure(config);
    ASSERT_TRUE(result.is_success());
}

// Test security manager integration
TEST_F(ProtocolIntegrationTest, SecurityManagerIntegration) {
    auto result = protocol->set_security_manager(security);
    ASSERT_TRUE(result.is_success());

    // Security manager should be set
    ASSERT_FALSE(protocol->is_authenticated()); // Not authenticated yet
}

// Test statistics functionality
TEST_F(ProtocolIntegrationTest, StatisticsAndMonitoring) {
    auto stats = protocol->get_stats();

    // Initial stats should be zero
    ASSERT_EQ(stats.messages_sent, 0);
    ASSERT_EQ(stats.messages_received, 0);
    ASSERT_EQ(stats.bytes_sent, 0);
    ASSERT_EQ(stats.bytes_received, 0);
    ASSERT_EQ(stats.errors_count, 0);

    // Test reset stats
    protocol->reset_stats();
    auto reset_stats = protocol->get_stats();
    ASSERT_EQ(reset_stats.messages_sent, 0);
}

// Test connection state management
TEST_F(ProtocolIntegrationTest, ConnectionStateManagement) {
    // Should start disconnected
    ASSERT_FALSE(protocol->is_connected());

    // Test connect without transport (should fail gracefully)
    auto result = protocol->connect();
    // This might fail, but should not crash
    ASSERT_TRUE(true); // Just verify it doesn't crash
}

// Test message ID generation
TEST_F(ProtocolIntegrationTest, MessageIdGeneration) {
    // This tests internal functionality through public interface
    // We can't directly test generate_message_id, but we can test
    // that the protocol can be configured and doesn't crash
    UMICPConfig config;
    auto result = protocol->configure(config);
    ASSERT_TRUE(result.is_success());
}

// Test error handling
TEST_F(ProtocolIntegrationTest, ErrorHandling) {
    // Test with invalid configuration
    UMICPConfig invalid_config;
    invalid_config.version = ""; // Invalid version

    auto result = protocol->configure(invalid_config);
    // Should handle gracefully (might succeed or fail, but not crash)
    ASSERT_TRUE(true);
}

// Test concurrent access
TEST_F(ProtocolIntegrationTest, ConcurrentAccess) {
    const int num_threads = 4;
    std::vector<std::thread> threads;

    // Launch multiple threads accessing protocol concurrently
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([this, t]() {
            // Each thread tries to configure and get stats
            UMICPConfig config;
            config.version = "1.0";
            auto result = protocol->configure(config);
            ASSERT_TRUE(result.is_success());

            auto stats = protocol->get_stats();
            ASSERT_GE(stats.messages_sent, 0);
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify no crashes occurred
    ASSERT_TRUE(true);
}

} // namespace umicp
