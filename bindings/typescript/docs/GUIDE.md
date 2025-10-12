# UMICP TypeScript SDK - Technical Guide

> **📝 TECHNICAL GUIDE**

**Version**: 1.0.0  
**Last Updated**: October 11, 2025

---

## 🚀 Quick Start

### Installation

```bash
npm install @hivellm/umicp-core
```

### Basic Usage

```typescript
import { Envelope, OperationType } from '@hivellm/umicp-core';

// Create envelope
const envelope = new Envelope({
  from: 'client-001',
  to: 'server-001',
  operation: OperationType.DATA,
  payload: Buffer.from('Hello!')
});

// Serialize
const json = JSON.stringify(envelope);
```

---

## 📝 Core API

### Envelope
```typescript
import { Envelope, OperationType, PayloadType } from '@hivellm/umicp-core';

const envelope = new Envelope({
  from: 'client',
  to: 'server',
  operation: OperationType.DATA,
  payload: Buffer.from('data'),
  payloadType: PayloadType.BINARY
});
```

### Matrix
```typescript
import { Matrix } from '@hivellm/umicp-core';

const a = [1.0, 2.0, 3.0];
const b = [4.0, 5.0, 6.0];

const dot = Matrix.dotProduct(a, b);
const cos = Matrix.cosineSimilarity(a, b);
const norm = Matrix.normalize(a);
```

---

## 🌐 WebSocket Transport

### Client
```typescript
import { WebSocketClient } from '@hivellm/umicp-core';

const client = new WebSocketClient('ws://localhost:8080');

client.on('connected', () => {
  console.log('Connected!');
});

client.on('message', (envelope) => {
  console.log('Received:', envelope.from);
});

await client.connect();
await client.send(envelope);
```

### Multiplexed Peer
```typescript
import { WebSocketPeer } from '@hivellm/umicp-core';

const peer = new WebSocketPeer({
  peerId: 'peer-001',
  serverHost: '0.0.0.0',
  serverPort: 8080,
  autoHandshake: true
});

peer.on('peerReady', (peerId, info) => {
  console.log('Peer ready:', peerId);
});

await peer.start();
await peer.connectToPeer('ws://localhost:8081');
await peer.sendToPeer('peer-002', envelope);
```

---

## 🔥 HTTP/2 Streaming

```typescript
import { StreamableHTTP } from '@hivellm/umicp-core';

const http = new StreamableHTTP('http://localhost:8080');

// POST request
const response = await http.post('/api/messages', envelope);

// GET request
const data = await http.get('/api/messages/123');
```

---

## 💡 Best Practices

### Error Handling
```typescript
try {
  await client.connect();
} catch (error) {
  console.error('Connection failed:', error);
}
```

### TypeScript Strict
```typescript
// Always enabled
{
  "compilerOptions": {
    "strict": true
  }
}
```

---

## 📚 Resources

- [README.md](./README.md)
- [STATUS.md](./STATUS.md)
- [REVIEWS.md](./REVIEWS.md)
- [Examples](../examples/)

---

*Last Updated: October 11, 2025*

