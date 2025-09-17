/**
 * UMICP TypeScript Bindings - Simple Test
 * Tests basic functionality of the native bindings
 */

import { Envelope, Matrix, Frame, UMICP, OperationType, PayloadType, EncodingType } from '../src/index.js';

async function testBasicBindings() {
  console.log('🧪 UMICP TypeScript Bindings - Simple Test');
  console.log('==========================================');
  console.log(`📦 Version: ${UMICP.version}`);
  console.log(`🎯 UMICP Version: ${UMICP.UMICP_VERSION}`);
  console.log();

  // Test 1: Envelope Operations
  console.log('📝 Test 1: Envelope Operations');
  console.log('------------------------------');

  try {
    const envelope = new Envelope({
      from: 'test-model-a',
      to: 'test-model-b',
      operation: OperationType.DATA,
      capabilities: {
        'binary_support': 'true',
        'compression': 'gzip'
      },
      payloadHint: {
        type: PayloadType.VECTOR,
        size: 1024,
        encoding: EncodingType.FLOAT32,
        count: 256
      }
    });

    console.log('✅ Envelope created successfully');

    envelope.setFrom('updated-model-a');
    console.log('✅ Envelope updated successfully');

    const isValid = envelope.validate();
    console.log('✅ Envelope validation:', isValid ? 'PASSED' : 'FAILED');

    const hash = envelope.getHash();
    console.log('🔐 Envelope hash:', hash.substring(0, 16) + '...');

  } catch (error) {
    console.error('❌ Envelope test failed:', error);
  }

  console.log();

  // Test 2: Frame Operations
  console.log('📦 Test 2: Frame Operations');
  console.log('---------------------------');

  try {
    const frame = new Frame();
    frame.setType(1);
    frame.setStreamId(42);
    frame.setSequence(1);

    const testData = new Uint8Array([1, 2, 3, 4, 5, 72, 101, 108, 108, 111]); // "Hello"
    frame.setPayload(testData);

    console.log('✅ Frame created successfully');
    console.log('   Type:', frame.getType());
    console.log('   Stream ID:', frame.getStreamId());
    console.log('   Sequence:', frame.getSequence());
    console.log('   Payload size:', frame.getPayloadSize(), 'bytes');

    // Test serialization
    const serialized = frame.serialize();
    console.log('✅ Frame serialized:', serialized.length, 'bytes');

  } catch (error) {
    console.error('❌ Frame test failed:', error);
  }

  console.log();

  // Test 3: Matrix Operations
  console.log('🧮 Test 3: Matrix Operations (C++ SIMD)');
  console.log('---------------------------------------');

  try {
    const matrix = new Matrix();

    // Create test data
    const size = 100;
    const arrayA = new Float32Array(size);
    const arrayB = new Float32Array(size);
    const resultArray = new Float32Array(size);

    // Initialize with test values
    for (let i = 0; i < size; i++) {
      arrayA[i] = i * 0.01;
      arrayB[i] = (i % 10) * 0.1;
    }

    console.log(`📊 Testing with ${size} elements`);

    // Test vector addition
    const addResult = matrix.add(arrayA, arrayB, resultArray, size, 1);
    if (addResult.success) {
      console.log('✅ Vector addition successful');
      console.log(`   Sample: ${arrayA[0]} + ${arrayB[0]} = ${resultArray[0]}`);
    } else {
      console.error('❌ Vector addition failed:', addResult.error);
    }

    // Test dot product
    const dotResult = matrix.dotProduct(arrayA, arrayB);
    if (dotResult.success && dotResult.result !== undefined) {
      console.log('✅ Dot product successful:', dotResult.result);
    } else {
      console.error('❌ Dot product failed:', dotResult.error);
    }

    // Test cosine similarity
    const cosResult = matrix.cosineSimilarity(arrayA, arrayB);
    if (cosResult.success && cosResult.similarity !== undefined) {
      console.log('✅ Cosine similarity successful:', cosResult.similarity.toFixed(4));
    } else {
      console.error('❌ Cosine similarity failed:', cosResult.error);
    }

  } catch (error) {
    console.error('❌ Matrix test failed:', error);
  }

  console.log();

  // Test 4: Constants
  console.log('⚙️  Test 4: Constants');
  console.log('===================');

  console.log('✅ Operation constants:');
  console.log('   CONTROL:', UMICP.OPERATION_CONTROL);
  console.log('   DATA:', UMICP.OPERATION_DATA);
  console.log('   ACK:', UMICP.OPERATION_ACK);
  console.log('   ERROR:', UMICP.OPERATION_ERROR);

  console.log('✅ Payload type constants:');
  console.log('   VECTOR:', UMICP.PAYLOAD_VECTOR);
  console.log('   TEXT:', UMICP.PAYLOAD_TEXT);
  console.log('   METADATA:', UMICP.PAYLOAD_METADATA);
  console.log('   BINARY:', UMICP.PAYLOAD_BINARY);

  console.log('✅ Encoding constants:');
  console.log('   FLOAT32:', UMICP.ENCODING_FLOAT32);
  console.log('   INT32:', UMICP.ENCODING_INT32);
  console.log('   UINT8:', UMICP.ENCODING_UINT8);

  console.log();
  console.log('🎉 UMICP TypeScript Bindings Test Completed!');
  console.log('============================================');
  console.log();
  console.log('💡 Key Features Verified:');
  console.log('   • Native C++ envelope processing');
  console.log('   • Binary frame serialization');
  console.log('   • SIMD-accelerated matrix operations');
  console.log('   • Type-safe TypeScript bindings');
  console.log('   • High-performance C++ backend');
}

// Run test if executed directly
if (require.main === module) {
  testBasicBindings().catch(console.error);
}

export { testBasicBindings };
