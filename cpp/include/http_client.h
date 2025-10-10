/**
 * UMICP StreamableHTTP Client
 * HTTP/1.1 client for UMICP envelope transport
 * Based on libcurl
 */

#ifndef UMICP_HTTP_CLIENT_H
#define UMICP_HTTP_CLIENT_H

#include "umicp_types.h"
#include "envelope.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <mutex>

// Forward declare CURL types
typedef void CURL;

namespace umicp {

/**
 * HTTP client configuration
 */
struct HTTPClientConfig {
    std::string base_url;
    std::string path = "/umicp";

    // Timeouts
    std::chrono::milliseconds timeout = std::chrono::milliseconds(30000);
    std::chrono::milliseconds connect_timeout = std::chrono::milliseconds(10000);

    // Headers
    std::unordered_map<std::string, std::string> headers;

    // SSL/TLS
    bool verify_ssl = true;
    std::string ssl_ca_path;
    std::string ssl_cert_path;
    std::string ssl_key_path;

    // Connection
    bool keep_alive = true;
    size_t max_redirects = 5;

    // Buffer sizes
    size_t buffer_size = 65536; // 64KB
};

/**
 * HTTP response
 */
struct HTTPResponse {
    int status_code;
    std::string status_message;
    std::unordered_map<std::string, std::string> headers;
    std::vector<uint8_t> body;
    bool success;
    std::string error;

    HTTPResponse() : status_code(0), success(false) {}
};

/**
 * StreamableHTTPClient class
 *
 * HTTP client for sending UMICP envelopes over HTTP/1.1
 *
 * Features:
 * - Request/Response pattern
 * - Connection keep-alive
 * - Custom headers
 * - SSL/TLS support
 * - Timeout handling
 * - Error handling
 *
 * Usage:
 * ```cpp
 * HTTPClientConfig config;
 * config.base_url = "http://localhost:9080";
 * config.path = "/umicp";
 *
 * StreamableHTTPClient client(config);
 * client.connect();
 *
 * Envelope envelope = create_envelope(...);
 * auto result = client.send(envelope);
 * if (result.is_ok()) {
 *     auto response_envelope = result.unwrap();
 *     // Process response
 * }
 *
 * client.disconnect();
 * ```
 */
class StreamableHTTPClient {
public:
    /**
     * Constructor
     */
    explicit StreamableHTTPClient(const HTTPClientConfig& config);

    /**
     * Destructor
     */
    ~StreamableHTTPClient();

    // Disable copy
    StreamableHTTPClient(const StreamableHTTPClient&) = delete;
    StreamableHTTPClient& operator=(const StreamableHTTPClient&) = delete;

    /**
     * Connect to server (verify health)
     * @return Success/failure result
     */
    Result<void> connect();

    /**
     * Send envelope and receive response
     * @param envelope Envelope to send
     * @return Response envelope or error
     */
    Result<Envelope> send(const Envelope& envelope);

    /**
     * Send raw data
     * @param data Data to send
     * @return HTTP response or error
     */
    Result<HTTPResponse> post(const std::vector<uint8_t>& data);

    /**
     * GET request
     * @param path Path to request (relative to base_url)
     * @return HTTP response or error
     */
    Result<HTTPResponse> get(const std::string& path = "");

    /**
     * Health check (GET /health)
     * @return Success if server is healthy
     */
    Result<void> health_check();

    /**
     * Disconnect from server
     */
    void disconnect();

    /**
     * Check if connected
     */
    bool is_connected() const;

    /**
     * Get configuration
     */
    const HTTPClientConfig& get_config() const { return config_; }

    /**
     * Set custom header
     */
    void set_header(const std::string& key, const std::string& value);

    /**
     * Remove custom header
     */
    void remove_header(const std::string& key);

    /**
     * Get statistics
     */
    struct Statistics {
        size_t requests_sent;
        size_t responses_received;
        size_t bytes_sent;
        size_t bytes_received;
        size_t errors;
        std::chrono::milliseconds avg_latency;
        std::chrono::system_clock::time_point last_request;
    };

    Statistics get_statistics() const;

    /**
     * Reset statistics
     */
    void reset_statistics();

private:
    HTTPClientConfig config_;
    CURL* curl_;
    bool connected_;
    mutable std::mutex mutex_;

    // Statistics
    Statistics stats_;
    std::vector<std::chrono::milliseconds> latencies_;

    /**
     * Initialize CURL handle
     */
    Result<void> init_curl();

    /**
     * Cleanup CURL handle
     */
    void cleanup_curl();

    /**
     * Perform HTTP request
     */
    Result<HTTPResponse> perform_request(
        const std::string& method,
        const std::string& path,
        const std::vector<uint8_t>& body = {}
    );

    /**
     * Build full URL
     */
    std::string build_url(const std::string& path = "") const;

    /**
     * Update statistics
     */
    void update_stats(size_t bytes_sent, size_t bytes_received,
                     std::chrono::milliseconds latency, bool success);
};

} // namespace umicp

#endif // UMICP_HTTP_CLIENT_H

