# UMICP TypeScript SDK - Communication Examples

Practical examples demonstrating real-world UMICP communication patterns.

## 📋 Examples Overview

| Example | Description | Key Features |
|---------|-------------|--------------|
| **01-basic-peer-communication.ts** | Basic P2P | Simple hub-worker communication |
| **02-mesh-network.ts** | Mesh Network | 3-node interconnected mesh |
| **03-http-communication.ts** | HTTP Transport | REST-like communication |
| **04-request-response-pattern.ts** | Request-Response | RPC-style messaging |
| **05-broadcast-patterns.ts** | Broadcast | Various broadcast strategies |
| **06-federated-learning-simulation.ts** | Federated Learning | ML model distribution |

## 🚀 Quick Start

### Prerequisites

```bash
# Install dependencies (requires 64-bit system)
npm install

# Build the SDK
npm run build
```

### Running Examples

#### Using ts-node (Recommended)
```bash
# Run specific example
npx ts-node examples/01-basic-peer-communication.ts

# Or compile and run
npm run build
node dist/examples/01-basic-peer-communication.js
```

#### Using npm scripts (add to package.json)
```json
{
  "scripts": {
    "example:basic": "ts-node examples/01-basic-peer-communication.ts",
    "example:mesh": "ts-node examples/02-mesh-network.ts",
    "example:http": "ts-node examples/03-http-communication.ts",
    "example:request-response": "ts-node examples/04-request-response-pattern.ts",
    "example:broadcast": "ts-node examples/05-broadcast-patterns.ts",
    "example:federated": "ts-node examples/06-federated-learning-simulation.ts"
  }
}
```

## 📖 Example Details

### Example 1: Basic Peer Communication

**Purpose:** Learn the fundamentals of peer-to-peer communication.

**What it demonstrates:**
- Creating a server peer (hub)
- Creating a client peer (worker)
- Connecting peers
- Sending messages
- Receiving acknowledgments
- Event handling

**Run it:**
```bash
npx ts-node examples/01-basic-peer-communication.ts
```

**Expected output:**
```
🚀 UMICP Example 1: Basic Peer Communication

Creating Hub peer on port 8081...
✅ Hub is ready and listening on port 8081
Creating Worker peer...
✅ Worker connected to ws://localhost:8081/umicp
🤝 Handshake complete with: worker-1
💬 Sending messages from worker to hub...
   Sent message #1
📨 Hub received message from incoming-...
   Message: Hello from worker! Message #1
   ✅ Sent acknowledgment
...
```

### Example 2: Mesh Network

**Purpose:** Create a distributed mesh of interconnected peers.

**What it demonstrates:**
- Multiple peer instances
- Mesh topology (A↔B↔C↔A)
- Broadcasting in mesh
- Point-to-point in mesh
- Connection type tracking

**Run it:**
```bash
npx ts-node examples/02-mesh-network.ts
```

**Expected output:**
```
🕸️  UMICP Example 2: Mesh Network

Creating mesh network nodes...
🔴 node-A ready on port 8091
🟢 node-B ready on port 8092
🔵 node-C ready on port 8093
🔗 Connecting mesh network...
✅ Mesh network established!
📡 Broadcasting from each node...
...
```

### Example 3: HTTP Communication

**Purpose:** Use HTTP as transport instead of WebSocket.

**What it demonstrates:**
- HTTP-based UMICP
- Health check endpoint
- Request-response over HTTP
- Concurrent HTTP requests
- StreamableHTTPPeer usage

**Run it:**
```bash
npx ts-node examples/03-http-communication.ts
```

**Expected output:**
```
🌐 UMICP Example 3: HTTP Communication

═══ Part 1: Basic HTTP Server-Client ═══
✅ HTTP Server listening on port 9080
Testing health check endpoint...
Health check response: { status: 'ok', protocol: 'UMICP', ... }
...
```

### Example 4: Request-Response Pattern

**Purpose:** Implement RPC-like request-response communication.

**What it demonstrates:**
- Sending requests and waiting for responses
- Correlation IDs (reply_to)
- Timeout handling
- Error responses
- Concurrent requests

**Run it:**
```bash
npx ts-node examples/04-request-response-pattern.ts
```

**Expected output:**
```
🔄 UMICP Example 4: Request-Response Pattern

✅ API Server ready
✅ Client connected

📤 Request 1: Get user data
📨 Server received request: get-user
📬 Response received:
   Status: success
   User: John Doe (john@example.com)
...
```

### Example 5: Broadcast Patterns

**Purpose:** Master different broadcasting strategies.

**What it demonstrates:**
- Broadcast to all peers
- Broadcast to incoming/outgoing only
- Selective broadcast (exclude peers)
- Role-based broadcast
- Sequential broadcast
- Conditional broadcast

**Run it:**
```bash
npx ts-node examples/05-broadcast-patterns.ts
```

**Expected output:**
```
📡 UMICP Example 5: Broadcast Patterns

✅ Broadcast Hub ready
Creating client peers...
✅ All clients connected

═══ Pattern 1: Broadcast to ALL ═══
📡 Broadcast sent to 5 peer(s)
   📨 worker-1 (worker) received: Important announcement...
...
```

### Example 6: Federated Learning

**Purpose:** Simulate a federated learning system.

**What it demonstrates:**
- Coordinator-worker pattern
- Model weight distribution
- Gradient aggregation
- PayloadHint for large vectors
- Multi-round training simulation

**Run it:**
```bash
npx ts-node examples/06-federated-learning-simulation.ts
```

**Expected output:**
```
🤖 UMICP Example 6: Federated Learning Simulation

✅ FL Coordinator ready
Creating FL workers...
✅ All workers connected and ready

═══ Starting Federated Learning Rounds ═══
🔄 Round 1 starting...
📤 Coordinator sent weights for round 1
   📥 worker-node-1 received weights for round 1
      🔄 Training on 1000 local samples...
...
```

## 🎓 Learning Path

### Beginner
1. Start with **Example 1** (Basic Communication)
2. Try **Example 3** (HTTP Transport)
3. Explore **Example 4** (Request-Response)

### Intermediate
4. Study **Example 2** (Mesh Network)
5. Master **Example 5** (Broadcast Patterns)

### Advanced
6. Implement **Example 6** (Federated Learning)

## 🔧 Customization

### Modifying Ports

All examples use ports 8090-8099 and 9080-9099. To change:

```typescript
// In each example
const peer = new UMICPWebSocketPeer({
  peerId: 'my-peer',
  server: {
    port: 8080, // Change this
    path: '/umicp'
  }
});
```

### Adding Your Own Logic

Each example has clear sections where you can add your own logic:

```typescript
peer.on('data', (envelope, peerConnection) => {
  // Add your message handling logic here
  console.log('Received:', envelope.getCapabilities());
  
  // Your custom processing
  processMessage(envelope);
  
  // Send response if needed
  const response = createResponse(envelope);
  peer.sendToPeer(peerConnection.id, response);
});
```

## 🐛 Troubleshooting

### Port Already in Use

```bash
Error: listen EADDRINUSE: address already in use :::8080
```

**Solution:** Change the port number or kill the process using that port:
```bash
# Windows
netstat -ano | findstr :8080
taskkill /PID <PID> /F

# Linux/Mac
lsof -ti:8080 | xargs kill -9
```

### Native Addon Not Found

```bash
Error: Cannot find module 'umicp_core.node'
```

**Solution:** Rebuild the native addon:
```bash
npm run build
```

### Connection Refused

```bash
Error: connect ECONNREFUSED 127.0.0.1:8080
```

**Solution:** Make sure the server peer is running first:
```typescript
// Wait for server to be ready
await new Promise<void>(resolve => {
  server.on('server:ready', () => resolve());
});

// Then connect client
await client.connectToPeer('ws://localhost:8080/umicp');
```

### Handshake Timeout

```bash
⚠️  Handshake timeout with peer
```

**Solution:** Ensure `autoProtocol` is enabled on both peers:
```typescript
const peer = new UMICPWebSocketPeer({
  peerId: 'my-peer',
  autoProtocol: true, // Enable this
  handshakeTimeout: 10000
});
```

## 📊 Example Comparison

| Feature | Example 1 | Example 2 | Example 3 | Example 4 | Example 5 | Example 6 |
|---------|-----------|-----------|-----------|-----------|-----------|-----------|
| **Transport** | WebSocket | WebSocket | HTTP | WebSocket | WebSocket | WebSocket |
| **Pattern** | Simple P2P | Mesh | Client-Server | RPC | Broadcast | Coordinator-Worker |
| **Complexity** | ⭐ | ⭐⭐⭐ | ⭐⭐ | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐ |
| **Peers** | 2 | 3 | 1 server + N clients | 2 | 1 hub + 5 clients | 1 coordinator + 3 workers |
| **Use Case** | Learning | Distributed | REST API | RPC/API | Pub/Sub | ML Training |

## 💡 Best Practices

### 1. Always Wait for server:ready
```typescript
await new Promise<void>(resolve => {
  peer.on('server:ready', () => resolve());
});
```

### 2. Handle Errors
```typescript
peer.on('error', (error, peerConnection) => {
  console.error('Error:', error.message);
  // Handle error appropriately
});
```

### 3. Cleanup Resources
```typescript
// Always shutdown peers when done
await peer.shutdown();
```

### 4. Use PayloadHint for Large Data
```typescript
const envelope = new Envelope({
  from: 'sender',
  to: 'receiver',
  operation: OperationType.DATA,
  messageId: 'large-data',
  payloadHint: {
    type: PayloadType.VECTOR,
    size: 1000000,
    encoding: EncodingType.FLOAT32
  }
});
```

### 5. Validate Envelopes
```typescript
if (!envelope.validate()) {
  console.error('Invalid envelope');
  return;
}
```

## 🔗 Related Documentation

- [Main README](../README.md) - SDK documentation
- [Multiplexed Peer Guide](../docs/MULTIPLEXED_PEER.md) - Detailed P2P docs
- [Quick Start](../docs/QUICKSTART_MULTIPLEXED.md) - Fast setup guide
- [API Reference](../docs/API.md) - Complete API docs
- [Test Suite](../test/README.md) - Test examples

## 🤝 Contributing

Want to add more examples? Great! Please:

1. Follow the existing naming pattern (`##-description.ts`)
2. Include comprehensive comments
3. Add entry to this README
4. Test your example thoroughly
5. Submit a pull request

## 📝 License

MIT License - see [LICENSE](../LICENSE) for details

---

**Created:** October 10, 2025  
**Examples:** 6 practical communication patterns  
**Status:** Ready to use ✅

