package com.hivellm.umicp.transport.pool;

/**
 * Connection Pool Configuration
 */
public class PoolConfig {
    private String address;
    private int minSize;
    private int maxSize;
    private int maxAgeSeconds;
    private int idleTimeoutSeconds;
    private int acquireTimeoutMs;

    /**
     * Default constructor with default values
     */
    public PoolConfig() {
        this.minSize = 2;
        this.maxSize = 10;
        this.maxAgeSeconds = 600;        // 10 minutes
        this.idleTimeoutSeconds = 300;   // 5 minutes
        this.acquireTimeoutMs = 5000;    // 5 seconds
    }

    /**
     * Constructor with address
     *
     * @param address WebSocket address
     */
    public PoolConfig(String address) {
        this();
        this.address = address;
    }

    // Getters
    public String getAddress() { return address; }
    public int getMinSize() { return minSize; }
    public int getMaxSize() { return maxSize; }
    public int getMaxAgeSeconds() { return maxAgeSeconds; }
    public int getIdleTimeoutSeconds() { return idleTimeoutSeconds; }
    public int getAcquireTimeoutMs() { return acquireTimeoutMs; }

    // Setters with validation
    public PoolConfig setAddress(String address) {
        if (address == null || address.isEmpty()) {
            throw new IllegalArgumentException("address cannot be null or empty");
        }
        this.address = address;
        return this;
    }

    public PoolConfig setMinSize(int minSize) {
        if (minSize < 0) {
            throw new IllegalArgumentException("minSize must be >= 0");
        }
        this.minSize = minSize;
        return this;
    }

    public PoolConfig setMaxSize(int maxSize) {
        if (maxSize <= 0) {
            throw new IllegalArgumentException("maxSize must be > 0");
        }
        this.maxSize = maxSize;
        return this;
    }

    public PoolConfig setMaxAgeSeconds(int maxAgeSeconds) {
        if (maxAgeSeconds <= 0) {
            throw new IllegalArgumentException("maxAgeSeconds must be > 0");
        }
        this.maxAgeSeconds = maxAgeSeconds;
        return this;
    }

    public PoolConfig setIdleTimeoutSeconds(int idleTimeoutSeconds) {
        if (idleTimeoutSeconds <= 0) {
            throw new IllegalArgumentException("idleTimeoutSeconds must be > 0");
        }
        this.idleTimeoutSeconds = idleTimeoutSeconds;
        return this;
    }

    public PoolConfig setAcquireTimeoutMs(int acquireTimeoutMs) {
        if (acquireTimeoutMs <= 0) {
            throw new IllegalArgumentException("acquireTimeoutMs must be > 0");
        }
        this.acquireTimeoutMs = acquireTimeoutMs;
        return this;
    }

    /**
     * Validate configuration
     * Ensures minSize <= maxSize
     */
    public void validate() {
        if (minSize > maxSize) {
            minSize = maxSize;
        }
    }

    @Override
    public String toString() {
        return "PoolConfig{" +
                "address='" + address + '\'' +
                ", minSize=" + minSize +
                ", maxSize=" + maxSize +
                ", maxAgeSeconds=" + maxAgeSeconds +
                ", idleTimeoutSeconds=" + idleTimeoutSeconds +
                ", acquireTimeoutMs=" + acquireTimeoutMs +
                '}';
    }
}

