# @hivellm/umicp2mcp

MCP Bridge for UMICP - Execute UMICP calls through Model Context Protocol.

## Overview

This package provides a bridge between the Model Context Protocol (MCP) and UMICP servers, allowing you to test and interact with UMICP implementations directly from Cursor or any MCP-compatible client.

## Installation

```bash
npm install -g @hivellm/umicp2mcp
```

Or run directly with npx:

```bash
npx @hivellm/umicp2mcp
```

## Usage

### Running the MCP Server

Start the bridge server:

```bash
npx @hivellm/umicp2mcp
```

### Configure in Cursor

Add to your MCP settings (`.cursor/mcp.json` or similar):

```json
{
  "mcpServers": {
    "umicp": {
      "command": "npx",
      "args": ["@hivellm/umicp2mcp"]
    }
  }
}
```

### Available Tools

#### umicp_call

Execute a UMICP call to a remote server.

**Parameters:**
- `server` (object, required):
  - `host` (string): Server hostname or IP (default: "localhost")
  - `port` (number, required): Server port
  - `timeout` (number): Connection timeout in ms (default: 30000)
  - `tls` (boolean): Enable TLS/SSL (default: false)
- `method` (string, required): Method to call on the UMICP server
- `payload` (object): Request payload
- `metadata` (object): Request metadata (key-value pairs)
- `timeout` (number): Request timeout in ms (default: 30000)

**Example:**
```json
{
  "server": {
    "host": "localhost",
    "port": 8080
  },
  "method": "echo",
  "payload": {
    "message": "Hello, UMICP!"
  },
  "metadata": {
    "client": "cursor"
  }
}
```

#### umicp_stats

Get bridge statistics including total calls, success/failure rates, and average execution time.

#### umicp_connections

List all active UMICP server connections.

#### umicp_close_connection

Close a specific server connection.

**Parameters:**
- `host` (string, required): Server hostname
- `port` (number, required): Server port

#### umicp_reset_stats

Reset all bridge statistics.

## Development

### Build

```bash
npm run build
```

### Watch Mode

```bash
npm run dev
```

### Clean

```bash
npm run clean
```

## Architecture

The bridge consists of four main components:

1. **MCP Server** (`src/index.ts`): Handles MCP protocol communication via stdio
2. **UMICP Bridge** (`src/bridge.ts`): Manages UMICP client connections and calls
3. **HTTP Client** (`src/http-client.ts`): HTTP transport for web-based UMICP servers
4. **TCP Client** (`src/client.ts`): Raw TCP socket transport for native UMICP servers
5. **Type Definitions** (`src/types.ts`): TypeScript interfaces and types

### Transport Flow

```
Cursor/MCP Client
      ↓
   MCP Server (stdio)
      ↓
  UMICP Bridge
      ↓
   Port Detection
    ↙     ↘
HTTP Client  TCP Client
    ↓           ↓
  Synap      Native UMICP
 (15500)      (8080)
```

## Features

- ✅ **Dual Transport Support**: HTTP and TCP sockets
- ✅ **Auto-detection**: Automatically chooses HTTP or TCP based on port
- ✅ **UMICP v2.0 Compatible**: Proper envelope format with Rust compatibility
- ✅ Execute UMICP calls through MCP
- ✅ Connection pooling (reuses existing connections)
- ✅ Configurable timeouts
- ✅ TLS/SSL support (both HTTP and TCP)
- ✅ Request/response metadata
- ✅ Bridge statistics tracking
- ✅ Graceful shutdown
- ✅ Error handling

### Transport Selection

The bridge automatically selects the appropriate transport:

- **HTTP Transport**: Used for ports 80, 443, or 10000-19999 (typical HTTP ports)
  - Connects to `/umicp` endpoint
  - Stateless (no persistent connection)
  - Perfect for web-based UMICP servers like Synap

- **TCP Transport**: Used for all other ports
  - Raw TCP socket connection
  - Persistent connection
  - Lower latency for high-frequency calls

## Testing

### Testing with Synap (HTTP Transport)

The bridge has been tested and verified with [Synap](https://github.com/hivellm/hivellm/tree/main/synap), a high-performance messaging system with UMICP support.

1. Start Synap server:
```bash
cd synap
cargo run --release --bin synap-server
```

2. In Cursor, test all 8 Synap operations via UMICP:

**KV Store:**
```json
// SET
{ "server": { "port": 15500 }, "method": "synap_kv_set", "payload": { "key": "test", "value": "works!", "ttl": 3600 } }

// GET
{ "server": { "port": 15500 }, "method": "synap_kv_get", "payload": { "key": "test", "type": "string" } }

// DELETE
{ "server": { "port": 15500 }, "method": "synap_kv_delete", "payload": { "key": "test" } }

// SCAN
{ "server": { "port": 15500 }, "method": "synap_kv_scan", "payload": { "prefix": "user:", "limit": 10 } }
```

**Queue:**
```json
{ "server": { "port": 15500 }, "method": "synap_queue_publish", "payload": { "queue": "tasks", "message": "job data", "priority": 5 } }

{ "server": { "port": 15500 }, "method": "synap_queue_consume", "payload": { "queue": "tasks", "consumer_id": "worker-1" } }
```

**Stream:**
```json
{ "server": { "port": 15500 }, "method": "synap_stream_publish", "payload": { "room": "chat", "event": "msg", "data": { "text": "hi" } } }
```

**Pub/Sub:**
```json
{ "server": { "port": 15500 }, "method": "synap_pubsub_publish", "payload": { "topic": "notifications", "message": { "type": "alert" } } }
```

### Test Results

All 8 Synap operations tested successfully:
- ✅ `synap_kv_set` - Stores key-value pairs
- ✅ `synap_kv_get` - Retrieves values
- ✅ `synap_kv_delete` - Deletes keys
- ✅ `synap_kv_scan` - Scans by prefix
- ✅ `synap_queue_publish` - Publishes to queue
- ✅ `synap_queue_consume` - Consumes from queue
- ✅ `synap_stream_publish` - Publishes to stream
- ✅ `synap_pubsub_publish` - Publishes to topic

**Stats:** 7 calls, 100% success rate, 9ms average latency

## Examples

### Synap KV Store (HTTP Transport)

```json
{
  "server": { "host": "localhost", "port": 15500 },
  "method": "synap_kv_set",
  "payload": {
    "key": "my-key",
    "value": "Hello UMICP!",
    "ttl": 3600
  }
}
```

```json
{
  "server": { "host": "localhost", "port": 15500 },
  "method": "synap_kv_get",
  "payload": {
    "key": "my-key",
    "type": "string"
  }
}
```

### Synap Queue (HTTP Transport)

```json
{
  "server": { "host": "localhost", "port": 15500 },
  "method": "synap_queue_publish",
  "payload": {
    "queue": "my-queue",
    "message": "Task data",
    "priority": 5
  }
}
```

### Synap Stream (HTTP Transport)

```json
{
  "server": { "host": "localhost", "port": 15500 },
  "method": "synap_stream_publish",
  "payload": {
    "room": "chat",
    "event": "message",
    "data": { "text": "Hello!" }
  }
}
```

### TCP Socket Server

```json
{
  "server": { "port": 8080 },
  "method": "echo",
  "payload": { "message": "test" }
}
```

### Call with Metadata

```json
{
  "server": { "host": "api.example.com", "port": 443, "tls": true },
  "method": "users.list",
  "metadata": { "authorization": "Bearer token123" }
}
```

### Custom Timeout

```json
{
  "server": { "port": 15500, "timeout": 5000 },
  "method": "synap_kv_scan",
  "payload": { "prefix": "user:", "limit": 100 },
  "timeout": 60000
}
```

## License

MIT

## Contributing

Contributions are welcome! Please read the contributing guidelines before submitting PRs.

## Links

- [UMICP Protocol](https://github.com/hivellm/hivellm/tree/main/umicp)
- [Model Context Protocol](https://modelcontextprotocol.io/)
- [HiveLLM](https://github.com/hivellm/hivellm)

