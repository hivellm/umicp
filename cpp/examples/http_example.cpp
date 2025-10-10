/**
 * UMICP StreamableHTTP Example
 * Demonstrates HTTP client and server usage
 */

#include "umicp.h"
#include "http_server.h"
#include "http_client.h"
#include "envelope.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace umicp;

// Server thread function
void run_server() {
    std::cout << "Starting HTTP server on port 9080...\n";

    HTTPServerConfig config;
    config.port = 9080;
    config.host = "0.0.0.0";
    config.path = "/umicp";
    config.worker_threads = 4;

    StreamableHTTPServer server(config);

    // Set message handler
    server.on_message([](const Envelope& envelope, const HTTPRequest& req) -> Result<Envelope> {
        std::cout << "Server received envelope:\n";
        std::cout << "  From: " << envelope.from << "\n";
        std::cout << "  To: " << envelope.to << "\n";
        std::cout << "  Operation: " << static_cast<int>(envelope.operation) << "\n";
        std::cout << "  Payload size: " << envelope.payload.size() << " bytes\n";

        // Create response envelope
        Envelope response;
        response.version = 1;
        response.from = "http-server";
        response.to = envelope.from;
        response.operation = OperationType::RESPONSE;
        response.timestamp = std::chrono::system_clock::now();

        // Echo back the payload with a prefix
        std::string response_msg = "Echo: ";
        std::string received_msg(envelope.payload.begin(), envelope.payload.end());
        response_msg += received_msg;
        response.payload.assign(response_msg.begin(), response_msg.end());

        return Result<Envelope>::ok(response);
    });

    // Set custom health check handler
    server.on_request("/health", [](const HTTPRequest& req) -> HTTPResponseBuilder {
        HTTPResponseBuilder builder;
        builder.status(200)
               .json("{\"status\": \"healthy\", \"service\": \"UMICP HTTP Example\"}");
        return builder;
    });

    // Set custom info endpoint
    server.on_request("/info", [&server](const HTTPRequest& req) -> HTTPResponseBuilder {
        auto stats = server.get_statistics();

        std::ostringstream json;
        json << "{"
             << "\"connections\": " << stats.connections_active << ", "
             << "\"requests\": " << stats.requests_handled << ", "
             << "\"bytes_sent\": " << stats.bytes_sent << ", "
             << "\"bytes_received\": " << stats.bytes_received
             << "}";

        HTTPResponseBuilder builder;
        builder.status(200).json(json.str());
        return builder;
    });

    // Start server
    auto result = server.start();
    if (!result.is_ok()) {
        std::cerr << "Failed to start server: " << result.error() << "\n";
        return;
    }

    std::cout << "HTTP server started successfully!\n";
    std::cout << "Listening on http://0.0.0.0:9080\n";
    std::cout << "Endpoints:\n";
    std::cout << "  - POST /umicp - UMICP message endpoint\n";
    std::cout << "  - GET /health - Health check\n";
    std::cout << "  - GET /info - Server statistics\n\n";

    // Keep server running
    while (server.is_running()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Print statistics every 10 seconds
        static int counter = 0;
        if (++counter % 10 == 0) {
            auto stats = server.get_statistics();
            std::cout << "\n=== Server Statistics ===\n";
            std::cout << "Active connections: " << stats.connections_active << "\n";
            std::cout << "Total connections: " << stats.connections_accepted << "\n";
            std::cout << "Requests handled: " << stats.requests_handled << "\n";
            std::cout << "Bytes sent: " << stats.bytes_sent << "\n";
            std::cout << "Bytes received: " << stats.bytes_received << "\n";
            std::cout << "Errors: " << stats.errors << "\n";
            std::cout << "Uptime: " << stats.uptime.count() / 1000 << " seconds\n\n";
        }
    }
}

// Client test function
void run_client() {
    std::cout << "Starting HTTP client...\n";

    // Wait for server to start
    std::this_thread::sleep_for(std::chrono::seconds(2));

    HTTPClientConfig config;
    config.base_url = "http://localhost:9080";
    config.path = "/umicp";
    config.timeout = std::chrono::milliseconds(10000);

    StreamableHTTPClient client(config);

    // Connect to server
    std::cout << "Connecting to server...\n";
    auto connect_result = client.connect();
    if (!connect_result.is_ok()) {
        std::cerr << "Failed to connect: " << connect_result.error() << "\n";
        return;
    }

    std::cout << "Connected to server!\n\n";

    // Send test messages
    for (int i = 1; i <= 5; ++i) {
        std::cout << "Sending message " << i << "...\n";

        // Create envelope
        Envelope envelope;
        envelope.version = 1;
        envelope.from = "http-client";
        envelope.to = "http-server";
        envelope.operation = OperationType::REQUEST;
        envelope.timestamp = std::chrono::system_clock::now();

        std::string message = "Hello from HTTP client! Message #" + std::to_string(i);
        envelope.payload.assign(message.begin(), message.end());

        // Send and receive response
        auto send_result = client.send(envelope);
        if (!send_result.is_ok()) {
            std::cerr << "Failed to send message: " << send_result.error() << "\n";
            continue;
        }

        auto response = send_result.unwrap();
        std::string response_msg(response.payload.begin(), response.payload.end());

        std::cout << "Received response:\n";
        std::cout << "  From: " << response.from << "\n";
        std::cout << "  Message: " << response_msg << "\n\n";

        // Wait between messages
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Test health check
    std::cout << "Testing health check endpoint...\n";
    auto health_result = client.get("/health");
    if (health_result.is_ok()) {
        auto health_response = health_result.unwrap();
        std::string health_body(health_response.body.begin(), health_response.body.end());
        std::cout << "Health check response: " << health_body << "\n\n";
    }

    // Test info endpoint
    std::cout << "Testing info endpoint...\n";
    auto info_result = client.get("/info");
    if (info_result.is_ok()) {
        auto info_response = info_result.unwrap();
        std::string info_body(info_response.body.begin(), info_response.body.end());
        std::cout << "Server info: " << info_body << "\n\n";
    }

    // Print client statistics
    auto stats = client.get_statistics();
    std::cout << "\n=== Client Statistics ===\n";
    std::cout << "Requests sent: " << stats.requests_sent << "\n";
    std::cout << "Responses received: " << stats.responses_received << "\n";
    std::cout << "Bytes sent: " << stats.bytes_sent << "\n";
    std::cout << "Bytes received: " << stats.bytes_received << "\n";
    std::cout << "Average latency: " << stats.avg_latency.count() << " ms\n";
    std::cout << "Errors: " << stats.errors << "\n\n";

    // Disconnect
    client.disconnect();
    std::cout << "Client disconnected.\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "  UMICP StreamableHTTP Example\n";
    std::cout << "===========================================\n\n";

    try {
        // Start server in separate thread
        std::thread server_thread(run_server);

        // Run client in main thread
        run_client();

        // Give server some time to finish
        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::cout << "\nExample completed successfully!\n";
        std::cout << "Press Ctrl+C to stop the server.\n";

        // Keep main thread alive
        server_thread.join();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

