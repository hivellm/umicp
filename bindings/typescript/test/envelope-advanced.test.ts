/**
 * Advanced Tests for Envelope Functionality
 *
 * Tests cover:
 * - PayloadHint usage
 * - Envelope validation edge cases
 * - Serialization/deserialization with complex data
 * - Hash generation and verification
 * - Chaining methods
 * - Error handling
 * - Operation types
 * - Encoding types
 */

import {
  Envelope,
  OperationType,
  PayloadType,
  EncodingType,
  type EnvelopeOptions,
  type PayloadHint
} from '../src/index.js';

describe('Envelope - Advanced Functionality', () => {
  describe('PayloadHint', () => {
    test('should create envelope with vector payload hint', () => {
      const payloadHint: PayloadHint = {
        type: PayloadType.VECTOR,
        size: 1536,
        encoding: EncodingType.FLOAT32,
        count: 1,
      };

      const envelope = new Envelope({
        from: 'embedding-service',
        to: 'vectordb',
        operation: OperationType.DATA,
        messageId: 'vector-1',
        payloadHint: payloadHint,
      });

      expect(envelope).toBeDefined();
      expect(envelope.validate()).toBe(true);
    });

    test('should create envelope with text payload hint', () => {
      const payloadHint: PayloadHint = {
        type: PayloadType.TEXT,
        size: 5000,
        encoding: EncodingType.UINT8, // UTF-8
      };

      const envelope = new Envelope({
        from: 'text-processor',
        to: 'nlp-service',
        operation: OperationType.DATA,
        messageId: 'text-1',
        payloadHint: payloadHint,
        capabilities: {
          'content-type': 'text/plain',
          'language': 'en',
        }
      });

      expect(envelope.validate()).toBe(true);
    });

    test('should create envelope with binary payload hint', () => {
      const payloadHint: PayloadHint = {
        type: PayloadType.BINARY,
        size: 1048576, // 1MB
        encoding: EncodingType.UINT8,
      };

      const envelope = new Envelope({
        from: 'file-service',
        to: 'storage',
        operation: OperationType.DATA,
        messageId: 'binary-1',
        payloadHint: payloadHint,
      });

      expect(envelope.validate()).toBe(true);
    });

    test('should create envelope with metadata payload hint', () => {
      const payloadHint: PayloadHint = {
        type: PayloadType.METADATA,
        size: 512,
      };

      const envelope = new Envelope({
        from: 'metadata-service',
        to: 'indexer',
        operation: OperationType.DATA,
        messageId: 'meta-1',
        payloadHint: payloadHint,
      });

      expect(envelope.validate()).toBe(true);
    });

    test('should serialize and deserialize envelope with payload hint', () => {
      const payloadHint: PayloadHint = {
        type: PayloadType.VECTOR,
        size: 768,
        encoding: EncodingType.FLOAT32,
        count: 100,
      };

      const original = new Envelope({
        from: 'ml-service',
        to: 'inference',
        operation: OperationType.DATA,
        messageId: 'ml-batch-1',
        payloadHint: payloadHint,
        capabilities: {
          'model': 'bert-base',
          'batch-size': '100',
        }
      });

      const serialized = original.serialize();
      expect(serialized).toBeTruthy();
      expect(typeof serialized).toBe('string');

      const deserialized = Envelope.deserialize(serialized);
      expect(deserialized.validate()).toBe(true);
      expect(deserialized.getFrom()).toBe('ml-service');
      expect(deserialized.getTo()).toBe('inference');
      expect(deserialized.getCapabilities()['model']).toBe('bert-base');
    });

    test('should handle multiple encoding types', () => {
      const encodings = [
        EncodingType.FLOAT32,
        EncodingType.FLOAT64,
        EncodingType.INT32,
        EncodingType.INT64,
        EncodingType.UINT8,
        EncodingType.UINT16,
        EncodingType.UINT32,
        EncodingType.UINT64,
      ];

      encodings.forEach((encoding, index) => {
        const envelope = new Envelope({
          from: 'encoder',
          to: 'decoder',
          operation: OperationType.DATA,
          messageId: `encoding-${index}`,
          payloadHint: {
            type: PayloadType.VECTOR,
            size: 256,
            encoding: encoding,
          }
        });

        expect(envelope.validate()).toBe(true);
      });
    });
  });

  describe('Envelope Validation', () => {
    test('should validate complete envelope', () => {
      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'valid-1',
        capabilities: {
          'key1': 'value1',
          'key2': 'value2',
        }
      });

      expect(envelope.validate()).toBe(true);
    });

    test('should validate minimal envelope', () => {
      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'minimal',
      });

      expect(envelope.validate()).toBe(true);
    });

    test('should handle empty capabilities', () => {
      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'empty-caps',
        capabilities: {}
      });

      expect(envelope.validate()).toBe(true);
    });

    test('should validate envelope with all operation types', () => {
      const operations = [
        OperationType.CONTROL,
        OperationType.DATA,
        OperationType.ACK,
        OperationType.ERROR,
      ];

      operations.forEach((operation, index) => {
        const envelope = new Envelope({
          from: 'sender',
          to: 'receiver',
          operation: operation,
          messageId: `op-${index}`,
        });

        expect(envelope.validate()).toBe(true);
      });
    });
  });

  describe('Method Chaining', () => {
    test('should support fluent interface', () => {
      const envelope = new Envelope()
        .setFrom('client')
        .setTo('server')
        .setOperation(OperationType.DATA)
        .setMessageId('chain-1')
        .setCapabilities({ 'test': 'chain' });

      expect(envelope.validate()).toBe(true);
      expect(envelope.getFrom()).toBe('client');
      expect(envelope.getTo()).toBe('server');
      expect(envelope.getCapabilities()['test']).toBe('chain');
    });

    test('should allow updating envelope properties', () => {
      const envelope = new Envelope({
        from: 'original',
        to: 'target',
        operation: OperationType.DATA,
        messageId: 'update-1',
      });

      envelope
        .setFrom('updated-sender')
        .setOperation(OperationType.CONTROL);

      expect(envelope.getFrom()).toBe('updated-sender');
      expect(envelope.getOperation()).toBe(OperationType.CONTROL);
    });

    test('should support incremental capability building', () => {
      const envelope = new Envelope({
        from: 'builder',
        to: 'target',
        operation: OperationType.DATA,
        messageId: 'incremental-1',
      });

      envelope.setCapabilities({ 'key1': 'value1' });
      envelope.setCapabilities({ 'key2': 'value2' });

      const caps = envelope.getCapabilities();
      // Note: setCapabilities replaces, not merges
      expect(caps['key2']).toBe('value2');
    });
  });

  describe('Hash Generation', () => {
    test('should generate consistent hash', () => {
      const envelope = new Envelope({
        from: 'hash-test',
        to: 'target',
        operation: OperationType.DATA,
        messageId: 'hash-1',
        capabilities: {
          'data': 'test-data',
        }
      });

      const hash1 = envelope.getHash();
      const hash2 = envelope.getHash();

      expect(hash1).toBe(hash2);
      expect(hash1).toBeTruthy();
      expect(typeof hash1).toBe('string');
    });

    test('should generate different hashes for different envelopes', () => {
      const envelope1 = new Envelope({
        from: 'sender1',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'hash-diff-1',
      });

      const envelope2 = new Envelope({
        from: 'sender2',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'hash-diff-2',
      });

      const hash1 = envelope1.getHash();
      const hash2 = envelope2.getHash();

      expect(hash1).not.toBe(hash2);
    });

    test('should use static hash method', () => {
      const envelope = new Envelope({
        from: 'static-hash',
        to: 'target',
        operation: OperationType.DATA,
        messageId: 'static-1',
      });

      const instanceHash = envelope.getHash();
      const staticHash = Envelope.hash(envelope);

      expect(instanceHash).toBe(staticHash);
    });
  });

  describe('Serialization Edge Cases', () => {
    test('should handle special characters in capabilities', () => {
      const envelope = new Envelope({
        from: 'special',
        to: 'chars',
        operation: OperationType.DATA,
        messageId: 'special-1',
        capabilities: {
          'emoji': '🚀',
          'unicode': 'こんにちは',
          'special': '<>&"\'',
        }
      });

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(deserialized.getCapabilities()['emoji']).toBe('🚀');
      expect(deserialized.getCapabilities()['unicode']).toBe('こんにちは');
      expect(deserialized.getCapabilities()['special']).toBe('<>&"\'');
    });

    test('should handle large capabilities', () => {
      const largeCapabilities: Record<string, string> = {};
      for (let i = 0; i < 100; i++) {
        largeCapabilities[`key${i}`] = `value${i}`.repeat(10);
      }

      const envelope = new Envelope({
        from: 'large',
        to: 'caps',
        operation: OperationType.DATA,
        messageId: 'large-1',
        capabilities: largeCapabilities,
      });

      const serialized = envelope.serialize();
      expect(serialized.length).toBeGreaterThan(1000);

      const deserialized = Envelope.deserialize(serialized);
      expect(deserialized.validate()).toBe(true);
      expect(Object.keys(deserialized.getCapabilities()).length).toBe(100);
    });

    test('should handle empty strings', () => {
      const envelope = new Envelope({
        from: '',
        to: '',
        operation: OperationType.DATA,
        messageId: '',
        capabilities: {
          'empty': '',
        }
      });

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(deserialized.getFrom()).toBe('');
      expect(deserialized.getTo()).toBe('');
      expect(deserialized.getMessageId()).toBe('');
    });
  });

  describe('Static Factory Methods', () => {
    test('should create envelope using static create', () => {
      const envelope = Envelope.create({
        from: 'factory',
        to: 'target',
        operation: OperationType.DATA,
        messageId: 'factory-1',
      });

      expect(envelope).toBeInstanceOf(Envelope);
      expect(envelope.validate()).toBe(true);
    });

    test('should serialize using static method', () => {
      const envelope = new Envelope({
        from: 'static',
        to: 'serialize',
        operation: OperationType.DATA,
        messageId: 'static-1',
      });

      const serialized = Envelope.serialize(envelope);
      expect(typeof serialized).toBe('string');
      expect(serialized.length).toBeGreaterThan(0);
    });

    test('should validate using static method', () => {
      const envelope = new Envelope({
        from: 'validate',
        to: 'static',
        operation: OperationType.DATA,
        messageId: 'validate-1',
      });

      const isValid = Envelope.validate(envelope);
      expect(isValid).toBe(true);
    });
  });

  describe('Operation Types Usage', () => {
    test('should create CONTROL envelope', () => {
      const envelope = new Envelope({
        from: 'controller',
        to: 'worker',
        operation: OperationType.CONTROL,
        messageId: 'ctrl-1',
        capabilities: {
          'command': 'shutdown',
          'graceful': 'true',
        }
      });

      expect(envelope.getOperation()).toBe(OperationType.CONTROL);
      expect(envelope.validate()).toBe(true);
    });

    test('should create ACK envelope', () => {
      const originalMessageId = 'req-123';
      const envelope = new Envelope({
        from: 'responder',
        to: 'requester',
        operation: OperationType.ACK,
        messageId: `ack-${originalMessageId}`,
        capabilities: {
          'original-message-id': originalMessageId,
          'status': 'received',
        }
      });

      expect(envelope.getOperation()).toBe(OperationType.ACK);
      expect(envelope.validate()).toBe(true);
    });

    test('should create ERROR envelope', () => {
      const envelope = new Envelope({
        from: 'service',
        to: 'client',
        operation: OperationType.ERROR,
        messageId: 'error-1',
        capabilities: {
          'error-code': '500',
          'error-message': 'Internal server error',
          'stack-trace': 'Error at ...',
        }
      });

      expect(envelope.getOperation()).toBe(OperationType.ERROR);
      expect(envelope.validate()).toBe(true);
    });
  });

  describe('Real-World Use Cases', () => {
    test('should create federated learning model update envelope', () => {
      const envelope = new Envelope({
        from: 'coordinator-node',
        to: 'worker-node-1',
        operation: OperationType.DATA,
        messageId: `fl-round-5-update-${Date.now()}`,
        payloadHint: {
          type: PayloadType.VECTOR,
          size: 50000000, // 50MB model weights
          encoding: EncodingType.FLOAT32,
          count: 12500000, // 12.5M parameters
        },
        capabilities: {
          'round': '5',
          'model-version': '2.1.0',
          'algorithm': 'fedavg',
          'compression': 'gzip',
          'learning-rate': '0.001',
        }
      });

      expect(envelope.validate()).toBe(true);
      expect(envelope.getCapabilities()['algorithm']).toBe('fedavg');
    });

    test('should create IoT sensor data envelope', () => {
      const envelope = new Envelope({
        from: 'sensor-temp-001',
        to: 'iot-gateway',
        operation: OperationType.DATA,
        messageId: `sensor-${Date.now()}`,
        payloadHint: {
          type: PayloadType.BINARY,
          size: 128,
          encoding: EncodingType.FLOAT32,
        },
        capabilities: {
          'sensor-type': 'temperature',
          'location': 'warehouse-a',
          'timestamp': new Date().toISOString(),
          'unit': 'celsius',
          'precision': '0.1',
        }
      });

      expect(envelope.validate()).toBe(true);
    });

    test('should create financial transaction envelope', () => {
      const envelope = new Envelope({
        from: 'payment-gateway',
        to: 'transaction-processor',
        operation: OperationType.DATA,
        messageId: `txn-${Date.now()}`,
        payloadHint: {
          type: PayloadType.METADATA,
          size: 2048,
        },
        capabilities: {
          'transaction-id': 'TXN-12345',
          'amount': '100.00',
          'currency': 'USD',
          'merchant-id': 'MERCH-001',
          'customer-id': 'CUST-789',
          'payment-method': 'credit-card',
          'encryption': 'aes-256',
        }
      });

      expect(envelope.validate()).toBe(true);
    });

    test('should create embedding communication envelope', () => {
      const envelope = new Envelope({
        from: 'embedding-service',
        to: 'vector-search',
        operation: OperationType.DATA,
        messageId: `embed-${Date.now()}`,
        payloadHint: {
          type: PayloadType.VECTOR,
          size: 1536, // OpenAI embedding dimension
          encoding: EncodingType.FLOAT32,
          count: 1,
        },
        capabilities: {
          'model': 'text-embedding-3-small',
          'input-type': 'text',
          'normalized': 'true',
          'query-text': 'search query example',
        }
      });

      expect(envelope.validate()).toBe(true);
      expect(envelope.getCapabilities()['model']).toBe('text-embedding-3-small');
    });

    test('should create distributed task envelope', () => {
      const envelope = new Envelope({
        from: 'task-scheduler',
        to: 'worker-pool',
        operation: OperationType.CONTROL,
        messageId: `task-${Date.now()}`,
        capabilities: {
          'task-id': 'TASK-456',
          'task-type': 'data-processing',
          'priority': 'high',
          'estimated-duration': '300',
          'resource-requirements': JSON.stringify({
            cpu: 2,
            memory: '4GB',
            gpu: false,
          }),
          'retry-policy': 'exponential-backoff',
          'max-retries': '3',
        }
      });

      expect(envelope.validate()).toBe(true);
    });
  });

  describe('Performance Characteristics', () => {
    test('should create envelopes quickly', () => {
      const start = Date.now();
      const count = 1000;

      for (let i = 0; i < count; i++) {
        const envelope = new Envelope({
          from: `sender-${i}`,
          to: `receiver-${i}`,
          operation: OperationType.DATA,
          messageId: `perf-${i}`,
        });
      }

      const elapsed = Date.now() - start;
      const perEnvelope = elapsed / count;

      expect(perEnvelope).toBeLessThan(1); // Less than 1ms per envelope
    });

    test('should serialize envelopes efficiently', () => {
      const envelopes = [];

      for (let i = 0; i < 100; i++) {
        envelopes.push(new Envelope({
          from: `sender-${i}`,
          to: `receiver-${i}`,
          operation: OperationType.DATA,
          messageId: `serial-${i}`,
          capabilities: {
            'index': i.toString(),
            'data': 'test-data'.repeat(10),
          }
        }));
      }

      const start = Date.now();

      envelopes.forEach(env => {
        const serialized = env.serialize();
        expect(serialized).toBeTruthy();
      });

      const elapsed = Date.now() - start;
      expect(elapsed).toBeLessThan(100); // Less than 100ms for 100 envelopes
    });

    test('should deserialize envelopes efficiently', () => {
      const serializedEnvelopes = [];

      for (let i = 0; i < 100; i++) {
        const env = new Envelope({
          from: `sender-${i}`,
          to: `receiver-${i}`,
          operation: OperationType.DATA,
          messageId: `deserial-${i}`,
        });
        serializedEnvelopes.push(env.serialize());
      }

      const start = Date.now();

      serializedEnvelopes.forEach(serialized => {
        const envelope = Envelope.deserialize(serialized);
        expect(envelope.validate()).toBe(true);
      });

      const elapsed = Date.now() - start;
      expect(elapsed).toBeLessThan(100); // Less than 100ms for 100 envelopes
    });
  });
});

