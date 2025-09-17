/**
 * UMICP TypeScript Bindings - Embedding Communication Example
 *
 * Demonstrates efficient communication of AI embeddings and transformer data
 * using binary serialization and optimized transport protocols.
 */

import { Envelope, Matrix, UMICP, OperationType, PayloadType, EncodingType } from '../src/index.js';

interface EmbeddingData {
  modelName: string;
  layerIndex: number;
  sequenceLength: number;
  embeddingDim: number;
  embeddings: Float32Array;
  attentionMask?: Uint8Array;
  tokenIds?: Int32Array;
  metadata: {
    batchSize: number;
    maxSeqLength: number;
    vocabSize: number;
    modelConfig: Record<string, any>;
  };
}

interface TransformerState {
  layerStates: Array<{
    attentionWeights: Float32Array;
    keyCache: Float32Array;
    valueCache: Float32Array;
    feedForwardOutput: Float32Array;
  }>;
  finalEmbeddings: Float32Array;
  classificationHead?: Float32Array;
}

interface ModelCommunication {
  sourceModel: string;
  targetModel: string;
  communicationType: 'embedding_transfer' | 'attention_sharing' | 'gradient_sync' | 'kv_cache_sync';
  data: EmbeddingData | TransformerState;
  timestamp: number;
  sequenceId: string;
}

class EmbeddingSerializer {
  private matrix: any;

  constructor() {
    this.matrix = UMICP.createMatrix();
  }

  // Serialize embedding data into efficient binary format
  serializeEmbedding(data: EmbeddingData): ArrayBuffer {
    const bufferSize = this.calculateEmbeddingBufferSize(data);
    const buffer = new ArrayBuffer(bufferSize);
    const view = new DataView(buffer);

    let offset = 0;

    // Write header
    const header = new TextEncoder().encode('UMICP_EMB');
    new Uint8Array(buffer, offset, header.length).set(header);
    offset += header.length;

    // Write metadata
    view.setUint32(offset, data.layerIndex, true);
    offset += 4;
    view.setUint32(offset, data.sequenceLength, true);
    offset += 4;
    view.setUint32(offset, data.embeddingDim, true);
    offset += 4;

    // Write model name
    const modelNameBytes = new TextEncoder().encode(data.modelName);
    view.setUint16(offset, modelNameBytes.length, true);
    offset += 2;
    new Uint8Array(buffer, offset, modelNameBytes.length).set(modelNameBytes);
    offset += modelNameBytes.length;

    // Write embeddings (float32 array)
    new Float32Array(buffer, offset, data.embeddings.length).set(data.embeddings);
    offset += data.embeddings.length * 4;

    // Write attention mask if present
    if (data.attentionMask) {
      view.setUint8(offset, 1); // has attention mask
      offset += 1;
      new Uint8Array(buffer, offset, data.attentionMask.length).set(data.attentionMask);
      offset += data.attentionMask.length;
    } else {
      view.setUint8(offset, 0); // no attention mask
      offset += 1;
    }

    // Write token IDs if present
    if (data.tokenIds) {
      view.setUint8(offset, 1); // has token IDs
      offset += 1;
      new Int32Array(buffer, offset, data.tokenIds.length).set(data.tokenIds);
      offset += data.tokenIds.length * 4;
    } else {
      view.setUint8(offset, 0); // no token IDs
      offset += 1;
    }

    // Write metadata JSON
    const metadataJson = JSON.stringify(data.metadata);
    const metadataBytes = new TextEncoder().encode(metadataJson);
    view.setUint32(offset, metadataBytes.length, true);
    offset += 4;
    new Uint8Array(buffer, offset, metadataBytes.length).set(metadataBytes);

    return buffer;
  }

  // Deserialize embedding data from binary format
  deserializeEmbedding(buffer: ArrayBuffer): EmbeddingData {
    const view = new DataView(buffer);
    let offset = 0;

    // Read and verify header
    const header = new Uint8Array(buffer, offset, 9);
    const headerStr = new TextDecoder().decode(header);
    if (headerStr !== 'UMICP_EMB') {
      throw new Error('Invalid embedding format');
    }
    offset += 9;

    // Read metadata
    const layerIndex = view.getUint32(offset, true);
    offset += 4;
    const sequenceLength = view.getUint32(offset, true);
    offset += 4;
    const embeddingDim = view.getUint32(offset, true);
    offset += 4;

    // Read model name
    const modelNameLength = view.getUint16(offset, true);
    offset += 2;
    const modelName = new TextDecoder().decode(
      new Uint8Array(buffer, offset, modelNameLength)
    );
    offset += modelNameLength;

    // Read embeddings
    const embeddingCount = sequenceLength * embeddingDim;
    const embeddings = new Float32Array(buffer, offset, embeddingCount);
    offset += embeddingCount * 4;

    // Read attention mask if present
    let attentionMask: Uint8Array | undefined;
    const hasAttentionMask = view.getUint8(offset);
    offset += 1;
    if (hasAttentionMask) {
      attentionMask = new Uint8Array(buffer, offset, sequenceLength);
      offset += sequenceLength;
    }

    // Read token IDs if present
    let tokenIds: Int32Array | undefined;
    const hasTokenIds = view.getUint8(offset);
    offset += 1;
    if (hasTokenIds) {
      tokenIds = new Int32Array(buffer, offset, sequenceLength);
      offset += sequenceLength * 4;
    }

    // Read metadata
    const metadataLength = view.getUint32(offset, true);
    offset += 4;
    const metadataJson = new TextDecoder().decode(
      new Uint8Array(buffer, offset, metadataLength)
    );
    const metadata = JSON.parse(metadataJson);

    return {
      modelName,
      layerIndex,
      sequenceLength,
      embeddingDim,
      embeddings: new Float32Array(embeddings), // Copy to avoid reference issues
      attentionMask,
      tokenIds,
      metadata
    };
  }

  private calculateEmbeddingBufferSize(data: EmbeddingData): number {
    let size = 0;

    // Header (9 bytes)
    size += 9;

    // Metadata (4 + 4 + 4 bytes)
    size += 12;

    // Model name (2 + length)
    size += 2 + data.modelName.length;

    // Embeddings (4 bytes per float)
    size += data.embeddings.length * 4;

    // Attention mask flag (1 byte)
    size += 1;
    if (data.attentionMask) {
      size += data.attentionMask.length;
    }

    // Token IDs flag (1 byte)
    size += 1;
    if (data.tokenIds) {
      size += data.tokenIds.length * 4;
    }

    // Metadata (4 + JSON length)
    const metadataJson = JSON.stringify(data.metadata);
    size += 4 + metadataJson.length;

    return size;
  }
}

class TransformerCommunication {
  private serializer: EmbeddingSerializer;
  private matrix: any;

  constructor() {
    this.serializer = new EmbeddingSerializer();
    this.matrix = UMICP.createMatrix();
  }

  // Create embedding transfer envelope
  createEmbeddingEnvelope(
    sourceModel: string,
    targetModel: string,
    embeddingData: EmbeddingData
  ): any {
    const binaryData = this.serializer.serializeEmbedding(embeddingData);

    return UMICP.createEnvelope({
      from: sourceModel,
      to: targetModel,
      operation: OperationType.DATA,
      messageId: `embedding-${Date.now()}-${Math.random().toString(36).substr(2, 6)}`,
      capabilities: {
        'message_type': 'embedding_transfer',
        'model_name': embeddingData.modelName,
        'layer_index': embeddingData.layerIndex.toString(),
        'sequence_length': embeddingData.sequenceLength.toString(),
        'embedding_dim': embeddingData.embeddingDim.toString(),
        'data_size': binaryData.byteLength.toString(),
        'compression': 'none',
        'format': 'binary'
      },
      payloadHint: {
        type: PayloadType.VECTOR,
        size: binaryData.byteLength,
        encoding: EncodingType.BINARY,
        count: 1
      }
    });
  }

  // Create attention sharing envelope
  createAttentionEnvelope(
    sourceModel: string,
    targetModel: string,
    attentionWeights: Float32Array,
    sequenceLength: number,
    numHeads: number
  ): any {
    return UMICP.createEnvelope({
      from: sourceModel,
      to: targetModel,
      operation: OperationType.DATA,
      messageId: `attention-${Date.now()}-${Math.random().toString(36).substr(2, 6)}`,
      capabilities: {
        'message_type': 'attention_sharing',
        'sequence_length': sequenceLength.toString(),
        'num_heads': numHeads.toString(),
        'attention_size': attentionWeights.length.toString(),
        'attention_shape': `${sequenceLength}x${sequenceLength}x${numHeads}`
      },
      payloadHint: {
        type: PayloadType.VECTOR,
        size: attentionWeights.length * 4,
        encoding: EncodingType.FLOAT32,
        count: attentionWeights.length
      }
    });
  }

  // Create KV cache synchronization envelope
  createKVCacheEnvelope(
    sourceModel: string,
    targetModel: string,
    keyCache: Float32Array,
    valueCache: Float32Array,
    layerIndex: number,
    sequencePos: number
  ): any {
    return UMICP.createEnvelope({
      from: sourceModel,
      to: targetModel,
      operation: OperationType.DATA,
      messageId: `kv-cache-${Date.now()}-${Math.random().toString(36).substr(2, 6)}`,
      capabilities: {
        'message_type': 'kv_cache_sync',
        'layer_index': layerIndex.toString(),
        'sequence_position': sequencePos.toString(),
        'key_cache_size': keyCache.length.toString(),
        'value_cache_size': valueCache.length.toString(),
        'cache_format': 'float32'
      },
      payloadHint: {
        type: PayloadType.VECTOR,
        size: (keyCache.length + valueCache.length) * 4,
        encoding: EncodingType.FLOAT32,
        count: keyCache.length + valueCache.length
      }
    });
  }

  // Process incoming embedding envelope
  processEmbeddingEnvelope(envelope: any): EmbeddingData {
    const capabilities = envelope.getCapabilities();

    if (capabilities['message_type'] !== 'embedding_transfer') {
      throw new Error('Not an embedding transfer envelope');
    }

    // In a real implementation, you'd extract the binary data from the envelope
    // For this example, we'll create mock data
    const sequenceLength = parseInt(capabilities['sequence_length']);
    const embeddingDim = parseInt(capabilities['embedding_dim']);
    const layerIndex = parseInt(capabilities['layer_index']);

    // Create mock embedding data
    const embeddings = new Float32Array(sequenceLength * embeddingDim);
    for (let i = 0; i < embeddings.length; i++) {
      embeddings[i] = Math.sin(i * 0.1) * 0.1;
    }

    return {
      modelName: capabilities['model_name'],
      layerIndex,
      sequenceLength,
      embeddingDim,
      embeddings,
      metadata: {
        batchSize: 1,
        maxSeqLength: sequenceLength,
        vocabSize: 30000,
        modelConfig: {
          hiddenSize: embeddingDim,
          numLayers: 12,
          numHeads: 12
        }
      }
    };
  }

  // Perform embedding similarity search
  findSimilarEmbeddings(
    queryEmbedding: Float32Array,
    candidateEmbeddings: Float32Array[],
    topK: number = 5
  ): Array<{index: number, similarity: number}> {
    const similarities: Array<{index: number, similarity: number}> = [];

    for (let i = 0; i < candidateEmbeddings.length; i++) {
      const similarity = this.matrix.cosineSimilarity(queryEmbedding, candidateEmbeddings[i]);
      if (similarity.success && similarity.similarity !== undefined) {
        similarities.push({
          index: i,
          similarity: similarity.similarity
        });
      }
    }

    // Sort by similarity (descending)
    similarities.sort((a, b) => b.similarity - a.similarity);

    return similarities.slice(0, topK);
  }

  // Aggregate embeddings from multiple sources
  aggregateEmbeddings(embeddings: Float32Array[]): Float32Array {
    if (embeddings.length === 0) {
      throw new Error('No embeddings to aggregate');
    }

    const embeddingDim = embeddings[0].length;
    const aggregated = new Float32Array(embeddingDim);

    // Simple averaging
    for (const embedding of embeddings) {
      if (embedding.length !== embeddingDim) {
        throw new Error('All embeddings must have the same dimension');
      }

      for (let i = 0; i < embeddingDim; i++) {
        aggregated[i] += embedding[i];
      }
    }

    // Normalize the result
    for (let i = 0; i < embeddingDim; i++) {
      aggregated[i] /= embeddings.length;
    }

    return aggregated;
  }
}

async function embeddingCommunicationExample() {

  const communication = new TransformerCommunication();

  // Example 1: BERT-style embedding transfer

  const bertEmbedding: EmbeddingData = {
    modelName: 'bert-base-uncased',
    layerIndex: 11, // Last layer
    sequenceLength: 128,
    embeddingDim: 768,
    embeddings: new Float32Array(128 * 768),
    attentionMask: new Uint8Array(128),
    tokenIds: new Int32Array(128),
    metadata: {
      batchSize: 1,
      maxSeqLength: 512,
      vocabSize: 30522,
      modelConfig: {
        hiddenSize: 768,
        numLayers: 12,
        numHeads: 12,
        intermediateSize: 3072
      }
    }
  };

  // Initialize with sample data
  for (let i = 0; i < bertEmbedding.embeddings.length; i++) {
    bertEmbedding.embeddings[i] = Math.sin(i * 0.001) * 0.1;
  }
  for (let i = 0; i < 128; i++) {
    bertEmbedding.attentionMask![i] = i < 64 ? 1 : 0; // First 64 tokens are real
    bertEmbedding.tokenIds![i] = Math.floor(Math.random() * 30522);
  }

  // Create and serialize envelope
  const embeddingEnvelope = communication.createEmbeddingEnvelope(
    'bert-encoder',
    'classification-head',
    bertEmbedding
  );


  // Example 2: GPT-style attention sharing

  const sequenceLength = 1024;
  const numHeads = 16;
  const attentionSize = sequenceLength * sequenceLength * numHeads;
  const attentionWeights = new Float32Array(attentionSize);

  // Initialize attention weights with some pattern
  for (let i = 0; i < attentionSize; i++) {
    attentionWeights[i] = Math.exp(-Math.abs(i % sequenceLength - (i / sequenceLength))) * 0.1;
  }

  const attentionEnvelope = communication.createAttentionEnvelope(
    'gpt-decoder',
    'attention-aggregator',
    attentionWeights,
    sequenceLength,
    numHeads
  );


  // Example 3: T5-style encoder-decoder communication

  // Encoder output
  const encoderEmbedding: EmbeddingData = {
    modelName: 't5-encoder',
    layerIndex: 11,
    sequenceLength: 256,
    embeddingDim: 512,
    embeddings: new Float32Array(256 * 512),
    metadata: {
      batchSize: 1,
      maxSeqLength: 512,
      vocabSize: 32100,
      modelConfig: {
        hiddenSize: 512,
        numLayers: 12,
        numHeads: 8,
        dKv: 64,
        dFf: 2048
      }
    }
  };

  // Initialize encoder embeddings
  for (let i = 0; i < encoderEmbedding.embeddings.length; i++) {
    encoderEmbedding.embeddings[i] = Math.cos(i * 0.01) * 0.05;
  }

  const encoderEnvelope = communication.createEmbeddingEnvelope(
    't5-encoder',
    't5-decoder',
    encoderEmbedding
  );


  // Example 4: Embedding similarity search

  // Create a database of embeddings
  const embeddingDatabase: Float32Array[] = [];
  const databaseSize = 1000;
  const embeddingDim = 384; // Sentence transformer size

  for (let i = 0; i < databaseSize; i++) {
    const embedding = new Float32Array(embeddingDim);
    for (let j = 0; j < embeddingDim; j++) {
      embedding[j] = Math.sin(i * 0.1 + j * 0.01) * 0.1;
    }
    embeddingDatabase.push(embedding);
  }

  // Query embedding (similar to first in database)
  const queryEmbedding = new Float32Array(embeddingDim);
  for (let j = 0; j < embeddingDim; j++) {
    queryEmbedding[j] = Math.sin(0.05 + j * 0.01) * 0.1; // Slightly different
  }

  const startTime = process.hrtime.bigint();
  const similarResults = communication.findSimilarEmbeddings(queryEmbedding, embeddingDatabase, 5);
  const endTime = process.hrtime.bigint();

  const searchTime = Number(endTime - startTime) / 1e9;


  for (let i = 0; i < similarResults.length; i++) {
  }

  // Example 5: Federated learning embedding aggregation

  // Simulate embeddings from multiple clients
  const clientEmbeddings: Float32Array[] = [];
  const numClients = 5;
  const sharedEmbeddingDim = 512;

  for (let client = 0; client < numClients; client++) {
    const clientEmbedding = new Float32Array(sharedEmbeddingDim);
    for (let i = 0; i < sharedEmbeddingDim; i++) {
      // Add some client-specific variation
      clientEmbedding[i] = Math.sin(i * 0.01) * 0.1 + (client * 0.01);
    }
    clientEmbeddings.push(clientEmbedding);
  }

  const aggregatedEmbedding = communication.aggregateEmbeddings(clientEmbeddings);

    Math.sqrt(aggregatedEmbedding.reduce((sum, val) => sum + val * val, 0)).toFixed(4)
  }`);

}

// Run example if executed directly
if (require.main === module) {
  embeddingCommunicationExample().catch(console.error);
}

export { EmbeddingSerializer, TransformerCommunication, embeddingCommunicationExample };
