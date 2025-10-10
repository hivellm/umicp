# Auto-Protocol: Automatic UMICP Protocol

## Overview

The `UMICPWebSocketPeer` class now implements **automatic UMICP protocol handling**, eliminating the need to manually implement:
- Hello messages
- Handshake ACKs
- Handshake timeout management
- Distinction between protocol messages and application messages

## Problem Solved

### ❌ Before: Manual Protocol Implementation

```typescript
const peer = new UMICPWebSocketPeer({
  peerId: 'my-agent',
  server: { port: 20081 }
});

// Had to implement EVERYTHING manually:
peer.on('message', async (envelope, peerConn) => {
  const operation = envelope.getOperation();
  
  switch (operation) {
    case OperationType.CONTROL:
      // Manually extract peer info
      // Manually send ACK
      // Manually mark handshake as complete
      break;
      
    case OperationType.ACK:
      // Manually process ACK
      // Manually mark handshake as complete
      break;
      
    case OperationType.DATA:
      // Finally process application data
      break;
  }
});

peer.on('peer:connect', (peerConn) => {
  // Manually send hello message
  const hello = new Envelope({
    from: 'my-agent',
    to: 'peer',
    operation: OperationType.CONTROL,
    messageId: `hello-${Date.now()}`,
    capabilities: {
      /* ... */
    }
  });
  peer.sendToPeer(peerConn.id, hello);
});
```

**Problems:**
- Repetitive code in every agent
- Easy to forget handshake steps
- Difficult to maintain consistency
- Mixes protocol with application logic

### ✅ After: Automatic Protocol

```typescript
const peer = new UMICPWebSocketPeer({
  peerId: 'my-agent',
  server: { port: 20081 },
  
  // Metadata sent in hello message automatically
  metadata: {
    agent_type: 'analyzer',
    version: '1.0.0',
  },
  
  // Automatic protocol (default: true)
  autoProtocol: true,
  handshakeTimeout: 10000,
});

// Only handles application messages (DATA)!
peer.on('data', async (envelope, peerConn) => {
  // Protocol already handled automatically
  // Just process application data
  const data = envelope.getCapabilities();
  console.log('Application data:', data);
});

// Notification when peer is ready (handshake complete)
peer.on('peer:ready', (peerConn, peerInfo) => {
  console.log(`Peer ${peerInfo.peerId} ready!`);
  console.log('Metadata:', peerInfo.metadata);
  // Now can send application messages
});
```

**Benefits:**
- ✅ Zero protocol boilerplate
- ✅ Automatic handshake
- ✅ Code focused on application logic
- ✅ Guaranteed consistency
- ✅ Clear separation: protocol vs. application

## How It Works

### 1. Peer Configuration

```typescript
interface UMICPWebSocketPeerOptions {
  peerId: string;
  server?: { /* ... */ };
  
  // New: metadata for hello message
  metadata?: Record<string, string>;
  
  // New: enable/disable automatic protocol
  autoProtocol?: boolean; // default: true
  
  // New: handshake timeout
  handshakeTimeout?: number; // default: 10000ms
}
```

### 2. Automatic Handshake Flow

**Outgoing Connection (you connect to another peer):**

```
[You]                          [Remote Peer]
   |                                  |
   |-- Socket Connect --------------->|
   |                                  |
   |-- CONTROL (hello) -------------->|
   |    {                             |
   |      peerId: "agent-a",          |
   |      meta_version: "1.0.0",      |
   |      ...                         |
   |    }                             |
   |                                  |
   |<- ACK (handshake_complete) ------|
   |    {                             |
   |      status: "handshake_complete"|
   |      peerId: "agent-b",          |
   |      meta_type: "analyzer",      |
   |      ...                         |
   |    }                             |
   |                                  |
   |-- Emit 'peer:ready' -----------  |
   |                                  |
```

**Incoming Connection (another peer connects to you):**

```
[Remote Peer]                   [You]
   |                                  |
   |-- Socket Connect --------------->|
   |                                  |
   |<- CONTROL (hello) ---------------|
   |                                  |
   |-- CONTROL (hello) -------------->|
   |                                  |
   |<- ACK (handshake_complete) ------|
   |                                  |
   |-- ACK (handshake_complete) ----->|
   |                                  |
   |                       Emit 'peer:ready'
```

### 3. Emitted Events

```typescript
// Event sequence when connecting:

// 1. Socket connected (low level)
peer.emit('peer:connect', peerConnection);

// 2. Handshake in progress (automatic)
//    - Sends/receives CONTROL messages
//    - Sends/receives ACKs

// 3. Handshake complete!
peer.emit('peer:ready', peerConnection, peerInfo);

// 4. Now can exchange application messages
peer.on('data', (envelope, peerConn) => {
  // Application DATA messages
});
```

### 4. PeerInfo Structure

```typescript
interface PeerInfo {
  peerId: string;                      // Remote peer ID
  metadata: Record<string, string>;    // Peer metadata
  capabilities: Record<string, string>;// Complete capabilities
  handshakeCompletedAt: Date;         // Timestamp
}

// Example of received peerInfo:
{
  peerId: "agent-b",
  metadata: {
    agent_type: "analyzer",
    version: "2.0.0",
    environment: "production"
  },
  capabilities: {
    peerId: "agent-b",
    meta_agent_type: "analyzer",
    meta_version: "2.0.0",
    // ...
  },
  handshakeCompletedAt: Date(2025-10-09T...)
}
```

## Complete API

### Protocol-Related Events

| Event | When Emitted | Arguments | Usage |
|-------|-------------|-----------|-------|
| `peer:connect` | Socket connected | `peerConnection` | Physical connection established |
| `peer:ready` | Handshake complete | `peerConnection`, `peerInfo` | Peer ready for communication |
| `peer:disconnect` | Disconnected | `peerConnection` | Resource cleanup |
| `peer:handshake-timeout` | Handshake timeout | `peerConnection` | Protocol error |
| `data` | DATA message received | `envelope`, `peerConnection` | **Main: application data** |
| `message` | Any message | `envelope`, `peerConnection` | Debug/logging |

### Configuration

```typescript
const peer = new UMICPWebSocketPeer({
  peerId: 'my-agent',
  
  server: {
    port: 20081,
    path: '/umicp',
  },
  
  // Metadata sent automatically in hello
  metadata: {
    agent_type: 'worker',
    version: '1.0.0',
    environment: 'production',
    region: 'us-east-1',
  },
  
  // Enable automatic protocol (default)
  autoProtocol: true,
  
  // Handshake timeout
  handshakeTimeout: 10000, // 10 seconds
});
```

### Send Application Messages

```typescript
// Wait for peer to be ready
peer.on('peer:ready', (peerConn, peerInfo) => {
  console.log(`Connected to ${peerInfo.peerId}`);
  
  // Now can send DATA messages
  const dataEnvelope = new Envelope({
    from: peer.peerId,
    to: peerInfo.peerId,
    operation: OperationType.DATA,
    messageId: `msg-${Date.now()}`,
    capabilities: {
      action: 'process_data',
      payload: JSON.stringify({ /* ... */ }),
    }
  });
  
  peer.sendToPeer(peerConn.id, dataEnvelope);
});

// Receive DATA messages
peer.on('data', async (envelope, peerConn) => {
  const caps = envelope.getCapabilities();
  
  switch (caps['action']) {
    case 'process_data':
      const payload = JSON.parse(caps['payload']);
      await processData(payload);
      break;
      
    case 'broadcast_message':
      console.log('Broadcast:', caps['message']);
      break;
  }
});
```

## Disable Automatic Protocol

If you need manual protocol control (not recommended):

```typescript
const peer = new UMICPWebSocketPeer({
  peerId: 'my-agent',
  server: { port: 20081 },
  
  // Disable automatic protocol
  autoProtocol: false,
});

// Now you have to implement everything manually
peer.on('data', (envelope, peerConn) => {
  // Receives ALL operation types
  const operation = envelope.getOperation();
  
  switch (operation) {
    case OperationType.CONTROL:
      // Implement manual handshake
      break;
    case OperationType.ACK:
      // Implement manual ACK
      break;
    case OperationType.DATA:
      // Application data
      break;
  }
});
```

## Usage Examples

### Example 1: Worker Pool

```typescript
// Hub receives workers
const hub = new UMICPWebSocketPeer({
  peerId: 'hub',
  server: { port: 20080 },
  metadata: {
    role: 'coordinator',
    max_workers: '10',
  }
});

const workers: Map<string, PeerInfo> = new Map();

hub.on('peer:ready', (peerConn, peerInfo) => {
  if (peerInfo.metadata.role === 'worker') {
    workers.set(peerConn.id, peerInfo);
    console.log(`Worker ${peerInfo.peerId} registered`);
  }
});

hub.on('data', async (envelope, peerConn) => {
  const caps = envelope.getCapabilities();
  
  if (caps['action'] === 'request_work') {
    // Distribute work
    assignWork(peerConn.id);
  }
});
```

### Example 2: Service Mesh

```typescript
// Each service is a peer
const service = new UMICPWebSocketPeer({
  peerId: 'payment-service',
  server: { port: 20085 },
  metadata: {
    service_type: 'payment',
    version: '2.1.0',
    health: 'healthy',
  }
});

// Connect to other services
await service.connectToPeer('ws://user-service:20081/umicp');
await service.connectToPeer('ws://order-service:20082/umicp');

// Automatically exchange metadata and capabilities
service.on('peer:ready', (peerConn, peerInfo) => {
  console.log(`Connected to ${peerInfo.metadata.service_type}`);
  
  // Discover service capabilities
  if (peerInfo.metadata.service_type === 'order') {
    // Can process orders
  }
});
```

### Example 3: Debugging

```typescript
const peer = new UMICPWebSocketPeer({
  peerId: 'debug-agent',
  server: { port: 20090 },
  metadata: {
    debug: 'true',
  }
});

// See all messages (including protocol)
peer.on('message', (envelope, peerConn) => {
  const op = OperationType[envelope.getOperation()];
  console.log(`[${op}] from ${envelope.getFrom()}:`, envelope.getCapabilities());
});

// See when handshake fails
peer.on('peer:handshake-timeout', (peerConn) => {
  console.warn(`Handshake timeout: ${peerConn.id}`);
  // Investigate issue
});
```

## Performance

- **Minimal overhead**: Hello + ACK = 2 extra messages per connection
- **Latency**: ~10-50ms for complete handshake
- **Default timeout**: 10 seconds (configurable)
- **Memory**: ~200 bytes extra per peer to store peerInfo

## Compatibility

- ✅ Compatible with peers that implement protocol manually
- ✅ Compatible with older UMICP versions
- ✅ Can be disabled (`autoProtocol: false`)

## Migration

### From Manual Implementation

```typescript
// BEFORE: 50+ lines of protocol code
peer.on('message', async (envelope, peerConn) => {
  switch (envelope.getOperation()) {
    case OperationType.CONTROL:
      // 20 lines of code
      break;
    case OperationType.ACK:
      // 15 lines of code
      break;
    case OperationType.DATA:
      // Finally data!
      break;
  }
});

// AFTER: 3 lines
peer.on('data', async (envelope, peerConn) => {
  // Straight to data!
});
```

## Conclusion

The automatic protocol:
- ✅ Eliminates 90% of boilerplate code
- ✅ Guarantees consistent implementation
- ✅ Facilitates debugging (clear events)
- ✅ Allows focus on application logic
- ✅ Maintains flexibility (can be disabled)

**Recommendation:** Always use `autoProtocol: true` (default) unless you have a specific need to control the protocol manually.

