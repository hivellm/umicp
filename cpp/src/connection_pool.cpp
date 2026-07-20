/**
 * @file connection_pool.cpp
 * @brief Implementation of Connection Pool for UMICP
 */

#include "connection_pool.h"
#include "websocket_client.h"
#include <algorithm>
#include <thread>
#include <random>
#include <sstream>
#include <iomanip>

namespace umicp {

// ============================================================================
// Helper: Generate UUID
// ============================================================================
static std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;

    for (int i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-4"; // Version 4 UUID
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen); // Variant
    for (int i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (int i = 0; i < 12; i++) {
        ss << dis(gen);
    }

    return ss.str();
}

// ============================================================================
// PooledConnection Implementation
// ============================================================================

PooledConnection::PooledConnection(const std::string& address,
                                   std::shared_ptr<WebSocketClient> client)
    : id_(generateUUID())
    , address_(address)
    , client_(client)
    , state_(PoolConnectionState::Available)
    , last_used_(std::chrono::system_clock::now())
    , created_at_(std::chrono::system_clock::now())
    , use_count_(0)
{
}

PoolConnectionState PooledConnection::getState() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

void PooledConnection::acquire() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = PoolConnectionState::InUse;
    last_used_ = std::chrono::system_clock::now();
    use_count_++;
}

void PooledConnection::release() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = PoolConnectionState::Available;
    last_used_ = std::chrono::system_clock::now();
}

bool PooledConnection::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_ = PoolConnectionState::Closed;

    if (client_) {
        client_->disconnect();
        return true;
    }

    return false;
}

bool PooledConnection::isStale(int max_age_seconds) const {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - created_at_);
    return elapsed.count() > max_age_seconds;
}

bool PooledConnection::isIdle(int idle_timeout_seconds) const {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_used_);
    return elapsed.count() > idle_timeout_seconds;
}

bool PooledConnection::isConnected() const {
    if (!client_) {
        return false;
    }
    return client_->is_connected();
}

// ============================================================================
// ConnectionPool Implementation
// ============================================================================

ConnectionPool::ConnectionPool(const PoolConfig& config)
    : config_(config)
    , closed_(false)
    , cleanup_running_(false)
{
    // Validate configuration
    if (config_.min_size < 0) {
        config_.min_size = 0;
    }
    if (config_.max_size <= 0) {
        config_.max_size = 10;
    }
    if (config_.min_size > config_.max_size) {
        config_.min_size = config_.max_size;
    }
}

ConnectionPool::~ConnectionPool() {
    shutdown();
}

bool ConnectionPool::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (closed_) {
        return false;
    }

    // Create minimum number of connections
    for (int i = 0; i < config_.min_size; i++) {
        auto conn = createConnection();
        if (conn) {
            connections_[conn->getId()] = conn;
            available_.push(conn);
            stats_.total_connections++;
            stats_.available_connections++;
            stats_.total_creates++;
        }
    }

    return true;
}

std::shared_ptr<PooledConnection> ConnectionPool::acquire(int timeout_ms) {
    if (timeout_ms == 0) {
        timeout_ms = config_.acquire_timeout_ms;
    }

    std::unique_lock<std::mutex> lock(mutex_);

    if (closed_) {
        stats_.failed_acquires++;
        return nullptr;
    }

    // Try to get available connection
    auto deadline = std::chrono::steady_clock::now() +
                   std::chrono::milliseconds(timeout_ms);

    while (available_.empty()) {
        // Try to create new connection if under max size
        if (connections_.size() < static_cast<size_t>(config_.max_size)) {
            lock.unlock();
            auto conn = createConnection();
            lock.lock();

            if (conn) {
                connections_[conn->getId()] = conn;
                conn->acquire();
                stats_.total_connections++;
                stats_.in_use_connections++;
                stats_.total_acquires++;
                stats_.total_creates++;
                return conn;
            }
        }

        // Wait for connection to become available
        if (cv_.wait_until(lock, deadline) == std::cv_status::timeout) {
            stats_.failed_acquires++;
            return nullptr;
        }

        if (closed_) {
            stats_.failed_acquires++;
            return nullptr;
        }
    }

    // Get connection from available queue
    auto conn = available_.front();
    available_.pop();

    // Validate connection
    if (!validateConnection(conn)) {
        // Connection is invalid, remove it and try again
        connections_.erase(conn->getId());
        stats_.total_connections--;
        stats_.total_closes++;

        lock.unlock();
        return acquire(timeout_ms);
    }

    conn->acquire();
    stats_.available_connections--;
    stats_.in_use_connections++;
    stats_.total_acquires++;

    return conn;
}

bool ConnectionPool::release(std::shared_ptr<PooledConnection> conn) {
    if (!conn) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    if (closed_) {
        return false;
    }

    // Check if connection belongs to this pool
    auto it = connections_.find(conn->getId());
    if (it == connections_.end()) {
        return false;
    }

    // Check if connection is still valid
    if (!validateConnection(conn)) {
        // Remove invalid connection
        connections_.erase(conn->getId());
        stats_.total_connections--;
        stats_.in_use_connections--;
        stats_.total_closes++;
        return false;
    }

    conn->release();
    available_.push(conn);

    stats_.in_use_connections--;
    stats_.available_connections++;
    stats_.total_releases++;

    // Notify waiting threads
    cv_.notify_one();

    return true;
}

bool ConnectionPool::remove(const std::string& conn_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = connections_.find(conn_id);
    if (it == connections_.end()) {
        return false;
    }

    auto conn = it->second;
    conn->close();

    connections_.erase(it);
    stats_.total_connections--;

    if (conn->getState() == PoolConnectionState::InUse) {
        stats_.in_use_connections--;
    } else if (conn->getState() == PoolConnectionState::Available) {
        stats_.available_connections--;
    }

    stats_.total_closes++;

    return true;
}

void ConnectionPool::shutdown() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (closed_) {
            return;
        }
        closed_ = true;
    }

    // Stop cleanup task
    stopCleanupTask();

    // Notify all waiting threads
    cv_.notify_all();

    // Close all connections
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : connections_) {
        pair.second->close();
    }

    connections_.clear();
    while (!available_.empty()) {
        available_.pop();
    }

    stats_.total_connections = 0;
    stats_.available_connections = 0;
    stats_.in_use_connections = 0;
}

PoolStats ConnectionPool::getStats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

bool ConnectionPool::isClosed() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return closed_;
}

int ConnectionPool::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);

    int removed_count = 0;

    auto it = connections_.begin();
    while (it != connections_.end()) {
        auto conn = it->second;

        // Only clean up available connections
        if (conn->getState() != PoolConnectionState::Available) {
            ++it;
            continue;
        }

        // Remove if stale or idle
        if (conn->isStale(config_.max_age_seconds) ||
            conn->isIdle(config_.idle_timeout_seconds)) {

            // Keep minimum number of connections
            if (connections_.size() <= static_cast<size_t>(config_.min_size)) {
                ++it;
                continue;
            }

            conn->close();
            it = connections_.erase(it);
            stats_.total_connections--;
            stats_.available_connections--;
            stats_.total_closes++;
            removed_count++;
        } else {
            ++it;
        }
    }

    return removed_count;
}

void ConnectionPool::startCleanupTask(int interval_seconds) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (cleanup_running_ || closed_) {
            return;
        }
        cleanup_running_ = true;
    }

    cleanup_thread_ = std::thread([this, interval_seconds]() {
        while (true) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (!cleanup_running_ || closed_) {
                    break;
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));

            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (!cleanup_running_ || closed_) {
                    break;
                }
            }

            cleanup();
        }
    });
}

void ConnectionPool::stopCleanupTask() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!cleanup_running_) {
            return;
        }
        cleanup_running_ = false;
    }

    if (cleanup_thread_.joinable()) {
        cleanup_thread_.join();
    }
}

std::shared_ptr<PooledConnection> ConnectionPool::createConnection() {
    // Create WebSocket client config
    WebSocketClientConfig ws_config;
    ws_config.url = config_.address;
    ws_config.connection_timeout = std::chrono::milliseconds(config_.acquire_timeout_ms);

    // Create WebSocket client
    auto client = std::make_shared<WebSocketClient>(ws_config);

    // Connect
    auto result = client->connect();
    if (!result.is_ok()) {
        return nullptr;
    }

    // Create pooled connection
    auto conn = std::make_shared<PooledConnection>(config_.address, client);

    return conn;
}

bool ConnectionPool::validateConnection(std::shared_ptr<PooledConnection> conn) {
    if (!conn) {
        return false;
    }

    // Check if closed
    if (conn->getState() == PoolConnectionState::Closed) {
        return false;
    }

    // Check if connected
    if (!conn->isConnected()) {
        return false;
    }

    // Check if stale
    if (conn->isStale(config_.max_age_seconds)) {
        return false;
    }

    return true;
}

} // namespace umicp

