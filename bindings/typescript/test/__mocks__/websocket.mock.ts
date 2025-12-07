/**
 * WebSocket Mock for Testing
 * Simulates WebSocket behavior without requiring real servers
 */

import { EventEmitter } from 'events';

export class MockWebSocket extends EventEmitter {
  public url: string;
  public readyState: number = 0; // CONNECTING
  public static CONNECTING = 0;
  public static OPEN = 1;
  public static CLOSING = 2;
  public static CLOSED = 3;

  private static connections: Map<string, MockWebSocket[]> = new Map();

  constructor(url: string) {
    super();
    this.url = url;

    // Register this connection
    const connections = MockWebSocket.connections.get(url) || [];
    connections.push(this);
    MockWebSocket.connections.set(url, connections);

    // Simulate async connection
    setTimeout(() => {
      this.readyState = MockWebSocket.OPEN;
      this.emit('open');
    }, 10);
  }

  send(data: string | Buffer) {
    if (this.readyState !== MockWebSocket.OPEN) {
      throw new Error('WebSocket is not open');
    }

    // Simulate message delivery to other connections on same URL
    const connections = MockWebSocket.connections.get(this.url) || [];
    connections.forEach(conn => {
      if (conn !== this && conn.readyState === MockWebSocket.OPEN) {
        setTimeout(() => {
          conn.emit('message', data);
        }, 5);
      }
    });
  }

  close() {
    if (this.readyState === MockWebSocket.CLOSED) return;

    this.readyState = MockWebSocket.CLOSING;

    setTimeout(() => {
      this.readyState = MockWebSocket.CLOSED;
      this.emit('close');

      // Remove from connections
      const connections = MockWebSocket.connections.get(this.url) || [];
      const index = connections.indexOf(this);
      if (index > -1) {
        connections.splice(index, 1);
      }
    }, 10);
  }

  // Clean up all connections (for afterEach)
  static cleanup() {
    MockWebSocket.connections.clear();
  }
}

export class MockWebSocketServer extends EventEmitter {
  public clients: Set<MockWebSocket> = new Set();
  private port: number;
  private isListening: boolean = false;

  constructor(options: { port: number }) {
    super();
    this.port = options.port;
  }

  listen(callback?: () => void) {
    this.isListening = true;

    setTimeout(() => {
      this.emit('listening');
      if (callback) callback();
    }, 10);
  }

  close(callback?: () => void) {
    this.isListening = false;
    this.clients.clear();

    setTimeout(() => {
      this.emit('close');
      if (callback) callback();
    }, 10);
  }

  // Simulate client connection
  simulateConnection(client: MockWebSocket) {
    this.clients.add(client);
    this.emit('connection', client);
  }

  address() {
    return {
      port: this.port,
      family: 'IPv4',
      address: '127.0.0.1'
    };
  }
}

// Mock UMICPWebSocketPeer for testing without real WebSocket
export class MockUMICPWebSocketPeer extends EventEmitter {
  public peerId: string;
  private peers: Map<string, any> = new Map();
  private isConnected: boolean = false;
  private serverPort?: number;

  constructor(options: { peerId: string; server?: { port: number } }) {
    super();
    this.peerId = options.peerId;
    this.serverPort = options.server?.port;

    if (this.serverPort) {
      // Simulate server ready
      setTimeout(() => {
        this.emit('server:ready', this.serverPort);
      }, 20);
    }
  }

  async connectToPeer(url: string, metadata?: any): Promise<string | null> {
    // Simulate connection
    const peerId = `mock-peer-${Date.now()}-${Math.random()}`;

    this.peers.set(peerId, {
      id: peerId,
      url,
      type: 'outgoing',
      metadata: metadata || {},
      connectedAt: Date.now()
    });

    this.isConnected = true;

    setTimeout(() => {
      this.emit('peer:connect', this.peers.get(peerId));
    }, 30);

    return peerId;
  }

  sendToPeer(peerId: string, message: any): boolean {
    const peer = this.peers.get(peerId);
    if (!peer) return false;

    // Simulate message delivery
    setTimeout(() => {
      this.emit('data', message, { id: peerId });
    }, 10);

    return true;
  }

  broadcast(message: any, excludePeerId?: string): number {
    let sent = 0;

    this.peers.forEach((peer, id) => {
      if (id !== excludePeerId) {
        this.sendToPeer(id, message);
        sent++;
      }
    });

    return sent;
  }

  broadcastToType(message: any, type: 'incoming' | 'outgoing'): number {
    let sent = 0;

    this.peers.forEach((peer, id) => {
      if (peer.type === type) {
        this.sendToPeer(id, message);
        sent++;
      }
    });

    return sent;
  }

  getPeers(): any[] {
    return Array.from(this.peers.values());
  }

  getPeer(peerId: string): any | undefined {
    return this.peers.get(peerId);
  }

  findPeerByMetadata(key: string, value: any): any | undefined {
    for (const peer of this.peers.values()) {
      if (peer.metadata && peer.metadata[key] === value) {
        return peer;
      }
    }
    return undefined;
  }

  disconnectPeer(peerId: string): boolean {
    const peer = this.peers.get(peerId);
    if (!peer) return false;

    this.peers.delete(peerId);

    setTimeout(() => {
      this.emit('peer:disconnect', peer);
    }, 10);

    return true;
  }

  disconnectPeerByUrl(url: string): boolean {
    for (const [id, peer] of this.peers.entries()) {
      if (peer.url === url) {
        return this.disconnectPeer(id);
      }
    }
    return false;
  }

  async sendAndWait(peerId: string, message: any, timeout: number = 5000): Promise<any> {
    const peer = this.peers.get(peerId);
    if (!peer) {
      throw new Error(`Peer ${peerId} not found`);
    }

    return new Promise((resolve, reject) => {
      const timer = setTimeout(() => {
        reject(new Error('Response timeout'));
      }, timeout);

      // Simulate response
      setTimeout(() => {
        clearTimeout(timer);
        resolve({ response: 'mock-response', from: peerId });
      }, 50);
    });
  }

  getStats() {
    const peers = Array.from(this.peers.values());
    return {
      peerId: this.peerId,
      totalPeers: peers.length,
      incomingConnections: peers.filter(p => p.type === 'incoming').length,
      outgoingConnections: peers.filter(p => p.type === 'outgoing').length,
      serverActive: !!this.serverPort,
      uptime: Date.now()
    };
  }

  async shutdown(): Promise<void> {
    this.peers.clear();
    this.isConnected = false;

    return new Promise(resolve => {
      setTimeout(() => {
        this.emit('shutdown');
        resolve();
      }, 20);
    });
  }
}

