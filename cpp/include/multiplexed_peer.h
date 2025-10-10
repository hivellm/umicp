/**
 * UMICP Multiplexed Peer
 * True P2P architecture - each peer can act as both server and client
 * Inspired by TypeScript implementation
 */

#ifndef UMICP_MULTIPLEXED_PEER_H
#define UMICP_MULTIPLEXED_PEER_H

#include "umicp_types.h"
#include "event_emitter.h"
#include "envelope.h"
#include "transport.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <optional>
#include <future>

namespace umicp {

// Forward declarations
class WebSocketServer;
class WebSocketClient;

/**
 * Connection type
 */
enum class ConnectionType {
    INCOMING,  // Connection initiated by remote peer
    OUTGOING   // Connection initiated by this peer
};

/**
 * Peer connection information
 */
struct PeerConnection {
    std::string id;
    ConnectionType type;
    std::optional<std::string> url;
    std::shared_ptr<Transport> transport;
    std::unordered_map<std::string, std::string> metadata;
    std::chrono::system_clock::time_point connected_at;
    bool is_connected;

    PeerConnection(
        std::string peer_id,
        ConnectionType conn_type,
        std::shared_ptr<Transport> trans
    ) : id(std::move(peer_id)),
        type(conn_type),
        transport(std::move(trans)),
        connected_at(std::chrono::system_clock::now()),
        is_connected(true) {}
};

/**
 * Server configuration for multiplexed peer
 */
struct ServerConfig {
    uint16_t port = 0;
    std::string path = "/umicp";
    bool compression = true;
    size_t max_payload = 100 * 1024 * 1024; // 100MB
    std::chrono::milliseconds ping_interval = std::chrono::milliseconds(30000);
    std::chrono::milliseconds connection_timeout = std::chrono::milliseconds(60000);
};

/**
 * Multiplexed peer options
 */
struct MultiplexedPeerOptions {
    std::string peer_id;
    std::optional<ServerConfig> server;

    // Callbacks (optional, can use event emitter instead)
    std::function<void(const Envelope&, const PeerConnection&)> on_message;
    std::function<void(const PeerConnection&)> on_peer_connect;
    std::function<void(const PeerConnection&)> on_peer_disconnect;
    std::function<void(const std::string&, const PeerConnection*)> on_error;
};

/**
 * MultiplexedPeer class
 *
 * Each peer can:
 * - Accept incoming connections (server component)
 * - Connect to multiple remote peers (client component)
 * - Send messages to specific peers or broadcast
 * - Handle messages from any connected peer
 *
 * Events emitted:
 * - "message" - When message received from any peer
 * - "peer:connect" - When new peer connects
 * - "peer:disconnect" - When peer disconnects
 * - "error" - When error occurs
 */
class MultiplexedPeer : public EventEmitter {
public:
    explicit MultiplexedPeer(const MultiplexedPeerOptions& options);
    ~MultiplexedPeer() override;

    // Server component - accepts incoming connections
    Result<void> start_server();
    Result<void> stop_server();
    bool is_server_running() const;

    // Client component - connect to remote peers
    Result<std::string> connect_to_peer(
        const std::string& url,
        const std::unordered_map<std::string, std::string>& metadata = {}
    );

    // Peer management
    Result<void> disconnect_peer(const std::string& peer_id);
    Result<void> disconnect_peer_by_url(const std::string& url);

    // Message sending
    Result<void> send_to_peer(const std::string& peer_id, const Envelope& envelope);
    Result<void> send_to_peer_by_url(const std::string& url, const Envelope& envelope);

    // Broadcast to multiple peers
    size_t broadcast(const Envelope& envelope, const std::string& exclude_peer_id = "");
    size_t broadcast_to_type(
        const Envelope& envelope,
        ConnectionType type,
        const std::string& exclude_peer_id = ""
    );

    // Request-response pattern
    Result<Envelope> send_and_wait(
        const std::string& peer_id,
        const Envelope& envelope,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)
    );

    // Peer queries
    std::vector<PeerConnection> get_peers() const;
    std::vector<PeerConnection> get_peers_by_type(ConnectionType type) const;
    std::optional<PeerConnection> get_peer(const std::string& peer_id) const;
    std::optional<PeerConnection> find_peer_by_metadata(
        const std::string& key,
        const std::string& value
    ) const;

    // Statistics
    struct PeerStats {
        size_t total_peers = 0;
        size_t incoming_connections = 0;
        size_t outgoing_connections = 0;
        size_t messages_sent = 0;
        size_t messages_received = 0;
    };

    PeerStats get_stats() const;
    void reset_stats();

    // Lifecycle
    Result<void> shutdown();

    // Get peer ID
    std::string get_peer_id() const { return peer_id_; }

private:
    std::string peer_id_;
    MultiplexedPeerOptions options_;

    // Server component
    std::unique_ptr<WebSocketServer> server_;
    bool server_running_;

    // Peer map (unified for incoming and outgoing)
    mutable std::mutex peers_mutex_;
    std::unordered_map<std::string, std::shared_ptr<PeerConnection>> peers_;

    // Statistics
    mutable std::mutex stats_mutex_;
    PeerStats stats_;

    // Request-response tracking
    struct PendingRequest {
        std::string request_id;
        std::promise<Envelope> response_promise;
        std::chrono::steady_clock::time_point expiry;
    };

    mutable std::mutex pending_requests_mutex_;
    std::unordered_map<std::string, std::shared_ptr<PendingRequest>> pending_requests_;

    // Internal methods
    std::string generate_peer_id();
    void handle_incoming_connection(std::shared_ptr<Transport> transport);
    void handle_message(const std::string& peer_id, const Envelope& envelope);
    void handle_peer_disconnected(const std::string& peer_id);
    void handle_error(const std::string& error, const std::string* peer_id = nullptr);

    // Request-response helpers
    void register_pending_request(const std::string& request_id, std::shared_ptr<PendingRequest> request);
    std::optional<std::shared_ptr<PendingRequest>> get_pending_request(const std::string& request_id);
    void cleanup_expired_requests();

    // Handshake protocol
    Result<void> send_handshake(const std::string& peer_id);
    Result<void> handle_handshake(const std::string& peer_id, const Envelope& envelope);
};

/**
 * Helper function to create a multiplexed peer with server
 */
inline std::unique_ptr<MultiplexedPeer> create_multiplexed_peer(
    const std::string& peer_id,
    uint16_t port,
    const std::string& path = "/umicp"
) {
    MultiplexedPeerOptions options;
    options.peer_id = peer_id;
    options.server = ServerConfig{};
    options.server->port = port;
    options.server->path = path;

    return std::make_unique<MultiplexedPeer>(options);
}

/**
 * Helper function to create a client-only multiplexed peer
 */
inline std::unique_ptr<MultiplexedPeer> create_multiplexed_client(
    const std::string& peer_id
) {
    MultiplexedPeerOptions options;
    options.peer_id = peer_id;
    // No server component

    return std::make_unique<MultiplexedPeer>(options);
}

} // namespace umicp

#endif // UMICP_MULTIPLEXED_PEER_H

