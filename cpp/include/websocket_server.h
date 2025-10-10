/**
 * UMICP Advanced WebSocket Server
 * Production-grade WebSocket server with peer management
 * Based on libwebsockets
 */

#ifndef UMICP_WEBSOCKET_SERVER_H
#define UMICP_WEBSOCKET_SERVER_H

#include "umicp_types.h"
#include "event_emitter.h"
#include "transport.h"
#include "envelope.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <chrono>
#include <functional>
#include <queue>

// Forward declare libwebsockets types
struct lws;
struct lws_context;

namespace umicp {

/**
 * WebSocket server configuration
 */
struct WebSocketServerConfig {
    uint16_t port = 8080;
    std::string path = "/umicp";
    std::string interface_address = "0.0.0.0"; // Listen on all interfaces

    // Compression
    bool per_message_deflate = true;

    // Size limits
    size_t max_payload_size = 100 * 1024 * 1024; // 100MB
    size_t rx_buffer_size = 4096;

    // Timeouts
    std::chrono::milliseconds ping_interval = std::chrono::milliseconds(30000);
    std::chrono::milliseconds connection_timeout = std::chrono::milliseconds(60000);
    std::chrono::milliseconds handshake_timeout = std::chrono::milliseconds(10000);

    // SSL/TLS
    bool use_ssl = false;
    std::string ssl_cert_path;
    std::string ssl_key_path;

    // Limits
    size_t max_connections = 1000;
};

/**
 * WebSocket client connection info
 */
struct WebSocketClientInfo {
    std::string id;
    std::string remote_address;
    uint16_t remote_port;
    std::chrono::system_clock::time_point connected_at;
    std::chrono::system_clock::time_point last_activity;
    size_t messages_sent;
    size_t messages_received;
    size_t bytes_sent;
    size_t bytes_received;
    bool is_connected;

    WebSocketClientInfo(const std::string& client_id)
        : id(client_id),
          remote_port(0),
          connected_at(std::chrono::system_clock::now()),
          last_activity(std::chrono::system_clock::now()),
          messages_sent(0),
          messages_received(0),
          bytes_sent(0),
          bytes_received(0),
          is_connected(true) {}
};

/**
 * WebSocketServer class
 *
 * High-level WebSocket server with:
 * - Multiple client management
 * - Automatic ping/pong heartbeat
 * - Connection timeout handling
 * - Per-message deflate compression
 * - Event-driven architecture
 *
 * Events emitted:
 * - "connection" - New client connected
 * - "disconnection" - Client disconnected
 * - "message" - Message received from client
 * - "error" - Error occurred
 */
class WebSocketServer : public EventEmitter {
public:
    // Callback types
    using ConnectionCallback = std::function<void(const std::string& client_id)>;
    using DisconnectionCallback = std::function<void(const std::string& client_id)>;
    using MessageCallback = std::function<void(const std::string& client_id, const ByteBuffer& data)>;
    using ErrorCallback = std::function<void(const std::string& error, const std::string* client_id)>;

    explicit WebSocketServer(const WebSocketServerConfig& config);
    ~WebSocketServer() override;

    // Server lifecycle
    Result<void> start();
    Result<void> stop();
    bool is_running() const;

    // Message sending
    Result<void> send_to_client(const std::string& client_id, const ByteBuffer& data);
    Result<void> send_to_client(const std::string& client_id, const Envelope& envelope);
    Result<void> broadcast(const ByteBuffer& data, const std::string& exclude_client_id = "");
    Result<void> broadcast(const Envelope& envelope, const std::string& exclude_client_id = "");

    // Client management
    std::vector<WebSocketClientInfo> get_clients() const;
    std::optional<WebSocketClientInfo> get_client(const std::string& client_id) const;
    Result<void> disconnect_client(const std::string& client_id);
    size_t get_client_count() const;

    // Callbacks (alternative to events)
    void set_connection_callback(ConnectionCallback callback);
    void set_disconnection_callback(DisconnectionCallback callback);
    void set_message_callback(MessageCallback callback);
    void set_error_callback(ErrorCallback callback);

    // Statistics
    struct ServerStats {
        size_t total_connections;
        size_t active_connections;
        size_t messages_sent;
        size_t messages_received;
        size_t bytes_sent;
        size_t bytes_received;
        size_t errors_count;
        std::chrono::system_clock::time_point start_time;
    };

    ServerStats get_stats() const;
    void reset_stats();

    // Configuration
    WebSocketServerConfig get_config() const { return config_; }

private:
    WebSocketServerConfig config_;

    // Server state
    bool running_;
    mutable std::mutex state_mutex_;

    // libwebsockets context (implementation detail)
    lws_context* lws_context_;
    std::unique_ptr<std::thread> service_thread_;

    // Client tracking
    mutable std::mutex clients_mutex_;
    std::unordered_map<std::string, std::shared_ptr<WebSocketClientInfo>> clients_;

    // Per-session data mapping (lws* to client_id)
    mutable std::mutex session_mutex_;
    std::unordered_map<lws*, std::string> wsi_to_client_id_;
    std::unordered_map<std::string, lws*> client_id_to_wsi_;

    // Send queues per client
    mutable std::mutex send_queues_mutex_;
    std::unordered_map<std::string, std::queue<ByteBuffer>> send_queues_;

    // Callbacks
    std::mutex callbacks_mutex_;
    ConnectionCallback connection_callback_;
    DisconnectionCallback disconnection_callback_;
    MessageCallback message_callback_;
    ErrorCallback error_callback_;

    // Statistics
    mutable std::mutex stats_mutex_;
    ServerStats stats_;

    // Internal methods
    void service_loop();
    void handle_connection(lws* wsi, const std::string& client_id, const std::string& remote_addr, uint16_t port);
    void handle_disconnection(lws* wsi);
    void handle_message(lws* wsi, const ByteBuffer& data);
    void handle_error(const std::string& error, const std::string* client_id = nullptr);
    void update_client_activity(const std::string& client_id);
    void check_timeouts();
    std::string get_client_id_for_wsi(lws* wsi) const;
    bool process_send_queue(lws* wsi, const std::string& client_id);
    void register_session(lws* wsi, const std::string& client_id);
    void unregister_session(lws* wsi);

    // libwebsockets callbacks (static)
    static int callback_websocket(lws* wsi, int reason,
                                   void* user, void* in, size_t len);
};

/**
 * Helper function to create a WebSocket server
 */
inline std::unique_ptr<WebSocketServer> create_websocket_server(
    uint16_t port,
    const std::string& path = "/umicp"
) {
    WebSocketServerConfig config;
    config.port = port;
    config.path = path;
    return std::make_unique<WebSocketServer>(config);
}

} // namespace umicp

#endif // UMICP_WEBSOCKET_SERVER_H

