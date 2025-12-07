#!/usr/bin/env node

import { Server } from '@modelcontextprotocol/sdk/server/index.js';
import { StdioServerTransport } from '@modelcontextprotocol/sdk/server/stdio.js';
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
  Tool,
} from '@modelcontextprotocol/sdk/types.js';
import { UmicpBridge } from './bridge.js';
import type { UmicpCallRequest } from './types.js';

/**
 * UMICP to MCP Bridge Server
 * Exposes UMICP functionality through Model Context Protocol
 */

const bridge = new UmicpBridge();

// Define available tools
const TOOLS: Tool[] = [
  {
    name: 'umicp_call',
    description: 'Execute a UMICP call to a remote server',
    inputSchema: {
      type: 'object',
      properties: {
        server: {
          type: 'object',
          properties: {
            host: {
              type: 'string',
              description: 'Server hostname or IP address',
              default: 'localhost',
            },
            port: {
              type: 'number',
              description: 'Server port',
            },
            timeout: {
              type: 'number',
              description: 'Connection timeout in milliseconds',
              default: 30000,
            },
            tls: {
              type: 'boolean',
              description: 'Enable TLS/SSL',
              default: false,
            },
          },
          required: ['port'],
        },
        method: {
          type: 'string',
          description: 'Method to call on the UMICP server',
        },
        payload: {
          type: 'object',
          description: 'Request payload (optional)',
        },
        metadata: {
          type: 'object',
          description: 'Request metadata (optional)',
          additionalProperties: {
            type: 'string',
          },
        },
        timeout: {
          type: 'number',
          description: 'Request timeout in milliseconds',
          default: 30000,
        },
      },
      required: ['server', 'method'],
    },
  },
  {
    name: 'umicp_stats',
    description: 'Get UMICP bridge statistics',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'umicp_connections',
    description: 'List active UMICP server connections',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
  {
    name: 'umicp_close_connection',
    description: 'Close connection to a specific UMICP server',
    inputSchema: {
      type: 'object',
      properties: {
        host: {
          type: 'string',
          description: 'Server hostname',
        },
        port: {
          type: 'number',
          description: 'Server port',
        },
      },
      required: ['host', 'port'],
    },
  },
  {
    name: 'umicp_reset_stats',
    description: 'Reset bridge statistics',
    inputSchema: {
      type: 'object',
      properties: {},
    },
  },
];

// Create MCP server
const server = new Server(
  {
    name: '@hivellm/umicp2mcp',
    version: '0.1.0',
  },
  {
    capabilities: {
      tools: {},
    },
  }
);

// Handle tool listing
server.setRequestHandler(ListToolsRequestSchema, async () => {
  return { tools: TOOLS };
});

// Handle tool calls
server.setRequestHandler(CallToolRequestSchema, async (request) => {
  const { name, arguments: args } = request.params;

  try {
    switch (name) {
      case 'umicp_call': {
        const callRequest = args as unknown as UmicpCallRequest;

        // Set default host if not provided
        if (!callRequest.server.host) {
          callRequest.server.host = 'localhost';
        }

        const response = await bridge.call(callRequest);

        return {
          content: [
            {
              type: 'text',
              text: JSON.stringify(response, null, 2),
            },
          ],
        };
      }

      case 'umicp_stats': {
        const stats = bridge.getStats();
        return {
          content: [
            {
              type: 'text',
              text: JSON.stringify(stats, null, 2),
            },
          ],
        };
      }

      case 'umicp_connections': {
        const connections = bridge.listConnections();
        return {
          content: [
            {
              type: 'text',
              text: JSON.stringify({ connections }, null, 2),
            },
          ],
        };
      }

      case 'umicp_close_connection': {
        const { host, port } = args as { host: string; port: number };
        const closed = await bridge.closeConnection(host, port);
        return {
          content: [
            {
              type: 'text',
              text: JSON.stringify({ closed, host, port }, null, 2),
            },
          ],
        };
      }

      case 'umicp_reset_stats': {
        bridge.resetStats();
        return {
          content: [
            {
              type: 'text',
              text: JSON.stringify({ message: 'Statistics reset successfully' }, null, 2),
            },
          ],
        };
      }

      default:
        throw new Error(`Unknown tool: ${name}`);
    }
  } catch (error) {
    const errorMessage = error instanceof Error ? error.message : String(error);
    return {
      content: [
        {
          type: 'text',
          text: JSON.stringify({ error: errorMessage }, null, 2),
        },
      ],
      isError: true,
    };
  }
});

// Graceful shutdown
async function shutdown() {
  console.error('Shutting down UMICP bridge...');
  await bridge.close();
  process.exit(0);
}

process.on('SIGINT', shutdown);
process.on('SIGTERM', shutdown);

// Start server
async function main() {
  const transport = new StdioServerTransport();
  await server.connect(transport);
  console.error('UMICP to MCP Bridge started');
}

main().catch((error) => {
  console.error('Fatal error:', error);
  process.exit(1);
});

