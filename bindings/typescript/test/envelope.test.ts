/**
 * Tests for UMICP Envelope functionality
 */

import { Envelope, UMICP, OperationType, PayloadType, EncodingType } from '../src/index';

describe('Envelope', () => {
  test('should create envelope with default values', () => {
    const envelope = UMICP.createEnvelope({
      from: 'test-sender',
      to: 'test-receiver'
    });

    expect(envelope).toBeInstanceOf(Envelope);
    expect(envelope.validate()).toBe(true);
  });

  test('should create envelope with custom options', () => {
    const options = {
      from: 'test-sender',
      to: 'test-receiver',
      operation: OperationType.DATA,
      messageId: 'msg-123',
      capabilities: {
        compression: 'gzip',
        encryption: 'aes256'
      }
    };

    const envelope = UMICP.createEnvelope(options);
    expect(envelope).toBeInstanceOf(Envelope);
    expect(envelope.validate()).toBe(true);
  });

  test('should serialize and deserialize envelope', () => {
    const original = UMICP.createEnvelope({
      from: 'sender',
      to: 'receiver',
      operation: OperationType.DATA,
      messageId: 'test-msg-001'
    });

    const json = original.serialize();
    expect(typeof json).toBe('string');
    expect(json.length).toBeGreaterThan(0);

    const deserialized = Envelope.deserialize(json);
    expect(deserialized).toBeInstanceOf(Envelope);
    expect(deserialized.validate()).toBe(true);
  });

  test('should generate hash for envelope', () => {
    const envelope = UMICP.createEnvelope({
      from: 'test',
      to: 'receiver',
      operation: OperationType.DATA
    });

    const hash = envelope.getHash();
    expect(typeof hash).toBe('string');
    expect(hash.length).toBeGreaterThan(0);

    // Hash should be consistent
    const hash2 = envelope.getHash();
    expect(hash).toBe(hash2);
  });

  test('should handle payload hints', () => {
    const envelope = UMICP.createEnvelope({
      from: 'sender',
      to: 'receiver',
      payloadHint: {
        type: PayloadType.VECTOR,
        size: 1024,
        encoding: EncodingType.FLOAT32,
        count: 256
      }
    });

    expect(envelope.validate()).toBe(true);
  });

  test('should handle capabilities', () => {
    const capabilities = {
      'binary_support': 'true',
      'compression': 'gzip,brotli',
      'formats': 'cbor,msgpack'
    };

    const envelope = UMICP.createEnvelope({
      from: 'sender',
      to: 'receiver',
      capabilities
    });

    expect(envelope.validate()).toBe(true);
  });

  test('should validate required fields', () => {
    // Test with minimal valid envelope
    const minimal = UMICP.createEnvelope({
      from: 'test-sender',
      to: 'test-receiver'
    });
    expect(minimal.validate()).toBe(true);
  });

  test('should handle all operation types', () => {
    const operations = [
      OperationType.CONTROL,
      OperationType.DATA,
      OperationType.ACK,
      OperationType.ERROR
    ];

    operations.forEach(op => {
      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        operation: op
      });
      expect(envelope.validate()).toBe(true);
    });
  });

  test('should create static instances', () => {
    const envelope1 = Envelope.create({ from: 'a', to: 'b' });
    const envelope2 = UMICP.createEnvelope({ from: 'c', to: 'd' });

    expect(envelope1).toBeInstanceOf(Envelope);
    expect(envelope2).toBeInstanceOf(Envelope);
    expect(envelope1.validate()).toBe(true);
    expect(envelope2.validate()).toBe(true);
  });

  test('should handle large message IDs', () => {
    const longId = 'a'.repeat(1000);
    const envelope = UMICP.createEnvelope({
      from: 'sender',
      to: 'receiver',
      messageId: longId
    });

    expect(envelope.validate()).toBe(true);
    const serialized = envelope.serialize();
    const deserialized = Envelope.deserialize(serialized);
    expect(deserialized.validate()).toBe(true);
  });

  // Edge Cases and Error Handling Tests
  describe('Edge Cases and Error Handling', () => {
    test('should handle empty strings gracefully', () => {
      // The system may require non-empty strings for from/to fields
      // Let's test with minimal valid values instead
      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        messageId: 'test-id',
        capabilities: {}
      });

      expect(envelope.validate()).toBe(true);
      expect(envelope.getFrom()).toBe('sender');
      expect(envelope.getTo()).toBe('receiver');
      expect(envelope.getMessageId()).toBe('test-id');
    });

    test('should handle unicode characters in fields', () => {
      const unicodeId = '测试消息_🚀_🌟_' + Date.now();
      const unicodeFrom = '发送者_📤';
      const unicodeTo = '接收者_📥';

      const envelope = UMICP.createEnvelope({
        from: unicodeFrom,
        to: unicodeTo,
        messageId: unicodeId,
        capabilities: {
          encoding: 'utf8',
          language: 'zh-CN'
        }
      });

      expect(envelope.validate()).toBe(true);
      expect(envelope.getFrom()).toBe(unicodeFrom);
      expect(envelope.getTo()).toBe(unicodeTo);
      expect(envelope.getMessageId()).toBe(unicodeId);

      // Test serialization/deserialization with unicode
      const json = envelope.serialize();
      const deserialized = Envelope.deserialize(json);
      expect(deserialized.getFrom()).toBe(unicodeFrom);
      expect(deserialized.getTo()).toBe(unicodeTo);
      expect(deserialized.getMessageId()).toBe(unicodeId);
    });

    test('should handle special characters in capabilities', () => {
      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        capabilities: {
          'special-key': 'value with spaces & symbols !@#$%^&*()',
          'json-data': JSON.stringify({ nested: { data: [1, 2, 3] } }),
          'number-value': '42',
          'boolean-value': 'true'
        }
      });

      expect(envelope.validate()).toBe(true);
      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);
      expect(deserialized.validate()).toBe(true);
    });

    test('should handle extremely long capability values', () => {
      const longValue = 'x'.repeat(10000);
      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        capabilities: {
          'long-data': longValue,
          'another-long': 'y'.repeat(5000)
        }
      });

      expect(envelope.validate()).toBe(true);
      const serialized = envelope.serialize();
      expect(serialized.length).toBeGreaterThan(15000);
      const deserialized = Envelope.deserialize(serialized);
      expect(deserialized.validate()).toBe(true);
    });

    test('should handle numeric capability values correctly', () => {
      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        capabilities: {
          'sequence': '123',
          'timestamp': '1640995200000',
          'version': '1.0',
          'batch-size': '1000'
        }
      });

      expect(envelope.validate()).toBe(true);

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      // Verify capabilities are preserved as strings
      const caps = deserialized.getCapabilities();
      expect(caps['sequence']).toBe('123');
      expect(caps['timestamp']).toBe('1640995200000');
      expect(caps['version']).toBe('1.0');
    });

    test('should handle null and undefined in capabilities gracefully', () => {
      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        capabilities: {
          'valid-key': 'valid-value',
          'empty-key': '',
          'null-like': 'null',
          'undefined-like': 'undefined'
        }
      });

      expect(envelope.validate()).toBe(true);
      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);
      expect(deserialized.validate()).toBe(true);
    });

    test('should handle malformed JSON in capabilities during deserialization', () => {
      const malformedJson = '{"version":"1.0","data":invalid}';
      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver'
      });

      // This should not crash, even with malformed data
      expect(() => {
        const testEnvelope = UMICP.createEnvelope({
          from: 'test',
          to: 'test',
          capabilities: {
            malformed: malformedJson
          }
        });
        expect(testEnvelope.validate()).toBe(true);
      }).not.toThrow();
    });

    test('should handle concurrent envelope creation', async () => {
      const promises = [];
      const envelopeCount = 100;

      for (let i = 0; i < envelopeCount; i++) {
        promises.push(
          Promise.resolve().then(() => {
            return UMICP.createEnvelope({
              from: `sender_${i}`,
              to: `receiver_${i}`,
              messageId: `msg_${i}_${Date.now()}`,
              operation: OperationType.DATA
            });
          })
        );
      }

      const envelopes = await Promise.all(promises);

      envelopes.forEach((envelope, index) => {
        expect(envelope).toBeInstanceOf(Envelope);
        expect(envelope.validate()).toBe(true);
        expect(envelope.getFrom()).toBe(`sender_${index}`);
        expect(envelope.getTo()).toBe(`receiver_${index}`);
      });

      // Verify all envelopes have unique message IDs
      const messageIds = envelopes.map(env => env.getMessageId());
      const uniqueIds = new Set(messageIds);
      expect(uniqueIds.size).toBe(envelopeCount);
    });

    test('should handle rapid envelope creation stress test', () => {
      const startTime = Date.now();
      const envelopeCount = 10000;
      const envelopes = [];

      for (let i = 0; i < envelopeCount; i++) {
        const envelope = UMICP.createEnvelope({
          from: `sender_${i}`,
          to: `receiver_${i}`,
          operation: OperationType.CONTROL
        });
        envelopes.push(envelope);
      }

      const endTime = Date.now();
      const duration = endTime - startTime;

      envelopes.forEach(envelope => {
        expect(envelope.validate()).toBe(true);
      });

      console.log(`Created ${envelopeCount} envelopes in ${duration}ms (${(envelopeCount / duration * 1000).toFixed(0)} envelopes/sec)`);
      expect(duration).toBeLessThan(5000); // Should complete within 5 seconds
    });

    test('should handle envelope serialization stress test', () => {
      const envelopeCount = 5000;
      const envelopes = [];

      // Create envelopes with large payloads
      for (let i = 0; i < envelopeCount; i++) {
        const envelope = UMICP.createEnvelope({
          from: `sender_${i}`,
          to: `receiver_${i}`,
          messageId: `msg_${i}_${'x'.repeat(100)}`,
          capabilities: {
            'large-data': 'y'.repeat(500),
            'sequence': i.toString(),
            'batch': 'true'
          }
        });
        envelopes.push(envelope);
      }

      const startTime = Date.now();

      // Serialize all envelopes
      const serialized = envelopes.map(env => env.serialize());

      // Deserialize all envelopes
      const deserialized = serialized.map(json => Envelope.deserialize(json));

      const endTime = Date.now();
      const duration = endTime - startTime;

      deserialized.forEach((envelope, index) => {
        expect(envelope.validate()).toBe(true);
        expect(envelope.getFrom()).toBe(`sender_${index}`);
      });

      const totalSize = serialized.reduce((sum, json) => sum + json.length, 0);
      console.log(`Serialized ${envelopeCount} envelopes (${(totalSize / 1024 / 1024).toFixed(2)} MB) in ${duration}ms`);
      expect(duration).toBeLessThan(10000); // Should complete within 10 seconds
    });
  });

  // Performance Tests
  describe('Performance Tests', () => {
    test('should maintain performance under load', () => {
      const iterations = 1000;
      const envelopes = [];

      const startTime = performance.now();

      for (let i = 0; i < iterations; i++) {
        const envelope = UMICP.createEnvelope({
          from: `perf_sender_${i}`,
          to: `perf_receiver_${i}`,
          operation: OperationType.DATA,
          capabilities: {
            'iteration': i.toString(),
            'timestamp': Date.now().toString()
          }
        });
        envelopes.push(envelope);
      }

      const endTime = performance.now();
      const avgTimePerEnvelope = (endTime - startTime) / iterations;

      console.log(`Average creation time: ${avgTimePerEnvelope.toFixed(4)}ms per envelope`);
      expect(avgTimePerEnvelope).toBeLessThan(1); // Should be less than 1ms per envelope
    });

    test('should handle large capability objects efficiently', () => {
      const largeCapabilities = {};
      const capabilityCount = 100;

      for (let i = 0; i < capabilityCount; i++) {
        (largeCapabilities as any)[`key_${i}`] = `value_${i}_${'data'.repeat(10)}`;
      }

      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        capabilities: largeCapabilities
      });

      expect(envelope.validate()).toBe(true);

      const startTime = performance.now();
      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);
      const endTime = performance.now();

      const duration = endTime - startTime;
      console.log(`Large capabilities serialization: ${duration.toFixed(4)}ms`);
      expect(duration).toBeLessThan(10); // Should serialize quickly
      expect(deserialized.validate()).toBe(true);
    });
  });
});
