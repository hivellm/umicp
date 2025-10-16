/**
 * UMICP Tool Discovery (v0.2.0)
 * MCP-compatible automatic tool introspection with JSON Schema
 */

/**
 * JSON Schema for operation input/output
 */
export type JSONSchema = Record<string, any>;

/**
 * Operation schema compatible with MCP JSON Schema
 */
export interface OperationSchema {
  /** Operation name */
  name: string;

  /** Display title */
  title?: string;

  /** Description of what the operation does */
  description?: string;

  /** JSON Schema for input parameters */
  input_schema: JSONSchema;

  /** Optional output schema */
  output_schema?: JSONSchema;

  /** Annotations (read_only, idempotent, destructive, etc.) */
  annotations?: Record<string, any>;
}

/**
 * Server information for discovery
 */
export interface ServerInfo {
  /** Server name/identifier */
  server: string;

  /** Server version */
  version: string;

  /** Protocol version (e.g., "UMICP/0.2") */
  protocol: string;

  /** List of supported features */
  features?: string[];

  /** Number of available operations */
  operations_count?: number;

  /** Whether server is MCP-compatible */
  mcp_compatible?: boolean;

  /** Additional metadata */
  metadata?: Record<string, any>;
}

/**
 * Interface for services that support tool discovery
 *
 * @example
 * ```typescript
 * class MyService implements DiscoverableService {
 *   listOperations(): OperationSchema[] {
 *     return [{
 *       name: 'search',
 *       title: 'Search',
 *       description: 'Search for content',
 *       input_schema: {
 *         type: 'object',
 *         properties: {
 *           query: { type: 'string' }
 *         },
 *         required: ['query']
 *       },
 *       annotations: { read_only: true }
 *     }];
 *   }
 *
 *   getSchema(name: string): OperationSchema | null {
 *     return this.listOperations().find(op => op.name === name) || null;
 *   }
 *
 *   getServerInfo(): ServerInfo {
 *     return {
 *       server: 'my-service',
 *       version: '1.0.0',
 *       protocol: 'UMICP/0.2',
 *       mcp_compatible: true
 *     };
 *   }
 * }
 * ```
 */
export interface DiscoverableService {
  /**
   * List all available operations with their schemas
   */
  listOperations(): OperationSchema[];

  /**
   * Get schema for a specific operation by name
   */
  getSchema(name: string): OperationSchema | null;

  /**
   * Get server information and metadata
   */
  getServerInfo(): ServerInfo;
}

/**
 * Helper to create an operation schema with builder pattern
 */
export class OperationSchemaBuilder {
  private schema: OperationSchema;

  constructor(name: string, inputSchema: JSONSchema) {
    this.schema = {
      name,
      input_schema: inputSchema
    };
  }

  withTitle(title: string): this {
    this.schema.title = title;
    return this;
  }

  withDescription(description: string): this {
    this.schema.description = description;
    return this;
  }

  withOutputSchema(schema: JSONSchema): this {
    this.schema.output_schema = schema;
    return this;
  }

  withAnnotations(annotations: Record<string, any>): this {
    this.schema.annotations = annotations;
    return this;
  }

  build(): OperationSchema {
    return this.schema;
  }
}

/**
 * Helper to create server info with builder pattern
 */
export class ServerInfoBuilder {
  private info: ServerInfo;

  constructor(server: string, version: string, protocol: string) {
    this.info = { server, version, protocol };
  }

  withFeatures(features: string[]): this {
    this.info.features = features;
    return this;
  }

  withOperationsCount(count: number): this {
    this.info.operations_count = count;
    return this;
  }

  withMcpCompatible(compatible: boolean): this {
    this.info.mcp_compatible = compatible;
    return this;
  }

  withMetadata(metadata: Record<string, any>): this {
    this.info.metadata = metadata;
    return this;
  }

  build(): ServerInfo {
    return this.info;
  }
}

/**
 * Generate JSON response for _list_operations
 */
export function generateOperationsResponse(service: DiscoverableService): Record<string, any> {
  const operations = service.listOperations();
  const info = service.getServerInfo();

  return {
    operations,
    count: operations.length,
    protocol: info.protocol,
    mcp_compatible: info.mcp_compatible ?? false
  };
}

/**
 * Generate JSON response for _get_schema
 */
export function generateSchemaResponse(service: DiscoverableService, operationName: string): Record<string, any> {
  const schema = service.getSchema(operationName);

  if (schema) {
    return schema;
  } else {
    return {
      error: 'Operation not found',
      operation: operationName
    };
  }
}

/**
 * Generate JSON response for _server_info
 */
export function generateServerInfoResponse(service: DiscoverableService): ServerInfo {
  return service.getServerInfo();
}

/**
 * Create a simple discoverable service from operations
 */
export function createDiscoverableService(
  operations: OperationSchema[],
  serverInfo: ServerInfo
): DiscoverableService {
  return {
    listOperations: () => operations,
    getSchema: (name: string) => operations.find(op => op.name === name) || null,
    getServerInfo: () => ({
      ...serverInfo,
      operations_count: operations.length
    })
  };
}

