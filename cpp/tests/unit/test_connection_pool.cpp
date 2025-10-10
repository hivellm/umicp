/**
 * @file test_connection_pool.cpp
 * @brief Unit tests for Connection Pool
 */

#include <gtest/gtest.h>
#include "connection_pool.h"
#include "websocket_client.h"
#include <thread>
#include <chrono>

using namespace umicp;

class ConnectionPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        config.address = "ws://localhost:8080";
        config.min_size = 2;
        config.max_size = 5;
        config.max_age_seconds = 10;
        config.idle_timeout_seconds = 5;
        config.acquire_timeout_ms = 1000;
    }

    void TearDown() override {
        // Cleanup
    }

    PoolConfig config;
};

// ============================================================================
// PooledConnection Tests
// ============================================================================

TEST_F(ConnectionPoolTest, PooledConnectionCreation) {
    WebSocketClientConfig ws_config;
    ws_config.url = "ws://localhost:8080";
    auto client = std::make_shared<WebSocketClient>(ws_config);
    PooledConnection conn("ws://localhost:8080", client);

    EXPECT_FALSE(conn.getId().empty());
    EXPECT_EQ(conn.getAddress(), "ws://localhost:8080");
    EXPECT_EQ(conn.getState(), PoolConnectionState::Available);
    EXPECT_EQ(conn.getUseCount(), 0);
    EXPECT_NE(conn.getClient(), nullptr);
}

TEST_F(ConnectionPoolTest, PooledConnectionAcquireRelease) {
    WebSocketClientConfig ws_config;
    ws_config.url = "ws://localhost:8080";
    auto client = std::make_shared<WebSocketClient>(ws_config);
    PooledConnection conn("ws://localhost:8080", client);

    EXPECT_EQ(conn.getState(), PoolConnectionState::Available);
    EXPECT_EQ(conn.getUseCount(), 0);

    conn.acquire();
    EXPECT_EQ(conn.getState(), PoolConnectionState::InUse);
    EXPECT_EQ(conn.getUseCount(), 1);

    conn.release();
    EXPECT_EQ(conn.getState(), PoolConnectionState::Available);
    EXPECT_EQ(conn.getUseCount(), 1); // Use count doesn't reset

    conn.acquire();
    EXPECT_EQ(conn.getUseCount(), 2);
}

TEST_F(ConnectionPoolTest, PooledConnectionClose) {
    WebSocketClientConfig ws_config;
    ws_config.url = "ws://localhost:8080";
    auto client = std::make_shared<WebSocketClient>(ws_config);
    PooledConnection conn("ws://localhost:8080", client);

    bool closed = conn.close();
    EXPECT_TRUE(closed);
    EXPECT_EQ(conn.getState(), PoolConnectionState::Closed);
}

TEST_F(ConnectionPoolTest, PooledConnectionStaleDetection) {
    WebSocketClientConfig ws_config;
    ws_config.url = "ws://localhost:8080";
    auto client = std::make_shared<WebSocketClient>(ws_config);
    PooledConnection conn("ws://localhost:8080", client);

    // Fresh connection should not be stale
    EXPECT_FALSE(conn.isStale(10));

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Should be stale with 1 second max age
    EXPECT_TRUE(conn.isStale(1));

    // Should not be stale with 10 second max age
    EXPECT_FALSE(conn.isStale(10));
}

TEST_F(ConnectionPoolTest, PooledConnectionIdleDetection) {
    WebSocketClientConfig ws_config;
    ws_config.url = "ws://localhost:8080";
    auto client = std::make_shared<WebSocketClient>(ws_config);
    PooledConnection conn("ws://localhost:8080", client);

    // Fresh connection should not be idle
    EXPECT_FALSE(conn.isIdle(10));

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Should be idle with 1 second timeout
    EXPECT_TRUE(conn.isIdle(1));

    // Should not be idle with 10 second timeout
    EXPECT_FALSE(conn.isIdle(10));

    // Using connection resets idle timer
    conn.acquire();
    EXPECT_FALSE(conn.isIdle(1));
}

// ============================================================================
// ConnectionPool Tests
// ============================================================================

TEST_F(ConnectionPoolTest, PoolCreation) {
    ConnectionPool pool(config);

    EXPECT_EQ(pool.getConfig().address, "ws://localhost:8080");
    EXPECT_EQ(pool.getConfig().min_size, 2);
    EXPECT_EQ(pool.getConfig().max_size, 5);
    EXPECT_FALSE(pool.isClosed());

    auto stats = pool.getStats();
    EXPECT_EQ(stats.total_connections, 0);
}

TEST_F(ConnectionPoolTest, PoolConfigValidation) {
    // Test invalid min/max
    PoolConfig bad_config;
    bad_config.address = "ws://localhost:8080";
    bad_config.min_size = 10;
    bad_config.max_size = 5;

    ConnectionPool pool(bad_config);

    // Min should be clamped to max
    EXPECT_LE(pool.getConfig().min_size, pool.getConfig().max_size);
}

TEST_F(ConnectionPoolTest, PoolStats) {
    ConnectionPool pool(config);

    auto stats = pool.getStats();
    EXPECT_EQ(stats.total_connections, 0);
    EXPECT_EQ(stats.available_connections, 0);
    EXPECT_EQ(stats.in_use_connections, 0);
    EXPECT_EQ(stats.total_acquires, 0);
    EXPECT_EQ(stats.total_releases, 0);
    EXPECT_EQ(stats.total_creates, 0);
    EXPECT_EQ(stats.total_closes, 0);
}

// Note: The following tests would require a real WebSocket server
// or mocking infrastructure. They test the structure and basic logic.

TEST_F(ConnectionPoolTest, PoolShutdown) {
    ConnectionPool pool(config);

    EXPECT_FALSE(pool.isClosed());

    pool.shutdown();

    EXPECT_TRUE(pool.isClosed());

    auto stats = pool.getStats();
    EXPECT_EQ(stats.total_connections, 0);
    EXPECT_EQ(stats.available_connections, 0);
    EXPECT_EQ(stats.in_use_connections, 0);
}

TEST_F(ConnectionPoolTest, PoolDoubleShutdown) {
    ConnectionPool pool(config);

    pool.shutdown();
    EXPECT_TRUE(pool.isClosed());

    // Second shutdown should be safe
    pool.shutdown();
    EXPECT_TRUE(pool.isClosed());
}

TEST_F(ConnectionPoolTest, PoolConfigStructure) {
    PoolConfig cfg;
    cfg.address = "ws://test:9000";
    cfg.min_size = 3;
    cfg.max_size = 20;
    cfg.max_age_seconds = 300;
    cfg.idle_timeout_seconds = 120;
    cfg.acquire_timeout_ms = 3000;

    EXPECT_EQ(cfg.address, "ws://test:9000");
    EXPECT_EQ(cfg.min_size, 3);
    EXPECT_EQ(cfg.max_size, 20);
    EXPECT_EQ(cfg.max_age_seconds, 300);
    EXPECT_EQ(cfg.idle_timeout_seconds, 120);
    EXPECT_EQ(cfg.acquire_timeout_ms, 3000);
}

TEST_F(ConnectionPoolTest, PoolStateEnum) {
    // Test enum values
    EXPECT_NE(PoolConnectionState::Available, PoolConnectionState::InUse);
    EXPECT_NE(PoolConnectionState::InUse, PoolConnectionState::Validating);
    EXPECT_NE(PoolConnectionState::Validating, PoolConnectionState::Closed);
}

TEST_F(ConnectionPoolTest, PoolStatsStructure) {
    PoolStats stats;

    // Test default initialization
    EXPECT_EQ(stats.total_connections, 0);
    EXPECT_EQ(stats.available_connections, 0);
    EXPECT_EQ(stats.in_use_connections, 0);
    EXPECT_EQ(stats.total_acquires, 0);
    EXPECT_EQ(stats.total_releases, 0);
    EXPECT_EQ(stats.total_creates, 0);
    EXPECT_EQ(stats.total_closes, 0);
    EXPECT_EQ(stats.failed_acquires, 0);

    // Test modification
    stats.total_connections = 10;
    stats.available_connections = 5;
    stats.in_use_connections = 5;

    EXPECT_EQ(stats.total_connections, 10);
    EXPECT_EQ(stats.available_connections, 5);
    EXPECT_EQ(stats.in_use_connections, 5);
}

TEST_F(ConnectionPoolTest, PoolMultipleInstances) {
    PoolConfig config1 = config;
    config1.address = "ws://localhost:8080";

    PoolConfig config2 = config;
    config2.address = "ws://localhost:8081";

    ConnectionPool pool1(config1);
    ConnectionPool pool2(config2);

    EXPECT_EQ(pool1.getConfig().address, "ws://localhost:8080");
    EXPECT_EQ(pool2.getConfig().address, "ws://localhost:8081");
}

TEST_F(ConnectionPoolTest, PoolConnectionLifecycle) {
    WebSocketClientConfig ws_config;
    ws_config.url = "ws://localhost:8080";
    auto client = std::make_shared<WebSocketClient>(ws_config);
    auto conn = std::make_shared<PooledConnection>("ws://localhost:8080", client);

    // Initial state
    EXPECT_EQ(conn->getState(), PoolConnectionState::Available);

    // Acquire
    conn->acquire();
    EXPECT_EQ(conn->getState(), PoolConnectionState::InUse);

    // Use
    auto last_used_before = conn->getLastUsed();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Release
    conn->release();
    EXPECT_EQ(conn->getState(), PoolConnectionState::Available);
    auto last_used_after = conn->getLastUsed();
    EXPECT_GT(last_used_after, last_used_before);

    // Close
    conn->close();
    EXPECT_EQ(conn->getState(), PoolConnectionState::Closed);
}

// ============================================================================
// Integration-style tests (basic structure validation)
// ============================================================================

TEST_F(ConnectionPoolTest, PoolInitializeStructure) {
    ConnectionPool pool(config);

    // Note: This would fail without a real WebSocket server
    // but tests the interface
    EXPECT_FALSE(pool.isClosed());
}

TEST_F(ConnectionPoolTest, PoolCleanupLogic) {
    ConnectionPool pool(config);

    // Cleanup on empty pool should work
    int removed = pool.cleanup();
    EXPECT_EQ(removed, 0);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

