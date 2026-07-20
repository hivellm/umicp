<?php

declare(strict_types=1);

namespace UMICP\Transport;

use Evenement\EventEmitter;
use React\EventLoop\LoopInterface;
use React\Promise\PromiseInterface;
use React\Promise\Deferred;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

/**
 * Multiplexed Peer - Each peer can be both server AND client
 *
 * @package UMICP\Transport
 */
class MultiplexedPeer extends EventEmitter
{
    private ?WebSocketServer $server = null;
    private array $peers = [];
    private array $config;

    public function __construct(
        private string $peerId,
        private LoopInterface $loop,
        ?array $serverConfig = null,
        private array $metadata = []
    ) {
        $this->config = [
            'auto_protocol' => true,
            'handshake_timeout' => 10000,
        ];

        // Start server if config provided
        if ($serverConfig !== null) {
            $this->initializeServer($serverConfig);
        }
    }

    private function initializeServer(array $serverConfig): void
    {
        $this->server = new WebSocketServer($this->loop, $serverConfig);

        // Handle incoming connections
        $this->server->on('client_connected', function ($client) {
            $peerId = 'incoming-' . $client['id'];

            $peerConnection = new PeerConnection(
                id: $peerId,
                type: 'incoming',
                url: null,
                client: $client,
                metadata: []
            );

            $this->peers[$peerId] = $peerConnection;

            $this->emit('peer:connect', [$peerConnection]);
        });

        $this->server->on('message', function (Envelope $envelope, $client) {
            $peerId = 'incoming-' . $client['id'];
            $peer = $this->peers[$peerId] ?? null;

            if ($peer) {
                $this->handleMessage($envelope, $peer);
            }
        });

        $this->server->on('client_disconnected', function ($client) {
            $peerId = 'incoming-' . $client['id'];
            $peer = $this->peers[$peerId] ?? null;

            if ($peer) {
                unset($this->peers[$peerId]);
                $this->emit('peer:disconnect', [$peer]);
            }
        });

        $this->server->on('error', function ($error, $client = null) {
            $peer = null;
            if ($client) {
                $peerId = 'incoming-' . $client['id'];
                $peer = $this->peers[$peerId] ?? null;
            }
            $this->emit('error', [$error, $peer]);
        });

        // Start server
        $this->server->start()->then(
            function () {
                $this->emit('server:ready');
            },
            function ($error) {
                $this->emit('error', [$error, null]);
            }
        );
    }

    public function connectToPeer(string $url, array $metadata = []): PromiseInterface
    {
        $deferred = new Deferred();

        $client = new WebSocketClient($this->loop, [
            'url' => $url,
            'compression' => true,
            'auto_reconnect' => true,
        ]);

        $client->on('connected', function () use ($client, $url, $metadata, $deferred) {
            $peerId = 'outgoing-' . spl_object_id($client);

            $peerConnection = new PeerConnection(
                id: $peerId,
                type: 'outgoing',
                url: $url,
                client: $client,
                metadata: $metadata
            );

            $this->peers[$peerId] = $peerConnection;

            $this->emit('peer:connect', [$peerConnection]);

            // Send hello if auto_protocol enabled
            if ($this->config['auto_protocol']) {
                $this->sendHelloMessage($peerConnection);
            }

            $deferred->resolve($peerId);
        });

        $client->on('message', function (Envelope $envelope) use ($client) {
            $peerId = 'outgoing-' . spl_object_id($client);
            $peer = $this->peers[$peerId] ?? null;

            if ($peer) {
                $this->handleMessage($envelope, $peer);
            }
        });

        $client->on('disconnected', function () use ($client) {
            $peerId = 'outgoing-' . spl_object_id($client);
            $peer = $this->peers[$peerId] ?? null;

            if ($peer) {
                unset($this->peers[$peerId]);
                $this->emit('peer:disconnect', [$peer]);
            }
        });

        $client->on('error', function ($error) use ($client) {
            $peerId = 'outgoing-' . spl_object_id($client);
            $peer = $this->peers[$peerId] ?? null;
            $this->emit('error', [$error, $peer]);
        });

        $client->connect()->then(
            null,
            function ($error) use ($deferred) {
                $deferred->reject($error);
            }
        );

        return $deferred->promise();
    }

    private function sendHelloMessage(PeerConnection $peer): void
    {
        $helloEnvelope = new Envelope(
            from: $this->peerId,
            to: 'peer',
            operation: OperationType::CONTROL,
            messageId: 'hello-' . uniqid(),
            capabilities: array_merge([
                'action' => 'hello',
                'peerId' => $this->peerId,
                'version' => '1.0.0',
            ], $this->buildMetadataCapabilities())
        );

        if ($peer->type === 'outgoing' && $peer->client instanceof WebSocketClient) {
            $peer->client->send($helloEnvelope);
        }
    }

    private function buildMetadataCapabilities(): array
    {
        $caps = [];
        foreach ($this->metadata as $key => $value) {
            $caps['meta_' . $key] = (string) $value;
        }
        return $caps;
    }

    private function handleMessage(Envelope $envelope, PeerConnection $peer): void
    {
        $operation = $envelope->getOperation();

        // Auto-protocol handling
        if ($operation === OperationType::CONTROL) {
            $action = $envelope->getCapability('action');

            if ($action === 'hello') {
                $this->handleHelloMessage($envelope, $peer);
                return;
            } elseif ($action === 'handshake_complete') {
                $this->handleHandshakeComplete($envelope, $peer);
                return;
            }
        }

        // Emit generic message event
        $this->emit('message', [$envelope, $peer]);

        // Emit operation-specific events
        match ($operation) {
            OperationType::DATA => $this->emit('data', [$envelope, $peer]),
            OperationType::CONTROL => $this->emit('control', [$envelope, $peer]),
            OperationType::ACK => $this->emit('ack', [$envelope, $peer]),
            OperationType::ERROR => $this->emit('error_message', [$envelope, $peer]),
            default => null
        };
    }

    private function handleHelloMessage(Envelope $envelope, PeerConnection $peer): void
    {
        $caps = $envelope->getCapabilities();
        $remotePeerId = $caps['peerId'] ?? $envelope->getFrom();

        // Extract peer info
        $peerMetadata = [];
        foreach ($caps as $key => $value) {
            if (str_starts_with($key, 'meta_')) {
                $peerMetadata[substr($key, 5)] = $value;
            }
        }

        $peerInfo = new PeerInfo(
            peerId: $remotePeerId,
            metadata: $peerMetadata,
            capabilities: $caps
        );

        $peer->peerInfo = $peerInfo;
        $peer->handshakeComplete = true;

        // Send ACK
        $ackEnvelope = new Envelope(
            from: $this->peerId,
            to: $envelope->getFrom(),
            operation: OperationType::ACK,
            messageId: 'ack-' . $envelope->getMessageId(),
            capabilities: array_merge([
                'status' => 'handshake_complete',
                'peerId' => $this->peerId,
            ], $this->buildMetadataCapabilities())
        );

        $this->sendToPeer($peer->id, $ackEnvelope);

        $this->emit('peer:ready', [$peer, $peerInfo]);
    }

    private function handleHandshakeComplete(Envelope $envelope, PeerConnection $peer): void
    {
        $caps = $envelope->getCapabilities();

        $peerMetadata = [];
        foreach ($caps as $key => $value) {
            if (str_starts_with($key, 'meta_')) {
                $peerMetadata[substr($key, 5)] = $value;
            }
        }

        $peerInfo = new PeerInfo(
            peerId: $caps['peerId'] ?? $envelope->getFrom(),
            metadata: $peerMetadata,
            capabilities: $caps
        );

        $peer->peerInfo = $peerInfo;
        $peer->handshakeComplete = true;

        $this->emit('peer:ready', [$peer, $peerInfo]);
    }

    public function sendToPeer(string $peerId, Envelope $envelope): bool
    {
        $peer = $this->peers[$peerId] ?? null;
        if (!$peer) {
            return false;
        }

        try {
            if ($peer->type === 'outgoing' && $peer->client instanceof WebSocketClient) {
                return $peer->client->send($envelope);
            } elseif ($peer->type === 'incoming' && $this->server) {
                return $this->server->sendToClient($peer->client['id'], $envelope);
            }
        } catch (\Throwable $e) {
            $this->emit('error', [$e, $peer]);
        }

        return false;
    }

    public function broadcast(Envelope $envelope, ?string $excludePeerId = null): int
    {
        $sent = 0;

        foreach ($this->peers as $peerId => $peer) {
            if ($excludePeerId && $peerId === $excludePeerId) {
                continue;
            }

            if ($this->sendToPeer($peerId, $envelope)) {
                $sent++;
            }
        }

        return $sent;
    }

    public function broadcastToType(Envelope $envelope, string $type, ?string $excludePeerId = null): int
    {
        $sent = 0;

        foreach ($this->peers as $peerId => $peer) {
            if ($peer->type !== $type) {
                continue;
            }

            if ($excludePeerId && $peerId === $excludePeerId) {
                continue;
            }

            if ($this->sendToPeer($peerId, $envelope)) {
                $sent++;
            }
        }

        return $sent;
    }

    public function getPeers(): array
    {
        return array_values($this->peers);
    }

    public function getPeersByType(string $type): array
    {
        return array_values(array_filter(
            $this->peers,
            fn($peer) => $peer->type === $type
        ));
    }

    public function getPeer(string $peerId): ?PeerConnection
    {
        return $this->peers[$peerId] ?? null;
    }

    public function findPeerByMetadata(string $key, mixed $value): ?PeerConnection
    {
        foreach ($this->peers as $peer) {
            if (isset($peer->metadata[$key]) && $peer->metadata[$key] === $value) {
                return $peer;
            }
        }

        return null;
    }

    public function disconnectPeer(string $peerId): bool
    {
        $peer = $this->peers[$peerId] ?? null;
        if (!$peer) {
            return false;
        }

        if ($peer->type === 'outgoing' && $peer->client instanceof WebSocketClient) {
            $peer->client->disconnect();
        } elseif ($peer->type === 'incoming' && $this->server) {
            $this->server->disconnectClient($peer->client['id']);
        }

        unset($this->peers[$peerId]);
        return true;
    }

    public function getStats(): array
    {
        $incoming = $this->getPeersByType('incoming');
        $outgoing = $this->getPeersByType('outgoing');

        return [
            'peer_id' => $this->peerId,
            'total_peers' => count($this->peers),
            'incoming_connections' => count($incoming),
            'outgoing_connections' => count($outgoing),
            'server_active' => $this->server !== null,
            'server_stats' => $this->server?->getStats(),
            'peers' => array_map(fn($peer) => $peer->toArray(), $this->peers),
        ];
    }

    public function shutdown(): PromiseInterface
    {
        $deferred = new Deferred();

        try {
            // Disconnect all peers
            foreach ($this->peers as $peerId => $peer) {
                $this->disconnectPeer($peerId);
            }

            $this->peers = [];

            // Stop server
            if ($this->server) {
                $this->server->stop()->then(
                    function () use ($deferred) {
                        $deferred->resolve(true);
                    },
                    function ($error) use ($deferred) {
                        $deferred->reject($error);
                    }
                );
            } else {
                $deferred->resolve(true);
            }
        } catch (\Throwable $e) {
            $deferred->reject($e);
        }

        return $deferred->promise();
    }
}

