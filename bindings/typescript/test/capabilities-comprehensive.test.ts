/**
 * Comprehensive Capabilities Tests (v0.2.0)
 * Testing native JSON types support
 */

import { describe, it, expect } from 'vitest';
import { Envelope, OperationType } from '../src/index';

describe('Capabilities - Native Types Comprehensive', () => {

  describe('Integer Types', () => {
    it('should handle positive integers', () => {
      const capabilities: Record<string, any> = { max_tokens: 100 };
      expect(capabilities.max_tokens).toBe(100);
      expect(typeof capabilities.max_tokens).toBe('number');
    });

    it('should handle negative integers', () => {
      const capabilities: Record<string, any> = { offset: -50 };
      expect(capabilities.offset).toBe(-50);
    });

    it('should handle zero', () => {
      const capabilities: Record<string, any> = { count: 0 };
      expect(capabilities.count).toBe(0);
    });

    it('should handle large integers', () => {
      const capabilities: Record<string, any> = { max_value: 2147483647 };
      expect(capabilities.max_value).toBe(2147483647);
    });
  });

  describe('Float Types', () => {
    it('should handle decimal floats', () => {
      const capabilities: Record<string, any> = { temperature: 0.7 };
      expect(capabilities.temperature).toBeCloseTo(0.7);
    });

    it('should handle scientific notation', () => {
      const capabilities: Record<string, any> = { learning_rate: 1.5e-4 };
      expect(capabilities.learning_rate).toBeCloseTo(1.5e-4);
    });

    it('should handle float zero', () => {
      const capabilities: Record<string, any> = { threshold: 0.0 };
      expect(capabilities.threshold).toBe(0.0);
    });
  });

  describe('Boolean Types', () => {
    it('should handle true', () => {
      const capabilities: Record<string, any> = { enabled: true };
      expect(capabilities.enabled).toBe(true);
    });

    it('should handle false', () => {
      const capabilities: Record<string, any> = { disabled: false };
      expect(capabilities.disabled).toBe(false);
    });
  });

  describe('String Types', () => {
    it('should handle simple strings', () => {
      const capabilities: Record<string, any> = { model: 'gpt-4' };
      expect(capabilities.model).toBe('gpt-4');
    });

    it('should handle empty strings', () => {
      const capabilities: Record<string, any> = { empty: '' };
      expect(capabilities.empty).toBe('');
    });

    it('should handle unicode strings', () => {
      const capabilities: Record<string, any> = { message: 'Hello 世界 🌍' };
      expect(capabilities.message).toBe('Hello 世界 🌍');
    });
  });

  describe('Array Types', () => {
    it('should handle string arrays', () => {
      const capabilities: Record<string, any> = { models: ['gpt-4', 'claude-3', 'llama-3'] };
      expect(capabilities.models).toHaveLength(3);
      expect(capabilities.models[0]).toBe('gpt-4');
    });

    it('should handle integer arrays', () => {
      const capabilities: Record<string, any> = { limits: [10, 20, 30, 40] };
      expect(capabilities.limits).toHaveLength(4);
      expect(capabilities.limits[2]).toBe(30);
    });

    it('should handle mixed arrays', () => {
      const capabilities: Record<string, any> = { mixed: [1, 'two', true, 4.5] };
      expect(capabilities.mixed[0]).toBe(1);
      expect(capabilities.mixed[1]).toBe('two');
      expect(capabilities.mixed[2]).toBe(true);
      expect(capabilities.mixed[3]).toBeCloseTo(4.5);
    });

    it('should handle empty arrays', () => {
      const capabilities: Record<string, any> = { empty_array: [] };
      expect(capabilities.empty_array).toHaveLength(0);
    });

    it('should handle nested arrays', () => {
      const capabilities: Record<string, any> = { nested: [[1, 2], [3, 4]] };
      expect(capabilities.nested[0][1]).toBe(2);
      expect(capabilities.nested[1][0]).toBe(3);
    });
  });

  describe('Object Types', () => {
    it('should handle simple objects', () => {
      const capabilities: Record<string, any> = {
        config: { timeout: 30, retries: 3 }
      };
      expect(capabilities.config.timeout).toBe(30);
      expect(capabilities.config.retries).toBe(3);
    });

    it('should handle nested objects', () => {
      const capabilities: Record<string, any> = {
        auth: {
          type: 'oauth',
          credentials: {
            client_id: 'abc123',
            scope: ['read', 'write']
          }
        }
      };
      expect(capabilities.auth.type).toBe('oauth');
      expect(capabilities.auth.credentials.client_id).toBe('abc123');
    });

    it('should handle empty objects', () => {
      const capabilities: Record<string, any> = { empty_obj: {} };
      expect(Object.keys(capabilities.empty_obj)).toHaveLength(0);
    });
  });

  describe('Null Types', () => {
    it('should handle null values', () => {
      const capabilities: Record<string, any> = { optional: null };
      expect(capabilities.optional).toBeNull();
    });
  });

  describe('Complex Mixed Types', () => {
    it('should handle all types together', () => {
      const capabilities: Record<string, any> = {
        string_val: 'test',
        int_val: 42,
        float_val: 3.14,
        bool_val: true,
        null_val: null,
        array_val: [1, 2, 3],
        object_val: { key: 'value' }
      };

      expect(capabilities.string_val).toBe('test');
      expect(capabilities.int_val).toBe(42);
      expect(capabilities.float_val).toBeCloseTo(3.14);
      expect(capabilities.bool_val).toBe(true);
      expect(capabilities.null_val).toBeNull();
      expect(capabilities.array_val).toHaveLength(3);
      expect(capabilities.object_val.key).toBe('value');
    });
  });

  describe('Edge Cases', () => {
    it('should handle special characters in keys', () => {
      const capabilities: Record<string, any> = {
        'key-with-dash': 1,
        'key_with_underscore': 2,
        'key.with.dot': 3,
        'key:with:colon': 4
      };

      expect(capabilities['key-with-dash']).toBe(1);
      expect(capabilities['key_with_underscore']).toBe(2);
      expect(capabilities['key.with.dot']).toBe(3);
      expect(capabilities['key:with:colon']).toBe(4);
    });

    it('should handle large objects', () => {
      const capabilities: Record<string, any> = {};
      for (let i = 0; i < 100; i++) {
        capabilities[`key_${i}`] = i;
      }

      expect(Object.keys(capabilities)).toHaveLength(100);
      expect(capabilities.key_50).toBe(50);
      expect(capabilities.key_99).toBe(99);
    });

    it('should handle deeply nested objects', () => {
      const capabilities: Record<string, any> = {
        deep: {
          level1: {
            level2: {
              level3: {
                level4: {
                  level5: 'deep_value'
                }
              }
            }
          }
        }
      };

      expect(capabilities.deep.level1.level2.level3.level4.level5).toBe('deep_value');
    });
  });

  describe('Type Preservation', () => {
    it('should preserve types correctly', () => {
      const capabilities: Record<string, any> = {
        int: 42,
        float: 3.14,
        bool: true,
        string: 'text',
        array: [1, 2],
        object: { a: 1 }
      };

      expect(typeof capabilities.int).toBe('number');
      expect(typeof capabilities.float).toBe('number');
      expect(typeof capabilities.bool).toBe('boolean');
      expect(typeof capabilities.string).toBe('string');
      expect(Array.isArray(capabilities.array)).toBe(true);
      expect(typeof capabilities.object).toBe('object');
    });
  });

  describe('Backward Compatibility', () => {
    it('should support old string-only format', () => {
      const capabilities: Record<string, any> = {
        model: 'gpt-4',
        count: '100'  // Old format as string
      };

      expect(capabilities.model).toBe('gpt-4');
      expect(capabilities.count).toBe('100');
    });
  });

  describe('Serialization Tests', () => {
    it('should serialize and deserialize integers', () => {
      const caps = { max_tokens: 100 };
      const json = JSON.stringify(caps);
      const parsed = JSON.parse(json);

      expect(parsed.max_tokens).toBe(100);
    });

    it('should serialize and deserialize floats', () => {
      const caps = { temperature: 0.7 };
      const json = JSON.stringify(caps);
      const parsed = JSON.parse(json);

      expect(parsed.temperature).toBeCloseTo(0.7);
    });

    it('should serialize and deserialize booleans', () => {
      const caps = { enabled: true };
      const json = JSON.stringify(caps);
      const parsed = JSON.parse(json);

      expect(parsed.enabled).toBe(true);
    });

    it('should serialize and deserialize arrays', () => {
      const caps = { models: ['gpt-4', 'claude-3'] };
      const json = JSON.stringify(caps);
      const parsed = JSON.parse(json);

      expect(parsed.models).toHaveLength(2);
      expect(parsed.models[0]).toBe('gpt-4');
    });

    it('should serialize and deserialize objects', () => {
      const caps = { config: { timeout: 30 } };
      const json = JSON.stringify(caps);
      const parsed = JSON.parse(json);

      expect(parsed.config.timeout).toBe(30);
    });
  });
});

