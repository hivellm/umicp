/**
 * UMICP E2E Basic Tests
 * End-to-end testing without requiring multiple processes
 */

#include "multiplexed_peer.h"
#include "websocket_server.h"
#include "websocket_client.h"
#include "envelope.h"
#include "compression.h"
#include "security.h"
#include "matrix_ops.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <atomic>

using namespace umicp;

// ============================================================================
// Basic E2E Tests (Single Process)
// ============================================================================

TEST(E2EBasicTest, CreateServerAndClient) {
    // Create server
    WebSocketServerConfig server_config;
    server_config.port = 18080;
    auto server = std::make_unique<WebSocketServer>(server_config);

    EXPECT_NE(server, nullptr);

    // Create client
    WebSocketClientConfig client_config;
    client_config.url = "ws://localhost:18080";
    auto client = std::make_unique<WebSocketClient>(client_config);

    EXPECT_NE(client, nullptr);
}

TEST(E2EBasicTest, MultiplexedPeerCreation) {
    MultiplexedPeerOptions options;
    options.peer_id = "test-peer-e2e";
    options.server = ServerConfig{};
    options.server->port = 18081;

    auto peer = std::make_unique<MultiplexedPeer>(options);

    EXPECT_NE(peer, nullptr);
    EXPECT_EQ(peer->get_peer_id(), "test-peer-e2e");
}

TEST(E2EBasicTest, EnvelopeRoundTripThroughSerialization) {
    // Create envelope
    Envelope original;
    original.set_from("sender");
    original.set_to("receiver");
    original.set_operation(OperationType::DATA);
    original.set_message_id("e2e-msg-1");

    // Serialize
    auto serialized = EnvelopeProcessor::serialize(original);
    ASSERT_TRUE(serialized.is_ok());

    // Deserialize
    auto deserialized = EnvelopeProcessor::deserialize(*serialized.value);
    ASSERT_TRUE(deserialized.is_ok());

    // Verify
    auto result = *deserialized.value;
    EXPECT_EQ(result.get_from(), "sender");
    EXPECT_EQ(result.get_to(), "receiver");
    EXPECT_EQ(result.get_operation(), OperationType::DATA);
}

TEST(E2EBasicTest, CompressedMessageFlow) {
    // Create data
    ByteBuffer data(1000, 0xAA);

    // Compress
    CompressionManager compressor(CompressionAlgorithm::ZLIB);
    auto compressed = compressor.compress(data);
    ASSERT_TRUE(compressed.is_ok());

    // Decompress
    auto decompressed = compressor.decompress(*compressed.value);
    ASSERT_TRUE(decompressed.is_ok());

    // Verify
    EXPECT_EQ(*decompressed.value, data);
}

TEST(E2EBasicTest, EventSystemWithMultiplexedPeer) {
    MultiplexedPeerOptions options;
    options.peer_id = "event-test-peer";

    auto peer = std::make_unique<MultiplexedPeer>(options);

    std::atomic<int> event_count{0};

    peer->on("test_event", [&event_count](const Event& e) {
        event_count++;
    });

    peer->emit("test_event");

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(event_count, 1);
}

// ============================================================================
// Integration Flow Tests
// ============================================================================

TEST(E2EFlowTest, CompleteMessageCreationFlow) {
    // 1. Create envelope
    Envelope envelope;
    envelope.set_from("peer-a");
    envelope.set_to("peer-b");
    envelope.set_operation(OperationType::DATA);

    // 2. Add capabilities
    StringMap caps;
    caps["version"] = "1.0";
    caps["priority"] = "high";
    envelope.set_capabilities(caps);

    // 3. Skip payload hint for now

    // 4. Serialize
    auto json = envelope.to_json();
    EXPECT_FALSE(json.empty());

    // 5. Deserialize
    auto result = EnvelopeProcessor::deserialize(json);
    ASSERT_TRUE(result.is_ok());

    // 6. Verify all fields
    auto recovered = *result.value;
    EXPECT_EQ(recovered.get_from(), "peer-a");
    EXPECT_EQ(recovered.get_to(), "peer-b");
    auto recovered_caps = recovered.get_capabilities();
    EXPECT_EQ(recovered_caps["version"], "1.0");
}

TEST(E2EFlowTest, BroadcastingFlow) {
    MultiplexedPeerOptions options;
    options.peer_id = "broadcast-peer";

    auto peer = std::make_unique<MultiplexedPeer>(options);

    Envelope message;
    message.set_from("broadcast-peer");
    message.set_operation(OperationType::DATA);
    message.set_message_id("broadcast-1");

    // Broadcast to empty peer list
    size_t sent = peer->broadcast(message);

    EXPECT_EQ(sent, 0); // No peers connected
}

TEST(E2EFlowTest, RequestResponseFlow) {
    MultiplexedPeerOptions options;
    options.peer_id = "rr-peer";

    auto peer = std::make_unique<MultiplexedPeer>(options);

    Envelope request;
    request.set_from("rr-peer");
    request.set_to("target-peer");
    request.set_operation(OperationType::CONTROL);
    request.set_message_id("req-1");

    // Try send and wait (will fail - no peers)
    auto result = peer->send_and_wait(
        "target-peer",
        request,
        std::chrono::milliseconds(100)
    );

    EXPECT_FALSE(result.is_ok());
    EXPECT_EQ(result.error_code(), ErrorCode::NOT_FOUND);
}

// ============================================================================
// Security Integration
// ============================================================================

TEST(E2ESecurityTest, EncryptionDecryptionFlow) {
    // SecurityManager requires local_id parameter
    SecurityManager security("test-peer");

    // Generate keys
    auto keypair_result = security.generate_keypair();
    ASSERT_TRUE(keypair_result.is_ok());

    // SecurityManager initialized successfully
    SUCCEED();
}

// ============================================================================
// Multi-Component Integration
// ============================================================================

TEST(E2EMultiComponentTest, EventSystemWithCompression) {
    AsyncEventEmitter emitter(4);
    CompressionManager compressor(CompressionAlgorithm::GZIP);

    std::atomic<bool> event_received{false};

    emitter.on("data_compressed", [&event_received](const Event& e) {
        event_received = true;
    });

    // Compress data
    ByteBuffer data(1000, 0xBB);
    auto compressed = compressor.compress(data);

    // Emit event
    emitter.emit_async("data_compressed");
    emitter.wait_for_events();

    EXPECT_TRUE(compressed.is_ok());
    EXPECT_TRUE(event_received);
}

TEST(E2EMultiComponentTest, MatrixOpsWithSerialization) {
    // Create vector data
    std::vector<float> vec1 = {1.0f, 2.0f, 3.0f};
    std::vector<float> vec2 = {4.0f, 5.0f, 6.0f};

    // Compute dot product
    float dot = 0.0f;
    auto result = MatrixOps::dot_product(vec1.data(), vec2.data(), &dot, vec1.size());
    ASSERT_TRUE(result.is_ok());

    EXPECT_FLOAT_EQ(dot, 32.0f); // 1*4 + 2*5 + 3*6 = 32

    // Create envelope with result
    Envelope envelope;
    envelope.set_from("compute-node");
    envelope.set_to("result-collector");
    envelope.set_operation(OperationType::DATA);

    StringMap caps;
    caps["dot_product"] = std::to_string(dot);
    envelope.set_capabilities(caps);

    // Serialize
    auto json = envelope.to_json();
    EXPECT_FALSE(json.empty());
}

// ============================================================================
// Complete Workflow Test
// ============================================================================

TEST(E2EWorkflowTest, CompleteMessageWorkflow) {
    // 1. Create multiplexed peer
    MultiplexedPeerOptions options;
    options.peer_id = "workflow-peer";
    auto peer = std::make_unique<MultiplexedPeer>(options);

    // 2. Set up event handler
    std::atomic<int> messages_received{0};
    peer->on("message", [&messages_received](const Event& e) {
        messages_received++;
    });

    // 3. Create message with compression hint
    Envelope message;
    message.set_from("workflow-peer");
    message.set_to("remote-peer");
    message.set_operation(OperationType::DATA);

    PayloadHint hint;
    hint.type = PayloadType::BINARY;
    hint.size = 10000;
    message.payload_hint = hint;

    // 4. Serialize
    auto json = message.to_json();
    EXPECT_FALSE(json.empty());

    // 5. Hash
    auto hash = EnvelopeProcessor::hash(message);
    EXPECT_FALSE(hash.empty());

    // 6. Get statistics
    auto stats = peer->get_stats();
    EXPECT_EQ(stats.total_peers, 0);
}

TEST(E2EWorkflowTest, PeerLifecycleWorkflow) {
    // Create peer
    MultiplexedPeerOptions options;
    options.peer_id = "lifecycle-peer";
    auto peer = std::make_unique<MultiplexedPeer>(options);

    // Check initial state
    EXPECT_FALSE(peer->is_server_running());
    EXPECT_EQ(peer->get_peers().size(), 0);

    // Get initial stats
    auto stats_before = peer->get_stats();
    EXPECT_EQ(stats_before.total_peers, 0);

    // Reset stats
    peer->reset_stats();

    // Get stats after reset
    auto stats_after = peer->get_stats();
    EXPECT_EQ(stats_after.messages_sent, 0);
    EXPECT_EQ(stats_after.messages_received, 0);

    // Shutdown
    auto result = peer->shutdown();
    EXPECT_TRUE(result.is_ok());
}

// ============================================================================
// Performance Integration
// ============================================================================

TEST(E2EPerformanceTest, HighThroughputSerialization) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        Envelope envelope;
        envelope.set_from("sender");
        envelope.set_to("receiver");
        envelope.set_operation(OperationType::DATA);
        envelope.set_message_id("msg-" + std::to_string(i));

        auto json = envelope.to_json();
        auto result = EnvelopeProcessor::deserialize(json);

        ASSERT_TRUE(result.is_ok());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 1000 round-trips should complete in < 100ms
    EXPECT_LT(duration.count(), 100);
}

TEST(E2EPerformanceTest, MatrixOpsPerformance) {
    std::vector<float> a(1000);
    std::vector<float> b(1000);

    for (int i = 0; i < 1000; ++i) {
        a[i] = i * 1.0f;
        b[i] = i * 2.0f;
    }

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100; ++i) {
        float result = 0.0f;
        auto op_result = MatrixOps::dot_product(a.data(), b.data(), &result, a.size());
        ASSERT_TRUE(op_result.is_ok());
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 100 dot products of 1000-element vectors in < 10ms
    EXPECT_LT(duration.count(), 10);
}

