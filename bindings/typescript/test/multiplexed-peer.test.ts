/**
 * Tests for UMICPWebSocketPeer - Multiplexed Peer Architecture
 *
 * Tests cover:
 * - Peer initialization (server component)
 * - Outgoing connections (client component)
 * - Incoming connections (server component)
 * - Multiplexed connections (simultaneous incoming/outgoing)
 * - EventEmitter pattern
 * - Handshake protocol
 * - Message routing
 * - Broadcast functionality
 * - Peer management
 * - Connection lifecycle
 */

import {
  UMICPWebSocketPeer,
  Envelope,
  OperationType,
  type PeerConnection,
  type PeerInfo
} from '../src/index.js';

describe('UMICPWebSocketPeer - Core Functionality', () => {
  let peer1: UMICPWebSocketPeer;
  let peer2: UMICPWebSocketPeer;
  let peer3: UMICPWebSocketPeer;

  afterEach(async () => {
    // Clean up all peers
    if (peer1) await peer1.shutdown();
    if (peer2) await peer2.shutdown();
    if (peer3) await peer3.shutdown();

    // Wait for cleanup
    await new Promise(resolve => setTimeout(resolve, 200));
  });

  describe('Peer Initialization', () => {
    test('should create peer with server component', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'test-peer-1',
        server: {
          port: 20091,
          path: '/umicp',
          compression: true,
        },
        metadata: {
          role: 'coordinator',
          version: '1.0.0',
        }
      });

      peer1.on('server:ready', () => {
        const stats = peer1.getStats();
        expect(stats.peerId).toBe('test-peer-1');
        expect(stats.serverActive).toBe(true);
        expect(stats.totalPeers).toBe(0);
        done();
      });
    });

    test('should create peer without server (client-only)', () => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'client-peer',
        metadata: {
          role: 'worker',
        }
      });

      const stats = peer1.getStats();
      expect(stats.peerId).toBe('client-peer');
      expect(stats.serverActive).toBe(false);
    });

    test('should support custom metadata', (done) => {
      const customMetadata = {
        environment: 'test',
        region: 'us-east-1',
        capabilities: 'ml-training',
      };

      peer1 = new UMICPWebSocketPeer({
        peerId: 'metadata-peer',
        server: { port: 20092 },
        metadata: customMetadata,
      });

      peer1.on('server:ready', () => {
        done();
      });
    });
  });

  describe('EventEmitter Pattern', () => {
    test('should emit server:ready event', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'event-peer',
        server: { port: 20093 },
      });

      peer1.on('server:ready', () => {
        expect(true).toBe(true);
        done();
      });
    });

    test('should emit peer:connect on incoming connection', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'server-peer',
        server: { port: 20094 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'client-peer',
        });

        peer1.on('peer:connect', (peerConnection: PeerConnection) => {
          expect(peerConnection.type).toBe('incoming');
          expect(peerConnection.id).toContain('incoming-');
          done();
        });

        await peer2.connectToPeer('ws://localhost:20094/umicp');
      });
    });

    test('should emit connected on outgoing connection', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'server-peer',
        server: { port: 20095 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'client-peer',
        });

        peer2.on('connected', (peerId: string, url: string) => {
          expect(peerId).toContain('outgoing-');
          expect(url).toBe('ws://localhost:20095/umicp');
          done();
        });

        await peer2.connectToPeer('ws://localhost:20095/umicp');
      });
    });

    test('should support multiple event listeners', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'multi-listener-peer',
        server: { port: 20096 },
      });

      let listener1Called = false;
      let listener2Called = false;

      peer1.on('server:ready', () => {
        listener1Called = true;
      });

      peer1.on('server:ready', () => {
        listener2Called = true;
      });

      setTimeout(() => {
        expect(listener1Called).toBe(true);
        expect(listener2Called).toBe(true);
        done();
      }, 100);
    });

    test('should emit error event on connection failure', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'error-peer',
      });

      peer1.on('error', (error: Error) => {
        expect(error).toBeDefined();
        done();
      });

      // Try to connect to non-existent server
      peer1.connectToPeer('ws://localhost:29999/umicp');
    });
  });

  describe('Handshake Protocol', () => {
    test('should complete handshake automatically', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'handshake-server',
        server: { port: 20097 },
        autoProtocol: true,
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'handshake-client',
          autoProtocol: true,
        });

        let serverReady = false;
        let clientReady = false;

        peer1.on('peer:ready', (peer: PeerConnection, peerInfo: PeerInfo) => {
          expect(peerInfo.peerId).toBe('handshake-client');
          expect(peer.handshakeComplete).toBe(true);
          serverReady = true;

          if (serverReady && clientReady) {
            done();
          }
        });

        peer2.on('peer:ready', (peer: PeerConnection, peerInfo: PeerInfo) => {
          expect(peerInfo.peerId).toBe('handshake-server');
          expect(peer.handshakeComplete).toBe(true);
          clientReady = true;

          if (serverReady && clientReady) {
            done();
          }
        });

        await peer2.connectToPeer('ws://localhost:20097/umicp');
      });
    });

    test('should timeout handshake after configured timeout', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'timeout-server',
        server: { port: 20098 },
        autoProtocol: false, // Disable auto protocol to simulate timeout
        handshakeTimeout: 1000,
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'timeout-client',
          autoProtocol: false,
          handshakeTimeout: 1000,
        });

        peer1.on('peer:handshake-timeout', (peer: PeerConnection) => {
          expect(peer.handshakeComplete).toBeFalsy();
          done();
        });

        await peer2.connectToPeer('ws://localhost:20098/umicp');
      });
    });

    test('should exchange metadata during handshake', (done) => {
      const metadata1 = { role: 'coordinator', capability: 'ml-training' };
      const metadata2 = { role: 'worker', capability: 'data-processing' };

      peer1 = new UMICPWebSocketPeer({
        peerId: 'metadata-server',
        server: { port: 20099 },
        metadata: metadata1,
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'metadata-client',
          metadata: metadata2,
        });

        peer1.on('peer:ready', (peer: PeerConnection, peerInfo: PeerInfo) => {
          expect(peerInfo.metadata.role).toBe('worker');
          expect(peerInfo.metadata.capability).toBe('data-processing');
          done();
        });

        await peer2.connectToPeer('ws://localhost:20099/umicp');
      });
    });
  });

  describe('Outgoing Connections', () => {
    test('should connect to remote peer successfully', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'server',
        server: { port: 20100 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'client',
        });

        const peerId = await peer2.connectToPeer('ws://localhost:20100/umicp');

        expect(peerId).toBeTruthy();
        expect(peerId).toContain('outgoing-');

        const stats = peer2.getStats();
        expect(stats.outgoingConnections).toBe(1);
        done();
      });
    });

    test('should connect to multiple remote peers', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'server1',
        server: { port: 20101 },
      });

      peer2 = new UMICPWebSocketPeer({
        peerId: 'server2',
        server: { port: 20102 },
      });

      let serversReady = 0;

      const checkServersReady = async () => {
        serversReady++;
        if (serversReady === 2) {
          peer3 = new UMICPWebSocketPeer({
            peerId: 'multi-client',
          });

          const peer1Id = await peer3.connectToPeer('ws://localhost:20101/umicp');
          const peer2Id = await peer3.connectToPeer('ws://localhost:20102/umicp');

          expect(peer1Id).toBeTruthy();
          expect(peer2Id).toBeTruthy();

          const stats = peer3.getStats();
          expect(stats.outgoingConnections).toBe(2);
          done();
        }
      };

      peer1.on('server:ready', checkServersReady);
      peer2.on('server:ready', checkServersReady);
    });

    test('should handle connection failure gracefully', async () => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'failure-client',
      });

      const peerId = await peer1.connectToPeer('ws://localhost:29998/umicp');
      expect(peerId).toBeNull();
    });

    test('should store connection metadata', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'meta-server',
        server: { port: 20103 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'meta-client',
        });

        const connectionMetadata = {
          priority: 'high',
          region: 'us-west',
        };

        const peerId = await peer2.connectToPeer(
          'ws://localhost:20103/umicp',
          connectionMetadata
        );

        const peer = peer2.getPeer(peerId!);
        expect(peer?.metadata.priority).toBe('high');
        expect(peer?.metadata.region).toBe('us-west');
        done();
      });
    });
  });

  describe('Incoming Connections', () => {
    test('should accept incoming connections', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'accept-server',
        server: { port: 20104 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'incoming-client',
        });

        peer1.on('peer:connect', (peer: PeerConnection) => {
          expect(peer.type).toBe('incoming');
          const stats = peer1.getStats();
          expect(stats.incomingConnections).toBe(1);
          done();
        });

        await peer2.connectToPeer('ws://localhost:20104/umicp');
      });
    });

    test('should accept multiple incoming connections', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'multi-accept-server',
        server: { port: 20105 },
      });

      let connectionsCount = 0;

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'client1' });
        peer3 = new UMICPWebSocketPeer({ peerId: 'client2' });

        peer1.on('peer:connect', () => {
          connectionsCount++;
          if (connectionsCount === 2) {
            const stats = peer1.getStats();
            expect(stats.incomingConnections).toBe(2);
            done();
          }
        });

        await peer2.connectToPeer('ws://localhost:20105/umicp');
        await peer3.connectToPeer('ws://localhost:20105/umicp');
      });
    });
  });

  describe('Multiplexed Connections', () => {
    test('should handle simultaneous incoming and outgoing connections', (done) => {
      // Create a mesh: peer1 <-> peer2 <-> peer3
      peer1 = new UMICPWebSocketPeer({
        peerId: 'mesh-peer-1',
        server: { port: 20106 },
      });

      peer2 = new UMICPWebSocketPeer({
        peerId: 'mesh-peer-2',
        server: { port: 20107 },
      });

      let serversReady = 0;

      const checkMeshSetup = async () => {
        serversReady++;
        if (serversReady === 2) {
          // peer1 connects to peer2
          await peer1.connectToPeer('ws://localhost:20107/umicp');

          // Wait for connections to establish
          await new Promise(resolve => setTimeout(resolve, 500));

          const stats1 = peer1.getStats();
          const stats2 = peer2.getStats();

          // peer1 has 0 incoming (peer2 didn't connect to it) and 1 outgoing
          expect(stats1.outgoingConnections).toBe(1);

          // peer2 has 1 incoming (from peer1) and 0 outgoing
          expect(stats2.incomingConnections).toBe(1);

          done();
        }
      };

      peer1.on('server:ready', checkMeshSetup);
      peer2.on('server:ready', checkMeshSetup);
    });

    test('should differentiate between incoming and outgoing peers', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'differentiate-server',
        server: { port: 20108 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'other-peer',
          server: { port: 20109 },
        });

        peer2.on('server:ready', async () => {
          // peer1 connects to peer2 (outgoing for peer1)
          await peer1.connectToPeer('ws://localhost:20109/umicp');

          await new Promise(resolve => setTimeout(resolve, 300));

          const outgoingPeers = peer1.getPeersByType('outgoing');
          const incomingPeers = peer2.getPeersByType('incoming');

          expect(outgoingPeers.length).toBe(1);
          expect(outgoingPeers[0].type).toBe('outgoing');
          expect(incomingPeers.length).toBe(1);
          expect(incomingPeers[0].type).toBe('incoming');

          done();
        });
      });
    });
  });

  describe('Message Routing', () => {
    test('should send message to specific peer by ID', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'routing-server',
        server: { port: 20110 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'routing-client',
        });

        let receivedMessage = false;

        peer2.on('data', (envelope: Envelope) => {
          expect(envelope.getCapabilities().message).toBe('hello');
          receivedMessage = true;
          done();
        });

        const peerId = await peer2.connectToPeer('ws://localhost:20110/umicp');

        // Wait for handshake
        await new Promise(resolve => setTimeout(resolve, 300));

        const peers = peer1.getPeers();
        const targetPeer = peers[0];

        const envelope = new Envelope({
          from: 'routing-server',
          to: 'routing-client',
          operation: OperationType.DATA,
          messageId: `msg-${Date.now()}`,
          capabilities: { message: 'hello' },
        });

        peer1.sendToPeer(targetPeer.id, envelope);
      });
    });

    test('should send message to peer by URL', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'url-routing-server',
        server: { port: 20111 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'url-routing-client',
        });

        peer1.on('data', (envelope: Envelope) => {
          expect(envelope.getCapabilities().test).toBe('url-routing');
          done();
        });

        await peer2.connectToPeer('ws://localhost:20111/umicp');

        // Wait for handshake
        await new Promise(resolve => setTimeout(resolve, 300));

        const envelope = new Envelope({
          from: 'url-routing-client',
          to: 'url-routing-server',
          operation: OperationType.DATA,
          messageId: `url-${Date.now()}`,
          capabilities: { test: 'url-routing' },
        });

        peer2.sendToPeerByUrl('ws://localhost:20111/umicp', envelope);
      });
    });

    test('should filter DATA messages from protocol messages', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'filter-server',
        server: { port: 20112 },
        autoProtocol: true,
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'filter-client',
          autoProtocol: true,
        });

        let dataEventCount = 0;
        let messageEventCount = 0;

        peer1.on('data', () => {
          dataEventCount++;
        });

        peer1.on('message', () => {
          messageEventCount++;
        });

        await peer2.connectToPeer('ws://localhost:20112/umicp');

        // Wait for handshake (this should trigger 'message' but not 'data')
        await new Promise(resolve => setTimeout(resolve, 300));

        // Send actual data message
        const envelope = new Envelope({
          from: 'filter-client',
          to: 'filter-server',
          operation: OperationType.DATA,
          messageId: `data-${Date.now()}`,
          capabilities: { payload: 'actual-data' },
        });

        const peers = peer2.getPeers();
        peer2.sendToPeer(peers[0].id, envelope);

        await new Promise(resolve => setTimeout(resolve, 200));

        // data event should only fire for DATA messages
        expect(dataEventCount).toBe(1);
        // message event should fire for all messages (including protocol)
        expect(messageEventCount).toBeGreaterThan(1);
        done();
      });
    });
  });

  describe('Broadcast Functionality', () => {
    test('should broadcast to all connected peers', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'broadcast-hub',
        server: { port: 20113 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'broadcast-client-1' });
        peer3 = new UMICPWebSocketPeer({ peerId: 'broadcast-client-2' });

        let receivedCount = 0;

        const checkBroadcast = () => {
          receivedCount++;
          if (receivedCount === 2) {
            done();
          }
        };

        peer2.on('data', checkBroadcast);
        peer3.on('data', checkBroadcast);

        await peer2.connectToPeer('ws://localhost:20113/umicp');
        await peer3.connectToPeer('ws://localhost:20113/umicp');

        // Wait for handshake
        await new Promise(resolve => setTimeout(resolve, 300));

        const broadcastEnvelope = new Envelope({
          from: 'broadcast-hub',
          to: 'all',
          operation: OperationType.DATA,
          messageId: `broadcast-${Date.now()}`,
          capabilities: { type: 'broadcast', message: 'Hello everyone!' },
        });

        const sent = peer1.broadcast(broadcastEnvelope);
        expect(sent).toBe(2);
      });
    });

    test('should broadcast excluding specific peer', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'exclude-broadcast-hub',
        server: { port: 20114 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'include-client' });
        peer3 = new UMICPWebSocketPeer({ peerId: 'exclude-client' });

        let peer2Received = false;
        let peer3Received = false;

        peer2.on('data', () => {
          peer2Received = true;
        });

        peer3.on('data', () => {
          peer3Received = true;
        });

        await peer2.connectToPeer('ws://localhost:20114/umicp');
        await peer3.connectToPeer('ws://localhost:20114/umicp');

        // Wait for handshake
        await new Promise(resolve => setTimeout(resolve, 300));

        const peers = peer1.getPeers();
        const excludePeerId = peers[1].id; // Exclude second peer

        const envelope = new Envelope({
          from: 'exclude-broadcast-hub',
          to: 'some',
          operation: OperationType.DATA,
          messageId: `selective-${Date.now()}`,
          capabilities: { type: 'selective-broadcast' },
        });

        peer1.broadcast(envelope, excludePeerId);

        await new Promise(resolve => setTimeout(resolve, 200));

        // Only one peer should receive
        const receivedCount = (peer2Received ? 1 : 0) + (peer3Received ? 1 : 0);
        expect(receivedCount).toBe(1);
        done();
      });
    });

    test('should broadcast to specific peer type', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'type-broadcast-hub',
        server: { port: 20115 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'type-peer',
          server: { port: 20116 },
        });

        peer2.on('server:ready', async () => {
          // Create incoming connection
          peer3 = new UMICPWebSocketPeer({ peerId: 'incoming-only' });
          await peer3.connectToPeer('ws://localhost:20115/umicp');

          // Create outgoing connection
          await peer1.connectToPeer('ws://localhost:20116/umicp');

          // Wait for connections
          await new Promise(resolve => setTimeout(resolve, 300));

          let incomingReceived = false;

          peer3.on('data', () => {
            incomingReceived = true;
          });

          const envelope = new Envelope({
            from: 'type-broadcast-hub',
            to: 'incoming-peers',
            operation: OperationType.DATA,
            messageId: `type-broadcast-${Date.now()}`,
            capabilities: { target: 'incoming-only' },
          });

          const sent = peer1.broadcastToType(envelope, 'incoming');

          await new Promise(resolve => setTimeout(resolve, 200));

          expect(sent).toBe(1);
          expect(incomingReceived).toBe(true);
          done();
        });
      });
    });
  });

  describe('Peer Management', () => {
    test('should get all connected peers', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'management-hub',
        server: { port: 20117 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'managed-peer-1' });
        peer3 = new UMICPWebSocketPeer({ peerId: 'managed-peer-2' });

        await peer2.connectToPeer('ws://localhost:20117/umicp');
        await peer3.connectToPeer('ws://localhost:20117/umicp');

        await new Promise(resolve => setTimeout(resolve, 300));

        const peers = peer1.getPeers();
        expect(peers.length).toBe(2);
        expect(peers.every(p => p.type === 'incoming')).toBe(true);
        done();
      });
    });

    test('should get peer by ID', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'getbyid-server',
        server: { port: 20118 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'getbyid-client' });

        peer1.on('peer:connect', (peerConnection: PeerConnection) => {
          const retrievedPeer = peer1.getPeer(peerConnection.id);
          expect(retrievedPeer).toBeDefined();
          expect(retrievedPeer?.id).toBe(peerConnection.id);
          done();
        });

        await peer2.connectToPeer('ws://localhost:20118/umicp');
      });
    });

    test('should find peer by metadata', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'metadata-search-server',
        server: { port: 20119 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'searchable-client' });

        const connectionMetadata = {
          role: 'ml-worker',
          gpu: 'nvidia-a100',
        };

        const peerId = await peer2.connectToPeer(
          'ws://localhost:20119/umicp',
          connectionMetadata
        );

        await new Promise(resolve => setTimeout(resolve, 300));

        const foundPeer = peer1.findPeerByMetadata('role', 'ml-worker');
        expect(foundPeer).toBeDefined();
        expect(foundPeer?.metadata.gpu).toBe('nvidia-a100');
        done();
      });
    });

    test('should disconnect specific peer', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'disconnect-server',
        server: { port: 20120 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'disconnect-client' });

        let disconnectEventFired = false;

        peer1.on('peer:disconnect', () => {
          disconnectEventFired = true;
        });

        const peerId = await peer2.connectToPeer('ws://localhost:20120/umicp');

        await new Promise(resolve => setTimeout(resolve, 300));

        const peers = peer1.getPeers();
        expect(peers.length).toBe(1);

        const disconnected = peer1.disconnectPeer(peers[0].id);
        expect(disconnected).toBe(true);

        await new Promise(resolve => setTimeout(resolve, 200));

        const remainingPeers = peer1.getPeers();
        expect(remainingPeers.length).toBe(0);
        expect(disconnectEventFired).toBe(true);
        done();
      });
    });

    test('should disconnect peer by URL', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'disconnect-url-server',
        server: { port: 20121 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'disconnect-url-client' });

        await peer2.connectToPeer('ws://localhost:20121/umicp');

        await new Promise(resolve => setTimeout(resolve, 300));

        const disconnected = peer2.disconnectPeerByUrl('ws://localhost:20121/umicp');
        expect(disconnected).toBe(true);

        await new Promise(resolve => setTimeout(resolve, 200));

        const stats = peer2.getStats();
        expect(stats.totalPeers).toBe(0);
        done();
      });
    });
  });

  describe('Connection Lifecycle', () => {
    test('should handle peer disconnection', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'lifecycle-server',
        server: { port: 20122 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'lifecycle-client' });

        let disconnectFired = false;

        peer1.on('peer:disconnect', (peer: PeerConnection) => {
          expect(peer.type).toBe('incoming');
          disconnectFired = true;
          done();
        });

        await peer2.connectToPeer('ws://localhost:20122/umicp');

        await new Promise(resolve => setTimeout(resolve, 300));

        // Disconnect client
        await peer2.shutdown();

        await new Promise(resolve => setTimeout(resolve, 200));
        expect(disconnectFired).toBe(true);
      });
    });

    test('should handle reconnection', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'reconnect-server',
        server: { port: 20123 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'reconnect-client' });

        let connectionCount = 0;

        peer1.on('peer:connect', () => {
          connectionCount++;

          if (connectionCount === 2) {
            // Reconnected successfully
            done();
          }
        });

        // First connection
        await peer2.connectToPeer('ws://localhost:20123/umicp');
        await new Promise(resolve => setTimeout(resolve, 300));

        // Disconnect
        await peer2.shutdown();
        await new Promise(resolve => setTimeout(resolve, 300));

        // Reconnect
        peer2 = new UMICPWebSocketPeer({ peerId: 'reconnect-client-2' });
        await peer2.connectToPeer('ws://localhost:20123/umicp');
      });
    });

    test('should complete full shutdown', async () => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'shutdown-server',
        server: { port: 20124 },
      });

      await new Promise(resolve => {
        peer1.on('server:ready', resolve);
      });

      peer2 = new UMICPWebSocketPeer({ peerId: 'shutdown-client' });
      await peer2.connectToPeer('ws://localhost:20124/umicp');

      await new Promise(resolve => setTimeout(resolve, 300));

      const stats1Before = peer1.getStats();
      expect(stats1Before.totalPeers).toBe(1);

      await peer1.shutdown();

      const stats1After = peer1.getStats();
      expect(stats1After.totalPeers).toBe(0);
      expect(stats1After.serverActive).toBe(false);
    });
  });

  describe('Statistics and Monitoring', () => {
    test('should provide accurate peer statistics', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'stats-hub',
        server: { port: 20125 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({
          peerId: 'stats-peer-1',
          server: { port: 20126 },
        });

        peer2.on('server:ready', async () => {
          // Create incoming connection to peer1
          peer3 = new UMICPWebSocketPeer({ peerId: 'stats-client' });
          await peer3.connectToPeer('ws://localhost:20125/umicp');

          // Create outgoing connection from peer1
          await peer1.connectToPeer('ws://localhost:20126/umicp');

          await new Promise(resolve => setTimeout(resolve, 300));

          const stats = peer1.getStats();
          expect(stats.peerId).toBe('stats-hub');
          expect(stats.totalPeers).toBe(2);
          expect(stats.incomingConnections).toBe(1);
          expect(stats.outgoingConnections).toBe(1);
          expect(stats.serverActive).toBe(true);
          expect(stats.peers).toHaveLength(2);
          done();
        });
      });
    });

    test('should track connection timestamps', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'timestamp-server',
        server: { port: 20127 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'timestamp-client' });

        const beforeConnection = new Date();

        peer1.on('peer:connect', (peer: PeerConnection) => {
          const afterConnection = new Date();

          expect(peer.connectedAt).toBeDefined();
          expect(peer.connectedAt.getTime()).toBeGreaterThanOrEqual(beforeConnection.getTime());
          expect(peer.connectedAt.getTime()).toBeLessThanOrEqual(afterConnection.getTime());
          done();
        });

        await peer2.connectToPeer('ws://localhost:20127/umicp');
      });
    });
  });

  describe('Send and Wait Pattern', () => {
    test('should send message and wait for response', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'request-server',
        server: { port: 20128 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'request-client' });

        // Setup response handler on server
        peer1.on('data', (envelope: Envelope, peer: PeerConnection) => {
          const response = new Envelope({
            from: 'request-server',
            to: 'request-client',
            operation: OperationType.DATA,
            messageId: `response-${Date.now()}`,
            capabilities: {
              reply_to: envelope.getMessageId(),
              result: 'processed',
            },
          });

          peer1.sendToPeer(peer.id, response);
        });

        const peerId = await peer2.connectToPeer('ws://localhost:20128/umicp');

        // Wait for handshake
        await new Promise(resolve => setTimeout(resolve, 300));

        const requestEnvelope = new Envelope({
          from: 'request-client',
          to: 'request-server',
          operation: OperationType.DATA,
          messageId: `req-${Date.now()}`,
          capabilities: { action: 'process' },
        });

        const peers = peer2.getPeers();

        try {
          const response = await peer2.sendAndWait(peers[0].id, requestEnvelope, 5000);
          expect(response.getCapabilities().result).toBe('processed');
          done();
        } catch (error) {
          done(error);
        }
      });
    });

    test('should timeout if no response received', (done) => {
      peer1 = new UMICPWebSocketPeer({
        peerId: 'timeout-request-server',
        server: { port: 20129 },
      });

      peer1.on('server:ready', async () => {
        peer2 = new UMICPWebSocketPeer({ peerId: 'timeout-request-client' });

        // Server doesn't respond

        const peerId = await peer2.connectToPeer('ws://localhost:20129/umicp');

        await new Promise(resolve => setTimeout(resolve, 300));

        const requestEnvelope = new Envelope({
          from: 'timeout-request-client',
          to: 'timeout-request-server',
          operation: OperationType.DATA,
          messageId: `timeout-req-${Date.now()}`,
          capabilities: { action: 'process' },
        });

        const peers = peer2.getPeers();

        try {
          await peer2.sendAndWait(peers[0].id, requestEnvelope, 1000);
          done(new Error('Should have timed out'));
        } catch (error) {
          expect(error).toBeDefined();
          expect((error as Error).message).toContain('timeout');
          done();
        }
      });
    });
  });
});

