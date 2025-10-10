/**
 * UMICP Advanced WebSocket Client
 * Production-grade WebSocket client with auto-reconnect
 * Based on libwebsockets
 */

#ifndef UMICP_WEBSOCKET_CLIENT_H
#define UMICP_WEBSOCKET_CLIENT_H

#include "umicp_types.h"
#include "event_emitter.h"
#include "transport.h"
#include "envelope.h"
#include <memory>
#include <string>
#include <mutex>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>

namespace umicp {

/**
 * Reconnection configuration
 */
struct ReconnectConfig {
    bool enabled = true;
    size_t max_attempts = 0; // 0 = unlimited
    std::chrono::milliseconds initial_delay = std::chrono::milliseconds(1000);
    std::chrono::milliseconds max_delay = std::chrono::milliseconds(30000);
    double backoff_multiplier = 2.0;
};

/**
 * WebSocket client configuration
 */
struct WebSocketClientConfig {
    std::string url;
    std::string path = "/umicp";

    // Connection
    std::chrono::milliseconds connection_timeout = std::chrono::milliseconds(10000);
    std::chrono::milliseconds handshake_timeout = std::chrono::milliseconds(5000);

    // Reconnection
    ReconnectConfig reconnect;

    // Compression
    bool per_message_deflate = true;

    // Size limits
    size_t max_payload_size = 100 * 1024 * 1024; // 100MB
    size_t rx_buffer_size = 4096;

    // Heartbeat
    std::chrono::milliseconds ping_interval = std::chrono::milliseconds(30000);

    // SSL/TLS
    bool use_ssl = false;
    bool verify_ssl = true;
    std::string ssl_ca_path;

    // Custom headers
    std::unordered_map<std::string, std::string> headers;
};

/**
 * Connection state
 */
enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    RECONNECTING,
    FAILED
};

/**
 * WebSocketClient class
 *
 * High-level WebSocket client with:
 * - Automatic reconnection with exponential backoff
 * - Handshake protocol support
 * - Ping/pong heartbeat
 * - Connection state management
 * - Event-driven architecture
 *
 * Events emitted:
 * - "connect" - Connected to server
 * - "disconnect" - Disconnected from server
 * - "reconnecting" - Attempting to reconnect
 * - "reconnect_failed" - Reconnection failed
 * - "message" - Message received from server
 * - "error" - Error occurred
 */
class WebSocketClient : public EventEmitter, public Transport {
public:
    // Callback types
    using ConnectCallback = std::function<void()>;
    using DisconnectCallback = std::function<void()>;
    using MessageCallback = std::function<void(const ByteBuffer& data)>;
    using ErrorCallback = std::function<void(const std::string& error)>;

    explicit WebSocketClient(const WebSocketClientConfig& config);
    ~WebSocketClient() override;

    // Transport interface implementation
    Result<void> connect() override;
    Result<void> disconnect() override;
    bool is_connected() const override;
    Result<void> send(const ByteBuffer& data) override;
    Result<void> send_envelope(const Envelope& envelope) override;
    Result<void> send_frame(const Frame& frame) override;
    Result<void> configure(const TransportConfig& config) override;
    TransportConfig get_config() const override;
    void set_message_callback(Transport::MessageCallback callback) override;
    void set_connection_callback(Transport::ConnectionCallback callback) override;
    void set_error_callback(Transport::ErrorCallback callback) override;
    TransportStats get_stats() const override;
    void reset_stats() override;
    TransportType get_type() const override { return TransportType::WEBSOCKET; }
    std::string get_endpoint() const override;

    // WebSocket-specific methods
    Result<void> send(const Envelope& envelope);
    ConnectionState get_connection_state() const;
    size_t get_reconnect_attempt() const;
    std::chrono::milliseconds get_next_reconnect_delay() const;

    // Callbacks (alternative to events)
    void set_connect_callback(ConnectCallback callback);
    void set_disconnect_callback(DisconnectCallback callback);
    void set_ws_message_callback(MessageCallback callback);
    void set_ws_error_callback(ErrorCallback callback);

    // Statistics
    struct ClientStats {
        size_t messages_sent;
        size_t messages_received;
        size_t bytes_sent;
        size_t bytes_received;
        size_t reconnect_attempts;
        size_t errors_count;
        std::chrono::system_clock::time_point connected_at;
        std::chrono::system_clock::time_point last_activity;
    };

    ClientStats get_client_stats() const;

    // Configuration
    WebSocketClientConfig get_ws_config() const { return config_; }

private:
    WebSocketClientConfig config_;

    // Connection state
    std::atomic<ConnectionState> state_;
    mutable std::mutex state_mutex_;

    // libwebsockets context (implementation detail)
    struct lws_context* lws_context_;
    struct lws* wsi_; // WebSocket instance
    std::unique_ptr<std::thread> service_thread_;
    std::atomic<bool> should_stop_;

    // Reconnection state
    size_t reconnect_attempts_;
    std::chrono::milliseconds current_reconnect_delay_;
    std::chrono::steady_clock::time_point next_reconnect_time_;

    // Callbacks
    std::mutex callbacks_mutex_;
    ConnectCallback connect_callback_;
    DisconnectCallback disconnect_callback_;
    MessageCallback ws_message_callback_;
    ErrorCallback ws_error_callback_;

    // Transport callbacks
    Transport::MessageCallback transport_message_callback_;
    Transport::ConnectionCallback transport_connection_callback_;
    Transport::ErrorCallback transport_error_callback_;

    // Statistics
    mutable std::mutex stats_mutex_;
    ClientStats stats_;
    TransportStats transport_stats_;

    // Internal methods
    void service_loop();
    Result<void> connect_internal();
    void handle_connected();
    void handle_disconnected();
    void handle_message(const ByteBuffer& data);
    void handle_error(const std::string& error);
    void attempt_reconnect();
    void reset_reconnect_state();
    void update_reconnect_delay();

    // libwebsockets callbacks (static)
    static int callback_websocket(struct lws* wsi, enum lws_callback_reasons reason,
                                   void* user, void* in, size_t len);
};

/**
 * Helper function to create a WebSocket client
 */
inline std::unique_ptr<WebSocketClient> create_websocket_client(
    const std::string& url,
    bool auto_reconnect = true
) {
    WebSocketClientConfig config;
    config.url = url;
    config.reconnect.enabled = auto_reconnect;
    return std::make_unique<WebSocketClient>(config);
}

/**
 * Helper function to create a WebSocket client with custom config
 */
inline std::unique_ptr<WebSocketClient> create_websocket_client_with_config(
    const std::string& url,
    const ReconnectConfig& reconnect_config
) {
    WebSocketClientConfig config;
    config.url = url;
    config.reconnect = reconnect_config;
    return std::make_unique<WebSocketClient>(config);
}

} // namespace umicp

#endif // UMICP_WEBSOCKET_CLIENT_H

