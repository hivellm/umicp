/**
 * Tests for StreamableHTTP Transport
 *
 * Tests cover:
 * - StreamableHTTPServer
 * - StreamableHTTPClient
 * - StreamableHTTPPeer
 * - HTTP/1.1 request/response cycle
 * - Streaming capabilities
 * - Error handling
 * - Integration patterns
 */

import {
  StreamableHTTPServer,
  StreamableHTTPClient,
  StreamableHTTPPeer,
  Envelope,
  OperationType,
  type HTTPPeerConnection
} from '../src/index.js';
import * as http from 'http';

describe('StreamableHTTP Transport', () => {
  let server: http.Server;
  let httpServer: StreamableHTTPServer;
  let client: StreamableHTTPClient;

  const TEST_PORT = 21000;
  const BASE_URL = `http://localhost:${TEST_PORT}`;

  afterEach(async () => {
    // Clean up
    if (client) {
      await client.disconnect?.();
    }

    if (server) {
      await new Promise<void>((resolve) => {
        server.close(() => resolve());
      });
    }

    await new Promise(resolve => setTimeout(resolve, 100));
  });

  describe('StreamableHTTPServer', () => {
    test('should create server instance', () => {
      httpServer = new StreamableHTTPServer({
        onMessage: async (envelope, req, res) => {
          // Handler
        }
      });

      expect(httpServer).toBeInstanceOf(StreamableHTTPServer);
    });

    test('should handle health check GET request', (done) => {
      httpServer = new StreamableHTTPServer();

      server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      server.listen(TEST_PORT, () => {
        http.get(BASE_URL + '/umicp', (res) => {
          let data = '';
          res.on('data', chunk => data += chunk);
          res.on('end', () => {
            const response = JSON.parse(data);
            expect(response.status).toBe('ok');
            expect(response.protocol).toBe('UMICP');
            expect(response.transport).toBe('streamable-http');
            done();
          });
        });
      });
    });

    test('should handle POST request with UMICP envelope', (done) => {
      let messageReceived = false;

      httpServer = new StreamableHTTPServer({
        onMessage: async (envelope, req, res) => {
          expect(envelope.getFrom()).toBe('test-client');
          expect(envelope.getOperation()).toBe(OperationType.DATA);
          messageReceived = true;

          // Send response
          const response = new Envelope({
            from: 'test-server',
            to: 'test-client',
            operation: OperationType.ACK,
            messageId: `ack-${envelope.getMessageId()}`,
          });

          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end(response.serialize());
        }
      });

      server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      server.listen(TEST_PORT, () => {
        const envelope = new Envelope({
          from: 'test-client',
          to: 'test-server',
          operation: OperationType.DATA,
          messageId: `msg-${Date.now()}`,
          capabilities: { test: 'true' }
        });

        const postData = envelope.serialize();

        const options = {
          hostname: 'localhost',
          port: TEST_PORT,
          path: '/umicp',
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
            'Content-Length': Buffer.byteLength(postData),
          }
        };

        const req = http.request(options, (res) => {
          let data = '';
          res.on('data', chunk => data += chunk);
          res.on('end', () => {
            expect(messageReceived).toBe(true);
            const response = Envelope.deserialize(data);
            expect(response.getOperation()).toBe(OperationType.ACK);
            done();
          });
        });

        req.write(postData);
        req.end();
      });
    });

    test('should reject invalid envelope', (done) => {
      httpServer = new StreamableHTTPServer({
        onError: (error, req, res) => {
          expect(error.message).toContain('Invalid');
        }
      });

      server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      server.listen(TEST_PORT, () => {
        const options = {
          hostname: 'localhost',
          port: TEST_PORT,
          path: '/umicp',
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          }
        };

        const req = http.request(options, (res) => {
          expect(res.statusCode).toBe(400);
          done();
        });

        req.write(JSON.stringify({ invalid: 'envelope' }));
        req.end();
      });
    });

    test('should handle connection tracking', (done) => {
      let connectionReceived = false;
      let disconnectionReceived = false;

      httpServer = new StreamableHTTPServer({
        onConnection: (req) => {
          connectionReceived = true;
        },
        onDisconnection: (req) => {
          disconnectionReceived = true;
        },
        onMessage: async (envelope, req, res) => {
          res.writeHead(200);
          res.end('ok');
        }
      });

      server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      server.listen(TEST_PORT, () => {
        const envelope = new Envelope({
          from: 'client',
          to: 'server',
          operation: OperationType.DATA,
          messageId: 'test',
        });

        const options = {
          hostname: 'localhost',
          port: TEST_PORT,
          path: '/umicp',
          method: 'POST',
        };

        const req = http.request(options, (res) => {
          res.on('data', () => {});
          res.on('end', () => {
            setTimeout(() => {
              expect(connectionReceived).toBe(true);
              expect(disconnectionReceived).toBe(true);
              done();
            }, 100);
          });
        });

        req.write(envelope.serialize());
        req.end();
      });
    });

    test('should reject unsupported HTTP methods', (done) => {
      httpServer = new StreamableHTTPServer();

      server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      server.listen(TEST_PORT, () => {
        const options = {
          hostname: 'localhost',
          port: TEST_PORT,
          path: '/umicp',
          method: 'DELETE',
        };

        const req = http.request(options, (res) => {
          expect(res.statusCode).toBe(405);
          done();
        });

        req.end();
      });
    });
  });

  describe('StreamableHTTPClient', () => {
    test('should create client instance', () => {
      client = new StreamableHTTPClient({
        baseUrl: BASE_URL,
        path: '/umicp',
      });

      expect(client).toBeInstanceOf(StreamableHTTPClient);
    });

    test('should send envelope and receive response', (done) => {
      httpServer = new StreamableHTTPServer({
        onMessage: async (envelope, req, res) => {
          const response = new Envelope({
            from: 'server',
            to: envelope.getFrom(),
            operation: OperationType.ACK,
            messageId: `ack-${envelope.getMessageId()}`,
            capabilities: { status: 'received' }
          });

          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end(response.serialize());
        }
      });

      server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      server.listen(TEST_PORT, async () => {
        client = new StreamableHTTPClient({
          baseUrl: BASE_URL,
          path: '/umicp',
        });

        const envelope = new Envelope({
          from: 'client',
          to: 'server',
          operation: OperationType.DATA,
          messageId: `req-${Date.now()}`,
          capabilities: { action: 'test' }
        });

        try {
          const response = await client.send(envelope);
          expect(response.getOperation()).toBe(OperationType.ACK);
          expect(response.getCapabilities().status).toBe('received');
          done();
        } catch (error) {
          done(error);
        }
      });
    });

    test('should handle timeout', (done) => {
      httpServer = new StreamableHTTPServer({
        onMessage: async (envelope, req, res) => {
          // Never respond to simulate timeout
          await new Promise(resolve => setTimeout(resolve, 5000));
        }
      });

      server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      server.listen(TEST_PORT, async () => {
        client = new StreamableHTTPClient({
          baseUrl: BASE_URL,
          path: '/umicp',
          timeout: 500, // Short timeout
        });

        const envelope = new Envelope({
          from: 'client',
          to: 'server',
          operation: OperationType.DATA,
          messageId: 'timeout-test',
        });

        try {
          await client.send(envelope);
          done(new Error('Should have timed out'));
        } catch (error) {
          expect(error).toBeDefined();
          done();
        }
      });
    });

    test('should handle connection errors', async () => {
      client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:29997', // Non-existent server
        path: '/umicp',
      });

      const envelope = new Envelope({
        from: 'client',
        to: 'server',
        operation: OperationType.DATA,
        messageId: 'error-test',
      });

      try {
        await client.send(envelope);
        fail('Should have thrown error');
      } catch (error) {
        expect(error).toBeDefined();
      }
    });

    test('should include custom headers', (done) => {
      let customHeaderReceived = false;

      httpServer = new StreamableHTTPServer({
        onMessage: async (envelope, req, res) => {
          if (req.headers['x-custom-header'] === 'test-value') {
            customHeaderReceived = true;
          }

          res.writeHead(200);
          res.end(new Envelope({
            from: 'server',
            to: 'client',
            operation: OperationType.ACK,
            messageId: 'ack',
          }).serialize());
        }
      });

      server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      server.listen(TEST_PORT, async () => {
        client = new StreamableHTTPClient({
          baseUrl: BASE_URL,
          path: '/umicp',
          headers: {
            'X-Custom-Header': 'test-value',
          }
        });

        const envelope = new Envelope({
          from: 'client',
          to: 'server',
          operation: OperationType.DATA,
          messageId: 'header-test',
        });

        await client.send(envelope);

        setTimeout(() => {
          expect(customHeaderReceived).toBe(true);
          done();
        }, 100);
      });
    });
  });

  describe('StreamableHTTPPeer', () => {
    let peer1: StreamableHTTPPeer;
    let peer2: StreamableHTTPPeer;
    let peer3: StreamableHTTPPeer;

    afterEach(async () => {
      if (peer1) await peer1.shutdown();
      if (peer2) await peer2.shutdown();
      if (peer3) await peer3.shutdown();
      await new Promise(resolve => setTimeout(resolve, 200));
    });

    test('should create peer with server component', (done) => {
      peer1 = new StreamableHTTPPeer({
        peerId: 'http-peer',
        server: {
          port: 21050,
          path: '/umicp',
        }
      });

      peer1.on('server:ready', () => {
        const stats = peer1.getStats();
        expect(stats.peerId).toBe('http-peer');
        expect(stats.serverActive).toBe(true);
        done();
      });
    });

    test('should send and receive messages between peers', (done) => {
      peer1 = new StreamableHTTPPeer({
        peerId: 'http-server',
        server: {
          port: 21051,
          path: '/umicp',
        }
      });

      peer1.on('server:ready', async () => {
        peer2 = new StreamableHTTPPeer({
          peerId: 'http-client',
        });

        peer1.on('data', (envelope: Envelope) => {
          expect(envelope.getCapabilities().message).toBe('hello');
          done();
        });

        await peer2.connectToPeer(`http://localhost:21051/umicp`);

        const messageEnvelope = new Envelope({
          from: 'http-client',
          to: 'http-server',
          operation: OperationType.DATA,
          messageId: `http-msg-${Date.now()}`,
          capabilities: { message: 'hello' }
        });

        const peers = peer2.getPeers();
        await peer2.sendToPeer(peers[0].id, messageEnvelope);
      });
    });

    test('should handle peer registration', (done) => {
      peer1 = new StreamableHTTPPeer({
        peerId: 'registration-server',
        server: {
          port: 21052,
          path: '/umicp',
        }
      });

      peer1.on('server:ready', async () => {
        peer2 = new StreamableHTTPPeer({
          peerId: 'registration-client',
        });

        let peerReady = false;

        peer1.on('peer:ready', (peer: HTTPPeerConnection) => {
          expect(peer.peerId).toBe('registration-client');
          peerReady = true;
        });

        await peer2.connectToPeer(`http://localhost:21052/umicp`);

        await new Promise(resolve => setTimeout(resolve, 300));
        expect(peerReady).toBe(true);
        done();
      });
    });

    test('should broadcast to all connected HTTP peers', (done) => {
      peer1 = new StreamableHTTPPeer({
        peerId: 'http-broadcast-hub',
        server: {
          port: 21053,
          path: '/umicp',
        }
      });

      peer1.on('server:ready', async () => {
        peer2 = new StreamableHTTPPeer({ peerId: 'http-client-1' });
        peer3 = new StreamableHTTPPeer({ peerId: 'http-client-2' });

        let receivedCount = 0;

        const checkBroadcast = () => {
          receivedCount++;
          if (receivedCount === 2) {
            done();
          }
        };

        peer2.on('data', checkBroadcast);
        peer3.on('data', checkBroadcast);

        await peer2.connectToPeer(`http://localhost:21053/umicp`);
        await peer3.connectToPeer(`http://localhost:21053/umicp`);

        await new Promise(resolve => setTimeout(resolve, 300));

        const broadcastEnvelope = new Envelope({
          from: 'http-broadcast-hub',
          to: 'all',
          operation: OperationType.DATA,
          messageId: `broadcast-${Date.now()}`,
          capabilities: { type: 'broadcast', message: 'Hello HTTP peers!' }
        });

        const sent = await peer1.broadcast(broadcastEnvelope);
        expect(sent).toBe(2);
      });
    });

    test('should get peer statistics', (done) => {
      peer1 = new StreamableHTTPPeer({
        peerId: 'stats-http-server',
        server: {
          port: 21054,
          path: '/umicp',
        }
      });

      peer1.on('server:ready', async () => {
        peer2 = new StreamableHTTPPeer({ peerId: 'stats-http-client' });

        await peer2.connectToPeer(`http://localhost:21054/umicp`);

        await new Promise(resolve => setTimeout(resolve, 300));

        const stats1 = peer1.getStats();
        expect(stats1.totalPeers).toBeGreaterThan(0);

        const stats2 = peer2.getStats();
        expect(stats2.totalPeers).toBeGreaterThan(0);

        done();
      });
    });

    test('should handle disconnection', (done) => {
      peer1 = new StreamableHTTPPeer({
        peerId: 'disconnect-http-server',
        server: {
          port: 21055,
          path: '/umicp',
        }
      });

      peer1.on('server:ready', async () => {
        peer2 = new StreamableHTTPPeer({ peerId: 'disconnect-http-client' });

        let disconnectFired = false;

        peer1.on('peer:disconnect', () => {
          disconnectFired = true;
        });

        await peer2.connectToPeer(`http://localhost:21055/umicp`);
        await new Promise(resolve => setTimeout(resolve, 300));

        await peer2.shutdown();
        await new Promise(resolve => setTimeout(resolve, 300));

        expect(disconnectFired).toBe(true);
        done();
      });
    });
  });

  describe('Integration Patterns', () => {
    test('should work with Express-like middleware pattern', (done) => {
      const middleware = (req: http.IncomingMessage, res: http.ServerResponse, next: Function) => {
        // Add custom property
        (req as any).customProperty = 'test-value';
        next();
      };

      httpServer = new StreamableHTTPServer({
        onMessage: async (envelope, req, res) => {
          expect((req as any).customProperty).toBe('test-value');

          res.writeHead(200);
          res.end(new Envelope({
            from: 'server',
            to: 'client',
            operation: OperationType.ACK,
            messageId: 'ack',
          }).serialize());
        }
      });

      server = http.createServer((req, res) => {
        middleware(req, res, () => {
          httpServer.handleRequest(req, res);
        });
      });

      server.listen(TEST_PORT, async () => {
        client = new StreamableHTTPClient({
          baseUrl: BASE_URL,
          path: '/umicp',
        });

        const envelope = new Envelope({
          from: 'client',
          to: 'server',
          operation: OperationType.DATA,
          messageId: 'middleware-test',
        });

        await client.send(envelope);
        done();
      });
    });

    test('should handle concurrent requests', (done) => {
      let requestCount = 0;

      httpServer = new StreamableHTTPServer({
        onMessage: async (envelope, req, res) => {
          requestCount++;

          const response = new Envelope({
            from: 'server',
            to: envelope.getFrom(),
            operation: OperationType.ACK,
            messageId: `ack-${envelope.getMessageId()}`,
            capabilities: { requestNumber: requestCount.toString() }
          });

          res.writeHead(200);
          res.end(response.serialize());
        }
      });

      server = http.createServer((req, res) => {
        httpServer.handleRequest(req, res);
      });

      server.listen(TEST_PORT, async () => {
        client = new StreamableHTTPClient({
          baseUrl: BASE_URL,
          path: '/umicp',
        });

        const promises = [];

        for (let i = 0; i < 5; i++) {
          const envelope = new Envelope({
            from: 'client',
            to: 'server',
            operation: OperationType.DATA,
            messageId: `concurrent-${i}`,
          });

          promises.push(client.send(envelope));
        }

        const responses = await Promise.all(promises);
        expect(responses.length).toBe(5);
        expect(requestCount).toBe(5);
        done();
      });
    });
  });
});

