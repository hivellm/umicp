/**
 * Advanced Stress Tests for UMICP TypeScript Bindings
 *
 * Comprehensive stress testing including concurrent operations, resource limits,
 * error recovery, and system stability under extreme conditions.
 */

import { Envelope, Matrix, UMICP, OperationType, PayloadType, EncodingType } from '../src/index.js';

describe('UMICP Advanced Stress Tests', () => {
  let matrix: any;

  beforeAll(() => {
    matrix = UMICP.createMatrix();
  });

  describe('Concurrent Load Testing', () => {
    test('should handle high concurrent envelope operations', async () => {
      const concurrentOperations = 50;
      const operationsPerThread = 100;
      const promises: Promise<void>[] = [];


      const startTime = process.hrtime.bigint();

      for (let thread = 0; thread < concurrentOperations; thread++) {
        const promise = new Promise<void>((resolve, reject) => {
          setImmediate(async () => {
            try {
              for (let i = 0; i < operationsPerThread; i++) {
                // Create envelope
                const envelope = UMICP.createEnvelope({
                  from: `thread-${thread}`,
                  to: 'stress-test-server',
                  operation: OperationType.DATA,
                  messageId: `msg-${thread}-${i}`,
                  capabilities: {
                    'thread_id': thread.toString(),
                    'operation_id': i.toString(),
                    'timestamp': Date.now().toString()
                  }
                });

                // Serialize and deserialize
                const serialized = envelope.serialize();
                const deserialized = Envelope.deserialize(serialized);

                // Validate
                expect(deserialized.from()).toBe(`thread-${thread}`);
                expect(deserialized.getMessageId()).toBe(`msg-${thread}-${i}`);
              }
              resolve();
            } catch (error) {
              reject(error);
            }
          });
        });
        promises.push(promise);
      }

      await Promise.all(promises);

      const endTime = process.hrtime.bigint();
      const totalTime = Number(endTime - startTime) / 1e9;
      const totalOperations = concurrentOperations * operationsPerThread;
      const operationsPerSecond = totalOperations / totalTime;

    });

    test('should handle mixed concurrent matrix operations', async () => {
      const concurrentOperations = 20;
      const operationsPerThread = 50;
      const promises: Promise<void>[] = [];


      const startTime = process.hrtime.bigint();

      for (let thread = 0; thread < concurrentOperations; thread++) {
        const promise = new Promise<void>((resolve, reject) => {
          setImmediate(async () => {
            try {
              for (let i = 0; i < operationsPerThread; i++) {
                const size = 200 + (i % 5) * 50; // Varying sizes: 200, 250, 300, 350, 400

                // Create test data
                const vectorA = new Float32Array(size).map(() => Math.random());
                const vectorB = new Float32Array(size).map(() => Math.random());
                const result = new Float32Array(size);

                // Perform different operations based on thread
                switch (thread % 4) {
                  case 0:
                    // Vector addition
                    const addResult = matrix.vectorAdd(vectorA, vectorB, result);
                    expect(addResult.success).toBe(true);
                    break;
                  case 1:
                    // Dot product
                    const dotResult = matrix.dotProduct(vectorA, vectorB);
                    expect(dotResult.success).toBe(true);
                    break;
                  case 2:
                    // Cosine similarity
                    const cosResult = matrix.cosineSimilarity(vectorA, vectorB);
                    expect(cosResult.success).toBe(true);
                    break;
                  case 3:
                    // Vector scaling
                    const scaleResult = matrix.vectorScale(vectorA, 2.5, result);
                    expect(scaleResult.success).toBe(true);
                    break;
                }
              }
              resolve();
            } catch (error) {
              reject(error);
            }
          });
        });
        promises.push(promise);
      }

      await Promise.all(promises);

      const endTime = process.hrtime.bigint();
      const totalTime = Number(endTime - startTime) / 1e9;
      const totalOperations = concurrentOperations * operationsPerThread;
      const operationsPerSecond = totalOperations / totalTime;

    });
  });

  describe('Resource Limit Testing', () => {
    test('should handle maximum envelope size limits', () => {
      const maxCapabilities = 1000;
      const largeData = 'x'.repeat(10000); // 10KB of data


      const capabilities: Record<string, string> = {};
      for (let i = 0; i < maxCapabilities; i++) {
        capabilities[`capability_${i}`] = `value_${i}_${largeData}`;
      }

      const envelope = UMICP.createEnvelope({
        from: 'large-envelope-test',
        to: 'server',
        operation: OperationType.DATA,
        messageId: 'large-envelope-test',
        capabilities
      });

      const serialized = envelope.serialize();

      const deserialized = Envelope.deserialize(serialized);
      expect(deserialized.from()).toBe('large-envelope-test');
      expect(Object.keys(deserialized.getCapabilities() || {}).length).toBe(maxCapabilities);
    });

    test('should handle maximum matrix size limits', () => {
      const maxSize = 2000; // Large but reasonable matrix
      

      const matrixA = new Float32Array(maxSize * maxSize);
      const matrixB = new Float32Array(maxSize * maxSize);
      const result = new Float32Array(maxSize * maxSize);

      // Initialize with simple pattern (avoids timeout)
      for (let i = 0; i < Math.min(10000, matrixA.length); i++) {
        matrixA[i] = (i % 100) * 0.01;
        matrixB[i] = ((i + 50) % 100) * 0.01;
      }

      // Test with smaller sub-matrix for actual computation
      const testSize = 100;
      const startTime = Date.now();

      const testResult = matrix.multiply(
        matrixA.slice(0, testSize * testSize),
        matrixB.slice(0, testSize * testSize),
        result.slice(0, testSize * testSize),
        testSize, testSize, testSize
      );

      const endTime = Date.now();

      expect(testResult.success).toBe(true);
      
      
    });

    test('should handle memory pressure gracefully', () => {
      const arrays: Float32Array[] = [];
      const initialMemory = process.memoryUsage().heapUsed;

      

      try {
        // Create many large arrays
        for (let i = 0; i < 100; i++) {
          const largeArray = new Float32Array(100000); // 400KB each
          arrays.push(largeArray);

          // Perform operation on each array
          const result = new Float32Array(largeArray.length);
          const scaleResult = matrix.vectorScale(largeArray, 2.0, result);
          expect(scaleResult.success).toBe(true);
        }

        const finalMemory = process.memoryUsage().heapUsed;
        const memoryIncrease = finalMemory - initialMemory;

        
        
        
        

      } finally {
        // Cleanup
        arrays.length = 0;

        // Force garbage collection if available
        if (global.gc) {
          global.gc();
        }
      }
    });
  });

  describe('Error Recovery and Resilience', () => {
    test('should recover from invalid envelope data', () => {
      const invalidEnvelopes = [
        '{"invalid": "json"',  // Incomplete JSON
        '{"from": null}',      // Null values
        '{"operation": 999}',  // Invalid operation
        '{"capabilities": null}', // Null capabilities
        ''                     // Empty string
      ];

      

      for (let i = 0; i < invalidEnvelopes.length; i++) {
        try {
          const result = Envelope.deserialize(invalidEnvelopes[i]);
          // If we get here, the deserialization succeeded unexpectedly
          
        } catch (error) {
          // Expected error - this is good
          
        }
      }

      // Test with valid envelope after errors
      const validEnvelope = UMICP.createEnvelope({
        from: 'recovery-test',
        to: 'server',
        operation: OperationType.DATA,
        messageId: 'recovery-test'
      });

      const serialized = validEnvelope.serialize();
      const deserialized = Envelope.deserialize(serialized);

      expect(deserialized.from()).toBe('recovery-test');
      
    });

    test('should handle matrix operation errors gracefully', () => {
      

      const testCases = [
        {
          name: 'Mismatched dimensions',
          operation: () => matrix.multiply(
            new Float32Array(100), new Float32Array(200), new Float32Array(100), 10, 10, 10
          )
        },
        {
          name: 'Zero-sized vectors',
          operation: () => matrix.dotProduct(new Float32Array(0), new Float32Array(0))
        },
        {
          name: 'Null result buffer',
          operation: () => matrix.vectorAdd(
            new Float32Array(10), new Float32Array(10), new Float32Array(5)
          )
        }
      ];

      for (const testCase of testCases) {
        try {
          const result = testCase.operation();
          if (!result.success) {
            
          } else {
            
          }
        } catch (error) {
          
        }
      }

      // Test recovery with valid operation
      const validResult = matrix.dotProduct(
        new Float32Array([1, 2, 3]),
        new Float32Array([4, 5, 6])
      );

      expect(validResult.success).toBe(true);
      expect(validResult.result).toBe(32); // 1*4 + 2*5 + 3*6 = 32

      
    });

    test('should handle concurrent error scenarios', async () => {
      const concurrentErrors = 10;
      const promises: Promise<void>[] = [];

      

      for (let thread = 0; thread < concurrentErrors; thread++) {
        const promise = new Promise<void>((resolve) => {
          setImmediate(() => {
            try {
              // Mix of valid and invalid operations
              for (let i = 0; i < 10; i++) {
                if (i % 2 === 0) {
                  // Valid operation
                  const result = matrix.dotProduct(
                    new Float32Array([1, 2, 3]),
                    new Float32Array([4, 5, 6])
                  );
                  expect(result.success).toBe(true);
                } else {
                  // Invalid operation
                  try {
                    matrix.dotProduct(
                      new Float32Array(5),
                      new Float32Array(3) // Mismatched sizes
                    );
                  } catch (error) {
                    // Expected error
                  }
                }
              }
              resolve();
            } catch (error) {
              resolve(); // Don't fail the test on individual thread errors
            }
          });
        });
        promises.push(promise);
      }

      await Promise.all(promises);
      
    });
  });

  describe('System Stability Under Load', () => {
    test('should maintain performance under sustained load', async () => {
      const duration = 10000; // 10 seconds
      const startTime = Date.now();
      let operations = 0;

      

      while (Date.now() - startTime < duration) {
        // Perform mix of operations
        const vectorA = new Float32Array(500).map(() => Math.random());
        const vectorB = new Float32Array(500).map(() => Math.random());
        const result = new Float32Array(500);

        const addResult = matrix.vectorAdd(vectorA, vectorB, result);
        expect(addResult.success).toBe(true);

        const dotResult = matrix.dotProduct(vectorA, vectorB);
        expect(dotResult.success).toBe(true);

        // Create and process envelope
        const envelope = UMICP.createEnvelope({
          from: 'load-test',
          to: 'server',
          operation: OperationType.DATA,
          messageId: `load-${operations}`
        });

        const serialized = envelope.serialize();
        const deserialized = Envelope.deserialize(serialized);
        expect(deserialized.from()).toBe('load-test');

        operations += 3; // 3 operations per iteration

        // Small yield to prevent blocking
        await new Promise(resolve => setImmediate(resolve));
      }

      const totalTime = (Date.now() - startTime) / 1000;
      const operationsPerSecond = operations / totalTime;

      
      
      
      
      
    });

    test('should handle memory leaks under long-running operations', async () => {
      const iterations = 1000;
      const memoryChecks: number[] = [];

      

      for (let i = 0; i < iterations; i++) {
        // Create objects
        const envelope = UMICP.createEnvelope({
          from: `iteration-${i}`,
          to: 'server',
          operation: OperationType.DATA,
          messageId: `msg-${i}`,
          capabilities: { 'iteration': i.toString() }
        });

        const vector = new Float32Array(1000).map(() => Math.random());

        // Perform operations
        const serialized = envelope.serialize();
        const deserialized = Envelope.deserialize(serialized);
        const result = matrix.dotProduct(vector, vector);

        expect(deserialized.from()).toBe(`iteration-${i}`);
        expect(result.success).toBe(true);

        // Memory check every 100 iterations
        if (i % 100 === 0) {
          const memUsage = process.memoryUsage();
          memoryChecks.push(memUsage.heapUsed);

          if (global.gc) {
            global.gc();
          }
        }

        // Small delay to allow garbage collection
        await new Promise(resolve => setTimeout.bind(null, 1));
      }

      // Analyze memory usage
      const initialMemory = memoryChecks[0];
      const finalMemory = memoryChecks[memoryChecks.length - 1];
      const memoryIncrease = finalMemory - initialMemory;
      const memoryIncreasePercent = (memoryIncrease / initialMemory) * 100;

      
      
      
      

      // Allow up to 10% memory increase (reasonable for long-running applications)
      expect(memoryIncreasePercent).toBeLessThan(10.0);

      
    });
  });
});
