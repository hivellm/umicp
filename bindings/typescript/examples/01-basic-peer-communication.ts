/**
 * Example 1: Basic Peer-to-Peer Communication
 *
 * This example demonstrates:
 * - Creating two peers
 * - Connecting them
 * - Exchanging messages
 * - Handling responses
 */

import {
  UMICPWebSocketPeer,
  Envelope,
  OperationType
} from '../src/index.js';

async function main() {
  console.log('🚀 UMICP Example 1: Basic Peer Communication\n');

  // Create Server Peer (Hub)
  console.log('Creating Hub peer on port 8081...');
  const hub = new UMICPWebSocketPeer({
    peerId: 'hub',
    server: {
      port: 8081,
      path: '/umicp',
      compression: true
    },
    metadata: {
      role: 'hub',
      location: 'datacenter-1'
    }
  });

  // Setup event handlers for hub
  hub.on('server:ready', () => {
    console.log('✅ Hub is ready and listening on port 8081');
  });

  hub.on('peer:connect', (peer) => {
    console.log(`📥 New peer connected: ${peer.id}`);
  });

  hub.on('peer:ready', (peer, peerInfo) => {
    console.log(`🤝 Handshake complete with: ${peerInfo.peerId}`);
    console.log(`   Metadata:`, peerInfo.metadata);
  });

  hub.on('data', (envelope, peer) => {
    console.log(`\n📨 Hub received message from ${peer.id}:`);
    console.log(`   From: ${envelope.getFrom()}`);
    console.log(`   Message: ${envelope.getCapabilities()['message']}`);

    // Send response
    const response = new Envelope({
      from: 'hub',
      to: envelope.getFrom(),
      operation: OperationType.ACK,
      messageId: `ack-${envelope.getMessageId()}`,
      capabilities: {
        status: 'received',
        timestamp: new Date().toISOString(),
        message: 'Message processed successfully!'
      }
    });

    hub.sendToPeer(peer.id, response);
    console.log('   ✅ Sent acknowledgment');
  });

  // Wait for hub to be ready
  await new Promise<void>(resolve => {
    hub.on('server:ready', () => resolve());
  });

  // Create Worker Peer (Client)
  console.log('\nCreating Worker peer...');
  const worker = new UMICPWebSocketPeer({
    peerId: 'worker-1',
    metadata: {
      role: 'worker',
      capability: 'data-processing'
    }
  });

  // Setup event handlers for worker
  worker.on('connected', (peerId, url) => {
    console.log(`✅ Worker connected to ${url}`);
  });

  worker.on('peer:ready', (peer, peerInfo) => {
    console.log(`🤝 Worker handshake complete with: ${peerInfo.peerId}`);
  });

  worker.on('data', (envelope, peer) => {
    console.log(`\n📨 Worker received response:`);
    console.log(`   Status: ${envelope.getCapabilities()['status']}`);
    console.log(`   Message: ${envelope.getCapabilities()['message']}`);
  });

  // Connect worker to hub
  console.log('Connecting worker to hub...');
  await worker.connectToPeer('ws://localhost:8081/umicp');

  // Wait for handshake to complete
  await new Promise(resolve => setTimeout(resolve, 1000));

  // Send messages from worker to hub
  console.log('\n💬 Sending messages from worker to hub...');

  const peers = worker.getPeers();
  if (peers.length > 0) {
    for (let i = 1; i <= 3; i++) {
      const message = new Envelope({
        from: 'worker-1',
        to: 'hub',
        operation: OperationType.DATA,
        messageId: `msg-${i}-${Date.now()}`,
        capabilities: {
          message: `Hello from worker! Message #${i}`,
          timestamp: new Date().toISOString(),
          sequence: i.toString()
        }
      });

      worker.sendToPeer(peers[0].id, message);
      console.log(`   Sent message #${i}`);

      await new Promise(resolve => setTimeout(resolve, 500));
    }
  }

  // Wait for responses
  await new Promise(resolve => setTimeout(resolve, 2000));

  // Show statistics
  console.log('\n📊 Connection Statistics:');
  console.log('Hub:', hub.getStats());
  console.log('Worker:', worker.getStats());

  // Cleanup
  console.log('\n🧹 Cleaning up...');
  await worker.shutdown();
  await hub.shutdown();

  console.log('✅ Example completed successfully!\n');
}

// Run example
main().catch(console.error);

