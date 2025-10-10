# Quick Start: UMICP Multiplexed Peer

## Installation

```bash
cd umicp/bindings/typescript
npm install
npm run build
```

## Simple Example: 2 Peers Communicating

### Peer A (Server + Client)

```typescript
import { UMICPWebSocketPeer, Envelope, OperationType } from '@hivellm/umicp';

const peerA = new UMICPWebSocketPeer({
  peerId: 'peer-a',
  
  // Accepts connections on port 20081
  server: {
    port: 20081,
    path: '/umicp'
  }
});

// Setup event handlers
peerA.on('message', async (envelope, peer) => {
  console.log(`[A] Received from ${peer.id}:`, envelope.getCapabilities());
  
  // Respond
  const response = new Envelope({
    from: 'peer-a',
    to: envelope.getFrom(),
    operation: OperationType.ACK,
    messageId: `ack-${Date.now()}`,
    capabilities: { message: 'Received!' }
  });
  
  peerA.sendToPeer(peer.id, response);
});

peerA.on('peer:connect', (peer) => {
  console.log(`[A] Peer connected: ${peer.id} (${peer.type})`);
});

// Wait for Peer B to start, then connect
setTimeout(async () => {
  const id = await peerA.connectToPeer('ws://localhost:20082/umicp');
  console.log(`[A] Connected to B: ${id}`);
  
  // Send test message
  const msg = new Envelope({
    from: 'peer-a',
    to: 'peer-b',
    operation: OperationType.DATA,
    messageId: `msg-${Date.now()}`,
    capabilities: { message: 'Hello from A!' }
  });
  
  peerA.sendToPeerByUrl('ws://localhost:20082/umicp', msg);
}, 2000);
```

### Peer B (Server + Client)

```typescript
import { UMICPWebSocketPeer, Envelope, OperationType } from '@hivellm/umicp';

const peerB = new UMICPWebSocketPeer({
  peerId: 'peer-b',
  
  // Accepts connections on port 20082
  server: {
    port: 20082,
    path: '/umicp'
  }
});

// Setup event handlers
peerB.on('message', async (envelope, peer) => {
  console.log(`[B] Received from ${peer.id}:`, envelope.getCapabilities());
  
  // Respond
  const response = new Envelope({
    from: 'peer-b',
    to: envelope.getFrom(),
    operation: OperationType.ACK,
    messageId: `ack-${Date.now()}`,
    capabilities: { message: 'Got it!' }
  });
  
  peerB.sendToPeer(peer.id, response);
});

peerB.on('peer:connect', (peer) => {
  console.log(`[B] Peer connected: ${peer.id} (${peer.type})`);
});

peerB.on('server:ready', () => {
  console.log('[B] Ready and listening on port 20082');
});
```

### Run

```bash
# Terminal 1
node peer-b.js

# Terminal 2
node peer-a.js
```

**Result:**
- Peer B accepts connection from A
- Peer A sends message to B
- B responds
- Bidirectional communication established!

## Example: Broadcast to Multiple Peers

```typescript
// Central hub
const hub = new UMICPWebSocketPeer({
  peerId: 'hub',
  server: { port: 20080 }
});

hub.on('message', async (envelope, peer) => {
  // Rebroadcast to all except sender
  hub.broadcast(envelope, peer.id);
});

hub.on('peer:connect', (peer) => {
  console.log(`Hub: New peer ${peer.id}`);
});

// Workers connect to hub
const worker1 = new UMICPWebSocketPeer({ peerId: 'worker-1' });
const worker2 = new UMICPWebSocketPeer({ peerId: 'worker-2' });
const worker3 = new UMICPWebSocketPeer({ peerId: 'worker-3' });

await worker1.connectToPeer('ws://localhost:20080/umicp');
await worker2.connectToPeer('ws://localhost:20080/umicp');
await worker3.connectToPeer('ws://localhost:20080/umicp');

// Worker 1 sends, all receive
const msg = new Envelope({
  from: 'worker-1',
  to: 'broadcast',
  operation: OperationType.DATA,
  messageId: `broadcast-${Date.now()}`,
  capabilities: { message: 'Hello everyone!' }
});

worker1.sendToPeerByUrl('ws://localhost:20080/umicp', msg);

// Hub rebroadcasts → worker2 and worker3 receive
```

## Complete Example: Agents with MCP

See the complete example at:
```
agent-framework/typescript/examples/umicp/multiplexed-agent.ts
```

Run:
```bash
cd agent-framework/typescript
npm install
npm run build

# Run the 3 interconnected agents example
node dist/examples/umicp/multiplexed-agent.js
```

This will start:
- **agent-alpha** (MCP:16001, UMICP:20081)
- **agent-beta** (MCP:16002, UMICP:20082)
- **agent-gamma** (MCP:16003, UMICP:20083)

Ring topology: alpha → beta → gamma → alpha

### Test via MCP

```bash
# List peers from agent-alpha
curl http://localhost:16001/mcp/tools/call \
  -H "Content-Type: application/json" \
  -d '{"name":"list_peers","arguments":{}}'

# Broadcast message
curl http://localhost:16001/mcp/tools/call \
  -H "Content-Type: application/json" \
  -d '{"name":"broadcast_message","arguments":{"message":"Hello Network!"}}'

# Get statistics
curl http://localhost:16001/mcp/tools/call \
  -H "Content-Type: application/json" \
  -d '{"name":"get_peer_stats","arguments":{}}'

# Connect to new peer dynamically
curl http://localhost:16001/mcp/tools/call \
  -H "Content-Type: application/json" \
  -d '{"name":"connect_to_peer","arguments":{"url":"ws://localhost:20084/umicp"}}'
```

## Common Patterns

### 1. Request-Response

```typescript
// Send and wait for response
const response = await peer.sendAndWait(
  'target-peer-id',
  requestEnvelope,
  30000 // timeout 30s
);

console.log('Response:', response.getCapabilities());
```

### 2. Pub-Sub

```typescript
// Publisher
const event = new Envelope({
  from: 'publisher',
  to: 'subscribers',
  operation: OperationType.DATA,
  messageId: `event-${Date.now()}`,
  capabilities: { 
    event: 'user.created',
    data: { userId: 123 }
  }
});

peer.broadcast(event);

// Subscribers
peer.on('message', async (envelope, peer) => {
  const caps = envelope.getCapabilities();
  if (caps.event === 'user.created') {
    console.log('New user:', caps.data);
  }
});
```

### 3. Pipeline

```typescript
// Stage 1: Ingest
const stage1 = new UMICPWebSocketPeer({ 
  peerId: 'ingest',
  server: { port: 20081 }
});

stage1.on('message', async (env, peer) => {
  // Process and forward to stage 2
  const processed = { ...env.getCapabilities(), processed: true };
  const next = new Envelope({
    from: 'ingest',
    to: 'transform',
    operation: OperationType.DATA,
    messageId: `fwd-${Date.now()}`,
    capabilities: processed
  });
  stage1.sendToPeerByUrl('ws://localhost:20082/umicp', next);
});

// Stage 2: Transform
const stage2 = new UMICPWebSocketPeer({ 
  peerId: 'transform',
  server: { port: 20082 }
});

stage2.on('message', async (env, peer) => {
  // Transform and forward to stage 3
  // ...
});

await stage1.connectToPeer('ws://localhost:20082/umicp');
```

### 4. Load Balancing

```typescript
let currentWorker = 0;
const workers = ['worker-1-id', 'worker-2-id', 'worker-3-id'];

hub.on('message', async (envelope, peer) => {
  // Round-robin
  const workerId = workers[currentWorker];
  currentWorker = (currentWorker + 1) % workers.length;
  
  hub.sendToPeer(workerId, envelope);
});
```

## Troubleshooting

### Peer doesn't connect

```typescript
// Add detailed logging
const peer = new UMICPWebSocketPeer({
  peerId: 'my-peer',
  server: { port: 20081 }
});

peer.on('peer:connect', (p) => console.log('✅ Connected:', p.id));
peer.on('peer:disconnect', (p) => console.log('❌ Disconnected:', p.id));
peer.on('error', (err, p) => console.error('⚠️  Error:', err.message, p?.id));

// Check if server is running
const stats = peer.getStats();
console.log('Server active:', stats.serverActive);
console.log('Total peers:', stats.totalPeers);
```

### Messages don't arrive

```typescript
// Check if peer is connected
const targetPeer = peer.getPeer('target-id');
if (!targetPeer) {
  console.error('Peer not found');
} else if (!targetPeer.client.isConnected()) {
  console.error('Peer not connected');
}

// Validate envelope
if (!envelope.validate()) {
  console.error('Invalid envelope');
}

// Check send status
const sent = peer.sendToPeer('target-id', envelope);
console.log('Sent:', sent);
```

### Debug connections

```typescript
// See all connections
peer.getPeers().forEach(p => {
  console.log(`ID: ${p.id}`);
  console.log(`Type: ${p.type}`);
  console.log(`URL: ${p.url || 'incoming'}`);
  console.log(`Connected: ${p.connectedAt}`);
  console.log(`---`);
});

// Monitor in real-time
setInterval(() => {
  const stats = peer.getStats();
  console.log(`[${new Date().toISOString()}] Peers: ${stats.totalPeers} (↓${stats.incomingConnections} ↑${stats.outgoingConnections})`);
}, 5000);
```

## Next Steps

1. Read the [complete documentation](./MULTIPLEXED_PEER.md)
2. Explore the [complete example](../../agent-framework/typescript/examples/umicp/multiplexed-agent.ts)
3. Implement your own use case
4. Contribute improvements!

## Resources

- [UMICP Spec](../../docs/PROTOCOL.md)
- [WebSocket Transport](./src/transports/websocket-transport.ts)
- [Examples](../../agent-framework/typescript/examples/umicp/)

