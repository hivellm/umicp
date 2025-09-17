/**
 * Integration tests for UMICP envelope and transport functionality
 */

import { Envelope, UMICP, OperationType } from '../src/index';
import { AdvancedWebSocketTransport } from '../examples/transport-example';

describe('UMICP Integration Tests', () => {
  let server: AdvancedWebSocketTransport;
  let client: AdvancedWebSocketTransport;

  beforeEach(() => {
    server = new AdvancedWebSocketTransport({
      url: 'ws://localhost:9000',
      isServer: true,
      port: 9000,
      heartbeatInterval: 2000
    });

    client = new AdvancedWebSocketTransport({
      url: 'ws://localhost:9000',
      isServer: false,
      maxReconnectAttempts: 1
    });
  });

  afterEach(async () => {
    try {
      client.disconnect();
      server.disconnect();
      await new Promise(resolve => setTimeout(resolve, 100));
    } catch (error) {
      // Ignore cleanup errors
    }
  });

  test('should send UMICP envelope over WebSocket', async () => {
    let envelopeReceived: any = null;
    let senderId = '';

    // Create UMICP envelope
    const originalEnvelope = UMICP.createEnvelope({
      from: 'test-client',
      to: 'test-server',
      operation: OperationType.DATA,
      messageId: 'integration-test-001',
      capabilities: {
        transport: 'websocket',
        format: 'json'
      }
    });

    // Start server
    const serverConnected = await server.connect();
    expect(serverConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 100));

    // Set up server message handler
    server.on('message', (message: any, info: any) => {
      if (typeof message === 'string') {
        try {
          const parsed = JSON.parse(message);
          if (parsed.v && parsed.from && parsed.to) {
            envelopeReceived = parsed;
            senderId = info?.id || '';
          }
        } catch (error) {
          // Not a JSON message
        }
      }
    });

    // Connect client
    const clientConnected = await client.connect();
    expect(clientConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Send envelope
    const envelopeJson = originalEnvelope.serialize();
    const sent = client.send(envelopeJson);
    expect(sent).toBe(true);

    // Wait for message delivery
    await new Promise(resolve => setTimeout(resolve, 200));

    // Verify envelope was received and parsed correctly
    expect(envelopeReceived).toBeTruthy();
    expect(envelopeReceived.from).toBe('test-client');
    expect(envelopeReceived.to).toBe('test-server');
    expect(envelopeReceived.msg_id).toBe('integration-test-001');
    expect(envelopeReceived.op).toBe(OperationType.DATA);
    expect(senderId).toBeTruthy();
  });

  test('should handle request-response pattern', async () => {
    let requestReceived = false;
    let responseSent = false;
    let responseReceived = false;

    const requestId = 'req_' + Date.now();

    // Start server
    const serverConnected = await server.connect();
    expect(serverConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 100));

    // Server handles requests
    server.on('message', async (message: any, info: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          if (envelope.op === OperationType.DATA && envelope.to === 'test-server') {
            requestReceived = true;

            // Create response envelope
            const responseEnvelope = UMICP.createEnvelope({
              from: 'test-server',
              to: envelope.from,
              operation: OperationType.ACK,
              messageId: 'ack_' + envelope.msg_id,
              capabilities: {
                processed: 'true',
                response_to: envelope.msg_id
              }
            });

            const responseJson = responseEnvelope.serialize();
            server.send(responseJson, info?.id);
            responseSent = true;
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    // Client handles responses
    client.on('message', (message: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          if (envelope.op === OperationType.ACK && envelope.to === 'test-client') {
            responseReceived = true;
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    // Connect client
    const clientConnected = await client.connect();
    expect(clientConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Send request
    const requestEnvelope = UMICP.createEnvelope({
      from: 'test-client',
      to: 'test-server',
      operation: OperationType.DATA,
      messageId: requestId,
      capabilities: {
        type: 'request',
        method: 'get_status'
      }
    });

    const requestJson = requestEnvelope.serialize();
    const sent = client.send(requestJson);
    expect(sent).toBe(true);

    // Wait for request-response cycle
    await new Promise(resolve => setTimeout(resolve, 500));

    expect(requestReceived).toBe(true);
    expect(responseSent).toBe(true);
    expect(responseReceived).toBe(true);
  });

  test('should handle multiple envelope types', async () => {
    const receivedEnvelopes: any[] = [];

    // Start server
    const serverConnected = await server.connect();
    expect(serverConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 100));

    server.on('message', (message) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          if (envelope.v && envelope.from) {
            receivedEnvelopes.push(envelope);
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    // Connect client
    const clientConnected = await client.connect();
    expect(clientConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Send different types of envelopes
    const envelopeTypes = [
      { operation: OperationType.CONTROL, type: 'control' },
      { operation: OperationType.DATA, type: 'data' },
      { operation: OperationType.ACK, type: 'ack' },
      { operation: OperationType.ERROR, type: 'error' }
    ];

    for (const envelopeType of envelopeTypes) {
      const envelope = UMICP.createEnvelope({
        from: 'test-client',
        to: 'test-server',
        operation: envelopeType.operation,
        messageId: `${envelopeType.type}_${Date.now()}`,
        capabilities: {
          envelope_type: envelopeType.type
        }
      });

      const json = envelope.serialize();
      client.send(json);
    }

    // Wait for all envelopes
    await new Promise(resolve => setTimeout(resolve, 300));

    expect(receivedEnvelopes.length).toBe(envelopeTypes.length);

    // Verify all operation types were received
    const receivedOps = receivedEnvelopes.map(env => env.op);
    expect(receivedOps).toContain(OperationType.CONTROL);
    expect(receivedOps).toContain(OperationType.DATA);
    expect(receivedOps).toContain(OperationType.ACK);
    expect(receivedOps).toContain(OperationType.ERROR);
  });

  test('should handle envelope serialization roundtrip', async () => {
    let receivedEnvelope: any = null;

    // Create complex envelope
    const originalEnvelope = UMICP.createEnvelope({
      from: 'test-sender',
      to: 'test-receiver',
      operation: OperationType.DATA,
      messageId: 'roundtrip-test',
      capabilities: {
        compression: 'gzip',
        encryption: 'aes256',
        format: 'json'
      }
    });

    // Start server
    const serverConnected = await server.connect();
    expect(serverConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 100));

    server.on('message', (message) => {
      if (typeof message === 'string') {
        try {
          receivedEnvelope = JSON.parse(message);
        } catch (error) {
          // Not JSON
        }
      }
    });

    // Connect client
    const clientConnected = await client.connect();
    expect(clientConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Send envelope
    const json = originalEnvelope.serialize();
    client.send(json);

    // Wait for delivery
    await new Promise(resolve => setTimeout(resolve, 200));

    // Verify received envelope matches original
    expect(receivedEnvelope).toBeTruthy();
    expect(receivedEnvelope.from).toBe('test-sender');
    expect(receivedEnvelope.to).toBe('test-receiver');
    expect(receivedEnvelope.msg_id).toBe('roundtrip-test');
    expect(receivedEnvelope.op).toBe(OperationType.DATA);
    expect(receivedEnvelope.capabilities.compression).toBe('gzip');
    expect(receivedEnvelope.capabilities.encryption).toBe('aes256');
  });

  test('should handle concurrent envelope processing', async () => {
    const receivedEnvelopes: any[] = [];
    const numEnvelopes = 10;

    // Start server
    const serverConnected = await server.connect();
    expect(serverConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 100));

    server.on('message', (message) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          if (envelope.v && envelope.from) {
            receivedEnvelopes.push(envelope);
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    // Connect client
    const clientConnected = await client.connect();
    expect(clientConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Send multiple envelopes concurrently
    const sendPromises = [];
    for (let i = 0; i < numEnvelopes; i++) {
      const envelope = UMICP.createEnvelope({
        from: 'test-client',
        to: 'test-server',
        operation: OperationType.DATA,
        messageId: `concurrent_${i}`,
        capabilities: {
          sequence: i.toString(),
          timestamp: Date.now().toString()
        }
      });

      const json = envelope.serialize();
      sendPromises.push(client.send(json));
    }

    // Wait for all sends to complete
    const sendResults = await Promise.all(sendPromises);
    sendResults.forEach((result: any) => expect(result).toBe(true));

    // Wait for all envelopes to be received
    await new Promise(resolve => setTimeout(resolve, 500));

    expect(receivedEnvelopes.length).toBe(numEnvelopes);

    // Verify all envelopes were received
    const messageIds = receivedEnvelopes.map(env => env.msg_id);
    for (let i = 0; i < numEnvelopes; i++) {
      expect(messageIds).toContain(`concurrent_${i}`);
    }
  });

  test('should handle large envelope payloads', async () => {
    let largeEnvelopeReceived = false;
    let receivedSize = 0;

    // Create envelope with large capabilities object
    const largeCapabilities: Record<string, string> = {};
    for (let i = 0; i < 1000; i++) {
      largeCapabilities[`key_${i}`] = `value_${i}_`.repeat(10); // ~100 chars per value
    }

    const largeEnvelope = UMICP.createEnvelope({
      from: 'test-client',
      to: 'test-server',
      operation: OperationType.DATA,
      messageId: 'large-envelope-test',
      capabilities: largeCapabilities
    });

    // Start server
    const serverConnected = await server.connect();
    expect(serverConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 100));

    server.on('message', (message: any) => {
      if (typeof message === 'string') {
        receivedSize = message.length;
        largeEnvelopeReceived = true;
      }
    });

    // Connect client
    const clientConnected = await client.connect();
    expect(clientConnected).toBe(true);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Send large envelope
    const json = largeEnvelope.serialize();
    expect(json.length).toBeGreaterThan(100000); // Should be > 100KB

    const sent = client.send(json);
    expect(sent).toBe(true);

    // Wait for delivery
    await new Promise(resolve => setTimeout(resolve, 500));

    expect(largeEnvelopeReceived).toBe(true);
    expect(receivedSize).toBeGreaterThan(100000);
  });
});
