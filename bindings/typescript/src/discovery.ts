/**
 * UMICP v0.2.0 Tool Discovery
 * MCP-compatible automatic tool introspection
 */

/**
 * JSON Schema type for operation parameters
 */
export type JSONSchema = Record<string, any>;

/**
 * Operation schema compatible with MCP JSON Schema
 */
export interface OperationSchema {
  /** Operation name */
  name: string;

  /** JSON Schema for input parameters */
  inputSchema: JSONSchema;

  /** Human-readable operation title */
  title?: string;

  /** Operation description */
  description?: string;

  /** JSON Schema for output/response */
  outputSchema?: JSONSchema;

  /** Additional metadata annotations */
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

  /** Protocol version */
  protocol: string;

  /** List of supported features */
  features?: string[];

  /** Number of available operations */
  operationsCount?: number;

  /** MCP protocol compatibility flag */
  mcpCompatible?: boolean;

  /** Additional server metadata */
  metadata?: Record<string, any>;
}

/**
 * Interface for services that support tool discovery
 */
export interface DiscoverableService {
  /**
   * List all available operations with their schemas
   * @returns Array of operation schemas
   */
  listOperations(): OperationSchema[];

  /**
   * Get schema for a specific operation by name
   * @param name - Operation name
   * @returns Operation schema or null if not found
   */
  getSchema(name: string): OperationSchema | null;

  /**
   * Get server information and metadata
   * @returns Server information
   */
  getServerInfo(): ServerInfo;
}

/**
 * Builder for OperationSchema
 */
export class OperationSchemaBuilder {
  private schema: Partial<OperationSchema>;

  constructor(name: string, inputSchema: JSONSchema) {
    this.schema = { name, inputSchema };
  }

  withTitle(title: string): this {
    this.schema.title = title;
    return this;
  }

  withDescription(description: string): this {
    this.schema.description = description;
    return this;
  }

  withOutputSchema(outputSchema: JSONSchema): this {
    this.schema.outputSchema = outputSchema;
    return this;
  }

  withAnnotations(annotations: Record<string, any>): this {
    this.schema.annotations = annotations;
    return this;
  }

  build(): OperationSchema {
    if (!this.schema.name || !this.schema.inputSchema) {
      throw new Error('Operation name and inputSchema are required');
    }
    return this.schema as OperationSchema;
  }
}

/**
 * Builder for ServerInfo
 */
export class ServerInfoBuilder {
  private info: Partial<ServerInfo>;

  constructor(server: string, version: string, protocol: string) {
    this.info = { server, version, protocol };
  }

  withFeatures(features: string[]): this {
    this.info.features = features;
    return this;
  }

  withOperationsCount(count: number): this {
    this.info.operationsCount = count;
    return this;
  }

  withMcpCompatible(compatible: boolean): this {
    this.info.mcpCompatible = compatible;
    return this;
  }

  withMetadata(metadata: Record<string, any>): this {
    this.info.metadata = metadata;
    return this;
  }

  build(): ServerInfo {
    if (!this.info.server || !this.info.version || !this.info.protocol) {
      throw new Error('Server, version, and protocol are required');
    }
    return this.info as ServerInfo;
  }
}

/**
 * Helper functions for generating discovery responses
 */
export class DiscoveryHelpers {
  /**
   * Generate JSON response for _list_operations
   */
  static generateOperationsResponse(service: DiscoverableService): Record<string, any> {
    const operations = service.listOperations();
    const info = service.getServerInfo();

    return {
      operations,
      count: operations.length,
      protocol: info.protocol,
      mcp_compatible: info.mcpCompatible ?? false
    };
  }

  /**
   * Generate JSON response for _get_schema
   */
  static generateSchemaResponse(
    service: DiscoverableService,
    operationName: string
  ): Record<string, any> {
    const schema = service.getSchema(operationName);

    if (schema) {
      const response: Record<string, any> = {
        name: schema.name,
        input_schema: schema.inputSchema
      };

      if (schema.title) response.title = schema.title;
      if (schema.description) response.description = schema.description;
      if (schema.outputSchema) response.output_schema = schema.outputSchema;
      if (schema.annotations) response.annotations = schema.annotations;

      return response;
    }

    return {
      error: 'Operation not found',
      operation: operationName
    };
  }

  /**
   * Generate JSON response for _server_info
   */
  static generateServerInfoResponse(service: DiscoverableService): ServerInfo {
    return service.getServerInfo();
  }
}

/**
 * Simple implementation of DiscoverableService
 */
export class SimpleDiscoverableService implements DiscoverableService {
  constructor(
    private operations: OperationSchema[],
    private serverInfo: ServerInfo
  ) {}

  listOperations(): OperationSchema[] {
    return this.operations;
  }

  getSchema(name: string): OperationSchema | null {
    return this.operations.find(op => op.name === name) ?? null;
  }

  getServerInfo(): ServerInfo {
    return {
      ...this.serverInfo,
      operationsCount: this.operations.length
    };
  }
}

