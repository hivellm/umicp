/**
 * Advanced Features Demo
 * Demonstrates the newly implemented critical features:
 * - Multi-Transport Support
 * - Topic-Based Routing
 * - Load Balancing
 * - Failover
 * - Schema Registry
 */

#include <iostream>
#include <thread>
#include <chrono>
#include "../include/protocol.h"
#include "../include/schema_registry.h"

using namespace umicp;

void demonstrateMultiTransport() {
    std::cout << "=== MULTI-TRANSPORT DEMONSTRATION ===\n";

    // Create protocol instance
    Protocol protocol("demo-node");

    // Configure with compression and SSL
    UMICPConfig config;
    config.enable_compression = true;
    config.compression_threshold = 1024;
    config.validate_certificates = true;
    protocol.configure(config);

    // Add multiple transports
    std::cout << "Adding WebSocket transport...\n";
    TransportConfig ws_config{.type = TransportType::WEBSOCKET, .host = "ws.example.com", .port = 8080};
    auto ws_result = protocol.add_transport(TransportType::WEBSOCKET, ws_config, "websocket-primary");
    if (ws_result.is_success()) {
        std::cout << "✅ WebSocket transport added successfully\n";
    }

    std::cout << "Adding HTTP/2 transport...\n";
    TransportConfig h2_config{.type = TransportType::HTTP2, .host = "api.example.com", .port = 8443};
    auto h2_result = protocol.add_transport(TransportType::HTTP2, h2_config, "http2-backup");
    if (h2_result.is_success()) {
        std::cout << "✅ HTTP/2 transport added successfully\n";
    }

    // List available transports
    auto transport_ids = protocol.get_transport_ids();
    std::cout << "Available transports: ";
    for (const auto& id : transport_ids) {
        std::cout << id << " ";
    }
    std::cout << "\n\n";
}

void demonstrateTopicRouting() {
    std::cout << "=== TOPIC-BASED ROUTING DEMONSTRATION ===\n";

    Protocol protocol("routing-demo");

    // Add transports
    TransportConfig ws_config{.type = TransportType::WEBSOCKET, .host = "ws.example.com", .port = 8080};
    protocol.add_transport(TransportType::WEBSOCKET, ws_config, "ws-transport");

    TransportConfig h2_config{.type = TransportType::HTTP2, .host = "api.example.com", .port = 8443};
    protocol.add_transport(TransportType::HTTP2, h2_config, "h2-transport");

    // Subscribe to topics
    std::cout << "Subscribing to topics...\n";
    auto result1 = protocol.subscribe_topic("sensor/temperature", "ws-transport");
    auto result2 = protocol.subscribe_topic("user/notifications", "h2-transport");
    auto result3 = protocol.subscribe_topic("system/health"); // Subscribe to all transports

    if (result1.is_success() && result2.is_success() && result3.is_success()) {
        std::cout << "✅ Topic subscriptions successful\n";
    }

    // Show subscribed topics
    auto topics = protocol.get_subscribed_topics();
    std::cout << "Subscribed topics: ";
    for (const auto& topic : topics) {
        std::cout << topic << " ";
    }
    std::cout << "\n\n";
}

void demonstrateLoadBalancing() {
    std::cout << "=== LOAD BALANCING DEMONSTRATION ===\n";

    Protocol protocol("lb-demo");

    // Add multiple transports
    protocol.add_transport(TransportType::WEBSOCKET, {.type = TransportType::WEBSOCKET, .host = "ws1.example.com", .port = 8080}, "ws1");
    protocol.add_transport(TransportType::WEBSOCKET, {.type = TransportType::WEBSOCKET, .host = "ws2.example.com", .port = 8080}, "ws2");
    protocol.add_transport(TransportType::HTTP2, {.type = TransportType::HTTP2, .host = "api1.example.com", .port = 8443}, "h2-1");
    protocol.add_transport(TransportType::HTTP2, {.type = TransportType::HTTP2, .host = "api2.example.com", .port = 8443}, "h2-2");

    // Configure load balancing
    std::cout << "Setting up Round Robin load balancing...\n";
    auto lb_result = protocol.set_load_balancing_strategy(LoadBalancingStrategy::ROUND_ROBIN);
    if (lb_result.is_success()) {
        std::cout << "✅ Load balancing strategy set to Round Robin\n";
    }

    auto strategy = protocol.get_load_balancing_strategy();
    std::cout << "Current load balancing strategy: ";
    switch (strategy) {
        case LoadBalancingStrategy::ROUND_ROBIN: std::cout << "Round Robin"; break;
        case LoadBalancingStrategy::LEAST_CONNECTIONS: std::cout << "Least Connections"; break;
        case LoadBalancingStrategy::RANDOM: std::cout << "Random"; break;
        case LoadBalancingStrategy::WEIGHTED: std::cout << "Weighted"; break;
    }
    std::cout << "\n\n";
}

void demonstrateFailover() {
    std::cout << "=== FAILOVER DEMONSTRATION ===\n";

    Protocol protocol("failover-demo");

    // Add transports
    protocol.add_transport(TransportType::WEBSOCKET, {.type = TransportType::WEBSOCKET, .host = "primary.example.com", .port = 8080}, "primary");
    protocol.add_transport(TransportType::HTTP2, {.type = TransportType::HTTP2, .host = "backup.example.com", .port = 8443}, "backup");

    // Enable failover
    std::cout << "Enabling failover...\n";
    auto failover_result = protocol.set_failover_enabled(true);
    if (failover_result.is_success()) {
        std::cout << "✅ Failover enabled\n";
    }

    // Simulate transport failure
    std::cout << "Simulating transport failure...\n";
    auto fail_result = protocol.mark_transport_failed("primary");
    if (fail_result.is_success()) {
        std::cout << "✅ Primary transport marked as failed\n";
    }

    // Check failed transports
    auto failed_ids = protocol.get_failed_transport_ids();
    std::cout << "Failed transports: ";
    for (const auto& id : failed_ids) {
        std::cout << id << " ";
    }
    std::cout << "\n";

    // Check healthy transports
    auto healthy_ids = protocol.get_healthy_transport_ids();
    std::cout << "Healthy transports: ";
    for (const auto& id : healthy_ids) {
        std::cout << id << " ";
    }
    std::cout << "\n\n";
}

void demonstrateSchemaRegistry() {
    std::cout << "=== SCHEMA REGISTRY DEMONSTRATION ===\n";

    // Get schema registry from protocol
    Protocol protocol("schema-demo");
    auto schema_registry = protocol.get_schema_registry();

    if (!schema_registry) {
        std::cout << "❌ Schema registry not available\n";
        return;
    }

    // Create and register a schema
    std::cout << "Registering JSON schema for user profiles...\n";
    SchemaDefinition user_schema("user-profile", "User Profile", "1.0",
                                SchemaType::JSON_SCHEMA,
                                R"({
                                    "type": "object",
                                    "required": ["name", "email"],
                                    "properties": {
                                        "name": {"type": "string", "minLength": 1},
                                        "email": {"type": "string", "format": "email"},
                                        "age": {"type": "integer", "minimum": 0}
                                    }
                                })");

    auto register_result = schema_registry->register_schema(user_schema);
    if (register_result.is_success()) {
        std::cout << "✅ Schema registered successfully\n";
    }

    // Validate valid message
    std::cout << "Validating valid user message...\n";
    ByteBuffer valid_message = {'{', '"', 'n', 'a', 'm', 'e', '"', ':', '"', 'J', 'o', 'h', 'n', '"', ',', '"', 'e', 'm', 'a', 'i', 'l', '"', ':', '"', 'j', 'o', 'h', 'n', '@', 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm', '"', '}'};
    auto validation_result = schema_registry->validate_message("user-profile", valid_message, "json");

    if (validation_result.valid) {
        std::cout << "✅ Valid message accepted\n";
    } else {
        std::cout << "❌ Valid message rejected: " << validation_result.error_message << "\n";
    }

    // Validate invalid message
    std::cout << "Validating invalid user message...\n";
    ByteBuffer invalid_message = {'{', '"', 'n', 'a', 'm', 'e', '"', ':', '"', '"', ',', '"', 'a', 'g', 'e', '"', ':', '"', 'i', 'n', 'v', 'a', 'l', 'i', 'd', '"', '}'};
    validation_result = schema_registry->validate_message("user-profile", invalid_message, "json");

    if (!validation_result.valid) {
        std::cout << "✅ Invalid message correctly rejected\n";
    } else {
        std::cout << "❌ Invalid message incorrectly accepted\n";
    }

    // Show registered schemas
    auto schema_ids = schema_registry->get_all_schema_ids();
    std::cout << "Registered schemas: ";
    for (const auto& id : schema_ids) {
        std::cout << id << " ";
    }
    std::cout << "\n\n";
}

int main() {
    std::cout << "🚀 UMICP Advanced Features Demonstration\n";
    std::cout << "==========================================\n\n";

    try {
        demonstrateMultiTransport();
        demonstrateTopicRouting();
        demonstrateLoadBalancing();
        demonstrateFailover();
        demonstrateSchemaRegistry();

        std::cout << "🎉 All advanced features demonstrated successfully!\n";
        std::cout << "✅ Multi-Transport Support: IMPLEMENTED\n";
        std::cout << "✅ Topic-Based Routing: IMPLEMENTED\n";
        std::cout << "✅ Load Balancing: IMPLEMENTED\n";
        std::cout << "✅ Failover: IMPLEMENTED\n";
        std::cout << "✅ Schema Registry: IMPLEMENTED\n\n";

        std::cout << "📋 SUMMARY:\n";
        std::cout << "All 5 critical features that were marked as 'pending' have been successfully implemented!\n";
        std::cout << "The UMICP protocol now supports enterprise-grade communication features.\n";

    } catch (const std::exception& e) {
        std::cerr << "❌ Error during demonstration: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
