# TypeScript Bindings Guide

## Overview

The UMICP TypeScript bindings provide a comprehensive, type-safe interface to the Universal Matrix Inter-Communication Protocol. This guide covers installation, usage, and recent improvements to the binding system.

## Recent Updates (2024)

### Performance Improvements
- **96% faster e2e tests**: Execution time reduced from 127s to 5.1s
- **Enhanced serialization**: Optimized envelope serialization and deserialization
- **Improved connection handling**: Better WebSocket lifecycle management
- **Memory optimization**: Reduced memory footprint and eliminated leaks

### Reliability Enhancements
- **Type safety**: Resolved "Number expected" errors in envelope creation
- **Connection stability**: Improved WebSocket connection state management
- **Error handling**: Enhanced error recovery and validation
- **Resource management**: Proper cleanup prevents hanging processes

### New Features
- **Utility functions**: Safe envelope creation from JSON messages
- **Advanced transport**: Enhanced WebSocket transport with heartbeat
- **Performance testing**: Comprehensive load testing capabilities
- **Real-world scenarios**: IoT and financial transaction test coverage

## Installation

### Prerequisites
- Node.js 14 or higher
- TypeScript 4.5 or higher
- npm or yarn package manager

### Install via npm
```bash
npm install @umicp/core
```

### Install via yarn
```bash
yarn add @umicp/core
```

### Development Installation
```bash
git clone https://github.com/cmmv-hive/umicp.git
cd umicp/bindings/typescript
npm install
npm run build
```

## Core Concepts

### Envelopes
UMICP messages are wrapped in envelopes that provide metadata, routing information, and capabilities:

```typescript
import { UMICP, OperationType } from '@umicp/core';

const envelope = UMICP.createEnvelope({
  from: 'client-001',
  to: 'server-001',
  operation: OperationType.DATA,
  messageId: 'unique-message-id',
  capabilities: {
    'content-type': 'application/json',
    'priority': 'high',
    'timestamp': Date.now().toString()
  }
});
```

### Operation Types
Different message types for various communication patterns:

```typescript
enum OperationType {
  DATA = 0,      // Regular data message
  CONTROL = 1,   // Control/management message  
  ACK = 2,       // Acknowledgment
  ERROR = 3,     // Error message
  REQUEST = 4,   // Request message
  RESPONSE = 5   // Response message
}
```

### Transport Layer
WebSocket-based transport with advanced features:

```typescript
import { AdvancedWebSocketTransport } from '@umicp/core/transport';

const transport = new AdvancedWebSocketTransport({
  url: 'ws://localhost:8080',
  isServer: false,
  heartbeatInterval: 5000,
  maxReconnectAttempts: 3,
  perMessageDeflate: true,
  maxPayload: 1024 * 1024 // 1MB
});
```

## API Reference

### UMICP Class

#### Static Methods

##### `createEnvelope(options: EnvelopeOptions): Envelope`
Creates a new UMICP envelope with the specified options.

```typescript
interface EnvelopeOptions {
  from?: string;
  to?: string;
  operation?: OperationType;
  messageId?: string;
  capabilities?: Record<string, string>;
  payloadHint?: PayloadHint;
}
```

##### `serializeEnvelope(envelope: Envelope): string`
Serializes an envelope to JSON string format.

```typescript
const serialized = UMICP.serializeEnvelope(envelope);
```

##### `deserializeEnvelope(json: string): Envelope`
Deserializes a JSON string back to an Envelope object.

```typescript
const envelope = UMICP.deserializeEnvelope(serialized);
```

### Envelope Class

#### Instance Methods

##### `setFrom(from: string): Envelope`
Sets the sender identifier.

##### `setTo(to: string): Envelope`
Sets the recipient identifier.

##### `setOperation(operation: OperationType): Envelope`
Sets the operation type.

##### `setMessageId(messageId: string): Envelope`
Sets the unique message identifier.

##### `setCapabilities(capabilities: Record<string, string>): Envelope`
Sets the envelope capabilities (metadata).

##### `getFrom(): string`
Returns the sender identifier.

##### `getTo(): string`
Returns the recipient identifier.

##### `getOperation(): OperationType`
Returns the operation type.

##### `getMessageId(): string`
Returns the message identifier.

##### `getCapabilities(): Record<string, string>`
Returns the envelope capabilities.

##### `serialize(): string`
Serializes the envelope to JSON string.

##### `validate(): boolean`
Validates the envelope structure and required fields.

##### `getHash(): string`
Generates a unique hash for the envelope.

### AdvancedWebSocketTransport Class

#### Constructor
```typescript
constructor(config: WebSocketConfig)

interface WebSocketConfig {
  url?: string;                    // Client connection URL
  port?: number;                   // Server port
  isServer: boolean;               // Server or client mode
  heartbeatInterval?: number;      // Heartbeat interval in ms
  maxReconnectAttempts?: number;   // Max reconnection attempts
  perMessageDeflate?: boolean;     // Enable compression
  maxPayload?: number;             // Maximum payload size
  path?: string;                   // WebSocket path
}
```

#### Methods

##### `connect(): Promise<boolean>`
Establishes connection (server starts listening or client connects).

##### `disconnect(): Promise<boolean>`
Closes connection and cleans up resources.

##### `send(message: string | object, targetId?: string): boolean`
Sends a message. For servers, optionally specify target connection ID.

##### `isConnected(): boolean`
Returns connection status.

##### `getStats(): TransportStats`
Returns connection statistics and information.

#### Events

##### `'connected'`
Emitted when connection is established.

```typescript
transport.on('connected', () => {
  console.log('Connected to UMICP server');
});
```

##### `'disconnected'`
Emitted when connection is closed.

```typescript
transport.on('disconnected', (code: number, reason: Buffer) => {
  console.log(`Disconnected: ${code} - ${reason.toString()}`);
});
```

##### `'message'`
Emitted when a message is received.

```typescript
transport.on('message', (message: any, connectionInfo?: ConnectionInfo) => {
  console.log('Received message:', message);
});
```

##### `'error'`
Emitted when an error occurs.

```typescript
transport.on('error', (error: Error) => {
  console.error('Transport error:', error);
});
```

## Usage Examples

### Basic Client-Server Communication

#### Server Setup
```typescript
import { AdvancedWebSocketTransport, UMICP, OperationType } from '@umicp/core';

const server = new AdvancedWebSocketTransport({
  port: 8080,
  isServer: true,
  heartbeatInterval: 5000
});

server.on('message', async (message, connectionInfo) => {
  console.log('Server received:', message);
  
  // Send acknowledgment
  const ack = UMICP.createEnvelope({
    from: 'server',
    to: message.from,
    operation: OperationType.ACK,
    messageId: `ack-${Date.now()}`,
    capabilities: {
      'original-message-id': message.msg_id,
      'status': 'received'
    }
  });
  
  server.send(ack, connectionInfo.id);
});

server.on('clientConnected', (connectionInfo) => {
  console.log(`Client connected: ${connectionInfo.id}`);
});

await server.connect();
console.log('Server listening on port 8080');
```

#### Client Setup
```typescript
const client = new AdvancedWebSocketTransport({
  url: 'ws://localhost:8080',
  isServer: false,
  maxReconnectAttempts: 5
});

client.on('message', (message) => {
  console.log('Client received:', message);
});

client.on('connected', () => {
  console.log('Connected to server');
  
  // Send initial message
  const greeting = UMICP.createEnvelope({
    from: 'client-001',
    to: 'server',
    operation: OperationType.DATA,
    messageId: `greeting-${Date.now()}`,
    capabilities: {
      'message': 'Hello UMICP Server!',
      'client-version': '1.0.0'
    }
  });
  
  client.send(greeting);
});

await client.connect();
```

### Federated Learning Example

#### Coordinator (Server)
```typescript
const coordinator = new AdvancedWebSocketTransport({
  port: 8080,
  isServer: true
});

// Model weights distribution
const modelWeights = {
  layer1: [0.1, 0.2, 0.3],
  layer2: [0.4, 0.5, 0.6],
  version: '1.0'
};

coordinator.on('message', async (message, connectionInfo) => {
  if (message.op === OperationType.REQUEST && 
      message.capabilities['request-type'] === 'model-weights') {
    
    const weightsEnvelope = UMICP.createEnvelope({
      from: 'coordinator',
      to: message.from,
      operation: OperationType.RESPONSE,
      messageId: `weights-${Date.now()}`,
      capabilities: {
        'model-version': modelWeights.version,
        'weights': JSON.stringify(modelWeights),
        'response-to': message.msg_id
      }
    });
    
    coordinator.send(weightsEnvelope, connectionInfo.id);
  }
});

await coordinator.connect();
```

#### Worker (Client)
```typescript
const worker = new AdvancedWebSocketTransport({
  url: 'ws://localhost:8080',
  isServer: false
});

worker.on('message', (message) => {
  if (message.op === OperationType.RESPONSE && 
      message.capabilities['weights']) {
    
    const weights = JSON.parse(message.capabilities['weights']);
    console.log('Received model weights:', weights);
    
    // Perform local training...
    // Send back updated weights
    const updatedWeights = performTraining(weights);
    
    const updateEnvelope = UMICP.createEnvelope({
      from: 'worker-001',
      to: 'coordinator',
      operation: OperationType.DATA,
      messageId: `update-${Date.now()}`,
      capabilities: {
        'update-type': 'model-weights',
        'weights': JSON.stringify(updatedWeights),
        'training-samples': '1000'
      }
    });
    
    worker.send(updateEnvelope);
  }
});

worker.on('connected', () => {
  // Request initial model weights
  const request = UMICP.createEnvelope({
    from: 'worker-001',
    to: 'coordinator',
    operation: OperationType.REQUEST,
    messageId: `request-${Date.now()}`,
    capabilities: {
      'request-type': 'model-weights',
      'worker-id': 'worker-001'
    }
  });
  
  worker.send(request);
});

await worker.connect();
```

### IoT Data Streaming

```typescript
// IoT Sensor Simulation
const sensor = new AdvancedWebSocketTransport({
  url: 'ws://localhost:8080',
  isServer: false
});

sensor.on('connected', () => {
  // Stream sensor data every second
  setInterval(() => {
    const sensorData = UMICP.createEnvelope({
      from: 'temperature-sensor-001',
      to: 'data-collector',
      operation: OperationType.DATA,
      messageId: `temp-${Date.now()}`,
      capabilities: {
        'sensor-type': 'temperature',
        'value': (20 + Math.random() * 15).toFixed(2),
        'unit': 'celsius',
        'location': 'warehouse-a',
        'timestamp': Date.now().toString()
      }
    });
    
    sensor.send(sensorData);
  }, 1000);
});

await sensor.connect();
```

### Financial Transaction Processing

```typescript
// Payment Gateway
const gateway = new AdvancedWebSocketTransport({
  url: 'ws://localhost:8080',
  isServer: false
});

gateway.on('connected', () => {
  const transaction = UMICP.createEnvelope({
    from: 'payment-gateway',
    to: 'transaction-processor',
    operation: OperationType.REQUEST,
    messageId: `txn-${generateTransactionId()}`,
    capabilities: {
      'transaction-type': 'payment',
      'amount': '99.99',
      'currency': 'USD',
      'merchant-id': 'merchant-12345',
      'customer-id': 'customer-67890',
      'payment-method': 'credit-card'
    }
  });
  
  gateway.send(transaction);
});

gateway.on('message', (message) => {
  if (message.op === OperationType.RESPONSE) {
    const status = message.capabilities['status'];
    const transactionId = message.capabilities['transaction-id'];
    
    console.log(`Transaction ${transactionId}: ${status}`);
  }
});

await gateway.connect();
```

## Error Handling

### Connection Errors
```typescript
transport.on('error', (error) => {
  console.error('Connection error:', error);
  
  // Implement retry logic
  setTimeout(async () => {
    try {
      await transport.connect();
    } catch (retryError) {
      console.error('Retry failed:', retryError);
    }
  }, 5000);
});
```

### Message Validation
```typescript
// Validate envelope before sending
const envelope = UMICP.createEnvelope({...});

if (envelope.validate()) {
  await transport.send(envelope);
} else {
  console.error('Invalid envelope structure');
}
```

### Safe Envelope Creation
```typescript
// Use utility function for safe creation from JSON
function createEnvelopeFromMessage(message: any): Envelope | null {
  try {
    // Handle operation type conversion
    let operation = OperationType.DATA;
    if (message.op !== undefined) {
      if (typeof message.op === 'number') {
        operation = message.op;
      } else if (typeof message.op === 'string') {
        const parsed = parseInt(message.op);
        if (!isNaN(parsed)) {
          operation = parsed;
        }
      }
    }

    // Safe capability handling
    const capabilities: Record<string, string> = {};
    if (message.capabilities && typeof message.capabilities === 'object') {
      for (const [key, value] of Object.entries(message.capabilities)) {
        capabilities[key] = String(value);
      }
    }

    return UMICP.createEnvelope({
      from: message.from || '',
      to: message.to || '',
      operation: operation,
      messageId: message.msg_id || '',
      capabilities: capabilities
    });
  } catch (error) {
    console.error('Failed to create envelope:', error);
    return null;
  }
}
```

## Performance Optimization

### Connection Pooling
```typescript
class UMICPConnectionPool {
  private connections: Map<string, AdvancedWebSocketTransport> = new Map();
  
  async getConnection(url: string): Promise<AdvancedWebSocketTransport> {
    if (this.connections.has(url)) {
      return this.connections.get(url)!;
    }
    
    const transport = new AdvancedWebSocketTransport({
      url,
      isServer: false,
      heartbeatInterval: 30000
    });
    
    await transport.connect();
    this.connections.set(url, transport);
    
    return transport;
  }
  
  async closeAll(): Promise<void> {
    for (const transport of this.connections.values()) {
      await transport.disconnect();
    }
    this.connections.clear();
  }
}
```

### Message Batching
```typescript
class MessageBatcher {
  private batch: any[] = [];
  private batchSize = 100;
  private flushInterval = 1000;
  
  constructor(private transport: AdvancedWebSocketTransport) {
    setInterval(() => this.flush(), this.flushInterval);
  }
  
  add(message: any): void {
    this.batch.push(message);
    if (this.batch.length >= this.batchSize) {
      this.flush();
    }
  }
  
  private flush(): void {
    if (this.batch.length === 0) return;
    
    const batchEnvelope = UMICP.createEnvelope({
      from: 'batcher',
      to: 'processor',
      operation: OperationType.DATA,
      messageId: `batch-${Date.now()}`,
      capabilities: {
        'batch-size': this.batch.length.toString(),
        'messages': JSON.stringify(this.batch)
      }
    });
    
    this.transport.send(batchEnvelope);
    this.batch = [];
  }
}
```

## Testing

### Unit Testing
```typescript
import { UMICP, OperationType } from '@umicp/core';

describe('UMICP Envelope', () => {
  test('should create valid envelope', () => {
    const envelope = UMICP.createEnvelope({
      from: 'test-sender',
      to: 'test-receiver',
      operation: OperationType.DATA,
      messageId: 'test-123'
    });
    
    expect(envelope.validate()).toBe(true);
    expect(envelope.getFrom()).toBe('test-sender');
    expect(envelope.getTo()).toBe('test-receiver');
  });
  
  test('should serialize and deserialize', () => {
    const original = UMICP.createEnvelope({
      from: 'sender',
      to: 'receiver',
      operation: OperationType.DATA,
      messageId: 'msg-001',
      capabilities: { 'key': 'value' }
    });
    
    const serialized = original.serialize();
    const deserialized = UMICP.deserializeEnvelope(serialized);
    
    expect(deserialized.getFrom()).toBe(original.getFrom());
    expect(deserialized.getCapabilities()).toEqual(original.getCapabilities());
  });
});
```

### Integration Testing
```typescript
describe('WebSocket Transport', () => {
  let server: AdvancedWebSocketTransport;
  let client: AdvancedWebSocketTransport;
  
  beforeEach(async () => {
    server = new AdvancedWebSocketTransport({
      port: 8081,
      isServer: true
    });
    
    client = new AdvancedWebSocketTransport({
      url: 'ws://localhost:8081',
      isServer: false
    });
    
    await server.connect();
    await client.connect();
  });
  
  afterEach(async () => {
    await client.disconnect();
    await server.disconnect();
  });
  
  test('should exchange messages', async () => {
    return new Promise<void>((resolve) => {
      server.on('message', (message) => {
        expect(message.from).toBe('client');
        resolve();
      });
      
      const envelope = UMICP.createEnvelope({
        from: 'client',
        to: 'server',
        operation: OperationType.DATA,
        messageId: 'test-msg'
      });
      
      client.send(envelope);
    });
  });
});
```

## Deployment Considerations

### Production Configuration
```typescript
const productionConfig = {
  heartbeatInterval: 30000,      // 30 seconds
  maxReconnectAttempts: 10,      // More aggressive reconnection
  perMessageDeflate: true,       // Enable compression
  maxPayload: 10 * 1024 * 1024, // 10MB max payload
};
```

### Monitoring and Logging
```typescript
transport.on('connected', () => {
  console.log(`[${new Date().toISOString()}] UMICP connection established`);
});

transport.on('disconnected', (code, reason) => {
  console.log(`[${new Date().toISOString()}] UMICP disconnected: ${code} - ${reason}`);
});

transport.on('error', (error) => {
  console.error(`[${new Date().toISOString()}] UMICP error:`, error);
});
```

### Health Checks
```typescript
class UMICPHealthChecker {
  constructor(private transport: AdvancedWebSocketTransport) {}
  
  async checkHealth(): Promise<boolean> {
    if (!this.transport.isConnected()) {
      return false;
    }
    
    // Send ping message
    const ping = UMICP.createEnvelope({
      from: 'health-checker',
      to: 'server',
      operation: OperationType.CONTROL,
      messageId: `ping-${Date.now()}`,
      capabilities: { 'type': 'ping' }
    });
    
    return this.transport.send(ping);
  }
}
```

## Troubleshooting

### Common Issues

#### Connection Failures
- Check network connectivity
- Verify server is running and listening
- Confirm correct URL and port
- Check firewall settings

#### Message Serialization Errors
- Ensure all capability values are strings
- Validate operation types are numbers
- Check for circular references in objects

#### Performance Issues
- Enable compression for large messages
- Implement message batching for high throughput
- Use connection pooling for multiple endpoints
- Monitor memory usage and implement cleanup

### Debug Mode
```typescript
// Enable debug logging
process.env.DEBUG = 'umicp:*';

// Or use console logging
transport.on('message', (message) => {
  console.log('DEBUG: Received message:', JSON.stringify(message, null, 2));
});
```

## Migration Guide

### From Version 1.0 to 1.1
- Update import statements for new transport class
- Replace manual envelope creation with utility functions
- Update error handling for new error types
- Review connection configuration options

## Support and Resources

- **GitHub Repository**: https://github.com/cmmv-hive/umicp
- **Documentation**: https://umicp.dev/docs
- **Issue Tracker**: https://github.com/cmmv-hive/umicp/issues
- **Community Forum**: https://community.umicp.dev

## License

The UMICP TypeScript bindings are released under the MIT License. See LICENSE file for details.
