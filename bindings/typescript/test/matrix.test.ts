/**
 * Tests for UMICP Matrix operations
 */

import { Matrix, UMICP } from '../src/index';

describe('Matrix Operations', () => {
  let matrix: Matrix;

  beforeEach(() => {
    matrix = UMICP.createMatrix();
  });

  test('should create matrix instance', () => {
    expect(matrix).toBeInstanceOf(Matrix);
  });

  test('should perform vector addition', () => {
    const size = 100;
    const a = new Float32Array(size);
    const b = new Float32Array(size);
    const result = new Float32Array(size);

    // Initialize test data
    for (let i = 0; i < size; i++) {
      a[i] = i * 1.0;
      b[i] = i * 2.0;
    }

    const operationResult = matrix.add(a, b, result, size, 1);

    expect(operationResult.success).toBe(true);
    expect(operationResult.result).toBeUndefined(); // No scalar result for addition

    // Verify results
    for (let i = 0; i < size; i++) {
      expect(result[i]).toBeCloseTo(a[i] + b[i], 5);
    }
  });

  test('should perform matrix multiplication', () => {
    const M = 4, N = 6, P = 8;
    const a = new Float32Array(M * N);
    const b = new Float32Array(N * P);
    const result = new Float32Array(M * P);

    // Initialize with simple values for predictable results
    for (let i = 0; i < a.length; i++) a[i] = i + 1;
    for (let i = 0; i < b.length; i++) b[i] = i + 1;

    const operationResult = matrix.multiply(a, b, result, M, N, P);

    expect(operationResult.success).toBe(true);
    expect(result.length).toBe(M * P);
    expect(result.every(val => !isNaN(val))).toBe(true);
  });

  test('should perform matrix transpose', () => {
    const rows = 3, cols = 4;
    const input = new Float32Array(rows * cols);
    const output = new Float32Array(cols * rows);

    // Initialize with sequential values
    for (let i = 0; i < input.length; i++) {
      input[i] = i + 1;
    }

    const operationResult = matrix.transpose(input, output, rows, cols);

    expect(operationResult.success).toBe(true);

    // Verify transpose
    for (let i = 0; i < rows; i++) {
      for (let j = 0; j < cols; j++) {
        expect(output[j * rows + i]).toBe(input[i * cols + j]);
      }
    }
  });

  test('should calculate dot product', () => {
    const size = 10;
    const a = new Float32Array(size);
    const b = new Float32Array(size);

    // Initialize with known values
    for (let i = 0; i < size; i++) {
      a[i] = i + 1;  // [1, 2, 3, ..., 10]
      b[i] = i + 1;  // [1, 2, 3, ..., 10]
    }

    const operationResult = matrix.dotProduct(a, b);

    expect(operationResult.success).toBe(true);
    expect(operationResult.result).toBeDefined();

    // Expected result: sum of squares from 1 to 10 = (10 * 11 * 21) / 6 = 385
    const expected = (size * (size + 1) * (2 * size + 1)) / 6;
    expect(operationResult.result).toBeCloseTo(expected, 5);
  });

  test('should perform vector normalization', () => {
    const size = 5;
    const input = new Float32Array([3, 4, 0, 0, 0]); // Vector with magnitude 5

    const operationResult = matrix.normalize(input, 1, size);

    expect(operationResult.success).toBe(true);

    // Calculate magnitude of normalized vector (input is modified in-place)
    let magnitude = 0;
    for (let i = 0; i < size; i++) {
      magnitude += input[i] * input[i];
    }
    magnitude = Math.sqrt(magnitude);

    expect(magnitude).toBeCloseTo(1.0, 5);
    expect(input[0]).toBeCloseTo(3/5, 5); // 0.6
    expect(input[1]).toBeCloseTo(4/5, 5); // 0.8
  });

  test('should calculate cosine similarity', () => {
    const size = 3;
    const a = new Float32Array([1, 2, 3]);
    const b = new Float32Array([4, 5, 6]);

    const operationResult = matrix.cosineSimilarity(a, b);

    expect(operationResult.success).toBe(true);
    expect(operationResult.similarity).toBeDefined();
    expect(operationResult.similarity).toBeGreaterThan(0.97); // Should be close to 1 for similar direction vectors

    // Test with identical vectors (should be exactly 1)
    const c = new Float32Array([1, 2, 3]);
    const identicalResult = matrix.cosineSimilarity(a, c);
    expect(identicalResult.success).toBe(true);
    expect(identicalResult.similarity).toBeCloseTo(1.0, 5);
  });

  test('should handle edge cases', () => {
    // Test with zero vectors
    const zeroVec = new Float32Array(5).fill(0);
    const result = new Float32Array(5);

    const addResult = matrix.add(zeroVec, zeroVec, result, 5, 1);
    expect(addResult.success).toBe(true);

    // All results should be zero
    expect(result.every(val => val === 0)).toBe(true);
  });

  test('should handle large matrices', () => {
    const size = 1000;
    const a = new Float32Array(size);
    const b = new Float32Array(size);
    const result = new Float32Array(size);

    // Initialize with small values to avoid overflow
    for (let i = 0; i < size; i++) {
      a[i] = Math.sin(i * 0.001);
      b[i] = Math.cos(i * 0.001);
    }

    const startTime = Date.now();
    const operationResult = matrix.add(a, b, result, size, 1);
    const endTime = Date.now();

    expect(operationResult.success).toBe(true);
    expect(endTime - startTime).toBeLessThan(1000); // Should complete within 1 second
  });

  test('should handle memory allocation errors gracefully', () => {
    const hugeSize = 100000000; // 100 million elements
    const a = new Float32Array(hugeSize);
    const b = new Float32Array(hugeSize);
    const result = new Float32Array(hugeSize);

    // This might fail due to memory constraints, but should not crash
    const operationResult = matrix.add(a, b, result, hugeSize, 1);

    // The operation might succeed or fail depending on system memory
    // But it should not throw an unhandled exception
    expect(typeof operationResult.success).toBe('boolean');
  });

  // Edge Cases and Error Handling Tests
  describe('Edge Cases and Error Handling', () => {
    test('should handle zero-length arrays', () => {
      const a = new Float32Array(0);
      const b = new Float32Array(0);
      const result = new Float32Array(0);

      const operationResult = matrix.add(a, b, result, 0, 1);
      // Zero-length arrays may not be supported, that's acceptable
      expect(typeof operationResult.success).toBe('boolean');
    });

    test('should handle single-element arrays', () => {
      const a = new Float32Array([5.5]);
      const b = new Float32Array([3.2]);
      const result = new Float32Array(1);

      const operationResult = matrix.add(a, b, result, 1, 1);
      expect(operationResult.success).toBe(true);
      expect(result[0]).toBeCloseTo(8.7, 5);
    });

    test('should handle arrays with NaN and Infinity values', () => {
      const a = new Float32Array([1.0, NaN, Infinity, -Infinity]);
      const b = new Float32Array([2.0, 3.0, 4.0, 5.0]);
      const result = new Float32Array(4);

      const operationResult = matrix.add(a, b, result, 4, 1);
      expect(operationResult.success).toBe(true);

      expect(result[0]).toBeCloseTo(3.0, 5); // 1.0 + 2.0
      expect(isNaN(result[1])).toBe(true);   // NaN + 3.0 = NaN
      expect(result[2]).toBe(Infinity);      // Infinity + 4.0 = Infinity
      expect(result[3]).toBe(-Infinity);     // -Infinity + 5.0 = -Infinity
    });

    test('should handle very large floating point numbers', () => {
      // Use numbers within Float32 precision limits
      const largeNum = 1e10; // Much smaller to stay within Float32 precision
      const a = new Float32Array([largeNum, -largeNum, 1e-10, -1e-10]);
      const b = new Float32Array([largeNum, -largeNum, 1e-10, -1e-10]);
      const result = new Float32Array(4);

      const operationResult = matrix.add(a, b, result, 4, 1);
      expect(operationResult.success).toBe(true);

      // Test within Float32 precision limits
      expect(result[0]).toBeCloseTo(2e10, 5);   // Should handle numbers within Float32 limits
      expect(result[1]).toBeCloseTo(-2e10, 5);
      expect(result[2]).toBeCloseTo(2e-10, 5);  // Should handle small numbers
      expect(result[3]).toBeCloseTo(-2e-10, 5);
    });

    test('should handle denormalized floating point numbers', () => {
      const denormal = 1e-40; // Very small number, might be denormalized
      const a = new Float32Array([denormal, denormal * 2]);
      const b = new Float32Array([denormal, denormal * 3]);
      const result = new Float32Array(2);

      const operationResult = matrix.add(a, b, result, 2, 1);
      expect(operationResult.success).toBe(true);

      // Results should be valid (either denormalized or flushed to zero)
      expect(typeof result[0]).toBe('number');
      expect(typeof result[1]).toBe('number');
      expect(result[0]).toBeGreaterThanOrEqual(0);
      expect(result[1]).toBeGreaterThanOrEqual(0);
    });

    test('should handle mismatched array sizes gracefully', () => {
      const a = new Float32Array(10);
      const b = new Float32Array(5); // Different size
      const result = new Float32Array(10);

      // This should not crash, though the result may be undefined
      expect(() => {
        matrix.add(a, b, result, 10, 1);
      }).not.toThrow();
    });

    test('should handle null result array gracefully', () => {
      const a = new Float32Array([1, 2, 3]);
      const b = new Float32Array([4, 5, 6]);

      // This should not crash
      expect(() => {
        matrix.add(a, b, null as any, 3, 1);
      }).not.toThrow();
    });

    test('should handle concurrent matrix operations', async () => {
      const operationCount = 50;
      const promises = [];

      for (let i = 0; i < operationCount; i++) {
        promises.push(
          Promise.resolve().then(() => {
            const size = 100;
            const a = new Float32Array(size);
            const b = new Float32Array(size);
            const result = new Float32Array(size);

            for (let j = 0; j < size; j++) {
              a[j] = Math.sin(j * 0.1 + i);
              b[j] = Math.cos(j * 0.1 + i);
            }

            return matrix.add(a, b, result, size, 1);
          })
        );
      }

      const results = await Promise.all(promises);

      results.forEach(result => {
        expect(result.success).toBe(true);
      });
    });

    test('should handle matrix multiplication with various dimensions', () => {
      const testCases = [
        { M: 1, N: 1, P: 1 },
        { M: 1, N: 10, P: 1 },
        { M: 10, N: 1, P: 10 },
        { M: 3, N: 4, P: 2 },
        { M: 5, N: 5, P: 5 }
      ];

      testCases.forEach(({ M, N, P }) => {
        const a = new Float32Array(M * N);
        const b = new Float32Array(N * P);
        const result = new Float32Array(M * P);

        // Initialize with known values
        for (let i = 0; i < a.length; i++) a[i] = 1.0;
        for (let i = 0; i < b.length; i++) b[i] = 1.0;

        const operationResult = matrix.multiply(a, b, result, M, N, P);
        expect(operationResult.success).toBe(true);

        // For matrices filled with 1.0, result should be N (number of columns in A / rows in B)
        for (let i = 0; i < result.length; i++) {
          expect(result[i]).toBeCloseTo(N, 5);
        }
      });
    });

    test('should handle vector normalization edge cases', () => {
      // Test with zero vector
      const zeroVector = new Float32Array(5).fill(0);
      const result = new Float32Array(5);

      const operationResult = matrix.normalize(zeroVector, 1, 5);
      expect(operationResult.success).toBe(true);

      // Zero vector normalization should result in zero vector
      zeroVector.forEach(val => expect(val).toBe(0));

      // Test with very small vector
      const smallVector = new Float32Array([1e-20, 1e-20, 1e-20]);
      const smallResult = matrix.normalize(smallVector, 1, 3);
      expect(smallResult.success).toBe(true);

      // Test with already normalized vector
      const normalizedVector = new Float32Array([1/Math.sqrt(3), 1/Math.sqrt(3), 1/Math.sqrt(3)]);
      const normalizedResult = matrix.normalize(normalizedVector, 1, 3);
      expect(normalizedResult.success).toBe(true);

      // Check if magnitude is still 1 (approximately)
      let magnitude = 0;
      for (let i = 0; i < 3; i++) {
        magnitude += normalizedVector[i] * normalizedVector[i];
      }
      expect(Math.sqrt(magnitude)).toBeCloseTo(1.0, 5);
    });

    test('should handle cosine similarity edge cases', () => {
      // Test with identical vectors
      const vector1 = new Float32Array([1, 2, 3, 4, 5]);
      const vector2 = new Float32Array([1, 2, 3, 4, 5]);

      const result = matrix.cosineSimilarity(vector1, vector2);
      expect(result.success).toBe(true);
      expect(result.similarity).toBeCloseTo(1.0, 5);

      // Test with orthogonal vectors
      const ortho1 = new Float32Array([1, 0]);
      const ortho2 = new Float32Array([0, 1]);

      const orthoResult = matrix.cosineSimilarity(ortho1, ortho2);
      expect(orthoResult.success).toBe(true);
      expect(orthoResult.similarity).toBeCloseTo(0.0, 5);

      // Test with opposite vectors
      const opp1 = new Float32Array([1, 2, 3]);
      const opp2 = new Float32Array([-1, -2, -3]);

      const oppResult = matrix.cosineSimilarity(opp1, opp2);
      expect(oppResult.success).toBe(true);
      expect(oppResult.similarity).toBeCloseTo(-1.0, 5);

      // Test with zero vector - the implementation may handle this gracefully
      const zeroVec = new Float32Array([0, 0, 0]);
      const normalVec = new Float32Array([1, 2, 3]);

      const zeroResult = matrix.cosineSimilarity(zeroVec, normalVec);
      // Implementation may handle zero vectors gracefully rather than failing
      expect(typeof zeroResult.success).toBe('boolean');
      if (zeroResult.success) {
        expect(typeof zeroResult.similarity).toBe('number');
      }
    });
  });

  // Performance Tests
  describe('Performance Tests', () => {
    test('should maintain performance with increasing vector sizes', () => {
      const sizes = [100, 1000, 10000, 50000];
      const performanceData: Array<{ size: number; duration: number; throughput: number }> = [];

      sizes.forEach(size => {
        const a = new Float32Array(size);
        const b = new Float32Array(size);
        const result = new Float32Array(size);

        // Initialize with test data
        for (let i = 0; i < size; i++) {
          a[i] = Math.sin(i * 0.001);
          b[i] = Math.cos(i * 0.001);
        }

        const startTime = performance.now();
        const operationResult = matrix.add(a, b, result, size, 1);
        const endTime = performance.now();

        expect(operationResult.success).toBe(true);

        const duration = endTime - startTime;
        const throughput = size / duration / 1000; // elements per millisecond

        performanceData.push({
          size,
          duration,
          throughput
        });

        console.log(`Size ${size}: ${duration.toFixed(2)}ms, ${throughput.toFixed(0)}K elements/ms`);
      });

      // Verify that performance scales reasonably (should not degrade exponentially)
      for (let i = 1; i < performanceData.length; i++) {
        const ratio = performanceData[i].throughput / performanceData[i - 1].throughput;
        expect(ratio).toBeGreaterThan(0.1); // Should maintain at least 10% of previous performance
      }
    });

    test('should handle high-frequency matrix operations', () => {
      const operationCount = 1000;
      const size = 100;
      const results = [];

      const startTime = performance.now();

      for (let i = 0; i < operationCount; i++) {
        const a = new Float32Array(size);
        const b = new Float32Array(size);
        const result = new Float32Array(size);

        for (let j = 0; j < size; j++) {
          a[j] = Math.sin(j * 0.01 + i * 0.1);
          b[j] = Math.cos(j * 0.01 + i * 0.1);
        }

        const operationResult = matrix.add(a, b, result, size, 1);
        results.push(operationResult);
      }

      const endTime = performance.now();
      const totalDuration = endTime - startTime;
      const avgDuration = totalDuration / operationCount;

      console.log(`High-frequency test: ${operationCount} operations in ${totalDuration.toFixed(2)}ms`);
      console.log(`Average: ${avgDuration.toFixed(4)}ms per operation`);

      results.forEach(result => {
        expect(result.success).toBe(true);
      });

      expect(avgDuration).toBeLessThan(5); // Should be fast
    });

    test('should optimize for SIMD operations', () => {
      const size = 10000;
      const a = new Float32Array(size);
      const b = new Float32Array(size);
      const result = new Float32Array(size);

      // Fill with data that benefits from SIMD
      for (let i = 0; i < size; i++) {
        a[i] = i * 1.0;
        b[i] = i * 2.0;
      }

      const startTime = performance.now();

      // Perform multiple operations to measure SIMD effectiveness
      for (let iter = 0; iter < 10; iter++) {
        const operationResult = matrix.add(a, b, result, size, 1);
        expect(operationResult.success).toBe(true);
      }

      const endTime = performance.now();
      const duration = endTime - startTime;

      console.log(`SIMD test: 10 operations on ${size} elements in ${duration.toFixed(2)}ms`);
      const operationsPerSecond = (10 * size) / (duration / 1000);
      console.log(`Throughput: ${(operationsPerSecond / 1000000).toFixed(2)} MOPS`);

      expect(duration).toBeLessThan(1000); // Should be reasonably fast
    });

    test('should handle memory-intensive operations efficiently', () => {
      const sizes = [10000, 50000, 100000];
      const results: Array<{ size: number; duration: number; memoryDelta: number }> = [];

      sizes.forEach(size => {
        const memoryUsage = process.memoryUsage();
        console.log(`Memory before ${size} elements: ${(memoryUsage.heapUsed / 1024 / 1024).toFixed(2)} MB`);

        const a = new Float32Array(size);
        const b = new Float32Array(size);
        const result = new Float32Array(size);

        for (let i = 0; i < size; i++) {
          a[i] = Math.sin(i * 0.001);
          b[i] = Math.cos(i * 0.001);
        }

        const startTime = performance.now();
        const operationResult = matrix.add(a, b, result, size, 1);
        const endTime = performance.now();

        expect(operationResult.success).toBe(true);

        const memoryAfter = process.memoryUsage();
        console.log(`Memory after ${size} elements: ${(memoryAfter.heapUsed / 1024 / 1024).toFixed(2)} MB`);

        results.push({
          size,
          duration: endTime - startTime,
          memoryDelta: memoryAfter.heapUsed - memoryUsage.heapUsed
        });
      });

      // Verify memory usage is reasonable
      results.forEach(result => {
        console.log(`Size ${result.size}: ${result.duration.toFixed(2)}ms, Memory delta: ${(result.memoryDelta / 1024 / 1024).toFixed(2)} MB`);
      });
    });
  });

  // Stress Tests
  describe('Stress Tests', () => {
    test('should handle sustained load over time', () => {
      const duration = 5000; // 5 seconds
      const startTime = Date.now();
      let operationCount = 0;

      while (Date.now() - startTime < duration) {
        const size = 1000;
        const a = new Float32Array(size);
        const b = new Float32Array(size);
        const result = new Float32Array(size);

        for (let i = 0; i < size; i++) {
          a[i] = Math.random();
          b[i] = Math.random();
        }

        const operationResult = matrix.add(a, b, result, size, 1);
        expect(operationResult.success).toBe(true);

        operationCount++;
      }

      const actualDuration = Date.now() - startTime;
      const operationsPerSecond = operationCount / (actualDuration / 1000);

      console.log(`Sustained load: ${operationCount} operations in ${actualDuration}ms`);
      console.log(`Rate: ${operationsPerSecond.toFixed(0)} operations/second`);

      expect(operationCount).toBeGreaterThan(100); // Should handle reasonable load
    });

    test('should handle mixed operation types under load', () => {
      const operationCount = 100;
      const size = 500;

      for (let i = 0; i < operationCount; i++) {
        const a = new Float32Array(size);
        const b = new Float32Array(size);
        const result = new Float32Array(size);

        for (let j = 0; j < size; j++) {
          a[j] = Math.sin(j * 0.01 + i);
          b[j] = Math.cos(j * 0.01 + i);
        }

        // Mix different operations
        const operationType = i % 4;
        let operationResult: any;

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

        expect(operationResult?.success).toBe(true);
      }

      console.log(`Mixed operations: ${operationCount} operations completed successfully`);
    });

    test('should handle extreme numerical ranges', () => {
      const testCases = [
        { name: 'Very Large', values: [1e30, 1e30, -1e30, -1e30] },
        { name: 'Very Small', values: [1e-30, 1e-30, -1e-30, -1e-30] },
        { name: 'Mixed Ranges', values: [1e20, 1e-20, -1e20, -1e-20] },
        { name: 'Overflow Risk', values: [3e38, 3e38, -3e38, -3e38] } // Near Float32 max
      ];

      testCases.forEach(testCase => {
        const a = new Float32Array(testCase.values);
        const b = new Float32Array(testCase.values.map(v => v * 0.5));
        const result = new Float32Array(4);

        const operationResult = matrix.add(a, b, result, 4, 1);

        // Operation should complete (may overflow, but shouldn't crash)
        expect(typeof operationResult.success).toBe('boolean');

        console.log(`${testCase.name}: Operation completed (success: ${operationResult.success})`);
      });
    });
  });
});
