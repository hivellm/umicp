import * as net from 'net';
import * as tls from 'tls';

/**
 * UMICP Protocol Constants
 */
const UMICP_VERSION = 1;
const UMICP_MAGIC = Buffer.from([0x55, 0x4D, 0x49, 0x43]); // "UMIC"

/**
 * UMICP Message Types
 */
enum MessageType {
  REQUEST = 0x01,
  RESPONSE = 0x02,
  ERROR = 0x03,
}

/**
 * UMICP Client Configuration
 */
export interface ClientConfig {
  host: string;
  port: number;
  tls?: boolean;
  timeout?: number;
}

/**
 * UMICP Request
 */
export interface UmicpRequest {
  method: string;
  payload: any;
  metadata: Record<string, string>;
}

/**
 * UMICP Response
 */
export interface UmicpResponse {
  data: any;
  metadata?: Record<string, string>;
}

/**
 * Pure TypeScript UMICP Client Implementation
 */
export class Client {
  private config: ClientConfig;
  private socket?: net.Socket | tls.TLSSocket;
  private connected = false;
  private requestId = 0;
  private pendingRequests = new Map<number, {
    resolve: (value: any) => void;
    reject: (error: Error) => void;
  }>();

  constructor(config: ClientConfig) {
    this.config = config;
  }

  /**
   * Connect to UMICP server
   */
  async connect(): Promise<void> {
    if (this.connected) {
      return;
    }

    return new Promise((resolve, reject) => {
      const options = {
        host: this.config.host,
        port: this.config.port,
      };

      if (this.config.tls) {
        this.socket = tls.connect(options, () => {
          this.connected = true;
          this.setupSocketHandlers();
          resolve();
        });
      } else {
        this.socket = net.connect(options, () => {
          this.connected = true;
          this.setupSocketHandlers();
          resolve();
        });
      }

      this.socket.on('error', (error) => {
        this.connected = false;
        reject(error);
      });

      // Set timeout if configured
      if (this.config.timeout) {
        this.socket.setTimeout(this.config.timeout);
      }
    });
  }

  /**
   * Setup socket event handlers
   */
  private setupSocketHandlers(): void {
    if (!this.socket) return;

    let buffer = Buffer.alloc(0);

    this.socket.on('data', (chunk) => {
      buffer = Buffer.concat([buffer, chunk]);

      // Try to parse complete messages from buffer
      while (buffer.length >= 16) { // Minimum header size
        const messageLength = this.tryParseMessage(buffer);
        if (messageLength === -1) {
          break; // Need more data
        }

        // Extract message and update buffer
        const message = buffer.slice(0, messageLength);
        buffer = buffer.slice(messageLength);

        // Process message
        this.handleMessage(message);
      }
    });

    this.socket.on('close', () => {
      this.connected = false;
      // Reject all pending requests
      for (const [id, pending] of this.pendingRequests) {
        pending.reject(new Error('Connection closed'));
      }
      this.pendingRequests.clear();
    });

    this.socket.on('error', (error) => {
      this.connected = false;
      // Reject all pending requests
      for (const [id, pending] of this.pendingRequests) {
        pending.reject(error);
      }
      this.pendingRequests.clear();
    });
  }

  /**
   * Try to parse a message from buffer
   * Returns message length if complete, -1 if need more data
   */
  private tryParseMessage(buffer: Buffer): number {
    if (buffer.length < 16) return -1;

    // Check magic bytes
    if (!buffer.slice(0, 4).equals(UMICP_MAGIC)) {
      throw new Error('Invalid UMICP magic bytes');
    }

    // Read payload length (bytes 8-12, little-endian uint32)
    const payloadLength = buffer.readUInt32LE(8);
    const totalLength = 16 + payloadLength; // Header + payload

    if (buffer.length < totalLength) {
      return -1; // Need more data
    }

    return totalLength;
  }

  /**
   * Handle incoming message
   */
  private handleMessage(buffer: Buffer): void {
    try {
      // Parse header
      const version = buffer.readUInt8(4);
      const messageType = buffer.readUInt8(5);
      const requestId = buffer.readUInt16LE(6);
      const payloadLength = buffer.readUInt32LE(8);
      const metadataLength = buffer.readUInt32LE(12);

      // Extract payload
      const payload = buffer.slice(16, 16 + payloadLength);
      const metadata = buffer.slice(16 + payloadLength, 16 + payloadLength + metadataLength);

      // Parse JSON payload
      let data: any = {};
      if (payloadLength > 0) {
        data = JSON.parse(payload.toString('utf8'));
      }

      // Parse metadata
      let metadataObj: Record<string, string> = {};
      if (metadataLength > 0) {
        metadataObj = JSON.parse(metadata.toString('utf8'));
      }

      // Handle based on message type
      const pending = this.pendingRequests.get(requestId);
      if (pending) {
        this.pendingRequests.delete(requestId);

        if (messageType === MessageType.RESPONSE) {
          pending.resolve({ data, metadata: metadataObj });
        } else if (messageType === MessageType.ERROR) {
          pending.reject(new Error(data.error || 'Unknown error'));
        }
      }
    } catch (error) {
      console.error('Error handling message:', error);
    }
  }

  /**
   * Execute a UMICP call
   */
  async call(request: UmicpRequest): Promise<any> {
    if (!this.connected || !this.socket) {
      throw new Error('Not connected to server');
    }

    const requestId = ++this.requestId;

    // Create message
    const message = this.createMessage(MessageType.REQUEST, requestId, request);

    // Send message
    return new Promise((resolve, reject) => {
      this.pendingRequests.set(requestId, { resolve, reject });

      this.socket!.write(message, (error) => {
        if (error) {
          this.pendingRequests.delete(requestId);
          reject(error);
        }
      });
    });
  }

  /**
   * Create UMICP message
   */
  private createMessage(
    messageType: MessageType,
    requestId: number,
    request: UmicpRequest
  ): Buffer {
    // Serialize payload and metadata
    const payloadBuffer = Buffer.from(JSON.stringify(request.payload || {}), 'utf8');
    const metadataObj = {
      ...request.metadata,
      method: request.method,
    };
    const metadataBuffer = Buffer.from(JSON.stringify(metadataObj), 'utf8');

    // Create header (16 bytes)
    const header = Buffer.alloc(16);
    UMICP_MAGIC.copy(header, 0); // Magic bytes (4 bytes)
    header.writeUInt8(UMICP_VERSION, 4); // Version (1 byte)
    header.writeUInt8(messageType, 5); // Message type (1 byte)
    header.writeUInt16LE(requestId, 6); // Request ID (2 bytes)
    header.writeUInt32LE(payloadBuffer.length, 8); // Payload length (4 bytes)
    header.writeUInt32LE(metadataBuffer.length, 12); // Metadata length (4 bytes)

    // Combine header, payload, and metadata
    return Buffer.concat([header, payloadBuffer, metadataBuffer]);
  }

  /**
   * Disconnect from server
   */
  async disconnect(): Promise<void> {
    if (this.socket) {
      return new Promise((resolve) => {
        this.socket!.once('close', () => {
          this.connected = false;
          resolve();
        });
        this.socket!.end();
      });
    }
  }

  /**
   * Check if connected
   */
  isConnected(): boolean {
    return this.connected;
  }
}

