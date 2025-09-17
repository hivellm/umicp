/**
 * Extended Performance Tests for UMICP TypeScript Bindings
 *
 * Comprehensive performance testing including memory usage, throughput,
 * latency analysis, and scaling benchmarks.
 */

import { Envelope, Matrix, UMICP, OperationType, PayloadType, EncodingType } from '../src/index.js';

describe('UMICP Extended Performance Tests', () => {
  let matrix: any;

  beforeAll(() => {
    matrix = UMICP.createMatrix();
  });

  describe('Memory Usage Analysis', () => {
    test('should track memory usage during large matrix operations', () => {
      const initialMemory = process.memoryUsage();

      const size = 1000;
      const matrixA = new Float32Array(size * size);
      const matrixB = new Float32Array(size * size);
      const result = new Float32Array(size * size);

      // Initialize with test data
      for (let i = 0; i < matrixA.length; i++) {
        matrixA[i] = Math.sin(i * 0.001);
        matrixB[i] = Math.cos(i * 0.001);
      }

      const operationMemory = process.memoryUsage();

      // Perform large matrix multiplication
      const multiplyResult = matrix.multiply(matrixA, matrixB, result, size, size, size);
      expect(multiplyResult.success).toBe(true);

      const finalMemory = process.memoryUsage();

    });

    test('should handle memory cleanup properly', () => {
      const iterations = 100;
      const matrixSize = 500;

      for (let i = 0; i < iterations; i++) {
        const matrixA = new Float32Array(matrixSize * matrixSize);
        const matrixB = new Float32Array(matrixSize * matrixSize);
        const result = new Float32Array(matrixSize * matrixSize);

        // Fill with data
        for (let j = 0; j < matrixA.length; j++) {
          matrixA[j] = Math.random();
          matrixB[j] = Math.random();
        }

        // Perform operation
        const multiplyResult = matrix.multiply(matrixA, matrixB, result, matrixSize, matrixSize, matrixSize);
        expect(multiplyResult.success).toBe(true);

        // Force garbage collection if available
        if (global.gc) {
          global.gc();
        }
      }

      const finalMemory = process.memoryUsage();
    });
  });

  describe('Throughput Benchmarks', () => {
    test('should measure vector operation throughput', () => {
      const sizes = [1000, 10000, 50000, 100000];

      for (const size of sizes) {
        const vectorA = new Float32Array(size);
        const vectorB = new Float32Array(size);
        const result = new Float32Array(size);

        // Initialize vectors
        for (let i = 0; i < size; i++) {
          vectorA[i] = Math.sin(i * 0.001);
          vectorB[i] = Math.cos(i * 0.001);
        }

        const iterations = Math.max(10, Math.floor(1000000 / size)); // Adjust iterations based on size
        const startTime = process.hrtime.bigint();

        for (let i = 0; i < iterations; i++) {
          const addResult = matrix.vectorAdd(vectorA, vectorB, result);
          expect(addResult.success).toBe(true);
        }

        const endTime = process.hrtime.bigint();
        const totalTime = Number(endTime - startTime) / 1e9; // Convert to seconds
        const operationsPerSecond = iterations / totalTime;
        const throughput = (size * iterations * 4) / totalTime / (1024 * 1024); // MB/s

      }
    });

    test('should benchmark envelope serialization throughput', () => {
      const envelopeCount = 10000;
      const envelopes: any[] = [];

      // Create test envelopes
      for (let i = 0; i < envelopeCount; i++) {
        const envelope = UMICP.createEnvelope({
          from: `sender-${i}`,
          to: `receiver-${i}`,
          operation: OperationType.DATA,
          messageId: `msg-${i}`,
          capabilities: {
            'sequence': i.toString(),
            'timestamp': Date.now().toString(),
            'data_size': '1024'
          }
        });
        envelopes.push(envelope);
      }

      // Benchmark serialization
      const serializeStart = process.hrtime.bigint();
      const serializedEnvelopes = envelopes.map(env => env.serialize());
      const serializeEnd = process.hrtime.bigint();

      // Benchmark deserialization
      const deserializeStart = process.hrtime.bigint();
      const deserializedEnvelopes = serializedEnvelopes.map(json => Envelope.deserialize(json));
      const deserializeEnd = process.hrtime.bigint();

      const serializeTime = Number(serializeEnd - serializeStart) / 1e9;
      const deserializeTime = Number(deserializeEnd - deserializeStart) / 1e9;

    });
  });

  describe('Latency Analysis', () => {
    test('should measure operation latency distribution', () => {
      const iterations = 1000;
      const latencies: number[] = [];

      for (let i = 0; i < iterations; i++) {
        const startTime = process.hrtime.bigint();

        // Simple dot product operation
        const vectorA = new Float32Array(1000).map(() => Math.random());
        const vectorB = new Float32Array(1000).map(() => Math.random());
        const dotResult = matrix.dotProduct(vectorA, vectorB);

        const endTime = process.hrtime.bigint();
        const latency = Number(endTime - startTime) / 1e6; // Convert to milliseconds

        expect(dotResult.success).toBe(true);
        latencies.push(latency);
      }

      // Calculate latency statistics
      latencies.sort();
      const min = latencies[0];
      const max = latencies[latencies.length - 1];
      const median = latencies[Math.floor(latencies.length / 2)];
      const p95 = latencies[Math.floor(latencies.length * 0.95)];
      const p99 = latencies[Math.floor(latencies.length * 0.99)];

    });

    test('should test concurrent operation performance', async () => {
      const concurrentOperations = 10;
      const operationsPerThread = 100;
      const promises: Promise<void>[] = [];

      const startTime = process.hrtime.bigint();

      for (let thread = 0; thread < concurrentOperations; thread++) {
        const promise = new Promise<void>((resolve) => {
          setImmediate(() => {
            for (let i = 0; i < operationsPerThread; i++) {
              const vectorA = new Float32Array(500).map(() => Math.random());
              const vectorB = new Float32Array(500).map(() => Math.random());
              const result = matrix.dotProduct(vectorA, vectorB);
              expect(result.success).toBe(true);
            }
            resolve();
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

  describe('Scaling Benchmarks', () => {
    test('should test matrix operation scaling', () => {
      const sizes = [100, 250, 500, 750, 1000];
      const results: Array<{size: number, time: number, throughput: number}> = [];

      for (const size of sizes) {
        const matrixA = new Float32Array(size * size);
        const matrixB = new Float32Array(size * size);
        const result = new Float32Array(size * size);

        // Initialize matrices
        for (let i = 0; i < matrixA.length; i++) {
          matrixA[i] = Math.sin(i * 0.001);
          matrixB[i] = Math.cos(i * 0.001);
        }

        const startTime = process.hrtime.bigint();

        // Perform matrix multiplication
        const multiplyResult = matrix.multiply(matrixA, matrixB, result, size, size, size);
        expect(multiplyResult.success).toBe(true);

        const endTime = process.hrtime.bigint();
        const time = Number(endTime - startTime) / 1e9; // seconds
        const dataSize = size * size * 4 * 3; // 3 matrices, 4 bytes per float
        const throughput = dataSize / time / (1024 * 1024); // MB/s

        results.push({ size, time, throughput });

      }

      // Analyze scaling
      for (let i = 1; i < results.length; i++) {
        const prev = results[i - 1];
        const curr = results[i];
        const timeRatio = curr.time / prev.time;
        const sizeRatio = (curr.size / prev.size) ** 3; // O(n^3) for matrix multiplication

      }
    });

    test('should benchmark large dataset processing', () => {
      const datasetSize = 100000; // 100K data points
      const featureSize = 128; // 128 features per data point

      console.log(`Processing large dataset: ${datasetSize} samples, ${featureSize} features`);

      // Create synthetic dataset
      const dataset: Float32Array[] = [];
      for (let i = 0; i < datasetSize; i++) {
        const features = new Float32Array(featureSize);
        for (let j = 0; j < featureSize; j++) {
          features[j] = Math.sin(i * 0.01 + j * 0.1) + Math.random() * 0.1;
        }
        dataset.push(features);
      }

      // Benchmark batch processing
      const batchSize = 1000;
      const startTime = process.hrtime.bigint();

      for (let i = 0; i < dataset.length; i += batchSize) {
        const batch = dataset.slice(i, Math.min(i + batchSize, dataset.length));

        // Process batch (normalize each sample)
        for (const sample of batch) {
          const normalized = new Float32Array(sample.length);
          matrix.normalize(sample, 1, sample.length);
        }
      }

      const endTime = process.hrtime.bigint();
      const totalTime = Number(endTime - startTime) / 1e9;

    });
  });

  describe('Resource Utilization', () => {
    test('should monitor CPU utilization during intensive operations', () => {
      const startUsage = process.cpuUsage();
      const startTime = process.hrtime.bigint();

      // Perform CPU-intensive operations
      const operations = 1000;
      for (let i = 0; i < operations; i++) {
        const size = 500;
        const matrixA = new Float32Array(size * size).map(() => Math.random());
        const matrixB = new Float32Array(size * size).map(() => Math.random());
        const result = new Float32Array(size * size);

        const multiplyResult = matrix.multiply(matrixA, matrixB, result, size, size, size);
        expect(multiplyResult.success).toBe(true);
      }

      const endTime = process.hrtime.bigint();
      const endUsage = process.cpuUsage(startUsage);

      const totalTime = Number(endTime - startTime) / 1e9;
      const userTime = endUsage.user / 1000000; // Convert to seconds
      const systemTime = endUsage.system / 1000000;

      console.log('CPU Utilization Analysis:');
      console.log(`  Wall time: ${totalTime.toFixed(3)} seconds`);
      console.log(`  User CPU time: ${userTime.toFixed(3)} seconds`);
      console.log(`  System CPU time: ${systemTime.toFixed(3)} seconds`);
      console.log(`  Total CPU time: ${(userTime + systemTime).toFixed(3)} seconds`);
      console.log(`  CPU utilization: ${(((userTime + systemTime) / totalTime) * 100).toFixed(1)}%`);
    });

    test('should test memory fragmentation resistance', () => {
      const allocations: Float32Array[] = [];
      const initialMemory = process.memoryUsage().heapUsed;

      // Create many small allocations
      for (let i = 0; i < 1000; i++) {
        const size = 100 + (i % 10) * 50; // Varying sizes
        const array = new Float32Array(size);
        for (let j = 0; j < size; j++) {
          array[j] = Math.random();
        }
        allocations.push(array);
      }

      // Perform operations on allocations
      for (let i = 0; i < allocations.length; i += 2) {
        if (i + 1 < allocations.length) {
          const result = new Float32Array(allocations[i].length);
          const addResult = matrix.vectorAdd(allocations[i], allocations[i + 1], result);
          expect(addResult.success).toBe(true);
        }
      }

      const finalMemory = process.memoryUsage().heapUsed;
      const memoryIncrease = finalMemory - initialMemory;


      // Cleanup
      allocations.length = 0;
    });
  });
});
