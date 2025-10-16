<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Discovery;

use PHPUnit\Framework\TestCase;
use UMICP\Discovery\OperationSchema;
use UMICP\Discovery\ServerInfo;
use UMICP\Discovery\DiscoverableService;
use UMICP\Discovery\SimpleDiscoverableService;
use UMICP\Discovery\DiscoveryHelpers;

/**
 * Tests for Tool Discovery functionality
 */
class ToolDiscoveryTest extends TestCase
{
    private array $operations;
    private ServerInfo $serverInfo;
    private SimpleDiscoverableService $service;

    protected function setUp(): void
    {
        $addInputSchema = [
            'type' => 'object',
            'properties' => [
                'a' => ['type' => 'number'],
                'b' => ['type' => 'number'],
            ],
        ];

        $addOp = new OperationSchema(
            name: 'add',
            inputSchema: $addInputSchema,
            title: 'Add Numbers',
            description: 'Adds two numbers together'
        );

        $multiplyOp = new OperationSchema(
            name: 'multiply',
            inputSchema: ['type' => 'object']
        );

        $this->operations = [$addOp, $multiplyOp];

        $this->serverInfo = new ServerInfo(
            server: 'math-service',
            version: '1.0.0',
            protocol: 'UMICP/1.0',
            mcpCompatible: true
        );

        $this->service = new SimpleDiscoverableService($this->operations, $this->serverInfo);
    }

    public function testOperationSchemaBasic(): void
    {
        $schema = new OperationSchema(
            name: 'test_op',
            inputSchema: ['type' => 'string']
        );

        $this->assertEquals('test_op', $schema->name);
        $this->assertNotNull($schema->inputSchema);
        $this->assertNull($schema->title);
        $this->assertNull($schema->description);
    }

    public function testOperationSchemaComplete(): void
    {
        $schema = new OperationSchema(
            name: 'complex_op',
            inputSchema: ['type' => 'object'],
            title: 'Complex Operation',
            description: 'A complex test operation',
            outputSchema: ['type' => 'boolean'],
            annotations: ['version' => '1.0', 'deprecated' => false]
        );

        $this->assertEquals('complex_op', $schema->name);
        $this->assertEquals('Complex Operation', $schema->title);
        $this->assertEquals('A complex test operation', $schema->description);
        $this->assertNotNull($schema->outputSchema);
        $this->assertNotNull($schema->annotations);
    }

    public function testOperationSchemaInvalidName(): void
    {
        $this->expectException(\InvalidArgumentException::class);
        new OperationSchema(name: '', inputSchema: ['type' => 'object']);
    }

    public function testOperationSchemaToArray(): void
    {
        $schema = new OperationSchema(
            name: 'test',
            inputSchema: ['type' => 'object'],
            title: 'Test'
        );

        $array = $schema->toArray();

        $this->assertArrayHasKey('name', $array);
        $this->assertArrayHasKey('input_schema', $array);
        $this->assertArrayHasKey('title', $array);
        $this->assertEquals('test', $array['name']);
    }

    public function testServerInfoBasic(): void
    {
        $info = new ServerInfo(
            server: 'test-server',
            version: '1.0.0',
            protocol: 'UMICP/1.0'
        );

        $this->assertEquals('test-server', $info->server);
        $this->assertEquals('1.0.0', $info->version);
        $this->assertEquals('UMICP/1.0', $info->protocol);
        $this->assertNull($info->features);
    }

    public function testServerInfoComplete(): void
    {
        $info = new ServerInfo(
            server: 'full-server',
            version: '2.0.0',
            protocol: 'UMICP/2.0',
            features: ['discovery', 'streaming', 'compression'],
            operationsCount: 42,
            mcpCompatible: true,
            metadata: ['region' => 'us-west-2', 'tier' => 'premium']
        );

        $this->assertEquals('full-server', $info->server);
        $this->assertEquals('2.0.0', $info->version);
        $this->assertEquals('UMICP/2.0', $info->protocol);
        $this->assertCount(3, $info->features);
        $this->assertEquals(42, $info->operationsCount);
        $this->assertTrue($info->mcpCompatible);
        $this->assertEquals('us-west-2', $info->metadata['region']);
    }

    public function testServerInfoInvalidServer(): void
    {
        $this->expectException(\InvalidArgumentException::class);
        new ServerInfo(server: '', version: '1.0.0', protocol: 'UMICP/1.0');
    }

    public function testServerInfoInvalidVersion(): void
    {
        $this->expectException(\InvalidArgumentException::class);
        new ServerInfo(server: 'test', version: '', protocol: 'UMICP/1.0');
    }

    public function testServerInfoInvalidProtocol(): void
    {
        $this->expectException(\InvalidArgumentException::class);
        new ServerInfo(server: 'test', version: '1.0.0', protocol: '');
    }

    public function testServerInfoToArray(): void
    {
        $info = new ServerInfo(
            server: 'test',
            version: '1.0.0',
            protocol: 'UMICP/1.0',
            mcpCompatible: true
        );

        $array = $info->toArray();

        $this->assertArrayHasKey('server', $array);
        $this->assertArrayHasKey('version', $array);
        $this->assertArrayHasKey('protocol', $array);
        $this->assertArrayHasKey('mcp_compatible', $array);
    }

    public function testSimpleDiscoverableServiceListOperations(): void
    {
        $ops = $this->service->listOperations();

        $this->assertCount(2, $ops);
        $this->assertEquals('add', $ops[0]->name);
        $this->assertEquals('multiply', $ops[1]->name);
    }

    public function testSimpleDiscoverableServiceGetSchema(): void
    {
        $schema = $this->service->getSchema('add');

        $this->assertNotNull($schema);
        $this->assertEquals('add', $schema->name);
        $this->assertEquals('Add Numbers', $schema->title);
    }

    public function testSimpleDiscoverableServiceGetNonExistentSchema(): void
    {
        $schema = $this->service->getSchema('nonexistent');

        $this->assertNull($schema);
    }

    public function testSimpleDiscoverableServiceGetServerInfo(): void
    {
        $info = $this->service->getServerInfo();

        $this->assertEquals('math-service', $info->server);
        $this->assertEquals('1.0.0', $info->version);
        $this->assertEquals(2, $info->operationsCount);
        $this->assertTrue($info->mcpCompatible);
    }

    public function testDiscoveryHelpersGenerateOperationsResponse(): void
    {
        $response = DiscoveryHelpers::generateOperationsResponse($this->service);

        $this->assertArrayHasKey('operations', $response);
        $this->assertArrayHasKey('count', $response);
        $this->assertArrayHasKey('protocol', $response);
        $this->assertArrayHasKey('mcp_compatible', $response);
        $this->assertCount(2, $response['operations']);
        $this->assertEquals(2, $response['count']);
        $this->assertEquals('UMICP/1.0', $response['protocol']);
        $this->assertTrue($response['mcp_compatible']);
    }

    public function testDiscoveryHelpersGenerateSchemaResponseSuccess(): void
    {
        $response = DiscoveryHelpers::generateSchemaResponse($this->service, 'add');

        $this->assertArrayHasKey('name', $response);
        $this->assertArrayHasKey('input_schema', $response);
        $this->assertArrayHasKey('title', $response);
        $this->assertEquals('add', $response['name']);
        $this->assertEquals('Add Numbers', $response['title']);
        $this->assertArrayNotHasKey('error', $response);
    }

    public function testDiscoveryHelpersGenerateSchemaResponseError(): void
    {
        $response = DiscoveryHelpers::generateSchemaResponse($this->service, 'missing');

        $this->assertArrayHasKey('error', $response);
        $this->assertArrayHasKey('operation', $response);
        $this->assertEquals('Operation not found', $response['error']);
        $this->assertEquals('missing', $response['operation']);
    }

    public function testDiscoveryHelpersGenerateServerInfoResponse(): void
    {
        $response = DiscoveryHelpers::generateServerInfoResponse($this->service);

        $this->assertArrayHasKey('server', $response);
        $this->assertArrayHasKey('version', $response);
        $this->assertArrayHasKey('protocol', $response);
        $this->assertArrayHasKey('operations_count', $response);
        $this->assertEquals('math-service', $response['server']);
        $this->assertEquals(2, $response['operations_count']);
    }

    public function testEmptyOperations(): void
    {
        $emptyService = new SimpleDiscoverableService(
            operations: [],
            serverInfo: new ServerInfo(
                server: 'empty-server',
                version: '1.0.0',
                protocol: 'UMICP/1.0'
            )
        );

        $this->assertCount(0, $emptyService->listOperations());
        $this->assertEquals(0, $emptyService->getServerInfo()->operationsCount);
    }

    public function testOperationSchemaWithAllFields(): void
    {
        $richSchema = new OperationSchema(
            name: 'rich_op',
            inputSchema: ['type' => 'object'],
            title: 'Rich Operation',
            description: 'A fully documented operation',
            outputSchema: ['type' => 'string'],
            annotations: ['version' => '2.0']
        );

        $richService = new SimpleDiscoverableService([$richSchema], $this->serverInfo);
        $response = DiscoveryHelpers::generateSchemaResponse($richService, 'rich_op');

        $this->assertEquals('Rich Operation', $response['title']);
        $this->assertEquals('A fully documented operation', $response['description']);
        $this->assertArrayHasKey('output_schema', $response);
        $this->assertArrayHasKey('annotations', $response);
    }
}
