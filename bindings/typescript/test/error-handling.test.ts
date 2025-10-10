/**
 * Error handling tests for UMICP TypeScript bindings
 */

import { Envelope, Matrix, UMICP, OperationType } from '../src/index';

describe('Envelope Error Handling', () => {
  describe('Construction Errors', () => {
    test('should handle creation without native addon', () => {
      // This tests the mock path when native addon isn't available
      expect(() => {
        new Envelope();
      }).not.toThrow();
    });

    test('should handle invalid options gracefully', () => {
      expect(() => {
        UMICP.createEnvelope({
          from: '',
          to: ''
        });
      }).not.toThrow();
    });
  });

  describe('Serialization Errors', () => {
    test('should handle serialization of minimal envelope', () => {
      const envelope = new Envelope({
        from: 'test',
        to: 'test'
      });

      expect(() => {
        envelope.serialize();
      }).not.toThrow();
    });

    test('should handle serialization with special characters', () => {
      const envelope = new Envelope({
        from: 'test@domain.com',
        to: 'peer-123_test'
      });

      expect(() => {
        const json = envelope.serialize();
        expect(json).toBeDefined();
      }).not.toThrow();
    });

    test('should handle serialization with unicode', () => {
      const envelope = new Envelope({
        from: 'sender-日本語',
        to: 'receiver-中文'
      });

      expect(() => {
        envelope.serialize();
      }).not.toThrow();
    });
  });

  describe('Deserialization Errors', () => {
    test('should handle empty string deserialization', () => {
      const envelope = new Envelope();

      expect(() => {
        envelope.deserialize('');
      }).toThrow();
    });

    test('should handle invalid JSON deserialization', () => {
      const envelope = new Envelope();

      expect(() => {
        envelope.deserialize('{invalid json}');
      }).toThrow();
    });

    test('should handle null deserialization', () => {
      expect(() => {
        Envelope.deserialize(null as any);
      }).toThrow();
    });

    test('should handle undefined deserialization', () => {
      expect(() => {
        Envelope.deserialize(undefined as any);
      }).toThrow();
    });

    test('should handle deserialization of non-string', () => {
      expect(() => {
        Envelope.deserialize(123 as any);
      }).toThrow();
    });

    test('should handle deserialization of object', () => {
      expect(() => {
        Envelope.deserialize({ from: 'test' } as any);
      }).toThrow();
    });
  });

  describe('Validation Errors', () => {
    test('should validate envelope without required fields', () => {
      const envelope = new Envelope();

      const isValid = envelope.validate();
      expect(typeof isValid).toBe('boolean');
    });

    test('should validate envelope with only from', () => {
      const envelope = new Envelope({ from: 'test' });

      const isValid = envelope.validate();
      expect(typeof isValid).toBe('boolean');
    });

    test('should validate envelope with only to', () => {
      const envelope = new Envelope({ to: 'test' });

      const isValid = envelope.validate();
      expect(typeof isValid).toBe('boolean');
    });
  });

  describe('Method Call Errors', () => {
    test('should handle getFrom on new envelope', () => {
      const envelope = new Envelope();

      expect(() => {
        const from = envelope.getFrom();
        expect(from).toBeDefined();
      }).not.toThrow();
    });

    test('should handle getTo on new envelope', () => {
      const envelope = new Envelope();

      expect(() => {
        const to = envelope.getTo();
        expect(to).toBeDefined();
      }).not.toThrow();
    });

    test('should handle getCapabilities on new envelope', () => {
      const envelope = new Envelope();

      expect(() => {
        const caps = envelope.getCapabilities();
        expect(caps).toBeDefined();
      }).not.toThrow();
    });

    test('should handle getHash on new envelope', () => {
      const envelope = new Envelope({ from: 'test', to: 'test' });

      expect(() => {
        const hash = envelope.getHash();
        expect(hash).toBeDefined();
      }).not.toThrow();
    });
  });
});

describe('Matrix Error Handling', () => {
  let matrix: Matrix;

  beforeEach(() => {
    matrix = new Matrix();
  });

  describe('Null/Undefined Inputs', () => {
    test('should handle null arrays in dotProduct', () => {
      expect(() => {
        matrix.dotProduct(null as any, null as any);
      }).toThrow();
    });

    test('should handle undefined arrays in dotProduct', () => {
      expect(() => {
        matrix.dotProduct(undefined as any, undefined as any);
      }).toThrow();
    });

    test('should handle mixed null/valid in dotProduct', () => {
      const valid = new Float32Array([1, 2, 3]);

      expect(() => {
        matrix.dotProduct(valid, null as any);
      }).toThrow();
    });
  });

  describe('Invalid Array Types', () => {
    test('should handle regular arrays instead of Float32Array', () => {
      const result = matrix.dotProduct([1, 2, 3] as any, [4, 5, 6] as any);

      expect(result).toBeDefined();
      expect(result.success).toBeDefined();
    });

    test('should handle empty arrays', () => {
      const a = new Float32Array([]);
      const b = new Float32Array([]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
    });

    test('should handle Int32Array instead of Float32Array', () => {
      const result = matrix.dotProduct(
        new Int32Array([1, 2, 3]) as any,
        new Int32Array([4, 5, 6]) as any
      );

      expect(result).toBeDefined();
    });
  });

  describe('Size Mismatches', () => {
    test('should handle different sized arrays in dotProduct', () => {
      const a = new Float32Array([1, 2, 3]);
      const b = new Float32Array([1, 2]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
      expect(typeof result.success).toBe('boolean');
    });

    test('should handle wrong size result array in add', () => {
      const a = new Float32Array([1, 2, 3, 4]);
      const b = new Float32Array([1, 2, 3, 4]);
      const result = new Float32Array(2); // Wrong size

      const res = matrix.add(a, b, result, 2, 2);
      expect(res).toBeDefined();
    });

    test('should handle wrong dimensions in multiply', () => {
      const a = new Float32Array([1, 2, 3, 4]);
      const b = new Float32Array([1, 2, 3, 4]);
      const result = new Float32Array([1, 2, 3, 4]);

      // Incompatible dimensions
      const res = matrix.multiply(a, b, result, 2, 2, 3);
      expect(res).toBeDefined();
    });
  });

  describe('Invalid Dimensions', () => {
    test('should handle negative dimensions', () => {
      const a = new Float32Array([1, 2, 3, 4]);
      const b = new Float32Array([1, 2, 3, 4]);
      const result = new Float32Array(4);

      const res = matrix.add(a, b, result, -2, 2);
      expect(res).toBeDefined();
      expect(typeof res.success).toBe('boolean');
    });

    test('should handle zero dimensions', () => {
      const a = new Float32Array([1, 2, 3, 4]);
      const b = new Float32Array([1, 2, 3, 4]);
      const result = new Float32Array(4);

      const res = matrix.add(a, b, result, 0, 0);
      expect(res).toBeDefined();
    });

    test('should handle huge dimensions', () => {
      const a = new Float32Array([1, 2, 3, 4]);
      const b = new Float32Array([1, 2, 3, 4]);
      const result = new Float32Array(4);

      const res = matrix.add(a, b, result, 1000000, 1000000);
      expect(res).toBeDefined();
    });
  });

  describe('Special Float Values', () => {
    test('should handle NaN values', () => {
      const a = new Float32Array([NaN, 2, 3]);
      const b = new Float32Array([1, 2, 3]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
    });

    test('should handle Infinity', () => {
      const a = new Float32Array([Infinity, 2, 3]);
      const b = new Float32Array([1, 2, 3]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
    });

    test('should handle -Infinity', () => {
      const a = new Float32Array([-Infinity, 2, 3]);
      const b = new Float32Array([1, 2, 3]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
    });

    test('should handle mixed special values', () => {
      const a = new Float32Array([NaN, Infinity, -Infinity]);
      const b = new Float32Array([1, 2, 3]);

      const result = matrix.dotProduct(a, b);
      expect(result).toBeDefined();
    });
  });

  describe('Result Object Structure', () => {
    test('should always return object with success field', () => {
      const a = new Float32Array([1, 2, 3]);
      const b = new Float32Array([4, 5, 6]);

      const result = matrix.dotProduct(a, b);

      expect(result).toHaveProperty('success');
      expect(typeof result.success).toBe('boolean');
    });

    test('should return error message on failure', () => {
      // Force a failure with mismatched sizes
      const a = new Float32Array([1, 2, 3]);
      const b = new Float32Array([1, 2]);

      const result = matrix.dotProduct(a, b);

      if (!result.success) {
        expect(result).toHaveProperty('error');
        expect(typeof result.error).toBe('string');
      }
    });

    test('should handle normalize with zero vector', () => {
      const vector = new Float32Array([0, 0, 0]);

      const result = matrix.normalize(vector, 1, 3);
      expect(result).toBeDefined();
      expect(result).toHaveProperty('success');
    });
  });
});

describe('Transport Error Handling', () => {
  describe('WebSocket Transport', () => {
    test('should throw when WebSocket not available', () => {
      if (!UMICP.hasWebSocketTransport) {
        expect(() => {
          UMICP.createWebSocketTransport('ws://localhost:8080');
        }).toThrow('WebSocket transport not available');
      }
    });

    test('should not throw when checking availability', () => {
      expect(() => {
        const available = UMICP.hasWebSocketTransport;
        expect(typeof available).toBe('boolean');
      }).not.toThrow();
    });
  });

  describe('HTTP2 Transport', () => {
    test('should throw when HTTP2 not available', () => {
      if (!UMICP.hasHTTP2Transport) {
        expect(() => {
          UMICP.createHTTP2Transport('http://localhost:8080');
        }).toThrow('HTTP/2 transport not available');
      }
    });

    test('should not throw when checking availability', () => {
      expect(() => {
        const available = UMICP.hasHTTP2Transport;
        expect(typeof available).toBe('boolean');
      }).not.toThrow();
    });
  });
});

describe('Edge Cases and Boundary Conditions', () => {
  test('should handle empty string in envelope fields', () => {
    expect(() => {
      const envelope = new Envelope({
        from: '',
        to: ''
      });
      expect(envelope).toBeDefined();
    }).not.toThrow();
  });

  test('should handle very long strings', () => {
    const longString = 'a'.repeat(100000);

    expect(() => {
      const envelope = new Envelope({
        from: longString,
        to: 'test'
      });
      expect(envelope).toBeDefined();
    }).not.toThrow();
  });

  test('should handle capabilities with special keys', () => {
    expect(() => {
      const envelope = new Envelope({
        from: 'test',
        to: 'test',
        capabilities: {
          '': 'empty key',
          'key with spaces': 'value',
          '日本語': 'unicode key'
        }
      });
      expect(envelope).toBeDefined();
    }).not.toThrow();
  });

  test('should handle multiple envelope instances', () => {
    const envelopes = [];

    for (let i = 0; i < 100; i++) {
      envelopes.push(new Envelope({
        from: `sender-${i}`,
        to: `receiver-${i}`
      }));
    }

    expect(envelopes.length).toBe(100);
  });

  test('should handle multiple matrix instances', () => {
    const matrices = [];

    for (let i = 0; i < 100; i++) {
      matrices.push(new Matrix());
    }

    expect(matrices.length).toBe(100);
  });
});

describe('Type Coercion and Conversion', () => {
  test('should handle number as string for operation', () => {
    expect(() => {
      const envelope = new Envelope({
        from: 'test',
        to: 'test',
        operation: 1 as any
      });
      expect(envelope).toBeDefined();
    }).not.toThrow();
  });

  test('should handle undefined optional fields', () => {
    expect(() => {
      const envelope = new Envelope({
        from: 'test',
        to: 'test',
        messageId: undefined,
        capabilities: undefined
      });
      expect(envelope).toBeDefined();
    }).not.toThrow();
  });

  test('should handle payloadHint with partial fields', () => {
    expect(() => {
      const envelope = new Envelope({
        from: 'test',
        to: 'test',
        payloadHint: {
          size: 100
          // Other fields undefined
        }
      });
      expect(envelope).toBeDefined();
    }).not.toThrow();
  });
});

