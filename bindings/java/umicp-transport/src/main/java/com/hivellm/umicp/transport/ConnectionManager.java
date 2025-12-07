package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Manages connection limits and resource tracking.
 *
 * <p>Provides:
 * <ul>
 *   <li>Connection count limiting</li>
 *   <li>Resource exhaustion prevention</li>
 *   <li>Connection lifecycle tracking</li>
 * </ul>
 *
 * <p>Thread-safe implementation for concurrent access.
 *
 * @author HiveLLM Team
 * @version 0.1.1
 * @since 0.1.1
 */
public class ConnectionManager {

    private static final Logger logger = LoggerFactory.getLogger(ConnectionManager.class);

    /**
     * Default maximum connections.
     */
    public static final int DEFAULT_MAX_CONNECTIONS = 1000;

    /**
     * Minimum connections limit.
     */
    public static final int MIN_CONNECTIONS = 1;

    /**
     * Maximum connections limit.
     */
    public static final int MAX_CONNECTIONS_LIMIT = 100000;

    private final int maxConnections;
    private final AtomicInteger activeConnections;
    private final AtomicLong totalAccepted;
    private final AtomicLong totalRejected;
    private final ConcurrentHashMap<String, Long> connectionTimestamps;

    /**
     * Creates a connection manager with default limit.
     */
    public ConnectionManager() {
        this(DEFAULT_MAX_CONNECTIONS);
    }

    /**
     * Creates a connection manager with specified limit.
     *
     * @param maxConnections maximum number of concurrent connections
     */
    public ConnectionManager(int maxConnections) {
        if (maxConnections < MIN_CONNECTIONS || maxConnections > MAX_CONNECTIONS_LIMIT) {
            throw new IllegalArgumentException(
                String.format("maxConnections must be between %d and %d",
                    MIN_CONNECTIONS, MAX_CONNECTIONS_LIMIT)
            );
        }

        this.maxConnections = maxConnections;
        this.activeConnections = new AtomicInteger(0);
        this.totalAccepted = new AtomicLong(0);
        this.totalRejected = new AtomicLong(0);
        this.connectionTimestamps = new ConcurrentHashMap<>();
    }

    /**
     * Attempts to acquire a connection slot.
     *
     * @param connectionId unique connection identifier
     * @return true if connection was accepted, false if limit reached
     */
    public boolean acceptConnection(@NotNull String connectionId) {
        if (connectionId == null || connectionId.isEmpty()) {
            throw new IllegalArgumentException("Connection ID cannot be null or empty");
        }

        // Check if already registered
        if (connectionTimestamps.containsKey(connectionId)) {
            logger.warn("Connection {} already registered", connectionId);
            return true; // Already accepted
        }

        // Try to increment active connections
        int current;
        do {
            current = activeConnections.get();
            if (current >= maxConnections) {
                totalRejected.incrementAndGet();
                logger.warn("Connection limit reached: {}/{} (rejected: {})",
                    current, maxConnections, totalRejected.get());
                return false;
            }
        } while (!activeConnections.compareAndSet(current, current + 1));

        // Record connection
        connectionTimestamps.put(connectionId, System.currentTimeMillis());
        totalAccepted.incrementAndGet();

        logger.debug("Connection {} accepted: {}/{}",
            connectionId, activeConnections.get(), maxConnections);

        return true;
    }

    /**
     * Releases a connection slot.
     *
     * @param connectionId unique connection identifier
     * @return true if connection was released, false if not found
     */
    public boolean releaseConnection(@NotNull String connectionId) {
        if (connectionId == null || connectionId.isEmpty()) {
            throw new IllegalArgumentException("Connection ID cannot be null or empty");
        }

        Long timestamp = connectionTimestamps.remove(connectionId);
        if (timestamp == null) {
            logger.warn("Connection {} not found for release", connectionId);
            return false;
        }

        int current = activeConnections.decrementAndGet();

        long duration = System.currentTimeMillis() - timestamp;
        logger.debug("Connection {} released after {}ms: {}/{}",
            connectionId, duration, current, maxConnections);

        return true;
    }

    /**
     * Checks if new connections can be accepted.
     *
     * @return true if below limit
     */
    public boolean canAcceptConnection() {
        return activeConnections.get() < maxConnections;
    }

    /**
     * Gets the number of active connections.
     *
     * @return active connection count
     */
    public int getActiveConnections() {
        return activeConnections.get();
    }

    /**
     * Gets the maximum number of connections allowed.
     *
     * @return maximum connections
     */
    public int getMaxConnections() {
        return maxConnections;
    }

    /**
     * Gets the total number of accepted connections.
     *
     * @return total accepted count
     */
    public long getTotalAccepted() {
        return totalAccepted.get();
    }

    /**
     * Gets the total number of rejected connections.
     *
     * @return total rejected count
     */
    public long getTotalRejected() {
        return totalRejected.get();
    }

    /**
     * Gets the connection usage ratio.
     *
     * @return usage ratio (0.0 to 1.0)
     */
    public double getUsageRatio() {
        return (double) activeConnections.get() / maxConnections;
    }

    /**
     * Checks if connection limit is nearly reached (>90%).
     *
     * @return true if nearing limit
     */
    public boolean isNearingLimit() {
        return getUsageRatio() > 0.9;
    }

    /**
     * Gets the connection duration in milliseconds.
     *
     * @param connectionId connection identifier
     * @return duration in ms, or -1 if not found
     */
    public long getConnectionDuration(@NotNull String connectionId) {
        Long timestamp = connectionTimestamps.get(connectionId);
        if (timestamp == null) {
            return -1;
        }
        return System.currentTimeMillis() - timestamp;
    }

    /**
     * Checks if a connection is registered.
     *
     * @param connectionId connection identifier
     * @return true if registered
     */
    public boolean isConnectionRegistered(@NotNull String connectionId) {
        return connectionTimestamps.containsKey(connectionId);
    }

    /**
     * Resets all statistics (keeps active connections).
     */
    public void resetStatistics() {
        totalAccepted.set(0);
        totalRejected.set(0);
        logger.info("Connection statistics reset");
    }

    /**
     * Forcefully clears all connections.
     *
     * <p>Warning: This should only be used in shutdown scenarios.
     */
    public void clearAllConnections() {
        int cleared = activeConnections.getAndSet(0);
        connectionTimestamps.clear();
        logger.warn("Forcefully cleared {} active connections", cleared);
    }

    @Override
    public String toString() {
        return String.format(
            "ConnectionManager{active=%d/%d (%.1f%%), accepted=%d, rejected=%d}",
            activeConnections.get(),
            maxConnections,
            getUsageRatio() * 100,
            totalAccepted.get(),
            totalRejected.get()
        );
    }
}

