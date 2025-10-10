/**
 * UMICP WebSocket Transport (High-Level)
 * Built on top of native WebSocket bindings
 * Integration with @cmmv/server
 *
 * Multiplexed Architecture: Each peer can BOTH receive incoming connections
 * AND connect to multiple remote peers simultaneously.
 */

import { WebSocketTransport as NativeWebSocketTransport, Envelope, OperationType } from '../index.js';
import type { Server as HTTPServer } from 'http';
import { WebSocketServer, WebSocket } from 'ws';
import { EventEmitter } from 'events';

export interface UMICPWebSocketServerOptions {
  /**
   * HTTP server to attach to
   */
  httpServer?: HTTPServer;

  /**
   * Port to listen on (if no httpServer provided)
   */
  port?: number;

  /**
   * Path to mount WebSocket endpoint
   * @default '/umicp'
   */
  path?: string;

  /**
   * Enable per-message deflate compression
   * @default true
   */
  compression?: boolean;

  /**
   * Maximum message size in bytes
   * @default 100MB
   */
  maxPayload?: number;

  /**
   * Ping interval in milliseconds
   * @default 30000
   */
  pingInterval?: number;

  /**
   * Connection timeout in milliseconds
   * @default 60000
   */
  connectionTimeout?: number;

  /**
   * Message handler callback
   */
  onMessage?: (envelope: Envelope, client: UMICPWebSocketClient) => Promise<void>;

  /**
   * Connection handler callback
   */
  onConnection?: (client: UMICPWebSocketClient) => void;

  /**
   * Disconnection handler callback
   */
  onDisconnection?: (client: UMICPWebSocketClient) => void;

  /**
   * Error handler callback
   */
  onError?: (error: Error, client?: UMICPWebSocketClient) => void;
}

export interface UMICPWebSocketClientOptions {
  /**
   * WebSocket URL
   */
  url: string;

  /**
   * Enable compression
   * @default true
   */
  compression?: boolean;

  /**
   * Connection timeout in milliseconds
   * @default 10000
   */
  connectionTimeout?: number;

  /**
   * Reconnect automatically on disconnect
   * @default true
   */
  autoReconnect?: boolean;

  /**
   * Reconnect delay in milliseconds
   * @default 5000
   */
  reconnectDelay?: number;

  /**
   * Max reconnect attempts (0 = unlimited)
   * @default 0
   */
  maxReconnectAttempts?: number;

  /**
   * Custom headers
   */
  headers?: Record<string, string>;

  /**
   * Message handler callback
   */
  onMessage?: (envelope: Envelope) => Promise<void>;

  /**
   * Connection handler callback
   */
  onConnect?: () => void;

  /**
   * Disconnection handler callback
   */
  onDisconnect?: () => void;

  /**
   * Error handler callback
   */
  onError?: (error: Error) => void;
}

/**
 * UMICP WebSocket Client (High-Level)
 */
export class UMICPWebSocketClient {
  private ws: WebSocket | null = null;
  private options: Required<Omit<UMICPWebSocketClientOptions, 'headers' | 'onMessage' | 'onConnect' | 'onDisconnect' | 'onError'>>;
  private handlers: {
    onMessage?: (envelope: Envelope) => Promise<void>;
    onConnect?: () => void;
    onDisconnect?: () => void;
    onError?: (error: Error) => void;
  };
  private connected: boolean = false;
  private reconnectAttempts: number = 0;
  private reconnectTimer: NodeJS.Timeout | null = null;
  public id: string;
  public metadata: Record<string, any> = {};

  constructor(options: UMICPWebSocketClientOptions) {
    this.options = {
      url: options.url,
      compression: options.compression ?? true,
      connectionTimeout: options.connectionTimeout || 10000,
      autoReconnect: options.autoReconnect ?? true,
      reconnectDelay: options.reconnectDelay || 5000,
      maxReconnectAttempts: options.maxReconnectAttempts || 0,
    };

    this.handlers = {
      onMessage: options.onMessage,
      onConnect: options.onConnect,
      onDisconnect: options.onDisconnect,
      onError: options.onError,
    };

    this.id = `client-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
  }

  /**
   * Connect to WebSocket server
   */
  async connect(): Promise<boolean> {
    return new Promise((resolve) => {
      try {
        this.ws = new WebSocket(this.options.url, {
          perMessageDeflate: this.options.compression,
          handshakeTimeout: this.options.connectionTimeout,
        });

        const timeout = setTimeout(() => {
          if (!this.connected) {
            this.ws?.terminate();
            resolve(false);
          }
        }, this.options.connectionTimeout);

        this.ws.on('open', () => {
          clearTimeout(timeout);
          this.connected = true;
          this.reconnectAttempts = 0;
          this.handlers.onConnect?.();
          resolve(true);
        });

        this.ws.on('message', async (data: Buffer) => {
          try {
            const message = data.toString();
            const envelope = Envelope.deserialize(message);

            if (envelope.validate()) {
              await this.handlers.onMessage?.(envelope);
            } else {
              throw new Error('Invalid envelope received');
            }
          } catch (error) {
            this.handlers.onError?.(error instanceof Error ? error : new Error(String(error)));
          }
        });

        this.ws.on('close', () => {
          this.connected = false;
          this.handlers.onDisconnect?.();

          if (this.options.autoReconnect) {
            this.scheduleReconnect();
          }
        });

        this.ws.on('error', (error) => {
          this.handlers.onError?.(error);
          resolve(false);
        });
      } catch (error) {
        this.handlers.onError?.(error instanceof Error ? error : new Error(String(error)));
        resolve(false);
      }
    });
  }

  /**
   * Schedule reconnection
   */
  private scheduleReconnect(): void {
    if (this.options.maxReconnectAttempts > 0 && this.reconnectAttempts >= this.options.maxReconnectAttempts) {
      console.error('Max reconnect attempts reached');
      return;
    }

    this.reconnectTimer = setTimeout(async () => {
      this.reconnectAttempts++;
      console.log(`Reconnecting... (attempt ${this.reconnectAttempts})`);
      await this.connect();
    }, this.options.reconnectDelay);
  }

  /**
   * Send UMICP envelope
   */
  send(envelope: Envelope): boolean {
    if (!this.connected || !this.ws) {
      return false;
    }

    if (!envelope.validate()) {
      return false;
    }

    try {
      this.ws.send(envelope.serialize());
      return true;
    } catch (error) {
      this.handlers.onError?.(error instanceof Error ? error : new Error(String(error)));
      return false;
    }
  }

  /**
   * Send and wait for response
   */
  async sendAndWait(envelope: Envelope, timeout: number = 30000): Promise<Envelope> {
    return new Promise((resolve, reject) => {
      const messageId = envelope.getMessageId();
      const timer = setTimeout(() => {
        reject(new Error('Response timeout'));
      }, timeout);

      const originalHandler = this.handlers.onMessage;
      this.handlers.onMessage = async (responseEnvelope: Envelope) => {
        // Check if this is a response to our message
        const caps = responseEnvelope.getCapabilities();
        if (caps['reply_to'] === messageId || responseEnvelope.getMessageId().includes(messageId)) {
          clearTimeout(timer);
          this.handlers.onMessage = originalHandler;
          resolve(responseEnvelope);
        } else if (originalHandler) {
          await originalHandler(responseEnvelope);
        }
      };

      if (!this.send(envelope)) {
        clearTimeout(timer);
        this.handlers.onMessage = originalHandler;
        reject(new Error('Failed to send envelope'));
      }
    });
  }

  /**
   * Disconnect from server
   */
  disconnect(): void {
    if (this.reconnectTimer) {
      clearTimeout(this.reconnectTimer);
      this.reconnectTimer = null;
    }

    this.options.autoReconnect = false;
    this.ws?.close();
    this.ws = null;
    this.connected = false;
  }

  /**
   * Check if connected
   */
  isConnected(): boolean {
    return this.connected;
  }

  /**
   * Get connection statistics
   */
  getStats() {
    return {
      connected: this.connected,
      reconnectAttempts: this.reconnectAttempts,
      url: this.options.url,
    };
  }
}

/**
 * UMICP WebSocket Server (High-Level)
 */
export class UMICPWebSocketServer {
  private wss: WebSocketServer;
  private clients: Map<string, UMICPWebSocketClient> = new Map();
  private options: Required<Omit<UMICPWebSocketServerOptions, 'httpServer' | 'port' | 'onMessage' | 'onConnection' | 'onDisconnection' | 'onError'>>;
  private handlers: {
    onMessage?: (envelope: Envelope, client: UMICPWebSocketClient) => Promise<void>;
    onConnection?: (client: UMICPWebSocketClient) => void;
    onDisconnection?: (client: UMICPWebSocketClient) => void;
    onError?: (error: Error, client?: UMICPWebSocketClient) => void;
  };

  constructor(options: UMICPWebSocketServerOptions) {
    this.options = {
      path: options.path || '/umicp',
      compression: options.compression ?? true,
      maxPayload: options.maxPayload || 100 * 1024 * 1024, // 100MB
      pingInterval: options.pingInterval || 30000,
      connectionTimeout: options.connectionTimeout || 60000,
    };

    this.handlers = {
      onMessage: options.onMessage,
      onConnection: options.onConnection,
      onDisconnection: options.onDisconnection,
      onError: options.onError,
    };

    // Create WebSocket server
    const serverOptions: any = {
      path: this.options.path,
      perMessageDeflate: this.options.compression,
      maxPayload: this.options.maxPayload,
      clientTracking: true,
    };

    if (options.httpServer) {
      serverOptions.server = options.httpServer;
    } else if (options.port) {
      serverOptions.port = options.port;
    } else {
      throw new Error('Either httpServer or port must be provided');
    }

    this.wss = new WebSocketServer(serverOptions);

    // Setup connection handler
    this.wss.on('connection', (ws: WebSocket) => {
      this.handleConnection(ws);
    });

    this.wss.on('error', (error) => {
      this.handlers.onError?.(error);
    });
  }

  /**
   * Handle new WebSocket connection
   */
  private handleConnection(ws: WebSocket): void {
    const clientId = `client-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;

    // Create high-level client wrapper
    const client = new UMICPWebSocketClient({
      url: '', // Not used for server-side client
      onMessage: async (envelope: Envelope) => {
        await this.handlers.onMessage?.(envelope, client);
      },
      onDisconnect: () => {
        this.clients.delete(clientId);
        this.handlers.onDisconnection?.(client);
      },
      onError: (error: Error) => {
        this.handlers.onError?.(error, client);
      },
    });

    client.id = clientId;
    (client as any).ws = ws; // Inject WebSocket connection
    (client as any).connected = true;

    this.clients.set(clientId, client);
    this.handlers.onConnection?.(client);

    // Setup ping/pong for keep-alive
    const pingInterval = setInterval(() => {
      if (ws.readyState === WebSocket.OPEN) {
        ws.ping();
      } else {
        clearInterval(pingInterval);
      }
    }, this.options.pingInterval);

    ws.on('message', async (data: Buffer) => {
      try {
        const message = data.toString();
        const envelope = Envelope.deserialize(message);

        if (envelope.validate()) {
          await this.handlers.onMessage?.(envelope, client);
        } else {
          throw new Error('Invalid envelope received');
        }
      } catch (error) {
        this.handlers.onError?.(error instanceof Error ? error : new Error(String(error)), client);
      }
    });

    ws.on('close', () => {
      clearInterval(pingInterval);
      this.clients.delete(clientId);
      this.handlers.onDisconnection?.(client);
    });

    ws.on('error', (error) => {
      this.handlers.onError?.(error, client);
    });
  }

  /**
   * Send envelope to specific client
   */
  sendToClient(clientId: string, envelope: Envelope): boolean {
    const client = this.clients.get(clientId);
    if (!client) {
      return false;
    }
    return client.send(envelope);
  }

  /**
   * Broadcast envelope to all clients
   */
  broadcast(envelope: Envelope, excludeClientId?: string): number {
    let sent = 0;
    for (const [clientId, client] of this.clients) {
      if (excludeClientId && clientId === excludeClientId) {
        continue;
      }
      if (client.send(envelope)) {
        sent++;
      }
    }
    return sent;
  }

  /**
   * Get all connected clients
   */
  getClients(): UMICPWebSocketClient[] {
    return Array.from(this.clients.values());
  }

  /**
   * Get client by ID
   */
  getClient(clientId: string): UMICPWebSocketClient | undefined {
    return this.clients.get(clientId);
  }

  /**
   * Get server statistics
   */
  getStats() {
    return {
      connectedClients: this.clients.size,
      path: this.options.path,
      compression: this.options.compression,
      maxPayload: this.options.maxPayload,
    };
  }

  /**
   * Close server
   */
  close(): Promise<void> {
    return new Promise((resolve) => {
      // Disconnect all clients
      for (const client of this.clients.values()) {
        client.disconnect();
      }
      this.clients.clear();

      // Close WebSocket server
      this.wss.close(() => {
        resolve();
      });
    });
  }
}

/**
 * Helper to integrate with CMMV @cmmv/server
 */
export function createUMICPWebSocketServer(
  httpServer: HTTPServer,
  options: Omit<UMICPWebSocketServerOptions, 'httpServer'>
): UMICPWebSocketServer {
  return new UMICPWebSocketServer({
    ...options,
    httpServer,
  });
}

// ============================================================================
// MULTIPLEXED PEER ARCHITECTURE
// ============================================================================

/**
 * Connection metadata for tracking peers
 */
export interface PeerConnection {
  id: string;
  type: 'incoming' | 'outgoing';
  url?: string;
  client: UMICPWebSocketClient;
  metadata: Record<string, any>;
  connectedAt: Date;
  handshakeComplete?: boolean;
  peerInfo?: PeerInfo;
  handshakeTimer?: NodeJS.Timeout;
}

/**
 * Configuration for multiplexed peer
 */
export interface UMICPWebSocketPeerOptions {
  /**
   * Peer identifier (unique name for this agent/tool)
   */
  peerId: string;

  /**
   * Server configuration (for receiving incoming connections)
   */
  server?: {
    httpServer?: HTTPServer;
    port?: number;
    path?: string;
    compression?: boolean;
    maxPayload?: number;
    pingInterval?: number;
  };

  /**
   * Peer metadata sent in hello message
   */
  metadata?: Record<string, string>;

  /**
   * Auto-handle UMICP protocol (hello, ACK, handshake)
   * @default true
   */
  autoProtocol?: boolean;

  /**
   * Timeout for handshake completion (ms)
   * @default 10000
   */
  handshakeTimeout?: number;
}

/**
 * Peer state after handshake
 */
export interface PeerInfo {
  peerId: string;
  metadata: Record<string, string>;
  capabilities: Record<string, string>;
  handshakeCompletedAt: Date;
}

/**
 * Event types emitted by UMICPWebSocketPeer
 */
export interface UMICPWebSocketPeerEvents {
  /**
   * Emitted when DATA message is received (application-level messages only)
   * Protocol messages (CONTROL, ACK) are handled automatically
   * @event data
   */
  data: (envelope: Envelope, peer: PeerConnection) => void;

  /**
   * Emitted when any message is received (including protocol messages)
   * Use this for debugging/logging. Use 'data' for application logic.
   * @event message
   */
  message: (envelope: Envelope, peer: PeerConnection) => void;

  /**
   * Emitted when peer completes handshake successfully
   * This is when the peer is truly ready for communication
   * @event peer:ready
   */
  'peer:ready': (peer: PeerConnection, peerInfo: PeerInfo) => void;

  /**
   * Emitted when peer connects (socket level)
   * Handshake may not be complete yet
   * @event peer:connect
   */
  'peer:connect': (peer: PeerConnection) => void;

  /**
   * Emitted when any peer disconnects
   * @event peer:disconnect
   */
  'peer:disconnect': (peer: PeerConnection) => void;

  /**
   * Emitted when handshake times out
   * @event peer:handshake-timeout
   */
  'peer:handshake-timeout': (peer: PeerConnection) => void;

  /**
   * Emitted when an error occurs
   * @event error
   */
  error: (error: Error, peer?: PeerConnection) => void;

  /**
   * Emitted when server is ready and listening
   * @event server:ready
   */
  'server:ready': () => void;

  /**
   * Emitted when successfully connected to a remote peer (outgoing)
   * @event connected
   */
  connected: (peerId: string, url: string) => void;
}

/**
 * Multiplexed WebSocket Peer
 *
 * This class represents a peer that can BOTH:
 * 1. Accept incoming connections from other agents/tools (acts as server)
 * 2. Connect to multiple remote agents/tools (acts as multiple clients)
 *
 * All connections are managed in a unified peer map, allowing seamless
 * bidirectional communication regardless of connection direction.
 *
 * @extends EventEmitter
 * @fires UMICPWebSocketPeer#message
 * @fires UMICPWebSocketPeer#peer:connect
 * @fires UMICPWebSocketPeer#peer:disconnect
 * @fires UMICPWebSocketPeer#error
 * @fires UMICPWebSocketPeer#server:ready
 * @fires UMICPWebSocketPeer#connected
 */
export class UMICPWebSocketPeer extends EventEmitter {
  private peerId: string;
  private server: UMICPWebSocketServer | null = null;
  private peers: Map<string, PeerConnection> = new Map();
  private options: Required<Omit<UMICPWebSocketPeerOptions, 'server'>>;
  private serverOptions?: UMICPWebSocketPeerOptions['server'];
  private peerMetadata: Record<string, string>;

  constructor(options: UMICPWebSocketPeerOptions) {
    super();
    this.peerId = options.peerId;
    this.peerMetadata = options.metadata || {};
    this.serverOptions = options.server;

    this.options = {
      peerId: options.peerId,
      metadata: options.metadata || {},
      autoProtocol: options.autoProtocol ?? true,
      handshakeTimeout: options.handshakeTimeout || 10000,
    };

    // Initialize server if configuration provided
    if (options.server) {
      this.initializeServer();
    }
  }

  /**
   * Handle incoming message with automatic protocol handling
   */
  private async handleMessage(envelope: Envelope, peer: PeerConnection): Promise<void> {
    // Emit raw message event for debugging/logging
    this.emit('message', envelope, peer);

    const operation = envelope.getOperation();

    // If autoProtocol is disabled, just emit data and return
    if (!this.options.autoProtocol) {
      this.emit('data', envelope, peer);
      return;
    }

    // Handle protocol messages automatically
    switch (operation) {
      case OperationType.CONTROL:
        await this.handleControlMessage(envelope, peer);
        break;

      case OperationType.ACK:
        await this.handleAckMessage(envelope, peer);
        break;

      case OperationType.DATA:
        // Only emit data events for DATA operation type
        this.emit('data', envelope, peer);
        break;

      case OperationType.ERROR:
        const errorMsg = envelope.getCapabilities()['error'] || 'Unknown error';
        this.emit('error', new Error(errorMsg), peer);
        break;
    }
  }

  /**
   * Handle CONTROL messages (hello, handshake)
   */
  private async handleControlMessage(envelope: Envelope, peer: PeerConnection): Promise<void> {
    const caps = envelope.getCapabilities();
    const from = envelope.getFrom();

    // Extract peer info from hello message
    const remotePeerInfo: PeerInfo = {
      peerId: from,
      metadata: {},
      capabilities: caps,
      handshakeCompletedAt: new Date(),
    };

    // Parse metadata if present
    Object.keys(caps).forEach(key => {
      if (key.startsWith('meta_')) {
        remotePeerInfo.metadata[key.substring(5)] = caps[key];
      }
    });

    // Store peer info
    peer.peerInfo = remotePeerInfo;

    // Clear handshake timeout
    if (peer.handshakeTimer) {
      clearTimeout(peer.handshakeTimer);
      peer.handshakeTimer = undefined;
    }

    // Send ACK response
    const ackEnvelope = new Envelope({
      from: this.peerId,
      to: from,
      operation: OperationType.ACK,
      messageId: `ack-${envelope.getMessageId()}`,
      capabilities: {
        status: 'handshake_complete',
        peerId: this.peerId,
        ...this.buildCapabilities(),
      }
    });

    peer.client.send(ackEnvelope);

    // Mark handshake as complete
    if (!peer.handshakeComplete) {
      peer.handshakeComplete = true;
      console.log(`[${this.peerId}] ✅ Handshake complete with ${from}`);
      this.emit('peer:ready', peer, remotePeerInfo);
    }
  }

  /**
   * Handle ACK messages
   */
  private async handleAckMessage(envelope: Envelope, peer: PeerConnection): Promise<void> {
    const caps = envelope.getCapabilities();
    const from = envelope.getFrom();

    if (caps['status'] === 'handshake_complete') {
      // Extract peer info
      const remotePeerInfo: PeerInfo = {
        peerId: caps['peerId'] || from,
        metadata: {},
        capabilities: caps,
        handshakeCompletedAt: new Date(),
      };

      // Parse metadata
      Object.keys(caps).forEach(key => {
        if (key.startsWith('meta_')) {
          remotePeerInfo.metadata[key.substring(5)] = caps[key];
        }
      });

      // Store peer info
      peer.peerInfo = remotePeerInfo;

      // Clear handshake timeout
      if (peer.handshakeTimer) {
        clearTimeout(peer.handshakeTimer);
        peer.handshakeTimer = undefined;
      }

      // Mark handshake as complete
      if (!peer.handshakeComplete) {
        peer.handshakeComplete = true;
        console.log(`[${this.peerId}] ✅ Handshake complete with ${from}`);
        this.emit('peer:ready', peer, remotePeerInfo);
      }
    }
  }

  /**
   * Send hello message to peer
   */
  private sendHelloMessage(peer: PeerConnection): void {
    const helloEnvelope = new Envelope({
      from: this.peerId,
      to: 'peer',
      operation: OperationType.CONTROL,
      messageId: `hello-${Date.now()}`,
      capabilities: {
        type: 'hello',
        version: '1.0.0',
        peerId: this.peerId,
        ...this.buildCapabilities(),
      }
    });

    peer.client.send(helloEnvelope);

    // Setup handshake timeout
    if (this.options.autoProtocol && this.options.handshakeTimeout > 0) {
      peer.handshakeTimer = setTimeout(() => {
        if (!peer.handshakeComplete) {
          console.warn(`[${this.peerId}] ⚠️  Handshake timeout with ${peer.id}`);
          this.emit('peer:handshake-timeout', peer);

          // Optionally disconnect on timeout
          // this.disconnectPeer(peer.id);
        }
      }, this.options.handshakeTimeout);
    }
  }

  /**
   * Build capabilities object from metadata
   */
  private buildCapabilities(): Record<string, string> {
    const caps: Record<string, string> = {};

    Object.entries(this.peerMetadata).forEach(([key, value]) => {
      caps[`meta_${key}`] = value;
    });

    return caps;
  }

  /**
   * Initialize the server component for accepting incoming connections
   */
  private initializeServer(): void {
    const serverConfig = this.serverOptions!;

    this.server = new UMICPWebSocketServer({
      httpServer: serverConfig.httpServer,
      port: serverConfig.port,
      path: serverConfig.path || '/umicp',
      compression: serverConfig.compression ?? true,
      maxPayload: serverConfig.maxPayload,
      pingInterval: serverConfig.pingInterval,

      onConnection: (client: UMICPWebSocketClient) => {
        const peerId = `incoming-${client.id}`;

        const peerConnection: PeerConnection = {
          id: peerId,
          type: 'incoming',
          client,
          metadata: client.metadata,
          connectedAt: new Date(),
          handshakeComplete: false,
        };

        this.peers.set(peerId, peerConnection);

        console.log(`[${this.peerId}] 📥 Incoming connection: ${peerId}`);
        this.emit('peer:connect', peerConnection);

        // Send hello message if autoProtocol is enabled
        if (this.options.autoProtocol) {
          this.sendHelloMessage(peerConnection);
        }
      },

      onMessage: async (envelope: Envelope, client: UMICPWebSocketClient) => {
        const peerId = `incoming-${client.id}`;
        const peer = this.peers.get(peerId);

        if (peer) {
          await this.handleMessage(envelope, peer);
        }
      },

      onDisconnection: (client: UMICPWebSocketClient) => {
        const peerId = `incoming-${client.id}`;
        const peer = this.peers.get(peerId);

        if (peer) {
          // Clear handshake timeout if exists
          if (peer.handshakeTimer) {
            clearTimeout(peer.handshakeTimer);
          }

          console.log(`[${this.peerId}] 📤 Disconnected: ${peerId}`);
          this.peers.delete(peerId);
          this.emit('peer:disconnect', peer);
        }
      },

      onError: (error: Error, client?: UMICPWebSocketClient) => {
        const peerId = client ? `incoming-${client.id}` : undefined;
        const peer = peerId ? this.peers.get(peerId) : undefined;
        this.emit('error', error, peer);
      },
    });

    // Emit server ready event
    setImmediate(() => this.emit('server:ready'));
  }

  /**
   * Connect to a remote peer (outgoing connection)
   *
   * @param url WebSocket URL of the remote peer
   * @param metadata Optional metadata to attach to this connection
   * @returns Promise<string> - Peer ID if successful, null otherwise
   */
  async connectToPeer(
    url: string,
    metadata: Record<string, any> = {}
  ): Promise<string | null> {
    const client = new UMICPWebSocketClient({
      url,
      compression: true,
      autoReconnect: true,
      reconnectDelay: 5000,

      onConnect: () => {
        const peerId = `outgoing-${client.id}`;

        const peerConnection: PeerConnection = {
          id: peerId,
          type: 'outgoing',
          url,
          client,
          metadata,
          connectedAt: new Date(),
          handshakeComplete: false,
        };

        this.peers.set(peerId, peerConnection);

        console.log(`[${this.peerId}] 📤 Connected to: ${url} (${peerId})`);
        this.emit('peer:connect', peerConnection);
        this.emit('connected', peerId, url);

        // Send hello message if autoProtocol is enabled
        if (this.options.autoProtocol) {
          this.sendHelloMessage(peerConnection);
        }
      },

      onMessage: async (envelope: Envelope) => {
        const peerId = `outgoing-${client.id}`;
        const peer = this.peers.get(peerId);

        if (peer) {
          await this.handleMessage(envelope, peer);
        }
      },

      onDisconnect: () => {
        const peerId = `outgoing-${client.id}`;
        const peer = this.peers.get(peerId);

        if (peer) {
          // Clear handshake timeout if exists
          if (peer.handshakeTimer) {
            clearTimeout(peer.handshakeTimer);
          }

          console.log(`[${this.peerId}] 🔌 Disconnected from: ${url} (${peerId})`);
          this.peers.delete(peerId);
          this.emit('peer:disconnect', peer);
        }
      },

      onError: (error: Error) => {
        const peerId = `outgoing-${client.id}`;
        const peer = this.peers.get(peerId);
        this.emit('error', error, peer);
      },
    });

    const connected = await client.connect();
    if (connected) {
      return `outgoing-${client.id}`;
    }

    return null;
  }

  /**
   * Send envelope to a specific peer by ID
   */
  sendToPeer(peerId: string, envelope: Envelope): boolean {
    const peer = this.peers.get(peerId);
    if (!peer) {
      console.error(`[${this.peerId}] Peer not found: ${peerId}`);
      return false;
    }

    return peer.client.send(envelope);
  }

  /**
   * Send envelope to a specific peer by URL (for outgoing connections)
   */
  sendToPeerByUrl(url: string, envelope: Envelope): boolean {
    for (const peer of this.peers.values()) {
      if (peer.type === 'outgoing' && peer.url === url) {
        return peer.client.send(envelope);
      }
    }

    console.error(`[${this.peerId}] No outgoing connection found for URL: ${url}`);
    return false;
  }

  /**
   * Broadcast envelope to all connected peers
   */
  broadcast(envelope: Envelope, excludePeerId?: string): number {
    let sent = 0;

    for (const [peerId, peer] of this.peers) {
      if (excludePeerId && peerId === excludePeerId) {
        continue;
      }

      if (peer.client.send(envelope)) {
        sent++;
      }
    }

    console.log(`[${this.peerId}] 📢 Broadcasted to ${sent} peer(s)`);
    return sent;
  }

  /**
   * Broadcast to all peers of a specific type
   */
  broadcastToType(
    envelope: Envelope,
    type: 'incoming' | 'outgoing',
    excludePeerId?: string
  ): number {
    let sent = 0;

    for (const [peerId, peer] of this.peers) {
      if (peer.type !== type) {
        continue;
      }

      if (excludePeerId && peerId === excludePeerId) {
        continue;
      }

      if (peer.client.send(envelope)) {
        sent++;
      }
    }

    console.log(`[${this.peerId}] 📢 Broadcasted to ${sent} ${type} peer(s)`);
    return sent;
  }

  /**
   * Send and wait for response from a specific peer
   */
  async sendAndWait(
    peerId: string,
    envelope: Envelope,
    timeout: number = 30000
  ): Promise<Envelope> {
    const peer = this.peers.get(peerId);
    if (!peer) {
      throw new Error(`Peer not found: ${peerId}`);
    }

    return peer.client.sendAndWait(envelope, timeout);
  }

  /**
   * Get all connected peers
   */
  getPeers(): PeerConnection[] {
    return Array.from(this.peers.values());
  }

  /**
   * Get peers by type
   */
  getPeersByType(type: 'incoming' | 'outgoing'): PeerConnection[] {
    return Array.from(this.peers.values()).filter(peer => peer.type === type);
  }

  /**
   * Get specific peer by ID
   */
  getPeer(peerId: string): PeerConnection | undefined {
    return this.peers.get(peerId);
  }

  /**
   * Find peer by metadata match
   */
  findPeerByMetadata(key: string, value: any): PeerConnection | undefined {
    for (const peer of this.peers.values()) {
      if (peer.metadata[key] === value) {
        return peer;
      }
    }
    return undefined;
  }

  /**
   * Disconnect from a specific peer
   */
  disconnectPeer(peerId: string): boolean {
    const peer = this.peers.get(peerId);
    if (!peer) {
      return false;
    }

    peer.client.disconnect();
    this.peers.delete(peerId);

    console.log(`[${this.peerId}] 🔌 Manually disconnected: ${peerId}`);
    return true;
  }

  /**
   * Disconnect from a peer by URL
   */
  disconnectPeerByUrl(url: string): boolean {
    for (const [peerId, peer] of this.peers) {
      if (peer.type === 'outgoing' && peer.url === url) {
        peer.client.disconnect();
        this.peers.delete(peerId);
        console.log(`[${this.peerId}] 🔌 Manually disconnected: ${url}`);
        return true;
      }
    }
    return false;
  }

  /**
   * Get connection statistics
   */
  getStats() {
    const incoming = this.getPeersByType('incoming');
    const outgoing = this.getPeersByType('outgoing');

    return {
      peerId: this.peerId,
      totalPeers: this.peers.size,
      incomingConnections: incoming.length,
      outgoingConnections: outgoing.length,
      serverActive: this.server !== null,
      serverStats: this.server?.getStats(),
      peers: Array.from(this.peers.entries()).map(([id, peer]) => ({
        id,
        type: peer.type,
        url: peer.url,
        connectedAt: peer.connectedAt,
        metadata: peer.metadata,
      })),
    };
  }

  /**
   * Close all connections and shutdown
   */
  async shutdown(): Promise<void> {
    console.log(`[${this.peerId}] 🛑 Shutting down...`);

    // Disconnect all peers
    for (const [peerId, peer] of this.peers) {
      peer.client.disconnect();
      console.log(`[${this.peerId}] 🔌 Disconnected: ${peerId}`);
    }
    this.peers.clear();

    // Close server if active
    if (this.server) {
      await this.server.close();
      console.log(`[${this.peerId}] 🛑 Server closed`);
    }

    console.log(`[${this.peerId}] ✅ Shutdown complete`);
  }
}

