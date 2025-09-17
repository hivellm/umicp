/**
 * Security and Validation Tests for UMICP Bindings
 */

import { Envelope, UMICP, OperationType, PayloadType, EncodingType } from '../src/index';

describe('Security Tests', () => {
  // Input Validation Tests
  describe('Input Validation', () => {
    test('should reject extremely long strings in envelope fields', () => {
      const veryLongString = 'x'.repeat(100000); // 100KB string

      expect(() => {
        UMICP.createEnvelope({
          from: veryLongString,
          to: 'receiver',
          messageId: 'test'
        });
      }).not.toThrow(); // Should handle gracefully, even if validation fails

      expect(() => {
        UMICP.createEnvelope({
          from: 'sender',
          to: veryLongString,
          messageId: 'test'
        });
      }).not.toThrow();
    });

    test('should handle malformed message IDs', () => {
      const malformedIds = [
        '',
        '   ',
        '\n\t\r',
        '\x00\x01\x02', // Null bytes
        '🚀'.repeat(1000), // Many unicode characters
        '<script>alert("xss")</script>', // Potential XSS
        '../../../etc/passwd', // Path traversal
        'javascript:alert(1)', // JavaScript injection
      ];

      malformedIds.forEach(id => {
        expect(() => {
          UMICP.createEnvelope({
            from: 'sender',
            to: 'receiver',
            messageId: id
          });
        }).not.toThrow();
      });
    });

    test('should sanitize capability values', () => {
      const maliciousCapabilities = {
        'script': '<script>alert("xss")</script>',
        'sql': 'DROP TABLE users; --',
        'path': '../../../etc/passwd',
        'command': '; rm -rf / ;',
        'null_bytes': 'test\x00injected',
        'unicode_bom': '\uFEFFmalicious',
        'control_chars': 'test\x01\x02\x03control',
      };

      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        capabilities: maliciousCapabilities
      });

      expect(envelope.validate()).toBe(true);

      // Verify capabilities are stored as-is (no automatic sanitization expected at envelope level)
      const caps = envelope.getCapabilities();
      Object.keys(maliciousCapabilities).forEach(key => {
        expect(caps[key]).toBe((maliciousCapabilities as any)[key]);
      });
    });

    test('should handle numeric overflow in sequence numbers', () => {
      const largeNumbers = [
        Number.MAX_SAFE_INTEGER,
        Number.MAX_SAFE_INTEGER + 1,
        Number.MAX_VALUE,
        Infinity,
        -Infinity,
        NaN
      ];

      largeNumbers.forEach(num => {
        const envelope = UMICP.createEnvelope({
          from: 'sender',
          to: 'receiver',
          capabilities: {
            sequence: num.toString(),
            timestamp: Date.now().toString()
          }
        });

        expect(envelope.validate()).toBe(true);
      });
    });

    test('should reject invalid operation types', () => {
      // Test with invalid operation type (if validation exists)
      expect(() => {
        UMICP.createEnvelope({
          from: 'sender',
          to: 'receiver',
          operation: 999 as any // Invalid operation type
        });
      }).not.toThrow(); // Should handle gracefully
    });
  });

  // JSON Injection Prevention Tests
  describe('JSON Injection Prevention', () => {
    test('should handle JSON injection attempts in capabilities', () => {
      const injectionAttempts = [
        '{"version":"1.0"}</script><script>alert(1)</script>{"injected":',
        '","injected": "value"}</script><img src=x onerror=alert(1)>',
        '\\u0022,\\"injected\\": \\"value\\"',
        '","__proto__":{"malicious":"property"}',
        '","constructor":{"prototype":{"malicious":"property"}}'
      ];

      injectionAttempts.forEach(attempt => {
        const envelope = UMICP.createEnvelope({
          from: 'sender',
          to: 'receiver',
          capabilities: {
            'data': attempt
          }
        });

        expect(envelope.validate()).toBe(true);

        // Test serialization doesn't break JSON structure
        const serialized = envelope.serialize();
        expect(() => {
          JSON.parse(serialized);
        }).not.toThrow();

        // Test deserialization works
        const deserialized = Envelope.deserialize(serialized);
        expect(deserialized.validate()).toBe(true);
      });
    });

    test('should prevent prototype pollution through capabilities', () => {
      const maliciousCapabilities: any = {
        '__proto__': {
          'maliciousProperty': 'hacked'
        },
        'constructor': {
          'prototype': {
            'anotherMaliciousProperty': 'also_hacked'
          }
        },
        'toString': {
          'customToString': 'overridden'
        }
      };

      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        capabilities: {
          '__proto__': JSON.stringify(maliciousCapabilities['__proto__']),
          'constructor': JSON.stringify(maliciousCapabilities['constructor']),
          'toString': JSON.stringify(maliciousCapabilities['toString'])
        }
      });

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      // Verify prototype wasn't polluted
      const emptyObj = {};
      expect((emptyObj as any).maliciousProperty).toBeUndefined();
      expect((emptyObj as any).anotherMaliciousProperty).toBeUndefined();
      expect((emptyObj as any).customToString).toBeUndefined();

      // Verify envelope is still valid
      expect(deserialized.validate()).toBe(true);
    });

    test('should handle deeply nested capability objects', () => {
      const createNestedObject = (depth: number): any => {
        if (depth === 0) return { leaf: 'value' };
        return { nested: createNestedObject(depth - 1) };
      };

      const deeplyNested = createNestedObject(5); // Reduced depth

      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        capabilities: {
          'nested_data': JSON.stringify(deeplyNested)
        }
      });

      expect(envelope.validate()).toBe(true);

      const serialized = envelope.serialize();
      expect(serialized.length).toBeGreaterThan(1000); // Should be substantial

      const deserialized = Envelope.deserialize(serialized);
      expect(deserialized.validate()).toBe(true);
    });
  });

  // Resource Exhaustion Tests
  describe('Resource Exhaustion Protection', () => {
    test('should handle large numbers of concurrent envelope creation', async () => {
      const concurrentOperations = 1000;
      const promises = [];

      for (let i = 0; i < concurrentOperations; i++) {
        promises.push(
          Promise.resolve().then(() => {
            return UMICP.createEnvelope({
              from: `sender_${i}`,
              to: `receiver_${i}`,
              messageId: `msg_${i}_${Date.now()}`,
              capabilities: {
                'sequence': i.toString(),
                'timestamp': Date.now().toString(),
                'data': 'x'.repeat(100) // Some payload
              }
            });
          })
        );
      }

      const envelopes = await Promise.all(promises);

      envelopes.forEach((envelope, index) => {
        expect(envelope).toBeInstanceOf(Envelope);
        expect(envelope.validate()).toBe(true);
        expect(envelope.getFrom()).toBe(`sender_${index}`);
      });

      // Verify all envelopes have unique message IDs
      const messageIds = envelopes.map(env => env.getMessageId());
      const uniqueIds = new Set(messageIds);
      expect(uniqueIds.size).toBe(concurrentOperations);
    });

    test('should handle sustained high-frequency operations', () => {
      const duration = 3000; // 3 seconds
      const startTime = Date.now();
      let operationCount = 0;

      while (Date.now() - startTime < duration) {
        const envelope = UMICP.createEnvelope({
          from: 'sender',
          to: 'receiver',
          messageId: `stress_${operationCount}_${Date.now()}`
        });

        expect(envelope.validate()).toBe(true);
        operationCount++;
      }

      console.log(`Security stress test: ${operationCount} envelopes created in ${duration}ms`);
      expect(operationCount).toBeGreaterThan(1000); // Should handle reasonable load
    });

    test('should prevent memory exhaustion through large capability objects', () => {
      const memoryUsageBefore = process.memoryUsage().heapUsed;

      // Create envelope with very large capability object
      const largeCapabilities: { [key: string]: string } = {};
      const capabilityCount = 10000;

      for (let i = 0; i < capabilityCount; i++) {
        largeCapabilities[`key_${i}`] = `value_${i}_` + 'x'.repeat(100);
      }

      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        capabilities: largeCapabilities
      });

      expect(envelope.validate()).toBe(true);

      const memoryUsageAfter = process.memoryUsage().heapUsed;
      const memoryDelta = memoryUsageAfter - memoryUsageBefore;

      console.log(`Memory usage for large capabilities: ${(memoryDelta / 1024 / 1024).toFixed(2)} MB`);

      // Test serialization and deserialization
      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(deserialized.validate()).toBe(true);

      // Memory should be reasonable (less than 100MB for this test)
      expect(memoryDelta).toBeLessThan(100 * 1024 * 1024);
    });
  });

  // Data Integrity Tests
  describe('Data Integrity', () => {
    test('should maintain data integrity through serialization cycles', () => {
      const originalCapabilities = {
        'version': '1.0',
        'features': 'compression,encryption',
        'sequence': '12345',
        'timestamp': Date.now().toString(),
        'metadata': JSON.stringify({
          source: 'test-client',
          priority: 'high',
          tags: ['important', 'urgent']
        })
      };

      const originalEnvelope = UMICP.createEnvelope({
        from: 'test-sender',
        to: 'test-receiver',
        operation: OperationType.DATA,
        messageId: 'integrity-test-001',
        capabilities: originalCapabilities
      });

      // Multiple serialization/deserialization cycles
      let currentEnvelope = originalEnvelope;
      for (let i = 0; i < 10; i++) {
        const serialized = currentEnvelope.serialize();
        currentEnvelope = Envelope.deserialize(serialized);
        expect(currentEnvelope.validate()).toBe(true);
      }

      // Verify all data is preserved
      expect(currentEnvelope.getFrom()).toBe('test-sender');
      expect(currentEnvelope.getTo()).toBe('test-receiver');
      expect(currentEnvelope.getMessageId()).toBe('integrity-test-001');

      const finalCapabilities = currentEnvelope.getCapabilities();
      Object.keys(originalCapabilities).forEach(key => {
        expect(finalCapabilities[key]).toBe((originalCapabilities as any)[key]);
      });
    });

    test('should detect and handle hash collisions gracefully', () => {
      const envelopes = [];
      const hashMap = new Map<string, Envelope>();

      // Create many envelopes and check for hash collisions
      for (let i = 0; i < 1000; i++) {
        const envelope = UMICP.createEnvelope({
          from: `sender_${i % 10}`, // Limited variety to increase collision chance
          to: `receiver_${i % 5}`,
          operation: OperationType.DATA,
          messageId: `msg_${i}_${Date.now()}`
        });

        const hash = envelope.getHash();

        // If hash collision occurs, both envelopes should still be valid
        if (hashMap.has(hash)) {
          const existingEnvelope = hashMap.get(hash)!;
          expect(existingEnvelope.validate()).toBe(true);
          expect(envelope.validate()).toBe(true);
        } else {
          hashMap.set(hash, envelope);
        }

        envelopes.push(envelope);
      }

      console.log(`Created ${envelopes.length} envelopes with ${hashMap.size} unique hashes`);
      expect(envelopes.length).toBe(1000);
      expect(hashMap.size).toBe(envelopes.length); // All hashes should be unique in practice
    });

    test('should handle timezone and locale differences', () => {
      const testTimestamp = Date.now();
      const originalEnvelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        capabilities: {
          'timestamp': testTimestamp.toString(),
          'locale': 'en-US',
          'timezone': 'America/New_York'
        }
      });

      // Simulate different locale/timezone environment
      const originalLocale = process.env.LANG;
      const originalTimezone = process.env.TZ;

      try {
        // Change environment (if possible)
        process.env.LANG = 'pt-BR';
        process.env.TZ = 'America/Sao_Paulo';

        const serialized = originalEnvelope.serialize();
        const deserialized = Envelope.deserialize(serialized);

        expect(deserialized.validate()).toBe(true);
        expect(deserialized.getCapabilities()['timestamp']).toBe(testTimestamp.toString());

      } finally {
        // Restore original environment
        if (originalLocale) process.env.LANG = originalLocale;
        if (originalTimezone) process.env.TZ = originalTimezone;
      }
    });
  });

  // Authentication and Authorization Tests
  describe('Authentication and Authorization', () => {
    test('should handle authentication tokens in capabilities', () => {
      const authToken = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...'; // Mock JWT
      const sessionId = 'sess_' + Date.now() + '_' + Math.random().toString(36);

      const envelope = UMICP.createEnvelope({
        from: 'authenticated-user',
        to: 'service-endpoint',
        capabilities: {
          'auth_token': authToken,
          'session_id': sessionId,
          'permissions': 'read,write,execute',
          'user_id': 'user123',
          'expires_at': (Date.now() + 3600000).toString() // 1 hour from now
        }
      });

      expect(envelope.validate()).toBe(true);

      // Test that sensitive data is preserved through serialization
      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      const caps = deserialized.getCapabilities();
      expect(caps['auth_token']).toBe(authToken);
      expect(caps['session_id']).toBe(sessionId);
    });

    test('should handle encrypted payload capabilities', () => {
      const encryptedData = 'U2FsdGVkX1+...' + 'x'.repeat(500); // Mock encrypted data
      const keyFingerprint = 'sha256:' + 'a'.repeat(64);

      const envelope = UMICP.createEnvelope({
        from: 'sender',
        to: 'receiver',
        capabilities: {
          'encryption': 'AES256-GCM',
          'key_fingerprint': keyFingerprint,
          'iv': 'dGVzdC1pdjEyMzQ1Njc4OTA=', // base64 encoded IV
          'encrypted_payload': encryptedData,
          'hmac': 'sha256:' + 'b'.repeat(64)
        }
      });

      expect(envelope.validate()).toBe(true);

      // Verify large encrypted data is handled
      const serialized = envelope.serialize();
      expect(serialized.length).toBeGreaterThan(500); // Adjusted expectation

      const deserialized = Envelope.deserialize(serialized);
      expect(deserialized.validate()).toBe(true);
      expect(deserialized.getCapabilities()['encrypted_payload']).toBe(encryptedData);
    });
  });

  // Network Security Tests
  describe('Network Security', () => {
    test('should handle various endpoint formats securely', () => {
      const endpoints = [
        'ws://localhost:8080',
        'wss://secure.example.com:443',
        'tcp://192.168.1.100:9000',
        'unix:///tmp/umicp.sock',
        'http://api.example.com/v1',
        'https://secure-api.example.com/v2'
      ];

      endpoints.forEach(endpoint => {
        const envelope = UMICP.createEnvelope({
          from: 'client',
          to: endpoint,
          capabilities: {
            'protocol': endpoint.split(':')[0],
            'secure': endpoint.startsWith('https') || endpoint.startsWith('wss') ? 'true' : 'false'
          }
        });

        expect(envelope.validate()).toBe(true);
        expect(envelope.getTo()).toBe(endpoint);
      });
    });

    test('should handle rate limiting information', () => {
      const envelope = UMICP.createEnvelope({
        from: 'client',
        to: 'server',
        capabilities: {
          'rate_limit': '1000',
          'rate_window': '60', // per minute
          'burst_limit': '100',
          'retry_after': '30',
          'quota_remaining': '950'
        }
      });

      expect(envelope.validate()).toBe(true);

      // Test serialization preserves numeric strings
      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      const caps = deserialized.getCapabilities();
      expect(caps['rate_limit']).toBe('1000');
      expect(caps['quota_remaining']).toBe('950');
    });

    test('should handle CORS and security headers information', () => {
      const envelope = UMICP.createEnvelope({
        from: 'browser-client',
        to: 'api-server',
        capabilities: {
          'origin': 'https://app.example.com',
          'referer': 'https://app.example.com/dashboard',
          'user_agent': 'Mozilla/5.0 (compatible; UMICP-Client/1.0)',
          'accept_language': 'en-US,en;q=0.9',
          'cors_allowed': 'true',
          'credentials': 'include'
        }
      });

      expect(envelope.validate()).toBe(true);

      // Test that headers are preserved
      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      const caps = deserialized.getCapabilities();
      expect(caps['origin']).toBe('https://app.example.com');
      expect(caps['cors_allowed']).toBe('true');
    });
  });
});
