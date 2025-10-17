<?php

namespace UMICP\Tests\Unit;

use PHPUnit\Framework\TestCase;
use UMICP\Transport\HttpClient;

/**
 * Custom Endpoint Support Tests (v0.2.2)
 *
 * Verifies that the PHP UMICP implementation supports
 * custom endpoint paths for compatibility with different servers
 * (e.g., Vectorizer uses /umicp, standard servers use /message)
 */
class CustomEndpointTest extends TestCase
{
    public function testClientWithDefaultPath()
    {
        $client = new HttpClient();

        $this->assertNotNull($client);
        // Default path should be /umicp
        $reflection = new \ReflectionClass($client);
        $property = $reflection->getProperty('path');
        $property->setAccessible(true);
        $this->assertEquals('/umicp', $property->getValue($client));
    }

    public function testClientWithCustomPathVectorizer()
    {
        $client = new HttpClient([
            'baseUrl' => 'http://localhost:8000',
            'path' => '/umicp'
        ]);

        $this->assertNotNull($client);

        $reflection = new \ReflectionClass($client);
        $property = $reflection->getProperty('path');
        $property->setAccessible(true);
        $this->assertEquals('/umicp', $property->getValue($client));
    }

    public function testClientWithCustomPathStandard()
    {
        $client = new HttpClient([
            'baseUrl' => 'http://localhost:9000',
            'path' => '/message'
        ]);

        $this->assertNotNull($client);

        $reflection = new \ReflectionClass($client);
        $property = $reflection->getProperty('path');
        $property->setAccessible(true);
        $this->assertEquals('/message', $property->getValue($client));
    }

    public function testClientWithTrailingSlashInBaseUrl()
    {
        $client = new HttpClient([
            'baseUrl' => 'http://localhost:8000/',
            'path' => '/umicp'
        ]);

        $this->assertNotNull($client);

        $reflection = new \ReflectionClass($client);
        $baseUrlProp = $reflection->getProperty('baseUrl');
        $baseUrlProp->setAccessible(true);
        $this->assertEquals('http://localhost:8000/', $baseUrlProp->getValue($client));
    }

    public function testMultipleClientsWithDifferentEndpoints()
    {
        $vectorizerClient = new HttpClient([
            'baseUrl' => 'http://localhost:8000',
            'path' => '/umicp'
        ]);

        $standardClient = new HttpClient([
            'baseUrl' => 'http://localhost:9000',
            'path' => '/message'
        ]);

        $this->assertNotNull($vectorizerClient);
        $this->assertNotNull($standardClient);
        $this->assertNotSame($vectorizerClient, $standardClient);
    }

    public function testClientPathIsStoredCorrectly()
    {
        $client = new HttpClient([
            'baseUrl' => 'http://localhost:8000',
            'path' => '/custom'
        ]);

        $reflection = new \ReflectionClass($client);
        $property = $reflection->getProperty('path');
        $property->setAccessible(true);
        $this->assertEquals('/custom', $property->getValue($client));
    }

    public function testDefaultPathValue()
    {
        $client1 = new HttpClient();
        $client2 = new HttpClient(['path' => '/umicp']);

        $reflection = new \ReflectionClass($client1);
        $property = $reflection->getProperty('path');
        $property->setAccessible(true);

        $path1 = $property->getValue($client1);
        $path2 = $property->getValue($client2);

        $this->assertEquals($path1, $path2);
        $this->assertEquals('/umicp', $path1);
    }

    public function testVectorizerConfiguration()
    {
        $client = new HttpClient([
            'baseUrl' => 'http://localhost:8000',
            'path' => '/umicp'
        ]);

        $this->assertNotNull($client);

        $reflection = new \ReflectionClass($client);
        $baseUrlProp = $reflection->getProperty('baseUrl');
        $baseUrlProp->setAccessible(true);
        $pathProp = $reflection->getProperty('path');
        $pathProp->setAccessible(true);

        $this->assertEquals('http://localhost:8000', $baseUrlProp->getValue($client));
        $this->assertEquals('/umicp', $pathProp->getValue($client));
    }

    public function testVersion_0_2_2_SupportsCustomEndpoints()
    {
        // Test that v0.2.2 supports custom endpoints
        $client1 = new HttpClient(['path' => '/umicp']);
        $client2 = new HttpClient(['path' => '/message']);

        $this->assertNotNull($client1);
        $this->assertNotNull($client2);
    }

    public function testBackwardCompatibility_OldCodeStillWorks()
    {
        // Old code that creates client without specifying path
        $client = new HttpClient(['baseUrl' => 'http://localhost:8000']);

        $this->assertNotNull($client);

        $reflection = new \ReflectionClass($client);
        $property = $reflection->getProperty('path');
        $property->setAccessible(true);
        // Should use default path
        $this->assertEquals('/umicp', $property->getValue($client));
    }

    public function testEmptyPath()
    {
        $client = new HttpClient([
            'baseUrl' => 'http://localhost:8000',
            'path' => ''
        ]);

        $this->assertNotNull($client);

        $reflection = new \ReflectionClass($client);
        $property = $reflection->getProperty('path');
        $property->setAccessible(true);
        $this->assertEquals('', $property->getValue($client));
    }

    public function testComplexPath()
    {
        $client = new HttpClient([
            'baseUrl' => 'http://localhost:8000',
            'path' => '/api/v1/umicp'
        ]);

        $this->assertNotNull($client);

        $reflection = new \ReflectionClass($client);
        $property = $reflection->getProperty('path');
        $property->setAccessible(true);
        $this->assertEquals('/api/v1/umicp', $property->getValue($client));
    }

    public function testClientStatsInitialized()
    {
        $client = new HttpClient();

        $stats = $client->getStats();
        $this->assertIsArray($stats);
        $this->assertArrayHasKey('requests', $stats);
        $this->assertEquals(0, $stats['requests']);
    }
}

