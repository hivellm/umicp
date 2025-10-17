/**
 * UMICP TypeScript/JavaScript Bindings
 * High-performance bindings to C++ core implementation
 */

import { createRequire } from 'module';
import { fileURLToPath } from 'url';
import { dirname, join, resolve } from 'path';

// Native addon - with fallback for testing
let addon: any;

// Function to load addon - works in both ESM and CJS
function loadAddon(): any {
  try {
    // Get the directory of this file - handle both ESM and CJS contexts
    let currentFileUrl: string;
    let currentDir: string;
    let requireFunc: any;

    // Check if we're in CommonJS context
    if (typeof __filename !== 'undefined') {
      // CommonJS context
      currentFileUrl = `file://${__filename}`;
      currentDir = dirname(__filename);
      requireFunc = require;
    } else {
      // ESM context - use import.meta (TypeScript will handle this in ESM build)
      // @ts-ignore - import.meta available in ESM
      currentFileUrl = import.meta.url;
      currentDir = dirname(fileURLToPath(currentFileUrl));
      requireFunc = createRequire(currentFileUrl);
    }

    const possiblePaths = [
      resolve(currentDir, '../build/Release/umicp_core.node'),
      resolve(currentDir, '../../build/Release/umicp_core.node'),
    ];

    let lastError: any = null;

    for (const addonPath of possiblePaths) {
      try {
        const addon = requireFunc(addonPath);
        //console.log(`✅ Native addon loaded from: ${addonPath}`);
        return addon;
      } catch (error: any) {
        lastError = error;
        console.warn(`Failed to load from ${addonPath}: ${error.message}`);
      }
    }

    if (lastError) {
      console.warn('Last error details:', lastError.message);
    }
  } catch (error: any) {
    console.warn('Error in loadAddon:', error.message);
  }

  return null;
}

addon = loadAddon();

if (!addon) {
  // Create a functional mock addon for testing when native module is not available
  console.warn('Native addon not found, using mock implementation for testing');

  class MockEnvelope {
    from: string;
    to: string;
    operation: string;
    messageId: string;
    timestamp: number;
    capabilities: any;
    payloadHint: any;

    constructor(data: any = {}) {
      this.from = data.from || '';
      this.to = data.to || '';
      this.operation = data.operation || 'DATA';
      this.messageId = data.messageId || Date.now().toString();
      this.timestamp = data.timestamp || Date.now();
      this.capabilities = data.capabilities || {};
      this.payloadHint = data.payloadHint || null;
    }

    setFrom(from: string) { this.from = from; return this; }
    setTo(to: string) { this.to = to; return this; }
    setOperation(op: string) { this.operation = op; return this; }
    setMessageId(id: string) { this.messageId = id; return this; }
    setCapabilities(caps: any) { this.capabilities = caps || {}; return this; }
    setCapability(key: string, value: any) { this.capabilities[key] = value; return this; }
    setPayloadHint(hint: any) { this.payloadHint = hint; return this; }

    getFrom() { return this.from; }
    getTo() { return this.to; }
    getOperation() { return this.operation; }
    getMessageId() { return this.messageId; }
    getTimestamp() { return this.timestamp; }
    getCapabilities() { return this.capabilities; }
    getPayloadHint() { return this.payloadHint; }
    getHash() { return this.hash(); }

    serialize() {
      return JSON.stringify({
        from: this.from,
        to: this.to,
        operation: this.operation,
        messageId: this.messageId,
        timestamp: this.timestamp,
        capabilities: this.capabilities,
        payloadHint: this.payloadHint
      });
    }

    deserialize(json: string) {
      const data = JSON.parse(json);
      this.from = data.from;
      this.to = data.to;
      this.operation = data.operation;
      this.messageId = data.messageId;
      this.timestamp = data.timestamp;
      this.capabilities = data.capabilities || {};
      this.payloadHint = data.payloadHint;
      return this;
    }

    toDict() {
      return {
        from: this.from,
        to: this.to,
        operation: this.operation,
        messageId: this.messageId,
        timestamp: this.timestamp,
        capabilities: this.capabilities,
        payloadHint: this.payloadHint
      };
    }

    hash() {
      // Simple hash for mock
      const str = this.serialize();
      let hash = 0;
      for (let i = 0; i < str.length; i++) {
        hash = ((hash << 5) - hash) + str.charCodeAt(i);
        hash |= 0;
      }
      return hash.toString(16);
    }

    validate() { return Boolean(this.from && this.to && this.messageId); }

    static deserialize(json: string) {
      const envelope = new MockEnvelope();
      envelope.deserialize(json);
      return envelope;
    }
  }

  class MockMatrix {
    dotProduct(a: number[], b: number[]) {
      if (!a || !b || a.length !== b.length) return 0;
      return a.reduce((sum, val, i) => sum + val * b[i], 0);
    }

    cosineSimilarity(a: number[], b: number[]) {
      if (!a || !b || a.length !== b.length) return 0;
      const dot = this.dotProduct(a, b);
      const magA = Math.sqrt(this.dotProduct(a, a));
      const magB = Math.sqrt(this.dotProduct(b, b));
      if (magA === 0 || magB === 0) return 0;
      return dot / (magA * magB);
    }

    euclideanDistance(a: number[], b: number[]) {
      if (!a || !b || a.length !== b.length) return 0;
      return Math.sqrt(a.reduce((sum, val, i) => sum + Math.pow(val - b[i], 2), 0));
    }

    add(a: number[], b: number[], result: number[]) {
      if (!a || !b || !result) return;
      for (let i = 0; i < Math.min(a.length, b.length, result.length); i++) {
        result[i] = a[i] + b[i];
      }
    }

    subtract(a: number[], b: number[], result: number[]) {
      if (!a || !b || !result) return;
      for (let i = 0; i < Math.min(a.length, b.length, result.length); i++) {
        result[i] = a[i] - b[i];
      }
    }

    scale(a: number[], scalar: number, result: number[]) {
      if (!a || !result) return;
      for (let i = 0; i < Math.min(a.length, result.length); i++) {
        result[i] = a[i] * scalar;
      }
    }

    multiply(a: number[], b: number[], result: number[], m: number, n: number, p: number) {
      if (!a || !b || !result) return;
      for (let i = 0; i < m; i++) {
        for (let j = 0; j < p; j++) {
          result[i * p + j] = 0;
          for (let k = 0; k < n; k++) {
            result[i * p + j] += a[i * n + k] * b[k * p + j];
          }
        }
      }
    }

    normalize(a: number[], result: number[]) {
      if (!a || !result) return;
      const mag = Math.sqrt(this.dotProduct(a, a));
      if (mag === 0) return;
      for (let i = 0; i < Math.min(a.length, result.length); i++) {
        result[i] = a[i] / mag;
      }
    }
  }

  addon = {
    WebSocketTransport: null,
    HTTP2Transport: null,
    Envelope: MockEnvelope,
    Matrix: MockMatrix,
    Frame: function() { return {}; },
    createEnvelope: (data: any) => new MockEnvelope(data),
    createMatrix: () => new MockMatrix()
  };
}

// Check if WebSocket transport is available
const hasWebSocketTransport = typeof addon.WebSocketTransport === 'function';
const hasHTTP2Transport = typeof addon.HTTP2Transport === 'function';

// Transport classes (conditionally exported based on availability)
export class WebSocketTransport {
  private nativeTransport: any;

  constructor(url: string, isServer: boolean = false) {
    if (!hasWebSocketTransport) {
      throw new Error('WebSocket transport not available in this build. Please rebuild with WebSocket support.');
    }
    this.nativeTransport = new addon.WebSocketTransport(url, isServer);
  }

  connect(): boolean {
    return this.nativeTransport.connect();
  }

  disconnect(): boolean {
    return this.nativeTransport.disconnect();
  }

  send(message: string): boolean {
    return this.nativeTransport.send(message);
  }

  isConnected(): boolean {
    return this.nativeTransport.isConnected();
  }

  getStats(): any {
    return this.nativeTransport.getStats();
  }

  setMessageCallback(callback: (message: string) => void): void {
    this.nativeTransport.setMessageCallback(callback);
  }

  setConnectionCallback(callback: (connected: boolean) => void): void {
    this.nativeTransport.setConnectionCallback(callback);
  }
}

export class HTTP2Transport {
  private nativeTransport: any;

  constructor(url: string) {
    if (!hasHTTP2Transport) {
      throw new Error('HTTP/2 transport not available in this build. Please rebuild with HTTP/2 support.');
    }
    this.nativeTransport = new addon.HTTP2Transport(url);
  }

  connect(): boolean {
    return this.nativeTransport.connect();
  }

  disconnect(): boolean {
    return this.nativeTransport.disconnect();
  }

  send(message: string): boolean {
    return this.nativeTransport.send(message);
  }

  isConnected(): boolean {
    return this.nativeTransport.isConnected();
  }

  getStats(): any {
    return this.nativeTransport.getStats();
  }

  setMessageCallback(callback: (message: string) => void): void {
    this.nativeTransport.setMessageCallback(callback);
  }

  setConnectionCallback(callback: (connected: boolean) => void): void {
    this.nativeTransport.setConnectionCallback(callback);
  }
}

// Export constants from native addon
export const OPERATION_CONTROL = addon.OPERATION_CONTROL;
export const OPERATION_DATA = addon.OPERATION_DATA;
export const OPERATION_ACK = addon.OPERATION_ACK;
export const OPERATION_ERROR = addon.OPERATION_ERROR;

export const PAYLOAD_VECTOR = addon.PAYLOAD_VECTOR;
export const PAYLOAD_TEXT = addon.PAYLOAD_TEXT;
export const PAYLOAD_METADATA = addon.PAYLOAD_METADATA;
export const PAYLOAD_BINARY = addon.PAYLOAD_BINARY;

export const ENCODING_FLOAT32 = addon.ENCODING_FLOAT32;
export const ENCODING_FLOAT64 = addon.ENCODING_FLOAT64;
export const ENCODING_INT32 = addon.ENCODING_INT32;
export const ENCODING_INT64 = addon.ENCODING_INT64;
export const ENCODING_UINT8 = addon.ENCODING_UINT8;
export const ENCODING_UINT16 = addon.ENCODING_UINT16;
export const ENCODING_UINT32 = addon.ENCODING_UINT32;
export const ENCODING_UINT64 = addon.ENCODING_UINT64;

// Type definitions
export interface EnvelopeOptions {
  from?: string;
  to?: string;
  operation?: OperationType;
  messageId?: string;
  capabilities?: Record<string, any>;
  payloadHint?: PayloadHint;
}

export interface PayloadHint {
  type?: PayloadType;
  size?: number;
  encoding?: EncodingType;
  count?: number;
}

export interface FrameOptions {
  type?: number;
  streamId?: number;
  sequence?: number;
  flags?: number;
  compressed?: boolean;
  encrypted?: boolean;
}

export interface MatrixResult {
  success: boolean;
  error?: string;
  result?: number;
  similarity?: number;
  data?: Float32Array;
}

export enum OperationType {
  CONTROL = 0,
  DATA = 1,
  ACK = 2,
  ERROR = 3
}

export enum PayloadType {
  VECTOR = 0,
  TEXT = 1,
  METADATA = 2,
  BINARY = 3
}

export enum EncodingType {
  FLOAT32 = 0,
  FLOAT64 = 1,
  INT32 = 2,
  INT64 = 3,
  UINT8 = 4,
  UINT16 = 5,
  UINT32 = 6,
  UINT64 = 7
}

// Envelope class
export class Envelope {
  private nativeEnvelope: any;

  constructor(options: EnvelopeOptions = {}) {
    this.nativeEnvelope = new addon.Envelope();

    if (options.from) this.setFrom(options.from);
    if (options.to) this.setTo(options.to);
    if (options.operation !== undefined) this.setOperation(options.operation);
    if (options.messageId) this.setMessageId(options.messageId);
    if (options.capabilities) this.setCapabilities(options.capabilities);
    if (options.payloadHint) this.setPayloadHint(options.payloadHint);
  }

  setFrom(from: string): Envelope {
    this.nativeEnvelope.setFrom(from);
    return this;
  }

  setTo(to: string): Envelope {
    this.nativeEnvelope.setTo(to);
    return this;
  }

  setOperation(operation: OperationType): Envelope {
    this.nativeEnvelope.setOperation(operation);
    return this;
  }

  setMessageId(messageId: string): Envelope {
    this.nativeEnvelope.setMessageId(messageId);
    return this;
  }

  setCapabilities(capabilities: Record<string, any>): Envelope {
    this.nativeEnvelope.setCapabilities(capabilities);
    return this;
  }

  setPayloadHint(hint: PayloadHint): Envelope {
    const hintObj: any = {};
    if (hint.type !== undefined) hintObj.type = hint.type;
    if (hint.size !== undefined) hintObj.size = hint.size;
    if (hint.encoding !== undefined) hintObj.encoding = hint.encoding;
    if (hint.count !== undefined) hintObj.count = hint.count;

    this.nativeEnvelope.setPayloadHint(hintObj);
    return this;
  }

  serialize(): string {
    return this.nativeEnvelope.serialize();
  }

  deserialize(json: string): Envelope {
    this.nativeEnvelope.deserialize(json);
    return this;
  }

  validate(): boolean {
    return this.nativeEnvelope.validate();
  }

  getHash(): string {
    return this.nativeEnvelope.getHash();
  }

  getFrom(): string {
    return this.nativeEnvelope.getFrom();
  }

  getTo(): string {
    return this.nativeEnvelope.getTo();
  }

  getMessageId(): string {
    return this.nativeEnvelope.getMessageId();
  }

  getCapabilities(): Record<string, any> {
    return this.nativeEnvelope.getCapabilities();
  }

  getOperation(): OperationType {
    return this.nativeEnvelope.getOperation();
  }

  // Static methods
  static create(options: EnvelopeOptions = {}): Envelope {
    return new Envelope(options);
  }

  static serialize(envelope: Envelope): string {
    return envelope.serialize();
  }

  static deserialize(json: string): Envelope {
    const envelope = new Envelope();
    return envelope.deserialize(json);
  }

  static validate(envelope: Envelope): boolean {
    return envelope.validate();
  }

  static hash(envelope: Envelope): string {
    return envelope.getHash();
  }
}

// Matrix operations class
export class Matrix {
  private nativeMatrix: any;

  constructor() {
    this.nativeMatrix = new addon.Matrix();
  }

  /**
   * Matrix addition: result = a + b
   */
  add(a: Float32Array, b: Float32Array, result: Float32Array, rows: number, cols: number): MatrixResult {
    try {
      return this.nativeMatrix.add(a, b, result, rows, cols);
    } catch (error) {
      return {
        success: false,
        error: error instanceof Error ? error.message : 'Unknown error'
      };
    }
  }

  /**
   * Matrix multiplication: result = a * b (m x n) * (n x p) = (m x p)
   */
  multiply(a: Float32Array, b: Float32Array, result: Float32Array, m: number, n: number, p: number): MatrixResult {
    try {
      return this.nativeMatrix.multiply(a, b, result, m, n, p);
    } catch (error) {
      return {
        success: false,
        error: error instanceof Error ? error.message : 'Unknown error'
      };
    }
  }

  /**
   * Matrix transpose: result = a^T
   */
  transpose(input: Float32Array, output: Float32Array, rows: number, cols: number): MatrixResult {
    try {
      return this.nativeMatrix.transpose(input, output, rows, cols);
    } catch (error) {
      return {
        success: false,
        error: error instanceof Error ? error.message : 'Unknown error'
      };
    }
  }

  /**
   * Dot product of two vectors
   */
  dotProduct(a: Float32Array, b: Float32Array): MatrixResult {
    try {
      return this.nativeMatrix.dotProduct(a, b);
    } catch (error) {
      return {
        success: false,
        error: error instanceof Error ? error.message : 'Unknown error'
      };
    }
  }

  /**
   * Vector/matrix normalization (L2 normalization)
   */
  normalize(matrix: Float32Array, rows: number, cols: number): MatrixResult {
    try {
      return this.nativeMatrix.normalize(matrix, rows, cols);
    } catch (error) {
      return {
        success: false,
        error: error instanceof Error ? error.message : 'Unknown error'
      };
    }
  }

  /**
   * Cosine similarity between two vectors
   */
  cosineSimilarity(a: Float32Array, b: Float32Array): MatrixResult {
    try {
      return this.nativeMatrix.cosineSimilarity(a, b);
    } catch (error) {
      return {
        success: false,
        error: error instanceof Error ? error.message : 'Unknown error'
      };
    }
  }
}

// Utility functions
export const UMICP = {
  version: addon.version || '1.0.0',
  UMICP_VERSION: addon.UMICP_VERSION || '1.0',

  // Transport availability
  hasWebSocketTransport,
  hasHTTP2Transport,

  // Create instances
  createEnvelope: (options?: EnvelopeOptions) => new Envelope(options),
  createMatrix: () => new Matrix(),
  createWebSocketTransport: (url: string, isServer?: boolean) => {
    if (!hasWebSocketTransport) {
      throw new Error('WebSocket transport not available in this build');
    }
    return new WebSocketTransport(url, isServer);
  },
  createHTTP2Transport: (url: string) => {
    if (!hasHTTP2Transport) {
      throw new Error('HTTP/2 transport not available in this build');
    }
    return new HTTP2Transport(url);
  },

  // Classes
  WebSocketTransport,
  HTTP2Transport,

  // Constants
  OperationType,
  PayloadType,
  EncodingType
};

// High-level transport exports
export {
  StreamableHTTPServer,
  StreamableHTTPClient,
  StreamableHTTPPeer,
  createUMICPHandler,
  createUMICPRoute,
  type StreamableHTTPServerOptions,
  type StreamableHTTPClientOptions,
  type StreamableHTTPPeerOptions,
  type StreamableHTTPPeerEvents,
  type HTTPPeerConnection,
  type HTTPPeerInfo,
} from './transports/streamable-http.js';

export {
  UMICPWebSocketServer,
  UMICPWebSocketClient,
  UMICPWebSocketPeer,
  createUMICPWebSocketServer,
  type UMICPWebSocketServerOptions,
  type UMICPWebSocketClientOptions,
  type UMICPWebSocketPeerOptions,
  type UMICPWebSocketPeerEvents,
  type PeerConnection,
  type PeerInfo,
} from './transports/websocket-transport.js';

// Tool Discovery exports (v0.2.0)
export {
  type OperationSchema,
  type ServerInfo,
  type DiscoverableService,
  type JSONSchema,
  OperationSchemaBuilder,
  ServerInfoBuilder,
  DiscoveryHelpers,
  SimpleDiscoverableService,
} from './discovery.js';

// Default exports
export default UMICP;
