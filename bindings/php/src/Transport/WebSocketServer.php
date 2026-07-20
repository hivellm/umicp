<?php

declare(strict_types=1);

namespace UMICP\Transport;

use Evenement\EventEmitter;
use Ratchet\MessageComponentInterface;
use Ratchet\ConnectionInterface;
use Ratchet\Server\IoServer;
use Ratchet\Http\HttpServer;
use Ratchet\WebSocket\WsServer;
use React\EventLoop\LoopInterface;
use React\Promise\PromiseInterface;
use React\Promise\Deferred;
use React\Socket\SocketServer;
use UMICP\Core\Envelope;

/**
 * WebSocket server for UMICP
 *
 * @package UMICP\Transport
 */
class WebSocketServer extends EventEmitter implements MessageComponentInterface
{
    private ?IoServer $server = null;
    private array $clients = [];
    private int $totalConnections = 0;
    private int $messagesSent = 0;
    private int $messagesReceived = 0;
    private array $config;

    public function __construct(
        private LoopInterface $loop,
        array $config = []
    ) {
        $this->config = array_merge([
            'port' => 20081,
            'host' => '0.0.0.0',
            'path' => '/umicp',
            'compression' => true,
            'max_payload' => 100 * 1024 * 1024, // 100MB
            'heartbeat_interval' => 30000,
            'client_timeout' => 60000,
        ], $config);
    }

    public function start(): PromiseInterface
    {
        $deferred = new Deferred();

        try {
            $address = $this->config['host'] . ':' . $this->config['port'];
            $socket = new SocketServer($address, [], $this->loop);

            $wsServer = new WsServer($this);
            $httpServer = new HttpServer($wsServer);
            $this->server = new IoServer($httpServer, $socket, $this->loop);

            $this->emit('listening', [$this->config['host'], $this->config['port']]);

            $deferred->resolve($this->server);
        } catch (\Throwable $e) {
            $deferred->reject($e);
        }

        return $deferred->promise();
    }

    public function stop(): PromiseInterface
    {
        $deferred = new Deferred();

        try {
            if ($this->server) {
                $this->server->socket->close();
                $this->server = null;
            }

            // Disconnect all clients
            foreach ($this->clients as $client) {
                $client['connection']->close();
            }

            $this->clients = [];
            $this->emit('stopped');

            $deferred->resolve(true);
        } catch (\Throwable $e) {
            $deferred->reject($e);
        }

        return $deferred->promise();
    }

    // Ratchet MessageComponentInterface implementation

    public function onOpen(ConnectionInterface $conn): void
    {
        $clientId = spl_object_id($conn);

        $this->clients[$clientId] = [
            'id' => 'client-' . $clientId,
            'connection' => $conn,
            'connected_at' => microtime(true),
            'messages_received' => 0,
            'messages_sent' => 0,
        ];

        $this->totalConnections++;

        $this->emit('client_connected', [$this->clients[$clientId]]);
    }

    public function onMessage(ConnectionInterface $from, $msg): void
    {
        $clientId = spl_object_id($from);

        if (!isset($this->clients[$clientId])) {
            return;
        }

        try {
            $envelope = Envelope::deserialize((string) $msg);

            $this->clients[$clientId]['messages_received']++;
            $this->messagesReceived++;

            $this->emit('message', [$envelope, $this->clients[$clientId]]);
        } catch (\Throwable $e) {
            $this->emit('error', [$e, $this->clients[$clientId] ?? null]);
        }
    }

    public function onClose(ConnectionInterface $conn): void
    {
        $clientId = spl_object_id($conn);

        if (isset($this->clients[$clientId])) {
            $client = $this->clients[$clientId];
            unset($this->clients[$clientId]);

            $this->emit('client_disconnected', [$client, 1000, 'Normal closure']);
        }
    }

    public function onError(ConnectionInterface $conn, \Exception $e): void
    {
        $clientId = spl_object_id($conn);
        $client = $this->clients[$clientId] ?? null;

        $this->emit('error', [$e, $client]);

        $conn->close();
    }

    // Public API methods

    public function sendToClient(string $clientId, Envelope $envelope): bool
    {
        foreach ($this->clients as $client) {
            if ($client['id'] === $clientId) {
                try {
                    $json = $envelope->serialize();
                    $client['connection']->send($json);
                    $client['messages_sent']++;
                    $this->messagesSent++;
                    return true;
                } catch (\Throwable $e) {
                    $this->emit('error', [$e, $client]);
                    return false;
                }
            }
        }

        return false;
    }

    public function broadcast(Envelope $envelope, ?string $excludeClientId = null): int
    {
        $sent = 0;

        try {
            $json = $envelope->serialize();

            foreach ($this->clients as $client) {
                if ($excludeClientId && $client['id'] === $excludeClientId) {
                    continue;
                }

                try {
                    $client['connection']->send($json);
                    $client['messages_sent']++;
                    $sent++;
                } catch (\Throwable $e) {
                    $this->emit('error', [$e, $client]);
                }
            }

            $this->messagesSent += $sent;
        } catch (\Throwable $e) {
            $this->emit('error', [$e, null]);
        }

        return $sent;
    }

    public function getClients(): array
    {
        return array_values($this->clients);
    }

    public function getClientCount(): int
    {
        return count($this->clients);
    }

    public function disconnectClient(string $clientId, int $code = 1000, string $reason = ''): bool
    {
        foreach ($this->clients as $client) {
            if ($client['id'] === $clientId) {
                $client['connection']->close();
                return true;
            }
        }

        return false;
    }

    public function getStats(): array
    {
        return [
            'server_running' => $this->server !== null,
            'host' => $this->config['host'],
            'port' => $this->config['port'],
            'connected_clients' => count($this->clients),
            'total_connections' => $this->totalConnections,
            'messages_sent' => $this->messagesSent,
            'messages_received' => $this->messagesReceived,
            'clients' => array_map(function ($client) {
                return [
                    'id' => $client['id'],
                    'connected_at' => $client['connected_at'],
                    'messages_received' => $client['messages_received'],
                    'messages_sent' => $client['messages_sent'],
                    'uptime' => microtime(true) - $client['connected_at'],
                ];
            }, $this->clients),
        ];
    }

    public function __destruct()
    {
        if ($this->server) {
            try {
                $this->stop();
            } catch (\Throwable $e) {
                // Ignore errors in destructor
            }
        }
    }
}

