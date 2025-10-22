/**
 * UMICP Server Configuration
 */
export interface UmicpServerConfig {
  /** Server host (default: localhost) */
  host: string;
  /** Server port */
  port: number;
  /** Connection timeout in milliseconds */
  timeout?: number;
  /** Enable TLS/SSL */
  tls?: boolean;
}

/**
 * UMICP Call Request
 */
export interface UmicpCallRequest {
  /** Target server configuration */
  server: UmicpServerConfig;
  /** Method to call */
  method: string;
  /** Request payload */
  payload?: any;
  /** Request metadata */
  metadata?: Record<string, string>;
  /** Request timeout in milliseconds */
  timeout?: number;
}

/**
 * UMICP Call Response
 */
export interface UmicpCallResponse {
  /** Response status */
  status: 'success' | 'error';
  /** Response data */
  data?: any;
  /** Error message if status is 'error' */
  error?: string;
  /** Response metadata */
  metadata?: Record<string, string>;
  /** Execution time in milliseconds */
  executionTime: number;
}

/**
 * Bridge Statistics
 */
export interface BridgeStats {
  /** Total calls made */
  totalCalls: number;
  /** Successful calls */
  successfulCalls: number;
  /** Failed calls */
  failedCalls: number;
  /** Average execution time */
  avgExecutionTime: number;
  /** Last call timestamp */
  lastCallTime?: Date;
}

