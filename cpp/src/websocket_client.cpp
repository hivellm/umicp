/**
 * UMICP WebSocket Client Implementation
 * Complete libwebsockets integration
 */

#include <libwebsockets.h>
#include "websocket_client.h"
#include "umicp_types.h"
#include "envelope.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace umicp {

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Parse WebSocket URL into components
 * Format: ws://host:port/path or wss://host:port/path
 */
struct ParsedURL {
    std::string host;
    int port;
    std::string path;
    bool use_ssl;
};

static ParsedURL parse_websocket_url(const std::string& url) {
    ParsedURL result;
    result.host = "localhost";
    result.port = 8080;
    result.path = "/";
    result.use_ssl = false;

    size_t pos = 0;

    // Check protocol
    if (url.find("wss://") == 0) {
        result.use_ssl = true;
        result.port = 443; // Default WSS port
        pos = 6; // Skip "wss://"
    } else if (url.find("ws://") == 0) {
        result.use_ssl = false;
        result.port = 80; // Default WS port
        pos = 5; // Skip "ws://"
    } else {
        // Assume ws:// if no protocol specified
        pos = 0;
    }

    // Find end of host:port section (before path)
    size_t path_pos = url.find('/', pos);
    std::string host_port = (path_pos != std::string::npos)
        ? url.substr(pos, path_pos - pos)
        : url.substr(pos);

    // Extract path
    if (path_pos != std::string::npos) {
        result.path = url.substr(path_pos);
    }

    // Parse host:port
    size_t colon_pos = host_port.find(':');
    if (colon_pos != std::string::npos) {
        result.host = host_port.substr(0, colon_pos);
        try {
            result.port = std::stoi(host_port.substr(colon_pos + 1));
        } catch (...) {
            // Keep default port
        }
    } else {
        result.host = host_port;
    }

    return result;
}

// ============================================================================
// WebSocketClient Implementation
// ============================================================================

WebSocketClient::WebSocketClient(const WebSocketClientConfig& config)
    : EventEmitter(),
      config_(config),
      state_(ConnectionState::DISCONNECTED),
      lws_context_(nullptr),
      wsi_(nullptr),
      should_stop_(false),
      reconnect_attempts_(0),
      current_reconnect_delay_(config.reconnect.initial_delay) {

    // Initialize statistics
    stats_ = ClientStats{};
    transport_stats_ = TransportStats{};
}

WebSocketClient::~WebSocketClient() {
    if (state_ != ConnectionState::DISCONNECTED) {
        disconnect();
    }
}

// ============================================================================
// Transport Interface Implementation
// ============================================================================

Result<void> WebSocketClient::connect() {
    return connect_internal();
}

Result<void> WebSocketClient::disconnect() {
    {
        std::lock_guard<std::mutex> lock(state_mutex_);

        if (state_ == ConnectionState::DISCONNECTED) {
            return Result<void>::ok();
        }

        should_stop_ = true;
        state_ = ConnectionState::DISCONNECTED;
    }

    // Close libwebsockets connection
    if (wsi_) {
        lws_callback_on_writable(wsi_);
        wsi_ = nullptr;
    }

    // Cancel service
    if (lws_context_) {
        lws_cancel_service(lws_context_);
    }

    // Stop service thread
    if (service_thread_ && service_thread_->joinable()) {
        service_thread_->join();
    }

    // Destroy context
    if (lws_context_) {
        lws_context_destroy(lws_context_);
        lws_context_ = nullptr;
    }

    reset_reconnect_state();

    // Emit event
    Event event("disconnect");
    emit("disconnect", event);

    // Call callbacks
    {
        std::lock_guard<std::mutex> callbacks_lock(callbacks_mutex_);
        if (disconnect_callback_) {
            disconnect_callback_();
        }
        if (transport_connection_callback_) {
            transport_connection_callback_(false, "");
        }
    }

    return Result<void>::ok();
}

bool WebSocketClient::is_connected() const {
    return state_.load() == ConnectionState::CONNECTED;
}

Result<void> WebSocketClient::send(const ByteBuffer& data) {
    if (!is_connected()) {
        return Result<void>::error(ErrorCode::NOT_CONNECTED, "Client not connected");
    }

    if (!wsi_) {
        return Result<void>::error(ErrorCode::NOT_CONNECTED, "No active connection");
    }

    // Add to send queue
    {
        std::lock_guard<std::mutex> lock(send_queue_mutex_);
        send_queue_.push(data);
    }

    // Request write callback
    if (wsi_) {
        lws_callback_on_writable(wsi_);
    }

    return Result<void>::ok();
}

bool WebSocketClient::process_send_queue() {
    if (!wsi_ || !is_connected()) {
        return false;
    }

    ByteBuffer data_to_send;

    {
        std::lock_guard<std::mutex> lock(send_queue_mutex_);
        if (send_queue_.empty()) {
            return false;
        }
        data_to_send = send_queue_.front();
        send_queue_.pop();
    }

    // Prepare buffer with LWS_PRE bytes padding
    const size_t pre_padding = LWS_PRE;
    std::vector<uint8_t> padded_buffer(pre_padding + data_to_send.size());
    std::copy(data_to_send.begin(), data_to_send.end(), padded_buffer.begin() + pre_padding);

    // Send via libwebsockets
    int written = lws_write(
        wsi_,
        padded_buffer.data() + pre_padding,
        data_to_send.size(),
        LWS_WRITE_BINARY
    );

    if (written < 0) {
        handle_error("Failed to send data");
        return false;
    }

    // Update statistics
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.messages_sent++;
        stats_.bytes_sent += data_to_send.size();
        transport_stats_.messages_sent++;
        transport_stats_.bytes_sent += data_to_send.size();
    }

    // Return true if there are more messages to send
    std::lock_guard<std::mutex> lock(send_queue_mutex_);
    return !send_queue_.empty();
}

Result<void> WebSocketClient::send_envelope(const Envelope& envelope) {
    auto json = envelope.to_json();
    ByteBuffer buffer(json.begin(), json.end());
    return send(buffer);
}

Result<void> WebSocketClient::send_frame(const Frame& frame) {
    // Serialize frame to binary
    // Note: Frame serialization should be implemented in frame.cpp
    ByteBuffer buffer;
    // TODO: Call frame.serialize() when available
    return send(buffer);
}

Result<void> WebSocketClient::configure(const TransportConfig& config) {
    return Result<void>::error(
        ErrorCode::NOT_IMPLEMENTED,
        "Configure not implemented for WebSocketClient"
    );
}

TransportConfig WebSocketClient::get_config() const {
    TransportConfig config;
    // Fill transport config fields
    return config;
}

void WebSocketClient::set_message_callback(umicp::MessageCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    transport_message_callback_ = std::move(callback);
}

void WebSocketClient::set_connection_callback(umicp::ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    transport_connection_callback_ = std::move(callback);
}

void WebSocketClient::set_error_callback(umicp::ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    transport_error_callback_ = std::move(callback);
}

TransportStats WebSocketClient::get_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return transport_stats_;
}

void WebSocketClient::reset_stats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = ClientStats{};
    transport_stats_ = TransportStats{};
}

std::string WebSocketClient::get_endpoint() const {
    return config_.url;
}

// ============================================================================
// WebSocket-Specific Methods
// ============================================================================

Result<void> WebSocketClient::send(const Envelope& envelope) {
    return send_envelope(envelope);
}

ConnectionState WebSocketClient::get_connection_state() const {
    return state_.load();
}

size_t WebSocketClient::get_reconnect_attempt() const {
    return reconnect_attempts_;
}

std::chrono::milliseconds WebSocketClient::get_next_reconnect_delay() const {
    return current_reconnect_delay_;
}

// ============================================================================
// Callbacks
// ============================================================================

void WebSocketClient::set_connect_callback(ConnectCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    connect_callback_ = std::move(callback);
}

void WebSocketClient::set_disconnect_callback(DisconnectCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    disconnect_callback_ = std::move(callback);
}

void WebSocketClient::set_ws_message_callback(MessageCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    ws_message_callback_ = std::move(callback);
}

void WebSocketClient::set_ws_error_callback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    ws_error_callback_ = std::move(callback);
}

// ============================================================================
// Statistics
// ============================================================================

WebSocketClient::ClientStats WebSocketClient::get_client_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

// ============================================================================
// Internal Methods
// ============================================================================

void WebSocketClient::service_loop() {
    while (!should_stop_) {
        // Check reconnection
        if (state_ == ConnectionState::RECONNECTING) {
            if (std::chrono::steady_clock::now() >= next_reconnect_time_) {
                attempt_reconnect();
            }
        }

        // Service libwebsockets
        if (lws_context_) {
            lws_service(lws_context_, 50);
        }

        // Small sleep to prevent CPU spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

Result<void> WebSocketClient::connect_internal() {
    {
        std::lock_guard<std::mutex> lock(state_mutex_);

        if (state_ == ConnectionState::CONNECTED || state_ == ConnectionState::CONNECTING) {
            return Result<void>::error(ErrorCode::INVALID_STATE, "Already connected or connecting");
        }

        state_ = ConnectionState::CONNECTING;
    }

    // Configure protocols
    static struct lws_protocols protocols[] = {
        {
            "umicp",
            reinterpret_cast<lws_callback_function*>(callback_websocket),
            0,
            0, // rx_buffer_size set via info
            0,
            nullptr,
            0
        },
        { nullptr, nullptr, 0, 0, 0, nullptr, 0 } // Terminator
    };

    // Create context info
    struct lws_context_creation_info info;
    std::memset(&info, 0, sizeof(info));

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.user = this;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.max_http_header_data = config_.rx_buffer_size;

    // Create libwebsockets context
    lws_context_ = lws_create_context(&info);
    if (!lws_context_) {
        state_ = ConnectionState::FAILED;
        return Result<void>::error(
            ErrorCode::INITIALIZATION_FAILED,
            "Failed to create libwebsockets context"
        );
    }

    // Parse URL
    ParsedURL parsed = parse_websocket_url(config_.url);

    // Configure connection
    struct lws_client_connect_info ccinfo;
    std::memset(&ccinfo, 0, sizeof(ccinfo));

    ccinfo.context = lws_context_;
    ccinfo.address = parsed.host.c_str();
    ccinfo.port = parsed.port;
    ccinfo.path = parsed.path.c_str();
    ccinfo.host = parsed.host.c_str();
    ccinfo.origin = parsed.host.c_str();
    ccinfo.protocol = "umicp";
    ccinfo.ietf_version_or_minus_one = -1;
    ccinfo.userdata = this;

    // Enable SSL if needed
    if (parsed.use_ssl || config_.use_ssl) {
        ccinfo.ssl_connection = LCCSCF_USE_SSL;

        if (!config_.verify_ssl) {
            ccinfo.ssl_connection |= LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
        }

        if (!config_.ssl_ca_path.empty()) {
            ccinfo.ssl_connection |= LCCSCF_USE_SSL;
            // Note: libwebsockets uses context-level CA path, set via context creation
        }
    }

    // Initiate connection
    wsi_ = lws_client_connect_via_info(&ccinfo);
    if (!wsi_) {
        state_ = ConnectionState::FAILED;
        lws_context_destroy(lws_context_);
        lws_context_ = nullptr;
        return Result<void>::error(
            ErrorCode::CONNECTION_FAILED,
            "Failed to initiate connection"
        );
    }

    // Start service thread
    should_stop_ = false;
    service_thread_ = std::make_unique<std::thread>([this]() {
        service_loop();
    });

    return Result<void>::ok();
}

void WebSocketClient::handle_connected() {
    state_ = ConnectionState::CONNECTED;
    reset_reconnect_state();

    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.connected_at = std::chrono::system_clock::now();
        stats_.last_activity = std::chrono::system_clock::now();
        transport_stats_.connection_count++;
    }

    // Emit event
    Event event("connect");
    emit("connect", event);

    // Call callbacks
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        if (connect_callback_) {
            connect_callback_();
        }
        if (transport_connection_callback_) {
            transport_connection_callback_(true, "");
        }
    }
}

void WebSocketClient::handle_disconnected() {
    auto previous_state = state_.exchange(ConnectionState::DISCONNECTED);

    if (previous_state == ConnectionState::DISCONNECTED) {
        return; // Already disconnected
    }

    // Check if we should reconnect
    if (config_.reconnect.enabled && !should_stop_) {
        state_ = ConnectionState::RECONNECTING;
        update_reconnect_delay();
        next_reconnect_time_ = std::chrono::steady_clock::now() + current_reconnect_delay_;

        // Emit event
        Event event("reconnecting");
        event.set("attempt", static_cast<int>(reconnect_attempts_ + 1));
        event.set("delay_ms", static_cast<int>(current_reconnect_delay_.count()));
        emit("reconnecting", event);
    } else {
        // Emit disconnect event
        Event event("disconnect");
        emit("disconnect", event);

        // Call callbacks
        {
            std::lock_guard<std::mutex> lock(callbacks_mutex_);
            if (disconnect_callback_) {
                disconnect_callback_();
            }
            if (transport_connection_callback_) {
                transport_connection_callback_(false, "");
            }
        }
    }
}

void WebSocketClient::handle_message(const ByteBuffer& data) {
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.messages_received++;
        stats_.bytes_received += data.size();
        stats_.last_activity = std::chrono::system_clock::now();
        transport_stats_.messages_received++;
        transport_stats_.bytes_received += data.size();
    }

    // Emit event
    Event event("message");
    event.set("data", data);
    emit("message", event);

    // Call callbacks
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        if (ws_message_callback_) {
            ws_message_callback_(data);
        }
        if (transport_message_callback_) {
            transport_message_callback_(data);
        }
    }
}

void WebSocketClient::handle_error(const std::string& error) {
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.errors_count++;
    }

    // Emit event
    Event event("error");
    event.set("error", error);
    emit("error", event);

    // Call callbacks
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        if (ws_error_callback_) {
            ws_error_callback_(error);
        }
        if (transport_error_callback_) {
            transport_error_callback_(ErrorCode::PROTOCOL_ERROR, error);
        }
    }
}

void WebSocketClient::attempt_reconnect() {
    reconnect_attempts_++;

    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.reconnect_attempts++;
    }

    // Check max attempts
    if (config_.reconnect.max_attempts > 0 &&
        reconnect_attempts_ >= config_.reconnect.max_attempts) {

        state_ = ConnectionState::FAILED;

        // Emit event
        Event event("reconnect_failed");
        event.set("attempts", static_cast<int>(reconnect_attempts_));
        emit("reconnect_failed", event);

        return;
    }

    // Try to connect
    auto result = connect_internal();

    if (!result.is_ok()) {
        // Schedule next retry
        update_reconnect_delay();
        next_reconnect_time_ = std::chrono::steady_clock::now() + current_reconnect_delay_;
    }
}

void WebSocketClient::reset_reconnect_state() {
    reconnect_attempts_ = 0;
    current_reconnect_delay_ = config_.reconnect.initial_delay;
}

void WebSocketClient::update_reconnect_delay() {
    // Exponential backoff
    current_reconnect_delay_ = std::chrono::milliseconds(
        static_cast<long long>(
            current_reconnect_delay_.count() * config_.reconnect.backoff_multiplier
        )
    );

    // Cap at max delay
    if (current_reconnect_delay_ > config_.reconnect.max_delay) {
        current_reconnect_delay_ = config_.reconnect.max_delay;
    }
}

// ============================================================================
// libwebsockets Callback (Static)
// ============================================================================

int WebSocketClient::callback_websocket(
    lws* wsi,
    int reason,
    void* user,
    void* in,
    size_t len
) {
    // Get client instance from context
    lws_context* context = lws_get_context(wsi);
    WebSocketClient* client = static_cast<WebSocketClient*>(
        lws_context_user(context)
    );

    if (!client) {
        return -1;
    }

    // Cast to enum for switch
    enum lws_callback_reasons lws_reason = static_cast<enum lws_callback_reasons>(reason);

    switch (lws_reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED: {
            // Successfully connected to server
            client->handle_connected();
            break;
        }

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: {
            // Connection failed
            std::string error_msg = in ? std::string(static_cast<char*>(in), len) : "Connection error";
            client->handle_error(error_msg);
            client->handle_disconnected();
            break;
        }

        case LWS_CALLBACK_CLOSED: {
            // Connection closed
            client->handle_disconnected();
            break;
        }

        case LWS_CALLBACK_CLIENT_RECEIVE: {
            // Data received from server
            if (in && len > 0) {
                ByteBuffer data(static_cast<uint8_t*>(in),
                               static_cast<uint8_t*>(in) + len);
                client->handle_message(data);
            }
            break;
        }

        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            // Client is ready to send data
            bool has_more = client->process_send_queue();

            // If there are more messages, request another write callback
            if (has_more) {
                lws_callback_on_writable(wsi);
            }
            break;
        }

        case LWS_CALLBACK_PROTOCOL_INIT: {
            // Protocol initialized
            break;
        }

        case LWS_CALLBACK_PROTOCOL_DESTROY: {
            // Protocol being destroyed
            break;
        }

        case LWS_CALLBACK_WSI_DESTROY: {
            // WebSocket instance being destroyed
            break;
        }

        default:
            break;
    }

    return 0;
}

} // namespace umicp

