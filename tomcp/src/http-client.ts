/**
 * UMICP HTTP Client
 *
 * Connects to UMICP servers over HTTP instead of raw TCP sockets
 */

export interface HttpClientConfig {
  baseUrl: string;
  timeout?: number;
}

interface UmicpRequest {
  method: string;
  payload: any;
  metadata: Record<string, string>;
}

export class HttpClient {
  private config: HttpClientConfig;
  private requestId = 0;

  constructor(config: HttpClientConfig) {
    this.config = config;
  }

  /**
   * No-op connect for HTTP (stateless)
   */
  async connect(): Promise<void> {
    // HTTP is stateless, just verify the server is reachable
    try {
      const response = await fetch(`${this.config.baseUrl}/discover`, {
        method: 'GET',
        signal: AbortSignal.timeout(this.config.timeout || 5000),
      });

      if (!response.ok) {
        throw new Error(`Server not reachable: ${response.status}`);
      }
    } catch (error) {
      throw new Error(`Failed to connect to ${this.config.baseUrl}: ${error}`);
    }
  }

  /**
   * Execute a UMICP call via HTTP POST
   */
  async call(request: UmicpRequest): Promise<any> {
    this.requestId++;

    // Build UMICP Envelope (v2.0 format) - matching Rust expectations
    const envelope = {
      v: "2.0",  // UMICP version (STRING required by Rust)
      msg_id: `http-${this.requestId}-${Date.now()}`,  // Rust expects msg_id not id
      from: 'umicp-mcp-bridge',
      to: 'synap-server',
      op: 'request',  // operation type: request for tool calls
      ts: new Date().toISOString(),  // timestamp (ISO 8601 format required)
      capabilities: {  // Rust expects "capabilities" not "caps"
        operation: request.method,
        ...request.payload,
      },
    };

    try {
      const response = await fetch(this.config.baseUrl, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(envelope),
        signal: AbortSignal.timeout(this.config.timeout || 30000),
      });

      if (!response.ok) {
        const errorText = await response.text();
        throw new Error(`HTTP ${response.status}: ${errorText}`);
      }

      const result = await response.json() as any;

      // Check if it's an error response
      if (result.error) {
        throw new Error(result.error as string);
      }

      // Extract data from UMICP response envelope
      if (result.capabilities) {
        return result.capabilities;
      }

      return result;
    } catch (error) {
      if (error instanceof Error) {
        throw error;
      }
      throw new Error(String(error));
    }
  }

  /**
   * No-op disconnect for HTTP (stateless)
   */
  async disconnect(): Promise<void> {
    // HTTP is stateless, nothing to disconnect
  }

  /**
   * Check if connected (always true for HTTP)
   */
  isConnected(): boolean {
    return true;
  }
}

