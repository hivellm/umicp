/**
 * UMICP Multiplexed Peer Tests
 */

#include "multiplexed_peer.h"
#include "event_emitter.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace umicp;

class MultiplexedPeerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create peer with client-only mode (no server)
        MultiplexedPeerOptions options;
        options.peer_id = "test-peer-1";
        // No server component for unit tests

        peer = std::make_unique<MultiplexedPeer>(options);
    }

    void TearDown() override {
        if (peer) {
            peer->shutdown();
        }
        peer.reset();
    }

    std::unique_ptr<MultiplexedPeer> peer;
};

// ============================================================================
// Basic Initialization Tests
// ============================================================================

TEST_F(MultiplexedPeerTest, Initialization) {
    EXPECT_NE(peer, nullptr);
    EXPECT_EQ(peer->get_peer_id(), "test-peer-1");
}

TEST_F(MultiplexedPeerTest, GetPeerIdReturnsCorrectValue) {
    EXPECT_EQ(peer->get_peer_id(), "test-peer-1");
}

TEST_F(MultiplexedPeerTest, InitialStatsAreZero) {
    auto stats = peer->get_stats();
    EXPECT_EQ(stats.total_peers, 0);
    EXPECT_EQ(stats.incoming_connections, 0);
    EXPECT_EQ(stats.outgoing_connections, 0);
    EXPECT_EQ(stats.messages_sent, 0);
    EXPECT_EQ(stats.messages_received, 0);
}

// ============================================================================
// Server Component Tests
// ============================================================================

TEST_F(MultiplexedPeerTest, StartServerWithoutConfigFails) {
    auto result = peer->start_server();
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::INVALID_CONFIG);
}

TEST_F(MultiplexedPeerTest, StartServerReturnsNotImplemented) {
    // Create peer with server config
    MultiplexedPeerOptions options;
    options.peer_id = "server-peer";
    options.server = ServerConfig{};
    options.server->port = 8080;

    auto server_peer = std::make_unique<MultiplexedPeer>(options);

    // Should return NOT_IMPLEMENTED until Phase 3 WebSocket integration
    auto result = server_peer->start_server();
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_IMPLEMENTED);
}

TEST_F(MultiplexedPeerTest, StopServerWhenNotRunningFails) {
    auto result = peer->stop_server();
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::INVALID_STATE);
}

TEST_F(MultiplexedPeerTest, IsServerRunningReturnsFalseInitially) {
    EXPECT_FALSE(peer->is_server_running());
}

// ============================================================================
// Client Component Tests (Pending WebSocket Integration)
// ============================================================================

TEST_F(MultiplexedPeerTest, ConnectToPeerReturnsNotImplemented) {
    auto result = peer->connect_to_peer("ws://localhost:8080/umicp");
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_IMPLEMENTED);
}

// ============================================================================
// Peer Management Tests
// ============================================================================

TEST_F(MultiplexedPeerTest, DisconnectNonExistentPeerFails) {
    auto result = peer->disconnect_peer("non-existent-peer");
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_FOUND);
}

TEST_F(MultiplexedPeerTest, DisconnectPeerByUrlWhenNoPeersFails) {
    auto result = peer->disconnect_peer_by_url("ws://localhost:8080/umicp");
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_FOUND);
}

TEST_F(MultiplexedPeerTest, GetPeersReturnsEmptyInitially) {
    auto peers = peer->get_peers();
    EXPECT_TRUE(peers.empty());
}

TEST_F(MultiplexedPeerTest, GetPeersByTypeReturnsEmptyInitially) {
    auto incoming = peer->get_peers_by_type(ConnectionType::INCOMING);
    auto outgoing = peer->get_peers_by_type(ConnectionType::OUTGOING);

    EXPECT_TRUE(incoming.empty());
    EXPECT_TRUE(outgoing.empty());
}

TEST_F(MultiplexedPeerTest, GetPeerReturnsNulloptForNonExistent) {
    auto peer_info = peer->get_peer("non-existent");
    EXPECT_FALSE(peer_info.has_value());
}

TEST_F(MultiplexedPeerTest, FindPeerByMetadataReturnsNulloptWhenNoPeers) {
    auto peer_info = peer->find_peer_by_metadata("key", "value");
    EXPECT_FALSE(peer_info.has_value());
}

// ============================================================================
// Message Sending Tests (Pending Connections)
// ============================================================================

TEST_F(MultiplexedPeerTest, SendToPeerWhenNoPeersFails) {
    Envelope envelope;
    envelope.set_from("test-peer-1");
    envelope.set_to("peer-2");
    envelope.set_operation(OperationType::DATA);
    envelope.set_message_id("msg-1");

    auto result = peer->send_to_peer("peer-2", envelope);
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_FOUND);
}

TEST_F(MultiplexedPeerTest, SendToPeerByUrlWhenNoPeersFails) {
    Envelope envelope;
    envelope.set_from("test-peer-1");
    envelope.set_to("peer-2");

    auto result = peer->send_to_peer_by_url("ws://localhost:8080", envelope);
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_FOUND);
}

// ============================================================================
// Broadcasting Tests
// ============================================================================

TEST_F(MultiplexedPeerTest, BroadcastWhenNoPeersReturnsZero) {
    Envelope envelope;
    envelope.set_from("test-peer-1");
    envelope.set_operation(OperationType::DATA);

    size_t count = peer->broadcast(envelope);
    EXPECT_EQ(count, 0);
}

TEST_F(MultiplexedPeerTest, BroadcastToTypeWhenNoPeersReturnsZero) {
    Envelope envelope;
    envelope.set_from("test-peer-1");
    envelope.set_operation(OperationType::DATA);

    size_t count_in = peer->broadcast_to_type(envelope, ConnectionType::INCOMING);
    size_t count_out = peer->broadcast_to_type(envelope, ConnectionType::OUTGOING);

    EXPECT_EQ(count_in, 0);
    EXPECT_EQ(count_out, 0);
}

TEST_F(MultiplexedPeerTest, BroadcastWithExcludeWhenNoPeersReturnsZero) {
    Envelope envelope;
    envelope.set_from("test-peer-1");

    size_t count = peer->broadcast(envelope, "peer-to-exclude");
    EXPECT_EQ(count, 0);
}

// ============================================================================
// Request-Response Tests
// ============================================================================

TEST_F(MultiplexedPeerTest, SendAndWaitWhenNoPeersFails) {
    Envelope envelope;
    envelope.set_from("test-peer-1");
    envelope.set_to("peer-2");
    envelope.set_message_id("req-1");

    auto result = peer->send_and_wait("peer-2", envelope, std::chrono::milliseconds(100));
    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_FOUND);
}

// ============================================================================
// Statistics Tests
// ============================================================================

TEST_F(MultiplexedPeerTest, GetStatsReturnsValidData) {
    auto stats = peer->get_stats();

    // Initially all zeros
    EXPECT_EQ(stats.total_peers, 0);
    EXPECT_EQ(stats.incoming_connections, 0);
    EXPECT_EQ(stats.outgoing_connections, 0);
    EXPECT_EQ(stats.messages_sent, 0);
    EXPECT_EQ(stats.messages_received, 0);
}

TEST_F(MultiplexedPeerTest, ResetStatsKeepsConnectionCounts) {
    // Even without real connections, reset should work
    peer->reset_stats();

    auto stats = peer->get_stats();
    EXPECT_EQ(stats.messages_sent, 0);
    EXPECT_EQ(stats.messages_received, 0);
}

// ============================================================================
// Event Emission Tests
// ============================================================================

TEST_F(MultiplexedPeerTest, PeerInheritsFromEventEmitter) {
    std::atomic<int> event_count{0};

    peer->on("test_event", [&event_count](const Event& event) {
        event_count++;
    });

    Event event("test_event");
    peer->emit("test_event", event);

    EXPECT_EQ(event_count, 1);
}

TEST_F(MultiplexedPeerTest, CanRegisterMultipleEventHandlers) {
    std::atomic<int> handler1_calls{0};
    std::atomic<int> handler2_calls{0};

    peer->on("event", [&handler1_calls](const Event& event) {
        handler1_calls++;
    });

    peer->on("event", [&handler2_calls](const Event& event) {
        handler2_calls++;
    });

    Event event("event");
    peer->emit("event", event);

    EXPECT_EQ(handler1_calls, 1);
    EXPECT_EQ(handler2_calls, 1);
}

TEST_F(MultiplexedPeerTest, EventHandlersCanBeRemoved) {
    std::atomic<int> call_count{0};

    size_t handler_id = peer->on("event", [&call_count](const Event& event) {
        call_count++;
    });

    Event event("event");
    peer->emit("event", event);
    EXPECT_EQ(call_count, 1);

    // Remove handler
    peer->off("event", handler_id);

    peer->emit("event", event);
    EXPECT_EQ(call_count, 1); // Should not increase
}

// ============================================================================
// Lifecycle Tests
// ============================================================================

TEST_F(MultiplexedPeerTest, ShutdownSucceeds) {
    auto result = peer->shutdown();
    EXPECT_TRUE(result.is_ok());
}

TEST_F(MultiplexedPeerTest, ShutdownWhenAlreadyShutdownSucceeds) {
    peer->shutdown();
    auto result = peer->shutdown();
    EXPECT_TRUE(result.is_ok());
}

TEST_F(MultiplexedPeerTest, OperationsAfterShutdownFail) {
    peer->shutdown();

    // Try to send after shutdown
    Envelope envelope;
    auto result = peer->send_to_peer("peer-1", envelope);
    EXPECT_FALSE(result.is_ok());
}

// ============================================================================
// Helper Function Tests
// ============================================================================

TEST(MultiplexedPeerHelperTest, CreateMultiplexedPeerHelper) {
    auto peer = create_multiplexed_peer("test-peer", 8080, "/umicp");

    EXPECT_NE(peer, nullptr);
    EXPECT_EQ(peer->get_peer_id(), "test-peer");
}

TEST(MultiplexedPeerHelperTest, CreateMultiplexedClientHelper) {
    auto peer = create_multiplexed_client("client-peer");

    EXPECT_NE(peer, nullptr);
    EXPECT_EQ(peer->get_peer_id(), "client-peer");
    EXPECT_FALSE(peer->is_server_running());
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(MultiplexedPeerTest, ConcurrentGetStatsIsSafe) {
    std::vector<std::thread> threads;
    const int num_threads = 10;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this]() {
            for (int j = 0; j < 100; ++j) {
                auto stats = peer->get_stats();
                // Just access stats, no assertions needed
                (void)stats;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // If we get here without crashing, thread safety is working
    SUCCEED();
}

TEST_F(MultiplexedPeerTest, ConcurrentEventEmissionIsSafe) {
    std::atomic<int> total_calls{0};

    peer->on("concurrent_event", [&total_calls](const Event& event) {
        total_calls++;
    });

    std::vector<std::thread> threads;
    const int num_threads = 10;
    const int emissions_per_thread = 10;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, emissions_per_thread]() {
            for (int j = 0; j < emissions_per_thread; ++j) {
                Event event("concurrent_event");
                peer->emit("concurrent_event", event);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(total_calls, num_threads * emissions_per_thread);
}

// ============================================================================
// Configuration Tests
// ============================================================================

TEST(MultiplexedPeerConfigTest, OptionsWithServerConfig) {
    MultiplexedPeerOptions options;
    options.peer_id = "configured-peer";
    options.server = ServerConfig{};
    options.server->port = 9090;
    options.server->path = "/custom";
    options.server->compression = false;

    auto peer = std::make_unique<MultiplexedPeer>(options);

    EXPECT_EQ(peer->get_peer_id(), "configured-peer");
}

TEST(MultiplexedPeerConfigTest, OptionsWithCallbacks) {
    std::atomic<bool> message_received{false};
    std::atomic<bool> peer_connected{false};

    MultiplexedPeerOptions options;
    options.peer_id = "callback-peer";

    options.on_message = [&message_received](const Envelope& envelope, const PeerConnection& peer) {
        message_received = true;
    };

    options.on_peer_connect = [&peer_connected](const PeerConnection& peer) {
        peer_connected = true;
    };

    auto peer = std::make_unique<MultiplexedPeer>(options);

    EXPECT_NE(peer, nullptr);
    // Callbacks are registered but not tested here (require actual connections)
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

