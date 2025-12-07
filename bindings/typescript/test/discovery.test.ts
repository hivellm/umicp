import { describe, it, expect, beforeEach } from 'vitest';
import {
  OperationSchema,
  ServerInfo,
  DiscoverableService,
  OperationSchemaBuilder,
  ServerInfoBuilder,
  DiscoveryHelpers,
  SimpleDiscoverableService,
} from '../src/discovery.js';

describe('OperationSchemaBuilder', () => {
  it('should build a basic operation schema', () => {
    const schema = new OperationSchemaBuilder('test_op', {
      type: 'object',
      properties: { param1: { type: 'string' } }
    }).build();

    expect(schema.name).toBe('test_op');
    expect(schema.inputSchema).toEqual({
      type: 'object',
      properties: { param1: { type: 'string' } }
    });
  });

  it('should build a complete operation schema with all fields', () => {
    const schema = new OperationSchemaBuilder('complex_op', {
      type: 'object',
      properties: { x: { type: 'number' } }
    })
      .withTitle('Complex Operation')
      .withDescription('A complex test operation')
      .withOutputSchema({ type: 'object', properties: { result: { type: 'boolean' } } })
      .withAnnotations({ version: '1.0', deprecated: false })
      .build();

    expect(schema.name).toBe('complex_op');
    expect(schema.title).toBe('Complex Operation');
    expect(schema.description).toBe('A complex test operation');
    expect(schema.outputSchema).toEqual({
      type: 'object',
      properties: { result: { type: 'boolean' } }
    });
    expect(schema.annotations).toEqual({ version: '1.0', deprecated: false });
  });

  it('should throw error when building without required fields', () => {
    const builder = new OperationSchemaBuilder('', {});
    expect(() => (builder as any).build()).toThrow();
  });
});

describe('ServerInfoBuilder', () => {
  it('should build basic server info', () => {
    const info = new ServerInfoBuilder('test-server', '1.0.0', 'UMICP/1.0').build();

    expect(info.server).toBe('test-server');
    expect(info.version).toBe('1.0.0');
    expect(info.protocol).toBe('UMICP/1.0');
  });

  it('should build complete server info with all fields', () => {
    const info = new ServerInfoBuilder('full-server', '2.0.0', 'UMICP/2.0')
      .withFeatures(['discovery', 'streaming', 'compression'])
      .withOperationsCount(42)
      .withMcpCompatible(true)
      .withMetadata({ region: 'us-west-2', tier: 'premium' })
      .build();

    expect(info.server).toBe('full-server');
    expect(info.version).toBe('2.0.0');
    expect(info.protocol).toBe('UMICP/2.0');
    expect(info.features).toEqual(['discovery', 'streaming', 'compression']);
    expect(info.operationsCount).toBe(42);
    expect(info.mcpCompatible).toBe(true);
    expect(info.metadata).toEqual({ region: 'us-west-2', tier: 'premium' });
  });

  it('should throw error when building without required fields', () => {
    const builder = new ServerInfoBuilder('', '', '');
    expect(() => (builder as any).build()).toThrow();
  });
});

describe('SimpleDiscoverableService', () => {
  const operations: OperationSchema[] = [
    {
      name: 'add',
      inputSchema: {
        type: 'object',
        properties: {
          a: { type: 'number' },
          b: { type: 'number' }
        }
      },
      title: 'Add Numbers',
      description: 'Adds two numbers together'
    },
    {
      name: 'multiply',
      inputSchema: {
        type: 'object',
        properties: {
          x: { type: 'number' },
          y: { type: 'number' }
        }
      },
      title: 'Multiply Numbers'
    }
  ];

  const serverInfo: ServerInfo = {
    server: 'math-service',
    version: '1.0.0',
    protocol: 'UMICP/1.0',
    mcpCompatible: true
  };

  let service: SimpleDiscoverableService;

  beforeEach(() => {
    service = new SimpleDiscoverableService(operations, serverInfo);
  });

  it('should list all operations', () => {
    const ops = service.listOperations();
    expect(ops).toHaveLength(2);
    expect(ops[0].name).toBe('add');
    expect(ops[1].name).toBe('multiply');
  });

  it('should get schema by name', () => {
    const schema = service.getSchema('add');
    expect(schema).not.toBeNull();
    expect(schema?.name).toBe('add');
    expect(schema?.title).toBe('Add Numbers');
  });

  it('should return null for non-existent operation', () => {
    const schema = service.getSchema('nonexistent');
    expect(schema).toBeNull();
  });

  it('should return server info with operations count', () => {
    const info = service.getServerInfo();
    expect(info.server).toBe('math-service');
    expect(info.version).toBe('1.0.0');
    expect(info.operationsCount).toBe(2);
    expect(info.mcpCompatible).toBe(true);
  });
});

describe('DiscoveryHelpers', () => {
  const operations: OperationSchema[] = [
    {
      name: 'test_op',
      inputSchema: { type: 'object' },
      title: 'Test Operation'
    }
  ];

  const serverInfo: ServerInfo = {
    server: 'test-server',
    version: '1.0.0',
    protocol: 'UMICP/1.0',
    mcpCompatible: true
  };

  let service: DiscoverableService;

  beforeEach(() => {
    service = new SimpleDiscoverableService(operations, serverInfo);
  });

  it('should generate operations response', () => {
    const response = DiscoveryHelpers.generateOperationsResponse(service);

    expect(response.operations).toHaveLength(1);
    expect(response.count).toBe(1);
    expect(response.protocol).toBe('UMICP/1.0');
    expect(response.mcp_compatible).toBe(true);
  });

  it('should generate schema response for existing operation', () => {
    const response = DiscoveryHelpers.generateSchemaResponse(service, 'test_op');

    expect(response.name).toBe('test_op');
    expect(response.input_schema).toEqual({ type: 'object' });
    expect(response.title).toBe('Test Operation');
    expect(response.error).toBeUndefined();
  });

  it('should generate error response for non-existent operation', () => {
    const response = DiscoveryHelpers.generateSchemaResponse(service, 'missing');

    expect(response.error).toBe('Operation not found');
    expect(response.operation).toBe('missing');
  });

  it('should generate server info response', () => {
    const response = DiscoveryHelpers.generateServerInfoResponse(service);

    expect(response.server).toBe('test-server');
    expect(response.version).toBe('1.0.0');
    expect(response.protocol).toBe('UMICP/1.0');
    expect(response.operationsCount).toBe(1);
  });

  it('should include optional fields in schema response', () => {
    const richSchema: OperationSchema = {
      name: 'rich_op',
      inputSchema: { type: 'object' },
      title: 'Rich Operation',
      description: 'A fully documented operation',
      outputSchema: { type: 'string' },
      annotations: { version: '2.0' }
    };

    const richService = new SimpleDiscoverableService([richSchema], serverInfo);
    const response = DiscoveryHelpers.generateSchemaResponse(richService, 'rich_op');

    expect(response.title).toBe('Rich Operation');
    expect(response.description).toBe('A fully documented operation');
    expect(response.output_schema).toEqual({ type: 'string' });
    expect(response.annotations).toEqual({ version: '2.0' });
  });
});

describe('DiscoverableService Interface', () => {
  class CustomDiscoverableService implements DiscoverableService {
    private ops: OperationSchema[] = [
      {
        name: 'custom_op',
        inputSchema: { type: 'string' }
      }
    ];

    listOperations(): OperationSchema[] {
      return this.ops;
    }

    getSchema(name: string): OperationSchema | null {
      return this.ops.find(op => op.name === name) ?? null;
    }

    getServerInfo(): ServerInfo {
      return {
        server: 'custom',
        version: '1.0.0',
        protocol: 'UMICP/1.0'
      };
    }
  }

  it('should allow custom implementations of DiscoverableService', () => {
    const service = new CustomDiscoverableService();
    
    expect(service.listOperations()).toHaveLength(1);
    expect(service.getSchema('custom_op')).not.toBeNull();
    expect(service.getServerInfo().server).toBe('custom');
  });
});

