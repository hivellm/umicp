/**
 * Load Testing for UMICP Bindings
 * Tests system performance under high load and stress conditions
 */

import { Envelope, UMICP, OperationType, PayloadType, EncodingType } from '../src/index';

describe('Load Testing', () => {
  let matrix: any;

  beforeEach(() => {
    matrix = UMICP.createMatrix();
  });

  describe('Envelope Creation Load Tests', () => {
    test('should handle rapid envelope creation (1,000 envelopes)', () => {
      const startTime = Date.now();
      const envelopeCount = 1000;
      const envelopes = [];

      for (let i = 0; i < envelopeCount; i++) {
        const envelope = UMICP.createEnvelope({
          from: `load_sender_${i}`,
          to: `load_receiver_${i}`,
          messageId: `load_msg_${i}_${Date.now()}`,
          operation: OperationType.DATA,
          capabilities: {
            'sequence': i.toString(),
            'batch_id': 'load_test_001',
            'priority': 'normal'
          }
        });
        envelopes.push(envelope);
      }

      const endTime = Date.now();
      const duration = endTime - startTime;
      const envelopesPerSecond = envelopeCount / (duration / 1000);

      console.log(`Load Test - Envelope Creation:`);
      console.log(`  Created: ${envelopeCount} envelopes`);
      console.log(`  Duration: ${duration}ms`);
      console.log(`  Rate: ${envelopesPerSecond.toFixed(0)} envelopes/sec`);
      console.log(`  Avg time per envelope: ${(duration / envelopeCount).toFixed(4)}ms`);

      // Validate all envelopes
      envelopes.forEach((envelope, index) => {
        expect(envelope.validate()).toBe(true);
        expect(envelope.getFrom()).toBe(`load_sender_${index}`);
      });

      expect(duration).toBeLessThan(10000); // Should complete within 10 seconds
      expect(envelopesPerSecond).toBeGreaterThan(500); // At least 500 envelopes per second
    });

    test('should handle concurrent envelope creation (100 concurrent operations)', async () => {
      const concurrentOperations = 100;
      const envelopesPerOperation = 100;
      const promises = [];

      const startTime = Date.now();

      for (let op = 0; op < concurrentOperations; op++) {
        promises.push(
          Promise.resolve().then(() => {
            const envelopes = [];
            for (let i = 0; i < envelopesPerOperation; i++) {
              const envelope = UMICP.createEnvelope({
                from: `concurrent_sender_${op}_${i}`,
                to: `concurrent_receiver_${op}_${i}`,
                messageId: `concurrent_msg_${op}_${i}_${Date.now()}`
              });
              envelopes.push(envelope);
            }
            return envelopes;
          })
        );
      }

      const results = await Promise.all(promises);
      const endTime = Date.now();
      const duration = endTime - startTime;

      const totalEnvelopes = results.flat().length;
      const envelopesPerSecond = totalEnvelopes / (duration / 1000);

      console.log(`Load Test - Concurrent Creation:`);
      console.log(`  Total envelopes: ${totalEnvelopes}`);
      console.log(`  Concurrent operations: ${concurrentOperations}`);
      console.log(`  Duration: ${duration}ms`);
      console.log(`  Rate: ${envelopesPerSecond.toFixed(0)} envelopes/sec`);

      // Validate sample of results
      results.forEach((envelopes, opIndex) => {
        expect(envelopes.length).toBe(envelopesPerOperation);
        envelopes.forEach((envelope, envIndex) => {
          expect(envelope.validate()).toBe(true);
          expect(envelope.getFrom()).toBe(`concurrent_sender_${opIndex}_${envIndex}`);
        });
      });

      expect(duration).toBeLessThan(5000); // Should complete within 5 seconds
    });
  });

  describe('Matrix Operations Load Tests', () => {
    test('should handle large matrix operations (10,000 elements)', () => {
      const size = 10000;
      const a = new Float32Array(size);
      const b = new Float32Array(size);
      const result = new Float32Array(size);

      // Initialize with test data
      for (let i = 0; i < size; i++) {
        a[i] = Math.sin(i * 0.001);
        b[i] = Math.cos(i * 0.001);
      }

      const iterations = 10;
      const startTime = performance.now();

      for (let iter = 0; iter < iterations; iter++) {
        const operationResult = matrix.add(a, b, result, size, 1);
        expect(operationResult.success).toBe(true);
      }

      const endTime = performance.now();
      const duration = endTime - startTime;
      const operationsPerSecond = iterations / (duration / 1000);
      const throughput = (size * iterations) / (duration / 1000) / 1000000; // MOPS

      console.log(`Load Test - Large Matrix Operations:`);
      console.log(`  Array size: ${size}`);
      console.log(`  Iterations: ${iterations}`);
      console.log(`  Duration: ${duration.toFixed(2)}ms`);
      console.log(`  Operations/sec: ${operationsPerSecond.toFixed(2)}`);
      console.log(`  Throughput: ${throughput.toFixed(2)} MOPS`);

      expect(duration).toBeLessThan(2000); // Should complete within 2 seconds
    });

    test('should handle sustained matrix operations (100,000 operations)', () => {
      const duration = 10000; // 10 seconds
      const startTime = Date.now();
      let operationCount = 0;

      const size = 1000;
      const a = new Float32Array(size);
      const b = new Float32Array(size);
      const result = new Float32Array(size);

      // Initialize with test data
      for (let i = 0; i < size; i++) {
        a[i] = Math.sin(i * 0.001);
        b[i] = Math.cos(i * 0.001);
      }

      while (Date.now() - startTime < duration) {
        const operationResult = matrix.add(a, b, result, size, 1);
        expect(operationResult.success).toBe(true);
        operationCount++;

        // Check result occasionally
        if (operationCount % 1000 === 0) {
          expect(result[0]).toBeCloseTo(a[0] + b[0], 5);
        }
      }

      const actualDuration = Date.now() - startTime;
      const operationsPerSecond = operationCount / (actualDuration / 1000);

      console.log(`Load Test - Sustained Matrix Operations:`);
      console.log(`  Operations: ${operationCount}`);
      console.log(`  Duration: ${actualDuration}ms`);
      console.log(`  Rate: ${operationsPerSecond.toFixed(0)} operations/sec`);

      expect(operationCount).toBeGreaterThan(10000); // At least 10K operations
      expect(operationsPerSecond).toBeGreaterThan(500); // At least 500 ops/sec
    });

    test('should handle mixed matrix operations under load', () => {
      const operationCount = 1000;
      const size = 500;

      const startTime = performance.now();

      for (let i = 0; i < operationCount; i++) {
        const a = new Float32Array(size);
        const b = new Float32Array(size);
        const result = new Float32Array(size);

        // Fill with different patterns for each operation
        for (let j = 0; j < size; j++) {
          a[j] = Math.sin(j * 0.01 + i * 0.1);
          b[j] = Math.cos(j * 0.01 + i * 0.1);
        }

        // Mix different operations
        const operationType = i % 4;
        let operationResult;

        switch (operationType) {
          case 0:
            operationResult = matrix.add(a, b, result, size, 1);
            break;
          case 1:
            operationResult = matrix.dotProduct(a, b);
            break;
          case 2:
            operationResult = matrix.normalize(a, 1, size);
            break;
          case 3:
            operationResult = matrix.cosineSimilarity(a, b);
            break;
        }

        expect(operationResult.success).toBe(true);
      }

      const endTime = performance.now();
      const duration = endTime - startTime;
      const operationsPerSecond = operationCount / (duration / 1000);

      console.log(`Load Test - Mixed Matrix Operations:`);
      console.log(`  Operations: ${operationCount}`);
      console.log(`  Duration: ${duration.toFixed(2)}ms`);
      console.log(`  Rate: ${operationsPerSecond.toFixed(0)} operations/sec`);

      expect(duration).toBeLessThan(5000); // Should complete within 5 seconds
    });
  });

  describe('Memory Load Tests', () => {
    test('should handle large capability objects (100MB)', () => {
      const initialMemory = process.memoryUsage().heapUsed;
      const largeCapabilities: { [key: string]: string } = {};

      // Create 10,000 capability entries with large values
      for (let i = 0; i < 10000; i++) {
        largeCapabilities[`capability_${i}`] = `value_${i}_` + 'x'.repeat(1000); // 1KB per value
      }

      const envelope = UMICP.createEnvelope({
        from: 'memory_test_sender',
        to: 'memory_test_receiver',
        capabilities: largeCapabilities
      });

      expect(envelope.validate()).toBe(true);

      const serialized = envelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(deserialized.validate()).toBe(true);

      const finalMemory = process.memoryUsage().heapUsed;
      const memoryDelta = finalMemory - initialMemory;

      console.log(`Memory Load Test - Large Capabilities:`);
      console.log(`  Capability entries: ${Object.keys(largeCapabilities).length}`);
      console.log(`  Serialized size: ${(serialized.length / 1024 / 1024).toFixed(2)} MB`);
      console.log(`  Memory delta: ${(memoryDelta / 1024 / 1024).toFixed(2)} MB`);

      // Should handle large objects without excessive memory usage
      expect(memoryDelta).toBeLessThan(200 * 1024 * 1024); // Less than 200MB
    });

    test('should handle rapid serialization/deserialization cycles', () => {
      const iterations = 5000;
      const envelopes = [];

      // Create base envelope
      const baseEnvelope = UMICP.createEnvelope({
        from: 'serialization_test',
        to: 'serialization_receiver',
        capabilities: {
          'test_data': 'x'.repeat(1000),
          'sequence': '0'
        }
      });

      const startTime = performance.now();

      for (let i = 0; i < iterations; i++) {
        // Modify capabilities for each iteration
        const envelope = UMICP.createEnvelope({
          from: 'serialization_test',
          to: 'serialization_receiver',
          capabilities: {
            'test_data': 'x'.repeat(1000),
            'sequence': i.toString(),
            'timestamp': Date.now().toString()
          }
        });

        const serialized = envelope.serialize();
        const deserialized = Envelope.deserialize(serialized);
        envelopes.push(deserialized);
      }

      const endTime = performance.now();
      const duration = endTime - startTime;
      const operationsPerSecond = iterations / (duration / 1000);

      console.log(`Memory Load Test - Serialization Cycles:`);
      console.log(`  Iterations: ${iterations}`);
      console.log(`  Duration: ${duration.toFixed(2)}ms`);
      console.log(`  Rate: ${operationsPerSecond.toFixed(0)} operations/sec`);

      envelopes.forEach((envelope, index) => {
        expect(envelope.validate()).toBe(true);
        expect(envelope.getCapabilities()['sequence']).toBe(index.toString());
      });

      expect(duration).toBeLessThan(10000); // Should complete within 10 seconds
    });
  });

  describe('Stress Tests', () => {
    test('should handle extreme numerical ranges', () => {
      const testCases = [
        {
          name: 'Very Large Numbers',
          values: [1e30, 1e30, -1e30, -1e30]
        },
        {
          name: 'Very Small Numbers',
          values: [1e-30, 1e-30, -1e-30, -1e-30]
        },
        {
          name: 'Mixed Ranges',
          values: [1e20, 1e-20, -1e20, -1e-20]
        },
        {
          name: 'Alternating Signs',
          values: [1e15, -1e15, 1e15, -1e15]
        },
        {
          name: 'Near Zero',
          values: [1e-15, -1e-15, 1e-15, -1e-15]
        }
      ];

      testCases.forEach(testCase => {
        const a = new Float32Array(testCase.values);
        const b = new Float32Array(testCase.values.map(v => v * 0.5));
        const result = new Float32Array(4);

        const operationResult = matrix.add(a, b, result, 4, 1);

        console.log(`${testCase.name}: Operation ${operationResult.success ? 'succeeded' : 'failed'}`);

        // Operation should complete (may overflow/underflow, but shouldn't crash)
        expect(typeof operationResult.success).toBe('boolean');
      });
    });

    test('should handle maximum array sizes', () => {
      // Test with largest reasonable array size
      const maxSize = 1000000; // 1 million elements
      const a = new Float32Array(maxSize);
      const b = new Float32Array(maxSize);
      const result = new Float32Array(maxSize);

      // Initialize with simple values
      for (let i = 0; i < maxSize; i++) {
        a[i] = 1.0;
        b[i] = 2.0;
      }

      const startTime = performance.now();
      const operationResult = matrix.add(a, b, result, maxSize, 1);
      const endTime = performance.now();

      const duration = endTime - startTime;

      console.log(`Stress Test - Maximum Array Size:`);
      console.log(`  Array size: ${maxSize}`);
      console.log(`  Duration: ${duration.toFixed(2)}ms`);
      console.log(`  Memory usage: ~${(maxSize * 4 * 3 / 1024 / 1024).toFixed(2)} MB`);

      expect(operationResult.success).toBe(true);
      expect(result[0]).toBeCloseTo(3.0, 5); // 1.0 + 2.0
      expect(duration).toBeLessThan(5000); // Should complete within 5 seconds
    });

    test('should handle high-frequency envelope validation', () => {
      const iterations = 100000;
      const envelopes = [];

      // Pre-create envelopes
      for (let i = 0; i < iterations; i++) {
        envelopes.push(UMICP.createEnvelope({
          from: `stress_sender_${i % 100}`, // Limited variety
          to: `stress_receiver_${i % 50}`,
          messageId: `stress_msg_${i}`
        }));
      }

      const startTime = performance.now();

      // Validate all envelopes
      let validCount = 0;
      envelopes.forEach(envelope => {
        if (envelope.validate()) {
          validCount++;
        }
      });

      const endTime = performance.now();
      const duration = endTime - startTime;
      const validationsPerSecond = iterations / (duration / 1000);

      console.log(`Stress Test - Envelope Validation:`);
      console.log(`  Validations: ${iterations}`);
      console.log(`  Valid count: ${validCount}`);
      console.log(`  Duration: ${duration.toFixed(2)}ms`);
      console.log(`  Rate: ${validationsPerSecond.toFixed(0)} validations/sec`);

      expect(validCount).toBe(iterations); // All should be valid
      expect(validationsPerSecond).toBeGreaterThan(10000); // At least 10K validations/sec
    });

    test('should handle hash generation under load', () => {
      const iterations = 50000;
      const hashes = new Set<string>();

      const startTime = performance.now();

      for (let i = 0; i < iterations; i++) {
        const envelope = UMICP.createEnvelope({
          from: 'hash_test_sender',
          to: 'hash_test_receiver',
          messageId: `hash_msg_${i}_${Date.now()}`
        });

        const hash = envelope.getHash();
        hashes.add(hash);
      }

      const endTime = performance.now();
      const duration = endTime - startTime;
      const hashesPerSecond = iterations / (duration / 1000);

      console.log(`Stress Test - Hash Generation:`);
      console.log(`  Hashes generated: ${iterations}`);
      console.log(`  Unique hashes: ${hashes.size}`);
      console.log(`  Duration: ${duration.toFixed(2)}ms`);
      console.log(`  Rate: ${hashesPerSecond.toFixed(0)} hashes/sec`);

      expect(hashes.size).toBe(iterations); // All hashes should be unique
      expect(hashesPerSecond).toBeGreaterThan(5000); // At least 5K hashes/sec
    });
  });

  describe('Endurance Tests', () => {
    test('should maintain performance over extended period (30 seconds)', () => {
      const duration = 30000; // 30 seconds
      const startTime = Date.now();
      let operationCount = 0;
      const performanceSamples = [];

      const size = 1000;
      const a = new Float32Array(size);
      const b = new Float32Array(size);
      const result = new Float32Array(size);

      for (let i = 0; i < size; i++) {
        a[i] = Math.sin(i * 0.001);
        b[i] = Math.cos(i * 0.001);
      }

      while (Date.now() - startTime < duration) {
        const opStartTime = performance.now();
        const operationResult = matrix.add(a, b, result, size, 1);
        const opEndTime = performance.now();

        expect(operationResult.success).toBe(true);
        operationCount++;

        // Sample performance every 100 operations
        if (operationCount % 100 === 0) {
          performanceSamples.push(opEndTime - opStartTime);
        }
      }

      const actualDuration = Date.now() - startTime;
      const operationsPerSecond = operationCount / (actualDuration / 1000);

      const avgOperationTime = performanceSamples.reduce((sum, time) => sum + time, 0) / performanceSamples.length;
      const minOperationTime = Math.min(...performanceSamples);
      const maxOperationTime = Math.max(...performanceSamples);

      console.log(`Endurance Test - Extended Period:`);
      console.log(`  Operations: ${operationCount}`);
      console.log(`  Duration: ${actualDuration}ms`);
      console.log(`  Rate: ${operationsPerSecond.toFixed(0)} operations/sec`);
      console.log(`  Avg operation time: ${avgOperationTime.toFixed(4)}ms`);
      console.log(`  Min operation time: ${minOperationTime.toFixed(4)}ms`);
      console.log(`  Max operation time: ${maxOperationTime.toFixed(4)}ms`);

      expect(operationCount).toBeGreaterThan(10000); // At least 10K operations
      expect(operationsPerSecond).toBeGreaterThan(200); // At least 200 ops/sec
      expect(avgOperationTime).toBeLessThan(10); // Average under 10ms
    });

    test('should handle memory stability over time', () => {
      const duration = 20000; // 20 seconds
      const startTime = Date.now();
      const memorySamples = [];
      let operationCount = 0;

      while (Date.now() - startTime < duration) {
        // Create and process envelope
        const envelope = UMICP.createEnvelope({
          from: 'memory_stability_sender',
          to: 'memory_stability_receiver',
          messageId: `memory_msg_${operationCount}_${Date.now()}`,
          capabilities: {
            'large_data': 'x'.repeat(1000),
            'sequence': operationCount.toString()
          }
        });

        const serialized = envelope.serialize();
        const deserialized = Envelope.deserialize(serialized);

        expect(deserialized.validate()).toBe(true);
        operationCount++;

        // Sample memory every 50 operations
        if (operationCount % 50 === 0) {
          const memUsage = process.memoryUsage();
          memorySamples.push(memUsage.heapUsed);

          // Force garbage collection if available
          if (global.gc) {
            global.gc();
          }
        }
      }

      const actualDuration = Date.now() - startTime;
      const operationsPerSecond = operationCount / (actualDuration / 1000);

      const initialMemory = memorySamples[0];
      const finalMemory = memorySamples[memorySamples.length - 1];
      const memoryDelta = finalMemory - initialMemory;
      const maxMemory = Math.max(...memorySamples);
      const minMemory = Math.min(...memorySamples);

      console.log(`Memory Stability Test:`);
      console.log(`  Operations: ${operationCount}`);
      console.log(`  Duration: ${actualDuration}ms`);
      console.log(`  Rate: ${operationsPerSecond.toFixed(0)} operations/sec`);
      console.log(`  Initial memory: ${(initialMemory / 1024 / 1024).toFixed(2)} MB`);
      console.log(`  Final memory: ${(finalMemory / 1024 / 1024).toFixed(2)} MB`);
      console.log(`  Memory delta: ${(memoryDelta / 1024 / 1024).toFixed(2)} MB`);
      console.log(`  Max memory: ${(maxMemory / 1024 / 1024).toFixed(2)} MB`);
      console.log(`  Min memory: ${(minMemory / 1024 / 1024).toFixed(2)} MB`);

      expect(operationCount).toBeGreaterThan(5000); // At least 5K operations
      expect(Math.abs(memoryDelta)).toBeLessThan(50 * 1024 * 1024); // Memory delta < 50MB
    });
  });
});
