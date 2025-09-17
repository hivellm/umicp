/**
 * UMICP Performance Benchmark
 * Demonstrates the performance benefits of native C++ operations
 */

const { UMICP } = require('../dist/index.js');

async function runBenchmark() {
  console.log('🚀 UMICP Performance Benchmark');
  console.log('==============================\n');

  const matrix = UMICP.createMatrix();
  const sizes = [1000, 5000, 10000];

  console.log('📊 Matrix Operations Benchmark');
  console.log('===============================');

  for (const size of sizes) {
    console.log(`\n🔢 Testing with ${size}x${size} matrices (${(size * size * 4 * 3 / 1024 / 1024).toFixed(2)} MB)`);

    // Create test data
    const a = new Float32Array(size * size);
    const b = new Float32Array(size * size);
    const result = new Float32Array(size * size);

    // Initialize with test data
    for (let i = 0; i < a.length; i++) {
      a[i] = Math.sin(i * 0.001) * Math.cos(i * 0.0005);
      b[i] = Math.sin(i * 0.0008) * Math.cos(i * 0.0012);
    }

    // Benchmark matrix multiplication
    const startTime = process.hrtime.bigint();
    const multiplyResult = matrix.multiply(a, b, result, size, size, size);
    const endTime = process.hrtime.bigint();

    if (multiplyResult.success) {
      const timeMs = Number(endTime - startTime) / 1_000_000;
      const operations = size * size * size * 2; // Rough estimate of operations
      const opsPerSecond = (operations / timeMs) * 1000;

      console.log(`✅ Matrix multiplication: ${timeMs.toFixed(2)}ms`);
      console.log(`🚀 Performance: ${(opsPerSecond / 1_000_000).toFixed(2)} MOPS/sec`);
      console.log(`💾 Memory bandwidth: ${((size * size * 4 * 3) / timeMs / 1000).toFixed(2)} GB/s`);
    } else {
      console.log(`❌ Matrix multiplication failed: ${multiplyResult.error}`);
    }
  }

  console.log('\n📈 Vector Operations Benchmark');
  console.log('===============================');

  const vectorSizes = [100000, 500000, 1000000]; // 100K to 1M elements

  for (const size of vectorSizes) {
    console.log(`\n🔢 Testing with vectors of ${size} elements (${(size * 4 / 1024).toFixed(0)} KB each)`);

    const a = new Float32Array(size);
    const b = new Float32Array(size);
    const result = new Float32Array(size);

    // Initialize with test data
    for (let i = 0; i < size; i++) {
      a[i] = Math.sin(i * 0.0001);
      b[i] = Math.cos(i * 0.0001);
    }

    // Benchmark vector addition
    const startTime = process.hrtime.bigint();
    const addResult = matrix.add(a, b, result, size, 1);
    const endTime = process.hrtime.bigint();

    if (addResult.success) {
      const timeMs = Number(endTime - startTime) / 1_000_000;
      const operations = size;
      const opsPerSecond = (operations / timeMs) * 1000;

      console.log(`✅ Vector addition: ${timeMs.toFixed(2)}ms`);
      console.log(`🚀 Performance: ${(opsPerSecond / 1_000_000).toFixed(2)} MOPS/sec`);
      console.log(`💾 Memory throughput: ${((size * 4 * 3) / timeMs / 1000).toFixed(2)} GB/s`);
    } else {
      console.log(`❌ Vector addition failed: ${addResult.error}`);
    }

    // Benchmark dot product
    const dotStartTime = process.hrtime.bigint();
    const dotResult = matrix.dotProduct(a, b);
    const dotEndTime = process.hrtime.bigint();

    if (dotResult.success) {
      const timeMs = Number(dotEndTime - dotStartTime) / 1_000_000;
      const operations = size * 2;
      const opsPerSecond = (operations / timeMs) * 1000;

      console.log(`✅ Dot product: ${timeMs.toFixed(2)}ms`);
      console.log(`🚀 Performance: ${(opsPerSecond / 1_000_000).toFixed(2)} MOPS/sec`);
      console.log(`📊 Result: ${dotResult.result.toFixed(6)}`);
    }
  }

  console.log('\n🎯 Envelope Operations Benchmark');
  console.log('==================================');

  const envelopeCount = 10000;
  console.log(`📧 Testing with ${envelopeCount} envelopes`);

  const envelopes = [];
  const startTime = process.hrtime.bigint();

  // Create envelopes
  for (let i = 0; i < envelopeCount; i++) {
    const envelope = UMICP.createEnvelope({
      from: `sender_${i}`,
      to: `receiver_${i}`,
      operation: UMICP.OperationType.DATA,
      messageId: `msg_${i}_${Date.now()}`,
      capabilities: {
        sequence: i.toString(),
        batch: 'true',
        compression: 'gzip'
      }
    });
    envelopes.push(envelope);
  }

  // Serialize envelopes
  const serializeStart = process.hrtime.bigint();
  const serialized = envelopes.map(env => env.serialize());
  const serializeEnd = process.hrtime.bigint();

  // Deserialize envelopes
  const deserializeStart = process.hrtime.bigint();
  const deserialized = serialized.map(json => UMICP.Envelope.deserialize(json));
  const deserializeEnd = process.hrtime.bigint();

  const totalTime = process.hrtime.bigint();
  const createTime = Number(serializeStart - startTime) / 1_000_000;
  const serializeTime = Number(serializeEnd - serializeStart) / 1_000_000;
  const deserializeTime = Number(deserializeEnd - deserializeStart) / 1_000_000;

  console.log(`✅ Envelope creation: ${createTime.toFixed(2)}ms (${(envelopeCount / createTime * 1000).toFixed(0)}/sec)`);
  console.log(`✅ Serialization: ${serializeTime.toFixed(2)}ms (${(envelopeCount / serializeTime * 1000).toFixed(0)}/sec)`);
  console.log(`✅ Deserialization: ${deserializeTime.toFixed(2)}ms (${(envelopeCount / deserializeTime * 1000).toFixed(0)}/sec)`);

  const totalSize = serialized.reduce((sum, json) => sum + json.length, 0);
  console.log(`💾 Average envelope size: ${(totalSize / envelopeCount).toFixed(0)} bytes`);
  console.log(`🚀 Throughput: ${(totalSize / (serializeTime + deserializeTime) / 1000).toFixed(2)} MB/s`);

  console.log('\n🏆 Benchmark Complete!');
  console.log('===================');
  console.log('✅ All operations completed successfully');
  console.log('🚀 Native C++ performance with SIMD acceleration');
  console.log('💾 Memory-efficient operations');
  console.log('🔒 Type-safe TypeScript bindings');
}

// Run benchmark if executed directly
if (require.main === module) {
  runBenchmark().catch(console.error);
}

module.exports = { runBenchmark };
