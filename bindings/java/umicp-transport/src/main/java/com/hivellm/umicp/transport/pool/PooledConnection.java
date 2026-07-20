package com.hivellm.umicp.transport.pool;

import com.hivellm.umicp.transport.UMICPWebSocketClient;
import java.time.Instant;
import java.util.UUID;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Connection state enum
 */
enum PoolConnectionState {
    AVAILABLE,    // Connection is available for use
    IN_USE,       // Connection is currently in use
    VALIDATING,   // Connection is being validated
    CLOSED        // Connection is closed
}

/**
 * Pooled connection wrapper
 * Wraps a WebSocket client with metadata for pool management
 */
public class PooledConnection {
    private final String id;
    private final String address;
    private final UMICPWebSocketClient client;
    private PoolConnectionState state;
    private Instant lastUsed;
    private final Instant createdAt;
    private int useCount;
    private final ReentrantLock lock;

    /**
     * Construct a new Pooled Connection
     *
     * @param address WebSocket address
     * @param client WebSocket client instance
     */
    public PooledConnection(String address, UMICPWebSocketClient client) {
        if (address == null || address.isEmpty()) {
            throw new IllegalArgumentException("address cannot be null or empty");
        }
        if (client == null) {
            throw new IllegalArgumentException("client cannot be null");
        }

        this.id = UUID.randomUUID().toString();
        this.address = address;
        this.client = client;
        this.state = PoolConnectionState.AVAILABLE;
        this.lastUsed = Instant.now();
        this.createdAt = Instant.now();
        this.useCount = 0;
        this.lock = new ReentrantLock();
    }

    // Getters
    public String getId() { return id; }
    public String getAddress() { return address; }
    public UMICPWebSocketClient getClient() { return client; }
    public Instant getLastUsed() { return lastUsed; }
    public Instant getCreatedAt() { return createdAt; }
    public int getUseCount() { return useCount; }

    public PoolConnectionState getState() {
        lock.lock();
        try {
            return state;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Mark connection as in use
     */
    public void acquire() {
        lock.lock();
        try {
            state = PoolConnectionState.IN_USE;
            lastUsed = Instant.now();
            useCount++;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Mark connection as available
     */
    public void release() {
        lock.lock();
        try {
            state = PoolConnectionState.AVAILABLE;
            lastUsed = Instant.now();
        } finally {
            lock.unlock();
        }
    }

    /**
     * Close the connection
     *
     * @return true if successfully closed
     */
    public boolean close() {
        lock.lock();
        try {
            state = PoolConnectionState.CLOSED;
            if (client != null) {
                try {
                    client.close();
                    return true;
                } catch (Exception e) {
                    return false;
                }
            }
            return false;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Check if connection is stale (older than max age)
     *
     * @param maxAgeSeconds Maximum age in seconds
     * @return true if connection is older than max age
     */
    public boolean isStale(int maxAgeSeconds) {
        Instant now = Instant.now();
        long elapsed = now.getEpochSecond() - createdAt.getEpochSecond();
        return elapsed > maxAgeSeconds;
    }

    /**
     * Check if connection has been idle too long
     *
     * @param idleTimeoutSeconds Idle timeout in seconds
     * @return true if connection has been idle longer than timeout
     */
    public boolean isIdle(int idleTimeoutSeconds) {
        lock.lock();
        try {
            Instant now = Instant.now();
            long elapsed = now.getEpochSecond() - lastUsed.getEpochSecond();
            return elapsed > idleTimeoutSeconds;
        } finally {
            lock.unlock();
        }
    }

    /**
     * Check if underlying client is connected
     *
     * @return true if connected
     */
    public boolean isConnected() {
        if (client == null) {
            return false;
        }
        return client.isConnected();
    }

    @Override
    public String toString() {
        return "PooledConnection{" +
                "id='" + id + '\'' +
                ", address='" + address + '\'' +
                ", state=" + state +
                ", useCount=" + useCount +
                '}';
    }
}

