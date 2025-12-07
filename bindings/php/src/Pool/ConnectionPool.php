<?php

namespace Umicp\Pool;

use SplQueue;

/**
 * Pool Configuration
 */
class PoolConfig
{
    public string $address;
    public int $minSize = 2;
    public int $maxSize = 10;
    public int $maxAgeSeconds = 600;      // 10 minutes
    public int $idleTimeoutSeconds = 300; // 5 minutes
    public int $acquireTimeoutMs = 5000;  // 5 seconds

    public function __construct(string $address)
    {
        $this->address = $address;
    }

    /**
     * Validate configuration
     */
    public function validate(): void
    {
        if ($this->minSize > $this->maxSize) {
            $this->minSize = $this->maxSize;
        }
    }
}

/**
 * Pool Statistics
 */
class PoolStats
{
    public int $totalConnections = 0;
    public int $availableConnections = 0;
    public int $inUseConnections = 0;
    public int $totalAcquires = 0;
    public int $totalReleases = 0;
    public int $totalCreates = 0;
    public int $totalCloses = 0;
    public int $failedAcquires = 0;

    public function toArray(): array
    {
        return get_object_vars($this);
    }
}

/**
 * Connection Pool Manager
 *
 * Manages a pool of reusable connections with automatic
 * lifecycle management, validation, and cleanup.
 */
class ConnectionPool
{
    private PoolConfig $config;
    private array $connections = [];
    private SplQueue $available;
    private PoolStats $stats;
    private bool $closed = false;

    /**
     * Construct a new Connection Pool
     *
     * @param PoolConfig $config Pool configuration
     */
    public function __construct(PoolConfig $config)
    {
        $config->validate();
        $this->config = $config;
        $this->available = new SplQueue();
        $this->stats = new PoolStats();
    }

    /**
     * Initialize the pool with minimum connections
     *
     * @return bool true if initialization successful
     */
    public function initialize(): bool
    {
        if ($this->closed) {
            return false;
        }

        for ($i = 0; $i < $this->config->minSize; $i++) {
            $conn = $this->createConnection();
            if ($conn) {
                $this->connections[$conn->getId()] = $conn;
                $this->available->enqueue($conn);
                $this->stats->totalConnections++;
                $this->stats->availableConnections++;
                $this->stats->totalCreates++;
            }
        }

        return true;
    }

    /**
     * Acquire a connection from the pool
     *
     * @param int $timeoutMs Timeout in milliseconds
     * @return PooledConnection|null Pooled connection or null if failed
     * @throws \RuntimeException if pool is closed
     */
    public function acquire(int $timeoutMs = 0): ?PooledConnection
    {
        if ($this->closed) {
            $this->stats->failedAcquires++;
            throw new \RuntimeException('Pool is closed');
        }

        if ($timeoutMs === 0) {
            $timeoutMs = $this->config->acquireTimeoutMs;
        }

        $deadline = microtime(true) + ($timeoutMs / 1000);

        // Try to get available connection
        while ($this->available->isEmpty()) {
            // Try to create new connection if under max size
            if (count($this->connections) < $this->config->maxSize) {
                $conn = $this->createConnection();
                if ($conn) {
                    $this->connections[$conn->getId()] = $conn;
                    $conn->acquire();
                    $this->stats->totalConnections++;
                    $this->stats->inUseConnections++;
                    $this->stats->totalAcquires++;
                    $this->stats->totalCreates++;
                    return $conn;
                }
            }

            // Check timeout
            if (microtime(true) >= $deadline) {
                $this->stats->failedAcquires++;
                return null;
            }

            usleep(100000); // Sleep 100ms
        }

        /** @var PooledConnection $conn */
        $conn = $this->available->dequeue();

        // Validate connection
        if (!$this->validateConnection($conn)) {
            unset($this->connections[$conn->getId()]);
            $this->stats->totalConnections--;
            $this->stats->totalCloses++;
            return $this->acquire($timeoutMs);
        }

        $conn->acquire();
        $this->stats->availableConnections--;
        $this->stats->inUseConnections++;
        $this->stats->totalAcquires++;

        return $conn;
    }

    /**
     * Release a connection back to the pool
     *
     * @param PooledConnection $conn Connection to release
     * @return bool true if successfully released
     */
    public function release(PooledConnection $conn): bool
    {
        if ($this->closed) {
            return false;
        }

        // Check if connection belongs to this pool
        if (!isset($this->connections[$conn->getId()])) {
            return false;
        }

        // Validate connection
        if (!$this->validateConnection($conn)) {
            unset($this->connections[$conn->getId()]);
            $this->stats->totalConnections--;
            $this->stats->inUseConnections--;
            $this->stats->totalCloses++;
            return false;
        }

        $conn->release();
        $this->available->enqueue($conn);

        $this->stats->inUseConnections--;
        $this->stats->availableConnections++;
        $this->stats->totalReleases++;

        return true;
    }

    /**
     * Remove a connection from the pool
     *
     * @param string $connId Connection ID to remove
     * @return bool true if connection was found and removed
     */
    public function remove(string $connId): bool
    {
        if (!isset($this->connections[$connId])) {
            return false;
        }

        $conn = $this->connections[$connId];
        $conn->close();

        unset($this->connections[$connId]);
        $this->stats->totalConnections--;

        $state = $conn->getState();
        if ($state === PoolConnectionState::IN_USE) {
            $this->stats->inUseConnections--;
        } elseif ($state === PoolConnectionState::AVAILABLE) {
            $this->stats->availableConnections--;
        }

        $this->stats->totalCloses++;

        return true;
    }

    /**
     * Shutdown the pool and close all connections
     */
    public function shutdown(): void
    {
        if ($this->closed) {
            return;
        }

        $this->closed = true;

        // Close all connections
        foreach ($this->connections as $conn) {
            $conn->close();
        }

        $this->connections = [];
        $this->available = new SplQueue();

        $this->stats->totalConnections = 0;
        $this->stats->availableConnections = 0;
        $this->stats->inUseConnections = 0;
    }

    /**
     * Get pool statistics
     *
     * @return PoolStats Current pool statistics
     */
    public function getStats(): PoolStats
    {
        return $this->stats;
    }

    /**
     * Get pool configuration
     *
     * @return PoolConfig Current configuration
     */
    public function getConfig(): PoolConfig
    {
        return $this->config;
    }

    /**
     * Check if pool is closed
     *
     * @return bool true if pool is closed
     */
    public function isClosed(): bool
    {
        return $this->closed;
    }

    /**
     * Cleanup stale and idle connections
     *
     * @return int Number of connections removed
     */
    public function cleanup(): int
    {
        $removedCount = 0;

        foreach ($this->connections as $connId => $conn) {
            // Only clean up available connections
            if ($conn->getState() !== PoolConnectionState::AVAILABLE) {
                continue;
            }

            // Remove if stale or idle
            if ($conn->isStale($this->config->maxAgeSeconds) ||
                $conn->isIdle($this->config->idleTimeoutSeconds)) {

                // Keep minimum number of connections
                if (count($this->connections) <= $this->config->minSize) {
                    continue;
                }

                $conn->close();
                unset($this->connections[$connId]);
                $this->stats->totalConnections--;
                $this->stats->availableConnections--;
                $this->stats->totalCloses++;
                $removedCount++;
            }
        }

        return $removedCount;
    }

    /**
     * Create a new connection
     *
     * @return PooledConnection|null New pooled connection, or null if failed
     */
    private function createConnection(): ?PooledConnection
    {
        try {
            // Placeholder - actual client creation would go here
            // In a real implementation, this would create a WebSocket client
            $client = null; // Would be actual client instance

            return new PooledConnection($this->config->address, $client);
        } catch (\Exception $e) {
            return null;
        }
    }

    /**
     * Validate a connection
     *
     * @param PooledConnection $conn Connection to validate
     * @return bool true if connection is valid and connected
     */
    private function validateConnection(PooledConnection $conn): bool
    {
        if ($conn->getState() === PoolConnectionState::CLOSED) {
            return false;
        }

        // In a real implementation, would check if client is actually connected
        // For now, just check if not stale
        if ($conn->isStale($this->config->maxAgeSeconds)) {
            return false;
        }

        return true;
    }
}

