/**
 * Tool Discovery Tests (v0.2.0)
 */

import { describe, it, expect } from 'vitest';
import {
  OperationSchema,
  ServerInfo,
  DiscoverableService,
  OperationSchemaBuilder,
  ServerInfoBuilder,
  generateOperationsResponse,
  generateSchemaResponse,
  generateServerInfoResponse,
  createDiscoverableService,
} from '../src/tool-discovery';

// Test service implementation
class TestService implements DiscoverableService {
  listOperations(): OperationSchema[] {
    return [
      {
        name: 'search_vectors',
        title: 'Search Vectors',
        description: 'Search for semantically similar content',
        input_schema: {
          type: 'object',
          properties: {
            collection: { type: 'string' },
            query: { type: 'string' },
            limit: { type: 'integer', default: 10 }
          },
          required: ['collection', 'query']
        },
        annotations: { read_only: true }
      },
      {
        name: 'create_collection',
        title: 'Create Collection',
        input_schema: {
          type: 'object',
          properties: {
            name: { type: 'string' },
            dimension: { type: 'integer' }
          },
          required: ['name', 'dimension']
        }
      }
    ];
  }

  getSchema(name: string): OperationSchema | null {
    return this.listOperations().find(op => op.name === name) || null;
  }

  getServerInfo(): ServerInfo {
    return {
      server: 'test-service',
      version: '1.0.0',
      protocol: 'UMICP/0.2',
      features: ['discovery', 'search'],
      operations_count: 2,
      mcp_compatible: true
    };
  }
}

describe('OperationSchema', () => {
  it('should create operation schema with required fields', () => {
    const schema: OperationSchema = {
      name: 'test_op',
      input_schema: { type: 'object' }
    };

    expect(schema.name).toBe('test_op');
    expect(schema.input_schema.type).toBe('object');
  });

  it('should support all optional fields', () => {
    const schema: OperationSchema = {
      name: 'test',
      title: 'Test Operation',
      description: 'A test',
      input_schema: { type: 'object' },
      output_schema: { type: 'string' },
      annotations: { read_only: true }
    };

    expect(schema.title).toBe('Test Operation');
    expect(schema.description).toBe('A test');
    expect(schema.output_schema).toBeDefined();
    expect(schema.annotations?.read_only).toBe(true);
  });
});

describe('ServerInfo', () => {
  it('should create server info with required fields', () => {
    const info: ServerInfo = {
      server: 'my-service',
      version: '1.0.0',
      protocol: 'UMICP/0.2'
    };

    expect(info.server).toBe('my-service');
    expect(info.version).toBe('1.0.0');
    expect(info.protocol).toBe('UMICP/0.2');
  });

  it('should support all optional fields', () => {
    const info: ServerInfo = {
      server: 'test',
      version: '1.0',
      protocol: 'UMICP/0.2',
      features: ['discovery'],
      operations_count: 5,
      mcp_compatible: true,
      metadata: { custom: 'value' }
    };

    expect(info.features).toContain('discovery');
    expect(info.operations_count).toBe(5);
    expect(info.mcp_compatible).toBe(true);
    expect(info.metadata?.custom).toBe('value');
  });
});

describe('DiscoverableService', () => {
  it('should list all operations', () => {
    const service = new TestService();
    const operations = service.listOperations();

    expect(operations).toHaveLength(2);
    expect(operations[0].name).toBe('search_vectors');
    expect(operations[1].name).toBe('create_collection');
  });

  it('should get schema by name', () => {
    const service = new TestService();
    const schema = service.getSchema('search_vectors');

    expect(schema).toBeDefined();
    expect(schema?.name).toBe('search_vectors');
    expect(schema?.title).toBe('Search Vectors');
  });

  it('should return null for non-existent operation', () => {
    const service = new TestService();
    const schema = service.getSchema('non_existent');

    expect(schema).toBeNull();
  });

  it('should return server info', () => {
    const service = new TestService();
    const info = service.getServerInfo();

    expect(info.server).toBe('test-service');
    expect(info.version).toBe('1.0.0');
    expect(info.protocol).toBe('UMICP/0.2');
    expect(info.features).toContain('discovery');
  });
});

describe('OperationSchemaBuilder', () => {
  it('should build basic schema', () => {
    const schema = new OperationSchemaBuilder('test', { type: 'object' })
      .build();

    expect(schema.name).toBe('test');
    expect(schema.input_schema.type).toBe('object');
  });

  it('should build schema with all fields', () => {
    const schema = new OperationSchemaBuilder('search', {
      type: 'object',
      properties: { query: { type: 'string' } }
    })
      .withTitle('Search')
      .withDescription('Search operation')
      .withOutputSchema({ type: 'array' })
      .withAnnotations({ read_only: true, idempotent: true })
      .build();

    expect(schema.name).toBe('search');
    expect(schema.title).toBe('Search');
    expect(schema.description).toBe('Search operation');
    expect(schema.output_schema).toBeDefined();
    expect(schema.annotations?.read_only).toBe(true);
  });
});

describe('ServerInfoBuilder', () => {
  it('should build basic server info', () => {
    const info = new ServerInfoBuilder('service', '1.0', 'UMICP/0.2')
      .build();

    expect(info.server).toBe('service');
    expect(info.version).toBe('1.0');
    expect(info.protocol).toBe('UMICP/0.2');
  });

  it('should build server info with all fields', () => {
    const info = new ServerInfoBuilder('service', '1.0', 'UMICP/0.2')
      .withFeatures(['discovery', 'search'])
      .withOperationsCount(10)
      .withMcpCompatible(true)
      .withMetadata({ region: 'us-east' })
      .build();

    expect(info.features).toContain('discovery');
    expect(info.operations_count).toBe(10);
    expect(info.mcp_compatible).toBe(true);
    expect(info.metadata?.region).toBe('us-east');
  });
});

describe('Discovery Response Generators', () => {
  it('should generate operations response', () => {
    const service = new TestService();
    const response = generateOperationsResponse(service);

    expect(response.operations).toHaveLength(2);
    expect(response.count).toBe(2);
    expect(response.protocol).toBe('UMICP/0.2');
    expect(response.mcp_compatible).toBe(true);
  });

  it('should generate schema response for found operation', () => {
    const service = new TestService();
    const response = generateSchemaResponse(service, 'search_vectors');

    expect(response.name).toBe('search_vectors');
    expect(response.title).toBe('Search Vectors');
    expect(response.error).toBeUndefined();
  });

  it('should generate error response for not found operation', () => {
    const service = new TestService();
    const response = generateSchemaResponse(service, 'invalid');

    expect(response.error).toBe('Operation not found');
    expect(response.operation).toBe('invalid');
  });

  it('should generate server info response', () => {
    const service = new TestService();
    const response = generateServerInfoResponse(service);

    expect(response.server).toBe('test-service');
    expect(response.version).toBe('1.0.0');
    expect(response.mcp_compatible).toBe(true);
  });
});

describe('createDiscoverableService', () => {
  it('should create service from operations and info', () => {
    const operations: OperationSchema[] = [
      {
        name: 'test_op',
        input_schema: { type: 'object' }
      }
    ];

    const info: ServerInfo = {
      server: 'test',
      version: '1.0',
      protocol: 'UMICP/0.2'
    };

    const service = createDiscoverableService(operations, info);

    expect(service.listOperations()).toHaveLength(1);
    expect(service.getSchema('test_op')).toBeDefined();
    expect(service.getServerInfo().operations_count).toBe(1);
  });

  it('should return null for non-existent schema', () => {
    const service = createDiscoverableService([], {
      server: 'test',
      version: '1.0',
      protocol: 'UMICP/0.2'
    });

    expect(service.getSchema('missing')).toBeNull();
  });
});

describe('Native Types in Capabilities', () => {
  it('should support integer values', () => {
    const capabilities: Record<string, any> = {
      max_tokens: 100,
      temperature: 0.7,
      enabled: true
    };

    expect(typeof capabilities.max_tokens).toBe('number');
    expect(typeof capabilities.temperature).toBe('number');
    expect(typeof capabilities.enabled).toBe('boolean');
  });

  it('should support array and object values', () => {
    const capabilities: Record<string, any> = {
      models: ['gpt-4', 'claude-3'],
      config: { timeout: 30 }
    };

    expect(Array.isArray(capabilities.models)).toBe(true);
    expect(typeof capabilities.config).toBe('object');
  });

  it('should support null values', () => {
    const capabilities: Record<string, any> = {
      optional_field: null
    };

    expect(capabilities.optional_field).toBeNull();
  });
});

