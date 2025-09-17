/**
 * WebSocket Example using libwebsockets
 * Demonstrates real WebSocket communication
 */

#include "protocol.h"
#include "transport.h"
#include "umicp_types.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace umicp;

int main() {
    std::cout << "🚀 UMICP WebSocket Example with libwebsockets" << std::endl;
    std::cout << "=============================================" << std::endl;

    try {
        // Create protocol instance
        Protocol protocol("websocket-client");

        // Configure protocol
        UMICPConfig config;
        config.max_message_size = 1024 * 1024;  // 1MB
        config.connection_timeout = 5000;
        config.heartbeat_interval = 30000;

        auto config_result = protocol.configure(config);
        if (!config_result.is_success()) {
            std::cerr << "❌ Configuration failed: " << config_result.error_message.value() << std::endl;
            return 1;
        }

        std::cout << "✅ Protocol configured successfully" << std::endl;

        // Create WebSocket transport
        TransportConfig ws_config;
        ws_config.type = TransportType::WEBSOCKET;
        ws_config.host = "localhost";
        ws_config.port = 8080;
        ws_config.path = "/umicp";

        auto transport = TransportFactory::create_websocket(ws_config);
        if (!transport) {
            std::cerr << "❌ Failed to create WebSocket transport" << std::endl;
            return 1;
        }

        std::cout << "✅ WebSocket transport created: " << transport->get_endpoint() << std::endl;

        // Set up callbacks
        transport->set_connection_callback([](bool connected, const std::string& error) {
            if (connected) {
                std::cout << "🔗 WebSocket connected successfully" << std::endl;
            } else {
                std::cout << "🔌 WebSocket disconnected: " << error << std::endl;
            }
        });

        transport->set_message_callback([](const ByteBuffer& data) {
            std::string message(data.begin(), data.end());
            std::cout << "📨 Received message: " << message << std::endl;
        });

        transport->set_error_callback([](ErrorCode code, const std::string& message) {
            std::cerr << "❌ WebSocket error: " << static_cast<int>(code) << " - " << message << std::endl;
        });

        // Set transport in protocol
        auto set_transport_result = protocol.set_transport(std::shared_ptr<Transport>(transport.release()));
        if (!set_transport_result.is_success()) {
            std::cerr << "❌ Failed to set transport: " << set_transport_result.error_message.value() << std::endl;
            return 1;
        }

        // Connect
        std::cout << "🔌 Connecting to WebSocket server..." << std::endl;
        auto connect_result = protocol.connect();
        if (!connect_result.is_success()) {
            std::cerr << "❌ Connection failed: " << connect_result.error_message.value() << std::endl;
            return 1;
        }

        std::cout << "✅ Connected successfully!" << std::endl;

        // Send some test messages
        std::cout << "📤 Sending test messages..." << std::endl;

        // Send control message
        auto control_result = protocol.send_control("server", OperationType::CONTROL, "ping", "test");
        if (control_result.is_success()) {
            std::cout << "✅ Control message sent: " << control_result.value.value() << std::endl;
        } else {
            std::cout << "❌ Control message failed: " << control_result.error_message.value() << std::endl;
        }

        // Send data message
        ByteBuffer test_data = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0x57, 0x6F, 0x72, 0x6C, 0x64}; // "Hello World"
        auto data_result = protocol.send_data("server", test_data);
        if (data_result.is_success()) {
            std::cout << "✅ Data message sent: " << data_result.value.value() << std::endl;
        } else {
            std::cout << "❌ Data message failed: " << data_result.error_message.value() << std::endl;
        }

        // Send ACK message
        auto ack_result = protocol.send_ack("server", "test-message-id");
        if (ack_result.is_success()) {
            std::cout << "✅ ACK message sent: " << ack_result.value.value() << std::endl;
        } else {
            std::cout << "❌ ACK message failed: " << ack_result.error_message.value() << std::endl;
        }

        // Wait for responses
        std::cout << "⏳ Waiting for responses..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Get statistics
        auto stats = transport->get_stats();
        std::cout << "📊 Transport Statistics:" << std::endl;
        std::cout << "  Messages sent: " << stats.messages_sent << std::endl;
        std::cout << "  Messages received: " << stats.messages_received << std::endl;
        std::cout << "  Bytes sent: " << stats.bytes_sent << std::endl;
        std::cout << "  Bytes received: " << stats.bytes_received << std::endl;
        std::cout << "  Connection count: " << stats.connection_count << std::endl;

        // Disconnect
        std::cout << "🔌 Disconnecting..." << std::endl;
        auto disconnect_result = protocol.disconnect();
        if (disconnect_result.is_success()) {
            std::cout << "✅ Disconnected successfully" << std::endl;
        } else {
            std::cout << "❌ Disconnect failed: " << disconnect_result.error_message.value() << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "❌ Exception: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "🎉 WebSocket example completed successfully!" << std::endl;
    return 0;
}
