/**
 * Example 2: Mesh Network Communication
 * 
 * This example demonstrates:
 * - Creating a mesh of 3 interconnected peers
 * - Each peer can communicate with any other peer
 * - Broadcasting messages to all peers
 * - Dynamic peer discovery
 */

import { 
  UMICPWebSocketPeer, 
  Envelope, 
  OperationType 
} from '../src/index.js';

async function main() {
  console.log('🕸️  UMICP Example 2: Mesh Network\n');

  const peers: UMICPWebSocketPeer[] = [];
  const peerConfigs = [
    { id: 'node-A', port: 8091, color: '🔴' },
    { id: 'node-B', port: 8092, color: '🟢' },
    { id: 'node-C', port: 8093, color: '🔵' }
  ];

  // Create all peers
  console.log('Creating mesh network nodes...');
  for (const config of peerConfigs) {
    const peer = new UMICPWebSocketPeer({
      peerId: config.id,
      server: {
        port: config.port,
        path: '/umicp'
      },
      metadata: {
        type: 'mesh-node',
        color: config.color
      }
    });

    peer.on('server:ready', () => {
      console.log(`${config.color} ${config.id} ready on port ${config.port}`);
    });

    peer.on('peer:connect', (peerConn) => {
      console.log(`${config.color} ${config.id} - New connection: ${peerConn.id}`);
    });

    peer.on('peer:ready', (peerConn, peerInfo) => {
      console.log(`${config.color} ${config.id} - Handshake with: ${peerInfo.peerId}`);
    });

    peer.on('data', (envelope, peerConn) => {
      const msg = envelope.getCapabilities()['message'];
      console.log(`\n${config.color} ${config.id} received: "${msg}"`);
      console.log(`   From: ${envelope.getFrom()}`);
    });

    peers.push(peer);
  }

  // Wait for all servers to be ready
  await new Promise(resolve => setTimeout(resolve, 1000));
  console.log('\n✅ All nodes initialized\n');

  // Connect peers in a mesh topology:
  // A -> B, B -> C, C -> A (triangle)
  console.log('🔗 Connecting mesh network...');
  
  await peers[0].connectToPeer('ws://localhost:8092/umicp'); // A -> B
  console.log('   Node-A connected to Node-B');
  
  await peers[1].connectToPeer('ws://localhost:8093/umicp'); // B -> C
  console.log('   Node-B connected to Node-C');
  
  await peers[2].connectToPeer('ws://localhost:8091/umicp'); // C -> A
  console.log('   Node-C connected to Node-A');

  await new Promise(resolve => setTimeout(resolve, 1500));
  console.log('\n✅ Mesh network established!\n');

  // Show topology
  console.log('📊 Network Topology:');
  peers.forEach((peer, idx) => {
    const stats = peer.getStats();
    const config = peerConfigs[idx];
    console.log(`${config.color} ${config.id}:`);
    console.log(`   Total peers: ${stats.totalPeers}`);
    console.log(`   Incoming: ${stats.incomingConnections}`);
    console.log(`   Outgoing: ${stats.outgoingConnections}`);
  });

  // Broadcast from each node
  console.log('\n📡 Broadcasting from each node...\n');
  
  for (let i = 0; i < peers.length; i++) {
    const config = peerConfigs[i];
    const message = new Envelope({
      from: config.id,
      to: 'all',
      operation: OperationType.DATA,
      messageId: `broadcast-${config.id}-${Date.now()}`,
      capabilities: {
        message: `${config.color} Broadcast from ${config.id}!`,
        timestamp: new Date().toISOString()
      }
    });

    const sent = peers[i].broadcast(message);
    console.log(`${config.color} ${config.id} broadcast to ${sent} peer(s)`);
    
    await new Promise(resolve => setTimeout(resolve, 1000));
  }

  // Point-to-point communication
  console.log('\n💬 Direct peer-to-peer messages...\n');
  
  // A sends to B
  const aToBEnvelope = new Envelope({
    from: 'node-A',
    to: 'node-B',
    operation: OperationType.DATA,
    messageId: `p2p-a-to-b-${Date.now()}`,
    capabilities: {
      message: '🔴 Private message from A to B'
    }
  });
  
  const aPeers = peers[0].getPeersByType('outgoing');
  if (aPeers.length > 0) {
    peers[0].sendToPeer(aPeers[0].id, aToBEnvelope);
  }

  await new Promise(resolve => setTimeout(resolve, 500));

  // B sends to C
  const bToCEnvelope = new Envelope({
    from: 'node-B',
    to: 'node-C',
    operation: OperationType.DATA,
    messageId: `p2p-b-to-c-${Date.now()}`,
    capabilities: {
      message: '🟢 Private message from B to C'
    }
  });
  
  const bPeers = peers[1].getPeersByType('outgoing');
  if (bPeers.length > 0) {
    peers[1].sendToPeer(bPeers[0].id, bToCEnvelope);
  }

  await new Promise(resolve => setTimeout(resolve, 500));

  // C sends to A
  const cToAEnvelope = new Envelope({
    from: 'node-C',
    to: 'node-A',
    operation: OperationType.DATA,
    messageId: `p2p-c-to-a-${Date.now()}`,
    capabilities: {
      message: '🔵 Private message from C to A'
    }
  });
  
  const cPeers = peers[2].getPeersByType('outgoing');
  if (cPeers.length > 0) {
    peers[2].sendToPeer(cPeers[0].id, cToAEnvelope);
  }

  await new Promise(resolve => setTimeout(resolve, 2000));

  // Final statistics
  console.log('\n📊 Final Network Statistics:');
  peers.forEach((peer, idx) => {
    const config = peerConfigs[idx];
    const stats = peer.getStats();
    console.log(`\n${config.color} ${config.id}:`);
    console.log(`   Total connections: ${stats.totalPeers}`);
    console.log(`   Server active: ${stats.serverActive}`);
  });

  // Cleanup
  console.log('\n🧹 Shutting down mesh network...');
  for (const peer of peers) {
    await peer.shutdown();
  }
  
  console.log('✅ Mesh network example completed!\n');
}

// Run example
main().catch(console.error);

