/**
 * Load and Stress Tests
 *
 * Tests cover:
 * - High throughput scenarios
 * - Sustained load handling
 * - Concurrent operations
 * - Resource exhaustion resistance
 * - Performance under stress
 * - Degradation gracefully
 */

import {
  UMICPWebSocketPeer,
  StreamableHTTPServer,
  StreamableHTTPClient,
  Envelope,
  OperationType
} from '../src/index.js';
import * as http from 'http';

describe('Load and Stress Tests', () => {
  describe('High Throughput', () => {
    test('should handle 1000 messages in rapid succession', async () => {
      let receivedCount = 0;

      const peer = new UMICPWebSocketPeer({
        peerId: 'throughput-hub',
        server: { port: 25001 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      peer.on('data', () => {
        receivedCount++;
      });

      const client = new UMICPWebSocketPeer({
        peerId: 'throughput-client'
      });

      await client.connectToPeer('ws://localhost:25001/umicp');
      await new Promise(resolve => setTimeout(resolve, 300));

      const start = Date.now();
      const peers = client.getPeers();

      // Send 1000 messages rapidly
      for (let i = 0; i < 1000; i++) {
        const envelope = new Envelope({
          from: 'throughput-client',
          to: 'throughput-hub',
          operation: OperationType.DATA,
          messageId: `rapid-${i}`,
        });
        client.sendToPeer(peers[0].id, envelope);
      }

      const sendTime = Date.now() - start;
      expect(sendTime).toBeLessThan(5000); // Should send in < 5 seconds

      // Wait for all messages to be received
      await new Promise(resolve => setTimeout(resolve, 2000));

      // Allow some tolerance for message loss (network/timing)
      expect(receivedCount).toBeGreaterThan(900);

      await client.shutdown();
      await peer.shutdown();
    }, 20000);

    test('should maintain performance under sustained load', async () => {
      let messageCount = 0;
      const durations: number[] = [];

      const peer = new UMICPWebSocketPeer({
        peerId: 'sustained-hub',
        server: { port: 25002 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      peer.on('data', () => {
        messageCount++;
      });

      const client = new UMICPWebSocketPeer({
        peerId: 'sustained-client'
      });

      await client.connectToPeer('ws://localhost:25002/umicp');
      await new Promise(resolve => setTimeout(resolve, 300));

      const peers = client.getPeers();

      // Send messages continuously for 5 seconds
      const endTime = Date.now() + 5000;
      let msgId = 0;

      while (Date.now() < endTime) {
        const start = Date.now();

        const envelope = new Envelope({
          from: 'sustained-client',
          to: 'sustained-hub',
          operation: OperationType.DATA,
          messageId: `sustained-${msgId++}`,
        });

        client.sendToPeer(peers[0].id, envelope);

        durations.push(Date.now() - start);
        await new Promise(resolve => setTimeout(resolve, 10));
      }

      // Calculate average send time
      const avgDuration = durations.reduce((a, b) => a + b, 0) / durations.length;
      expect(avgDuration).toBeLessThan(50); // Average < 50ms

      expect(messageCount).toBeGreaterThan(100);

      await client.shutdown();
      await peer.shutdown();
    }, 15000);

    test('should handle burst traffic', async () => {
      let receivedCount = 0;

      const peer = new UMICPWebSocketPeer({
        peerId: 'burst-hub',
        server: { port: 25003 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      peer.on('data', () => {
        receivedCount++;
      });

      const client = new UMICPWebSocketPeer({
        peerId: 'burst-client'
      });

      await client.connectToPeer('ws://localhost:25003/umicp');
      await new Promise(resolve => setTimeout(resolve, 300));

      const peers = client.getPeers();

      // Send 3 bursts of 100 messages
      for (let burst = 0; burst < 3; burst++) {
        for (let i = 0; i < 100; i++) {
          const envelope = new Envelope({
            from: 'burst-client',
            to: 'burst-hub',
            operation: OperationType.DATA,
            messageId: `burst-${burst}-${i}`,
          });
          client.sendToPeer(peers[0].id, envelope);
        }

        // Pause between bursts
        await new Promise(resolve => setTimeout(resolve, 500));
      }

      await new Promise(resolve => setTimeout(resolve, 1000));
      expect(receivedCount).toBeGreaterThan(250);

      await client.shutdown();
      await peer.shutdown();
    }, 15000);
  });

  describe('Concurrent Connections', () => {
    test('should handle 200 simultaneous connections', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'concurrent-hub',
        server: { port: 25004 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      const clients: UMICPWebSocketPeer[] = [];

      // Connect 200 clients
      for (let i = 0; i < 200; i++) {
        const client = new UMICPWebSocketPeer({
          peerId: `concurrent-${i}`
        });

        const promise = client.connectToPeer('ws://localhost:25004/umicp');
        clients.push(client);

        // Small delay to avoid overwhelming
        if (i % 20 === 0) {
          await new Promise(resolve => setTimeout(resolve, 100));
        }
      }

      await new Promise(resolve => setTimeout(resolve, 2000));

      const stats = peer.getStats();
      expect(stats.totalPeers).toBeGreaterThan(180); // Allow some failures

      // Cleanup
      for (const client of clients) {
        await client.shutdown();
      }
      await peer.shutdown();
    }, 60000);

    test('should handle concurrent broadcasts', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'concurrent-broadcast-hub',
        server: { port: 25005 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      const clients: UMICPWebSocketPeer[] = [];
      const receiveCounts = new Map<string, number>();

      // Setup 30 clients
      for (let i = 0; i < 30; i++) {
        const client = new UMICPWebSocketPeer({
          peerId: `broadcast-client-${i}`
        });

        receiveCounts.set(`broadcast-client-${i}`, 0);

        client.on('data', () => {
          receiveCounts.set(`broadcast-client-${i}`, (receiveCounts.get(`broadcast-client-${i}`) || 0) + 1);
        });

        await client.connectToPeer('ws://localhost:25005/umicp');
        clients.push(client);
      }

      await new Promise(resolve => setTimeout(resolve, 1000));

      // Send 10 broadcasts
      for (let i = 0; i < 10; i++) {
        const envelope = new Envelope({
          from: 'concurrent-broadcast-hub',
          to: 'all',
          operation: OperationType.DATA,
          messageId: `concurrent-broadcast-${i}`,
        });

        peer.broadcast(envelope);
        await new Promise(resolve => setTimeout(resolve, 50));
      }

      await new Promise(resolve => setTimeout(resolve, 1000));

      // Most clients should have received most broadcasts
      const totalReceived = Array.from(receiveCounts.values()).reduce((a, b) => a + b, 0);
      expect(totalReceived).toBeGreaterThan(250); // 30 clients * 10 broadcasts = 300

      // Cleanup
      for (const client of clients) {
        await client.shutdown();
      }
      await peer.shutdown();
    }, 30000);
  });

  describe('HTTP Load Testing', () => {
    test('should handle 100 concurrent HTTP requests', async () => {
      let requestCount = 0;

      const httpServer = new StreamableHTTPServer({
        onMessage: async (envelope, req, res) => {
          requestCount++;

          res.writeHead(200);
          res.end(new Envelope({
            from: 'server',
            to: envelope.getFrom(),
            operation: OperationType.ACK,
            messageId: `ack-${envelope.getMessageId()}`
          }).serialize());
        }
      });

      const server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      await new Promise<void>(resolve => {
        server.listen(25100, () => resolve());
      });

      const client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:25100',
        path: '/umicp',
        timeout: 10000
      });

      // Send 100 concurrent requests
      const promises: Promise<Envelope | null>[] = [];
      for (let i = 0; i < 100; i++) {
        const envelope = new Envelope({
          from: `client-${i}`,
          to: 'server',
          operation: OperationType.DATA,
          messageId: `http-load-${i}`,
        });

        promises.push(client.send(envelope).catch(() => null as Envelope | null));
      }

      await Promise.all(promises);

      expect(requestCount).toBeGreaterThan(90); // Allow some failures

      await new Promise<void>(resolve => {
        server.close(() => resolve());
      });
    }, 30000);

    test('should maintain latency under HTTP load', async () => {
      const latencies: number[] = [];

      const httpServer = new StreamableHTTPServer({
        onMessage: async (envelope, req, res) => {
          res.writeHead(200);
          res.end(new Envelope({
            from: 'server',
            to: envelope.getFrom(),
            operation: OperationType.ACK,
            messageId: 'ack'
          }).serialize());
        }
      });

      const server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      await new Promise<void>(resolve => {
        server.listen(25101, () => resolve());
      });

      const client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:25101',
        path: '/umicp'
      });

      // Measure latency for 50 requests
      for (let i = 0; i < 50; i++) {
        const start = Date.now();

        const envelope = new Envelope({
          from: 'client',
          to: 'server',
          operation: OperationType.DATA,
          messageId: `latency-${i}`,
        });

        await client.send(envelope);
        latencies.push(Date.now() - start);
      }

      const avgLatency = latencies.reduce((a, b) => a + b, 0) / latencies.length;
      const maxLatency = Math.max(...latencies);

      expect(avgLatency).toBeLessThan(100); // Average < 100ms
      expect(maxLatency).toBeLessThan(500); // Max < 500ms

      await new Promise<void>(resolve => {
        server.close(() => resolve());
      });
    }, 30000);
  });

  describe('Resource Exhaustion Resistance', () => {
    test('should handle connection churn', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'churn-hub',
        server: { port: 25006 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      // Repeatedly connect and disconnect
      for (let i = 0; i < 50; i++) {
        const client = new UMICPWebSocketPeer({
          peerId: `churn-${i}`
        });

        await client.connectToPeer('ws://localhost:25006/umicp');
        await new Promise(resolve => setTimeout(resolve, 50));
        await client.shutdown();
      }

      await new Promise(resolve => setTimeout(resolve, 500));

      const stats = peer.getStats();
      expect(stats.totalPeers).toBe(0); // All should be disconnected

      await peer.shutdown();
    }, 30000);

    test('should not accumulate pending operations', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'pending-test',
        server: { port: 25007 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      const client = new UMICPWebSocketPeer({
        peerId: 'pending-client'
      });

      await client.connectToPeer('ws://localhost:25007/umicp');
      await new Promise(resolve => setTimeout(resolve, 300));

      const peers = client.getPeers();

      // Queue many operations rapidly
      for (let i = 0; i < 1000; i++) {
        const envelope = new Envelope({
          from: 'pending-client',
          to: 'pending-test',
          operation: OperationType.DATA,
          messageId: `pending-${i}`,
        });
        client.sendToPeer(peers[0].id, envelope);
      }

      await new Promise(resolve => setTimeout(resolve, 2000));

      // Should complete without hanging
      await client.shutdown();
      await peer.shutdown();
    }, 15000);
  });

  describe('Stress Scenarios', () => {
    test('should recover from rapid connect/disconnect cycles', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'stress-hub',
        server: { port: 25008 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      // 20 cycles of rapid connect/disconnect
      for (let cycle = 0; cycle < 20; cycle++) {
        const clients: UMICPWebSocketPeer[] = [];

        // Connect 5 clients
        for (let i = 0; i < 5; i++) {
          const client = new UMICPWebSocketPeer({
            peerId: `stress-${cycle}-${i}`
          });
          await client.connectToPeer('ws://localhost:25008/umicp');
          clients.push(client);
        }

        await new Promise(resolve => setTimeout(resolve, 100));

        // Disconnect all
        for (const client of clients) {
          await client.shutdown();
        }
      }

      await new Promise(resolve => setTimeout(resolve, 500));

      // Peer should still be functional
      const testClient = new UMICPWebSocketPeer({
        peerId: 'stress-verify'
      });
      await testClient.connectToPeer('ws://localhost:25008/umicp');

      const stats = peer.getStats();
      expect(stats.totalPeers).toBe(1);

      await testClient.shutdown();
      await peer.shutdown();
    }, 45000);

    test('should handle message flooding', async () => {
      let messageCount = 0;

      const peer = new UMICPWebSocketPeer({
        peerId: 'flood-hub',
        server: { port: 25009 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      peer.on('data', () => {
        messageCount++;
      });

      const client = new UMICPWebSocketPeer({
        peerId: 'flood-client'
      });

      await client.connectToPeer('ws://localhost:25009/umicp');
      await new Promise(resolve => setTimeout(resolve, 300));

      const peers = client.getPeers();

      // Flood with messages (no delay)
      for (let i = 0; i < 5000; i++) {
        const envelope = new Envelope({
          from: 'flood-client',
          to: 'flood-hub',
          operation: OperationType.DATA,
          messageId: `flood-${i}`,
        });
        client.sendToPeer(peers[0].id, envelope);
      }

      await new Promise(resolve => setTimeout(resolve, 5000));

      // Should receive significant portion despite flooding
      expect(messageCount).toBeGreaterThan(1000);

      await client.shutdown();
      await peer.shutdown();
    }, 20000);
  });

  describe('Degradation Scenarios', () => {
    test('should degrade gracefully under extreme load', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'degradation-hub',
        server: { port: 25010 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      const clients: UMICPWebSocketPeer[] = [];

      // Connect many clients
      for (let i = 0; i < 100; i++) {
        const client = new UMICPWebSocketPeer({
          peerId: `degradation-${i}`
        });
        await client.connectToPeer('ws://localhost:25010/umicp');
        clients.push(client);

        if (i % 10 === 0) {
          await new Promise(resolve => setTimeout(resolve, 100));
        }
      }

      await new Promise(resolve => setTimeout(resolve, 1000));

      // Each client sends multiple messages
      for (let idx = 0; idx < clients.length; idx++) {
        const client = clients[idx];
        const peers = client.getPeers();
        if (peers.length > 0) {
          for (let i = 0; i < 5; i++) {
            const envelope = new Envelope({
              from: `degradation-${idx}`,
              to: 'degradation-hub',
              operation: OperationType.DATA,
              messageId: `degrade-${i}`,
            });
            client.sendToPeer(peers[0].id, envelope);
          }
        }
      }

      await new Promise(resolve => setTimeout(resolve, 2000));

      // System should still respond
      const stats = peer.getStats();
      expect(stats.totalPeers).toBeGreaterThan(80);

      // Cleanup
      for (const client of clients) {
        await client.shutdown();
      }
      await peer.shutdown();
    }, 60000);
  });
});

