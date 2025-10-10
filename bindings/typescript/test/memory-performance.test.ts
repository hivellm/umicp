/**
 * Memory and Performance Tests
 *
 * Tests cover:
 * - Memory leak detection
 * - Large payload handling
 * - Long-running connections
 * - Resource cleanup verification
 * - Performance benchmarks
 */

import {
  UMICPWebSocketPeer,
  StreamableHTTPClient,
  StreamableHTTPServer,
  Envelope,
  OperationType,
  PayloadType,
  EncodingType
} from '../src/index.js';
import * as http from 'http';

describe('Memory and Performance', () => {
  describe('Memory Leak Detection', () => {
    test('should not leak memory with repeated connections', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'memory-test-hub',
        server: { port: 23001 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      const initialMemory = process.memoryUsage().heapUsed;
      const clients: UMICPWebSocketPeer[] = [];

      // Create and destroy 50 connections
      for (let i = 0; i < 50; i++) {
        const client = new UMICPWebSocketPeer({
          peerId: `leak-test-${i}`
        });
        await client.connectToPeer('ws://localhost:23001/umicp');
        clients.push(client);
      }

      await new Promise(resolve => setTimeout(resolve, 500));

      // Cleanup all
      for (const client of clients) {
        await client.shutdown();
      }

      await new Promise(resolve => setTimeout(resolve, 500));

      // Force garbage collection if available
      if (global.gc) {
        global.gc();
      }

      await new Promise(resolve => setTimeout(resolve, 500));

      const finalMemory = process.memoryUsage().heapUsed;
      const memoryGrowth = finalMemory - initialMemory;

      // Memory growth should be reasonable (< 50MB for 50 connections)
      expect(memoryGrowth).toBeLessThan(50 * 1024 * 1024);

      await peer.shutdown();
    }, 30000);

    test('should cleanup event listeners on disconnect', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'listener-cleanup',
        server: { port: 23002 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      const initialListenerCount = peer.listenerCount('peer:connect');

      // Add multiple listeners
      const handlers = [];
      for (let i = 0; i < 10; i++) {
        const handler = () => {};
        peer.on('peer:connect', handler);
        handlers.push(handler);
      }

      expect(peer.listenerCount('peer:connect')).toBeGreaterThan(initialListenerCount);

      // Remove listeners
      handlers.forEach(h => peer.removeListener('peer:connect', h));

      expect(peer.listenerCount('peer:connect')).toBe(initialListenerCount);

      await peer.shutdown();
    });

    test('should not accumulate dead connections', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'dead-conn-test',
        server: { port: 23003 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      // Create and immediately destroy connections
      for (let i = 0; i < 20; i++) {
        const client = new UMICPWebSocketPeer({
          peerId: `temp-${i}`
        });
        await client.connectToPeer('ws://localhost:23003/umicp');
        await client.shutdown();
        await new Promise(resolve => setTimeout(resolve, 50));
      }

      await new Promise(resolve => setTimeout(resolve, 500));

      const stats = peer.getStats();
      // Should have 0 connections after all shutdowns
      expect(stats.totalPeers).toBe(0);

      await peer.shutdown();
    }, 15000);
  });

  describe('Large Payload Handling', () => {
    test('should handle 1MB envelope', async () => {
      const largeData = 'x'.repeat(1024 * 1024); // 1MB

      const envelope = new Envelope({
        from: 'large-sender',
        to: 'large-receiver',
        operation: OperationType.DATA,
        messageId: 'large-1mb',
        capabilities: {
          'large-data': largeData
        }
      });

      expect(envelope.validate()).toBe(true);

      const serialized = envelope.serialize();
      expect(serialized.length).toBeGreaterThan(1024 * 1024);

      const deserialized = Envelope.deserialize(serialized);
      expect(deserialized.validate()).toBe(true);
      expect(deserialized.getCapabilities()['large-data']).toBe(largeData);
    });

    test('should handle large PayloadHint values', () => {
      const envelope = new Envelope({
        from: 'ml-service',
        to: 'storage',
        operation: OperationType.DATA,
        messageId: 'large-model',
        payloadHint: {
          type: PayloadType.VECTOR,
          size: 500000000, // 500MB
          encoding: EncodingType.FLOAT32,
          count: 125000000 // 125M parameters
        },
        capabilities: {
          'model': 'gpt-4',
          'compression': 'zstd'
        }
      });

      expect(envelope.validate()).toBe(true);

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(deserialized.validate()).toBe(true);
    });

    test('should transmit 5MB envelope over HTTP', async () => {
      const largeData = 'y'.repeat(5 * 1024 * 1024); // 5MB
      let receivedSize = 0;

      const httpServer = new StreamableHTTPServer({
        onMessage: async (envelope, req, res) => {
          receivedSize = envelope.getCapabilities()['data'].length;

          res.writeHead(200);
          res.end(new Envelope({
            from: 'server',
            to: 'client',
            operation: OperationType.ACK,
            messageId: 'ack-large',
          }).serialize());
        }
      });

      const server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      await new Promise<void>(resolve => {
        server.listen(23100, () => resolve());
      });

      const client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:23100',
        path: '/umicp',
        timeout: 10000
      });

      const envelope = new Envelope({
        from: 'client',
        to: 'server',
        operation: OperationType.DATA,
        messageId: 'large-http',
        capabilities: {
          'data': largeData
        }
      });

      await client.send(envelope);

      expect(receivedSize).toBe(5 * 1024 * 1024);

      await new Promise<void>(resolve => {
        server.close(() => resolve());
      });
    }, 20000);

    test('should handle multiple large envelopes sequentially', async () => {
      const sizes = [100000, 500000, 1000000]; // 100KB, 500KB, 1MB
      const results: number[] = [];

      for (const size of sizes) {
        const data = 'z'.repeat(size);
        const envelope = new Envelope({
          from: 'sender',
          to: 'receiver',
          operation: OperationType.DATA,
          messageId: `large-${size}`,
          capabilities: { 'payload': data }
        });

        const serialized = envelope.serialize();
        const deserialized = Envelope.deserialize(serialized);

        results.push(deserialized.getCapabilities()['payload'].length);
      }

      expect(results).toEqual(sizes);
    });
  });

  describe('Performance Benchmarks', () => {
    test('should create 1000 envelopes in < 100ms', () => {
      const start = Date.now();

      for (let i = 0; i < 1000; i++) {
        const envelope = new Envelope({
          from: `sender-${i}`,
          to: `receiver-${i}`,
          operation: OperationType.DATA,
          messageId: `perf-${i}`,
        });
        expect(envelope.validate()).toBe(true);
      }

      const elapsed = Date.now() - start;
      expect(elapsed).toBeLessThan(100);
    });

    test('should serialize 1000 envelopes in < 200ms', () => {
      const envelopes: Envelope[] = [];

      for (let i = 0; i < 1000; i++) {
        envelopes.push(new Envelope({
          from: `sender-${i}`,
          to: `receiver-${i}`,
          operation: OperationType.DATA,
          messageId: `serial-${i}`,
          capabilities: {
            'index': i.toString(),
            'data': 'test-data'
          }
        }));
      }

      const start = Date.now();

      envelopes.forEach(env => {
        const serialized = env.serialize();
        expect(serialized).toBeTruthy();
      });

      const elapsed = Date.now() - start;
      expect(elapsed).toBeLessThan(200);
    });

    test('should deserialize 1000 envelopes in < 200ms', () => {
      const serialized: string[] = [];

      for (let i = 0; i < 1000; i++) {
        const env = new Envelope({
          from: `sender-${i}`,
          to: `receiver-${i}`,
          operation: OperationType.DATA,
          messageId: `deserial-${i}`,
        });
        serialized.push(env.serialize());
      }

      const start = Date.now();

      serialized.forEach(s => {
        const envelope = Envelope.deserialize(s);
        expect(envelope.validate()).toBe(true);
      });

      const elapsed = Date.now() - start;
      expect(elapsed).toBeLessThan(200);
    });

    test('should handle 100 concurrent peer connections', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'perf-hub',
        server: { port: 23004 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      const clients: UMICPWebSocketPeer[] = [];
      const start = Date.now();

      // Connect 100 clients
      const promises = [];
      for (let i = 0; i < 100; i++) {
        const client = new UMICPWebSocketPeer({
          peerId: `perf-client-${i}`
        });
        promises.push(client.connectToPeer('ws://localhost:23004/umicp'));
        clients.push(client);
      }

      await Promise.all(promises);
      const elapsed = Date.now() - start;

      // Should connect 100 clients in reasonable time (< 10 seconds)
      expect(elapsed).toBeLessThan(10000);

      const stats = peer.getStats();
      expect(stats.totalPeers).toBe(100);

      // Cleanup
      for (const client of clients) {
        await client.shutdown();
      }
      await peer.shutdown();
    }, 30000);

    test('should broadcast to 50 peers efficiently', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'broadcast-perf-hub',
        server: { port: 23005 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      const clients: UMICPWebSocketPeer[] = [];

      // Setup 50 clients
      for (let i = 0; i < 50; i++) {
        const client = new UMICPWebSocketPeer({
          peerId: `broadcast-client-${i}`
        });
        await client.connectToPeer('ws://localhost:23005/umicp');
        clients.push(client);
      }

      await new Promise(resolve => setTimeout(resolve, 1000));

      const envelope = new Envelope({
        from: 'broadcast-perf-hub',
        to: 'all',
        operation: OperationType.DATA,
        messageId: 'broadcast-perf',
      });

      const start = Date.now();
      const sent = peer.broadcast(envelope);
      const elapsed = Date.now() - start;

      expect(sent).toBe(50);
      // Broadcasting should be fast (< 100ms)
      expect(elapsed).toBeLessThan(100);

      // Cleanup
      for (const client of clients) {
        await client.shutdown();
      }
      await peer.shutdown();
    }, 30000);
  });

  describe('Resource Cleanup', () => {
    test('should release resources after shutdown', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'cleanup-test',
        server: { port: 23006 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      const client = new UMICPWebSocketPeer({
        peerId: 'cleanup-client'
      });

      await client.connectToPeer('ws://localhost:23006/umicp');
      await new Promise(resolve => setTimeout(resolve, 300));

      const beforeStats = peer.getStats();
      expect(beforeStats.totalPeers).toBeGreaterThan(0);

      await peer.shutdown();

      const afterStats = peer.getStats();
      expect(afterStats.totalPeers).toBe(0);
      expect(afterStats.serverActive).toBe(false);

      await client.shutdown();
    });

    test('should cleanup HTTP server resources', async () => {
      let connectionsReceived = 0;

      const httpServer = new StreamableHTTPServer({
        onConnection: () => {
          connectionsReceived++;
        },
        onMessage: async (envelope, req, res) => {
          res.writeHead(200);
          res.end('ok');
        }
      });

      const server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      await new Promise<void>(resolve => {
        server.listen(23101, () => resolve());
      });

      // Make some requests
      for (let i = 0; i < 5; i++) {
        await fetch('http://localhost:23101/umicp');
      }

      expect(connectionsReceived).toBe(5);

      await new Promise<void>(resolve => {
        server.close(() => resolve());
      });
    });
  });

  describe('Long-Running Stability', () => {
    test('should maintain connection for extended period', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'stable-hub',
        server: { port: 23007 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      const client = new UMICPWebSocketPeer({
        peerId: 'stable-client'
      });

      await client.connectToPeer('ws://localhost:23007/umicp');
      await new Promise(resolve => setTimeout(resolve, 300));

      // Keep connection alive for 5 seconds
      await new Promise(resolve => setTimeout(resolve, 5000));

      const stats = peer.getStats();
      expect(stats.totalPeers).toBe(1);

      // Connection should still work
      const envelope = new Envelope({
        from: 'stable-client',
        to: 'stable-hub',
        operation: OperationType.DATA,
        messageId: 'long-running-test',
      });

      const peers = client.getPeers();
      const sent = client.sendToPeer(peers[0].id, envelope);
      expect(sent).toBe(true);

      await client.shutdown();
      await peer.shutdown();
    }, 10000);

    test('should handle continuous message flow', async () => {
      let messagesReceived = 0;

      const peer = new UMICPWebSocketPeer({
        peerId: 'flow-hub',
        server: { port: 23008 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      peer.on('data', () => {
        messagesReceived++;
      });

      const client = new UMICPWebSocketPeer({
        peerId: 'flow-client'
      });

      await client.connectToPeer('ws://localhost:23008/umicp');
      await new Promise(resolve => setTimeout(resolve, 300));

      // Send 100 messages continuously
      const peers = client.getPeers();
      for (let i = 0; i < 100; i++) {
        const envelope = new Envelope({
          from: 'flow-client',
          to: 'flow-hub',
          operation: OperationType.DATA,
          messageId: `flow-${i}`,
        });
        client.sendToPeer(peers[0].id, envelope);
        await new Promise(resolve => setTimeout(resolve, 10));
      }

      await new Promise(resolve => setTimeout(resolve, 1000));

      expect(messagesReceived).toBeGreaterThan(90); // Allow some tolerance

      await client.shutdown();
      await peer.shutdown();
    }, 15000);
  });
});

