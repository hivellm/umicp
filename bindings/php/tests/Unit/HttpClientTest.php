<?php

namespace UMICP\Tests\Unit;

use PHPUnit\Framework\TestCase;
use UMICP\Transport\HttpClient;
use UMICP\Core\Envelope;

class HttpClientTest extends TestCase
{
    public function testConstructorWithDefaults(): void
    {
        $client = new HttpClient();
        $this->assertInstanceOf(HttpClient::class, $client);
    }

    public function testConstructorWithOptions(): void
    {
        $client = new HttpClient([
            'baseUrl' => 'http://localhost:8080',
            'path' => '/api/umicp',
            'timeout' => 60,
            'verifySsl' => false,
        ]);

        $this->assertInstanceOf(HttpClient::class, $client);
    }

    public function testGetStats(): void
    {
        $client = new HttpClient();
        $stats = $client->getStats();

        $this->assertIsArray($stats);
        $this->assertArrayHasKey('requests', $stats);
        $this->assertArrayHasKey('responses', $stats);
        $this->assertArrayHasKey('errors', $stats);
        $this->assertEquals(0, $stats['requests']);
    }

    public function testClose(): void
    {
        $client = new HttpClient();
        $client->close();
        $this->assertTrue(true); // Should not throw
    }
}

