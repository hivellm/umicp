<?php

declare(strict_types=1);

namespace UMICP\Transport;

use DateTime;

/**
 * Peer information from handshake
 *
 * @package UMICP\Transport
 */
class PeerInfo
{
    public function __construct(
        public readonly string $peerId,
        public readonly array $metadata = [],
        public readonly array $capabilities = [],
        public readonly DateTime $handshakeCompletedAt = new DateTime()
    ) {
    }

    public function toArray(): array
    {
        return [
            'peer_id' => $this->peerId,
            'metadata' => $this->metadata,
            'capabilities' => $this->capabilities,
            'handshake_completed_at' => $this->handshakeCompletedAt->format('c'),
        ];
    }
}

