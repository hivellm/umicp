<?php

declare(strict_types=1);

namespace UMICP\Transport;

/**
 * Connection state tracking
 *
 * @package UMICP\Transport
 */
class ConnectionState
{
    private bool $connected = false;
    private int $messagesSent = 0;
    private int $messagesReceived = 0;
    private int $bytesReceived = 0;
    private int $bytesSent = 0;
    private ?float $connectedAt = null;
    private ?float $disconnectedAt = null;

    public function setConnected(bool $connected): void
    {
        $this->connected = $connected;

        if ($connected) {
            $this->connectedAt = microtime(true);
            $this->disconnectedAt = null;
        } else {
            $this->disconnectedAt = microtime(true);
        }
    }

    public function isConnected(): bool
    {
        return $this->connected;
    }

    public function incrementMessagesSent(): void
    {
        $this->messagesSent++;
    }

    public function incrementMessagesReceived(): void
    {
        $this->messagesReceived++;
    }

    public function addBytesSent(int $bytes): void
    {
        $this->bytesSent += $bytes;
    }

    public function addBytesReceived(int $bytes): void
    {
        $this->bytesReceived += $bytes;
    }

    public function getMessagesSent(): int
    {
        return $this->messagesSent;
    }

    public function getMessagesReceived(): int
    {
        return $this->messagesReceived;
    }

    public function getBytesSent(): int
    {
        return $this->bytesSent;
    }

    public function getBytesReceived(): int
    {
        return $this->bytesReceived;
    }

    public function getConnectedAt(): ?float
    {
        return $this->connectedAt;
    }

    public function getDisconnectedAt(): ?float
    {
        return $this->disconnectedAt;
    }

    public function getUptime(): ?float
    {
        if ($this->connectedAt === null) {
            return null;
        }

        $endTime = $this->disconnectedAt ?? microtime(true);
        return $endTime - $this->connectedAt;
    }

    public function reset(): void
    {
        $this->connected = false;
        $this->messagesSent = 0;
        $this->messagesReceived = 0;
        $this->bytesSent = 0;
        $this->bytesReceived = 0;
        $this->connectedAt = null;
        $this->disconnectedAt = null;
    }

    public function toArray(): array
    {
        return [
            'connected' => $this->connected,
            'messages_sent' => $this->messagesSent,
            'messages_received' => $this->messagesReceived,
            'bytes_sent' => $this->bytesSent,
            'bytes_received' => $this->bytesReceived,
            'connected_at' => $this->connectedAt,
            'disconnected_at' => $this->disconnectedAt,
            'uptime' => $this->getUptime(),
        ];
    }
}

