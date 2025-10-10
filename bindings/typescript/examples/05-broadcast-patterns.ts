/**
 * Example 5: Broadcast Patterns
 * 
 * This example demonstrates:
 * - Broadcasting to all peers
 * - Selective broadcast
 * - Broadcast by peer type
 * - Topic-based broadcasting
 */

import { 
  UMICPWebSocketPeer, 
  Envelope, 
  OperationType 
} from '../src/index.js';

async function main() {
  console.log('📡 UMICP Example 5: Broadcast Patterns\n');

  // Create hub/coordinator
  const hub = new UMICPWebSocketPeer({
    peerId: 'broadcast-hub',
    server: {
      port: 8095,
      path: '/umicp'
    },
    metadata: {
      role: 'coordinator'
    }
  });

  hub.on('peer:ready', (peer, peerInfo) => {
    console.log(`✅ Peer connected: ${peerInfo.peerId} (${peerInfo.metadata.role})`);
  });

  await new Promise<void>(resolve => {
    hub.on('server:ready', () => {
      console.log('✅ Broadcast Hub ready\n');
      resolve();
    });
  });

  // Create multiple clients with different roles
  const clients: { peer: UMICPWebSocketPeer; role: string; }[] = [];

  const clientConfigs = [
    { id: 'worker-1', role: 'worker', topic: 'ml-training' },
    { id: 'worker-2', role: 'worker', topic: 'ml-training' },
    { id: 'monitor-1', role: 'monitor', topic: 'monitoring' },
    { id: 'monitor-2', role: 'monitor', topic: 'monitoring' },
    { id: 'storage-1', role: 'storage', topic: 'data-storage' }
  ];

  console.log('Creating client peers...\n');
  for (const config of clientConfigs) {
    const peer = new UMICPWebSocketPeer({
      peerId: config.id,
      metadata: {
        role: config.role,
        topic: config.topic
      }
    });

    let messagesReceived = 0;
    peer.on('data', (envelope) => {
      messagesReceived++;
      const broadcastType = envelope.getCapabilities()['broadcast-type'];
      const message = envelope.getCapabilities()['message'];
      console.log(`   📨 ${config.id} (${config.role}) received: ${message}`);
      console.log(`      Type: ${broadcastType}`);
    });

    await peer.connectToPeer('ws://localhost:8095/umicp');
    clients.push({ peer, role: config.role });
  }

  await new Promise(resolve => setTimeout(resolve, 1500));
  console.log('\n✅ All clients connected\n');

  // Pattern 1: Broadcast to ALL peers
  console.log('═══ Pattern 1: Broadcast to ALL ═══\n');
  const broadcastAll = new Envelope({
    from: 'broadcast-hub',
    to: 'all',
    operation: OperationType.DATA,
    messageId: `broadcast-all-${Date.now()}`,
    capabilities: {
      'broadcast-type': 'all',
      'message': '📢 Important announcement for everyone!',
      'priority': 'high'
    }
  });

  const sentToAll = hub.broadcast(broadcastAll);
  console.log(`📡 Broadcast sent to ${sentToAll} peer(s)\n`);
  
  await new Promise(resolve => setTimeout(resolve, 1000));

  // Pattern 2: Broadcast to INCOMING peers only
  console.log('\n═══ Pattern 2: Broadcast to INCOMING peers ═══\n');
  const broadcastIncoming = new Envelope({
    from: 'broadcast-hub',
    to: 'incoming-peers',
    operation: OperationType.DATA,
    messageId: `broadcast-incoming-${Date.now()}`,
    capabilities: {
      'broadcast-type': 'incoming-only',
      'message': '👥 Message for incoming connections only'
    }
  });

  const sentToIncoming = hub.broadcastToType(broadcastIncoming, 'incoming');
  console.log(`📡 Broadcast sent to ${sentToIncoming} incoming peer(s)\n`);
  
  await new Promise(resolve => setTimeout(resolve, 1000));

  // Pattern 3: Selective broadcast (exclude one peer)
  console.log('\n═══ Pattern 3: Selective Broadcast (exclude one) ═══\n');
  const peers = hub.getPeers();
  const excludePeerId = peers[0].id;
  console.log(`Excluding peer: ${excludePeerId}\n`);

  const selectiveBroadcast = new Envelope({
    from: 'broadcast-hub',
    to: 'selected-peers',
    operation: OperationType.DATA,
    messageId: `broadcast-selective-${Date.now()}`,
    capabilities: {
      'broadcast-type': 'selective',
      'message': '🎯 Selective message (one peer excluded)'
    }
  });

  const sentSelective = hub.broadcast(selectiveBroadcast, excludePeerId);
  console.log(`📡 Broadcast sent to ${sentSelective} peer(s)\n`);
  
  await new Promise(resolve => setTimeout(resolve, 1000));

  // Pattern 4: Role-based broadcast (simulated)
  console.log('\n═══ Pattern 4: Role-based Broadcast ═══\n');

  // Broadcast to workers only
  console.log('Broadcasting to WORKERS only:\n');
  const workerPeers = hub.getPeers().filter((peer, idx) => 
    idx < 2 // First two clients are workers
  );

  const workerBroadcast = new Envelope({
    from: 'broadcast-hub',
    to: 'workers',
    operation: OperationType.DATA,
    messageId: `broadcast-workers-${Date.now()}`,
    capabilities: {
      'broadcast-type': 'role-based',
      'message': '⚙️  Task assignment for workers',
      'task': 'process-batch-001'
    }
  });

  let sentToWorkers = 0;
  for (const workerPeer of workerPeers) {
    if (hub.sendToPeer(workerPeer.id, workerBroadcast)) {
      sentToWorkers++;
    }
  }
  console.log(`📡 Sent to ${sentToWorkers} worker(s)\n`);
  
  await new Promise(resolve => setTimeout(resolve, 1000));

  // Broadcast to monitors only
  console.log('Broadcasting to MONITORS only:\n');
  const monitorPeers = hub.getPeers().filter((peer, idx) => 
    idx >= 2 && idx < 4 // Third and fourth clients are monitors
  );

  const monitorBroadcast = new Envelope({
    from: 'broadcast-hub',
    to: 'monitors',
    operation: OperationType.DATA,
    messageId: `broadcast-monitors-${Date.now()}`,
    capabilities: {
      'broadcast-type': 'role-based',
      'message': '📊 Metrics update for monitors',
      'metrics': JSON.stringify({ cpu: 45, memory: 62, connections: 5 })
    }
  });

  let sentToMonitors = 0;
  for (const monitorPeer of monitorPeers) {
    if (hub.sendToPeer(monitorPeer.id, monitorBroadcast)) {
      sentToMonitors++;
    }
  }
  console.log(`📡 Sent to ${sentToMonitors} monitor(s)\n`);
  
  await new Promise(resolve => setTimeout(resolve, 1000));

  // Pattern 5: Sequential broadcast (one by one)
  console.log('\n═══ Pattern 5: Sequential Broadcast ═══\n');
  const sequentialMessage = new Envelope({
    from: 'broadcast-hub',
    to: 'peers',
    operation: OperationType.DATA,
    messageId: `broadcast-sequential-${Date.now()}`,
    capabilities: {
      'broadcast-type': 'sequential',
      'message': '🔢 Sequential message delivery'
    }
  });

  console.log('Sending to peers sequentially:\n');
  for (let i = 0; i < hub.getPeers().length; i++) {
    const peer = hub.getPeers()[i];
    hub.sendToPeer(peer.id, sequentialMessage);
    console.log(`   Sent to peer ${i + 1}`);
    await new Promise(resolve => setTimeout(resolve, 200));
  }
  
  await new Promise(resolve => setTimeout(resolve, 1000));

  // Pattern 6: Conditional broadcast
  console.log('\n═══ Pattern 6: Conditional Broadcast ═══\n');
  console.log('Broadcasting only to peers with specific metadata:\n');

  const conditionalMessage = new Envelope({
    from: 'broadcast-hub',
    to: 'matching-peers',
    operation: OperationType.DATA,
    messageId: `broadcast-conditional-${Date.now()}`,
    capabilities: {
      'broadcast-type': 'conditional',
      'message': '🎲 Message for peers with topic: ml-training'
    }
  });

  let conditionalCount = 0;
  for (const peerConnection of hub.getPeers()) {
    if (peerConnection.peerInfo?.metadata?.topic === 'ml-training') {
      if (hub.sendToPeer(peerConnection.id, conditionalMessage)) {
        conditionalCount++;
      }
    }
  }
  console.log(`📡 Sent to ${conditionalCount} matching peer(s)\n`);
  
  await new Promise(resolve => setTimeout(resolve, 1000));

  // Final statistics
  console.log('\n📊 Final Statistics:');
  console.log(`Hub: ${hub.getStats().totalPeers} connected peers`);
  
  // Cleanup
  console.log('\n🧹 Cleaning up...');
  for (const client of clients) {
    await client.peer.shutdown();
  }
  await hub.shutdown();
  
  console.log('✅ Broadcast patterns example completed!\n');
}

// Run example
main().catch(console.error);

