import { Client } from './client.js';
import { HttpClient } from './http-client.js';
import type {
  UmicpServerConfig,
  UmicpCallRequest,
  UmicpCallResponse,
  BridgeStats,
} from './types.js';

/**
 * UMICP Bridge
 * Handles communication between MCP and UMICP servers
 */
export class UmicpBridge {
  private stats: BridgeStats = {
    totalCalls: 0,
    successfulCalls: 0,
    failedCalls: 0,
    avgExecutionTime: 0,
  };

  private clients: Map<string, Client | HttpClient> = new Map();

  /**
   * Execute a UMICP call
   */
  async call(request: UmicpCallRequest): Promise<UmicpCallResponse> {
    const startTime = Date.now();
    this.stats.totalCalls++;

    try {
      // Get or create client for this server
      const client = await this.getClient(request.server);

      // Prepare UMICP request
      const umicpRequest = {
        method: request.method,
        payload: request.payload || {},
        metadata: request.metadata || {},
      };

      // Execute call with timeout
      const timeout = request.timeout || request.server.timeout || 30000;
      const responsePromise = client.call(umicpRequest);

      const timeoutPromise = new Promise((_, reject) => {
        setTimeout(() => reject(new Error('Request timeout')), timeout);
      });

      const response = await Promise.race([responsePromise, timeoutPromise]);

      const executionTime = Date.now() - startTime;
      this.updateStats(executionTime, true);

      return {
        status: 'success',
        data: response,
        executionTime,
      };
    } catch (error) {
      const executionTime = Date.now() - startTime;
      this.updateStats(executionTime, false);

      return {
        status: 'error',
        error: error instanceof Error ? error.message : String(error),
        executionTime,
      };
    }
  }

  /**
   * Get or create a client for the specified server
   */
  private async getClient(config: UmicpServerConfig): Promise<Client | HttpClient> {
    const key = `${config.host}:${config.port}`;

    let client = this.clients.get(key);
    if (!client) {
      // Detect if we should use HTTP or TCP
      // If port is 80, 443, or in the 1xxxx range (typical HTTP ports), use HTTP client
      const useHttp = config.port === 80 || config.port === 443 ||
                      (config.port >= 10000 && config.port < 20000);

      if (useHttp) {
        // Use HTTP client
        const protocol = config.tls ? 'https' : 'http';
        const baseUrl = `${protocol}://${config.host}:${config.port}/umicp`;

        client = new HttpClient({
          baseUrl,
          timeout: config.timeout || 30000,
        });
      } else {
        // Use TCP client
        client = new Client({
          host: config.host,
          port: config.port,
          tls: config.tls || false,
        });
      }

      await client.connect();
      this.clients.set(key, client);
    }

    return client;
  }

  /**
   * Update bridge statistics
   */
  private updateStats(executionTime: number, success: boolean): void {
    if (success) {
      this.stats.successfulCalls++;
    } else {
      this.stats.failedCalls++;
    }

    // Update average execution time
    const totalTime = this.stats.avgExecutionTime * (this.stats.totalCalls - 1);
    this.stats.avgExecutionTime = (totalTime + executionTime) / this.stats.totalCalls;
    this.stats.lastCallTime = new Date();
  }

  /**
   * Get bridge statistics
   */
  getStats(): BridgeStats {
    return { ...this.stats };
  }

  /**
   * Reset bridge statistics
   */
  resetStats(): void {
    this.stats = {
      totalCalls: 0,
      successfulCalls: 0,
      failedCalls: 0,
      avgExecutionTime: 0,
    };
  }

  /**
   * Close all client connections
   */
  async close(): Promise<void> {
    const closePromises = Array.from(this.clients.values()).map((client) =>
      client.disconnect().catch((err) => {
        console.error('Error closing client:', err);
      })
    );

    await Promise.all(closePromises);
    this.clients.clear();
  }

  /**
   * List active server connections
   */
  listConnections(): string[] {
    return Array.from(this.clients.keys());
  }

  /**
   * Close connection to a specific server
   */
  async closeConnection(host: string, port: number): Promise<boolean> {
    const key = `${host}:${port}`;
    const client = this.clients.get(key);

    if (client) {
      await client.disconnect();
      this.clients.delete(key);
      return true;
    }

    return false;
  }
}

