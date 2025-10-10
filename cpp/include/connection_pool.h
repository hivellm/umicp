/**
 * @file connection_pool.h
 * @brief Connection Pool for UMICP WebSocket connections
 *
 * Provides connection pooling capabilities for efficient WebSocket
 * connection management and reuse.
 */

#ifndef UMICP_CONNECTION_POOL_H
#define UMICP_CONNECTION_POOL_H

#include <string>
#include <memory>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <map>
#include <queue>
#include <functional>
#include <thread>

namespace umicp {

// Forward declarations
class WebSocketClient;

/**
 * @brief Connection state enum
 */
enum class PoolConnectionState {
    Available,    ///< Connection is available for use
    InUse,        ///< Connection is currently in use
    Validating,   ///< Connection is being validated
    Closed        ///< Connection is closed
};

/**
 * @brief Pooled connection wrapper
 *
 * Wraps a WebSocket client with metadata for pool management.
 */
class PooledConnection {
public:
    /**
     * @brief Construct a new Pooled Connection object
     *
     * @param address WebSocket address
     * @param client WebSocket client instance
     */
    PooledConnection(const std::string& address,
                     std::shared_ptr<WebSocketClient> client);

    /**
     * @brief Destroy the Pooled Connection object
     */
    ~PooledConnection() = default;

    // Getters
    const std::string& getId() const { return id_; }
    const std::string& getAddress() const { return address_; }
    std::shared_ptr<WebSocketClient> getClient() const { return client_; }
    PoolConnectionState getState() const;
    std::chrono::system_clock::time_point getLastUsed() const { return last_used_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return created_at_; }
    int getUseCount() const { return use_count_; }

    /**
     * @brief Mark connection as in use
     */
    void acquire();

    /**
     * @brief Mark connection as available
     */
    void release();

    /**
     * @brief Close the connection
     *
     * @return true if successfully closed
     */
    bool close();

    /**
     * @brief Check if connection is stale (older than max age)
     *
     * @param max_age_seconds Maximum age in seconds
     * @return true if connection is older than max age
     */
    bool isStale(int max_age_seconds) const;

    /**
     * @brief Check if connection has been idle too long
     *
     * @param idle_timeout_seconds Idle timeout in seconds
     * @return true if connection has been idle longer than timeout
     */
    bool isIdle(int idle_timeout_seconds) const;

    /**
     * @brief Check if underlying client is connected
     *
     * @return true if connected
     */
    bool isConnected() const;

private:
    std::string id_;
    std::string address_;
    std::shared_ptr<WebSocketClient> client_;
    PoolConnectionState state_;
    std::chrono::system_clock::time_point last_used_;
    std::chrono::system_clock::time_point created_at_;
    int use_count_;
    mutable std::mutex mutex_;
};

/**
 * @brief Connection pool statistics
 */
struct PoolStats {
    size_t total_connections = 0;       ///< Total connections in pool
    size_t available_connections = 0;   ///< Available connections
    size_t in_use_connections = 0;      ///< Connections currently in use
    size_t total_acquires = 0;          ///< Total number of acquires
    size_t total_releases = 0;          ///< Total number of releases
    size_t total_creates = 0;           ///< Total connections created
    size_t total_closes = 0;            ///< Total connections closed
    size_t failed_acquires = 0;         ///< Failed acquire attempts
};

/**
 * @brief Connection Pool Configuration
 */
struct PoolConfig {
    std::string address;              ///< WebSocket address
    int min_size = 2;                 ///< Minimum pool size
    int max_size = 10;                ///< Maximum pool size
    int max_age_seconds = 600;        ///< Max connection age (10 minutes)
    int idle_timeout_seconds = 300;   ///< Idle timeout (5 minutes)
    int acquire_timeout_ms = 5000;    ///< Acquire timeout in milliseconds
};

/**
 * @brief Connection Pool Manager
 *
 * Manages a pool of reusable WebSocket connections with automatic
 * lifecycle management, validation, and cleanup.
 */
class ConnectionPool {
public:
    /**
     * @brief Construct a new Connection Pool object
     *
     * @param config Pool configuration
     */
    explicit ConnectionPool(const PoolConfig& config);

    /**
     * @brief Destroy the Connection Pool object
     */
    ~ConnectionPool();

    /**
     * @brief Initialize the pool with minimum connections
     *
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Acquire a connection from the pool
     *
     * Blocks until a connection is available or timeout occurs.
     *
     * @param timeout_ms Timeout in milliseconds (0 = use config default)
     * @return Pooled connection, or nullptr if failed/timeout
     */
    std::shared_ptr<PooledConnection> acquire(int timeout_ms = 0);

    /**
     * @brief Release a connection back to the pool
     *
     * @param conn Connection to release
     * @return true if successfully released
     */
    bool release(std::shared_ptr<PooledConnection> conn);

    /**
     * @brief Remove a connection from the pool
     *
     * @param conn_id Connection ID to remove
     * @return true if connection was found and removed
     */
    bool remove(const std::string& conn_id);

    /**
     * @brief Close all connections and shutdown the pool
     */
    void shutdown();

    /**
     * @brief Get pool statistics
     *
     * @return Current pool statistics
     */
    PoolStats getStats() const;

    /**
     * @brief Get pool configuration
     *
     * @return Current configuration
     */
    const PoolConfig& getConfig() const { return config_; }

    /**
     * @brief Check if pool is closed
     *
     * @return true if pool is closed
     */
    bool isClosed() const;

    /**
     * @brief Cleanup stale and idle connections
     *
     * @return Number of connections removed
     */
    int cleanup();

    /**
     * @brief Start background cleanup task
     *
     * @param interval_seconds Cleanup interval in seconds
     */
    void startCleanupTask(int interval_seconds = 60);

    /**
     * @brief Stop background cleanup task
     */
    void stopCleanupTask();

private:
    /**
     * @brief Create a new connection
     *
     * @return New pooled connection, or nullptr if failed
     */
    std::shared_ptr<PooledConnection> createConnection();

    /**
     * @brief Validate a connection
     *
     * @param conn Connection to validate
     * @return true if connection is valid and connected
     */
    bool validateConnection(std::shared_ptr<PooledConnection> conn);

    /**
     * @brief Background cleanup task
     */
    void cleanupTask();

    PoolConfig config_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::map<std::string, std::shared_ptr<PooledConnection>> connections_;
    std::queue<std::shared_ptr<PooledConnection>> available_;
    PoolStats stats_;
    bool closed_;
    bool cleanup_running_;
    std::thread cleanup_thread_;
};

} // namespace umicp

#endif // UMICP_CONNECTION_POOL_H

