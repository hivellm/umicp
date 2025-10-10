package com.hivellm.umicp.transport.pool;

import com.hivellm.umicp.transport.ClientOptions;
import com.hivellm.umicp.transport.UMICPWebSocketClient;
import com.hivellm.umicp.types.ConnectionException;

import java.net.URI;
import java.util.*;
import java.util.concurrent.*;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Connection Pool Manager
 * Manages a pool of reusable WebSocket connections with automatic
 * lifecycle management, validation, and cleanup.
 */
public class ConnectionPool implements AutoCloseable {
    private final PoolConfig config;
    private final Map<String, PooledConnection> connections;
    private final BlockingQueue<PooledConnection> available;
    private final PoolStats stats;
    private final ReentrantLock lock;
    private boolean closed;
    private ScheduledExecutorService cleanupExecutor;
    private ScheduledFuture<?> cleanupTask;

    /**
     * Construct a new Connection Pool
     *
     * @param config Pool configuration
     */
    public ConnectionPool(PoolConfig config) {
        if (config == null) {
            throw new IllegalArgumentException("config cannot be null");
        }

        config.validate();
        this.config = config;
        this.connections = new ConcurrentHashMap<>();
        this.available = new LinkedBlockingQueue<>(config.getMaxSize());
        this.stats = new PoolStats();
        this.lock = new ReentrantLock();
        this.closed = false;
    }

    /**
     * Initialize the pool with minimum connections
     *
     * @throws ConnectionException if initialization fails
     */
    public void initialize() throws ConnectionException {
        lock.lock();
        try {
            if (closed) {
                throw new ConnectionException("Pool is closed");
            }

            for (int i = 0; i < config.getMinSize(); i++) {
                PooledConnection conn = createConnection();
                if (conn != null) {
                    connections.put(conn.getId(), conn);
                    available.offer(conn);
                    stats.incrementTotalConnections();
                    stats.incrementAvailableConnections();
                    stats.incrementTotalCreates();
                }
            }
        } finally {
            lock.unlock();
        }
    }

    /**
     * Acquire a connection from the pool
     *
     * @return Pooled connection
     * @throws ConnectionException if pool is closed or timeout occurs
     * @throws InterruptedException if thread is interrupted
     */
    public PooledConnection acquire() throws ConnectionException, InterruptedException {
        return acquire(config.getAcquireTimeoutMs());
    }

    /**
     * Acquire a connection from the pool with custom timeout
     *
     * @param timeoutMs Timeout in milliseconds
     * @return Pooled connection
     * @throws ConnectionException if pool is closed or timeout occurs
     * @throws InterruptedException if thread is interrupted
     */
    public PooledConnection acquire(int timeoutMs) throws ConnectionException, InterruptedException {
        if (closed) {
            stats.incrementFailedAcquires();
            throw new ConnectionException("Pool is closed");
        }

        // Try to get available connection
        PooledConnection conn = available.poll(timeoutMs, TimeUnit.MILLISECONDS);

        if (conn == null) {
            // Try to create new connection if under max size
            lock.lock();
            try {
                if (connections.size() < config.getMaxSize()) {
                    conn = createConnection();
                    if (conn != null) {
                        connections.put(conn.getId(), conn);
                        stats.incrementTotalConnections();
                        stats.incrementTotalCreates();
                    }
                }
            } finally {
                lock.unlock();
            }

            if (conn == null) {
                stats.incrementFailedAcquires();
                throw new ConnectionException("No available connections (timeout)");
            }
        }

        // Validate connection
        if (!validateConnection(conn)) {
            // Connection is invalid, remove and try again
            remove(conn.getId());
            return acquire(timeoutMs);
        }

        conn.acquire();
        lock.lock();
        try {
            stats.decrementAvailableConnections();
            stats.incrementInUseConnections();
            stats.incrementTotalAcquires();
        } finally {
            lock.unlock();
        }

        return conn;
    }

    /**
     * Release a connection back to the pool
     *
     * @param conn Connection to release
     * @return true if successfully released
     */
    public boolean release(PooledConnection conn) {
        if (conn == null) {
            return false;
        }

        lock.lock();
        try {
            if (closed) {
                return false;
            }

            // Check if connection belongs to this pool
            if (!connections.containsKey(conn.getId())) {
                return false;
            }

            // Validate connection
            if (!validateConnection(conn)) {
                // Remove invalid connection
                connections.remove(conn.getId());
                stats.decrementTotalConnections();
                stats.decrementInUseConnections();
                stats.incrementTotalCloses();
                return false;
            }

            conn.release();
            available.offer(conn);

            stats.decrementInUseConnections();
            stats.incrementAvailableConnections();
            stats.incrementTotalReleases();

            return true;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Remove a connection from the pool
     *
     * @param connId Connection ID to remove
     * @return true if connection was found and removed
     */
    public boolean remove(String connId) {
        if (connId == null || connId.isEmpty()) {
            return false;
        }

        lock.lock();
        try {
            PooledConnection conn = connections.remove(connId);
            if (conn == null) {
                return false;
            }

            conn.close();
            stats.decrementTotalConnections();

            PoolConnectionState state = conn.getState();
            if (state == PoolConnectionState.IN_USE) {
                stats.decrementInUseConnections();
            } else if (state == PoolConnectionState.AVAILABLE) {
                stats.decrementAvailableConnections();
            }

            stats.incrementTotalCloses();
            return true;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Shutdown the pool and close all connections
     */
    public void shutdown() {
        lock.lock();
        try {
            if (closed) {
                return;
            }
            closed = true;

            // Stop cleanup task
            stopCleanupTask();

            // Close all connections
            for (PooledConnection conn : connections.values()) {
                conn.close();
            }

            connections.clear();
            available.clear();

            stats.setTotalConnections(0);
            stats.setAvailableConnections(0);
            stats.setInUseConnections(0);
        } finally {
            lock.unlock();
        }
    }

    /**
     * Get pool statistics
     *
     * @return Current pool statistics
     */
    public PoolStats getStats() {
        return stats;
    }

    /**
     * Get pool configuration
     *
     * @return Current configuration
     */
    public PoolConfig getConfig() {
        return config;
    }

    /**
     * Check if pool is closed
     *
     * @return true if pool is closed
     */
    public boolean isClosed() {
        lock.lock();
        try {
            return closed;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Cleanup stale and idle connections
     *
     * @return Number of connections removed
     */
    public int cleanup() {
        lock.lock();
        try {
            int removedCount = 0;

            Iterator<Map.Entry<String, PooledConnection>> iterator = connections.entrySet().iterator();
            while (iterator.hasNext()) {
                Map.Entry<String, PooledConnection> entry = iterator.next();
                PooledConnection conn = entry.getValue();

                // Only clean up available connections
                if (conn.getState() != PoolConnectionState.AVAILABLE) {
                    continue;
                }

                // Remove if stale or idle
                if (conn.isStale(config.getMaxAgeSeconds()) ||
                    conn.isIdle(config.getIdleTimeoutSeconds())) {

                    // Keep minimum number of connections
                    if (connections.size() <= config.getMinSize()) {
                        continue;
                    }

                    conn.close();
                    iterator.remove();
                    stats.decrementTotalConnections();
                    stats.decrementAvailableConnections();
                    stats.incrementTotalCloses();
                    removedCount++;
                }
            }

            return removedCount;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Start background cleanup task
     *
     * @param intervalSeconds Cleanup interval in seconds
     */
    public void startCleanupTask(int intervalSeconds) {
        lock.lock();
        try {
            if (cleanupExecutor != null || closed) {
                return;
            }

            cleanupExecutor = Executors.newSingleThreadScheduledExecutor(r -> {
                Thread thread = new Thread(r, "ConnectionPool-Cleanup");
                thread.setDaemon(true);
                return thread;
            });

            cleanupTask = cleanupExecutor.scheduleAtFixedRate(
                this::cleanup,
                intervalSeconds,
                intervalSeconds,
                TimeUnit.SECONDS
            );
        } finally {
            lock.unlock();
        }
    }

    /**
     * Stop background cleanup task
     */
    public void stopCleanupTask() {
        lock.lock();
        try {
            if (cleanupTask != null) {
                cleanupTask.cancel(false);
                cleanupTask = null;
            }

            if (cleanupExecutor != null) {
                cleanupExecutor.shutdown();
                try {
                    if (!cleanupExecutor.awaitTermination(5, TimeUnit.SECONDS)) {
                        cleanupExecutor.shutdownNow();
                    }
                } catch (InterruptedException e) {
                    cleanupExecutor.shutdownNow();
                    Thread.currentThread().interrupt();
                }
                cleanupExecutor = null;
            }
        } finally {
            lock.unlock();
        }
    }

    /**
     * Create a new connection
     *
     * @return New pooled connection, or null if failed
     */
    private PooledConnection createConnection() {
        try {
            // Create client options
            ClientOptions options = new ClientOptions();
            // Note: ClientOptions would need to be configured based on config

            // Create WebSocket client
            URI uri = new URI(config.getAddress());
            UMICPWebSocketClient client = new UMICPWebSocketClient(uri, options);

            // Connect
            if (!client.connectBlocking(config.getAcquireTimeoutMs(), TimeUnit.MILLISECONDS)) {
                return null;
            }

            // Create pooled connection
            return new PooledConnection(config.getAddress(), client);
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Validate a connection
     *
     * @param conn Connection to validate
     * @return true if connection is valid and connected
     */
    private boolean validateConnection(PooledConnection conn) {
        if (conn == null) {
            return false;
        }

        // Check if closed
        if (conn.getState() == PoolConnectionState.CLOSED) {
            return false;
        }

        // Check if connected
        if (!conn.isConnected()) {
            return false;
        }

        // Check if stale
        if (conn.isStale(config.getMaxAgeSeconds())) {
            return false;
        }

        return true;
    }

    @Override
    public void close() {
        shutdown();
    }
}

