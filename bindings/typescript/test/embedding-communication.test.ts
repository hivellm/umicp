/**
 * Tests for Embedding Communication Example
 *
 * Unit tests for the embedding communication functionality
 */

import { EmbeddingSerializer, TransformerCommunication } from '../examples/embedding-communication';

describe('Embedding Communication Tests', () => {
  let serializer: EmbeddingSerializer;
  let communication: TransformerCommunication;

  beforeEach(() => {
    serializer = new EmbeddingSerializer();
    communication = new TransformerCommunication();
  });

  describe('Embedding Serializer', () => {
    test('should create BERT-style embeddings with correct dimensions', () => {
      const data = serializer.createBertEmbedding(128, 768);

      expect(data.modelName).toBe('bert-base-uncased');
      expect(data.layerIndex).toBe(11);
      expect(data.sequenceLength).toBe(128);
      expect(data.embeddingDim).toBe(768);
      expect(data.embeddings.length).toBe(128 * 768);
      expect(data.attentionMask).toBeDefined();
      expect(data.tokenIds).toBeDefined();
      expect(data.metadata).toBeDefined();
    });

    test('should create GPT-style attention weights', () => {
      const sequenceLength = 64;
      const numHeads = 8;
      const attention = serializer.createGptAttention(sequenceLength, numHeads);

      const expectedSize = sequenceLength * sequenceLength * numHeads;
      expect(attention.length).toBe(expectedSize);
      expect(attention.every(val => typeof val === 'number')).toBe(true);
    });

    test('should create T5 encoder embeddings', () => {
      const data = serializer.createT5EncoderOutput(256, 512);

      expect(data.modelName).toBe('t5-encoder');
      expect(data.layerIndex).toBe(11);
      expect(data.sequenceLength).toBe(256);
      expect(data.embeddingDim).toBe(512);
      expect(data.embeddings.length).toBe(256 * 512);
    });
  });

  describe('Transformer Communication', () => {
    test('should create embedding transfer envelope', () => {
      const embeddingData = serializer.createBertEmbedding(64, 384);
      const envelope = communication.createEmbeddingEnvelope(
        'bert-encoder',
        'classifier',
        embeddingData
      );

      expect(envelope.from()).toBe('bert-encoder');
      expect(envelope.to()).toBe('classifier');
      expect(envelope.getCapabilities()['message_type']).toBe('embedding_transfer');
      expect(envelope.getCapabilities()['model_name']).toBe('bert-base-uncased');
    });

    test('should create attention sharing envelope', () => {
      const attentionWeights = new Float32Array(1024);
      const envelope = communication.createAttentionEnvelope(
        'gpt-decoder',
        'attention-aggregator',
        attentionWeights,
        32,
        1
      );

      expect(envelope.from()).toBe('gpt-decoder');
      expect(envelope.to()).toBe('attention-aggregator');
      expect(envelope.getCapabilities()['message_type']).toBe('attention_sharing');
      expect(envelope.getCapabilities()['sequence_length']).toBe('32');
      expect(envelope.getCapabilities()['num_heads']).toBe('1');
    });

    test('should find similar embeddings', () => {
      const embeddingDim = 128;
      const databaseSize = 100;

      // Create database
      const database: Float32Array[] = [];
      for (let i = 0; i < databaseSize; i++) {
        const embedding = new Float32Array(embeddingDim);
        for (let j = 0; j < embeddingDim; j++) {
          embedding[j] = Math.sin(i * 0.1 + j * 0.01);
        }
        database.push(embedding);
      }

      // Create query (similar to first embedding)
      const query = new Float32Array(embeddingDim);
      for (let j = 0; j < embeddingDim; j++) {
        query[j] = Math.sin(0.05 + j * 0.01);
      }

      const similar = communication.findSimilarEmbeddings(query, database, 3);

      expect(similar.length).toBe(3);
      expect(similar[0].similarity).toBeGreaterThan(similar[1].similarity);
      expect(similar[0].index).toBeGreaterThanOrEqual(0);
      expect(similar[0].index).toBeLessThan(databaseSize);
    });

    test('should aggregate embeddings correctly', () => {
      const embeddingDim = 64;
      const numEmbeddings = 5;

      const embeddings: Float32Array[] = [];
      for (let i = 0; i < numEmbeddings; i++) {
        const embedding = new Float32Array(embeddingDim);
        for (let j = 0; j < embeddingDim; j++) {
          embedding[j] = i + 1; // Simple values for predictable averaging
        }
        embeddings.push(embedding);
      }

      const aggregated = communication.aggregateEmbeddings(embeddings);

      expect(aggregated.length).toBe(embeddingDim);
      // Expected average: (1+2+3+4+5)/5 = 3
      expect(aggregated[0]).toBeCloseTo(3.0, 5);
    });

    test('should handle edge cases in embedding operations', () => {
      // Test with empty database
      expect(() => {
        communication.findSimilarEmbeddings(new Float32Array(10), [], 5);
      }).not.toThrow();

      // Test with single embedding
      const singleEmbedding = [new Float32Array([1, 2, 3])];
      const results = communication.findSimilarEmbeddings(
        new Float32Array([1, 2, 3]),
        singleEmbedding,
        5
      );
      expect(results.length).toBe(1);
      expect(results[0].similarity).toBeCloseTo(1.0, 5);
    });
  });

  describe('Performance Benchmarks', () => {
    test('should perform embedding operations within time limits', () => {
      const startTime = Date.now();

      // Create and process multiple embeddings
      for (let i = 0; i < 10; i++) {
        const embedding = serializer.createBertEmbedding(64, 384);
        const envelope = communication.createEmbeddingEnvelope(
          'test-source',
          'test-target',
          embedding
        );

        // Verify envelope was created successfully
        expect(envelope.from()).toBe('test-source');
      }

      const endTime = Date.now();
      const duration = endTime - startTime;

      // Should complete within reasonable time (adjust based on system)
      expect(duration).toBeLessThan(5000); // 5 seconds max
    });

    test('should handle large embedding databases efficiently', () => {
      const embeddingDim = 256;
      const databaseSize = 500;

      // Create large database
      const database: Float32Array[] = [];
      for (let i = 0; i < databaseSize; i++) {
        const embedding = new Float32Array(embeddingDim);
        for (let j = 0; j < embeddingDim; j++) {
          embedding[j] = Math.random();
        }
        database.push(embedding);
      }

      const query = new Float32Array(embeddingDim);
      for (let j = 0; j < embeddingDim; j++) {
        query[j] = Math.random();
      }

      const startTime = Date.now();
      const similar = communication.findSimilarEmbeddings(query, database, 10);
      const endTime = Date.now();

      const duration = endTime - startTime;

      expect(similar.length).toBe(10);
      expect(duration).toBeLessThan(10000); // Should complete within 10 seconds

    });
  });

  describe('Error Handling', () => {
    test('should handle mismatched embedding dimensions', () => {
      const embedding1 = new Float32Array([1, 2, 3]);
      const embedding2 = new Float32Array([4, 5]); // Different dimension

      expect(() => {
        communication.findSimilarEmbeddings(embedding1, [embedding2], 1);
      }).toThrow();
    });

    test('should handle empty embedding aggregation', () => {
      expect(() => {
        communication.aggregateEmbeddings([]);
      }).toThrow('No embeddings to aggregate');
    });

    test('should handle invalid envelope processing', () => {
      // Create envelope without proper embedding data
      const invalidEnvelope = {
        from: () => 'test',
        to: () => 'test',
        getCapabilities: () => ({ message_type: 'invalid_type' })
      };

      expect(() => {
        communication.processEmbeddingEnvelope(invalidEnvelope as any);
      }).toThrow('Not an embedding transfer envelope');
    });
  });
});
