/**
 * UMICP WebSocket Server Implementation
 * Complete libwebsockets integration
 */

#include <libwebsockets.h>
#include "websocket_server.h"
#include "umicp_types.h"
#include "envelope.h"
#include <algorithm>
#include <cstring>

namespace umicp {

// ============================================================================
// WebSocketServer Implementation
// ============================================================================

WebSocketServer::WebSocketServer(const WebSocketServerConfig& config)
    : EventEmitter(),
      config_(config),
      running_(false),
      lws_context_(nullptr) {

    // Initialize statistics
    stats_ = ServerStats{};
    stats_.start_time = std::chrono::system_clock::now();
}

WebSocketServer::~WebSocketServer() {
    if (running_) {
        stop();
    }
}

// ============================================================================
// Server Lifecycle
// ============================================================================

Result<void> WebSocketServer::start() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (running_) {
        return Result<void>::error(ErrorCode::INVALID_STATE, "Server already running");
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

    // Configure compression extensions
    static const struct lws_extension extensions[] = {
        {
            "permessage-deflate",
            lws_extension_callback_pm_deflate,
            "permessage-deflate"
        },
        { nullptr, nullptr, nullptr } // Terminator
    };

    // Create context info
    struct lws_context_creation_info info;
    std::memset(&info, 0, sizeof(info));

    info.port = config_.port;
    info.iface = config_.interface_address.empty() ? nullptr : config_.interface_address.c_str();
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.user = this;
    info.options = LWS_SERVER_OPTION_VALIDATE_UTF8 | LWS_SERVER_OPTION_EXPLICIT_VHOSTS;
    info.max_http_header_data = config_.rx_buffer_size;
    info.max_http_header_pool = 16;
    info.pt_serv_buf_size = config_.rx_buffer_size;

    // Enable compression if configured
    if (config_.per_message_deflate) {
        info.extensions = extensions;
    }

    // SSL/TLS configuration
    if (config_.use_ssl) {
        info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        info.ssl_cert_filepath = config_.ssl_cert_path.empty() ? nullptr : config_.ssl_cert_path.c_str();
        info.ssl_private_key_filepath = config_.ssl_key_path.empty() ? nullptr : config_.ssl_key_path.c_str();
    }

    // Create libwebsockets context
    lws_context_ = lws_create_context(&info);
    if (!lws_context_) {
        return Result<void>::error(
            ErrorCode::INITIALIZATION_FAILED,
            "Failed to create libwebsockets context"
        );
    }

    running_ = true;

    // Start service thread
    service_thread_ = std::make_unique<std::thread>([this]() {
        service_loop();
    });

    return Result<void>::ok();
}

Result<void> WebSocketServer::stop() {
    {
        std::lock_guard<std::mutex> lock(state_mutex_);

        if (!running_) {
            return Result<void>::error(ErrorCode::INVALID_STATE, "Server not running");
        }

        running_ = false;
    }

    // Cancel service thread
    if (lws_context_) {
        lws_cancel_service(lws_context_);
    }

    // Wait for service thread to finish
    if (service_thread_ && service_thread_->joinable()) {
        service_thread_->join();
    }

    // Disconnect all clients
    std::vector<std::string> client_ids;
    {
        std::lock_guard<std::mutex> clients_lock(clients_mutex_);
        for (const auto& [id, client] : clients_) {
            client_ids.push_back(id);
        }
    }

    for (const auto& id : client_ids) {
        disconnect_client(id);
    }

    // Destroy libwebsockets context
    if (lws_context_) {
        lws_context_destroy(lws_context_);
        lws_context_ = nullptr;
    }

    return Result<void>::ok();
}

bool WebSocketServer::is_running() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return running_;
}

// ============================================================================
// Message Sending
// ============================================================================

Result<void> WebSocketServer::send_to_client(const std::string& client_id, const ByteBuffer& data) {
    if (!running_) {
        return Result<void>::error(ErrorCode::NOT_CONNECTED, "Server not running");
    }

    lws* wsi = nullptr;

    // Check if client exists and get wsi
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(client_id);
        if (it == clients_.end()) {
            return Result<void>::error(ErrorCode::NOT_FOUND, "Client not found: " + client_id);
        }

        if (!it->second->is_connected) {
            return Result<void>::error(ErrorCode::NOT_CONNECTED, "Client not connected: " + client_id);
        }
    }

    // Get wsi for client
    {
        std::lock_guard<std::mutex> lock(session_mutex_);
        auto it = client_id_to_wsi_.find(client_id);
        if (it == client_id_to_wsi_.end()) {
            return Result<void>::error(ErrorCode::NOT_FOUND, "Client session not found: " + client_id);
        }
        wsi = it->second;
    }

    // Add to send queue
    {
        std::lock_guard<std::mutex> lock(send_queues_mutex_);
        send_queues_[client_id].push(data);
    }

    // Request write callback
    if (wsi) {
        lws_callback_on_writable(wsi);
    }

    return Result<void>::ok();
}

Result<void> WebSocketServer::send_to_client(const std::string& client_id, const Envelope& envelope) {
    auto json = envelope.to_json();
    ByteBuffer buffer(json.begin(), json.end());
    return send_to_client(client_id, buffer);
}

Result<void> WebSocketServer::broadcast(const ByteBuffer& data, const std::string& exclude_client_id) {
    std::vector<std::string> client_ids;

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (const auto& [id, client] : clients_) {
            if (id != exclude_client_id && client->is_connected) {
                client_ids.push_back(id);
            }
        }
    }

    size_t success_count = 0;
    for (const auto& client_id : client_ids) {
        auto result = send_to_client(client_id, data);
        if (result.is_ok()) {
            success_count++;
        }
    }

    if (success_count == 0 && !client_ids.empty()) {
        return Result<void>::error(ErrorCode::TRANSMISSION_ERROR, "Broadcast failed to all clients");
    }

    return Result<void>::ok();
}

Result<void> WebSocketServer::broadcast(const Envelope& envelope, const std::string& exclude_client_id) {
    auto json = envelope.to_json();
    ByteBuffer buffer(json.begin(), json.end());
    return broadcast(buffer, exclude_client_id);
}

// ============================================================================
// Client Management
// ============================================================================

std::vector<WebSocketClientInfo> WebSocketServer::get_clients() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);

    std::vector<WebSocketClientInfo> result;
    result.reserve(clients_.size());

    for (const auto& [id, client] : clients_) {
        result.push_back(*client);
    }

    return result;
}

std::optional<WebSocketClientInfo> WebSocketServer::get_client(const std::string& client_id) const {
    std::lock_guard<std::mutex> lock(clients_mutex_);

    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        return *it->second;
    }

    return std::nullopt;
}

Result<void> WebSocketServer::disconnect_client(const std::string& client_id) {
    std::shared_ptr<WebSocketClientInfo> client;

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(client_id);
        if (it == clients_.end()) {
            return Result<void>::error(ErrorCode::NOT_FOUND, "Client not found: " + client_id);
        }
        client = it->second;
        clients_.erase(it);
    }

    client->is_connected = false;

    // Note: Actual close happens in libwebsockets callback
    // We just mark the client as disconnected and remove from tracking

    // Emit event
    Event event("disconnection");
    event.set("client_id", client_id);
    emit("disconnection", event);

    // Call callback
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        if (disconnection_callback_) {
            disconnection_callback_(client_id);
        }
    }

    // Update stats
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.active_connections--;
    }

    return Result<void>::ok();
}

size_t WebSocketServer::get_client_count() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_.size();
}

// ============================================================================
// Callbacks
// ============================================================================

void WebSocketServer::set_connection_callback(ConnectionCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    connection_callback_ = std::move(callback);
}

void WebSocketServer::set_disconnection_callback(DisconnectionCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    disconnection_callback_ = std::move(callback);
}

void WebSocketServer::set_message_callback(MessageCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    message_callback_ = std::move(callback);
}

void WebSocketServer::set_error_callback(ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    error_callback_ = std::move(callback);
}

// ============================================================================
// Statistics
// ============================================================================

WebSocketServer::ServerStats WebSocketServer::get_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void WebSocketServer::reset_stats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.messages_sent = 0;
    stats_.messages_received = 0;
    stats_.bytes_sent = 0;
    stats_.bytes_received = 0;
    stats_.errors_count = 0;
}

// ============================================================================
// Internal Methods
// ============================================================================

void WebSocketServer::service_loop() {
    while (running_) {
        // Service libwebsockets (50ms timeout)
        if (lws_context_) {
            lws_service(lws_context_, 50);
        }

        // Check for client timeouts
        check_timeouts();

        // Small sleep to prevent CPU spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void WebSocketServer::handle_connection(
    lws* wsi,
    const std::string& client_id,
    const std::string& remote_addr,
    uint16_t port
) {
    auto client = std::make_shared<WebSocketClientInfo>(client_id);
    client->remote_address = remote_addr;
    client->remote_port = port;

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_[client_id] = client;
    }

    // Register session mapping
    register_session(wsi, client_id);

    // Update stats
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.total_connections++;
        stats_.active_connections++;
    }

    // Emit event
    Event event("connection");
    event.set("client_id", client_id);
    event.set("remote_address", remote_addr);
    event.set("remote_port", static_cast<int>(port));
    emit("connection", event);

    // Call callback
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        if (connection_callback_) {
            connection_callback_(client_id);
        }
    }
}

void WebSocketServer::handle_disconnection(lws* wsi) {
    std::string client_id = get_client_id_for_wsi(wsi);
    if (client_id.empty()) {
        return; // Unknown client
    }

    unregister_session(wsi);
    disconnect_client(client_id);
}

void WebSocketServer::handle_message(lws* wsi, const ByteBuffer& data) {
    std::string client_id = get_client_id_for_wsi(wsi);
    if (client_id.empty()) {
        return; // Unknown client
    }
    // Update client activity
    update_client_activity(client_id);

    // Update stats
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.messages_received++;
        stats_.bytes_received += data.size();
    }

    // Update client stats
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(client_id);
        if (it != clients_.end()) {
            it->second->messages_received++;
            it->second->bytes_received += data.size();
        }
    }

    // Emit event
    Event event("message");
    event.set("client_id", client_id);
    event.set("data", data);
    emit("message", event);

    // Call callback
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        if (message_callback_) {
            message_callback_(client_id, data);
        }
    }
}

void WebSocketServer::handle_error(const std::string& error, const std::string* client_id) {
    // Update stats
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.errors_count++;
    }

    // Emit event
    Event event("error");
    event.set("error", error);
    if (client_id) {
        event.set("client_id", *client_id);
    }
    emit("error", event);

    // Call callback
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex_);
        if (error_callback_) {
            error_callback_(error, client_id);
        }
    }
}

void WebSocketServer::update_client_activity(const std::string& client_id) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
        it->second->last_activity = std::chrono::system_clock::now();
    }
}

void WebSocketServer::check_timeouts() {
    auto now = std::chrono::system_clock::now();
    std::vector<std::string> timed_out_clients;

    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (const auto& [id, client] : clients_) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - client->last_activity
            );

            if (elapsed > config_.connection_timeout) {
                timed_out_clients.push_back(id);
            }
        }
    }

    for (const auto& client_id : timed_out_clients) {
        disconnect_client(client_id);
    }
}

// ============================================================================
// libwebsockets Callback (Static)
// ============================================================================

int WebSocketServer::callback_websocket(
    lws* wsi,
    int reason,
    void* user,
    void* in,
    size_t len
) {
    // Get server instance from context
    lws_context* context = lws_get_context(wsi);
    WebSocketServer* server = static_cast<WebSocketServer*>(
        lws_context_user(context)
    );

    if (!server) {
        return -1;
    }

    // Cast to enum for switch
    enum lws_callback_reasons lws_reason = static_cast<enum lws_callback_reasons>(reason);

    switch (lws_reason) {
        case LWS_CALLBACK_ESTABLISHED: {
            // New client connected
            char client_name[128];
            char client_ip[128];

            lws_get_peer_addresses(wsi, lws_get_socket_fd(wsi),
                                   client_name, sizeof(client_name),
                                   client_ip, sizeof(client_ip));

            // Generate client ID using socket fd and IP
            int sock_fd = lws_get_socket_fd(wsi);
            std::string client_id = std::string(client_ip) + ":" + std::to_string(sock_fd);

            server->handle_connection(wsi, client_id, client_ip, 0);
            break;
        }

        case LWS_CALLBACK_CLOSED: {
            // Client disconnected
            server->handle_disconnection(wsi);
            break;
        }

        case LWS_CALLBACK_RECEIVE: {
            // Data received from client
            if (in && len > 0) {
                ByteBuffer data(static_cast<uint8_t*>(in),
                               static_cast<uint8_t*>(in) + len);

                server->handle_message(wsi, data);
            }
            break;
        }

        case LWS_CALLBACK_SERVER_WRITEABLE: {
            // Server is ready to send data to client
            std::string client_id = server->get_client_id_for_wsi(wsi);
            if (!client_id.empty()) {
                bool has_more = server->process_send_queue(wsi, client_id);

                // If there are more messages, request another write callback
                if (has_more) {
                    lws_callback_on_writable(wsi);
                }
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

        default:
            break;
    }

    return 0;
}

void WebSocketServer::register_session(lws* wsi, const std::string& client_id) {
    std::lock_guard<std::mutex> lock(session_mutex_);
    wsi_to_client_id_[wsi] = client_id;
    client_id_to_wsi_[client_id] = wsi;
}

void WebSocketServer::unregister_session(lws* wsi) {
    std::lock_guard<std::mutex> lock(session_mutex_);

    auto it = wsi_to_client_id_.find(wsi);
    if (it != wsi_to_client_id_.end()) {
        std::string client_id = it->second;
        wsi_to_client_id_.erase(it);
        client_id_to_wsi_.erase(client_id);

        // Clean up send queue
        {
            std::lock_guard<std::mutex> queue_lock(send_queues_mutex_);
            send_queues_.erase(client_id);
        }
    }
}

std::string WebSocketServer::get_client_id_for_wsi(lws* wsi) const {
    std::lock_guard<std::mutex> lock(session_mutex_);

    auto it = wsi_to_client_id_.find(wsi);
    if (it != wsi_to_client_id_.end()) {
        return it->second;
    }

    return "";
}

bool WebSocketServer::process_send_queue(lws* wsi, const std::string& client_id) {
    if (!wsi) {
        return false;
    }

    ByteBuffer data_to_send;

    {
        std::lock_guard<std::mutex> lock(send_queues_mutex_);
        auto it = send_queues_.find(client_id);
        if (it == send_queues_.end() || it->second.empty()) {
            return false;
        }
        data_to_send = it->second.front();
        it->second.pop();
    }

    // Prepare buffer with LWS_PRE bytes padding
    const size_t pre_padding = LWS_PRE;
    std::vector<uint8_t> padded_buffer(pre_padding + data_to_send.size());
    std::copy(data_to_send.begin(), data_to_send.end(), padded_buffer.begin() + pre_padding);

    // Send via libwebsockets
    int written = lws_write(
        wsi,
        padded_buffer.data() + pre_padding,
        data_to_send.size(),
        LWS_WRITE_BINARY
    );

    if (written < 0) {
        handle_error("Failed to send data to client", &client_id);
        return false;
    }

    // Update statistics
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.messages_sent++;
        stats_.bytes_sent += data_to_send.size();
    }

    // Update client stats
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(client_id);
        if (it != clients_.end()) {
            it->second->messages_sent++;
            it->second->bytes_sent += data_to_send.size();
        }
    }

    // Return true if there are more messages to send
    std::lock_guard<std::mutex> lock(send_queues_mutex_);
    auto it = send_queues_.find(client_id);
    return it != send_queues_.end() && !it->second.empty();
}

} // namespace umicp

