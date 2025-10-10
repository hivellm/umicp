/**
 * UMICP Multiplexed Peer Implementation
 * Note: This is a partial implementation requiring advanced WebSocket (Phase 3)
 */

#include "multiplexed_peer.h"
#include "websocket_server.h"
#include "websocket_client.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace umicp {

// ============================================================================
// MultiplexedPeer Implementation
// ============================================================================

MultiplexedPeer::MultiplexedPeer(const MultiplexedPeerOptions& options)
    : EventEmitter(),
      peer_id_(options.peer_id),
      options_(options),
      server_running_(false) {

    // Initialize statistics
    stats_ = PeerStats{};

    // If server config provided, we'll start it later in start_server()
}

MultiplexedPeer::~MultiplexedPeer() {
    shutdown();
}

// ============================================================================
// Server Component
// ============================================================================

Result<void> MultiplexedPeer::start_server() {
    if (server_running_) {
        return Result<void>::error(ErrorCode::INVALID_STATE, "Server already running");
    }

    if (!options_.server.has_value()) {
        return Result<void>::error(ErrorCode::INVALID_CONFIG, "No server configuration provided");
    }

    // Create WebSocket server config from peer config
    WebSocketServerConfig ws_config;
    ws_config.port = options_.server->port;
    ws_config.path = options_.server->path;
    ws_config.per_message_deflate = options_.server->compression;
    ws_config.max_payload_size = options_.server->max_payload;
    ws_config.ping_interval = options_.server->ping_interval;
    ws_config.connection_timeout = options_.server->connection_timeout;

    // Create WebSocket server
    server_ = std::make_unique<WebSocketServer>(ws_config);

    // Set up callbacks
    server_->set_connection_callback([this](const std::string& client_id) {
        // Create transport wrapper for the client
        // Note: For now, we'll create a PeerConnection without a direct Transport object
        // since WebSocketServer manages the connection internally
        auto conn = std::make_shared<PeerConnection>(
            client_id,
            ConnectionType::INCOMING,
            nullptr // Transport is managed by server
        );

        {
            std::lock_guard<std::mutex> lock(peers_mutex_);
            peers_[client_id] = conn;
            stats_.total_peers++;
            stats_.incoming_connections++;
        }

        // Emit event
        Event event("peer:connect");
        event.set("peer_id", client_id);
        event.set("type", std::string("incoming"));
        emit("peer:connect", event);

        if (options_.on_peer_connect) {
            options_.on_peer_connect(*conn);
        }
    });

    server_->set_disconnection_callback([this](const std::string& client_id) {
        handle_peer_disconnected(client_id);
    });

    server_->set_message_callback([this](const std::string& client_id, const ByteBuffer& data) {
        // Parse envelope from data
        try {
            std::string json_str(data.begin(), data.end());
            auto parse_result = EnvelopeProcessor::deserialize(json_str);

            if (parse_result.is_ok() && parse_result.value.has_value()) {
                handle_message(client_id, *parse_result.value);
            } else {
                handle_error("Failed to parse envelope: " + parse_result.error(), &client_id);
            }
        } catch (...) {
            handle_error("Failed to parse envelope", &client_id);
        }
    });

    server_->set_error_callback([this](const std::string& error, const std::string* client_id) {
        handle_error(error, client_id);
    });

    // Start server
    auto result = server_->start();
    if (result.is_ok()) {
        server_running_ = true;
    }

    return result;
}

Result<void> MultiplexedPeer::stop_server() {
    if (!server_running_) {
        return Result<void>::error(ErrorCode::INVALID_STATE, "Server not running");
    }

    if (server_) {
        auto result = server_->stop();
        if (!result.is_ok()) {
            return result;
        }
        server_.reset();
    }

    server_running_ = false;

    return Result<void>::ok();
}

bool MultiplexedPeer::is_server_running() const {
    return server_running_;
}

// ============================================================================
// Client Component
// ============================================================================

Result<std::string> MultiplexedPeer::connect_to_peer(
    const std::string& url,
    const std::unordered_map<std::string, std::string>& metadata
) {
    // Create WebSocket client config
    WebSocketClientConfig ws_config;
    ws_config.url = url;
    ws_config.reconnect.enabled = true;
    ws_config.reconnect.max_attempts = 10;
    ws_config.per_message_deflate = true;

    // Create WebSocket client
    auto client = std::make_shared<WebSocketClient>(ws_config);

    // Generate peer ID
    std::string peer_id = generate_peer_id();

    // Create peer connection
    auto conn = std::make_shared<PeerConnection>(
        peer_id,
        ConnectionType::OUTGOING,
        client
    );
    conn->url = url;
    conn->metadata = metadata;

    // Set up client callbacks
    client->set_connect_callback([this, peer_id]() {
        // Connection established, send handshake
        send_handshake(peer_id);
    });

    client->set_disconnect_callback([this, peer_id]() {
        handle_peer_disconnected(peer_id);
    });

    client->set_ws_message_callback([this, peer_id](const ByteBuffer& data) {
        // Parse envelope from data
        try {
            std::string json_str(data.begin(), data.end());
            auto parse_result = EnvelopeProcessor::deserialize(json_str);

            if (parse_result.is_ok() && parse_result.value.has_value()) {
                handle_message(peer_id, *parse_result.value);
            } else {
                handle_error("Failed to parse envelope: " + parse_result.error(), &peer_id);
            }
        } catch (...) {
            handle_error("Failed to parse envelope", &peer_id);
        }
    });

    client->set_ws_error_callback([this, peer_id](const std::string& error) {
        handle_error(error, &peer_id);
    });

    // Connect
    auto result = client->connect();
    if (!result.is_ok()) {
        return Result<std::string>::error(result.error_code(), result.error());
    }

    // Add to peers map
    {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        peers_[peer_id] = conn;
        stats_.total_peers++;
        stats_.outgoing_connections++;
    }

    // Emit event
    Event event("peer:connect");
    event.set("peer_id", peer_id);
    event.set("type", std::string("outgoing"));
    event.set("url", url);
    emit("peer:connect", event);

    if (options_.on_peer_connect) {
        options_.on_peer_connect(*conn);
    }

    return Result<std::string>::ok(peer_id);
}

// ============================================================================
// Peer Management
// ============================================================================

Result<void> MultiplexedPeer::disconnect_peer(const std::string& peer_id) {
    std::shared_ptr<PeerConnection> conn;

    {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        auto it = peers_.find(peer_id);
        if (it == peers_.end()) {
            return Result<void>::error(ErrorCode::NOT_FOUND, "Peer not found: " + peer_id);
        }
        conn = it->second;
        peers_.erase(it);

        // Update stats
        if (conn->type == ConnectionType::INCOMING) {
            stats_.incoming_connections--;
        } else {
            stats_.outgoing_connections--;
        }
        stats_.total_peers--;
    }

    // Disconnect transport
    if (conn->transport) {
        conn->transport->disconnect();
    }
    conn->is_connected = false;

    // Emit event
    Event event("peer:disconnect");
    event.set("peer_id", peer_id);
    emit("peer:disconnect", event);

    if (options_.on_peer_disconnect) {
        options_.on_peer_disconnect(*conn);
    }

    return Result<void>::ok();
}

Result<void> MultiplexedPeer::disconnect_peer_by_url(const std::string& url) {
    std::string peer_id_to_disconnect;

    {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        for (const auto& [id, conn] : peers_) {
            if (conn->url.has_value() && conn->url.value() == url) {
                peer_id_to_disconnect = id;
                break;
            }
        }
    }

    if (peer_id_to_disconnect.empty()) {
        return Result<void>::error(ErrorCode::NOT_FOUND, "Peer with URL not found: " + url);
    }

    return disconnect_peer(peer_id_to_disconnect);
}

// ============================================================================
// Message Sending
// ============================================================================

Result<void> MultiplexedPeer::send_to_peer(const std::string& peer_id, const Envelope& envelope) {
    std::shared_ptr<PeerConnection> conn;

    {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        auto it = peers_.find(peer_id);
        if (it == peers_.end()) {
            return Result<void>::error(ErrorCode::NOT_FOUND, "Peer not found: " + peer_id);
        }
        conn = it->second;
    }

    if (!conn->is_connected) {
        return Result<void>::error(ErrorCode::NOT_CONNECTED, "Peer not connected: " + peer_id);
    }

    // Serialize envelope
    auto json = envelope.to_json();
    ByteBuffer buffer(json.begin(), json.end());

    Result<void> result;

    // Send based on connection type
    if (conn->type == ConnectionType::INCOMING) {
        // Incoming connection - use server to send
        if (!server_) {
            return Result<void>::error(ErrorCode::INVALID_STATE, "Server not available");
        }
        result = server_->send_to_client(peer_id, buffer);
    } else {
        // Outgoing connection - use transport (WebSocketClient)
        if (!conn->transport) {
            return Result<void>::error(ErrorCode::NOT_CONNECTED, "Transport not available");
        }
        result = conn->transport->send(buffer);
    }

    if (result.is_ok()) {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.messages_sent++;
    }

    return result;
}

Result<void> MultiplexedPeer::send_to_peer_by_url(const std::string& url, const Envelope& envelope) {
    std::string peer_id_to_send;

    {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        for (const auto& [id, conn] : peers_) {
            if (conn->url.has_value() && conn->url.value() == url) {
                peer_id_to_send = id;
                break;
            }
        }
    }

    if (peer_id_to_send.empty()) {
        return Result<void>::error(ErrorCode::NOT_FOUND, "Peer with URL not found: " + url);
    }

    return send_to_peer(peer_id_to_send, envelope);
}

// ============================================================================
// Broadcasting
// ============================================================================

size_t MultiplexedPeer::broadcast(const Envelope& envelope, const std::string& exclude_peer_id) {
    std::vector<std::string> peer_ids;

    {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        for (const auto& [id, conn] : peers_) {
            if (id != exclude_peer_id && conn->is_connected) {
                peer_ids.push_back(id);
            }
        }
    }

    size_t success_count = 0;
    for (const auto& peer_id : peer_ids) {
        auto result = send_to_peer(peer_id, envelope);
        if (result.is_ok()) {
            success_count++;
        }
    }

    return success_count;
}

size_t MultiplexedPeer::broadcast_to_type(
    const Envelope& envelope,
    ConnectionType type,
    const std::string& exclude_peer_id
) {
    std::vector<std::string> peer_ids;

    {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        for (const auto& [id, conn] : peers_) {
            if (id != exclude_peer_id && conn->is_connected && conn->type == type) {
                peer_ids.push_back(id);
            }
        }
    }

    size_t success_count = 0;
    for (const auto& peer_id : peer_ids) {
        auto result = send_to_peer(peer_id, envelope);
        if (result.is_ok()) {
            success_count++;
        }
    }

    return success_count;
}

// ============================================================================
// Request-Response Pattern
// ============================================================================

Result<Envelope> MultiplexedPeer::send_and_wait(
    const std::string& peer_id,
    const Envelope& envelope,
    std::chrono::milliseconds timeout
) {
    // Generate request ID
    std::string request_id = envelope.get_message_id();

    // Create pending request
    auto pending = std::make_shared<PendingRequest>();
    pending->request_id = request_id;
    pending->expiry = std::chrono::steady_clock::now() +
                      std::chrono::duration_cast<std::chrono::steady_clock::duration>(timeout);

    // Get future before registering
    std::future<Envelope> response_future = pending->response_promise.get_future();

    // Register pending request
    register_pending_request(request_id, pending);

    // Send request
    auto send_result = send_to_peer(peer_id, envelope);
    if (!send_result.is_ok()) {
        // Remove pending request
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        pending_requests_.erase(request_id);
        return Result<Envelope>::error(send_result.error_code(), send_result.error());
    }

    // Wait for response with timeout
    auto status = response_future.wait_for(timeout);

    if (status == std::future_status::timeout) {
        // Cleanup
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        pending_requests_.erase(request_id);
        return Result<Envelope>::error(ErrorCode::TIMEOUT, "Request timeout");
    }

    try {
        Envelope response = response_future.get();
        return Result<Envelope>::ok(std::move(response));
    } catch (const std::exception& e) {
        return Result<Envelope>::error(ErrorCode::PROTOCOL_ERROR, std::string("Request failed: ") + e.what());
    }
}

// ============================================================================
// Peer Queries
// ============================================================================

std::vector<PeerConnection> MultiplexedPeer::get_peers() const {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    std::vector<PeerConnection> result;
    result.reserve(peers_.size());

    for (const auto& [id, conn] : peers_) {
        result.push_back(*conn);
    }

    return result;
}

std::vector<PeerConnection> MultiplexedPeer::get_peers_by_type(ConnectionType type) const {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    std::vector<PeerConnection> result;

    for (const auto& [id, conn] : peers_) {
        if (conn->type == type) {
            result.push_back(*conn);
        }
    }

    return result;
}

std::optional<PeerConnection> MultiplexedPeer::get_peer(const std::string& peer_id) const {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    auto it = peers_.find(peer_id);
    if (it != peers_.end()) {
        return *it->second;
    }

    return std::nullopt;
}

std::optional<PeerConnection> MultiplexedPeer::find_peer_by_metadata(
    const std::string& key,
    const std::string& value
) const {
    std::lock_guard<std::mutex> lock(peers_mutex_);

    for (const auto& [id, conn] : peers_) {
        auto it = conn->metadata.find(key);
        if (it != conn->metadata.end() && it->second == value) {
            return *conn;
        }
    }

    return std::nullopt;
}

// ============================================================================
// Statistics
// ============================================================================

MultiplexedPeer::PeerStats MultiplexedPeer::get_stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void MultiplexedPeer::reset_stats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.messages_sent = 0;
    stats_.messages_received = 0;
    // Keep connection counts
}

// ============================================================================
// Lifecycle
// ============================================================================

Result<void> MultiplexedPeer::shutdown() {
    // Stop server
    if (server_running_) {
        stop_server();
    }

    // Disconnect all peers
    std::vector<std::string> peer_ids;
    {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        for (const auto& [id, conn] : peers_) {
            peer_ids.push_back(id);
        }
    }

    for (const auto& peer_id : peer_ids) {
        disconnect_peer(peer_id);
    }

    // Clear all pending requests
    {
        std::lock_guard<std::mutex> lock(pending_requests_mutex_);
        pending_requests_.clear();
    }

    return Result<void>::ok();
}

// ============================================================================
// Internal Methods
// ============================================================================

std::string MultiplexedPeer::generate_peer_id() {
    // Generate random peer ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis;

    std::stringstream ss;
    ss << "peer-" << std::hex << std::setfill('0') << std::setw(8) << dis(gen);

    return ss.str();
}

void MultiplexedPeer::handle_incoming_connection(std::shared_ptr<Transport> transport) {
    std::string peer_id = generate_peer_id();

    auto conn = std::make_shared<PeerConnection>(
        peer_id,
        ConnectionType::INCOMING,
        transport
    );

    {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        peers_[peer_id] = conn;
        stats_.total_peers++;
        stats_.incoming_connections++;
    }

    // Emit event
    Event event("peer:connect");
    event.set("peer_id", peer_id);
    event.set("type", std::string("incoming"));
    emit("peer:connect", event);

    if (options_.on_peer_connect) {
        options_.on_peer_connect(*conn);
    }
}

void MultiplexedPeer::handle_message(const std::string& peer_id, const Envelope& envelope) {
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.messages_received++;
    }

    // Check if this is a response to a pending request
    std::string reply_to = envelope.get_capabilities().count("reply_to") > 0
        ? envelope.get_capabilities().at("reply_to")
        : "";

    if (!reply_to.empty()) {
        auto pending = get_pending_request(reply_to);
        if (pending.has_value()) {
            // Fulfill the promise
            try {
                pending.value()->response_promise.set_value(envelope);
            } catch (...) {
                // Promise already fulfilled or moved
            }

            // Remove from pending
            std::lock_guard<std::mutex> lock(pending_requests_mutex_);
            pending_requests_.erase(reply_to);
            return; // Don't emit message event for responses
        }
    }

    // Get peer connection
    auto conn = get_peer(peer_id);

    // Emit message event
    Event event("message");
    event.set("envelope", envelope);
    event.set("peer_id", peer_id);
    if (conn.has_value()) {
        event.set("peer", conn.value());
    }
    emit("message", event);

    // Call callback if provided
    if (options_.on_message && conn.has_value()) {
        options_.on_message(envelope, conn.value());
    }
}

void MultiplexedPeer::handle_peer_disconnected(const std::string& peer_id) {
    disconnect_peer(peer_id);
}

void MultiplexedPeer::handle_error(const std::string& error, const std::string* peer_id) {
    Event event("error");
    event.set("error", error);
    if (peer_id) {
        event.set("peer_id", *peer_id);
    }
    emit("error", event);

    if (options_.on_error) {
        auto conn = peer_id ? get_peer(*peer_id) : std::nullopt;
        options_.on_error(error, conn.has_value() ? &conn.value() : nullptr);
    }
}

// ============================================================================
// Request-Response Helpers
// ============================================================================

void MultiplexedPeer::register_pending_request(
    const std::string& request_id,
    std::shared_ptr<PendingRequest> request
) {
    std::lock_guard<std::mutex> lock(pending_requests_mutex_);
    pending_requests_[request_id] = request;
}

std::optional<std::shared_ptr<MultiplexedPeer::PendingRequest>>
MultiplexedPeer::get_pending_request(const std::string& request_id) {
    std::lock_guard<std::mutex> lock(pending_requests_mutex_);

    auto it = pending_requests_.find(request_id);
    if (it != pending_requests_.end()) {
        return it->second;
    }

    return std::nullopt;
}

void MultiplexedPeer::cleanup_expired_requests() {
    auto now = std::chrono::steady_clock::now();

    std::lock_guard<std::mutex> lock(pending_requests_mutex_);

    auto it = pending_requests_.begin();
    while (it != pending_requests_.end()) {
        if (it->second->expiry < now) {
            // Set exception on promise
            try {
                it->second->response_promise.set_exception(
                    std::make_exception_ptr(std::runtime_error("Request expired"))
                );
            } catch (...) {
                // Promise already fulfilled
            }
            it = pending_requests_.erase(it);
        } else {
            ++it;
        }
    }
}

// ============================================================================
// Handshake Protocol
// ============================================================================

Result<void> MultiplexedPeer::send_handshake(const std::string& peer_id) {
    Envelope handshake;
    handshake.set_from(peer_id_);
    handshake.set_to(peer_id);
    handshake.set_operation(OperationType::CONTROL);
    handshake.set_message_id("handshake-" + generate_peer_id());

    auto caps = handshake.get_capabilities();
    caps["command"] = "handshake";
    caps["peer_id"] = peer_id_;
    handshake.set_capabilities(caps);

    return send_to_peer(peer_id, handshake);
}

Result<void> MultiplexedPeer::handle_handshake(const std::string& peer_id, const Envelope& envelope) {
    // Send ACK
    Envelope ack;
    ack.set_from(peer_id_);
    ack.set_to(envelope.get_from());
    ack.set_operation(OperationType::ACK);
    ack.set_message_id("ack-" + envelope.get_message_id());

    return send_to_peer(peer_id, ack);
}

} // namespace umicp


