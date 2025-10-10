/**
 * UMICP StreamableHTTP Server
 * HTTP/1.1 server for UMICP envelope transport
 * Based on native sockets with event loop
 */

#ifndef UMICP_HTTP_SERVER_H
#define UMICP_HTTP_SERVER_H

#include "umicp_types.h"
#include "envelope.h"
#include "event_emitter.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

namespace umicp {

/**
 * HTTP request method
 */
enum class HTTPMethod {
    GET,
    POST,
    PUT,
    DELETE,
    HEAD,
    OPTIONS,
    PATCH,
    UNKNOWN
};

/**
 * HTTP request
 */
struct HTTPRequest {
    HTTPMethod method;
    std::string path;
    std::string query_string;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> query_params;
    std::vector<uint8_t> body;

    // Connection info
    std::string remote_address;
    uint16_t remote_port;
    std::chrono::system_clock::time_point received_at;

    HTTPRequest() : method(HTTPMethod::UNKNOWN), remote_port(0) {}
};

/**
 * HTTP response builder
 */
class HTTPResponseBuilder {
public:
    HTTPResponseBuilder() : status_code_(200) {}

    /**
     * Set status code
     */
    HTTPResponseBuilder& status(int code);

    /**
     * Set header
     */
    HTTPResponseBuilder& header(const std::string& key, const std::string& value);

    /**
     * Set body
     */
    HTTPResponseBuilder& body(const std::vector<uint8_t>& data);
    HTTPResponseBuilder& body(const std::string& data);
    HTTPResponseBuilder& json(const std::string& json_str);

    /**
     * Build response data
     */
    std::vector<uint8_t> build() const;

private:
    int status_code_;
    std::unordered_map<std::string, std::string> headers_;
    std::vector<uint8_t> body_;

    std::string get_status_message(int code) const;
};

/**
 * HTTP client connection info
 */
struct HTTPClientInfo {
    std::string id;
    int socket_fd;
    std::string remote_address;
    uint16_t remote_port;
    std::chrono::system_clock::time_point connected_at;
    std::chrono::system_clock::time_point last_activity;
    size_t requests_handled;
    size_t bytes_received;
    size_t bytes_sent;
    bool keep_alive;

    HTTPClientInfo() : socket_fd(-1), remote_port(0), requests_handled(0),
                       bytes_received(0), bytes_sent(0), keep_alive(true) {}

    HTTPClientInfo(const std::string& client_id, int fd)
        : id(client_id),
          socket_fd(fd),
          remote_port(0),
          connected_at(std::chrono::system_clock::now()),
          last_activity(std::chrono::system_clock::now()),
          requests_handled(0),
          bytes_received(0),
          bytes_sent(0),
          keep_alive(true) {}
};

/**
 * HTTP server configuration
 */
struct HTTPServerConfig {
    uint16_t port = 9080;
    std::string host = "0.0.0.0"; // Listen on all interfaces
    std::string path = "/umicp";

    // Connection limits
    size_t max_connections = 1000;
    size_t max_request_size = 100 * 1024 * 1024; // 100MB
    size_t max_header_size = 8192; // 8KB

    // Timeouts
    std::chrono::milliseconds request_timeout = std::chrono::milliseconds(30000);
    std::chrono::milliseconds keep_alive_timeout = std::chrono::milliseconds(60000);

    // Buffer sizes
    size_t buffer_size = 65536; // 64KB

    // Thread pool
    size_t worker_threads = 4;

    // SSL/TLS (optional)
    bool use_ssl = false;
    std::string ssl_cert_path;
    std::string ssl_key_path;
};

/**
 * Request handler callback
 * @param request HTTP request
 * @return HTTP response
 */
using HTTPRequestHandler = std::function<HTTPResponseBuilder(const HTTPRequest&)>;

/**
 * StreamableHTTPServer class
 *
 * HTTP/1.1 server for receiving UMICP envelopes
 *
 * Features:
 * - Multi-threaded request handling
 * - Keep-alive connections
 * - Request/Response pattern
 * - Health check endpoint
 * - Custom request handlers
 * - Connection management
 * - Statistics tracking
 *
 * Events emitted:
 * - "connection" - New client connected
 * - "disconnection" - Client disconnected
 * - "request" - Request received
 * - "error" - Error occurred
 *
 * Usage:
 * ```cpp
 * HTTPServerConfig config;
 * config.port = 9080;
 * config.path = "/umicp";
 *
 * StreamableHTTPServer server(config);
 *
 * // Set message handler
 * server.on_message([](const Envelope& envelope, const HTTPRequest& req) -> HTTPResponseBuilder {
 *     // Process envelope
 *     Envelope response = process(envelope);
 *
 *     // Build response
 *     HTTPResponseBuilder builder;
 *     builder.status(200)
 *            .header("Content-Type", "application/octet-stream")
 *            .body(serialize_envelope(response));
 *     return builder;
 * });
 *
 * server.start();
 * // Server running...
 * server.stop();
 * ```
 */
class StreamableHTTPServer : public EventEmitter {
public:
    /**
     * Constructor
     */
    explicit StreamableHTTPServer(const HTTPServerConfig& config);

    /**
     * Destructor
     */
    ~StreamableHTTPServer();

    // Disable copy
    StreamableHTTPServer(const StreamableHTTPServer&) = delete;
    StreamableHTTPServer& operator=(const StreamableHTTPServer&) = delete;

    /**
     * Start server
     * @return Success/failure result
     */
    Result<void> start();

    /**
     * Stop server
     */
    void stop();

    /**
     * Check if server is running
     */
    bool is_running() const;

    /**
     * Set message handler for UMICP envelopes
     * Handler receives envelope and should return response envelope
     */
    using MessageHandler = std::function<Result<Envelope>(const Envelope&, const HTTPRequest&)>;
    void on_message(MessageHandler handler);

    /**
     * Set custom request handler for specific path
     * @param path Request path
     * @param handler Handler function
     */
    void on_request(const std::string& path, HTTPRequestHandler handler);

    /**
     * Get connected clients
     */
    std::vector<HTTPClientInfo> get_clients() const;

    /**
     * Get client by ID
     */
    Result<HTTPClientInfo> get_client(const std::string& client_id) const;

    /**
     * Disconnect client
     */
    Result<void> disconnect_client(const std::string& client_id);

    /**
     * Get configuration
     */
    const HTTPServerConfig& get_config() const { return config_; }

    /**
     * Get statistics
     */
    struct Statistics {
        size_t connections_accepted;
        size_t connections_active;
        size_t requests_handled;
        size_t bytes_received;
        size_t bytes_sent;
        size_t errors;
        std::chrono::system_clock::time_point started_at;
        std::chrono::milliseconds uptime;
    };

    Statistics get_statistics() const;

    /**
     * Reset statistics
     */
    void reset_statistics();

private:
    HTTPServerConfig config_;
    int server_socket_;
    std::atomic<bool> running_;
    std::atomic<bool> should_stop_;

    // Threads
    std::thread accept_thread_;
    std::vector<std::thread> worker_threads_;

    // Client management
    std::unordered_map<std::string, HTTPClientInfo> clients_;
    mutable std::mutex clients_mutex_;

    // Request handlers
    MessageHandler message_handler_;
    std::unordered_map<std::string, HTTPRequestHandler> request_handlers_;
    mutable std::mutex handlers_mutex_;

    // Statistics
    Statistics stats_;
    mutable std::mutex stats_mutex_;

    /**
     * Accept loop (runs in accept_thread_)
     */
    void accept_loop();

    /**
     * Worker loop (runs in worker_threads_)
     */
    void worker_loop();

    /**
     * Handle client connection
     */
    void handle_client(int client_fd);

    /**
     * Parse HTTP request
     */
    Result<HTTPRequest> parse_request(int client_fd);

    /**
     * Parse request line
     */
    Result<void> parse_request_line(const std::string& line, HTTPRequest& req);

    /**
     * Parse headers
     */
    Result<void> parse_headers(const std::vector<std::string>& lines, HTTPRequest& req);

    /**
     * Read request body
     */
    Result<void> read_body(int client_fd, HTTPRequest& req, size_t content_length);

    /**
     * Handle request
     */
    HTTPResponseBuilder handle_request(const HTTPRequest& req);

    /**
     * Handle health check
     */
    HTTPResponseBuilder handle_health_check(const HTTPRequest& req);

    /**
     * Handle UMICP message
     */
    HTTPResponseBuilder handle_umicp_message(const HTTPRequest& req);

    /**
     * Send response
     */
    Result<void> send_response(int client_fd, const HTTPResponseBuilder& builder);

    /**
     * Create server socket
     */
    Result<int> create_server_socket();

    /**
     * Close client connection
     */
    void close_client(const std::string& client_id);

    /**
     * Update statistics
     */
    void update_stats(size_t bytes_received, size_t bytes_sent, bool error = false);

    /**
     * Generate client ID
     */
    std::string generate_client_id(int fd, const std::string& remote_addr, uint16_t remote_port);
};

} // namespace umicp

#endif // UMICP_HTTP_SERVER_H

