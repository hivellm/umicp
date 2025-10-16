<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Discovery;

use PHPUnit\Framework\TestCase;
use UMICP\Discovery\OperationSchema;
use UMICP\Discovery\ServerInfo;
use UMICP\Discovery\DiscoverableService;
use UMICP\Discovery\OperationSchemaBuilder;
use UMICP\Discovery\ServerInfoBuilder;
use UMICP\Discovery\DiscoveryHelpers;
use UMICP\Discovery\SimpleDiscoverableService;

class TestService implements DiscoverableService
{
    public function listOperations(): array
    {
        return [
            new OperationSchema(
                'search_vectors',
                [
                    'type' => 'object',
                    'properties' => [
                        'collection' => ['type' => 'string'],
                        'query' => ['type' => 'string'],
                        'limit' => ['type' => 'integer', 'default' => 10],
                    ],
                    'required' => ['collection', 'query'],
                ],
                'Search Vectors',
                'Search for semantically similar content',
                null,
                ['read_only' => true]
            ),
            new OperationSchema(
                'create_collection',
                [
                    'type' => 'object',
                    'properties' => [
                        'name' => ['type' => 'string'],
                        'dimension' => ['type' => 'integer'],
                    ],
                    'required' => ['name', 'dimension'],
                ],
                'Create Collection'
            ),
        ];
    }

    public function getSchema(string $name): ?OperationSchema
    {
        foreach ($this->listOperations() as $operation) {
            if ($operation->name === $name) {
                return $operation;
            }
        }
        return null;
    }

    public function getServerInfo(): ServerInfo
    {
        return new ServerInfo(
            'test-service',
            '1.0.0',
            'UMICP/0.2',
            ['discovery', 'search'],
            2,
            true
        );
    }
}

class ToolDiscoveryTest extends TestCase
{
    public function testOperationSchemaCreation(): void
    {
        $schema = new OperationSchema(
            'test_op',
            ['type' => 'object']
        );

        $this->assertEquals('test_op', $schema->name);
        $this->assertEquals(['type' => 'object'], $schema->input_schema);
    }

    public function testOperationSchemaWithAllFields(): void
    {
        $schema = new OperationSchema(
            'test',
            ['type' => 'object'],
            'Test Operation',
            'A test',
            ['type' => 'string'],
            ['read_only' => true]
        );

        $this->assertEquals('Test Operation', $schema->title);
        $this->assertEquals('A test', $schema->description);
        $this->assertNotNull($schema->output_schema);
        $this->assertNotNull($schema->annotations);
        $this->assertTrue($schema->annotations['read_only']);
    }

    public function testOperationSchemaToArray(): void
    {
        $schema = new OperationSchema(
            'test',
            ['type' => 'object'],
            'Test',
            'Description'
        );

        $array = $schema->toArray();

        $this->assertArrayHasKey('name', $array);
        $this->assertArrayHasKey('title', $array);
        $this->assertArrayHasKey('description', $array);
        $this->assertArrayHasKey('input_schema', $array);
    }

    public function testServerInfoCreation(): void
    {
        $info = new ServerInfo('my-service', '1.0.0', 'UMICP/0.2');

        $this->assertEquals('my-service', $info->server);
        $this->assertEquals('1.0.0', $info->version);
        $this->assertEquals('UMICP/0.2', $info->protocol);
    }

    public function testServerInfoWithAllFields(): void
    {
        $info = new ServerInfo(
            'test',
            '1.0',
            'UMICP/0.2',
            ['discovery'],
            5,
            true,
            ['region' => 'us-east']
        );

        $this->assertContains('discovery', $info->features);
        $this->assertEquals(5, $info->operations_count);
        $this->assertTrue($info->mcp_compatible);
        $this->assertEquals('us-east', $info->metadata['region']);
    }

    public function testServerInfoToArray(): void
    {
        $info = new ServerInfo(
            'test',
            '1.0',
            'UMICP/0.2',
            ['discovery'],
            5,
            true
        );

        $array = $info->toArray();

        $this->assertArrayHasKey('server', $array);
        $this->assertArrayHasKey('version', $array);
        $this->assertArrayHasKey('features', $array);
        $this->assertArrayHasKey('operations_count', $array);
        $this->assertArrayHasKey('mcp_compatible', $array);
    }

    public function testOperationSchemaBuilder(): void
    {
        $schema = (new OperationSchemaBuilder('test', ['type' => 'object']))
            ->withTitle('Test Operation')
            ->withDescription('A test')
            ->withAnnotations(['read_only' => true])
            ->build();

        $this->assertEquals('test', $schema->name);
        $this->assertEquals('Test Operation', $schema->title);
        $this->assertEquals('A test', $schema->description);
        $this->assertTrue($schema->annotations['read_only']);
    }

    public function testServerInfoBuilder(): void
    {
        $info = (new ServerInfoBuilder('service', '1.0', 'UMICP/0.2'))
            ->withFeatures(['discovery', 'search'])
            ->withOperationsCount(10)
            ->withMcpCompatible(true)
            ->withMetadata(['region' => 'us-east'])
            ->build();

        $this->assertEquals('service', $info->server);
        $this->assertCount(2, $info->features);
        $this->assertEquals(10, $info->operations_count);
        $this->assertTrue($info->mcp_compatible);
        $this->assertEquals('us-east', $info->metadata['region']);
    }

    public function testDiscoverableServiceListOperations(): void
    {
        $service = new TestService();
        $operations = $service->listOperations();

        $this->assertCount(2, $operations);
        $this->assertEquals('search_vectors', $operations[0]->name);
        $this->assertEquals('create_collection', $operations[1]->name);
    }

    public function testDiscoverableServiceGetSchema(): void
    {
        $service = new TestService();
        $schema = $service->getSchema('search_vectors');

        $this->assertNotNull($schema);
        $this->assertEquals('search_vectors', $schema->name);
        $this->assertEquals('Search Vectors', $schema->title);
    }

    public function testDiscoverableServiceGetSchemaNotFound(): void
    {
        $service = new TestService();
        $schema = $service->getSchema('non_existent');

        $this->assertNull($schema);
    }

    public function testDiscoverableServiceGetServerInfo(): void
    {
        $service = new TestService();
        $info = $service->getServerInfo();

        $this->assertEquals('test-service', $info->server);
        $this->assertEquals('1.0.0', $info->version);
        $this->assertEquals('UMICP/0.2', $info->protocol);
        $this->assertContains('discovery', $info->features);
    }

    public function testGenerateOperationsResponse(): void
    {
        $service = new TestService();
        $response = DiscoveryHelpers::generateOperationsResponse($service);

        $this->assertArrayHasKey('operations', $response);
        $this->assertArrayHasKey('count', $response);
        $this->assertEquals(2, $response['count']);
        $this->assertEquals('UMICP/0.2', $response['protocol']);
        $this->assertTrue($response['mcp_compatible']);
    }

    public function testGenerateSchemaResponseFound(): void
    {
        $service = new TestService();
        $response = DiscoveryHelpers::generateSchemaResponse($service, 'search_vectors');

        $this->assertEquals('search_vectors', $response['name']);
        $this->assertEquals('Search Vectors', $response['title']);
        $this->assertArrayNotHasKey('error', $response);
    }

    public function testGenerateSchemaResponseNotFound(): void
    {
        $service = new TestService();
        $response = DiscoveryHelpers::generateSchemaResponse($service, 'invalid');

        $this->assertEquals('Operation not found', $response['error']);
        $this->assertEquals('invalid', $response['operation']);
    }

    public function testGenerateServerInfoResponse(): void
    {
        $service = new TestService();
        $info = DiscoveryHelpers::generateServerInfoResponse($service);

        $this->assertEquals('test-service', $info['server']);
        $this->assertEquals('1.0.0', $info['version']);
        $this->assertTrue($info['mcp_compatible']);
    }

    public function testSimpleDiscoverableService(): void
    {
        $operations = [
            new OperationSchema('test_op', ['type' => 'object']),
        ];

        $serverInfo = new ServerInfo('test', '1.0', 'UMICP/0.2');

        $service = new SimpleDiscoverableService($operations, $serverInfo);

        $this->assertCount(1, $service->listOperations());
        $this->assertNotNull($service->getSchema('test_op'));
        $this->assertEquals(1, $service->getServerInfo()->operations_count);
    }

    public function testNativeTypesInCapabilities(): void
    {
        // PHP supports mixed types natively
        $capabilities = [
            'max_tokens' => 100,
            'temperature' => 0.7,
            'enabled' => true,
            'models' => ['gpt-4', 'claude-3'],
            'config' => ['timeout' => 30],
            'optional' => null,
        ];

        $this->assertIsInt($capabilities['max_tokens']);
        $this->assertIsFloat($capabilities['temperature']);
        $this->assertIsBool($capabilities['enabled']);
        $this->assertIsArray($capabilities['models']);
        $this->assertIsArray($capabilities['config']);
        $this->assertNull($capabilities['optional']);
    }
}

