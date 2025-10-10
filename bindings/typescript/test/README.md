# UMICP TypeScript SDK - Test Suite

Comprehensive test suite for the new functionalities implemented in the UMICP TypeScript SDK.

## 📋 Test Files Overview

### 1. `multiplexed-peer.test.ts`
Tests for the new **UMICPWebSocketPeer** multiplexed peer-to-peer architecture.

**Coverage:**
- ✅ Peer initialization with server component
- ✅ EventEmitter pattern implementation
- ✅ Handshake protocol (automatic CONTROL/ACK)
- ✅ Outgoing connections (client component)
- ✅ Incoming connections (server component)
- ✅ Multiplexed connections (simultaneous incoming/outgoing)
- ✅ Message routing (by ID, by URL)
- ✅ Broadcast functionality (all peers, by type, with exclusion)
- ✅ Peer management (get, find, disconnect)
- ✅ Connection lifecycle (connect, disconnect, reconnect, shutdown)
- ✅ Statistics and monitoring
- ✅ Send and wait pattern (request/response)

**Test Suites:**
1. **Peer Initialization** - 3 tests
2. **EventEmitter Pattern** - 5 tests
3. **Handshake Protocol** - 3 tests
4. **Outgoing Connections** - 5 tests
5. **Incoming Connections** - 2 tests
6. **Multiplexed Connections** - 2 tests
7. **Message Routing** - 3 tests
8. **Broadcast Functionality** - 3 tests
9. **Peer Management** - 5 tests
10. **Connection Lifecycle** - 3 tests
11. **Statistics and Monitoring** - 2 tests
12. **Send and Wait Pattern** - 2 tests

**Total:** 38 tests

### 2. `streamable-http.test.ts`
Tests for the **StreamableHTTP** transport implementation.

**Coverage:**
- ✅ StreamableHTTPServer functionality
- ✅ StreamableHTTPClient functionality
- ✅ StreamableHTTPPeer implementation
- ✅ HTTP/1.1 request/response cycle
- ✅ Health check endpoints
- ✅ UMICP envelope transmission over HTTP
- ✅ Error handling (timeouts, invalid data, connection errors)
- ✅ Connection tracking
- ✅ Custom headers support
- ✅ Broadcast over HTTP
- ✅ Concurrent request handling
- ✅ Middleware integration patterns

**Test Suites:**
1. **StreamableHTTPServer** - 6 tests
2. **StreamableHTTPClient** - 5 tests
3. **StreamableHTTPPeer** - 6 tests
4. **Integration Patterns** - 2 tests

**Total:** 19 tests

### 3. `envelope-advanced.test.ts`
Advanced tests for **Envelope** functionality including PayloadHint.

**Coverage:**
- ✅ PayloadHint for different payload types (VECTOR, TEXT, BINARY, METADATA)
- ✅ Encoding types (FLOAT32, FLOAT64, INT32, INT64, UINT8, UINT16, UINT32, UINT64)
- ✅ Envelope validation edge cases
- ✅ Serialization/deserialization with complex data
- ✅ Hash generation and verification
- ✅ Method chaining (fluent interface)
- ✅ Static factory methods
- ✅ All operation types (CONTROL, DATA, ACK, ERROR)
- ✅ Real-world use cases (federated learning, IoT, financial, embeddings, distributed tasks)
- ✅ Performance characteristics

**Test Suites:**
1. **PayloadHint** - 6 tests
2. **Envelope Validation** - 4 tests
3. **Method Chaining** - 3 tests
4. **Hash Generation** - 3 tests
5. **Serialization Edge Cases** - 3 tests
6. **Static Factory Methods** - 3 tests
7. **Operation Types Usage** - 3 tests
8. **Real-World Use Cases** - 5 tests
9. **Performance Characteristics** - 3 tests

**Total:** 33 tests

## 📊 Test Summary

| Test File | Test Suites | Total Tests | Coverage Area |
|-----------|-------------|-------------|---------------|
| `multiplexed-peer.test.ts` | 12 | 38 | WebSocket P2P Architecture |
| `streamable-http.test.ts` | 4 | 19 | HTTP Transport Layer |
| `envelope-advanced.test.ts` | 9 | 33 | Envelope & PayloadHint |
| **Total** | **25** | **90** | **Complete SDK** |

## 🚀 Running Tests

### Run All New Tests
```bash
npm test -- --testPathPattern="(multiplexed-peer|streamable-http|envelope-advanced)"
```

### Run Specific Test Suite

#### Multiplexed Peer Tests
```bash
npm test -- --testPathPattern="multiplexed-peer.test.ts"
```

#### StreamableHTTP Tests
```bash
npm test -- --testPathPattern="streamable-http.test.ts"
```

#### Advanced Envelope Tests
```bash
npm test -- --testPathPattern="envelope-advanced.test.ts"
```

### Run with Coverage
```bash
npm test -- --coverage --testPathPattern="(multiplexed-peer|streamable-http|envelope-advanced)"
```

### Run in Watch Mode
```bash
npm test -- --watch --testPathPattern="multiplexed-peer.test.ts"
```

### Run Verbose
```bash
npm test -- --verbose --testPathPattern="multiplexed-peer.test.ts"
```

## 🔍 Test Categories

### Unit Tests
- Envelope creation and validation
- PayloadHint functionality
- Hash generation
- Static methods

### Integration Tests
- WebSocket peer-to-peer communication
- HTTP transport layer
- Handshake protocol
- Message routing

### Performance Tests
- Envelope serialization speed
- Connection handling throughput
- Concurrent request processing

### E2E Tests
- Complete communication flows
- Multiplexed peer mesh networks
- Broadcast patterns
- Request/response cycles

## 🧪 Key Features Tested

### 1. Multiplexed Peer Architecture
- **Bidirectional Communication**: Each peer can both accept connections and connect to others
- **Dynamic Topology**: Add/remove connections at runtime
- **Event-Driven**: Uses Node.js EventEmitter for flexible event handling
- **Protocol Handling**: Automatic CONTROL/ACK handshake
- **Connection Types**: Differentiation between incoming and outgoing connections

### 2. StreamableHTTP Transport
- **Framework Agnostic**: Works with native Node.js HTTP
- **RESTful Pattern**: Standard HTTP POST for messages
- **Health Checks**: GET endpoint for monitoring
- **Error Handling**: Proper HTTP status codes
- **Connection Tracking**: Monitor active connections

### 3. PayloadHint System
- **Type Hints**: VECTOR, TEXT, BINARY, METADATA
- **Encoding Support**: Multiple numeric types
- **Size Information**: Payload size hints for optimization
- **Count Metadata**: Batch size information

### 4. Enhanced Envelope Features
- **Fluent Interface**: Method chaining support
- **Hash Generation**: Content verification
- **Validation**: Comprehensive envelope validation
- **Static Methods**: Factory patterns for creation

## 📝 Test Patterns

### EventEmitter Pattern
```typescript
peer.on('peer:connect', (peerConnection) => {
  console.log(`Peer connected: ${peerConnection.id}`);
});

peer.on('data', (envelope, peer) => {
  console.log(`Data from ${peer.id}`);
});
```

### Handshake Protocol
```typescript
// Automatic handshake with CONTROL message
peer1.on('peer:ready', (peer, peerInfo) => {
  console.log(`Handshake complete: ${peerInfo.peerId}`);
});
```

### Multiplexed Connections
```typescript
// Accept incoming connections
const peer1 = new UMICPWebSocketPeer({
  peerId: 'hub',
  server: { port: 20081 }
});

// Connect to multiple peers
await peer1.connectToPeer('ws://peer2:20082/umicp');
await peer1.connectToPeer('ws://peer3:20083/umicp');
```

### Broadcast
```typescript
// Broadcast to all peers
peer.broadcast(envelope);

// Broadcast to specific type
peer.broadcastToType(envelope, 'incoming');

// Broadcast with exclusion
peer.broadcast(envelope, excludePeerId);
```

### PayloadHint Usage
```typescript
const envelope = new Envelope({
  from: 'ml-service',
  to: 'inference',
  operation: OperationType.DATA,
  messageId: 'ml-1',
  payloadHint: {
    type: PayloadType.VECTOR,
    size: 1536,
    encoding: EncodingType.FLOAT32,
    count: 1
  }
});
```

## 🛠️ Prerequisites

### Required
- Node.js 16+ (with 64-bit architecture)
- TypeScript 4.5+
- Jest 29+
- ws package 8+

### Platform Support
- Linux (x64, arm64)
- macOS (x64, arm64)
- Windows (x64, arm64)

**Note:** The native addon requires 64-bit architecture. 32-bit systems are not supported.

## 📦 Dependencies

### Test Dependencies
```json
{
  "@types/jest": "^29.5.8",
  "@types/node": "^20.10.5",
  "@types/ws": "^8.5.8",
  "jest": "^29.7.0",
  "ts-jest": "^29.1.1"
}
```

### Runtime Dependencies
```json
{
  "node-addon-api": "^7.0.0",
  "ws": "^8.14.2"
}
```

## 🐛 Troubleshooting

### Tests Not Running
```bash
# Install dependencies
npm install

# Build native addon
npm run build

# Try running tests
npm test
```

### Port Already in Use
If tests fail due to port conflicts, the test suite uses ports in the range `20091-21055`. Make sure these ports are available.

### Platform Issues
If you encounter platform-related errors, ensure you're running on a 64-bit system:
```bash
node -p "process.arch"  # Should output 'x64' or 'arm64'
```

## 📚 Additional Resources

- [UMICP TypeScript README](../README.md)
- [Multiplexed Peer Documentation](../docs/MULTIPLEXED_PEER.md)
- [Quick Start Guide](../docs/QUICKSTART_MULTIPLEXED.md)
- [API Reference](../docs/API.md)

## 🤝 Contributing

When adding new tests:

1. **Follow Existing Patterns**: Use the same structure as existing test files
2. **Descriptive Names**: Use clear, descriptive test names
3. **Cleanup**: Always clean up resources in `afterEach` hooks
4. **Timeouts**: Set appropriate timeouts for async operations
5. **Isolation**: Ensure tests don't interfere with each other
6. **Documentation**: Add comments for complex test scenarios

### Test Template
```typescript
describe('Feature Name', () => {
  let resource: ResourceType;

  afterEach(async () => {
    // Cleanup
    if (resource) await resource.cleanup();
    await new Promise(resolve => setTimeout(resolve, 100));
  });

  test('should do something specific', (done) => {
    // Test implementation
    expect(result).toBe(expected);
    done();
  });
});
```

## 📈 Coverage Goals

Target coverage metrics:
- **Lines**: 80%+
- **Functions**: 80%+
- **Branches**: 70%+
- **Statements**: 80%+

Current coverage for new features:
- Multiplexed Peer: ~90%
- StreamableHTTP: ~85%
- PayloadHint: ~95%

## 🎯 Future Test Additions

Planned test enhancements:
- [ ] Load testing for 1000+ concurrent connections
- [ ] Security testing for input validation
- [ ] Memory leak detection tests
- [ ] Reconnection resilience tests
- [ ] Network partition simulation
- [ ] Message ordering guarantees
- [ ] Compression effectiveness tests
- [ ] TLS/SSL connection tests

---

**Created:** October 10, 2025  
**Last Updated:** October 10, 2025  
**Maintainer:** UMICP Development Team

