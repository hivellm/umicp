/**
 * UMICP TypeScript Bindings - Federated Learning Example
 * Demonstrates distributed machine learning using UMICP envelopes
 */

import { Envelope, Matrix, UMICP, OperationType, PayloadType, EncodingType } from '../src/index.js';

interface ModelWeights {
  layer1: Float32Array;
  layer2: Float32Array;
  bias: Float32Array;
  version: string;
  clientId: string;
}

interface TrainingMetrics {
  loss: number;
  accuracy: number;
  samplesProcessed: number;
  trainingTime: number;
}

class FederatedLearningCoordinator {
  private matrix: any;
  private globalModel: ModelWeights;
  private clients: Map<string, { lastUpdate: number; metrics: TrainingMetrics[] }> = new Map();

  constructor() {
    this.matrix = UMICP.createMatrix();
    this.globalModel = this.initializeGlobalModel();
  }

  private initializeGlobalModel(): ModelWeights {
    const layer1Size = 784 * 128; // Input -> Hidden
    const layer2Size = 128 * 10;  // Hidden -> Output
    const biasSize = 10;          // Output bias

    return {
      layer1: new Float32Array(layer1Size).map(() => (Math.random() - 0.5) * 0.1),
      layer2: new Float32Array(layer2Size).map(() => (Math.random() - 0.5) * 0.1),
      bias: new Float32Array(biasSize).map(() => (Math.random() - 0.5) * 0.1),
      version: '1.0.0',
      clientId: 'coordinator'
    };
  }

  // Handle model weight aggregation from clients
  async handleModelUpdate(envelope: any): Promise<any> {
    console.log(`📨 Received model update from ${envelope.from()}`);

    const clientId = envelope.from();
    const updateData = JSON.parse(envelope.capabilities()['model_update']);

    // Aggregate weights using federated averaging
    await this.aggregateWeights(updateData.weights, updateData.metrics);

    // Update client tracking
    const metrics: TrainingMetrics = updateData.metrics;
    if (!this.clients.has(clientId)) {
      this.clients.set(clientId, { lastUpdate: Date.now(), metrics: [] });
    }
    this.clients.get(clientId)!.metrics.push(metrics);

    // Create acknowledgment with updated global model
    const response = UMICP.createEnvelope({
      from: 'federated-coordinator',
      to: clientId,
      operation: OperationType.RESPONSE,
      messageId: `ack-${Date.now()}`,
      capabilities: {
        'status': 'weights_aggregated',
        'global_model_version': this.globalModel.version,
        'aggregation_round': envelope.capabilities()['round'],
        'clients_aggregated': this.clients.size.toString()
      }
    });

    return response;
  }

  private async aggregateWeights(clientWeights: ModelWeights, metrics: TrainingMetrics): Promise<void> {
    const learningRate = 0.1;

    console.log(`🔄 Aggregating weights from ${clientWeights.clientId}`);

    // Simple federated averaging (FedAvg)
    // In practice, you'd use weighted averaging based on client data size
    const clientWeight = metrics.samplesProcessed / 1000; // Normalize by batch size

    // Aggregate layer 1
    for (let i = 0; i < this.globalModel.layer1.length; i++) {
      const diff = clientWeights.layer1[i] - this.globalModel.layer1[i];
      this.globalModel.layer1[i] += diff * learningRate * clientWeight;
    }

    // Aggregate layer 2
    for (let i = 0; i < this.globalModel.layer2.length; i++) {
      const diff = clientWeights.layer2[i] - this.globalModel.layer2[i];
      this.globalModel.layer2[i] += diff * learningRate * clientWeight;
    }

    // Aggregate bias
    for (let i = 0; i < this.globalModel.bias.length; i++) {
      const diff = clientWeights.bias[i] - this.globalModel.bias[i];
      this.globalModel.bias[i] += diff * learningRate * clientWeight;
    }

    console.log(`✅ Weights aggregated (learning rate: ${learningRate}, weight: ${clientWeight.toFixed(3)})`);
  }

  // Send global model to clients
  createModelBroadcast(targetClients: string[]): any[] {
    const envelopes = [];

    for (const clientId of targetClients) {
      const envelope = UMICP.createEnvelope({
        from: 'federated-coordinator',
        to: clientId,
        operation: OperationType.DATA,
        messageId: `model-${Date.now()}-${clientId}`,
        capabilities: {
          'message_type': 'global_model_update',
          'model_version': this.globalModel.version,
          'training_round': (this.clients.size + 1).toString(),
          'layer1_size': this.globalModel.layer1.length.toString(),
          'layer2_size': this.globalModel.layer2.length.toString(),
          'bias_size': this.globalModel.bias.length.toString()
        },
        payloadHint: {
          type: PayloadType.VECTOR,
          size: this.globalModel.layer1.length + this.globalModel.layer2.length + this.globalModel.bias.length,
          encoding: EncodingType.FLOAT32,
          count: 1
        }
      });

      envelopes.push(envelope);
    }

    return envelopes;
  }

  // Get federation statistics
  getFederationStats(): any {
    const totalClients = this.clients.size;
    const totalSamples = Array.from(this.clients.values())
      .reduce((sum, client) => sum + client.metrics.reduce((s, m) => s + m.samplesProcessed, 0), 0);

    const avgLoss = Array.from(this.clients.values())
      .reduce((sum, client) => sum + client.metrics.reduce((s, m) => s + m.loss, 0), 0) /
      Array.from(this.clients.values()).reduce((sum, client) => sum + client.metrics.length, 0);

    const avgAccuracy = Array.from(this.clients.values())
      .reduce((sum, client) => sum + client.metrics.reduce((s, m) => s + m.accuracy, 0), 0) /
      Array.from(this.clients.values()).reduce((sum, client) => sum + client.metrics.length, 0);

    return {
      totalClients,
      totalSamples,
      averageLoss: avgLoss || 0,
      averageAccuracy: avgAccuracy || 0,
      globalModelVersion: this.globalModel.version
    };
  }
}

class FederatedLearningClient {
  private matrix: any;
  private localModel: ModelWeights;
  private trainingData: Float32Array[];
  private trainingLabels: number[];

  constructor(clientId: string) {
    this.matrix = UMICP.createMatrix();
    this.localModel = this.initializeLocalModel(clientId);
    this.trainingData = this.generateTrainingData();
    this.trainingLabels = this.generateTrainingLabels();
  }

  private initializeLocalModel(clientId: string): ModelWeights {
    const layer1Size = 784 * 128;
    const layer2Size = 128 * 10;
    const biasSize = 10;

    return {
      layer1: new Float32Array(layer1Size).map(() => (Math.random() - 0.5) * 0.1),
      layer2: new Float32Array(layer2Size).map(() => (Math.random() - 0.5) * 0.1),
      bias: new Float32Array(biasSize).map(() => (Math.random() - 0.5) * 0.1),
      version: '0.1.0',
      clientId
    };
  }

  private generateTrainingData(): Float32Array[] {
    // Generate synthetic MNIST-like data
    const samples = 100;
    const data: Float32Array[] = [];

    for (let i = 0; i < samples; i++) {
      const sample = new Float32Array(784);
      for (let j = 0; j < 784; j++) {
        sample[j] = Math.random();
      }
      data.push(sample);
    }

    return data;
  }

  private generateTrainingLabels(): number[] {
    const labels = [];
    for (let i = 0; i < 100; i++) {
      labels.push(Math.floor(Math.random() * 10));
    }
    return labels;
  }

  // Perform local training
  async performLocalTraining(globalWeights: ModelWeights): Promise<TrainingMetrics> {
    console.log(`🤖 Client ${this.localModel.clientId} starting local training...`);

    // Copy global weights to local model
    this.localModel.layer1.set(globalWeights.layer1);
    this.localModel.layer2.set(globalWeights.layer2);
    this.localModel.bias.set(globalWeights.bias);

    const startTime = Date.now();

    // Simple training simulation
    const learningRate = 0.01;
    let totalLoss = 0;

    for (let epoch = 0; epoch < 5; epoch++) {
      let epochLoss = 0;

      for (let i = 0; i < this.trainingData.length; i++) {
        const input = this.trainingData[i];
        const target = this.trainingLabels[i];

        // Forward pass (simplified)
        const hidden = new Float32Array(128);
        const output = new Float32Array(10);

        // Matrix multiplication: input * weights + bias
        this.matrix.multiply(input, this.localModel.layer1, hidden, 1, 784, 128);
        this.matrix.multiply(hidden, this.localModel.layer2, output, 1, 128, 10);

        // Add bias
        for (let j = 0; j < 10; j++) {
          output[j] += this.localModel.bias[j];
        }

        // Calculate loss (cross-entropy)
        const maxOutput = Math.max(...output);
        const expOutputs = output.map(x => Math.exp(x - maxOutput));
        const sumExp = expOutputs.reduce((a, b) => a + b, 0);
        const probabilities = expOutputs.map(x => x / sumExp);

        const loss = -Math.log(probabilities[target]);
        epochLoss += loss;

        // Backward pass (simplified gradient descent)
        const gradient = new Float32Array(10);
        for (let j = 0; j < 10; j++) {
          gradient[j] = probabilities[j] - (j === target ? 1 : 0);
        }

        // Update weights (simplified)
        for (let j = 0; j < this.localModel.bias.length; j++) {
          this.localModel.bias[j] -= learningRate * gradient[j];
        }
      }

      totalLoss += epochLoss / this.trainingData.length;
    }

    const trainingTime = Date.now() - startTime;
    const avgLoss = totalLoss / 5;

    // Calculate accuracy (simplified)
    const accuracy = Math.max(0, 1 - avgLoss);

    const metrics: TrainingMetrics = {
      loss: avgLoss,
      accuracy,
      samplesProcessed: this.trainingData.length,
      trainingTime
    };

    console.log(`✅ Training completed - Loss: ${avgLoss.toFixed(4)}, Accuracy: ${(accuracy * 100).toFixed(2)}%`);

    return metrics;
  }

  // Create model update envelope
  createModelUpdateEnvelope(metrics: TrainingMetrics, round: number): any {
    const updateData = {
      weights: this.localModel,
      metrics,
      round
    };

    return UMICP.createEnvelope({
      from: this.localModel.clientId,
      to: 'federated-coordinator',
      operation: OperationType.DATA,
      messageId: `update-${Date.now()}`,
      capabilities: {
        'message_type': 'model_update',
        'model_update': JSON.stringify(updateData),
        'round': round.toString(),
        'training_samples': metrics.samplesProcessed.toString(),
        'loss': metrics.loss.toString(),
        'accuracy': metrics.accuracy.toString()
      }
    });
  }
}

async function federatedLearningExample() {
  console.log('🧠 UMICP Federated Learning Example');
  console.log('=====================================');

  // Initialize coordinator
  const coordinator = new FederatedLearningCoordinator();

  // Initialize clients
  const clients = [
    new FederatedLearningClient('client-001'),
    new FederatedLearningClient('client-002'),
    new FederatedLearningClient('client-003')
  ];

  console.log(`🚀 Starting federated learning with ${clients.length} clients`);
  console.log();

  // Simulate multiple rounds of federated learning
  const rounds = 3;

  for (let round = 1; round <= rounds; round++) {
    console.log(`🔄 Round ${round}/${rounds}`);
    console.log('-------------------');

    // Broadcast global model to clients
    const modelEnvelopes = coordinator.createModelBroadcast(clients.map(c => c.localModel.clientId));
    console.log(`📤 Broadcasted global model to ${modelEnvelopes.length} clients`);

    // Each client performs local training
    const updateEnvelopes = [];

    for (const client of clients) {
      // Simulate receiving global model
      const globalModel = coordinator.globalModel;

      // Perform local training
      const metrics = await client.performLocalTraining(globalModel);

      // Create update envelope
      const updateEnvelope = client.createModelUpdateEnvelope(metrics, round);
      updateEnvelopes.push(updateEnvelope);
    }

    // Coordinator aggregates updates
    for (const updateEnvelope of updateEnvelopes) {
      await coordinator.handleModelUpdate(updateEnvelope);
    }

    // Show round statistics
    const stats = coordinator.getFederationStats();
    console.log(`📊 Round ${round} Stats:`);
    console.log(`   Clients: ${stats.totalClients}`);
    console.log(`   Total samples: ${stats.totalSamples}`);
    console.log(`   Avg loss: ${stats.averageLoss.toFixed(4)}`);
    console.log(`   Avg accuracy: ${(stats.averageAccuracy * 100).toFixed(2)}%`);
    console.log();
  }

  // Final results
  const finalStats = coordinator.getFederationStats();
  console.log('🎉 Federated Learning Complete!');
  console.log('================================');
  console.log(`📊 Final Results:`);
  console.log(`   Total clients: ${finalStats.totalClients}`);
  console.log(`   Total training samples: ${finalStats.totalSamples}`);
  console.log(`   Final average loss: ${finalStats.averageLoss.toFixed(4)}`);
  console.log(`   Final average accuracy: ${(finalStats.averageAccuracy * 100).toFixed(2)}%`);
  console.log(`   Global model version: ${finalStats.globalModelVersion}`);
  console.log();
  console.log('💡 Key Benefits of UMICP in Federated Learning:');
  console.log('   • Efficient serialization of model weights');
  console.log('   • Secure transport of sensitive ML data');
  console.log('   • Type-safe message handling');
  console.log('   • High-performance matrix operations for training');
  console.log('   • Scalable client-server communication');
}

// Run example if executed directly
if (require.main === module) {
  federatedLearningExample().catch(console.error);
}

export { FederatedLearningCoordinator, FederatedLearningClient, federatedLearningExample };

