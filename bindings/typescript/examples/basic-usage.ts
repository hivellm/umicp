/**
 * UMICP TypeScript/JavaScript Bindings - Basic Usage Example
 * Demonstrates high-performance C++ operations through Node.js bindings
 */

import { Envelope, Matrix, UMICP, OperationType, PayloadType, EncodingType } from '../src/index.js';

async function basicUMICPExample() {
  console.log('🚀 UMICP TypeScript Bindings - Basic Example');
  console.log('===========================================');
  console.log(`📦 Version: ${UMICP.version}`);
  console.log(`🎯 UMICP Version: ${UMICP.UMICP_VERSION}`);
  console.log();

  // Example 1: Envelope Operations
  console.log('📝 Example 1: Envelope Operations');
  console.log('----------------------------------');

  // Create envelope using fluent API
  const envelope = UMICP.createEnvelope({
    from: 'ai-model-a',
    to: 'ai-model-b',
    operation: OperationType.DATA,
    capabilities: {
      'binary_support': 'true',
      'compression': 'gzip,brotli',
      'formats': 'cbor,msgpack'
    },
    payloadHint: {
      type: PayloadType.VECTOR,
      size: 1024,
      encoding: EncodingType.FLOAT32,
      count: 256
    }
  });

  console.log('✅ Envelope created successfully');

  // Serialize to JSON
  const jsonStr = envelope.serialize();
  console.log('📄 Serialized envelope length:', jsonStr.length, 'bytes');

  // Validate envelope
  const isValid = envelope.validate();
  console.log('✅ Envelope validation:', isValid ? 'PASSED' : 'FAILED');

  // Get hash for integrity
  const hash = envelope.getHash();
  console.log('🔐 Envelope hash:', hash.substring(0, 16) + '...');

  // Deserialize from JSON
  const deserializedEnvelope = Envelope.deserialize(jsonStr);
  console.log('✅ Envelope deserialized successfully');
  console.log();

  // Example 2: High-Performance Matrix Operations
  console.log('🧮 Example 2: Matrix Operations (SIMD Accelerated)');
  console.log('--------------------------------------------------');

  const matrix = UMICP.createMatrix();

  // Create sample data (simulating embeddings)
  const size = 768;
  const a = new Float32Array(size);
  const b = new Float32Array(size);
  const result = new Float32Array(size);

  // Initialize with sample data
  for (let i = 0; i < size; i++) {
    a[i] = Math.sin(i * 0.01) * 0.5;
    b[i] = Math.cos(i * 0.01) * 0.3;
  }

  console.log(`📊 Operating on vectors of size ${size} (${size * 4} bytes each)`);

  // Vector addition
  const addResult = matrix.add(a, b, result, size, 1);
  if (addResult.success) {
    console.log('✅ Vector addition completed');
    console.log(`📈 Sample result: a[0]=${a[0].toFixed(4)}, b[0]=${b[0].toFixed(4)}, result[0]=${result[0].toFixed(4)}`);
  } else {
    console.error('❌ Vector addition failed:', addResult.error);
  }

  // Dot product
  const dotResult = matrix.dotProduct(a, b);
  if (dotResult.success && dotResult.result !== undefined) {
    console.log('✅ Dot product calculated:', dotResult.result.toFixed(6));
  } else {
    console.error('❌ Dot product failed:', dotResult.error);
  }

  // Cosine similarity
  const cosResult = matrix.cosineSimilarity(a, b);
  if (cosResult.success && cosResult.similarity !== undefined) {
    console.log('✅ Cosine similarity:', cosResult.similarity.toFixed(6));
  } else {
    console.error('❌ Cosine similarity failed:', cosResult.error);
  }

  // Vector normalization
  const normalizedA = new Float32Array(a); // Copy for normalization
  const normResult = matrix.normalize(normalizedA, 1, size);
  if (normResult.success) {
    console.log('✅ Vector normalization completed');
    console.log(`📏 Original magnitude: ${Math.sqrt(a.reduce((sum, val) => sum + val * val, 0)).toFixed(4)}`);
    console.log(`📏 Normalized magnitude: ${Math.sqrt(normalizedA.reduce((sum, val) => sum + val * val, 0)).toFixed(4)}`);
  } else {
    console.error('❌ Vector normalization failed:', normResult.error);
  }

  console.log();

  // Example 3: Matrix Multiplication
  console.log('🔢 Example 3: Matrix Multiplication');
  console.log('-----------------------------------');

  const M = 64, N = 128, P = 256;
  const matrixA = new Float32Array(M * N);
  const matrixB = new Float32Array(N * P);
  const matrixResult = new Float32Array(M * P);

  // Initialize matrices with sample data
  for (let i = 0; i < matrixA.length; i++) {
    matrixA[i] = (i % 100) * 0.01;
  }
  for (let i = 0; i < matrixB.length; i++) {
    matrixB[i] = ((i + 50) % 100) * 0.01;
  }

  console.log(`📊 Matrix multiplication: (${M}x${N}) * (${N}x${P}) = (${M}x${P})`);
  console.log(`💾 Memory usage: ${((M*N + N*P + M*P) * 4) / (1024*1024)} MB`);

  const multiplyResult = matrix.multiply(matrixA, matrixB, matrixResult, M, N, P);
  if (multiplyResult.success) {
    console.log('✅ Matrix multiplication completed');
    console.log(`📈 Sample result[0][0]: ${matrixResult[0].toFixed(6)}`);
  } else {
    console.error('❌ Matrix multiplication failed:', multiplyResult.error);
  }

  console.log();

  // Example 4: Performance Comparison
  console.log('⚡ Example 4: Performance Demonstration');
  console.log('--------------------------------------');

  const perfSize = 10000;
  const perfA = new Float32Array(perfSize);
  const perfB = new Float32Array(perfSize);
  const perfResult = new Float32Array(perfSize);

  // Initialize test data
  for (let i = 0; i < perfSize; i++) {
    perfA[i] = Math.sin(i * 0.001);
    perfB[i] = Math.cos(i * 0.001);
  }

  console.log(`🏃 Running performance test with ${perfSize} elements...`);

  const iterations = 100;
  const startTime = process.hrtime.bigint();

  for (let iter = 0; iter < iterations; iter++) {
    const perfTest = matrix.add(perfA, perfB, perfResult, perfSize, 1);
    if (!perfTest.success) {
      console.error('Performance test failed:', perfTest.error);
      break;
    }
  }

  const endTime = process.hrtime.bigint();
  const totalTimeMs = Number(endTime - startTime) / 1_000_000;
  const avgTimePerOp = totalTimeMs / iterations;

  console.log(`⏱️  Total time: ${totalTimeMs.toFixed(2)} ms`);
  console.log(`📊 Average time per operation: ${avgTimePerOp.toFixed(4)} ms`);
  console.log(`🚀 Operations per second: ${(1000 / avgTimePerOp).toLocaleString()}`);
  console.log(`💾 Data processed: ${(perfSize * 4 * 3 * iterations) / (1024 * 1024)} MB`);

  console.log();
  console.log('🎉 UMICP TypeScript Bindings Example Completed!');
  console.log('===============================================');
  console.log();
  console.log('💡 Key Benefits:');
  console.log('   • Native C++ performance with SIMD acceleration');
  console.log('   • Zero-copy operations for large matrices');
  console.log('   • Memory-efficient envelope serialization');
  console.log('   • Type-safe TypeScript bindings');
  console.log('   • Seamless Node.js integration');
}

// Run example if executed directly
if (require.main === module) {
  basicUMICPExample().catch(console.error);
}

export { basicUMICPExample };
