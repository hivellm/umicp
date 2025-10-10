/**
 * UMICP WebSocket Client Implementation
 * Note: This is a stub implementation. Full libwebsockets integration required.
 */

#include "websocket_client.h"
#include "umicp_types.h"
#include "envelope.h"
#include <algorithm>
#include <cmath>

namespace umicp {

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
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (state_ == ConnectionState::DISCONNECTED) {
        return Result<void>::ok();
    }

    should_stop_ = true;
    state_ = ConnectionState::DISCONNECTED;

    // TODO: Close libwebsockets connection
    // if (wsi_) {
    //     lws_close_reason(wsi_, LWS_CLOSE_STATUS_NORMAL, NULL, 0);
    //     wsi_ = nullptr;
    // }

    // Stop service thread
    // if (service_thread_ && service_thread_->joinable()) {
    //     service_thread_->join();
    // }

    // if (lws_context_) {
    //     lws_context_destroy(lws_context_);
    //     lws_context_ = nullptr;
    // }

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

    // TODO: Implement libwebsockets send
    return Result<void>::error(
        ErrorCode::NOT_IMPLEMENTED,
        "Send requires libwebsockets integration"
    );

    // Future implementation:
    // lws_write(wsi_, data.data(), data.size(), LWS_WRITE_BINARY);
    // Update statistics
    // {
    //     std::lock_guard<std::mutex> lock(stats_mutex_);
    //     stats_.messages_sent++;
    //     stats_.bytes_sent += data.size();
    //     transport_stats_.messages_sent++;
    //     transport_stats_.bytes_sent += data.size();
    // }
}

Result<void> WebSocketClient::send_envelope(const Envelope& envelope) {
    auto json = envelope.to_json();
    ByteBuffer buffer(json.begin(), json.end());
    return send(buffer);
}

Result<void> WebSocketClient::send_frame(const Frame& frame) {
    // Serialize frame to binary
    ByteBuffer buffer; // TODO: Implement frame serialization
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
    config.endpoint = config_.url;
    // TODO: Fill other transport config fields
    return config;
}

void WebSocketClient::set_message_callback(Transport::MessageCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    transport_message_callback_ = std::move(callback);
}

void WebSocketClient::set_connection_callback(Transport::ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    transport_connection_callback_ = std::move(callback);
}

void WebSocketClient::set_error_callback(Transport::ErrorCallback callback) {
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
    // TODO: Implement libwebsockets service loop
    // while (!should_stop_) {
    //     if (state_ == ConnectionState::RECONNECTING) {
    //         if (std::chrono::steady_clock::now() >= next_reconnect_time_) {
    //             attempt_reconnect();
    //         }
    //     }
    //
    //     if (lws_context_) {
    //         lws_service(lws_context_, 50);
    //     }
    //
    //     std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // }
}

Result<void> WebSocketClient::connect_internal() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (state_ == ConnectionState::CONNECTED || state_ == ConnectionState::CONNECTING) {
        return Result<void>::error(ErrorCode::INVALID_STATE, "Already connected or connecting");
    }

    state_ = ConnectionState::CONNECTING;

    // TODO: Implement libwebsockets connection
    return Result<void>::error(
        ErrorCode::NOT_IMPLEMENTED,
        "WebSocketClient requires libwebsockets integration. "
        "See docs/WEBSOCKET_README.md for implementation details."
    );

    // Future implementation:
    /*
    struct lws_protocols protocols[] = {
        {
            "umicp",
            callback_websocket,
            0,
            config_.rx_buffer_size,
        },
        { NULL, NULL, 0, 0 }
    };

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.user = this;

    if (config_.use_ssl) {
        // Setup SSL
    }

    lws_context_ = lws_create_context(&info);
    if (!lws_context_) {
        state_ = ConnectionState::FAILED;
        return Result<void>::error(ErrorCode::INITIALIZATION_FAILED,
                                    "Failed to create libwebsockets context");
    }

    // Parse URL and create connection
    struct lws_client_connect_info ccinfo;
    memset(&ccinfo, 0, sizeof(ccinfo));
    ccinfo.context = lws_context_;
    ccinfo.address = ...; // Parse from URL
    ccinfo.port = ...; // Parse from URL
    ccinfo.path = config_.path.c_str();
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = "umicp";

    wsi_ = lws_client_connect_via_info(&ccinfo);
    if (!wsi_) {
        state_ = ConnectionState::FAILED;
        return Result<void>::error(ErrorCode::CONNECTION_FAILED,
                                    "Failed to initiate connection");
    }

    should_stop_ = false;
    service_thread_ = std::make_unique<std::thread>([this]() {
        service_loop();
    });

    return Result<void>::ok();
    */
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
    struct lws* wsi,
    enum lws_callback_reasons reason,
    void* user,
    void* in,
    size_t len
) {
    // TODO: Implement libwebsockets protocol callback
    // This will handle:
    // - LWS_CALLBACK_CLIENT_ESTABLISHED - connected to server
    // - LWS_CALLBACK_CLIENT_CONNECTION_ERROR - connection failed
    // - LWS_CALLBACK_CLOSED - connection closed
    // - LWS_CALLBACK_CLIENT_RECEIVE - data received
    // - LWS_CALLBACK_CLIENT_WRITEABLE - ready to send
    // - etc.

    return 0;
}

} // namespace umicp

