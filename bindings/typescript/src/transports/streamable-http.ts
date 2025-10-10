/**
 * UMICP StreamableHTTP Transport
 * Framework-agnostic implementation with CMMV integration helpers
 */

import { Envelope, OperationType } from '../index.js';
import type { IncomingMessage, ServerResponse } from 'http';

export interface StreamableHTTPServerOptions {
  /**
   * Message handler callback
   */
  onMessage?: (envelope: Envelope, req: IncomingMessage, res: ServerResponse) => Promise<void>;

  /**
   * Connection handler callback
   */
  onConnection?: (req: IncomingMessage) => void;

  /**
   * Disconnection handler callback
   */
  onDisconnection?: (req: IncomingMessage) => void;

  /**
   * Error handler callback
   */
  onError?: (error: Error, req: IncomingMessage, res: ServerResponse) => void;
}

export interface StreamableHTTPClientOptions {
  /**
   * Base URL of the UMICP server
   */
  baseUrl: string;

  /**
   * Path to UMICP endpoint
   * @default '/umicp'
   */
  path?: string;

  /**
   * Request timeout in milliseconds
   * @default 30000
   */
  timeout?: number;

  /**
   * Custom headers
   */
  headers?: Record<string, string>;
}

/**
 * StreamableHTTP Server (Framework Agnostic)
 * Works with Node.js http.IncomingMessage and http.ServerResponse
 */
export class StreamableHTTPServer {
  private handlers: {
    onMessage?: (envelope: Envelope, req: IncomingMessage, res: ServerResponse) => Promise<void>;
    onConnection?: (req: IncomingMessage) => void;
    onDisconnection?: (req: IncomingMessage) => void;
    onError?: (error: Error, req: IncomingMessage, res: ServerResponse) => void;
  };
  private activeConnections: Set<IncomingMessage> = new Set();

  constructor(options: StreamableHTTPServerOptions = {}) {
    this.handlers = {
      onMessage: options.onMessage,
      onConnection: options.onConnection,
      onDisconnection: options.onDisconnection,
      onError: options.onError,
    };
  }

  /**
   * Handle HTTP request (framework agnostic)
   */
  async handleRequest(req: IncomingMessage, res: ServerResponse): Promise<void> {
    try {
      // Track connection
      this.activeConnections.add(req);
      this.handlers.onConnection?.(req);

      // Set streaming headers
      res.setHeader('Content-Type', 'application/json');
      res.setHeader('Cache-Control', 'no-cache');
      res.setHeader('X-UMICP-Version', '1.0');

      // Handle cleanup on close
      req.on('close', () => {
        this.activeConnections.delete(req);
        this.handlers.onDisconnection?.(req);
      });

      // Handle different HTTP methods
      if (req.method === 'POST') {
        // Parse body
        const body = await this.parseBody(req);
        let envelope: Envelope;

        if (typeof body === 'string') {
          envelope = Envelope.deserialize(body);
        } else if (body && typeof body === 'object') {
          envelope = Envelope.deserialize(JSON.stringify(body));
        } else {
          throw new Error('Invalid request body');
        }

        // Validate envelope
        if (!envelope.validate()) {
          throw new Error('Invalid UMICP envelope');
        }

        // Handle message
        if (this.handlers.onMessage) {
          await this.handlers.onMessage(envelope, req, res);
        } else {
          // Default response
          const response = new Envelope({
            from: envelope.getTo(),
            to: envelope.getFrom(),
            operation: OperationType.ACK,
            messageId: `ack-${envelope.getMessageId()}`,
          });

          res.writeHead(200, { 'Content-Type': 'application/json' });
          res.end(response.serialize());
        }
      } else if (req.method === 'GET') {
        // Health check
        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({
          status: 'ok',
          protocol: 'UMICP',
          version: '1.0',
          transport: 'streamable-http',
          activeConnections: this.activeConnections.size,
        }));
      } else {
        res.writeHead(405, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({ error: 'Method not allowed' }));
      }
    } catch (error) {
      const err = error instanceof Error ? error : new Error(String(error));
      this.handlers.onError?.(err, req, res);

      if (!res.headersSent) {
        res.writeHead(400, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify({
          error: err.message,
          protocol: 'UMICP',
          version: '1.0',
        }));
      }
    }
  }

  /**
   * Parse request body
   */
  private parseBody(req: IncomingMessage): Promise<any> {
    return new Promise((resolve, reject) => {
      let body = '';
      req.on('data', chunk => {
        body += chunk.toString();
      });
      req.on('end', () => {
        try {
          resolve(JSON.parse(body));
        } catch {
          resolve(body);
        }
      });
      req.on('error', reject);
    });
  }

  /**
   * Send envelope to specific response
   */
  send(res: ServerResponse, envelope: Envelope): boolean {
    try {
      if (!envelope.validate()) {
        throw new Error('Invalid envelope');
      }

      if (!res.headersSent) {
        res.writeHead(200, { 'Content-Type': 'application/json' });
      }

      res.write(envelope.serialize() + '\n');
      return true;
    } catch (error) {
      console.error('Failed to send envelope:', error);
      return false;
    }
  }

  /**
   * End response
   */
  endResponse(res: ServerResponse): void {
    if (!res.writableEnded) {
      res.end();
    }
  }

  /**
   * Get number of active connections
   */
  getConnectionCount(): number {
    return this.activeConnections.size;
  }
}

/**
 * StreamableHTTP Client
 * Sends UMICP messages over HTTP
 */
export class StreamableHTTPClient {
  private options: Required<StreamableHTTPClientOptions>;
  private connected: boolean = false;

  constructor(options: StreamableHTTPClientOptions) {
    this.options = {
      baseUrl: options.baseUrl,
      path: options.path || '/umicp',
      timeout: options.timeout || 30000,
      headers: options.headers || {},
    };
  }

  /**
   * Connect to server (check health)
   */
  async connect(): Promise<boolean> {
    try {
      const response = await fetch(`${this.options.baseUrl}${this.options.path}`, {
        method: 'GET',
        headers: {
          'Accept': 'application/json',
          ...this.options.headers,
        },
        signal: AbortSignal.timeout(this.options.timeout),
      });

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}`);
      }

      const data = await response.json() as { status: string };
      this.connected = data.status === 'ok';
      return this.connected;
    } catch (error) {
      console.error('Failed to connect:', error);
      this.connected = false;
      return false;
    }
  }

  /**
   * Send envelope and wait for response
   */
  async send(envelope: Envelope): Promise<Envelope> {
    if (!envelope.validate()) {
      throw new Error('Invalid envelope');
    }

    try {
      const response = await fetch(`${this.options.baseUrl}${this.options.path}`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Accept': 'application/json',
          'X-UMICP-Version': '1.0',
          'X-UMICP-From': envelope.getFrom(),
          ...this.options.headers,
        },
        body: envelope.serialize(),
        signal: AbortSignal.timeout(this.options.timeout),
      });

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}: ${await response.text()}`);
      }

      const data = await response.json();
      return Envelope.deserialize(JSON.stringify(data));
    } catch (error) {
      throw new Error(`Failed to send envelope: ${error instanceof Error ? error.message : String(error)}`);
    }
  }

  /**
   * Send envelope without waiting for response
   */
  async sendAsync(envelope: Envelope): Promise<boolean> {
    try {
      await this.send(envelope);
      return true;
    } catch (error) {
      console.error('Failed to send envelope:', error);
      return false;
    }
  }

  /**
   * Check if connected
   */
  isConnected(): boolean {
    return this.connected;
  }

  /**
   * Disconnect (cleanup)
   */
  disconnect(): void {
    this.connected = false;
  }
}

/**
 * CMMV Integration Helper
 * Creates a handler compatible with @cmmv/http routes
 */
export function createUMICPHandler(options: StreamableHTTPServerOptions) {
  const server = new StreamableHTTPServer(options);

  // Return CMMV-compatible handler
  return async (req: any, res: any) => {
    await server.handleRequest(req as IncomingMessage, res as ServerResponse);
  };
}

/**
 * Helper to create UMICP route for CMMV applications
 *
 * Example usage:
 * ```typescript
 * import { createUMICPRoute } from '@hivellm/umicp';
 *
 * const umicpRoute = createUMICPRoute({
 *   path: '/umicp',
 *   onMessage: async (envelope, req, res) => {
 *     // Handle UMICP message
 *   }
 * });
 * ```
 */
export function createUMICPRoute(options: StreamableHTTPServerOptions & { path?: string }) {
  const server = new StreamableHTTPServer(options);

  return {
    path: options.path || '/umicp',
    method: ['GET', 'POST'],
    handler: async (req: IncomingMessage, res: ServerResponse) => {
      await server.handleRequest(req, res);
    },
  };
}

// ============================================================================
// Multiplexed StreamableHTTP Peer
// ============================================================================

import { EventEmitter } from 'events';
import { createServer, Server } from 'http';

export interface HTTPPeerConnection {
  id: string;
  type: 'incoming' | 'outgoing';
  url?: string;
  baseUrl?: string;
  metadata?: Record<string, any>;
  connectedAt: Date;
}

export interface HTTPPeerInfo {
  peerId: string;
  metadata?: Record<string, any>;
}

export interface StreamableHTTPPeerOptions {
  /**
   * Peer ID
   */
  peerId: string;

  /**
   * Server configuration (to accept incoming connections)
   */
  server?: {
    port: number;
    host?: string;
    path?: string;
  };

  /**
   * Metadata to send in hello message
   */
  metadata?: Record<string, any>;

  /**
   * Auto-handle UMICP protocol (hello, ACK, handshake)
   */
  autoProtocol?: boolean;

  /**
   * Handshake timeout in milliseconds
   */
  handshakeTimeout?: number;

  /**
   * Request timeout for outgoing connections
   */
  requestTimeout?: number;
}

export interface StreamableHTTPPeerEvents {
  'data': (envelope: Envelope, peerConnection: HTTPPeerConnection) => void;
  'peer:ready': (peerConnection: HTTPPeerConnection, peerInfo: HTTPPeerInfo) => void;
  'peer:connect': (peerConnection: HTTPPeerConnection) => void;
  'peer:disconnect': (peerConnection: HTTPPeerConnection) => void;
  'error': (error: Error, peerConnection?: HTTPPeerConnection) => void;
}

/**
 * StreamableHTTPPeer - Multiplexed HTTP Transport
 *
 * Can both:
 * - Accept incoming HTTP connections (server mode)
 * - Connect to multiple remote HTTP servers (client mode)
 */
export class StreamableHTTPPeer extends EventEmitter {
  private peerId: string;
  private options: StreamableHTTPPeerOptions;
  private httpServer?: Server;
  private streamServer?: StreamableHTTPServer;

  // Track peer connections
  private peers: Map<string, HTTPPeerConnection> = new Map();
  private peerClients: Map<string, StreamableHTTPClient> = new Map();
  private peerHandshakes: Map<string, HTTPPeerInfo> = new Map();

  constructor(options: StreamableHTTPPeerOptions) {
    super();
    this.peerId = options.peerId;
    this.options = {
      autoProtocol: true,
      handshakeTimeout: 10000,
      requestTimeout: 30000,
      ...options,
    };

    if (this.options.server) {
      this.startServer();
    }
  }

  /**
   * Start HTTP server to accept incoming connections
   */
  private startServer(): void {
    const { port, host = '0.0.0.0', path = '/umicp' } = this.options.server!;

    this.streamServer = new StreamableHTTPServer({
      onMessage: async (envelope, req, res) => {
        await this.handleIncomingMessage(envelope, req, res);
      },
      onError: (error, req) => {
        const from = req.headers['x-umicp-from'] as string;
        const connectionId = from ? `incoming-http-${from}` : this.getConnectionId(req);
        const peer = this.peers.get(connectionId);
        this.emit('error', error, peer);
      },
    });

    this.httpServer = createServer((req, res) => {
      if (req.url?.startsWith(path)) {
        this.streamServer!.handleRequest(req, res);
      } else {
        res.writeHead(404);
        res.end('Not Found');
      }
    });

    this.httpServer.listen(port, host, () => {
      console.log(`[${this.peerId}] 🎧 HTTP Server listening on ${host}:${port}${path}`);
    });

    this.httpServer.on('error', (error) => {
      console.error(`[${this.peerId}] ❌ Error:`, error.message);
      this.emit('error', error);
    });
  }

  /**
   * Handle incoming UMICP message
   */
  private async handleIncomingMessage(
    envelope: Envelope,
    req: IncomingMessage,
    res: ServerResponse
  ): Promise<void> {
    const from = envelope.getFrom();
    const operation = envelope.getOperation();

    // For HTTP, we identify peers by their peerId from the envelope
    // Find or create peer entry
    let connectionId = `incoming-http-${from}`;
    let peer = this.peers.get(connectionId);

    if (!peer) {
      // Create peer on first contact
      peer = {
        id: connectionId,
        type: 'incoming',
        connectedAt: new Date(),
      };
      this.peers.set(connectionId, peer);
      this.emit('peer:connect', peer);
    }

    if (this.options.autoProtocol) {
      // Handle CONTROL (hello)
      if (operation === OperationType.CONTROL) {
        console.log(`[${this.peerId}] 📨 [CONTROL] from ${from}`);

        const caps = envelope.getCapabilities();
        const peerInfo: HTTPPeerInfo = {
          peerId: from,
          metadata: caps,
        };

        this.peerHandshakes.set(connectionId, peerInfo);

        // Send hello back (single response in HTTP)
        const helloResponse = new Envelope({
          from: this.peerId,
          to: from,
          operation: OperationType.CONTROL,
          messageId: `hello-${Date.now()}`,
          capabilities: this.options.metadata || {},
        });

        res.writeHead(200, { 'Content-Type': 'application/json' });
        res.end(helloResponse.serialize());

        console.log(`[${this.peerId}] ✅ Handshake complete with ${from}`);
        console.log(`[${this.peerId}] 📋 Peer metadata:`, peerInfo.metadata);
        this.emit('peer:ready', peer, peerInfo);
        return;
      }

      // Handle ACK
      if (operation === OperationType.ACK) {
        console.log(`[${this.peerId}] 📨 [ACK] from ${from}`);
        res.writeHead(200);
        res.end();
        return;
      }
    }

    // Handle DATA messages
    if (operation === OperationType.DATA) {
      this.emit('data', envelope, peer);

      // Send ACK
      const ackResponse = new Envelope({
        from: this.peerId,
        to: from,
        operation: OperationType.ACK,
        messageId: `ack-${envelope.getMessageId()}`,
      });

      res.writeHead(200, { 'Content-Type': 'application/json' });
      res.end(ackResponse.serialize());
      return;
    }

    // Default response
    res.writeHead(200);
    res.end();
  }

  /**
   * Connect to a remote HTTP peer
   */
  async connectToPeer(url: string, metadata?: Record<string, any>): Promise<string | null> {
    try {
      const peerId = `outgoing-http-${Date.now()}-${Math.random().toString(36).substring(2, 11)}`;

      const client = new StreamableHTTPClient({
        baseUrl: url,
        path: '',
        timeout: this.options.requestTimeout,
      });

      // Test connection
      const connected = await client.connect();
      if (!connected) {
        console.log(`[${this.peerId}] ❌ Failed to connect to ${url}`);
        return null;
      }

      const peer: HTTPPeerConnection = {
        id: peerId,
        type: 'outgoing',
        url,
        baseUrl: url,
        metadata,
        connectedAt: new Date(),
      };

      this.peers.set(peerId, peer);
      this.peerClients.set(peerId, client);

      console.log(`[${this.peerId}] 📤 Connected to: ${url} (${peerId})`);
      this.emit('peer:connect', peer);

      // Send hello if autoProtocol is enabled
      if (this.options.autoProtocol) {
        await this.sendHello(peerId);
      }

      return peerId;
    } catch (error) {
      console.error(`[${this.peerId}] ❌ Error connecting to ${url}:`, error);
      return null;
    }
  }

  /**
   * Send hello message to peer
   */
  private async sendHello(peerId: string): Promise<void> {
    const client = this.peerClients.get(peerId);
    const peer = this.peers.get(peerId);

    if (!client || !peer) return;

    const helloEnvelope = new Envelope({
      from: this.peerId,
      to: 'unknown', // Will be set by remote peer
      operation: OperationType.CONTROL,
      messageId: `hello-${Date.now()}`,
      capabilities: this.options.metadata || {},
    });

    try {
      const response = await client.send(helloEnvelope);

      if (response.getOperation() === OperationType.CONTROL) {
        const remotePeerId = response.getFrom();
        const peerInfo: HTTPPeerInfo = {
          peerId: remotePeerId,
          metadata: response.getCapabilities(),
        };

        this.peerHandshakes.set(peerId, peerInfo);
        peer.metadata = peerInfo.metadata;

        console.log(`[${this.peerId}] 📨 [CONTROL] from ${remotePeerId}`);
        console.log(`[${this.peerId}] ✅ Handshake complete with ${remotePeerId}`);
        console.log(`[${this.peerId}] 📋 Peer metadata:`, peerInfo.metadata);

        this.emit('peer:ready', peer, peerInfo);
      } else {
        throw new Error(`Unexpected response operation: ${response.getOperation()}`);
      }
    } catch (error) {
      console.error(`[${this.peerId}] ❌ Hello handshake failed:`, error);
      this.emit('error', error as Error, peer);

      // Remove failed peer
      this.peers.delete(peerId);
      this.peerClients.delete(peerId);
    }
  }

  /**
   * Send envelope to specific peer
   */
  async sendToPeer(peerIdOrName: string, envelope: Envelope): Promise<boolean> {
    // Find peer by ID or by handshake name
    let targetPeerId: string | undefined;

    for (const [id, peerInfo] of this.peerHandshakes.entries()) {
      if (id === peerIdOrName || peerInfo.peerId === peerIdOrName) {
        targetPeerId = id;
        break;
      }
    }

    if (!targetPeerId) {
      console.warn(`[${this.peerId}] ❌ Peer not found: ${peerIdOrName}`);
      return false;
    }

    const client = this.peerClients.get(targetPeerId);
    if (!client) {
      console.warn(`[${this.peerId}] ❌ No client for peer: ${peerIdOrName}`);
      return false;
    }

    try {
      await client.send(envelope);
      return true;
    } catch (error) {
      console.error(`[${this.peerId}] ❌ Failed to send to ${peerIdOrName}:`, error);
      return false;
    }
  }

  /**
   * Broadcast envelope to all connected peers
   */
  async broadcast(envelope: Envelope): Promise<number> {
    let sent = 0;

    for (const [peerId, client] of this.peerClients.entries()) {
      try {
        await client.send(envelope);
        sent++;
      } catch (error) {
        console.error(`[${this.peerId}] ❌ Failed to broadcast to ${peerId}:`, error);
      }
    }

    return sent;
  }

  /**
   * Disconnect from specific peer
   */
  disconnectPeer(peerIdOrName: string): boolean {
    // Find peer
    let targetPeerId: string | undefined;

    for (const [id, peerInfo] of this.peerHandshakes.entries()) {
      if (id === peerIdOrName || peerInfo.peerId === peerIdOrName) {
        targetPeerId = id;
        break;
      }
    }

    if (!targetPeerId) {
      return false;
    }

    const client = this.peerClients.get(targetPeerId);
    const peer = this.peers.get(targetPeerId);

    if (client) {
      client.disconnect();
      this.peerClients.delete(targetPeerId);
    }

    if (peer) {
      this.peers.delete(targetPeerId);
      this.peerHandshakes.delete(targetPeerId);
      this.emit('peer:disconnect', peer);
      return true;
    }

    return false;
  }

  /**
   * Get all connected peers
   */
  getPeers(): HTTPPeerConnection[] {
    return Array.from(this.peers.values());
  }

  /**
   * Get peer info by ID or name
   */
  getPeerInfo(peerIdOrName: string): HTTPPeerInfo | undefined {
    for (const [id, peerInfo] of this.peerHandshakes.entries()) {
      if (id === peerIdOrName || peerInfo.peerId === peerIdOrName) {
        return peerInfo;
      }
    }
    return undefined;
  }

  /**
   * Get connection ID from request
   */
  private getConnectionId(req: IncomingMessage): string {
    // Use socket remote address + port as unique ID
    const socket = req.socket;
    return `incoming-http-${socket.remoteAddress}:${socket.remotePort}`;
  }

  /**
   * Close server and disconnect all peers
   */
  async close(): Promise<void> {
    // Disconnect all outgoing peers
    for (const [peerId] of this.peerClients.entries()) {
      this.disconnectPeer(peerId);
    }

    // Close server
    if (this.httpServer) {
      return new Promise((resolve) => {
        this.httpServer!.close(() => {
          console.log(`[${this.peerId}] 🔌 HTTP Server closed`);
          resolve();
        });
      });
    }
  }
}
