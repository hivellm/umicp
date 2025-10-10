<?php

namespace Umicp\Pool;

use DateTimeInterface;
use DateTime;

/**
 * Connection state enum
 */
enum PoolConnectionState: string
{
    case AVAILABLE = 'available';
    case IN_USE = 'in_use';
    case VALIDATING = 'validating';
    case CLOSED = 'closed';
}

/**
 * Pooled Connection
 *
 * Wraps a connection with metadata for pool management
 */
class PooledConnection
{
    private string $id;
    private string $address;
    private mixed $client;
    private PoolConnectionState $state;
    private DateTimeInterface $lastUsed;
    private DateTimeInterface $createdAt;
    private int $useCount = 0;

    /**
     * Construct a new Pooled Connection
     *
     * @param string $address Connection address
     * @param mixed $client Client instance
     */
    public function __construct(string $address, mixed $client)
    {
        $this->id = $this->generateUuid();
        $this->address = $address;
        $this->client = $client;
        $this->state = PoolConnectionState::AVAILABLE;
        $this->lastUsed = new DateTime();
        $this->createdAt = new DateTime();
    }

    // Getters
    public function getId(): string { return $this->id; }
    public function getAddress(): string { return $this->address; }
    public function getClient(): mixed { return $this->client; }
    public function getState(): PoolConnectionState { return $this->state; }
    public function getLastUsed(): DateTimeInterface { return $this->lastUsed; }
    public function getCreatedAt(): DateTimeInterface { return $this->createdAt; }
    public function getUseCount(): int { return $this->useCount; }

    /**
     * Mark connection as in use
     */
    public function acquire(): void
    {
        $this->state = PoolConnectionState::IN_USE;
        $this->lastUsed = new DateTime();
        $this->useCount++;
    }

    /**
     * Mark connection as available
     */
    public function release(): void
    {
        $this->state = PoolConnectionState::AVAILABLE;
        $this->lastUsed = new DateTime();
    }

    /**
     * Close the connection
     *
     * @return bool true if successfully closed
     */
    public function close(): bool
    {
        $this->state = PoolConnectionState::CLOSED;

        if ($this->client && method_exists($this->client, 'close')) {
            $this->client->close();
            return true;
        }

        return false;
    }

    /**
     * Check if connection is stale (older than max age)
     *
     * @param int $maxAgeSeconds Maximum age in seconds
     * @return bool true if connection is older than max age
     */
    public function isStale(int $maxAgeSeconds): bool
    {
        $now = new DateTime();
        $elapsed = $now->getTimestamp() - $this->createdAt->getTimestamp();
        return $elapsed > $maxAgeSeconds;
    }

    /**
     * Check if connection has been idle too long
     *
     * @param int $idleTimeoutSeconds Idle timeout in seconds
     * @return bool true if connection has been idle longer than timeout
     */
    public function isIdle(int $idleTimeoutSeconds): bool
    {
        $now = new DateTime();
        $elapsed = $now->getTimestamp() - $this->lastUsed->getTimestamp();
        return $elapsed > $idleTimeoutSeconds;
    }

    /**
     * Check if underlying client is connected
     *
     * @return bool true if connected
     */
    public function isConnected(): bool
    {
        if (!$this->client) {
            return false;
        }

        if (method_exists($this->client, 'isConnected')) {
            return $this->client->isConnected();
        }

        return false;
    }

    /**
     * Generate UUID v4
     *
     * @return string UUID string
     */
    private function generateUuid(): string
    {
        $data = random_bytes(16);
        $data[6] = chr(ord($data[6]) & 0x0f | 0x40); // Version 4
        $data[8] = chr(ord($data[8]) & 0x3f | 0x80); // Variant

        return vsprintf('%s%s-%s-%s-%s-%s%s%s', str_split(bin2hex($data), 4));
    }
}

