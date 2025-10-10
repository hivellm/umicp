<?php

namespace UMICP\Transport;

use UMICP\Core\Envelope;
use UMICP\Exception\TransportException;

/**
 * UMICP HTTP Server
 *
 * Simple HTTP/1.1 server for UMICP envelope transport
 * Uses PHP's built-in server or can integrate with frameworks
 *
 * @package HiveLLM\UMICP\Transport
 */
class HttpServer
{
    private string $host;
    private int $port;
    private string $path;
    private $messageHandler;
    private $requestHandler;
    private array $stats;
    private bool $running = false;

    /**
     * Constructor
     *
     * @param array $options Configuration options
     */
    public function __construct(array $options = [])
    {
        $this->host = $options['host'] ?? '0.0.0.0';
        $this->port = $options['port'] ?? 9080;
        $this->path = $options['path'] ?? '/umicp';

        $this->stats = [
            'requests' => 0,
            'responses' => 0,
            'errors' => 0,
            'bytes_received' => 0,
            'bytes_sent' => 0,
        ];
    }

    /**
     * Set message handler
     *
     * @param callable $handler Handler function(Envelope $envelope): Envelope
     */
    public function onMessage(callable $handler): void
    {
        $this->messageHandler = $handler;
    }

    /**
     * Set request handler for specific path
     *
     * @param string $path Request path
     * @param callable $handler Handler function(array $request): array
     */
    public function onRequest(string $path, callable $handler): void
    {
        $this->requestHandler[$path] = $handler;
    }

    /**
     * Handle incoming HTTP request
     * This method is called by the HTTP server (PHP built-in or framework)
     *
     * @return array Response data
     */
    public function handleRequest(): array
    {
        $this->stats['requests']++;

        // Get request method and path
        $method = $_SERVER['REQUEST_METHOD'] ?? 'GET';
        $requestPath = $_SERVER['REQUEST_URI'] ?? '/';

        // Health check endpoint
        if ($requestPath === '/health' && $method === 'GET') {
            return [
                'status' => 200,
                'body' => json_encode(['status' => 'ok']),
                'headers' => ['Content-Type' => 'application/json'],
            ];
        }

        // UMICP message endpoint
        if ($requestPath === $this->path && $method === 'POST') {
            try {
                $body = file_get_contents('php://input');
                $this->stats['bytes_received'] += strlen($body);

                $data = json_decode($body, true);
                if ($data === null) {
                    $this->stats['errors']++;
                    return [
                        'status' => 400,
                        'body' => json_encode(['error' => 'Invalid JSON']),
                        'headers' => ['Content-Type' => 'application/json'],
                    ];
                }

                $envelope = Envelope::fromArray($data);

                if ($this->messageHandler) {
                    $response = ($this->messageHandler)($envelope);

                    if ($response instanceof Envelope) {
                        $responseBody = json_encode($response->toArray());
                        $this->stats['responses']++;
                        $this->stats['bytes_sent'] += strlen($responseBody);

                        return [
                            'status' => 200,
                            'body' => $responseBody,
                            'headers' => ['Content-Type' => 'application/json'],
                        ];
                    }
                }

                // Default ACK response
                $ack = Envelope::builder()
                    ->from('http-server')
                    ->to($envelope->from)
                    ->operation(\UMICP\Core\OperationType::ACK)
                    ->build();

                $responseBody = json_encode($ack->toArray());
                $this->stats['responses']++;
                $this->stats['bytes_sent'] += strlen($responseBody);

                return [
                    'status' => 200,
                    'body' => $responseBody,
                    'headers' => ['Content-Type' => 'application/json'],
                ];

            } catch (\Exception $e) {
                $this->stats['errors']++;
                return [
                    'status' => 500,
                    'body' => json_encode(['error' => $e->getMessage()]),
                    'headers' => ['Content-Type' => 'application/json'],
                ];
            }
        }

        // Custom request handlers
        if (isset($this->requestHandler[$requestPath]) && $method === 'POST') {
            try {
                $body = file_get_contents('php://input');
                $data = json_decode($body, true);

                $result = ($this->requestHandler[$requestPath])($data ?? []);

                return [
                    'status' => 200,
                    'body' => json_encode($result),
                    'headers' => ['Content-Type' => 'application/json'],
                ];

            } catch (\Exception $e) {
                return [
                    'status' => 500,
                    'body' => json_encode(['error' => $e->getMessage()]),
                    'headers' => ['Content-Type' => 'application/json'],
                ];
            }
        }

        // 404 Not Found
        return [
            'status' => 404,
            'body' => json_encode(['error' => 'Not Found']),
            'headers' => ['Content-Type' => 'application/json'],
        ];
    }

    /**
     * Get statistics
     *
     * @return array Statistics
     */
    public function getStats(): array
    {
        return $this->stats;
    }

    /**
     * Start built-in PHP server (for development)
     *
     * @return void
     */
    public function start(): void
    {
        $this->running = true;
        echo "HTTP Server starting on {$this->host}:{$this->port}\n";
        echo "UMICP path: {$this->path}\n";
        echo "Health check: /health\n";

        // Note: For production, use with frameworks (Laravel, Symfony)
        // or web servers (Apache, Nginx)
    }

    /**
     * Stop server
     */
    public function stop(): void
    {
        $this->running = false;
    }
}

