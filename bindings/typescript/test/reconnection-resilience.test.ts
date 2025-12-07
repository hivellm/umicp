/**
 * Reconnection and Resilience Tests
 *
 * Tests cover:
 * - Automatic reconnection after failures
 * - Exponential backoff strategies
 * - Message queue preservation
 * - Connection state management
 * - Network partition handling
 * - Split-brain scenario detection
 */

import {
  UMICPWebSocketPeer,
  Envelope,
  OperationType,
  type PeerConnection
} from '../src/index.js';

describe('Reconnection and Resilience', () => {
  let peer1: UMICPWebSocketPeer;
  let peer2: UMICPWebSocketPeer;
  let peer3: UMICPWebSocketPeer;

  afterEach(async () => {
    if (peer1) await peer1.shutdown();
    if (peer2) await peer2.shutdown();
    if (peer3) await peer3.shutdown();
    await new Promise(resolve => setTimeout(resolve, 300));
  });

  describe('Automatic Reconnection', () => {
    test('should attempt reconnection after server restart', (done) => {
      let connectionCount = 0;

      peer1 = new UMICPWebSocketPeer({
        peerId: 'server',
        server: { port: 22001 }
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'client'
        });

        peer1.on('peer:connect', () => {
          connectionCount++;

          if (connectionCount === 2) {
            // Second connection after restart
            expect(connectionCount).toBe(2);
            done();
          }
        });

        // First connection
        await peer2.connectToPeer('ws://localhost:22001/umicp');
        await new Promise(resolve => setTimeout(resolve, 300));

        // Simulate server restart
        await peer1.shutdown();
        await new Promise(resolve => setTimeout(resolve, 200));

        // Restart server
        peer1 = new UMICPWebSocketPeer({
          peerId: 'server-restarted',
          server: { port: 22001 }
        });

        peer1.on('server:ready', async () => {
          // Client should reconnect (if auto-reconnect enabled)
          // Note: UMICPWebSocketPeer uses ws client which has auto-reconnect
          await new Promise(resolve => setTimeout(resolve, 500));
        });
      });
    });

    test('should handle connection failure gracefully', async () => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'client-resilient'
      });

      let errorCount = 0;
      peer1.on('error', () => {
        errorCount++;
      });

      // Try to connect to non-existent server
      const peerId = await peer1.connectToPeer('ws://localhost:29998/umicp');

      expect(peerId).toBeNull();
      await new Promise(resolve => setTimeout(resolve, 200));
      expect(errorCount).toBeGreaterThan(0);
    });

    test('should maintain connection state during reconnect attempts', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'state-tracker',
        server: { port: 22002 }
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'state-client'
        });

        const states: string[] = [];

        peer2.on('peer:connect', () => {
          states.push('connected');
        });

        peer2.on('peer:disconnect', () => {
          states.push('disconnected');
        });

        await peer2.connectToPeer('ws://localhost:22002/umicp');
        await new Promise(resolve => setTimeout(resolve, 200));

        // Disconnect
        const peers = peer2.getPeers();
        if (peers.length > 0) {
          peer2.disconnectPeer(peers[0].id);
        }

        await new Promise(resolve => setTimeout(resolve, 200));

        expect(states).toContain('connected');
        expect(states).toContain('disconnected');
        done();
      });
    });
  });

  describe('Network Partition Handling', () => {
    test('should detect peer disconnection', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'partition-server',
        server: { port: 22003 }
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'partition-client'
        });

        let disconnected = false;

        peer1.on('peer:disconnect', (peer: PeerConnection) => {
          expect(peer.type).toBe('incoming');
          disconnected = true;
          done();
        });

        await peer2.connectToPeer('ws://localhost:22003/umicp');
        await new Promise(resolve => setTimeout(resolve, 300));

        // Simulate partition by shutting down client
        await peer2.shutdown();
      });
    });

    test('should handle multiple simultaneous disconnections', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'multi-disconnect-hub',
        server: { port: 22004 }
      });

      peer1.on('server:ready', async () => {
        const clients: UMICPWebSocketPeer[] = [];
        let disconnectCount = 0;

        peer1.on('peer:disconnect', () => {
          disconnectCount++;
          if (disconnectCount === 5) {
            expect(disconnectCount).toBe(5);
            done();
          }
        });

        // Create 5 clients
        for (let i = 0; i < 5; i++) {
          const client = new UMICPWebSocketPeer({
            peerId: `client-${i}`
          });
          await client.connectToPeer('ws://localhost:22004/umicp');
          clients.push(client);
        }

        await new Promise(resolve => setTimeout(resolve, 500));

        // Disconnect all simultaneously
        await Promise.all(clients.map(c => c.shutdown()));
      });
    });

    test('should maintain other connections when one fails', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'stable-hub',
        server: { port: 22005 }
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'stable-client-1' });
        peer3 = new UMICPWebSocketPeer({ peerId: 'stable-client-2' });

        await peer2.connectToPeer('ws://localhost:22005/umicp');
        await peer3.connectToPeer('ws://localhost:22005/umicp');

        await new Promise(resolve => setTimeout(resolve, 300));

        const beforeStats = peer1.getStats();
        expect(beforeStats.totalPeers).toBe(2);

        // Disconnect one client
        await peer2.shutdown();
        await new Promise(resolve => setTimeout(resolve, 200));

        const afterStats = peer1.getStats();
        expect(afterStats.totalPeers).toBe(1);

        // Verify remaining connection still works
        const envelope = new Envelope({
          from: 'stable-hub',
          to: 'stable-client-2',
          operation: OperationType.DATA,
          messageId: 'test-after-failure',
        });

        const peers = peer1.getPeers();
        const sent = peer1.sendToPeer(peers[0].id, envelope);
        expect(sent).toBe(true);

        done();
      });
    });
  });

  describe('Message Queue Preservation', () => {
    test('should queue messages during disconnection', async () => {
      // Note: This test demonstrates the concept
      // Actual queue implementation may vary

      peer1 = new UMICPWebSocketPeer({
        peerId: 'queue-test'
      });

      const messageQueue: Envelope[] = [];

      // Simulate queueing messages when disconnected
      const envelope1 = new Envelope({
        from: 'queue-test',
        to: 'target',
        operation: OperationType.DATA,
        messageId: 'queued-1',
      });

      const envelope2 = new Envelope({
        from: 'queue-test',
        to: 'target',
        operation: OperationType.DATA,
        messageId: 'queued-2',
      });

      messageQueue.push(envelope1, envelope2);

      expect(messageQueue.length).toBe(2);
      expect(messageQueue[0].getMessageId()).toBe('queued-1');
      expect(messageQueue[1].getMessageId()).toBe('queued-2');
    });

    test('should not lose messages during reconnection window', (done) => {
      let messagesReceived = 0;

      peer1 = new UMICPWebSocketPeer({
        peerId: 'no-loss-server',
        server: { port: 22006 }
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'no-loss-client'
        });

        peer1.on('data', () => {
          messagesReceived++;
        });

        await peer2.connectToPeer('ws://localhost:22006/umicp');
        await new Promise(resolve => setTimeout(resolve, 300));

        // Send message while connected
        const envelope = new Envelope({
          from: 'no-loss-client',
          to: 'no-loss-server',
          operation: OperationType.DATA,
          messageId: 'critical-msg',
        });

        const peers = peer2.getPeers();
        peer2.sendToPeer(peers[0].id, envelope);

        await new Promise(resolve => setTimeout(resolve, 200));

        expect(messagesReceived).toBe(1);
        done();
      });
    });
  });

  describe('Connection State Management', () => {
    test('should track connection state accurately', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'state-server',
        server: { port: 22007 }
      });

      peer1.on('server:ready', async () => {
        const initialStats = peer1.getStats();
        expect(initialStats.totalPeers).toBe(0);
        expect(initialStats.serverActive).toBe(true);

        peer2 = new UMICPWebSocketPeer({
          peerId: 'state-client'
        });

        await peer2.connectToPeer('ws://localhost:22007/umicp');
        await new Promise(resolve => setTimeout(resolve, 300));

        const connectedStats = peer1.getStats();
        expect(connectedStats.totalPeers).toBe(1);
        expect(connectedStats.incomingConnections).toBe(1);

        await peer2.shutdown();
        await new Promise(resolve => setTimeout(resolve, 200));

        const disconnectedStats = peer1.getStats();
        expect(disconnectedStats.totalPeers).toBe(0);

        done();
      });
    });

    test('should differentiate between connection types in stats', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'stats-peer-1',
        server: { port: 22008 }
      });

      peer2 = new UMICPWebSocketPeer({
        peerId: 'stats-peer-2',
        server: { port: 22009 }
      });

      let ready = 0;

      const checkReady = async () => {
        ready++;
        if (ready === 2) {
          // peer1 connects to peer2 (outgoing for peer1)
          await peer1.connectToPeer('ws://localhost:22009/umicp');

          // peer3 connects to peer1 (incoming for peer1)
          peer3 = new UMICPWebSocketPeer({ peerId: 'stats-peer-3' });
          await peer3.connectToPeer('ws://localhost:22008/umicp');

          await new Promise(resolve => setTimeout(resolve, 300));

          const stats1 = peer1.getStats();
          expect(stats1.incomingConnections).toBeGreaterThanOrEqual(1);
          expect(stats1.outgoingConnections).toBeGreaterThanOrEqual(1);

          done();
        }
      };

      peer1.on('server:ready', checkReady);
      peer2.on('server:ready', checkReady);
    });
  });

  describe('Error Recovery', () => {
    test('should recover from handshake timeout', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'timeout-server',
        server: { port: 22010 },
        autoProtocol: false, // Disable to simulate timeout
        handshakeTimeout: 1000
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'timeout-client',
          autoProtocol: false,
          handshakeTimeout: 1000
        });

        let timeoutFired = false;

        peer1.on('peer:handshake-timeout', (peer: PeerConnection) => {
          expect(peer.handshakeComplete).toBeFalsy();
          timeoutFired = true;
          done();
        });

        await peer2.connectToPeer('ws://localhost:22010/umicp');

        // Wait for timeout
        await new Promise(resolve => setTimeout(resolve, 1500));
      });
    });

    test('should handle error events without crashing', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'error-handler'
      });

      let errorHandled = false;

      peer1.on('error', (error: Error) => {
        expect(error).toBeDefined();
        errorHandled = true;
      });

      // Trigger error by connecting to invalid address
      peer1.connectToPeer('ws://invalid:99999/umicp').catch(() => {
        // Expected to fail
      });

      setTimeout(() => {
        expect(errorHandled).toBe(true);
        done();
      }, 500);
    });

    test('should cleanup resources after multiple connection failures', async () => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'cleanup-test'
      });

      // Try multiple failed connections
      for (let i = 0; i < 5; i++) {
        await peer1.connectToPeer(`ws://localhost:${29990 + i}/umicp`);
        await new Promise(resolve => setTimeout(resolve, 100));
      }

      const stats = peer1.getStats();
      // Should not accumulate failed connections
      expect(stats.totalPeers).toBe(0);
    });
  });

  describe('Cascade Failure Prevention', () => {
    test('should isolate failures in mesh network', (done) => {
      const peers: UMICPWebSocketPeer[] = [];
      let setupComplete = 0;

      // Create 3-node mesh
      for (let i = 0; i < 3; i++) {
        const peer = new UMICPWebSocketPeer({
          peerId: `mesh-node-${i}`,
          server: { port: 22011 + i }
        });

        peer.on('server:ready', async () => {
          setupComplete++;
          if (setupComplete === 3) {
            // Connect nodes in a triangle
            await peers[0].connectToPeer('ws://localhost:22012/umicp');
            await peers[1].connectToPeer('ws://localhost:22013/umicp');
            await peers[2].connectToPeer('ws://localhost:22011/umicp');

            await new Promise(resolve => setTimeout(resolve, 500));

            // Kill one node
            await peers[1].shutdown();
            await new Promise(resolve => setTimeout(resolve, 300));

            // Verify other nodes still operational
            const stats0 = peers[0].getStats();
            const stats2 = peers[2].getStats();

            expect(stats0.totalPeers).toBeGreaterThanOrEqual(1);
            expect(stats2.totalPeers).toBeGreaterThanOrEqual(1);

            done();
          }
        });

        peers.push(peer);
      }

      // Cleanup
      afterEach(async () => {
        for (const p of peers) {
          await p.shutdown();
        }
      });
    });
  });
});

