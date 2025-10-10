/**
 * Example 4: Request-Response Pattern
 *
 * This example demonstrates:
 * - Sending a request and waiting for response
 * - Timeout handling
 * - Correlation IDs
 * - Error responses
 */

import {
  UMICPWebSocketPeer,
  Envelope,
  OperationType
} from '../src/index.js';

async function main() {
  console.log('🔄 UMICP Example 4: Request-Response Pattern\n');

  // Create server peer
  const server = new UMICPWebSocketPeer({
    peerId: 'api-server',
    server: {
      port: 8094,
      path: '/umicp'
    }
  });

  // Setup request handler
  server.on('data', async (envelope, peer) => {
    const action = envelope.getCapabilities()['action'];
    console.log(`\n📨 Server received request: ${action}`);
    console.log(`   Request ID: ${envelope.getMessageId()}`);

    // Simulate processing
    await new Promise(resolve => setTimeout(resolve, 500));

    let response: Envelope;

    // Handle different actions
    switch (action) {
      case 'get-user':
        response = new Envelope({
          from: 'api-server',
          to: envelope.getFrom(),
          operation: OperationType.DATA,
          messageId: `response-${envelope.getMessageId()}`,
          capabilities: {
            reply_to: envelope.getMessageId(),
            status: 'success',
            user: JSON.stringify({
              id: '12345',
              name: 'John Doe',
              email: 'john@example.com'
            })
          }
        });
        break;

      case 'calculate':
        const num1 = parseInt(envelope.getCapabilities()['num1']);
        const num2 = parseInt(envelope.getCapabilities()['num2']);
        const result = num1 + num2;

        response = new Envelope({
          from: 'api-server',
          to: envelope.getFrom(),
          operation: OperationType.DATA,
          messageId: `response-${envelope.getMessageId()}`,
          capabilities: {
            reply_to: envelope.getMessageId(),
            status: 'success',
            result: result.toString()
          }
        });
        break;

      case 'error-test':
        response = new Envelope({
          from: 'api-server',
          to: envelope.getFrom(),
          operation: OperationType.ERROR,
          messageId: `error-${envelope.getMessageId()}`,
          capabilities: {
            reply_to: envelope.getMessageId(),
            error: 'Something went wrong!',
            code: '500'
          }
        });
        break;

      default:
        response = new Envelope({
          from: 'api-server',
          to: envelope.getFrom(),
          operation: OperationType.ERROR,
          messageId: `error-${envelope.getMessageId()}`,
          capabilities: {
            reply_to: envelope.getMessageId(),
            error: 'Unknown action',
            code: '400'
          }
        });
    }

    server.sendToPeer(peer.id, response);
    console.log(`   ✅ Sent response`);
  });

  await new Promise<void>(resolve => {
    server.on('server:ready', () => {
      console.log('✅ API Server ready\n');
      resolve();
    });
  });

  // Create client peer
  const client = new UMICPWebSocketPeer({
    peerId: 'api-client'
  });

  await client.connectToPeer('ws://localhost:8094/umicp');
  await new Promise(resolve => setTimeout(resolve, 1000));

  console.log('✅ Client connected\n');

  const peers = client.getPeers();
  if (peers.length === 0) {
    console.error('No peers connected');
    return;
  }

  // Example 1: Get user data
  console.log('📤 Request 1: Get user data');
  try {
    const getUserRequest = new Envelope({
      from: 'api-client',
      to: 'api-server',
      operation: OperationType.DATA,
      messageId: `req-user-${Date.now()}`,
      capabilities: {
        action: 'get-user',
        userId: '12345'
      }
    });

    const userResponse = await client.sendAndWait(peers[0].id, getUserRequest, 5000);
    console.log('📬 Response received:');
    console.log(`   Status: ${userResponse.getCapabilities()['status']}`);
    const userData = JSON.parse(userResponse.getCapabilities()['user']);
    console.log(`   User: ${userData.name} (${userData.email})`);
  } catch (error) {
    console.error('❌ Request failed:', error);
  }

  await new Promise(resolve => setTimeout(resolve, 1000));

  // Example 2: Calculate
  console.log('\n📤 Request 2: Calculate 15 + 27');
  try {
    const calcRequest = new Envelope({
      from: 'api-client',
      to: 'api-server',
      operation: OperationType.DATA,
      messageId: `req-calc-${Date.now()}`,
      capabilities: {
        action: 'calculate',
        num1: '15',
        num2: '27'
      }
    });

    const calcResponse = await client.sendAndWait(peers[0].id, calcRequest, 5000);
    console.log('📬 Response received:');
    console.log(`   Result: ${calcResponse.getCapabilities()['result']}`);
  } catch (error) {
    console.error('❌ Request failed:', error);
  }

  await new Promise(resolve => setTimeout(resolve, 1000));

  // Example 3: Error handling
  console.log('\n📤 Request 3: Trigger error');
  try {
    const errorRequest = new Envelope({
      from: 'api-client',
      to: 'api-server',
      operation: OperationType.DATA,
      messageId: `req-error-${Date.now()}`,
      capabilities: {
        action: 'error-test'
      }
    });

    const errorResponse = await client.sendAndWait(peers[0].id, errorRequest, 5000);
    if (errorResponse.getOperation() === OperationType.ERROR) {
      console.log('📬 Error response received:');
      console.log(`   Error: ${errorResponse.getCapabilities()['error']}`);
      console.log(`   Code: ${errorResponse.getCapabilities()['code']}`);
    }
  } catch (error) {
    console.error('❌ Request failed:', error);
  }

  await new Promise(resolve => setTimeout(resolve, 1000));

  // Example 4: Timeout handling
  console.log('\n📤 Request 4: Test timeout (will timeout in 1 second)');
  try {
    const timeoutRequest = new Envelope({
      from: 'api-client',
      to: 'api-server',
      operation: OperationType.DATA,
      messageId: `req-timeout-${Date.now()}`,
      capabilities: {
        action: 'never-respond' // Server won't respond to this
      }
    });

    // Set very short timeout
    await client.sendAndWait(peers[0].id, timeoutRequest, 1000);
    console.log('📬 Response received (unexpected)');
  } catch (error) {
    console.log('⏱️  Request timed out (as expected)');
  }

  await new Promise(resolve => setTimeout(resolve, 1000));

  // Example 5: Multiple concurrent requests
  console.log('\n📤 Request 5: Multiple concurrent requests');
  const concurrentRequests = [];

  for (let i = 1; i <= 5; i++) {
    const request = new Envelope({
      from: 'api-client',
      to: 'api-server',
      operation: OperationType.DATA,
      messageId: `req-concurrent-${i}-${Date.now()}`,
      capabilities: {
        action: 'calculate',
        num1: i.toString(),
        num2: (i * 2).toString()
      }
    });

    concurrentRequests.push(
      client.sendAndWait(peers[0].id, request, 5000)
        .then(response => ({
          success: true,
          result: response.getCapabilities()['result']
        }))
        .catch(error => ({
          success: false,
          error: error.message
        }))
    );
  }

  const results = await Promise.all(concurrentRequests);
  console.log('📬 All responses received:');
  results.forEach((result, idx) => {
    if (result.success) {
      console.log(`   Request ${idx + 1}: Result = ${result.result}`);
    } else {
      console.log(`   Request ${idx + 1}: Failed - ${result.error}`);
    }
  });

  // Cleanup
  console.log('\n🧹 Cleaning up...');
  await client.shutdown();
  await server.shutdown();

  console.log('✅ Request-Response example completed!\n');
}

// Run example
main().catch(console.error);

