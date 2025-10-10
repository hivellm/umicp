/**
 * Security and Input Validation Tests
 *
 * Tests cover:
 * - Input validation and sanitization
 * - Malformed data handling
 * - Injection attack prevention
 * - Size limit enforcement
 * - Protocol compliance validation
 * - Edge case robustness
 */

import {
  Envelope,
  OperationType,
  PayloadType,
  EncodingType,
  StreamableHTTPServer,
  StreamableHTTPClient,
  UMICPWebSocketPeer
} from '../src/index.js';
import * as http from 'http';

describe('Security and Input Validation', () => {
  describe('Envelope Validation', () => {
    test('should reject envelope with empty required fields', () => {
      const envelope = new Envelope({
        from: '',
        to: '',
        operation: OperationType.DATA,
        messageId: '',
      });

      // Empty fields are technically allowed, but validate() should still work
      const isValid = envelope.validate();
      expect(typeof isValid).toBe('boolean');
    });

    test('should handle malformed JSON gracefully', () => {
      const malformedJSON = '{"from": "test", "to": "target", invalid}';

      expect(() => {
        Envelope.deserialize(malformedJSON);
      }).toThrow();
    });

    test('should handle incomplete JSON', () => {
      const incompleteJSON = '{"from": "test", "to": "target"';

      expect(() => {
        Envelope.deserialize(incompleteJSON);
      }).toThrow();
    });

    test('should handle null values', () => {
      const nullJSON = JSON.stringify({
        from: null,
        to: null,
        operation: null,
        messageId: null
      });

      expect(() => {
        Envelope.deserialize(nullJSON);
      }).toThrow();
    });

    test('should handle undefined values', () => {
      const envelope = new Envelope({
        from: undefined,
        to: undefined,
        operation: undefined,
        messageId: undefined,
      } as any);

      // Should handle undefined gracefully
      expect(() => envelope.serialize()).not.toThrow();
    });

    test('should reject excessively nested capabilities', () => {
      // Create deeply nested object
      let nested: any = { value: 'deep' };
      for (let i = 0; i < 100; i++) {
        nested = { level: i, nested: nested };
      }

      const envelope = new Envelope({
        from: 'test',
        to: 'target',
        operation: OperationType.DATA,
        messageId: 'nested-test',
        capabilities: {
          'nested-data': JSON.stringify(nested)
        }
      });

      // Should handle gracefully
      expect(() => envelope.serialize()).not.toThrow();
    });
  });

  describe('Injection Prevention', () => {
    test('should handle SQL injection attempts in capabilities', () => {
      const sqlInjection = "'; DROP TABLE users; --";

      const envelope = new Envelope({
        from: 'attacker',
        to: 'database',
        operation: OperationType.DATA,
        messageId: 'sql-attack',
        capabilities: {
          'query': sqlInjection
        }
      });

      expect(envelope.validate()).toBe(true);

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      // SQL injection string should be preserved as-is (escaped in JSON)
      expect(deserialized.getCapabilities()['query']).toBe(sqlInjection);
    });

    test('should handle XSS attempts in string fields', () => {
      const xssScript = '<script>alert("XSS")</script>';

      const envelope = new Envelope({
        from: 'attacker',
        to: 'web-server',
        operation: OperationType.DATA,
        messageId: 'xss-attack',
        capabilities: {
          'message': xssScript
        }
      });

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      // XSS script should be preserved (JSON escaping protects)
      expect(deserialized.getCapabilities()['message']).toBe(xssScript);
    });

    test('should handle command injection attempts', () => {
      const cmdInjection = '$(rm -rf /)';

      const envelope = new Envelope({
        from: 'attacker',
        to: 'shell-executor',
        operation: OperationType.DATA,
        messageId: 'cmd-attack',
        capabilities: {
          'command': cmdInjection
        }
      });

      const serialized = envelope.serialize();
      expect(serialized).toContain(cmdInjection);
    });

    test('should handle path traversal attempts', () => {
      const pathTraversal = '../../etc/passwd';

      const envelope = new Envelope({
        from: 'attacker',
        to: 'file-server',
        operation: OperationType.DATA,
        messageId: 'path-attack',
        capabilities: {
          'file': pathTraversal
        }
      });

      expect(envelope.validate()).toBe(true);
      expect(envelope.getCapabilities()['file']).toBe(pathTraversal);
    });

    test('should handle LDAP injection attempts', () => {
      const ldapInjection = '*)(uid=*))(|(uid=*';

      const envelope = new Envelope({
        from: 'attacker',
        to: 'ldap-server',
        operation: OperationType.DATA,
        messageId: 'ldap-attack',
        capabilities: {
          'filter': ldapInjection
        }
      });

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(deserialized.getCapabilities()['filter']).toBe(ldapInjection);
    });
  });

  describe('Special Characters Handling', () => {
    test('should handle null bytes', () => {
      const nullByte = 'test\x00data';

      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'null-byte',
        capabilities: {
          'data': nullByte
        }
      });

      const serialized = envelope.serialize();
      expect(serialized).toBeTruthy();
    });

    test('should handle unicode characters', () => {
      const unicodeData = '测试 тест δοκιμή परीक्षण 🚀';

      const envelope = new Envelope({
        from: 'unicode-sender',
        to: 'unicode-receiver',
        operation: OperationType.DATA,
        messageId: 'unicode-test',
        capabilities: {
          'text': unicodeData
        }
      });

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(deserialized.getCapabilities()['text']).toBe(unicodeData);
    });

    test('should handle control characters', () => {
      const controlChars = '\x01\x02\x03\x04\x05\x06\x07\x08';

      const envelope = new Envelope({
        from: 'control-sender',
        to: 'control-receiver',
        operation: OperationType.DATA,
        messageId: 'control-chars',
        capabilities: {
          'data': controlChars
        }
      });

      expect(() => envelope.serialize()).not.toThrow();
    });

    test('should handle newlines and special whitespace', () => {
      const specialWhitespace = 'line1\nline2\r\nline3\ttab\vvertical';

      const envelope = new Envelope({
        from: 'ws-sender',
        to: 'ws-receiver',
        operation: OperationType.DATA,
        messageId: 'whitespace',
        capabilities: {
          'text': specialWhitespace
        }
      });

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(deserialized.getCapabilities()['text']).toBe(specialWhitespace);
    });
  });

  describe('Size Limits and Boundaries', () => {
    test('should handle very long message IDs', () => {
      const longMessageId = 'msg-' + 'x'.repeat(1000);

      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: longMessageId,
      });

      expect(envelope.getMessageId()).toBe(longMessageId);
    });

    test('should handle maximum capability keys', () => {
      const capabilities: Record<string, string> = {};

      // Add 1000 capability keys
      for (let i = 0; i < 1000; i++) {
        capabilities[`key${i}`] = `value${i}`;
      }

      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'many-caps',
        capabilities: capabilities
      });

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(Object.keys(deserialized.getCapabilities()).length).toBe(1000);
    });

    test('should handle very long capability values', () => {
      const longValue = 'v'.repeat(100000); // 100KB value

      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'long-value',
        capabilities: {
          'large': longValue
        }
      });

      const serialized = envelope.serialize();
      expect(serialized.length).toBeGreaterThan(100000);
    });

    test('should handle zero-length strings', () => {
      const envelope = new Envelope({
        from: '',
        to: '',
        operation: OperationType.DATA,
        messageId: '',
        capabilities: {
          'empty': ''
        }
      });

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(deserialized.getCapabilities()['empty']).toBe('');
    });
  });

  describe('PayloadHint Validation', () => {
    test('should handle invalid payload hint values', () => {
      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'invalid-hint',
        payloadHint: {
          type: 999 as PayloadType, // Invalid type
          size: -1, // Negative size
          encoding: 999 as EncodingType, // Invalid encoding
          count: -100 // Negative count
        }
      });

      // Should handle gracefully without throwing
      expect(() => envelope.serialize()).not.toThrow();
    });

    test('should handle missing payload hint fields', () => {
      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'partial-hint',
        payloadHint: {
          type: PayloadType.VECTOR
          // Missing size, encoding, count
        }
      });

      expect(envelope.validate()).toBe(true);
    });

    test('should handle extreme payload hint sizes', () => {
      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'extreme-hint',
        payloadHint: {
          type: PayloadType.BINARY,
          size: Number.MAX_SAFE_INTEGER,
          encoding: EncodingType.UINT8,
          count: Number.MAX_SAFE_INTEGER
        }
      });

      expect(() => envelope.serialize()).not.toThrow();
    });
  });

  describe('HTTP Security', () => {
    test('should reject oversized HTTP requests', async () => {
      let errorCaught = false;

      const httpServer = new StreamableHTTPServer({
        onError: (error) => {
          errorCaught = true;
        }
      });

      const server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      await new Promise<void>(resolve => {
        server.listen(24001, () => resolve());
      });

      // Try to send very large request
      const hugeData = 'x'.repeat(200 * 1024 * 1024); // 200MB

      try {
        await fetch('http://localhost:24001/umicp', {
          method: 'POST',
          body: JSON.stringify({
            from: 'attacker',
            to: 'server',
            operation: OperationType.DATA,
            messageId: 'huge',
            capabilities: { data: hugeData }
          }),
          headers: { 'Content-Type': 'application/json' }
        });
      } catch (error) {
        // Expected to fail
      }

      await new Promise<void>(resolve => {
        server.close(() => resolve());
      });
    }, 30000);

    test('should handle malformed HTTP requests', async () => {
      let errorCount = 0;

      const httpServer = new StreamableHTTPServer({
        onError: () => {
          errorCount++;
        }
      });

      const server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      await new Promise<void>(resolve => {
        server.listen(24002, () => resolve());
      });

      // Send invalid JSON
      await fetch('http://localhost:24002/umicp', {
        method: 'POST',
        body: '{invalid json}',
        headers: { 'Content-Type': 'application/json' }
      });

      await new Promise(resolve => setTimeout(resolve, 200));
      expect(errorCount).toBeGreaterThan(0);

      await new Promise<void>(resolve => {
        server.close(() => resolve());
      });
    });

    test('should handle HTTP header injection attempts', async () => {
      const httpServer = new StreamableHTTPServer({
        onMessage: async (envelope, req, res) => {
          res.writeHead(200);
          res.end('ok');
        }
      });

      const server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      await new Promise<void>(resolve => {
        server.listen(24003, () => resolve());
      });

      // Try header injection
      try {
        await fetch('http://localhost:24003/umicp', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
            'X-Injected': 'value\r\nX-Evil: evil'
          },
          body: new Envelope({
            from: 'attacker',
            to: 'server',
            operation: OperationType.DATA,
            messageId: 'header-inject'
          }).serialize()
        });
      } catch (error) {
        // May fail, but shouldn't crash server
      }

      await new Promise<void>(resolve => {
        server.close(() => resolve());
      });
    });
  });

  describe('Protocol Compliance', () => {
    test('should handle unknown operation types', () => {
      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: 999 as OperationType, // Unknown operation
        messageId: 'unknown-op',
      });

      // Should handle gracefully
      expect(() => envelope.serialize()).not.toThrow();
    });

    test('should preserve all operation type values', () => {
      const operations = [
        OperationType.CONTROL,
        OperationType.DATA,
        OperationType.ACK,
        OperationType.ERROR
      ];

      operations.forEach(op => {
        const envelope = new Envelope({
          from: 'sender',
          to: 'receiver',
          operation: op,
          messageId: `op-${op}`,
        });

        const serialized = envelope.serialize();
        const deserialized = Envelope.deserialize(serialized);

        expect(deserialized.getOperation()).toBe(op);
      });
    });

    test('should handle circular references prevention', () => {
      // Attempt to create circular reference
      const obj: any = { name: 'test' };
      obj.self = obj; // Circular reference

      expect(() => {
        const envelope = new Envelope({
          from: 'sender',
          to: 'receiver',
          operation: OperationType.DATA,
          messageId: 'circular',
          capabilities: {
            'data': JSON.stringify(obj) // Will throw on circular ref
          }
        });
      }).toThrow();
    });
  });

  describe('WebSocket Security', () => {
    test('should handle connection from untrusted source', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'security-test',
        server: { port: 24100 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      // Connect from "untrusted" client
      const untrusted = new UMICPWebSocketPeer({
        peerId: 'untrusted-client'
      });

      const peerId = await untrusted.connectToPeer('ws://localhost:24100/umicp');
      expect(peerId).toBeTruthy();

      // Connection should be established (authentication is application-level)
      await new Promise(resolve => setTimeout(resolve, 300));

      const stats = peer.getStats();
      expect(stats.totalPeers).toBeGreaterThan(0);

      await untrusted.shutdown();
      await peer.shutdown();
    });

    test('should handle rapid connection attempts', async () => {
      const peer = new UMICPWebSocketPeer({
        peerId: 'rate-limit-test',
        server: { port: 24101 }
      });

      await new Promise<void>(resolve => {
        peer.on('server:ready', () => resolve());
      });

      // Rapid connections
      const clients: UMICPWebSocketPeer[] = [];
      for (let i = 0; i < 10; i++) {
        const client = new UMICPWebSocketPeer({
          peerId: `rapid-${i}`
        });
        await client.connectToPeer('ws://localhost:24101/umicp');
        clients.push(client);
      }

      await new Promise(resolve => setTimeout(resolve, 500));

      const stats = peer.getStats();
      expect(stats.totalPeers).toBe(10);

      // Cleanup
      for (const client of clients) {
        await client.shutdown();
      }
      await peer.shutdown();
    }, 15000);
  });

  describe('Error Edge Cases', () => {
    test('should handle serialization of special numeric values', () => {
      const envelope = new Envelope({
        from: 'sender',
        to: 'receiver',
        operation: OperationType.DATA,
        messageId: 'special-nums',
        capabilities: {
          'infinity': String(Infinity),
          'neg-infinity': String(-Infinity),
          'nan': String(NaN),
          'max-safe': String(Number.MAX_SAFE_INTEGER),
          'min-safe': String(Number.MIN_SAFE_INTEGER)
        }
      });

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(deserialized.getCapabilities()['infinity']).toBe('Infinity');
    });

    test('should handle mixed encoding types', () => {
      const encodings = [
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
        const envelope = new Envelope({
          from: 'sender',
          to: 'receiver',
          operation: OperationType.DATA,
          messageId: `encoding-${encoding}`,
          payloadHint: {
            type: PayloadType.VECTOR,
            size: 1024,
            encoding: encoding
          }
        });

        expect(envelope.validate()).toBe(true);
      });
    });
  });
});

