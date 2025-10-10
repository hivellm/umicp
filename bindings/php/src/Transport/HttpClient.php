<?php

namespace UMICP\Transport;

use UMICP\Core\Envelope;
use UMICP\Exception\TransportException;
use UMICP\Exception\ConnectionException;
use UMICP\Exception\TimeoutException;

/**
 * UMICP HTTP Client
 *
 * HTTP/1.1 and HTTP/2 client for UMICP envelope transport
 * Uses cURL for maximum compatibility and performance
 *
 * @package HiveLLM\UMICP\Transport
 */
class HttpClient
{
    private string $baseUrl;
    private string $path;
    private int $timeout;
    private bool $verifySsl;
    private array $headers;
    private $curlHandle;
    private array $stats;

    /**
     * Constructor
     *
     * @param array $options Configuration options
     */
    public function __construct(array $options = [])
    {
        $this->baseUrl = $options['baseUrl'] ?? 'http://localhost:9080';
        $this->path = $options['path'] ?? '/umicp';
        $this->timeout = $options['timeout'] ?? 30;
        $this->verifySsl = $options['verifySsl'] ?? true;
        $this->headers = $options['headers'] ?? [];

        $this->stats = [
            'requests' => 0,
            'responses' => 0,
            'errors' => 0,
            'bytes_sent' => 0,
            'bytes_received' => 0,
        ];

        $this->initCurl();
    }

    /**
     * Initialize cURL handle
     */
    private function initCurl(): void
    {
        $this->curlHandle = curl_init();

        curl_setopt_array($this->curlHandle, [
            CURLOPT_RETURNTRANSFER => true,
            CURLOPT_FOLLOWLOCATION => true,
            CURLOPT_MAXREDIRS => 5,
            CURLOPT_TIMEOUT => $this->timeout,
            CURLOPT_SSL_VERIFYPEER => $this->verifySsl,
            CURLOPT_SSL_VERIFYHOST => $this->verifySsl ? 2 : 0,
        ]);
    }

    /**
     * Send envelope via HTTP POST
     *
     * @param Envelope $envelope Envelope to send
     * @return Envelope Response envelope
     * @throws TransportException
     */
    public function send(Envelope $envelope): Envelope
    {
        $url = rtrim($this->baseUrl, '/') . '/' . ltrim($this->path, '/');
        $json = json_encode($envelope->toArray());

        if ($json === false) {
            throw new TransportException('Failed to encode envelope');
        }

        $headers = array_merge($this->headers, [
            'Content-Type: application/json',
            'Accept: application/json',
            'Content-Length: ' . strlen($json),
        ]);

        curl_setopt_array($this->curlHandle, [
            CURLOPT_URL => $url,
            CURLOPT_POST => true,
            CURLOPT_POSTFIELDS => $json,
            CURLOPT_HTTPHEADER => $headers,
        ]);

        $this->stats['requests']++;
        $this->stats['bytes_sent'] += strlen($json);

        $response = curl_exec($this->curlHandle);
        $httpCode = curl_getinfo($this->curlHandle, CURLINFO_HTTP_CODE);

        if ($response === false) {
            $this->stats['errors']++;
            $error = curl_error($this->curlHandle);
            throw new ConnectionException("HTTP request failed: $error");
        }

        if ($httpCode !== 200) {
            $this->stats['errors']++;
            throw new TransportException("HTTP error $httpCode: $response");
        }

        $this->stats['responses']++;
        $this->stats['bytes_received'] += strlen($response);

        $data = json_decode($response, true);
        if ($data === null) {
            throw new TransportException('Invalid JSON response');
        }

        return Envelope::fromArray($data);
    }

    /**
     * Health check
     *
     * @return bool True if server is healthy
     */
    public function healthCheck(): bool
    {
        $url = rtrim($this->baseUrl, '/') . '/health';

        curl_setopt_array($this->curlHandle, [
            CURLOPT_URL => $url,
            CURLOPT_HTTPGET => true,
        ]);

        $response = curl_exec($this->curlHandle);
        $httpCode = curl_getinfo($this->curlHandle, CURLINFO_HTTP_CODE);

        return $response !== false && $httpCode === 200;
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
     * Close connection
     */
    public function close(): void
    {
        if ($this->curlHandle) {
            curl_close($this->curlHandle);
            $this->curlHandle = null;
        }
    }

    /**
     * Destructor
     */
    public function __destruct()
    {
        $this->close();
    }
}

