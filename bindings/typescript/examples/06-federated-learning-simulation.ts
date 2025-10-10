/**
 * Example 6: Federated Learning Simulation
 *
 * This example demonstrates:
 * - Coordinator distributing model weights
 * - Workers training on local data
 * - Workers sending gradients back
 * - Model aggregation
 * - PayloadHint for large vectors
 */

import {
  UMICPWebSocketPeer,
  Envelope,
  OperationType,
  PayloadType,
  EncodingType
} from '../src/index.js';

async function main() {
  console.log('🤖 UMICP Example 6: Federated Learning Simulation\n');

  // Create coordinator
  const coordinator = new UMICPWebSocketPeer({
    peerId: 'fl-coordinator',
    server: {
      port: 8096,
      path: '/umicp'
    },
    metadata: {
      role: 'coordinator',
      model: 'simple-nn'
    }
  });

  let round = 0;
  const modelWeights: number[][] = [];
  let workersReady = 0;
  let gradientsReceived = 0;

  coordinator.on('peer:ready', (peer, peerInfo) => {
    console.log(`✅ Worker joined: ${peerInfo.peerId}`);
    workersReady++;
  });

  coordinator.on('data', (envelope, peer) => {
    const messageType = envelope.getCapabilities()['type'];

    if (messageType === 'gradients') {
      gradientsReceived++;
      console.log(`   📊 Received gradients from ${envelope.getFrom()}`);

      // Parse gradients
      const gradientsData = JSON.parse(envelope.getCapabilities()['gradients']);
      modelWeights.push(gradientsData);

      console.log(`      Loss: ${envelope.getCapabilities()['loss']}`);
      console.log(`      Accuracy: ${envelope.getCapabilities()['accuracy']}`);

      // When all gradients received, aggregate and send new weights
      if (gradientsReceived === workersReady) {
        console.log(`\n   🔄 All gradients received. Aggregating...`);
        setTimeout(() => aggregateAndDistribute(), 500);
      }
    }
  });

  function aggregateAndDistribute() {
    // Simulate federated averaging
    const aggregated = aggregateWeights(modelWeights);
    round++;

    console.log(`   ✅ Aggregation complete for round ${round}`);
    console.log(`   📤 Distributing updated weights to workers\n`);

    const weightsEnvelope = new Envelope({
      from: 'fl-coordinator',
      to: 'all-workers',
      operation: OperationType.DATA,
      messageId: `weights-round-${round}-${Date.now()}`,
      payloadHint: {
        type: PayloadType.VECTOR,
        size: aggregated.length,
        encoding: EncodingType.FLOAT32,
        count: 1
      },
      capabilities: {
        type: 'model-weights',
        round: round.toString(),
        weights: JSON.stringify(aggregated),
        learning_rate: '0.001',
        model_version: '1.0'
      }
    });

    coordinator.broadcast(weightsEnvelope);
    modelWeights.length = 0; // Clear
    gradientsReceived = 0;
  }

  await new Promise<void>(resolve => {
    coordinator.on('server:ready', () => {
      console.log('✅ FL Coordinator ready\n');
      resolve();
    });
  });

  // Create workers
  const workers: UMICPWebSocketPeer[] = [];
  const workerConfigs = [
    { id: 'worker-node-1', dataset: 'batch-A', samples: 1000 },
    { id: 'worker-node-2', dataset: 'batch-B', samples: 1500 },
    { id: 'worker-node-3', dataset: 'batch-C', samples: 1200 }
  ];

  console.log('Creating FL workers...\n');
  for (const config of workerConfigs) {
    const worker = new UMICPWebSocketPeer({
      peerId: config.id,
      metadata: {
        role: 'worker',
        dataset: config.dataset,
        samples: config.samples.toString()
      }
    });

    worker.on('data', async (envelope) => {
      const messageType = envelope.getCapabilities()['type'];

      if (messageType === 'model-weights') {
        const round = envelope.getCapabilities()['round'];
        console.log(`   📥 ${config.id} received weights for round ${round}`);

        // Simulate training
        console.log(`      🔄 Training on ${config.samples} local samples...`);
        await new Promise(resolve => setTimeout(resolve, 1000));

        // Generate mock gradients
        const gradients = generateMockGradients(100);
        const loss = (Math.random() * 0.5).toFixed(4);
        const accuracy = (0.7 + Math.random() * 0.2).toFixed(4);

        console.log(`      ✅ Training complete. Loss: ${loss}, Acc: ${accuracy}`);

        // Send gradients back to coordinator
        const gradientsEnvelope = new Envelope({
          from: config.id,
          to: 'fl-coordinator',
          operation: OperationType.DATA,
          messageId: `gradients-${config.id}-round-${round}-${Date.now()}`,
          payloadHint: {
            type: PayloadType.VECTOR,
            size: gradients.length,
            encoding: EncodingType.FLOAT32,
            count: 1
          },
          capabilities: {
            type: 'gradients',
            round: round,
            gradients: JSON.stringify(gradients),
            loss: loss,
            accuracy: accuracy,
            samples_trained: config.samples.toString()
          }
        });

        const peers = worker.getPeers();
        if (peers.length > 0) {
          worker.sendToPeer(peers[0].id, gradientsEnvelope);
        }
      }
    });

    await worker.connectToPeer('ws://localhost:8096/umicp');
    workers.push(worker);
  }

  await new Promise(resolve => setTimeout(resolve, 2000));
  console.log('\n✅ All workers connected and ready\n');

  // Start federated learning rounds
  console.log('═══ Starting Federated Learning Rounds ═══\n');

  for (let r = 1; r <= 3; r++) {
    console.log(`🔄 Round ${r} starting...\n`);

    // Coordinator sends initial weights
    const initialWeights = generateMockGradients(100);
    const weightsEnvelope = new Envelope({
      from: 'fl-coordinator',
      to: 'all-workers',
      operation: OperationType.DATA,
      messageId: `weights-round-${r}-${Date.now()}`,
      payloadHint: {
        type: PayloadType.VECTOR,
        size: initialWeights.length,
        encoding: EncodingType.FLOAT32,
        count: 1
      },
      capabilities: {
        type: 'model-weights',
        round: r.toString(),
        weights: JSON.stringify(initialWeights),
        learning_rate: '0.001'
      }
    });

    coordinator.broadcast(weightsEnvelope);
    console.log(`📤 Coordinator sent weights for round ${r}\n`);

    // Wait for workers to train and send gradients back
    await new Promise(resolve => setTimeout(resolve, 5000));

    console.log(`✅ Round ${r} complete\n`);
  }

  // Show final statistics
  console.log('\n📊 Federated Learning Statistics:');
  console.log(`Total rounds completed: 3`);
  console.log(`Workers participated: ${workers.length}`);
  console.log(`Total gradients exchanged: ${workers.length * 3}`);

  const coordStats = coordinator.getStats();
  console.log(`\nCoordinator connections: ${coordStats.totalPeers}`);

  // Cleanup
  console.log('\n🧹 Cleaning up...');
  for (const worker of workers) {
    await worker.shutdown();
  }
  await coordinator.shutdown();

  console.log('✅ Federated Learning example completed!\n');
}

// Helper functions
function aggregateWeights(weights: number[][]): number[] {
  if (weights.length === 0) return [];

  const aggregated = new Array(weights[0].length).fill(0);

  for (const w of weights) {
    for (let i = 0; i < w.length; i++) {
      aggregated[i] += w[i];
    }
  }

  // Average
  for (let i = 0; i < aggregated.length; i++) {
    aggregated[i] /= weights.length;
  }

  return aggregated;
}

function generateMockGradients(size: number): number[] {
  return Array.from({ length: size }, () => (Math.random() - 0.5) * 0.1);
}

// Run example
main().catch(console.error);

