/**
 * UMICP WebSocket Client/Server Example
 * Demonstrates WebSocket communication with auto-reconnect
 */

#include "websocket_server.h"
#include "websocket_client.h"
#include "envelope.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace umicp;

void server_example() {
    std::cout << "=== WebSocket Server Example ===" << std::endl << std::endl;

    // Create server configuration
    WebSocketServerConfig config;
    config.port = 8080;
    config.path = "/umicp";
    config.per_message_deflate = true;
    config.max_connections = 100;

    // Create server
    auto server = std::make_unique<WebSocketServer>(config);
    std::cout << "Server created on port " << config.port << std::endl;

    // Register event handlers
    server->on("connection", [](const Event& event) {
        auto client_id = event.get<std::string>("client_id");
        std::cout << "Client connected: " << client_id << std::endl;
    });

    server->on("disconnection", [](const Event& event) {
        auto client_id = event.get<std::string>("client_id");
        std::cout << "Client disconnected: " << client_id << std::endl;
    });

    server->on("message", [](const Event& event) {
        auto client_id = event.get<std::string>("client_id");
        auto data = event.get<ByteBuffer>("data");
        std::cout << "Received " << data.size() << " bytes from " << client_id << std::endl;
    });

    server->on("error", [](const Event& event) {
        auto error = event.get<std::string>("error");
        std::cout << "Server error: " << error << std::endl;
    });

    // Start server
    std::cout << "Starting server..." << std::endl;
    auto result = server->start();

    if (result.is_ok()) {
        std::cout << "Server started successfully!" << std::endl;
        std::cout << "Listening on ws://0.0.0.0:" << config.port << config.path << std::endl;

        // Server statistics
        auto stats = server->get_stats();
        std::cout << "\nServer Statistics:" << std::endl;
        std::cout << "  Total connections: " << stats.total_connections << std::endl;
        std::cout << "  Active connections: " << stats.active_connections << std::endl;
        std::cout << "  Messages sent: " << stats.messages_sent << std::endl;
        std::cout << "  Messages received: " << stats.messages_received << std::endl;

        // Keep server running for demonstration
        std::cout << "\nServer running for 30 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(30));

        // Stop server
        std::cout << "\nStopping server..." << std::endl;
        server->stop();
        std::cout << "Server stopped" << std::endl;
    } else {
        std::cout << "Failed to start server: " << result.error_message.value() << std::endl;
    }

    std::cout << std::endl;
}

void client_example() {
    std::cout << "=== WebSocket Client Example ===" << std::endl << std::endl;

    // Create client configuration
    WebSocketClientConfig config;
    config.url = "ws://localhost:8080/umicp";
    config.reconnect.enabled = true;
    config.reconnect.initial_delay = std::chrono::milliseconds(1000);
    config.reconnect.max_delay = std::chrono::milliseconds(30000);
    config.reconnect.backoff_multiplier = 2.0;
    config.reconnect.max_attempts = 5;

    // Create client
    auto client = std::make_unique<WebSocketClient>(config);
    std::cout << "Client created" << std::endl;

    // Register event handlers
    client->on("connect", [](const Event& event) {
        std::cout << "Connected to server!" << std::endl;
    });

    client->on("disconnect", [](const Event& event) {
        std::cout << "Disconnected from server" << std::endl;
    });

    client->on("reconnecting", [](const Event& event) {
        auto attempt = event.get<int>("attempt");
        auto delay_ms = event.get<int>("delay_ms");
        std::cout << "Reconnecting... attempt " << attempt
                  << ", delay " << delay_ms << "ms" << std::endl;
    });

    client->on("message", [](const Event& event) {
        auto data = event.get<ByteBuffer>("data");
        std::cout << "Received " << data.size() << " bytes from server" << std::endl;
    });

    client->on("error", [](const Event& event) {
        auto error = event.get<std::string>("error");
        std::cout << "Client error: " << error << std::endl;
    });

    // Connect to server
    std::cout << "Connecting to " << config.url << "..." << std::endl;
    auto result = client->connect();

    if (result.is_ok()) {
        std::cout << "Connection initiated" << std::endl;

        // Wait for connection
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Send a message
        if (client->is_connected()) {
            Envelope env;
            env.msg_id = "msg-001";
            env.version = "1.0";
            env.op = OperationType::DATA;
            env.from = "cpp-client";

            std::cout << "\nSending envelope..." << std::endl;
            auto send_result = client->send(env);
            if (send_result.is_ok()) {
                std::cout << "Message sent successfully" << std::endl;
            }
        }

        // Client statistics
        auto stats = client->get_client_stats();
        std::cout << "\nClient Statistics:" << std::endl;
        std::cout << "  Messages sent: " << stats.messages_sent << std::endl;
        std::cout << "  Messages received: " << stats.messages_received << std::endl;
        std::cout << "  Reconnect attempts: " << stats.reconnect_attempts << std::endl;
        std::cout << "  Current state: " << static_cast<int>(client->get_connection_state()) << std::endl;

        // Keep client running
        std::cout << "\nClient running for 10 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));

        // Disconnect
        std::cout << "\nDisconnecting..." << std::endl;
        client->disconnect();
        std::cout << "Client disconnected" << std::endl;
    } else {
        std::cout << "Failed to connect: " << result.error_message.value() << std::endl;
    }

    std::cout << std::endl;
}

void integrated_example() {
    std::cout << "=== Integrated Client/Server Example ===" << std::endl << std::endl;

    // Start server in a thread
    std::thread server_thread([]() {
        WebSocketServerConfig config;
        config.port = 9090;
        auto server = create_websocket_server(config.port);

        server->on("connection", [](const Event& e) {
            std::cout << "[Server] Client connected" << std::endl;
        });

        server->on("message", [server](const Event& e) {
            auto client_id = e.get<std::string>("client_id");
            auto data = e.get<ByteBuffer>("data");
            std::cout << "[Server] Received message, echoing back..." << std::endl;

            // Echo message back
            server->send_to_client(client_id, data);
        });

        auto result = server->start();
        if (result.is_ok()) {
            std::cout << "[Server] Started on port 9090" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(20));
            server->stop();
            std::cout << "[Server] Stopped" << std::endl;
        }
    });

    // Wait for server to start
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Create client
    WebSocketClientConfig client_config;
    client_config.url = "ws://localhost:9090/umicp";
    auto client = std::make_unique<WebSocketClient>(client_config);

    client->on("connect", [](const Event& e) {
        std::cout << "[Client] Connected!" << std::endl;
    });

    client->on("message", [](const Event& e) {
        auto data = e.get<ByteBuffer>("data");
        std::cout << "[Client] Received echo: " << data.size() << " bytes" << std::endl;
    });

    // Connect and send message
    std::cout << "[Client] Connecting..." << std::endl;
    auto result = client->connect();

    if (result.is_ok()) {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (client->is_connected()) {
            std::cout << "[Client] Sending test message..." << std::endl;
            ByteBuffer test_data = {'H', 'e', 'l', 'l', 'o', '!'};
            client->send(test_data);

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        client->disconnect();
    }

    // Wait for server thread
    server_thread.join();

    std::cout << std::endl;
}

int main() {
    std::cout << "=== UMICP WebSocket Examples ===" << std::endl << std::endl;

    std::cout << "Choose an example to run:" << std::endl;
    std::cout << "1. Server example" << std::endl;
    std::cout << "2. Client example" << std::endl;
    std::cout << "3. Integrated example (recommended)" << std::endl;
    std::cout << std::endl;

    std::cout << "Running integrated example..." << std::endl << std::endl;

    integrated_example();

    std::cout << "=== WebSocket Examples Complete ===" << std::endl;

    return 0;
}

