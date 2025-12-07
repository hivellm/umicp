<?php

declare(strict_types=1);

namespace UMICP\Transport;

use Evenement\EventEmitter;
use Ratchet\Client\Connector;
use Ratchet\Client\WebSocket;
use React\EventLoop\LoopInterface;
use React\Promise\PromiseInterface;
use React\Socket\Connector as SocketConnector;
use UMICP\Core\Envelope;
use UMICP\Exception\ConnectionException;
use UMICP\Exception\TimeoutException;

/**
 * WebSocket client for UMICP
 *
 * @package UMICP\Transport
 */
class WebSocketClient extends EventEmitter
{
    private ?WebSocket $connection = null;
    private ConnectionState $state;
    private array $config;
    private int $reconnectAttempts = 0;
    private ?object $reconnectTimer = null;
    private bool $isReconnecting = false;
    private array $pendingMessages = [];
    private ?object $heartbeatTimer = null;

    public function __construct(
        private LoopInterface $loop,
        array $config = []
    ) {
        $this->config = array_merge([
            'url' => 'ws://localhost:8080',
            'compression' => true,
            'auto_reconnect' => true,
            'reconnect_delay' => 5000,
            'max_reconnect_attempts' => 5,
            'heartbeat_interval' => 30000,
            'connection_timeout' => 10000,
        ], $config);

        $this->state = new ConnectionState();
    }

    public function connect(): PromiseInterface
    {
        $connector = new Connector($this->loop, new SocketConnector($this->loop));

        return $connector($this->config['url'])
            ->then(
                function (WebSocket $conn) {
                    $this->connection = $conn;
                    $this->state->setConnected(true);
                    $this->reconnectAttempts = 0;
                    $this->isReconnecting = false;

                    $this->setupConnection($conn);
                    $this->emit('connected');

                    // Send pending messages
                    $this->flushPendingMessages();

                    return $conn;
                },
                function (\Exception $e) {
                    $this->emit('error', [$e]);

                    if ($this->config['auto_reconnect'] &&
                        $this->reconnectAttempts < $this->config['max_reconnect_attempts']) {
                        $this->scheduleReconnect();
                    } else {
                        $this->emit('connect_failed', [$e]);
                    }

                    throw $e;
                }
            );
    }

    private function setupConnection(WebSocket $conn): void
    {
        // Message handler
        $conn->on('message', function ($msg) {
            try {
                $envelope = Envelope::deserialize((string) $msg);
                $this->state->incrementMessagesReceived();
                $this->state->addBytesReceived(strlen((string) $msg));
                $this->emit('message', [$envelope]);
            } catch (\Throwable $e) {
                $this->emit('error', [$e]);
            }
        });

        // Close handler
        $conn->on('close', function ($code = null, $reason = null) {
            $this->state->setConnected(false);
            $this->connection = null;

            if ($this->heartbeatTimer) {
                $this->loop->cancelTimer($this->heartbeatTimer);
                $this->heartbeatTimer = null;
            }

            $this->emit('disconnected', [$code, $reason]);

            if ($this->config['auto_reconnect'] && !$this->isReconnecting) {
                $this->scheduleReconnect();
            }
        });

        // Error handler
        $conn->on('error', function ($e) {
            $this->emit('error', [$e]);
        });

        // Setup heartbeat
        if ($this->config['heartbeat_interval'] > 0) {
            $this->setupHeartbeat($conn);
        }
    }

    private function setupHeartbeat(WebSocket $conn): void
    {
        $interval = $this->config['heartbeat_interval'] / 1000; // Convert to seconds

        $this->heartbeatTimer = $this->loop->addPeriodicTimer($interval, function () use ($conn) {
            if ($this->isConnected()) {
                try {
                    $conn->send(json_encode(['type' => 'ping', 'timestamp' => time()]));
                } catch (\Throwable $e) {
                    $this->emit('error', [$e]);
                }
            }
        });
    }

    public function send(Envelope $envelope): bool
    {
        if (!$this->isConnected()) {
            // Queue message if auto-reconnect is enabled
            if ($this->config['auto_reconnect']) {
                $this->pendingMessages[] = $envelope;
                return false;
            }
            return false;
        }

        try {
            $json = $envelope->serialize();
            $this->connection->send($json);
            $this->state->incrementMessagesSent();
            $this->state->addBytesSent(strlen($json));
            return true;
        } catch (\Throwable $e) {
            $this->emit('error', [$e]);
            return false;
        }
    }

    public function disconnect(): void
    {
        $this->config['auto_reconnect'] = false; // Disable auto-reconnect

        if ($this->reconnectTimer) {
            $this->loop->cancelTimer($this->reconnectTimer);
            $this->reconnectTimer = null;
        }

        if ($this->heartbeatTimer) {
            $this->loop->cancelTimer($this->heartbeatTimer);
            $this->heartbeatTimer = null;
        }

        if ($this->connection) {
            $this->connection->close();
            $this->connection = null;
        }

        $this->state->setConnected(false);
        $this->pendingMessages = [];
    }

    public function isConnected(): bool
    {
        return $this->state->isConnected() && $this->connection !== null;
    }

    public function getStats(): array
    {
        return array_merge([
            'url' => $this->config['url'],
            'reconnect_attempts' => $this->reconnectAttempts,
            'pending_messages' => count($this->pendingMessages),
        ], $this->state->toArray());
    }

    private function scheduleReconnect(): void
    {
        if ($this->isReconnecting) {
            return;
        }

        $this->isReconnecting = true;
        $this->reconnectAttempts++;

        $delay = $this->config['reconnect_delay'] / 1000; // Convert to seconds

        $this->reconnectTimer = $this->loop->addTimer($delay, function () {
            $this->connect()->then(
                null,
                function () {
                    $this->isReconnecting = false;
                }
            );
        });

        $this->emit('reconnecting', [$this->reconnectAttempts, $this->config['max_reconnect_attempts']]);
    }

    private function flushPendingMessages(): void
    {
        if (empty($this->pendingMessages)) {
            return;
        }

        $count = 0;
        foreach ($this->pendingMessages as $envelope) {
            if ($this->send($envelope)) {
                $count++;
            }
        }

        $this->pendingMessages = [];

        if ($count > 0) {
            $this->emit('messages_flushed', [$count]);
        }
    }

    public function sendAndWait(Envelope $envelope, int $timeout = 30000): PromiseInterface
    {
        $deferred = new \React\Promise\Deferred();
        $responded = false;
        $timer = null;

        // Setup response handler
        $responseHandler = function (Envelope $response) use ($envelope, $deferred, &$responded, &$timer) {
            // Check if this is a response to our message
            $msgId = $envelope->getMessageId();
            $responseMsgId = $response->getCapability('in_reply_to');

            if ($msgId && $responseMsgId === $msgId) {
                $responded = true;

                if ($timer) {
                    $this->loop->cancelTimer($timer);
                }

                $this->removeListener('message', $responseHandler);
                $deferred->resolve($response);
            }
        };

        $this->on('message', $responseHandler);

        // Setup timeout
        $timer = $this->loop->addTimer($timeout / 1000, function () use ($deferred, &$responded, $responseHandler) {
            if (!$responded) {
                $this->removeListener('message', $responseHandler);
                $deferred->reject(new TimeoutException(
                    "No response received within {$this->config['connection_timeout']}ms"
                ));
            }
        });

        // Send message
        if (!$this->send($envelope)) {
            $this->loop->cancelTimer($timer);
            $this->removeListener('message', $responseHandler);
            $deferred->reject(new ConnectionException('Failed to send message'));
        }

        return $deferred->promise();
    }
}

