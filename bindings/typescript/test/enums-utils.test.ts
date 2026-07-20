/**
 * Tests for UMICP Enums and Utility Functions
 */

import {
  UMICP,
  OperationType,
  PayloadType,
  EncodingType,
  Envelope,
  Matrix
} from '../src/index';

describe('OperationType Enum', () => {
  test('should have all operation types defined', () => {
    expect(OperationType.CONTROL).toBeDefined();
    expect(OperationType.DATA).toBeDefined();
    expect(OperationType.ACK).toBeDefined();
    expect(OperationType.ERROR).toBeDefined();
  });

  test('should have correct numeric values', () => {
    expect(OperationType.CONTROL).toBe(0);
    expect(OperationType.DATA).toBe(1);
    expect(OperationType.ACK).toBe(2);
    expect(OperationType.ERROR).toBe(3);
  });

  test('should be usable in switch statements', () => {
    const getOpName = (op: OperationType): string => {
      switch (op) {
        case OperationType.CONTROL:
          return 'control';
        case OperationType.DATA:
          return 'data';
        case OperationType.ACK:
          return 'ack';
        case OperationType.ERROR:
          return 'error';
        default:
          return 'unknown';
      }
    };

    expect(getOpName(OperationType.CONTROL)).toBe('control');
    expect(getOpName(OperationType.DATA)).toBe('data');
    expect(getOpName(OperationType.ACK)).toBe('ack');
    expect(getOpName(OperationType.ERROR)).toBe('error');
  });

  test('should work with envelope creation', () => {
    for (const opType of [
      OperationType.CONTROL,
      OperationType.DATA,
      OperationType.ACK,
      OperationType.ERROR
    ]) {
      const envelope = UMICP.createEnvelope({
        from: 'test',
        to: 'test',
        operation: opType
      });

      expect(envelope.getOperation()).toBe(opType);
    }
  });
});

describe('PayloadType Enum', () => {
  test('should have all payload types defined', () => {
    expect(PayloadType.VECTOR).toBeDefined();
    expect(PayloadType.TEXT).toBeDefined();
    expect(PayloadType.METADATA).toBeDefined();
    expect(PayloadType.BINARY).toBeDefined();
  });

  test('should have correct numeric values', () => {
    expect(PayloadType.VECTOR).toBe(0);
    expect(PayloadType.TEXT).toBe(1);
    expect(PayloadType.METADATA).toBe(2);
    expect(PayloadType.BINARY).toBe(3);
  });

  test('should be usable in payload hints', () => {
    for (const payloadType of [
      PayloadType.VECTOR,
      PayloadType.TEXT,
      PayloadType.METADATA,
      PayloadType.BINARY
    ]) {
      const envelope = UMICP.createEnvelope({
        from: 'test',
        to: 'test',
        payloadHint: { type: payloadType }
      });

      expect(envelope).toBeDefined();
    }
  });

  test('should work in type guards', () => {
    const isVectorPayload = (type: PayloadType): boolean => {
      return type === PayloadType.VECTOR;
    };

    expect(isVectorPayload(PayloadType.VECTOR)).toBe(true);
    expect(isVectorPayload(PayloadType.TEXT)).toBe(false);
  });
});

describe('EncodingType Enum', () => {
  test('should have all encoding types defined', () => {
    expect(EncodingType.FLOAT32).toBeDefined();
    expect(EncodingType.FLOAT64).toBeDefined();
    expect(EncodingType.INT32).toBeDefined();
    expect(EncodingType.INT64).toBeDefined();
    expect(EncodingType.UINT8).toBeDefined();
    expect(EncodingType.UINT16).toBeDefined();
    expect(EncodingType.UINT32).toBeDefined();
    expect(EncodingType.UINT64).toBeDefined();
  });

  test('should have correct numeric values', () => {
    expect(EncodingType.FLOAT32).toBe(0);
    expect(EncodingType.FLOAT64).toBe(1);
    expect(EncodingType.INT32).toBe(2);
    expect(EncodingType.INT64).toBe(3);
    expect(EncodingType.UINT8).toBe(4);
    expect(EncodingType.UINT16).toBe(5);
    expect(EncodingType.UINT32).toBe(6);
    expect(EncodingType.UINT64).toBe(7);
  });

  test('should work in payload hints', () => {
    for (const encoding of [
      EncodingType.FLOAT32,
      EncodingType.FLOAT64,
      EncodingType.INT32,
      EncodingType.INT64,
      EncodingType.UINT8,
      EncodingType.UINT16,
      EncodingType.UINT32,
      EncodingType.UINT64
    ]) {
      const envelope = UMICP.createEnvelope({
        from: 'test',
        to: 'test',
        payloadHint: { encoding }
      });

      expect(envelope).toBeDefined();
    }
  });

  test('should be usable for type selection', () => {
    const getBytesPerElement = (encoding: EncodingType): number => {
      switch (encoding) {
        case EncodingType.UINT8:
          return 1;
        case EncodingType.UINT16:
          return 2;
        case EncodingType.FLOAT32:
        case EncodingType.INT32:
        case EncodingType.UINT32:
          return 4;
        case EncodingType.FLOAT64:
        case EncodingType.INT64:
        case EncodingType.UINT64:
          return 8;
        default:
          return 0;
      }
    };

    expect(getBytesPerElement(EncodingType.UINT8)).toBe(1);
    expect(getBytesPerElement(EncodingType.UINT16)).toBe(2);
    expect(getBytesPerElement(EncodingType.FLOAT32)).toBe(4);
    expect(getBytesPerElement(EncodingType.FLOAT64)).toBe(8);
  });
});

describe('UMICP Utility Object', () => {
  test('should have version information', () => {
    expect(UMICP.version).toBeDefined();
    expect(typeof UMICP.version).toBe('string');
    expect(UMICP.UMICP_VERSION).toBeDefined();
  });

  test('should report transport availability', () => {
    expect(typeof UMICP.hasWebSocketTransport).toBe('boolean');
    expect(typeof UMICP.hasHTTP2Transport).toBe('boolean');
  });

  test('should have factory methods', () => {
    expect(typeof UMICP.createEnvelope).toBe('function');
    expect(typeof UMICP.createMatrix).toBe('function');
  });

  test('should create envelope via utility', () => {
    const envelope = UMICP.createEnvelope({
      from: 'test',
      to: 'test'
    });

    expect(envelope).toBeInstanceOf(Envelope);
  });

  test('should create matrix via utility', () => {
    const matrix = UMICP.createMatrix();
    expect(matrix).toBeInstanceOf(Matrix);
  });

  test('should have enum references', () => {
    expect(UMICP.OperationType).toBe(OperationType);
    expect(UMICP.PayloadType).toBe(PayloadType);
    expect(UMICP.EncodingType).toBe(EncodingType);
  });
});

describe('Envelope Static Methods', () => {
  test('should create envelope using static method', () => {
    const envelope = Envelope.create({
      from: 'sender',
      to: 'receiver'
    });

    expect(envelope).toBeInstanceOf(Envelope);
    expect(envelope.validate()).toBe(true);
  });

  test('should serialize using static method', () => {
    const envelope = Envelope.create({
      from: 'sender',
      to: 'receiver'
    });

    const json = Envelope.serialize(envelope);
    expect(typeof json).toBe('string');
    expect(json.length).toBeGreaterThan(0);
  });

  test('should deserialize using static method', () => {
    const original = Envelope.create({
      from: 'sender',
      to: 'receiver'
    });

    const json = Envelope.serialize(original);
    const restored = Envelope.deserialize(json);

    expect(restored).toBeInstanceOf(Envelope);
    expect(restored.validate()).toBe(true);
  });

  test('should validate using static method', () => {
    const validEnvelope = Envelope.create({
      from: 'sender',
      to: 'receiver'
    });

    expect(Envelope.validate(validEnvelope)).toBe(true);
  });

  test('should hash using static method', () => {
    const envelope = Envelope.create({
      from: 'sender',
      to: 'receiver',
      messageId: 'test-123'
    });

    const hash = Envelope.hash(envelope);
    expect(typeof hash).toBe('string');
    expect(hash.length).toBeGreaterThan(0);
  });
});

describe('Envelope Fluent Interface', () => {
  test('should chain setter methods', () => {
    const envelope = new Envelope()
      .setFrom('sender')
      .setTo('receiver')
      .setOperation(OperationType.DATA)
      .setMessageId('msg-001')
      .setCapabilities({ test: 'value' })
      .setPayloadHint({ type: PayloadType.TEXT, size: 100 });

    expect(envelope.getFrom()).toBe('sender');
    expect(envelope.getTo()).toBe('receiver');
    expect(envelope.getOperation()).toBe(OperationType.DATA);
    expect(envelope.getMessageId()).toBe('msg-001');
    expect(envelope.getCapabilities().test).toBe('value');
  });

  test('should support method chaining after deserialize', () => {
    const original = new Envelope()
      .setFrom('sender')
      .setTo('receiver');

    const json = original.serialize();

    const restored = new Envelope()
      .deserialize(json)
      .setCapabilities({ new: 'capability' });

    expect(restored.getCapabilities().new).toBe('capability');
  });
});

describe('Edge Cases', () => {
  test('should handle empty capabilities', () => {
    const envelope = UMICP.createEnvelope({
      from: 'test',
      to: 'test',
      capabilities: {}
    });

    const caps = envelope.getCapabilities();
    expect(caps).toBeDefined();
    expect(Object.keys(caps).length).toBe(0);
  });

  test('should handle empty payload hint', () => {
    const envelope = UMICP.createEnvelope({
      from: 'test',
      to: 'test',
      payloadHint: {}
    });

    expect(envelope).toBeDefined();
    expect(envelope.validate()).toBe(true);
  });

  test('should handle minimal envelope options', () => {
    const envelope = UMICP.createEnvelope({
      from: 'a',
      to: 'b'
    });

    expect(envelope.validate()).toBe(true);
  });

  test('should handle long strings', () => {
    const longString = 'a'.repeat(10000);

    const envelope = UMICP.createEnvelope({
      from: longString,
      to: 'test'
    });

    expect(envelope.getFrom()).toBe(longString);
    expect(envelope.getFrom().length).toBe(10000);
  });

  test('should handle special characters', () => {
    const envelope = UMICP.createEnvelope({
      from: 'user@domain.com',
      to: 'peer-123_test',
      messageId: 'msg-äöü-😀'
    });

    expect(envelope.getFrom()).toBe('user@domain.com');
    expect(envelope.getTo()).toBe('peer-123_test');
  });

  test('should handle numeric-like strings', () => {
    const envelope = UMICP.createEnvelope({
      from: '12345',
      to: '67890',
      messageId: '00000'
    });

    expect(envelope.getFrom()).toBe('12345');
    expect(envelope.getTo()).toBe('67890');
  });

  test('should handle many capabilities', () => {
    const manyCaps: Record<string, string> = {};
    for (let i = 0; i < 100; i++) {
      manyCaps[`key${i}`] = `value${i}`;
    }

    const envelope = UMICP.createEnvelope({
      from: 'test',
      to: 'test',
      capabilities: manyCaps
    });

    const caps = envelope.getCapabilities();
    expect(Object.keys(caps).length).toBe(100);
    expect(caps['key42']).toBe('value42');
  });
});

describe('Type Safety', () => {
  test('should accept valid OperationType values', () => {
    const operations: OperationType[] = [
      OperationType.CONTROL,
      OperationType.DATA,
      OperationType.ACK,
      OperationType.ERROR
    ];

    operations.forEach(op => {
      const envelope = new Envelope().setOperation(op);
      expect(envelope.getOperation()).toBe(op);
    });
  });

  test('should accept valid PayloadType values', () => {
    const types: PayloadType[] = [
      PayloadType.VECTOR,
      PayloadType.TEXT,
      PayloadType.METADATA,
      PayloadType.BINARY
    ];

    types.forEach(type => {
      const envelope = UMICP.createEnvelope({
        from: 'test',
        to: 'test',
        payloadHint: { type }
      });
      expect(envelope).toBeDefined();
    });
  });

  test('should accept valid EncodingType values', () => {
    const encodings: EncodingType[] = [
      EncodingType.FLOAT32,
      EncodingType.FLOAT64,
      EncodingType.INT32,
      EncodingType.INT64,
      EncodingType.UINT8,
      EncodingType.UINT16,
      EncodingType.UINT32,
      EncodingType.UINT64
    ];

    encodings.forEach(encoding => {
      const envelope = UMICP.createEnvelope({
        from: 'test',
        to: 'test',
        payloadHint: { encoding }
      });
      expect(envelope).toBeDefined();
    });
  });
});

