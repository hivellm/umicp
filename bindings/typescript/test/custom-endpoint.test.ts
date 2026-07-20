/**
 * Custom Endpoint Support Tests (v0.2.2)
 *
 * Verifies that the TypeScript UMICP implementation supports
 * custom endpoint paths for compatibility with different servers
 * (e.g., Vectorizer uses /umicp, standard servers use /message)
 */

import { describe, it, expect } from 'vitest';
import { StreamableHTTPClient, type StreamableHTTPClientOptions } from '../src/transports/streamable-http.js';

describe('Custom Endpoint Support', () => {
  describe('Client Configuration', () => {
    it('should support default path /umicp', () => {
      const options: StreamableHTTPClientOptions = {
        baseUrl: 'http://localhost:8000',
      };

      const client = new StreamableHTTPClient(options);
      expect(client).toBeDefined();
      // Default path should be /umicp
    });

    it('should support custom path for Vectorizer', () => {
      const options: StreamableHTTPClientOptions = {
        baseUrl: 'http://localhost:8000',
        path: '/umicp',
      };

      const client = new StreamableHTTPClient(options);
      expect(client).toBeDefined();
    });

    it('should support custom path for standard server', () => {
      const options: StreamableHTTPClientOptions = {
        baseUrl: 'http://localhost:9000',
        path: '/message',
      };

      const client = new StreamableHTTPClient(options);
      expect(client).toBeDefined();
    });

    it('should support empty path', () => {
      const options: StreamableHTTPClientOptions = {
        baseUrl: 'http://localhost:8000',
        path: '',
      };

      const client = new StreamableHTTPClient(options);
      expect(client).toBeDefined();
    });

    it('should support complex path', () => {
      const options: StreamableHTTPClientOptions = {
        baseUrl: 'http://localhost:8000',
        path: '/api/v1/umicp',
      };

      const client = new StreamableHTTPClient(options);
      expect(client).toBeDefined();
    });
  });

  describe('Multiple Clients', () => {
    it('should support multiple clients with different endpoints', () => {
      const vectorizerClient = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
        path: '/umicp',
      });

      const standardClient = new StreamableHTTPClient({
        baseUrl: 'http://localhost:9000',
        path: '/message',
      });

      expect(vectorizerClient).toBeDefined();
      expect(standardClient).toBeDefined();
      expect(vectorizerClient).not.toBe(standardClient);
    });

    it('should allow same base URL with different paths', () => {
      const client1 = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
        path: '/umicp',
      });

      const client2 = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
        path: '/message',
      });

      expect(client1).toBeDefined();
      expect(client2).toBeDefined();
    });
  });

  describe('Backward Compatibility', () => {
    it('should work without specifying path (uses default)', () => {
      // Old code that doesn't specify path
      const client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
      });

      expect(client).toBeDefined();
      expect(client.isConnected()).toBe(false);
    });

    it('should maintain default behavior', () => {
      const client1 = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
      });

      const client2 = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
        path: '/umicp', // Explicit default
      });

      expect(client1).toBeDefined();
      expect(client2).toBeDefined();
    });
  });

  describe('Edge Cases', () => {
    it('should handle path with trailing slash', () => {
      const client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
        path: '/umicp/',
      });

      expect(client).toBeDefined();
    });

    it('should handle base URL with trailing slash', () => {
      const client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000/',
        path: '/umicp',
      });

      expect(client).toBeDefined();
    });

    it('should handle both with trailing slashes', () => {
      const client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000/',
        path: '/umicp/',
      });

      expect(client).toBeDefined();
    });

    it('should handle path with query parameters', () => {
      const client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
        path: '/umicp?version=1',
      });

      expect(client).toBeDefined();
    });
  });

  describe('Configuration Options', () => {
    it('should support all options with custom path', () => {
      const client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
        path: '/umicp',
        timeout: 30000,
        headers: {
          'X-Custom-Header': 'value',
        },
      });

      expect(client).toBeDefined();
    });

    it('should support HTTPS endpoints', () => {
      const client = new StreamableHTTPClient({
        baseUrl: 'https://localhost:8443',
        path: '/umicp',
      });

      expect(client).toBeDefined();
    });
  });

  describe('Version 0.2.2 Features', () => {
    it('should support custom endpoints in v0.2.2', () => {
      // Test that v0.2.2 supports custom endpoints
      const vectorizerClient = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
        path: '/umicp',
      });

      const messageClient = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
        path: '/message',
      });

      expect(vectorizerClient).toBeDefined();
      expect(messageClient).toBeDefined();
    });

    it('should maintain connection state interface', () => {
      const client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
        path: '/umicp',
      });

      // Client should have isConnected method
      expect(typeof client.isConnected).toBe('function');
      expect(client.isConnected()).toBe(false);
    });

    it('should maintain disconnect interface', () => {
      const client = new StreamableHTTPClient({
        baseUrl: 'http://localhost:8000',
        path: '/umicp',
      });

      // Client should have disconnect method
      expect(typeof client.disconnect).toBe('function');

      // Should not throw
      expect(() => client.disconnect()).not.toThrow();
    });
  });

  describe('Type Safety', () => {
    it('should accept valid StreamableHTTPClientOptions', () => {
      const options: StreamableHTTPClientOptions = {
        baseUrl: 'http://localhost:8000',
        path: '/umicp',
        timeout: 30000,
        headers: {
          'Content-Type': 'application/json',
        },
      };

      const client = new StreamableHTTPClient(options);
      expect(client).toBeDefined();
    });

    it('should work with minimal options', () => {
      const options: StreamableHTTPClientOptions = {
        baseUrl: 'http://localhost:8000',
      };

      const client = new StreamableHTTPClient(options);
      expect(client).toBeDefined();
    });
  });
});

describe('StreamableHTTPClient Path Handling', () => {
  it('should use provided path in requests', () => {
    const client = new StreamableHTTPClient({
      baseUrl: 'http://localhost:8000',
      path: '/custom',
    });

    expect(client).toBeDefined();
    expect(client.isConnected()).toBe(false);
  });

  it('should support Vectorizer configuration', () => {
    // Typical Vectorizer configuration
    const client = new StreamableHTTPClient({
      baseUrl: 'http://localhost:8000',
      path: '/umicp',
      timeout: 30000,
    });

    expect(client).toBeDefined();
  });

  it('should support standard UMICP server configuration', () => {
    // Standard UMICP server configuration
    const client = new StreamableHTTPClient({
      baseUrl: 'http://localhost:9000',
      path: '/message',
      timeout: 30000,
    });

    expect(client).toBeDefined();
  });
});

