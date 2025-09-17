# UMICP TypeScript Bindings

[![npm version](https://badge.fury.io/js/%40umicp%2Fcore.svg)](https://badge.fury.io/js/%40umicp%2Fcore)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Tests](https://github.com/cmmv-hive/umicp/workflows/tests/badge.svg)](https://github.com/cmmv-hive/umicp/actions)

TypeScript bindings for the Universal Matrix Inter-Communication Protocol (UMICP), providing high-performance communication and matrix operations for distributed systems, federated learning, and real-time applications.

## 🚀 Features

- **🔗 Universal Communication**: WebSocket-based transport with automatic reconnection
- **📦 Type-Safe Envelopes**: Strongly-typed message serialization and validation
- **⚡ High Performance**: Optimized matrix operations with SIMD support
- **🔄 Federated Learning**: Built-in support for ML model distribution and aggregation
- **🛡️ Security First**: Input validation, rate limiting, and error handling
- **📊 Real-time**: Low-latency communication for IoT and financial applications
- **🧪 Well Tested**: Comprehensive test suite with 100% pass rate

## 📦 Installation

```bash
npm install @umicp/core
```

### Prerequisites
- Node.js 14+
- TypeScript 4.5+

## 🏃 Quick Start

### Basic Envelope Usage

```typescript
import { UMICP, OperationType } from '@umicp/core';

// Create a UMICP envelope
const envelope = UMICP.createEnvelope({
  from: 'client-001',
  to: 'server-001',
  operation: OperationType.DATA,
  messageId: 'msg-12345',
  capabilities: {
    'content-type': 'application/json',
    'priority': 'high'
  }
});

// Serialize for transmission
const serialized = envelope.serialize();
console.log('Serialized envelope:', serialized);

// Deserialize received data
const received = UMICP.deserializeEnvelope(serialized);
console.log('From:', received.getFrom());
console.log('Capabilities:', received.getCapabilities());
```

### WebSocket Transport

```typescript
import { AdvancedWebSocketTransport } from '@umicp/core/transport';

// Server setup
const server = new AdvancedWebSocketTransport({
  port: 8080,
  isServer: true,
  heartbeatInterval: 5000
});

// Client setup
const client = new AdvancedWebSocketTransport({
  url: 'ws://localhost:8080',
  isServer: false,
  maxReconnectAttempts: 3
});

// Message handling
server.on('message', (envelope, connectionInfo) => {
  console.log('Received:', envelope.getCapabilities());
  
  // Echo response
  const response = UMICP.createEnvelope({
    from: 'server',
    to: envelope.getFrom(),
    operation: OperationType.ACK,
    messageId: `response-${Date.now()}`
  });
  
  server.send(response, connectionInfo.id);
});

// Start communication
await server.connect();
await client.connect();

// Send message
const message = UMICP.createEnvelope({
  from: 'client',
  to: 'server',
  operation: OperationType.DATA,
  messageId: 'hello-001',
  capabilities: { 'message': 'Hello UMICP!' }
});

await client.send(message);
```

### Matrix Operations

```typescript
import { Matrix } from '@umicp/core';

// Create matrix instance
const matrix = new Matrix();

// Vector operations
const vector1 = new Float32Array([1, 2, 3, 4]);
const vector2 = new Float32Array([5, 6, 7, 8]);
const result = new Float32Array(4);

// Vector addition
matrix.vectorAdd(vector1, vector2, result);
console.log('Addition result:', result); // [6, 8, 10, 12]

// Dot product
const dotProduct = matrix.dotProduct(vector1, vector2);
console.log('Dot product:', dotProduct); // 70

// Matrix multiplication
const matrixA = new Float32Array([1, 2, 3, 4]); // 2x2 matrix
const matrixB = new Float32Array([5, 6, 7, 8]); // 2x2 matrix
const matrixResult = new Float32Array(4);

matrix.matrixMultiply(matrixA, matrixB, 2, 2, 2, matrixResult);
console.log('Matrix multiplication:', matrixResult);
```

## 🧪 Testing

### Run All Tests
```bash
npm test
```

### Run Specific Test Suites
```bash
# End-to-end integration tests
npm test -- --testPathPattern="e2e.test.ts"

# Performance tests
npm test -- --testPathPattern="load.test.ts"

# Security tests
npm test -- --testPathPattern="security.test.ts"
```

### Test Performance
- **E2E Tests**: ~5.1 seconds (10 tests, 100% pass rate)
- **Load Tests**: High-throughput and stress testing
- **Security Tests**: Input validation and injection protection

## 📚 API Reference

### Core Classes

#### `UMICP`
Static utility class for envelope operations.

```typescript
// Create envelope
UMICP.createEnvelope(options: EnvelopeOptions): Envelope

// Serialize envelope
UMICP.serializeEnvelope(envelope: Envelope): string

// Deserialize envelope
UMICP.deserializeEnvelope(json: string): Envelope
```

#### `Envelope`
Message container with metadata and capabilities.

```typescript
envelope.setFrom(from: string): Envelope
envelope.setTo(to: string): Envelope
envelope.setOperation(op: OperationType): Envelope
envelope.setMessageId(id: string): Envelope
envelope.setCapabilities(caps: Record<string, string>): Envelope

envelope.getFrom(): string
envelope.getTo(): string
envelope.getOperation(): OperationType
envelope.getMessageId(): string
envelope.getCapabilities(): Record<string, string>

envelope.serialize(): string
envelope.validate(): boolean
envelope.getHash(): string
```

#### `AdvancedWebSocketTransport`
High-performance WebSocket transport with connection management.

```typescript
// Configuration
interface WebSocketConfig {
  url?: string;           // Client connection URL
  port?: number;          // Server port
  isServer: boolean;      // Server or client mode
  heartbeatInterval?: number;
  maxReconnectAttempts?: number;
  perMessageDeflate?: boolean;
  maxPayload?: number;
}

// Methods
transport.connect(): Promise<boolean>
transport.disconnect(): Promise<boolean>
transport.send(message: string | object, targetId?: string): boolean
transport.isConnected(): boolean
transport.getStats(): TransportStats

// Events
transport.on('connected', () => {})
transport.on('disconnected', (code, reason) => {})
transport.on('message', (message, connectionInfo) => {})
transport.on('error', (error) => {})
```

### Operation Types

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

## 🎯 Use Cases

### Federated Learning
```typescript
// Model weight distribution
const weightsEnvelope = UMICP.createEnvelope({
  from: 'coordinator',
  to: 'worker-001',
  operation: OperationType.DATA,
  messageId: `weights-${Date.now()}`,
  capabilities: {
    'model-version': '1.0',
    'layer-count': '12',
    'weights': JSON.stringify(modelWeights)
  }
});

await transport.send(weightsEnvelope);
```

### IoT Data Streaming
```typescript
// Sensor data collection
const sensorData = UMICP.createEnvelope({
  from: 'sensor-temp-001',
  to: 'data-collector',
  operation: OperationType.DATA,
  messageId: `temp-${Date.now()}`,
  capabilities: {
    'sensor-type': 'temperature',
    'value': temperature.toString(),
    'unit': 'celsius',
    'timestamp': Date.now().toString()
  }
});

await transport.send(sensorData);
```

### Financial Transactions
```typescript
// Transaction processing
const transaction = UMICP.createEnvelope({
  from: 'payment-gateway',
  to: 'transaction-processor',
  operation: OperationType.REQUEST,
  messageId: `txn-${transactionId}`,
  capabilities: {
    'amount': amount.toString(),
    'currency': 'USD',
    'merchant-id': merchantId,
    'customer-id': customerId
  }
});

await transport.send(transaction);
```

## 🔧 Configuration

### Environment Variables
```bash
# WebSocket configuration
UMICP_WS_PORT=8080
UMICP_WS_HOST=localhost
UMICP_WS_MAX_PAYLOAD=1048576

# Performance tuning
UMICP_HEARTBEAT_INTERVAL=5000
UMICP_MAX_RECONNECT_ATTEMPTS=3
UMICP_CONNECTION_TIMEOUT=10000
```

### TypeScript Configuration
```json
{
  "compilerOptions": {
    "target": "ES2020",
    "module": "commonjs",
    "lib": ["ES2020"],
    "strict": true,
    "esModuleInterop": true,
    "skipLibCheck": true
  }
}
```

## 📈 Performance

### Benchmarks
- **Envelope Creation**: ~1ms per envelope
- **Serialization**: ~8ms for complex envelopes  
- **WebSocket Connection**: <100ms establishment
- **Message Throughput**: 1000+ messages/second
- **Matrix Operations**: SIMD-optimized performance

### Memory Usage
- **Envelope Overhead**: ~200 bytes per envelope
- **Connection Overhead**: ~50KB per WebSocket connection
- **Matrix Operations**: Efficient memory allocation with cleanup

## 🛠️ Development

### Build from Source
```bash
git clone https://github.com/cmmv-hive/umicp.git
cd umicp/bindings/typescript
npm install
npm run build
```

### Development Scripts
```bash
npm run dev          # Development mode with watch
npm run build        # Production build
npm run test         # Run all tests
npm run test:watch   # Watch mode testing
npm run lint         # Code linting
npm run docs         # Generate documentation
```

## 📖 Documentation

- [API Reference](./docs/API.md)
- [E2E Test Guide](./docs/E2E_TESTS.md)
- [Transport Layer](./docs/TRANSPORT.md)
- [Security Guide](./docs/SECURITY.md)
- [Performance Tuning](./docs/PERFORMANCE.md)

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](./CONTRIBUTING.md) for details.

### Development Setup
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

## 🔗 Links

- [GitHub Repository](https://github.com/cmmv-hive/umicp)
- [NPM Package](https://www.npmjs.com/package/@umicp/core)
- [Documentation](https://umicp.dev/docs)
- [Issue Tracker](https://github.com/cmmv-hive/umicp/issues)

## 🙏 Acknowledgments

- CMMV Hive community for protocol design and testing
- Contributors and maintainers
- Open source dependencies and tools

---

**UMICP TypeScript Bindings** - Empowering distributed systems with universal communication protocols.
