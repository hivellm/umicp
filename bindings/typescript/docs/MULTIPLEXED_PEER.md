# UMICP Multiplexed Peer Architecture

## Overview

The new `UMICPWebSocketPeer` class implements a multiplexed architecture where each peer can **simultaneously**:

1. **Accept connections** from other agents/tools (function as server)
2. **Connect to** multiple remote agents/tools (function as multiple client)

This creates a truly peer-to-peer network where any agent can communicate with any other agent bidirectionally, regardless of who initiated the connection.

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                 UMICPWebSocketPeer                      │
│                  (Agent Alpha)                          │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  🔽 Server Component (port 20081)                      │
│     • Accepts incoming connections                     │
│     • Manages multiple connecting clients              │
│                                                         │
│  🔼 Client Component (multiple connections)            │
│     • Connects to Agent Beta (ws://localhost:20082)    │
│     • Connects to Agent Gamma (ws://localhost:20083)   │
│     • Connects to external tools                       │
│                                                         │
│  📊 Unified Peer Map                                   │
│     • incoming-client-123 → Agent Delta (connected)    │
│     • outgoing-client-456 → Agent Beta (connected)     │
│     • outgoing-client-789 → Agent Gamma (connected)    │
│                                                         │
│  📨 Unified Message Handler                            │
│     • Handles messages from ALL connections            │
│     • Connection direction doesn't matter              │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

## Comparison: Before vs After

### ❌ Old Architecture (Separated)

```typescript
// Agent A - client only
const clientA = new UMICPWebSocketClient({
  url: 'ws://localhost:20082/umicp',
  // ... can only connect, doesn't receive connections
});

// Agent B - server only
const serverB = new UMICPWebSocketServer({
  port: 20082,
  // ... can only receive, doesn't connect to others
});
```

**Problems:**
- Fixed hierarchy: some agents are "servers", others are "clients"
- No true bidirectional communication
- Limited and rigid topology

### ✅ New Architecture (Multiplexed)

```typescript
// Agent A - server + multiple client
const peerA = new UMICPWebSocketPeer({
  peerId: 'agent-alpha',
  
  // Accepts connections
  server: {
    port: 20081,
    path: '/umicp'
  }
});

// Unified handler for ALL connections
peerA.on('message', async (envelope, peer) => {
  console.log(`Message from ${peer.id} (${peer.type})`);
});

// Connect to multiple peers
await peerA.connectToPeer('ws://localhost:20082/umicp');
await peerA.connectToPeer('ws://localhost:20083/umicp');

// Now peerA can:
// - Receive connections from other agents
// - Send messages to any connected peer
// - Broadcast to all peers
```

**Advantages:**
- Fully peer-to-peer: any agent can connect to any other
- Dynamic topology: connections can be created/removed at runtime
- Unified handler: simpler and more consistent code
- Supports complex networks: mesh, hub-and-spoke, hierarchical, etc.

## Complete API

### Create Peer

```typescript
import { UMICPWebSocketPeer, Envelope, OperationType } from '@hivellm/umicp';

const peer = new UMICPWebSocketPeer({
  peerId: 'my-agent',
  
  // Optional: server configuration
  server: {
    httpServer?: HTTPServer,  // or
    port?: number,
    path?: string,
    compression?: boolean,
    maxPayload?: number,
    pingInterval?: number
  }
});

// Events using EventEmitter pattern
peer.on('message', async (envelope: Envelope, peerConnection: PeerConnection) => {
  // Handles messages from ANY peer (incoming or outgoing)
});

peer.on('peer:connect', (peerConnection: PeerConnection) => {
  // Peer connected (direction = peerConnection.type: 'incoming' | 'outgoing')
});

peer.on('peer:disconnect', (peerConnection: PeerConnection) => {
  // Peer disconnected
});

peer.on('error', (error: Error, peerConnection?: PeerConnection) => {
  // Error
});

peer.on('server:ready', () => {
  // Server ready and listening
});

peer.on('connected', (peerId: string, url: string) => {
  // Outgoing connection successfully established
});
```

### Available Events

| Event | Arguments | Description |
|-------|-----------|-------------|
| `message` | `envelope`, `peerConnection` | Message received from any peer |
| `peer:connect` | `peerConnection` | Peer connected (incoming or outgoing) |
| `peer:disconnect` | `peerConnection` | Peer disconnected |
| `error` | `error`, `peerConnection?` | Error occurred |
| `server:ready` | - | Server ready and listening |
| `connected` | `peerId`, `url` | Outgoing connection established |

**Multiple Listeners:**
```typescript
// You can add multiple handlers for the same event
peer.on('message', handler1);
peer.on('message', handler2);

// Remove specific listener
peer.off('message', handler1);

// Remove all listeners for an event
peer.removeAllListeners('message');

// One-time listener (executes only once)
peer.once('connected', (peerId, url) => {
  console.log('First connection established!');
});
```

### Connect to Peers

```typescript
// Connect to a remote peer
const peerId = await peer.connectToPeer(
  'ws://localhost:20082/umicp',
  { metadata: 'optional' }
);

if (peerId) {
  console.log(`Connected: ${peerId}`);
}
```

### Send Messages

```typescript
// Send to specific peer by ID
peer.sendToPeer('incoming-client-123', envelope);

// Send to specific peer by URL
peer.sendToPeerByUrl('ws://localhost:20082/umicp', envelope);

// Broadcast to ALL peers
peer.broadcast(envelope);

// Broadcast only to incoming OR outgoing
peer.broadcastToType(envelope, 'incoming');
peer.broadcastToType(envelope, 'outgoing');

// Send and wait for response
const response = await peer.sendAndWait('peer-id', envelope, 30000);
```

### Manage Peers

```typescript
// List all peers
const allPeers = peer.getPeers();

// List by type
const incomingPeers = peer.getPeersByType('incoming');
const outgoingPeers = peer.getPeersByType('outgoing');

// Get specific peer
const specificPeer = peer.getPeer('incoming-client-123');

// Search by metadata
const found = peer.findPeerByMetadata('role', 'analyzer');

// Disconnect specific peer
peer.disconnectPeer('peer-id');
peer.disconnectPeerByUrl('ws://localhost:20082/umicp');

// Statistics
const stats = peer.getStats();
console.log(`Total peers: ${stats.totalPeers}`);
console.log(`Incoming: ${stats.incomingConnections}`);
console.log(`Outgoing: ${stats.outgoingConnections}`);

// Complete shutdown
await peer.shutdown();
```

## Types

### PeerConnection

```typescript
interface PeerConnection {
  id: string;                    // Unique identifier
  type: 'incoming' | 'outgoing'; // Connection direction
  url?: string;                  // URL (only for outgoing)
  client: UMICPWebSocketClient;  // Underlying WebSocket client
  metadata: Record<string, any>; // Custom metadata
  connectedAt: Date;             // Connection timestamp
}
```

## Example: 3-Agent Network

```typescript
// Each agent can receive AND connect to others

// Agent Alpha
const alpha = new UMICPWebSocketPeer({
  peerId: 'agent-alpha',
  server: { port: 20081 }
});

alpha.on('message', async (env, peer) => { /* ... */ });

// Agent Beta
const beta = new UMICPWebSocketPeer({
  peerId: 'agent-beta',
  server: { port: 20082 }
});

beta.on('message', async (env, peer) => { /* ... */ });

// Agent Gamma
const gamma = new UMICPWebSocketPeer({
  peerId: 'agent-gamma',
  server: { port: 20083 }
});

gamma.on('message', async (env, peer) => { /* ... */ });

// Create ring topology: alpha → beta → gamma → alpha
await alpha.connectToPeer('ws://localhost:20082/umicp'); // alpha → beta
await beta.connectToPeer('ws://localhost:20083/umicp');  // beta → gamma
await gamma.connectToPeer('ws://localhost:20081/umicp'); // gamma → alpha

// Now each agent has:
// - 1 outgoing connection (initiated by it)
// - 1 incoming connection (initiated by another)
// - Total: all can communicate bidirectionally
```

## Supported Topologies

### 1. Full Mesh
Everyone connects with everyone:

```
     A ←→ B
     ↕ ✗ ↕
     C ←→ D
```

```typescript
await peerA.connectToPeer(urlB);
await peerA.connectToPeer(urlC);
await peerA.connectToPeer(urlD);
// And so on for each peer
```

### 2. Hub and Spoke
One central peer connected to all:

```
     B ← A → C
         ↓
         D
```

```typescript
// On hub (A), only accepts connections
const hub = new UMICPWebSocketPeer({ 
  peerId: 'hub', 
  server: { port: 20081 } 
});

// Spokes connect to hub
await peerB.connectToPeer('ws://hub:20081/umicp');
await peerC.connectToPeer('ws://hub:20081/umicp');
await peerD.connectToPeer('ws://hub:20081/umicp');
```

### 3. Pipeline/Chain
Sequential processing:

```
A → B → C → D
```

```typescript
await peerA.connectToPeer(urlB);
await peerB.connectToPeer(urlC);
await peerC.connectToPeer(urlD);
```

### 4. Hierarchical
Multiple levels:

```
        A
       ↙ ↘
      B   C
     ↙ ↘   ↘
    D   E   F
```

```typescript
await peerA.connectToPeer(urlB);
await peerA.connectToPeer(urlC);
await peerB.connectToPeer(urlD);
await peerB.connectToPeer(urlE);
await peerC.connectToPeer(urlF);
```

## Use Cases

### 1. Collaborative Agent Network
Specialized agents that communicate to solve complex tasks:
- Agent Analyzer (receives data, sends to Processor)
- Agent Processor (processes, sends to Storage)
- Agent Storage (persists, notifies Analyzer)

### 2. Distributed Messaging System
Event broadcast to multiple consumers:
- Producer connects to multiple Workers
- Workers process in parallel
- Results aggregated in Collector

### 3. Service Mesh
Microservices that discover and communicate dynamically:
- Service Discovery
- Load Balancing
- Automatic failover

### 4. Processing Pipeline
Data flow through multiple stages:
- Ingest → Transform → Validate → Store → Notify

## Migrating Existing Code

### From UMICPWebSocketServer/Client to Peer

**Before:**
```typescript
// Server
const server = new UMICPWebSocketServer({
  port: 20081,
  onMessage: async (env, client) => { /* ... */ }
});

// Client
const client = new UMICPWebSocketClient({
  url: 'ws://localhost:20082/umicp',
  onMessage: async (env) => { /* ... */ }
});
```

**After:**
```typescript
const peer = new UMICPWebSocketPeer({
  peerId: 'my-peer',
  
  // Server component
  server: { port: 20081 }
});

// Unified handler using EventEmitter
peer.on('message', async (env, peerConn) => {
  // Same logic, works for incoming AND outgoing
});

// Connect as client
await peer.connectToPeer('ws://localhost:20082/umicp');
```

## Debugging and Monitoring

```typescript
// Real-time statistics
setInterval(() => {
  const stats = peer.getStats();
  console.log(JSON.stringify(stats, null, 2));
}, 10000);

// Log all connections
peer.getPeers().forEach(p => {
  console.log(`${p.id} (${p.type}): ${p.url || 'incoming'}`);
  console.log(`  Connected: ${p.connectedAt}`);
  console.log(`  Metadata:`, p.metadata);
});

// Check specific peer connectivity
const peerConn = peer.getPeer('some-id');
if (peerConn && peerConn.client.isConnected()) {
  console.log('Peer is connected');
}
```

## Complete Example

See `agent-framework/typescript/examples/umicp/multiplexed-agent.ts` for a complete example of 3 interconnected agents with:
- MCP server for external control
- Tools to manage connections
- Message broadcasting
- Real-time statistics

## Performance

- **Minimal overhead**: Multiplexing doesn't add significant latency
- **Scalable**: Tested with 100+ simultaneous peers
- **Auto-reconnect**: Outgoing connections reconnect automatically
- **Compression**: Supports per-message deflate to reduce bandwidth

## Next Steps

1. **Service Discovery**: Integration with service registry
2. **Load Balancing**: Intelligent message distribution
3. **Routing**: Automatic message routing in the network
4. **Encryption**: TLS/SSL for secure connections
5. **Authentication**: Peer validation before accepting connections

