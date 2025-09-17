/**
 * Multi-Transport Integration Tests
 * Tests for advanced features: multi-transport, topic routing, load balancing, failover, schema registry
 */

#include <gtest/gtest.h>
#include "umicp_types.h"
#include "protocol.h"
#include "transport.h"
#include "schema_registry.h"
#include "../utils/test_helpers.h"

using namespace umicp;
using namespace umicp::testing;

class MultiTransportTest : public UMICPTestFixture {
protected:
    void SetUp() override {
        protocol_ = std::make_unique<Protocol>("test-node");
        UMICPConfig config;
        config.enable_compression = true;
        protocol_->configure(config);

        // Create mock transports
        auto ws_transport = std::make_shared<MockTransport>(TransportConfig{.type = TransportType::WEBSOCKET, .host = "ws.example.com", .port = 8080});
        auto h2_transport = std::make_shared<MockTransport>(TransportConfig{.type = TransportType::HTTP2, .host = "api.example.com", .port = 8443});

        protocol_->add_transport(ws_transport, "websocket-transport");
        protocol_->add_transport(h2_transport, "http2-transport");
    }

    std::unique_ptr<Protocol> protocol_;
};

// ============================================================================
// Multi-Transport Tests
// ============================================================================

TEST_F(MultiTransportTest, AddAndRemoveTransports) {
    // Check initial transports
    auto transport_ids = protocol_->get_transport_ids();
    EXPECT_EQ(transport_ids.size(), 2);
    EXPECT_TRUE(std::find(transport_ids.begin(), transport_ids.end(), "websocket-transport") != transport_ids.end());
    EXPECT_TRUE(std::find(transport_ids.begin(), transport_ids.end(), "http2-transport") != transport_ids.end());

    // Add another transport
    auto new_transport = std::make_shared<MockTransport>(TransportConfig{.type = TransportType::WEBSOCKET, .host = "backup.example.com", .port = 8080});
    auto result = protocol_->add_transport(new_transport, "backup-transport");
    EXPECT_TRUE(result.is_success());

    transport_ids = protocol_->get_transport_ids();
    EXPECT_EQ(transport_ids.size(), 3);

    // Remove transport
    result = protocol_->remove_transport("backup-transport");
    EXPECT_TRUE(result.is_success());

    transport_ids = protocol_->get_transport_ids();
    EXPECT_EQ(transport_ids.size(), 2);
}

// ============================================================================
// Topic-Based Routing Tests
// ============================================================================

TEST_F(MultiTransportTest, TopicSubscriptionAndPublishing) {
    // Subscribe to topics on specific transports
    auto result = protocol_->subscribe_topic("sensor/data", "websocket-transport");
    EXPECT_TRUE(result.is_success());

    result = protocol_->subscribe_topic("user/events", "http2-transport");
    EXPECT_TRUE(result.is_success());

    // Check subscribed topics
    auto topics = protocol_->get_subscribed_topics();
    EXPECT_EQ(topics.size(), 2);
    EXPECT_TRUE(std::find(topics.begin(), topics.end(), "sensor/data") != topics.end());
    EXPECT_TRUE(std::find(topics.begin(), topics.end(), "user/events") != topics.end());

    // Unsubscribe from topic
    result = protocol_->unsubscribe_topic("sensor/data");
    EXPECT_TRUE(result.is_success());

    topics = protocol_->get_subscribed_topics();
    EXPECT_EQ(topics.size(), 1);
    EXPECT_EQ(topics[0], "user/events");
}

// ============================================================================
// Load Balancing Tests
// ============================================================================

TEST_F(MultiTransportTest, LoadBalancingStrategies) {
    // Subscribe both transports to same topic
    protocol_->subscribe_topic("test/topic");

    // Test Round Robin
    auto result = protocol_->set_load_balancing_strategy(LoadBalancingStrategy::ROUND_ROBIN);
    EXPECT_TRUE(result.is_success());
    EXPECT_EQ(protocol_->get_load_balancing_strategy(), LoadBalancingStrategy::ROUND_ROBIN);

    // Test Least Connections
    result = protocol_->set_load_balancing_strategy(LoadBalancingStrategy::LEAST_CONNECTIONS);
    EXPECT_TRUE(result.is_success());
    EXPECT_EQ(protocol_->get_load_balancing_strategy(), LoadBalancingStrategy::LEAST_CONNECTIONS);

    // Test Random
    result = protocol_->set_load_balancing_strategy(LoadBalancingStrategy::RANDOM);
    EXPECT_TRUE(result.is_success());
    EXPECT_EQ(protocol_->get_load_balancing_strategy(), LoadBalancingStrategy::RANDOM);

    // Test Weighted
    result = protocol_->set_load_balancing_strategy(LoadBalancingStrategy::WEIGHTED);
    EXPECT_TRUE(result.is_success());
    EXPECT_EQ(protocol_->get_load_balancing_strategy(), LoadBalancingStrategy::WEIGHTED);
}

// ============================================================================
// Failover Tests
// ============================================================================

TEST_F(MultiTransportTest, FailoverManagement) {
    // Enable failover
    auto result = protocol_->set_failover_enabled(true);
    EXPECT_TRUE(result.is_success());
    EXPECT_TRUE(protocol_->is_failover_enabled());

    // Mark transport as failed
    result = protocol_->mark_transport_failed("websocket-transport");
    EXPECT_TRUE(result.is_success());

    // Check failed transports
    auto failed_ids = protocol_->get_failed_transport_ids();
    EXPECT_EQ(failed_ids.size(), 1);
    EXPECT_EQ(failed_ids[0], "websocket-transport");

    // Check healthy transports
    auto healthy_ids = protocol_->get_healthy_transport_ids();
    EXPECT_EQ(healthy_ids.size(), 1);
    EXPECT_EQ(healthy_ids[0], "http2-transport");

    // Disable failover
    result = protocol_->set_failover_enabled(false);
    EXPECT_TRUE(result.is_success());
    EXPECT_FALSE(protocol_->is_failover_enabled());
}

// ============================================================================
// Schema Registry Tests
// ============================================================================

TEST_F(MultiTransportTest, SchemaRegistryIntegration) {
    auto schema_registry = protocol_->get_schema_registry();
    ASSERT_TRUE(schema_registry != nullptr);

    // Create and register a schema
    SchemaDefinition schema("user-profile", "User Profile", "1.0",
                           SchemaType::JSON_SCHEMA,
                           R"({"type": "object", "properties": {"name": {"type": "string"}, "age": {"type": "number"}}})");

    auto result = schema_registry->register_schema(schema);
    EXPECT_TRUE(result.is_success());

    // Validate message against schema
    ByteBuffer valid_message = {'{', '"', 'n', 'a', 'm', 'e', '"', ':', '"', 'J', 'o', 'h', 'n', '"', ',', '"', 'a', 'g', 'e', '"', ':', '3', '0', '}'} ;
    auto validation_result = schema_registry->validate_message("user-profile", valid_message, "json");
    EXPECT_TRUE(validation_result.valid);

    // Test invalid message
    ByteBuffer invalid_message = {'{', '"', 'n', 'a', 'm', 'e', '"', ':', '1', '2', '3', '}'};
    validation_result = schema_registry->validate_message("user-profile", invalid_message, "json");
    EXPECT_FALSE(validation_result.valid);
}

TEST_F(MultiTransportTest, SchemaValidationInProtocol) {
    // Register schema through protocol
    SchemaDefinition schema("message-schema", "Test Message", "1.0",
                           SchemaType::JSON_SCHEMA,
                           R"({"type": "object", "required": ["type", "data"]})");

    auto result = protocol_->register_message_schema(schema);
    EXPECT_TRUE(result.is_success());

    // Validate message through protocol
    ByteBuffer valid_message = {'{', '"', 't', 'y', 'p', 'e', '"', ':', '"', 't', 'e', 's', 't', '"', ',', '"', 'd', 'a', 't', 'a', '"', ':', '"', 'h', 'e', 'l', 'l', 'o', '"', '}'};
    result = protocol_->validate_message_with_schema("message-schema", valid_message, "json");
    EXPECT_TRUE(result.is_success());
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(MultiTransportTest, TopicPublishingWithLoadBalancing) {
    // Setup: subscribe both transports to topic
    protocol_->subscribe_topic("integration/test");

    // Setup: connect transports
    auto result = protocol_->connect_transport("websocket-transport");
    EXPECT_TRUE(result.is_success());
    result = protocol_->connect_transport("http2-transport");
    EXPECT_TRUE(result.is_success());

    // Test: publish to topic (should use load balancing)
    ByteBuffer test_data = {'t', 'e', 's', 't', ' ', 'm', 'e', 's', 's', 'a', 'g', 'e'};
    result = protocol_->publish_topic("integration/test", test_data);
    EXPECT_TRUE(result.is_success());
}

TEST_F(MultiTransportTest, FailoverWithTopicPublishing) {
    // Setup: subscribe both transports to topic
    protocol_->subscribe_topic("failover/test");
    protocol_->set_failover_enabled(true);

    // Setup: connect one transport, fail the other
    auto result = protocol_->connect_transport("websocket-transport");
    EXPECT_TRUE(result.is_success());
    result = protocol_->mark_transport_failed("http2-transport");
    EXPECT_TRUE(result.is_success());

    // Test: publish should still work with failover
    ByteBuffer test_data = {'f', 'a', 'i', 'l', 'o', 'v', 'e', 'r', ' ', 't', 'e', 's', 't'};
    result = protocol_->publish_topic("failover/test", test_data);
    EXPECT_TRUE(result.is_success());
}

TEST_F(MultiTransportTest, SchemaValidationWithTopicPublishing) {
    // Setup: register schema
    SchemaDefinition schema("topic-schema", "Topic Message", "1.0",
                           SchemaType::JSON_SCHEMA,
                           R"({"type": "object", "required": ["topic", "payload"]})");

    auto result = protocol_->register_message_schema(schema);
    EXPECT_TRUE(result.is_success());

    // Setup: subscribe to topic
    result = protocol_->subscribe_topic("schema/test");
    EXPECT_TRUE(result.is_success());

    // Test: publish valid message
    ByteBuffer valid_message = {'{', '"', 't', 'o', 'p', 'i', 'c', '"', ':', '"', 't', 'e', 's', 't', '"', ',', '"', 'p', 'a', 'y', 'l', 'o', 'a', 'd', '"', ':', '"', 'd', 'a', 't', 'a', '"', '}'};
    result = protocol_->publish_topic("schema/test", valid_message);
    EXPECT_TRUE(result.is_success());
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST_F(MultiTransportTest, MultiTransportPerformance) {
    // Setup: connect both transports
    protocol_->connect_transport("websocket-transport");
    protocol_->connect_transport("http2-transport");

    // Setup: subscribe to multiple topics
    for (int i = 0; i < 10; ++i) {
        protocol_->subscribe_topic("perf/topic" + std::to_string(i));
    }

    // Performance test: publish messages rapidly
    auto start = std::chrono::high_resolution_clock::now();

    const int NUM_MESSAGES = 100;
    for (int i = 0; i < NUM_MESSAGES; ++i) {
        ByteBuffer data = {'m', 'e', 's', 's', 'a', 'g', 'e', ' ', static_cast<uint8_t>('0' + (i % 10))};
        auto result = protocol_->publish_topic("perf/topic" + std::to_string(i % 10), data);
        ASSERT_TRUE(result.is_success());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should handle 100 messages in reasonable time
    EXPECT_LT(duration.count(), 1000); // Less than 1 second
}
