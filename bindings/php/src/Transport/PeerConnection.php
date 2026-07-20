<?php

declare(strict_types=1);

namespace UMICP\Transport;

use DateTime;

/**
 * Peer connection information
 *
 * @package UMICP\Transport
 */
class PeerConnection
{
    public function __construct(
        public readonly string $id,
        public readonly string $type, // 'incoming' or 'outgoing'
        public readonly ?string $url,
        public readonly object $client,
        public array $metadata = [],
        public readonly DateTime $connectedAt = new DateTime(),
        public bool $handshakeComplete = false,
        public ?PeerInfo $peerInfo = null
    ) {
    }

    public function toArray(): array
    {
        return [
            'id' => $this->id,
            'type' => $this->type,
            'url' => $this->url,
            'metadata' => $this->metadata,
            'connected_at' => $this->connectedAt->format('c'),
            'handshake_complete' => $this->handshakeComplete,
            'peer_info' => $this->peerInfo?->toArray(),
        ];
    }
}

