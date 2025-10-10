/**
 * Vitest setup file for UMICP TypeScript bindings tests
 */

import { beforeAll, afterAll, beforeEach, afterEach, expect, vi } from 'vitest';
import { Envelope, UMICP, OperationType } from '../src/index';

// Extend global interface
declare global {
  var testUtils: {
    wait: (ms: number) => Promise<void>;
    generateRandomId: () => string;
    createTestData: (size: number) => Float32Array;
    createTestEnvelope: (overrides?: Partial<Parameters<typeof UMICP.createEnvelope>[0]>) => ReturnType<typeof UMICP.createEnvelope>;
    createLargeCapabilityObject: (count: number, valueSize?: number) => { [key: string]: string };
    generateMaliciousStrings: () => string[];
    measurePerformance: <T>(fn: () => T) => { result: T; duration: number };
    createConcurrentLoad: (count: number, operation: () => Promise<any>) => Promise<any[]>;
    simulateMemoryPressure: (targetMB: number) => void;
    cleanupMemoryPressure: () => void;
  };
}

// Store original console methods
const originalConsole = { ...console };

// Performance monitoring
let performanceMarks: { [key: string]: number } = {};

// Mock console methods for cleaner test output (but allow errors through)
beforeAll(() => {
  console.log = vi.fn();
  console.warn = vi.fn();
  // Keep console.error for actual errors
});

afterAll(() => {
  // Restore console methods
  Object.assign(console, originalConsole);
});

// Global test utilities
global.testUtils = {
  wait: (ms: number) => new Promise(resolve => setTimeout(resolve, ms)),

  generateRandomId: () => Math.random().toString(36).substr(2, 9),

  createTestData: (size: number) => {
    const data = new Float32Array(size);
    for (let i = 0; i < size; i++) {
      data[i] = Math.sin(i * 0.1) * Math.cos(i * 0.05);
    }
    return data;
  },

  createTestEnvelope: (overrides = {}) => {
    return UMICP.createEnvelope({
      from: 'test-sender',
      to: 'test-receiver',
      operation: OperationType.DATA,
      messageId: `test-msg-${Date.now()}-${Math.random()}`,
      capabilities: {
        'test': 'true',
        'timestamp': Date.now().toString()
      },
      ...overrides
    });
  },

  createLargeCapabilityObject: (count: number, valueSize = 1000) => {
    const capabilities: { [key: string]: string } = {};
    const baseValue = 'x'.repeat(valueSize);

    for (let i = 0; i < count; i++) {
      capabilities[`capability_${i}`] = `${baseValue}_${i}`;
    }

    return capabilities;
  },

  generateMaliciousStrings: () => {
    return [
      '<script>alert("xss")</script>',
      '{"injected": "value"}</script><script>alert(1)</script>{"safe":',
      '../../../etc/passwd',
      'javascript:alert(1)',
      '<img src=x onerror=alert(1)>',
      '${process.env}',
      'eval("malicious code")',
      'DROP TABLE users; --',
      '<iframe src="javascript:alert(1)"></iframe>',
      'data:text/html,<script>alert(1)</script>',
      '\x00\x01\x02', // Null bytes
      '\u0000\u0001\u0002', // Unicode null bytes
      '🚀'.repeat(1000), // Many unicode characters
      'a'.repeat(100000), // Very long string
    ];
  },

  measurePerformance: <T>(fn: () => T): { result: T; duration: number } => {
    const startTime = performance.now();
    const result = fn();
    const endTime = performance.now();
    return {
      result,
      duration: endTime - startTime
    };
  },

  createConcurrentLoad: async (count: number, operation: () => Promise<any>): Promise<any[]> => {
    const promises = [];
    for (let i = 0; i < count; i++) {
      promises.push(operation());
    }
    return Promise.all(promises);
  },

  simulateMemoryPressure: (targetMB: number) => {
    // Create large objects to simulate memory pressure
    const largeObjects: any[] = [];
    const chunkSize = 1024 * 1024; // 1MB chunks

    for (let i = 0; i < targetMB; i++) {
      largeObjects.push(new Array(chunkSize).fill('x'));
    }

    // Store reference to prevent garbage collection
    (global as any).__memoryPressureObjects = largeObjects;
  },

  cleanupMemoryPressure: () => {
    // Clean up memory pressure objects
    if ((global as any).__memoryPressureObjects) {
      (global as any).__memoryPressureObjects = null;

      // Force garbage collection if available
      if (global.gc) {
        global.gc();
      }
    }
  }
};

// Performance test helpers
beforeEach(() => {
  performanceMarks = {};
});

afterEach(() => {
  // Clean up any memory pressure from tests
  global.testUtils.cleanupMemoryPressure();
});

// Custom matchers for better test assertions
expect.extend({
  toBeValidEnvelope(received: any) {
    const { isNot } = this;

    if (!received) {
      return {
        message: () => 'Expected envelope to be defined',
        pass: false
      };
    }

    if (typeof received.validate !== 'function') {
      return {
        message: () => 'Expected envelope to have validate method',
        pass: false
      };
    }

    const isValid = received.validate();
    return {
      message: () => `Expected envelope to ${isNot ? 'not ' : ''}be valid, but validation returned ${isValid}`,
      pass: isValid
    };
  },

  toBeWithinPerformanceRange(received: number, expected: number, tolerancePercent: number = 10) {
    const { isNot } = this;
    const tolerance = expected * (tolerancePercent / 100);
    const min = expected - tolerance;
    const max = expected + tolerance;
    const pass = received >= min && received <= max;

    return {
      message: () =>
        `Expected ${received}ms to ${isNot ? 'not ' : ''}be within ${tolerancePercent}% of ${expected}ms (range: ${min}ms - ${max}ms)`,
      pass
    };
  },

  toBeValidCapabilityValue(received: any) {
    const { isNot } = this;
    const isString = typeof received === 'string';
    const isNotEmpty = received !== '';
    const hasNoNullBytes = !received?.includes('\x00');

    const pass = isString && isNotEmpty && hasNoNullBytes;

    return {
      message: () =>
        `Expected capability value to ${isNot ? 'not ' : ''}be a non-empty string without null bytes, got: ${typeof received}`,
      pass
    };
  }
});

// Declare custom matchers for TypeScript
interface CustomMatchers<R = unknown> {
  toBeValidEnvelope(): R;
  toBeWithinPerformanceRange(expected: number, tolerancePercent?: number): R;
  toBeValidCapabilityValue(): R;
}

declare module 'vitest' {
  interface Assertion<T = any> extends CustomMatchers<T> {}
  interface AsymmetricMatchersContaining extends CustomMatchers {}
}
