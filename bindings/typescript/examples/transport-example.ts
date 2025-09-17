/**
 * UMICP TypeScript Bindings - Transport Layer Example
 * Demonstrates WebSocket and HTTP/2 transport functionality
 * using native Node.js libraries with advanced features
 */

import WebSocket, { WebSocketServer } from 'ws';
import * as http2 from 'http2';
import { EventEmitter } from 'events';
import { Envelope, UMICP, OperationType, PayloadType, EncodingType } from '../src/index';

interface WebSocketConfig {
  url?: string;
  isServer?: boolean;
  port?: number;
  path?: string;
  maxReconnectAttempts?: number;
  reconnectInterval?: number;
  heartbeatInterval?: number;
  perMessageDeflate?: boolean;
  maxPayload?: number;
}

interface ConnectionInfo {
  id: string;
  remoteAddress: string;
  connectedAt: Date;
}

class AdvancedWebSocketTransport extends EventEmitter {
  private ws: WebSocket | null = null;
  private wss: WebSocketServer | null = null;
  private config: WebSocketConfig;
  private reconnectAttempts = 0;
  private reconnectTimer: NodeJS.Timeout | null = null;
  private heartbeatTimer: NodeJS.Timeout | null = null;
  private connections = new Map<WebSocket, ConnectionInfo>();
  private isReconnecting = false;

  constructor(config: WebSocketConfig) {
    super();
    this.config = {
      maxReconnectAttempts: 5,
      reconnectInterval: 3000,
      heartbeatInterval: 30000,
      perMessageDeflate: true,
      maxPayload: 1024 * 1024, // 1MB
      ...config
    };
  }

  async connect(): Promise<boolean> {
    try {
      if (this.config.isServer) {
        return this.startServer();
      } else {
        return this.connectClient();
      }
    } catch (error) {
      console.error('Connection failed:', error);
      this.emit('error', error);
      return false;
    }
  }

  private async startServer(): Promise<boolean> {
    return new Promise((resolve) => {
      try {
        const port = this.config.port || 8080;
        const options: WebSocket.ServerOptions = {
          port,
          path: this.config.path,
          perMessageDeflate: this.config.perMessageDeflate,
          maxPayload: this.config.maxPayload
        };

        this.wss = new WebSocketServer(options);

        this.wss.on('connection', (ws: WebSocket, request) => {
          const connectionId = this.generateConnectionId();
          const connectionInfo: ConnectionInfo = {
            id: connectionId,
            remoteAddress: request.socket.remoteAddress || 'unknown',
            connectedAt: new Date()
          };

          this.connections.set(ws, connectionInfo);

          // Configure WebSocket
          ws.on('message', (data: WebSocket.Data) => {
            this.handleMessage(ws, data);
          });

          ws.on('close', (code: number, reason: Buffer) => {
            this.connections.delete(ws);
            this.emit('clientDisconnected', connectionInfo);
          });

          ws.on('error', (error) => {
            console.error(`Client ${connectionId} error:`, error);
            this.emit('clientError', connectionInfo, error);
          });

          ws.on('ping', () => {
            ws.pong();
          });

          ws.on('pong', () => {
            // Handle pong response for heartbeat
          });

          this.emit('clientConnected', connectionInfo);
        });

        this.wss.on('listening', () => {
          this.emit('serverStarted', port);
          this.startHeartbeat();
          resolve(true);
        });

        this.wss.on('error', (error) => {
          console.error('WebSocket server error:', error);
          this.emit('error', error);
          resolve(false);
        });

      } catch (error) {
        console.error('Failed to start WebSocket server:', error);
        this.emit('error', error);
        resolve(false);
      }
    });
  }

  private async connectClient(): Promise<boolean> {
    if (this.isReconnecting) {
      return false;
    }

    return new Promise((resolve) => {
      try {
        if (!this.config.url) {
          throw new Error('URL is required for client connection');
        }

        const options: WebSocket.ClientOptions = {
          perMessageDeflate: this.config.perMessageDeflate,
          maxPayload: this.config.maxPayload
        };

        this.ws = new WebSocket(this.config.url, options);

        this.ws.on('open', () => {
          this.reconnectAttempts = 0;
          this.isReconnecting = false;
          this.startHeartbeat();
          this.emit('connected');
          resolve(true);
        });

        this.ws.on('message', (data: WebSocket.Data) => {
          this.handleMessage(this.ws!, data);
        });

        this.ws.on('close', (code: number, reason: Buffer) => {
          this.stopHeartbeat();
          this.emit('disconnected', code, reason);

          if (code !== 1000 && this.reconnectAttempts < (this.config.maxReconnectAttempts || 5)) {
            this.scheduleReconnect();
          }
        });

        this.ws.on('error', (error) => {
          console.error('WebSocket client error:', error);
          this.emit('error', error);
          resolve(false);
        });

        this.ws.on('ping', () => {
          if (this.ws) this.ws.pong();
        });

        this.ws.on('pong', () => {
          this.emit('heartbeat');
        });

      } catch (error) {
        console.error('Failed to create WebSocket client:', error);
        this.emit('error', error);
        resolve(false);
      }
    });
  }

  private handleMessage(ws: WebSocket, data: WebSocket.Data): void {
    try {
      const message = data.toString();
      const connectionInfo = this.connections.get(ws);

      // Try to parse as JSON first
      try {
        const parsedMessage = JSON.parse(message);

        // Check if this looks like a serialized UMICP envelope
        if (parsedMessage && typeof parsedMessage === 'object' &&
            ('from' in parsedMessage || 'to' in parsedMessage || 'operation' in parsedMessage)) {
          // Try to deserialize as UMICP envelope
          try {
            const { UMICP } = require('../src/index');
            const envelope = UMICP.deserializeEnvelope(message);
            this.emit('message', envelope, connectionInfo);
            return;
          } catch (envelopeError) {
            // If envelope deserialization fails, fall back to parsed JSON
            this.emit('message', parsedMessage, connectionInfo);
            return;
          }
        }

        // Regular JSON object
        if (parsedMessage && typeof parsedMessage === 'object') {
          this.emit('message', parsedMessage, connectionInfo);
        } else {
          this.emit('message', message, connectionInfo);
        }
      } catch {
        // If not JSON, emit as raw string
        this.emit('message', message, connectionInfo);
      }
    } catch (error) {
      this.emit('messageError', error);
    }
  }

  private generateConnectionId(): string {
    return `conn_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
  }

  private scheduleReconnect(): void {
    if (this.isReconnecting) return;

    this.isReconnecting = true;
    this.reconnectAttempts++;

    const interval = this.config.reconnectInterval || 3000;

    this.reconnectTimer = setTimeout(() => {
      this.connectClient().then(success => {
        if (!success) {
          this.isReconnecting = false;
          if (this.reconnectAttempts >= (this.config.maxReconnectAttempts || 5)) {
            console.error('Max reconnect attempts reached');
            this.emit('maxReconnectAttemptsReached');
          }
        }
      });
    }, interval);
  }

  private startHeartbeat(): void {
    const interval = this.config.heartbeatInterval || 30000;
    this.heartbeatTimer = setInterval(() => {
      if (this.ws && this.ws.readyState === WebSocket.OPEN) {
        this.ws.ping();
      }
    }, interval);
  }

  private stopHeartbeat(): void {
    if (this.heartbeatTimer) {
      clearInterval(this.heartbeatTimer);
      this.heartbeatTimer = null;
    }
  }

  send(message: string | object, targetConnection?: string): boolean {
    try {
      let data: string;
      if (typeof message === 'string') {
        data = message;
      } else if (message && typeof message === 'object' && 'serialize' in message && typeof message.serialize === 'function') {
        // Handle Envelope objects with serialize method
        data = message.serialize();
      } else {
        // Handle regular objects
        data = JSON.stringify(message);
      }

      if (this.config.isServer && this.wss) {
        // Server mode - broadcast or send to specific client
        if (targetConnection) {
          const targetWs = Array.from(this.connections.entries())
            .find(([_, info]) => info.id === targetConnection)?.[0];

          if (targetWs && targetWs.readyState === WebSocket.OPEN) {
            targetWs.send(data);
            return true;
          }
          return false;
        } else {
          // Broadcast to all clients
          let sent = 0;
          this.connections.forEach((info, ws) => {
            if (ws.readyState === WebSocket.OPEN) {
              ws.send(data);
              sent++;
            }
          });
          return sent > 0;
        }
      } else if (!this.config.isServer && this.ws) {
        // Client mode
        if (this.ws.readyState === WebSocket.OPEN) {
          this.ws.send(data);
          return true;
        }
      }

      return false;
    } catch (error) {
      console.error('Failed to send message:', error);
      this.emit('sendError', error);
      return false;
    }
  }

  broadcast(message: string | object, excludeConnection?: string): number {
    if (!this.config.isServer || !this.wss) {
      return 0;
    }

    const data = typeof message === 'string' ? message : JSON.stringify(message);
    let sent = 0;

    this.connections.forEach((info, ws) => {
      if (ws.readyState === WebSocket.OPEN && info.id !== excludeConnection) {
        ws.send(data);
        sent++;
      }
    });

    return sent;
  }

  isConnected(): boolean {
    if (this.config.isServer) {
      // For server, check if the server instance exists (it's listening if it exists)
      return this.wss !== null;
    } else {
      return this.ws !== null && this.ws.readyState === WebSocket.OPEN;
    }
  }

  getStats(): any {
    const baseStats = {
      isServer: this.config.isServer,
      url: this.config.url,
      reconnectAttempts: this.reconnectAttempts,
      isReconnecting: this.isReconnecting
    };

    if (this.config.isServer) {
      return {
        ...baseStats,
        connectionCount: this.connections.size,
        connections: Array.from(this.connections.values()).map(info => ({
          id: info.id,
          remoteAddress: info.remoteAddress,
          connectedAt: info.connectedAt,
          uptime: Date.now() - info.connectedAt.getTime()
        }))
      };
    } else {
      return {
        ...baseStats,
        readyState: this.ws?.readyState,
        bufferedAmount: this.ws?.bufferedAmount || 0
      };
    }
  }

  async disconnect(): Promise<boolean> {
    try {
      this.stopHeartbeat();

      if (this.reconnectTimer) {
        clearTimeout(this.reconnectTimer);
        this.reconnectTimer = null;
      }

      if (this.wss) {
        // Close all client connections first
        const closePromises: Promise<void>[] = [];
        this.wss.clients.forEach(client => {
          closePromises.push(new Promise<void>((resolve) => {
            if (client.readyState === WebSocket.OPEN) {
              client.once('close', () => resolve());
              client.close(1000, 'Server shutdown');
            } else {
              resolve();
            }
          }));
        });

        // Wait for all clients to disconnect
        await Promise.all(closePromises);

        // Close the server
        await new Promise<void>((resolve) => {
          this.wss!.close(() => resolve());
        });
        this.wss = null;
      }

      if (this.ws) {
        if (this.ws.readyState === WebSocket.OPEN) {
          await new Promise<void>((resolve) => {
            this.ws!.once('close', () => resolve());
            this.ws!.close(1000, 'Client disconnect');
          });
        }
        this.ws = null;
      }

      this.connections.clear();
      this.emit('disconnected');
      return true;
    } catch (error) {
      console.error('Error disconnecting:', error);
      this.emit('error', error);
      return false;
    }
  }
}

class HTTP2Transport {
  private client: http2.ClientHttp2Session | null = null;
  private url: string;
  private messageCallback?: (message: string) => void;
  private connectionCallback?: (connected: boolean) => void;
  private stream: http2.ClientHttp2Stream | null = null;

  constructor(url: string) {
    this.url = url;
  }

  async connect(): Promise<boolean> {
    return new Promise((resolve) => {
      try {
        this.client = http2.connect(this.url);

        this.client.on('connect', () => {
          console.log('Connected to HTTP/2 server');
          if (this.connectionCallback) {
            this.connectionCallback(true);
          }
          resolve(true);
        });

        this.client.on('error', (error) => {
          console.error('HTTP/2 client error:', error);
          resolve(false);
        });

        this.client.on('close', () => {
          console.log('HTTP/2 connection closed');
          if (this.connectionCallback) {
            this.connectionCallback(false);
          }
        });

      } catch (error) {
        console.error('Failed to connect HTTP/2 client:', error);
        resolve(false);
      }
    });
  }

  send(message: string): boolean {
    if (!this.client || this.client.destroyed) {
      console.error('HTTP/2 client not connected');
      return false;
    }

    try {
      // Create a new stream for each message
      this.stream = this.client.request({
        ':method': 'POST',
        ':path': '/umicp',
        'content-type': 'application/json',
        'content-length': Buffer.byteLength(message, 'utf8')
      });

      this.stream.write(message);
      this.stream.end();

      this.stream.on('data', (chunk) => {
        const response = chunk.toString();
        console.log('HTTP/2 response:', response);
        if (this.messageCallback) {
          this.messageCallback(response);
        }
      });

      this.stream.on('end', () => {
        console.log('HTTP/2 stream ended');
      });

      return true;
    } catch (error) {
      console.error('Failed to send HTTP/2 message:', error);
      return false;
    }
  }

  isConnected(): boolean {
    return this.client && !this.client.destroyed ? true : false;
  }

  getStats(): any {
    return {
      connected: this.isConnected(),
      url: this.url,
      destroyed: this.client?.destroyed || false
    };
  }

  setMessageCallback(callback: (message: string) => void): void {
    this.messageCallback = callback;
  }

  setConnectionCallback(callback: (connected: boolean) => void): void {
    this.connectionCallback = callback;
  }

  disconnect(): boolean {
    try {
      if (this.client && !this.client.destroyed) {
        this.client.close();
      }
      return true;
    } catch (error) {
      console.error('Error disconnecting HTTP/2:', error);
      return false;
    }
  }
}

async function demonstrateWebSocketTransport() {
  console.log('🌐 Advanced WebSocket Transport Example');
  console.log('=======================================');

  // Create a WebSocket server with advanced features
  const wsServer = new AdvancedWebSocketTransport({
    url: 'ws://localhost:8080',
    isServer: true,
    port: 8080,
    path: '/umicp',
    heartbeatInterval: 15000,
    maxPayload: 2 * 1024 * 1024 // 2MB
  });

  // Set up event listeners
  wsServer.on('serverStarted', (port) => {
    console.log(`🚀 WebSocket server started on port ${port}`);
  });

  wsServer.on('clientConnected', (info) => {
    console.log(`👋 Client ${info.id} connected from ${info.remoteAddress}`);
  });

  wsServer.on('clientDisconnected', (info) => {
    console.log(`👋 Client ${info.id} disconnected`);
  });

  wsServer.on('message', (message, info) => {
    console.log(`📨 Message from ${info?.id || 'unknown'}:`, message);

    // Echo the message back to the client
    if (info?.id) {
      wsServer.send({
        type: 'echo',
        original: message,
        timestamp: new Date().toISOString()
      }, info.id);
    }
  });

  wsServer.on('error', (error) => {
    console.error('❌ Server error:', error);
  });

  // Start server
  const serverStarted = await wsServer.connect();
  if (!serverStarted) {
    console.error('Failed to start WebSocket server');
    return;
  }

  // Create a WebSocket client with reconnection
  const wsClient = new AdvancedWebSocketTransport({
    url: 'ws://localhost:8080/umicp',
    isServer: false,
    maxReconnectAttempts: 3,
    reconnectInterval: 2000,
    heartbeatInterval: 10000
  });

  wsClient.on('connected', () => {
    console.log('🔗 Client connected to server');
  });

  wsClient.on('disconnected', (code, reason) => {
    console.log(`🔌 Client disconnected: ${code} - ${reason.toString()}`);
  });

  wsClient.on('message', (message) => {
    console.log('📨 Client received:', message);
  });

  wsClient.on('heartbeat', () => {
    console.log('💓 Heartbeat received');
  });

  wsClient.on('error', (error) => {
    console.error('❌ Client error:', error);
  });

  // Connect client
  const clientConnected = await wsClient.connect();
  if (!clientConnected) {
    console.error('Failed to connect WebSocket client');
    return;
  }

  // Wait for connection to establish
  await new Promise(resolve => setTimeout(resolve, 1000));

  // Send multiple test messages
  const testMessages = [
    { type: 'umicp_envelope', data: 'Hello from WebSocket client!' },
    { type: 'ping', timestamp: Date.now() },
    { type: 'data', payload: { vector: [1, 2, 3, 4, 5] } }
  ];

  for (const message of testMessages) {
    const sent = wsClient.send(message);
    console.log('📤 Message sent:', sent, message.type);
    await new Promise(resolve => setTimeout(resolve, 500));
  }

  // Demonstrate server broadcast
  console.log('\n📢 Broadcasting message to all clients...');
  const broadcastCount = wsServer.broadcast({
    type: 'broadcast',
    message: 'Hello to all connected clients!',
    serverTime: new Date().toISOString()
  });
  console.log(`📢 Broadcast sent to ${broadcastCount} clients`);

  // Show server stats
  console.log('\n📊 Server Stats:', JSON.stringify(wsServer.getStats(), null, 2));

  // Wait for message exchange
  await new Promise(resolve => setTimeout(resolve, 3000));

  // Clean up
  wsClient.disconnect();
  await new Promise(resolve => setTimeout(resolve, 1000)); // Wait for clean disconnect
  wsServer.disconnect();

  console.log('✅ WebSocket example completed\n');
}

async function demonstrateHTTP2Transport() {
  console.log('🔒 HTTP/2 Transport Example');
  console.log('===========================');

  // Note: HTTP/2 requires TLS, so this is just a demonstration
  // In a real scenario, you'd need a proper HTTPS server
  console.log('⚠️  HTTP/2 transport requires TLS/HTTPS setup');
  console.log('   This example shows the API structure\n');

  const http2Transport = new HTTP2Transport('https://localhost:8443');

  http2Transport.setConnectionCallback((connected) => {
    console.log('HTTP/2 connection status:', connected);
  });

  http2Transport.setMessageCallback((message) => {
    console.log('HTTP/2 received message:', message);
  });

  console.log('HTTP/2 transport created (not connected for demo)');
  console.log('Stats:', http2Transport.getStats());
}

async function demonstrateUMICPWithTransports() {
  console.log('📦 UMICP with Advanced Transport Layer Example');
  console.log('==============================================');

  // Create UMICP envelope with advanced features
  const envelope = UMICP.createEnvelope({
    from: 'typescript-client',
    to: 'umicp-server',
    operation: OperationType.DATA,
    messageId: 'msg_' + Date.now(),
    capabilities: {
      transport: 'websocket',
      format: 'json',
      compression: 'gzip',
      encryption: 'aes256'
    },
    payloadHint: {
      type: PayloadType.VECTOR,
      size: 1024,
      encoding: EncodingType.FLOAT32
    }
  });

  const envelopeJson = envelope.serialize();
  console.log('📝 Created UMICP envelope:', envelopeJson.length, 'bytes');
  console.log('🔍 Envelope is valid:', envelope.validate());

  // Create server to handle UMICP messages
  const wsServer = new AdvancedWebSocketTransport({
    url: 'ws://localhost:8080',
    isServer: true,
    port: 8080,
    path: '/umicp'
  });

  wsServer.on('serverStarted', (port) => {
    console.log(`🚀 UMICP WebSocket server started on port ${port}`);
  });

  wsServer.on('message', async (message, info) => {
    try {
      // Try to parse as UMICP envelope
      if (typeof message === 'string') {
        const parsed = JSON.parse(message);

        if (parsed.v && parsed.from && parsed.to) {
          console.log('📨 Received UMICP envelope from:', parsed.from);

          // Create response envelope
          const responseEnvelope = UMICP.createEnvelope({
            from: 'umicp-server',
            to: parsed.from,
            operation: OperationType.ACK,
            messageId: 'ack_' + parsed.msg_id,
            capabilities: {
              transport: 'websocket',
              processed: 'true'
            }
          });

          // Send acknowledgment
          const responseJson = responseEnvelope.serialize();
          wsServer.send(responseJson, info?.id);
          console.log('✅ Sent acknowledgment to:', parsed.from);
        }
      }
    } catch (error) {
      console.log('📨 Received non-UMICP message:', message);
    }
  });

  // Start server
  const serverStarted = await wsServer.connect();
  if (!serverStarted) {
    console.error('Failed to start UMICP server');
    return;
  }

  // Create client to send UMICP messages
  const wsClient = new AdvancedWebSocketTransport({
    url: 'ws://localhost:8080/umicp',
    isServer: false,
    maxReconnectAttempts: 2,
    reconnectInterval: 1000
  });

  let receivedAck = false;

  wsClient.on('connected', () => {
    console.log('🔗 UMICP client connected');
  });

  wsClient.on('message', (message) => {
    try {
      if (typeof message === 'string') {
        const parsed = JSON.parse(message);

        if (parsed.op === 'ack' && parsed.to === 'typescript-client') {
          console.log('✅ Received acknowledgment for message:', parsed.msg_id);
          receivedAck = true;
        }
      }
    } catch (error) {
      console.log('📨 Client received non-JSON message:', message);
    }
  });

  // Connect client
  const clientConnected = await wsClient.connect();
  if (!clientConnected) {
    console.error('Failed to connect UMICP client');
    wsServer.disconnect();
    return;
  }

  // Wait for connection
  await new Promise(resolve => setTimeout(resolve, 500));

  // Send UMICP envelope
  console.log('\n📤 Sending UMICP envelope via WebSocket...');
  const sent = wsClient.send(envelopeJson);
  console.log('📤 Envelope sent:', sent);

  // Wait for acknowledgment with timeout
  const timeout = 5000;
  const startTime = Date.now();

  while (!receivedAck && (Date.now() - startTime) < timeout) {
    await new Promise(resolve => setTimeout(resolve, 100));
  }

  if (receivedAck) {
    console.log('🎉 UMICP message exchange completed successfully!');
  } else {
    console.log('⚠️  Timeout waiting for acknowledgment');
  }

  // Clean up
  wsClient.disconnect();
  await new Promise(resolve => setTimeout(resolve, 500));
  wsServer.disconnect();

  console.log('✅ UMICP transport example completed!\n');
}

async function demonstrateChatRoom() {
  console.log('💬 WebSocket Chat Room Example');
  console.log('===============================');

  // Create chat server
  const chatServer = new AdvancedWebSocketTransport({
    url: 'ws://localhost:8081',
    isServer: true,
    port: 8081,
    path: '/chat'
  });

  const users = new Map<string, string>(); // connectionId -> username

  chatServer.on('serverStarted', (port) => {
    console.log(`🚀 Chat server started on port ${port}`);
  });

  chatServer.on('clientConnected', (info) => {
    console.log(`👋 User ${info.id} joined the chat`);
    users.set(info.id, `User_${info.id.slice(-4)}`);

    // Announce new user
    chatServer.broadcast({
      type: 'system',
      message: `${users.get(info.id)} joined the chat`,
      timestamp: new Date().toISOString()
    });
  });

  chatServer.on('clientDisconnected', (info) => {
    const username = users.get(info.id);
    console.log(`👋 User ${username} left the chat`);
    users.delete(info.id);

    // Announce user left
    chatServer.broadcast({
      type: 'system',
      message: `${username} left the chat`,
      timestamp: new Date().toISOString()
    });
  });

  chatServer.on('message', (message, info) => {
    try {
      if (typeof message === 'object' && message.type === 'chat') {
        const username = users.get(info!.id);
        const chatMessage = {
          type: 'chat',
          from: username,
          message: message.message,
          timestamp: new Date().toISOString()
        };

        console.log(`💬 ${username}: ${message.message}`);

        // Broadcast to all users except sender
        chatServer.broadcast(chatMessage, info!.id);
      }
    } catch (error) {
      console.error('Error handling chat message:', error);
    }
  });

  // Start chat server
  const serverStarted = await chatServer.connect();
  if (!serverStarted) {
    console.error('Failed to start chat server');
    return;
  }

  // Create multiple chat clients
  const clients: AdvancedWebSocketTransport[] = [];
  const clientNames = ['Alice', 'Bob', 'Charlie'];

  for (let i = 0; i < clientNames.length; i++) {
    const client = new AdvancedWebSocketTransport({
      url: 'ws://localhost:8081/chat',
      isServer: false
    });

    client.on('connected', () => {
      console.log(`🔗 ${clientNames[i]} connected to chat`);
    });

    client.on('message', (message) => {
      if (typeof message === 'object') {
        if (message.type === 'chat') {
          console.log(`📨 ${clientNames[i]} received: ${message.from}: ${message.message}`);
        } else if (message.type === 'system') {
          console.log(`📨 ${clientNames[i]} received: ${message.message}`);
        }
      }
    });

    clients.push(client);
  }

  // Connect all clients
  console.log('\n👥 Connecting chat clients...');
  for (let i = 0; i < clients.length; i++) {
    const connected = await clients[i].connect();
    if (connected) {
      console.log(`✅ ${clientNames[i]} connected`);
    } else {
      console.log(`❌ ${clientNames[i]} failed to connect`);
    }
    await new Promise(resolve => setTimeout(resolve, 200));
  }

  // Wait for connections to establish
  await new Promise(resolve => setTimeout(resolve, 1000));

  // Send chat messages
  console.log('\n💬 Starting chat simulation...');

  const messages = [
    { from: 0, message: 'Hello everyone!' },
    { from: 1, message: 'Hi Alice!' },
    { from: 2, message: 'Hey folks!' },
    { from: 0, message: 'How is everyone doing?' },
    { from: 1, message: 'Great! Working on UMICP transport layer' },
    { from: 2, message: 'Awesome project!' }
  ];

  for (const msg of messages) {
    const client = clients[msg.from];
    const sent = client.send({
      type: 'chat',
      message: msg.message
    });
    console.log(`📤 ${clientNames[msg.from]} sent: ${msg.message} (${sent})`);
    await new Promise(resolve => setTimeout(resolve, 500));
  }

  // Show server stats
  console.log('\n📊 Chat server stats:', JSON.stringify(chatServer.getStats(), null, 2));

  // Clean up
  console.log('\n👋 Closing chat room...');
  for (const client of clients) {
    client.disconnect();
  }
  await new Promise(resolve => setTimeout(resolve, 1000));
  chatServer.disconnect();

  console.log('✅ Chat room example completed!\n');
}

async function runTransportExamples() {
  console.log('🚀 UMICP Transport Layer Examples');
  console.log('=================================\n');

  try {
    // Example 1: Basic WebSocket transport
    await demonstrateWebSocketTransport();

    // Example 2: HTTP/2 transport (structure only)
    await demonstrateHTTP2Transport();

    // Example 3: UMICP with WebSocket transport
    await demonstrateUMICPWithTransports();

    // Example 4: Chat room application
    await demonstrateChatRoom();

  } catch (error) {
    console.error('Example failed:', error);
  }

  console.log('🎉 All transport examples completed!');
}

// Run examples if executed directly
if (require.main === module) {
  runTransportExamples().catch(console.error);
}

export { HTTP2Transport, AdvancedWebSocketTransport, runTransportExamples };
