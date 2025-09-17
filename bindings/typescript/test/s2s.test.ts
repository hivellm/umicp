/**
 * S2S (Server-to-Server) Integration Tests
 * Tests direct communication between UMICP servers simulating federated AI architecture
 */

import { AdvancedWebSocketTransport } from '../examples/transport-example';
import { Envelope, UMICP, OperationType } from '../src/index';

describe('S2S (Server-to-Server) Communication', () => {
  let serverA: AdvancedWebSocketTransport;
  let serverB: AdvancedWebSocketTransport;
  let clientA: AdvancedWebSocketTransport;
  let clientB: AdvancedWebSocketTransport;

  beforeEach(() => {
    // Server A (AI Model Alpha)
    serverA = new AdvancedWebSocketTransport({
      url: 'ws://localhost:9001',
      isServer: true,
      port: 9001,
      path: '/umicp/alpha'
    });

    // Server B (AI Model Beta)
    serverB = new AdvancedWebSocketTransport({
      url: 'ws://localhost:9002',
      isServer: true,
      port: 9002,
      path: '/umicp/beta'
    });

    // Client for Server A
    clientA = new AdvancedWebSocketTransport({
      url: 'ws://localhost:9001/umicp/alpha',
      isServer: false,
      maxReconnectAttempts: 2
    });

    // Client for Server B
    clientB = new AdvancedWebSocketTransport({
      url: 'ws://localhost:9002/umicp/beta',
      isServer: false,
      maxReconnectAttempts: 2
    });
  });

  afterEach(async () => {
    try {
      await Promise.all([
        clientA.disconnect(),
        clientB.disconnect(),
        serverA.disconnect(),
        serverB.disconnect()
      ]);
    } catch (error) {
      // Ignore cleanup errors
    }
  });

  test('should establish S2S communication channels', async () => {
    let serverAStarted = false;
    let serverBStarted = false;
    let clientAConnected = false;
    let clientBConnected = false;

    // Set up event handlers
    serverA.on('serverStarted', (port: number) => {
      expect(port).toBe(9001);
      serverAStarted = true;
    });

    serverB.on('serverStarted', (port: number) => {
      expect(port).toBe(9002);
      serverBStarted = true;
    });

    clientA.on('connected', () => {
      clientAConnected = true;
    });

    clientB.on('connected', () => {
      clientBConnected = true;
    });

    // Start both servers
    const [serverAStartedResult, serverBStartedResult] = await Promise.all([
      serverA.connect(),
      serverB.connect()
    ]);

    expect(serverAStartedResult).toBe(true);
    expect(serverBStartedResult).toBe(true);

    // Wait for servers to start
    await new Promise(resolve => setTimeout(resolve, 200));
    expect(serverAStarted).toBe(true);
    expect(serverBStarted).toBe(true);

    // Connect clients to respective servers
    const [clientAConnectedResult, clientBConnectedResult] = await Promise.all([
      clientA.connect(),
      clientB.connect()
    ]);

    expect(clientAConnectedResult).toBe(true);
    expect(clientBConnectedResult).toBe(true);

    // Wait for connections
    await new Promise(resolve => setTimeout(resolve, 200));
    expect(clientAConnected).toBe(true);
    expect(clientBConnected).toBe(true);
  });

  test('should enable cross-server model collaboration', async () => {
    let alphaProcessedRequest = false;
    let betaProcessedRequest = false;
    let collaborationCompleted = false;

    // Start both servers
    await Promise.all([serverA.connect(), serverB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Connect clients
    await Promise.all([clientA.connect(), clientB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Set up collaboration handlers
    serverA.on('message', async (message: any, info: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          if (envelope.op === OperationType.DATA && envelope.to === 'ai-alpha') {
            alphaProcessedRequest = true;
            console.log('Alpha received collaboration request:', envelope.msg_id);

            // Alpha processes the request and forwards to Beta
            const betaEnvelope = UMICP.createEnvelope({
              from: 'ai-alpha',
              to: 'ai-beta',
              operation: OperationType.DATA,
              messageId: `alpha_${envelope.msg_id}`,
              capabilities: {
                collaboration: 'true',
                task: envelope.capabilities?.task,
                stage: 'processing'
              }
            });

            // Send to Beta via its client
            const betaJson = betaEnvelope.serialize();
            clientB.send(betaJson);
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    serverB.on('message', async (message: any, info: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          if (envelope.op === OperationType.DATA && envelope.to === 'ai-beta') {
            betaProcessedRequest = true;
            console.log('Beta received collaboration request:', envelope.msg_id);

            // Beta completes the collaboration
            const responseEnvelope = UMICP.createEnvelope({
              from: 'ai-beta',
              to: 'ai-alpha',
              operation: OperationType.ACK,
              messageId: `beta_response_${envelope.msg_id}`,
              capabilities: {
                collaboration: 'completed',
                result: 'success',
                confidence: '0.95'
              }
            });

            // Send response back to Alpha
            const responseJson = responseEnvelope.serialize();
            clientA.send(responseJson);
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    clientA.on('message', (message: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          if (envelope.op === OperationType.ACK && envelope.from === 'ai-beta') {
            collaborationCompleted = true;
            console.log('Alpha received collaboration response:', envelope.msg_id);
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    // Start collaboration from client to Alpha
    const initialEnvelope = UMICP.createEnvelope({
      from: 'client',
      to: 'ai-alpha',
      operation: OperationType.DATA,
      messageId: 'collab_request_001',
      capabilities: {
        task: 'sentiment_analysis',
        data: 'Sample text for collaborative processing',
        collaboration_mode: 's2s'
      }
    });

    const initialJson = initialEnvelope.serialize();
    clientA.send(initialJson);

    // Wait for the complete collaboration cycle
    await new Promise(resolve => setTimeout(resolve, 1000));

    expect(alphaProcessedRequest).toBe(true);
    expect(betaProcessedRequest).toBe(true);
    expect(collaborationCompleted).toBe(true);
  });

  test('should handle federated learning data exchange', async () => {
    const receivedGradients: any[] = [];
    let modelUpdated = false;

    // Start servers
    await Promise.all([serverA.connect(), serverB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Connect clients
    await Promise.all([clientA.connect(), clientB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Set up federated learning handlers
    serverA.on('message', async (message: any, info: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          if (envelope.capabilities?.federated_learning === 'gradient_update') {
            receivedGradients.push({
              from: envelope.from,
              gradient: envelope.capabilities.gradient_data,
              round: envelope.capabilities.round
            });

            // Aggregate gradients and update global model
            if (receivedGradients.length >= 2) {
              const aggregatedGradient = receivedGradients.reduce((acc, curr) => {
                return acc + parseFloat(curr.gradient);
              }, 0) / receivedGradients.length;

              // Send updated global model
              const updateEnvelope = UMICP.createEnvelope({
                from: 'federated-coordinator',
                to: 'all-models',
                operation: OperationType.DATA,
                messageId: 'global_model_update',
                capabilities: {
                  federated_learning: 'model_update',
                  global_model: aggregatedGradient.toString(),
                  round: envelope.capabilities.round
                }
              });

              const updateJson = updateEnvelope.serialize();
              serverA.broadcast(updateJson);
              serverB.broadcast(updateJson);
            }
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    serverB.on('message', (message: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          if (envelope.capabilities?.federated_learning === 'model_update') {
            modelUpdated = true;
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    // Simulate gradient updates from multiple models
    const gradientUpdates = [
      { from: 'model-1', gradient: '0.001', round: '1' },
      { from: 'model-2', gradient: '0.002', round: '1' }
    ];

    for (const update of gradientUpdates) {
      const envelope = UMICP.createEnvelope({
        from: update.from,
        to: 'federated-coordinator',
        operation: OperationType.DATA,
        messageId: `gradient_${update.from}_${Date.now()}`,
        capabilities: {
          federated_learning: 'gradient_update',
          gradient_data: update.gradient,
          round: update.round,
          model_version: 'v1.0'
        }
      });

      const json = envelope.serialize();
      clientA.send(json);
    }

    // Wait for federated learning cycle
    await new Promise(resolve => setTimeout(resolve, 800));

    expect(receivedGradients.length).toBe(2);
    expect(modelUpdated).toBe(true);
  });

  test('should implement distributed inference pipeline', async () => {
    const inferenceStages = ['preprocessing', 'feature_extraction', 'classification', 'postprocessing'];
    const completedStages: string[] = [];

    // Start servers
    await Promise.all([serverA.connect(), serverB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Connect clients
    await Promise.all([clientA.connect(), clientB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Set up distributed inference pipeline
    serverA.on('message', async (message: any, info: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          const stage = envelope.capabilities?.inference_stage;

          if (stage === 'preprocessing') {
            completedStages.push('preprocessing');

            // Forward to feature extraction (Server B)
            const featureEnvelope = UMICP.createEnvelope({
              from: 'inference-stage-1',
              to: 'inference-stage-2',
              operation: OperationType.DATA,
              messageId: `inference_${envelope.msg_id}_stage2`,
              capabilities: {
                inference_stage: 'feature_extraction',
                data: 'preprocessed_data_' + envelope.capabilities.data,
                pipeline_id: envelope.capabilities.pipeline_id
              }
            });

            const featureJson = featureEnvelope.serialize();
            clientB.send(featureJson);
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    serverB.on('message', async (message: any, info: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          const stage = envelope.capabilities?.inference_stage;

          if (stage === 'feature_extraction') {
            completedStages.push('feature_extraction');

            // Complete remaining stages
            const remainingStages = ['classification', 'postprocessing'];
            for (const nextStage of remainingStages) {
              completedStages.push(nextStage);
            }

            // Send final result back
            const resultEnvelope = UMICP.createEnvelope({
              from: 'inference-pipeline',
              to: 'client',
              operation: OperationType.DATA,
              messageId: `inference_complete_${envelope.msg_id}`,
              capabilities: {
                inference_stage: 'completed',
                result: 'inference_result_final',
                pipeline_id: envelope.capabilities.pipeline_id,
                confidence: '0.92'
              }
            });

            const resultJson = resultEnvelope.serialize();
            clientA.send(resultJson);
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    let pipelineCompleted = false;
    clientA.on('message', (message: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);
          if (envelope.capabilities?.inference_stage === 'completed') {
            pipelineCompleted = true;
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    // Start distributed inference
    const initialEnvelope = UMICP.createEnvelope({
      from: 'client',
      to: 'inference-stage-1',
      operation: OperationType.DATA,
      messageId: 'distributed_inference_001',
      capabilities: {
        inference_stage: 'preprocessing',
        data: 'input_text_sample',
        pipeline_id: 'pipeline_123',
        model_type: 'distributed'
      }
    });

    const initialJson = initialEnvelope.serialize();
    clientA.send(initialJson);

    // Wait for distributed inference completion
    await new Promise(resolve => setTimeout(resolve, 1000));

    expect(completedStages).toContain('preprocessing');
    expect(completedStages).toContain('feature_extraction');
    expect(completedStages).toContain('classification');
    expect(completedStages).toContain('postprocessing');
    expect(pipelineCompleted).toBe(true);
  });

  test('should handle S2S security and authentication', async () => {
    let authChallengeReceived = false;
    let authResponseSent = false;
    let secureChannelEstablished = false;

    // Start servers
    await Promise.all([serverA.connect(), serverB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Connect clients
    await Promise.all([clientA.connect(), clientB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Set up security handshake
    serverA.on('message', async (message: any, info: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);

          if (envelope.capabilities?.security === 'auth_request') {
            authChallengeReceived = true;

            // Send authentication challenge
            const challengeEnvelope = UMICP.createEnvelope({
              from: 'auth-server-alpha',
              to: envelope.from,
              operation: OperationType.CONTROL,
              messageId: `auth_challenge_${envelope.msg_id}`,
              capabilities: {
                security: 'auth_challenge',
                challenge: 'nonce_12345',
                algorithm: 'ecdsa',
                required_cert: 'server-alpha-cert'
              }
            });

            const challengeJson = challengeEnvelope.serialize();
            clientA.send(challengeJson);
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    clientA.on('message', (message: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);

          if (envelope.capabilities?.security === 'auth_challenge') {
            authResponseSent = true;

            // Send authentication response
            const responseEnvelope = UMICP.createEnvelope({
              from: envelope.to,
              to: 'secure-channel',
              operation: OperationType.CONTROL,
              messageId: `auth_response_${envelope.msg_id}`,
              capabilities: {
                security: 'auth_response',
                signature: 'signature_of_nonce',
                certificate: 'client-beta-cert',
                session_key: 'established_session_key'
              }
            });

            const responseJson = responseEnvelope.serialize();
            clientA.send(responseJson);
          }

          if (envelope.capabilities?.security === 'channel_established') {
            secureChannelEstablished = true;
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    // Initiate secure S2S connection
    const authRequestEnvelope = UMICP.createEnvelope({
      from: 'server-beta',
      to: 'auth-server-alpha',
      operation: OperationType.CONTROL,
      messageId: 's2s_auth_request_001',
      capabilities: {
        security: 'auth_request',
        protocol: 'mutual_tls',
        requested_service: 'model_collaboration',
        session_timeout: '3600'
      }
    });

    const authRequestJson = authRequestEnvelope.serialize();
    clientA.send(authRequestJson);

    // Wait for security handshake completion
    await new Promise(resolve => setTimeout(resolve, 1000));

    expect(authChallengeReceived).toBe(true);
    expect(authResponseSent).toBe(true);
    expect(secureChannelEstablished).toBe(true);
  });

  test('should support S2S load balancing and failover', async () => {
    const serverStats: any = { load: {}, requests: {} };

    // Start servers
    await Promise.all([serverA.connect(), serverB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Connect clients
    await Promise.all([clientA.connect(), clientB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Set up load balancing handlers
    serverA.on('message', async (message: any, info: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);

          if (envelope.capabilities?.load_balancing === 'request') {
            serverStats.requests.alpha = (serverStats.requests.alpha || 0) + 1;

            // Check load and redirect if necessary
            if (serverStats.requests.alpha > 2) {
              const redirectEnvelope = UMICP.createEnvelope({
                from: 'load-balancer-alpha',
                to: envelope.from,
                operation: OperationType.CONTROL,
                messageId: `redirect_${envelope.msg_id}`,
                capabilities: {
                  load_balancing: 'redirect',
                  redirect_to: 'server-beta',
                  reason: 'high_load'
                }
              });

              const redirectJson = redirectEnvelope.serialize();
              clientA.send(redirectJson);
            }
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    serverB.on('message', (message: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);

          if (envelope.capabilities?.load_balancing === 'request') {
            serverStats.requests.beta = (serverStats.requests.beta || 0) + 1;
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    // Send multiple requests to test load balancing
    for (let i = 0; i < 5; i++) {
      const requestEnvelope = UMICP.createEnvelope({
        from: 'client',
        to: 'load-balancer-alpha',
        operation: OperationType.DATA,
        messageId: `load_test_${i}`,
        capabilities: {
          load_balancing: 'request',
          request_type: 'inference',
          priority: i % 2 === 0 ? 'high' : 'normal'
        }
      });

      const requestJson = requestEnvelope.serialize();
      clientA.send(requestJson);

      // Small delay between requests
      await new Promise(resolve => setTimeout(resolve, 50));
    }

    // Wait for load balancing decisions
    await new Promise(resolve => setTimeout(resolve, 800));

    expect(serverStats.requests.alpha).toBeGreaterThan(0);
    expect(serverStats.requests.beta).toBeGreaterThan(0);

    // Alpha should have received more requests initially, then redirected some to Beta
    expect(serverStats.requests.alpha).toBeGreaterThan(serverStats.requests.beta);
  });

  test('should handle S2S protocol negotiation', async () => {
    let protocolNegotiated = false;
    let negotiatedVersion = '';
    let featuresNegotiated: string[] = [];

    // Start servers
    await Promise.all([serverA.connect(), serverB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Connect clients
    await Promise.all([clientA.connect(), clientB.connect()]);
    await new Promise(resolve => setTimeout(resolve, 200));

    // Set up protocol negotiation handlers
    serverA.on('message', async (message: any, info: any) => {
      if (typeof message === 'string') {
        try {
          const envelope = JSON.parse(message);

          if (envelope.capabilities?.protocol === 'negotiation_request') {
            // Respond with supported protocols and features
            const negotiationEnvelope = UMICP.createEnvelope({
              from: 'server-alpha',
              to: envelope.from,
              operation: OperationType.CONTROL,
              messageId: `negotiation_${envelope.msg_id}`,
              capabilities: {
                protocol: 'negotiation_response',
                supported_versions: JSON.stringify(['1.0', '1.1', '2.0']),
                preferred_version: '1.1',
                supported_features: JSON.stringify(['compression', 'encryption', 'streaming', 'federated_learning']),
                max_message_size: '1048576',
                timeout: '30'
              }
            });

            const negotiationJson = negotiationEnvelope.serialize();
            clientA.send(negotiationJson);
          }

          if (envelope.capabilities?.protocol === 'negotiation_confirm') {
            protocolNegotiated = true;
            negotiatedVersion = envelope.capabilities.agreed_version;
            featuresNegotiated = envelope.capabilities.agreed_features;
          }
        } catch (error) {
          // Not a valid envelope
        }
      }
    });

    // Initiate protocol negotiation
    const negotiationRequestEnvelope = UMICP.createEnvelope({
      from: 'server-beta',
      to: 'server-alpha',
      operation: OperationType.CONTROL,
      messageId: 'protocol_negotiation_001',
      capabilities: {
        protocol: 'negotiation_request',
        client_version: '1.0',
        requested_features: JSON.stringify(['compression', 'encryption', 'federated_learning']),
        max_message_size: '2097152'
      }
    });

    const negotiationRequestJson = negotiationRequestEnvelope.serialize();
    clientA.send(negotiationRequestJson);

    // Wait for negotiation response
    await new Promise(resolve => setTimeout(resolve, 300));

    // Confirm negotiated protocol
    const confirmEnvelope = UMICP.createEnvelope({
      from: 'server-beta',
      to: 'server-alpha',
      operation: OperationType.CONTROL,
      messageId: 'protocol_confirm_001',
      capabilities: {
        protocol: 'negotiation_confirm',
        agreed_version: '1.1',
        agreed_features: JSON.stringify(['compression', 'encryption', 'federated_learning']),
        session_id: 'session_12345'
      }
    });

    const confirmJson = confirmEnvelope.serialize();
    clientA.send(confirmJson);

    // Wait for confirmation
    await new Promise(resolve => setTimeout(resolve, 300));

    expect(protocolNegotiated).toBe(true);
    expect(negotiatedVersion).toBe('1.1');
    expect(featuresNegotiated).toContain('compression');
    expect(featuresNegotiated).toContain('encryption');
    expect(featuresNegotiated).toContain('federated_learning');
  });
});
