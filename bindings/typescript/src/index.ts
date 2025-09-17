/**
 * UMICP TypeScript/JavaScript Bindings
 * High-performance bindings to C++ core implementation
 */

// Native addon - with fallback for testing
let addon: any;

try {
  addon = require('../build/Release/umicp_core.node');
} catch (error) {
  // Create a mock addon for testing when native module is not available
  console.warn('Native addon not found, using mock implementation for testing');
  addon = {
    WebSocketTransport: null,
    HTTP2Transport: null,
    Envelope: function() { throw new Error('Native addon not available'); },
    Matrix: function() { throw new Error('Native addon not available'); },
    Frame: function() { throw new Error('Native addon not available'); },
    createEnvelope: function() { throw new Error('Native addon not available'); },
    createMatrix: function() { throw new Error('Native addon not available'); }
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
  capabilities?: Record<string, string>;
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

  setCapabilities(capabilities: Record<string, string>): Envelope {
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

  getCapabilities(): Record<string, string> {
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

// Default exports
export default UMICP;
