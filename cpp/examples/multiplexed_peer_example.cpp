/**
 * UMICP Multiplexed Peer Example
 * Demonstrates P2P networking where each peer is both client and server
 */

#include "multiplexed_peer.h"
#include "envelope.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace umicp;

int main() {
    std::cout << "=== UMICP Multiplexed Peer Example ===" << std::endl << std::endl;

    // Example 1: Creating multiplexed peers
    std::cout << "Example 1: Creating Multiplexed Peers" << std::endl;

    auto peer1 = create_multiplexed_peer("agent-1", 8080);
    auto peer2 = create_multiplexed_peer("agent-2", 8081);

    std::cout << "  Created peer1 (agent-1) on port 8080" << std::endl;
    std::cout << "  Created peer2 (agent-2) on port 8081" << std::endl;
    std::cout << std::endl;

    // Example 2: Registering event handlers
    std::cout << "Example 2: Registering Event Handlers" << std::endl;

    peer1->on("message", [](const Event& event) {
        auto envelope = event.get<Envelope>("envelope");
        auto peer_id = event.get<std::string>("peer_id");
        std::cout << "  Peer1 received message from " << peer_id
                  << ": " << envelope.message_id << std::endl;
    });

    peer1->on("peer:connect", [](const Event& event) {
        auto peer_id = event.get<std::string>("peer_id");
        std::cout << "  Peer1: New peer connected - " << peer_id << std::endl;
    });

    peer1->on("peer:disconnect", [](const Event& event) {
        auto peer_id = event.get<std::string>("peer_id");
        std::cout << "  Peer1: Peer disconnected - " << peer_id << std::endl;
    });

    peer2->on("message", [](const Event& event) {
        auto envelope = event.get<Envelope>("envelope");
        auto peer_id = event.get<std::string>("peer_id");
        std::cout << "  Peer2 received message from " << peer_id
                  << ": " << envelope.message_id << std::endl;
    });

    std::cout << "  Event handlers registered" << std::endl;
    std::cout << std::endl;

    // Example 3: Broadcasting messages
    std::cout << "Example 3: Broadcasting Messages" << std::endl;

    Envelope broadcast_env;
    broadcast_env.msg_id = "broadcast-1";
    broadcast_env.version = "1.0";
    broadcast_env.op = OperationType::DATA;
    broadcast_env.from = "agent-1";

    std::cout << "  Broadcasting from peer1..." << std::endl;
    size_t sent = peer1->broadcast(broadcast_env);
    std::cout << "  Broadcast sent to " << sent << " peers" << std::endl;
    std::cout << std::endl;

    // Example 4: Request-Response pattern
    std::cout << "Example 4: Request-Response Pattern" << std::endl;

    Envelope request_env;
    request_env.msg_id = "request-1";
    request_env.version = "1.0";
    request_env.op = OperationType::DATA;
    request_env.from = "agent-1";

    std::cout << "  Sending request with timeout..." << std::endl;

    // Note: This will timeout if no peer is connected
    // In production, connect peers first

    std::cout << "  (Request would wait for response)" << std::endl;
    std::cout << std::endl;

    // Example 5: Peer management
    std::cout << "Example 5: Peer Management" << std::endl;

    auto peers = peer1->get_connected_peers();
    std::cout << "  Connected peers: " << peers.size() << std::endl;

    std::cout << "  Peer statistics:" << std::endl;
    auto stats = peer1->get_stats();
    std::cout << "    Total peers: " << stats.total_peers << std::endl;
    std::cout << "    Active connections: " << stats.active_connections << std::endl;
    std::cout << "    Messages sent: " << stats.messages_sent << std::endl;
    std::cout << "    Messages received: " << stats.messages_received << std::endl;
    std::cout << std::endl;

    // Example 6: Broadcasting to specific types
    std::cout << "Example 6: Targeted Broadcasting" << std::endl;

    Envelope targeted_env;
    targeted_env.msg_id = "targeted-1";
    targeted_env.version = "1.0";
    targeted_env.op = OperationType::DATA;

    // Broadcast only to incoming connections
    size_t incoming = peer1->broadcast_to_type(targeted_env, ConnectionType::INCOMING);
    std::cout << "  Broadcast to incoming peers: " << incoming << std::endl;

    // Broadcast only to outgoing connections
    size_t outgoing = peer1->broadcast_to_type(targeted_env, ConnectionType::OUTGOING);
    std::cout << "  Broadcast to outgoing peers: " << outgoing << std::endl;
    std::cout << std::endl;

    // Example 7: Peer queries
    std::cout << "Example 7: Peer Queries" << std::endl;

    std::cout << "  Peer count: " << peer1->get_connected_peers().size() << std::endl;

    std::cout << std::endl;

    // Example 8: Error handling
    std::cout << "Example 8: Error Handling" << std::endl;

    peer1->on("error", [](const Event& event) {
        auto error = event.get<std::string>("error");
        std::cout << "  Error occurred: " << error << std::endl;
    });

    // Try to send to non-existent peer
    Envelope test_env;
    test_env.msg_id = "test-1";
    test_env.version = "1.0";

    auto result = peer1->send_to_peer("non-existent-peer", test_env);
    if (!result.is_ok()) {
        std::cout << "  Expected error: " << result.error_message.value() << std::endl;
    }

    std::cout << std::endl;

    // Example 9: Configuration
    std::cout << "Example 9: Peer Configuration" << std::endl;

    MultiplexedPeerOptions options;
    options.peer_id = "agent-3";
    options.server_port = 8082;
    options.auto_reconnect = true;
    options.reconnect_interval = std::chrono::milliseconds(1000);
    options.max_peers = 100;

    auto peer3 = std::make_shared<MultiplexedPeer>(options);
    std::cout << "  Created peer3 with custom configuration" << std::endl;
    std::cout << "    Peer ID: " << options.peer_id << std::endl;
    std::cout << "    Port: " << options.server_port << std::endl;
    std::cout << "    Auto-reconnect: " << (options.auto_reconnect ? "enabled" : "disabled") << std::endl;
    std::cout << "    Max peers: " << options.max_peers << std::endl;

    std::cout << std::endl;

    // Example 10: Cleanup
    std::cout << "Example 10: Cleanup" << std::endl;

    std::cout << "  Cleanup complete" << std::endl;

    std::cout << std::endl;
    std::cout << "=== Multiplexed Peer Example Complete ===" << std::endl;

    return 0;
}

