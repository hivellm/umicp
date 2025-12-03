/**
 * Example 3: HTTP Transport Communication
 *
 * This example demonstrates:
 * - HTTP-based UMICP communication
 * - Request-response pattern
 * - Multiple concurrent clients
 * - Health check endpoint
 */

import {
  StreamableHTTPServer,
  StreamableHTTPClient,
  StreamableHTTPPeer,
  Envelope,
  OperationType
} from '../src/index.js';
import * as http from 'http';

async function main() {
  console.log('🌐 UMICP Example 3: HTTP Communication\n');

  // Example 3.1: Basic HTTP Server-Client
  console.log('═══ Part 1: Basic HTTP Server-Client ═══\n');
  await basicHTTPExample();

  await new Promise(resolve => setTimeout(resolve, 2000));

  // Example 3.2: HTTP Peer with multiple clients
  console.log('\n═══ Part 2: HTTP Peer with Multiple Clients ═══\n');
  await httpPeerExample();

  console.log('\n✅ HTTP communication example completed!\n');
}

async function basicHTTPExample() {
  let messagesReceived = 0;

  // Create HTTP server
  const httpServer = new StreamableHTTPServer({
    onMessage: async (envelope, req, res) => {
      messagesReceived++;
      console.log(`📨 Server received message #${messagesReceived}:`);
      console.log(`   From: ${envelope.getFrom()}`);
      console.log(`   Action: ${envelope.getCapabilities()['action']}`);

      // Process and respond
      const response = new Envelope({
        from: 'http-server',
        to: envelope.getFrom(),
        operation: OperationType.ACK,
        messageId: `response-${envelope.getMessageId()}`,
        capabilities: {
          status: 'success',
          result: 'Data processed',
          timestamp: new Date().toISOString()
        }
      });

      res.writeHead(200, { 'Content-Type': 'application/json' });
      res.end(response.serialize());
      console.log('   ✅ Sent response');
    },

    onConnection: (req) => {
      console.log(`🔗 New HTTP connection from ${req.socket.remoteAddress}`);
    }
  });

  // Create Node.js HTTP server
  const server = http.createServer((req, res) => {
    httpServer.handleRequest(req, res);
  });

  await new Promise<void>(resolve => {
    server.listen(9080, () => {
      console.log('✅ HTTP Server listening on port 9080\n');
      resolve();
    });
  });

  // Test health check
  console.log('Testing health check endpoint...');
  const healthResponse = await fetch('http://localhost:9080/umicp');
  const healthData = await healthResponse.json();
  console.log('Health check response:', healthData);
  console.log('');

  // Create HTTP client
  const client = new StreamableHTTPClient({
    baseUrl: 'http://localhost:9080',
    path: '/umicp',
    timeout: 5000
  });

  // Send multiple requests
  console.log('Sending requests...\n');
  for (let i = 1; i <= 3; i++) {
    const request = new Envelope({
      from: 'http-client',
      to: 'http-server',
      operation: OperationType.DATA,
      messageId: `req-${i}-${Date.now()}`,
      capabilities: {
        action: 'process-data',
        data: `Data payload ${i}`,
        priority: i === 1 ? 'high' : 'normal'
      }
    });

    const response = await client.send(request);
    console.log(`📬 Client received response #${i}:`);
    console.log(`   Status: ${response.getCapabilities()['status']}`);
    console.log(`   Result: ${response.getCapabilities()['result']}`);
    console.log('');

    await new Promise(resolve => setTimeout(resolve, 500));
  }

  // Cleanup
  await new Promise<void>(resolve => {
    server.close(() => resolve());
  });
}

async function httpPeerExample() {
  // Create HTTP peer (server)
  const serverPeer = new StreamableHTTPPeer({
    peerId: 'http-peer-server',
    server: {
      port: 9081,
      path: '/umicp'
    }
  });

  serverPeer.on('server:ready', () => {
    console.log('✅ HTTP Peer Server ready on port 9081');
  });

  serverPeer.on('peer:ready', (peer) => {
    console.log(`🤝 Peer registered: ${peer.peerId}`);
  });

  let messageCount = 0;
  serverPeer.on('data', (envelope, peer) => {
    messageCount++;
    console.log(`\n📨 Peer server received message #${messageCount}:`);
    console.log(`   From: ${envelope.getFrom()}`);
    console.log(`   Content: ${envelope.getCapabilities()['content']}`);
  });

  await new Promise<void>(resolve => {
    serverPeer.on('server:ready', () => resolve());
  });

  // Create multiple HTTP peer clients
  console.log('\nCreating client peers...\n');
  const clients: StreamableHTTPPeer[] = [];

  for (let i = 1; i <= 3; i++) {
    const clientPeer = new StreamableHTTPPeer({
      peerId: `http-client-${i}`
    });

    clientPeer.on('data', (envelope) => {
      console.log(`   Client ${i} received data`);
    });

    await clientPeer.connectToPeer('http://localhost:9081/umicp');
    console.log(`✅ Client ${i} connected`);

    clients.push(clientPeer);
  }

  await new Promise(resolve => setTimeout(resolve, 1000));

  // Send messages from clients
  console.log('\nClients sending messages...\n');
  for (let i = 0; i < clients.length; i++) {
    const message = new Envelope({
      from: `http-client-${i + 1}`,
      to: 'http-peer-server',
      operation: OperationType.DATA,
      messageId: `client-msg-${i + 1}-${Date.now()}`,
      capabilities: {
        content: `Message from client ${i + 1}`,
        timestamp: new Date().toISOString()
      }
    });

    const peers = clients[i].getPeers();
    if (peers.length > 0) {
      await clients[i].sendToPeer(peers[0].id, message);
      console.log(`   Client ${i + 1} sent message`);
    }

    await new Promise(resolve => setTimeout(resolve, 300));
  }

  await new Promise(resolve => setTimeout(resolve, 1000));

  // Broadcast from server to all clients
  console.log('\nServer broadcasting to all clients...\n');
  const broadcast = new Envelope({
    from: 'http-peer-server',
    to: 'all',
    operation: OperationType.DATA,
    messageId: `broadcast-${Date.now()}`,
    capabilities: {
      type: 'announcement',
      message: 'Server announcement to all clients!'
    }
  });

  const sent = await serverPeer.broadcast(broadcast);
  console.log(`📡 Broadcast sent to ${sent} client(s)`);

  await new Promise(resolve => setTimeout(resolve, 1000));

  // Statistics
  console.log('\n📊 Statistics:');
  console.log('Server:', serverPeer.getStats());
  clients.forEach((client, idx) => {
    console.log(`Client ${idx + 1}:`, client.getStats());
  });

  // Cleanup
  console.log('\n🧹 Cleaning up...');
  for (const client of clients) {
    await client.shutdown();
  }
  await serverPeer.shutdown();
}

// Run example
main().catch(console.error);

