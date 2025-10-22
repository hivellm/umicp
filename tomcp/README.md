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

The bridge consists of three main components:

1. **MCP Server** (`src/index.ts`): Handles MCP protocol communication
2. **UMICP Bridge** (`src/bridge.ts`): Manages UMICP client connections and calls
3. **Type Definitions** (`src/types.ts`): TypeScript interfaces and types

## Features

- ✅ Execute UMICP calls through MCP
- ✅ Connection pooling (reuses existing connections)
- ✅ Configurable timeouts
- ✅ TLS/SSL support
- ✅ Request/response metadata
- ✅ Bridge statistics tracking
- ✅ Graceful shutdown
- ✅ Error handling

## Testing

### Running the Test Server

A simple UMICP echo server is included for testing. To run it:

```bash
cd examples
npm install
npm run test-server
```

The test server will listen on port 8080 and echo back any UMICP requests.

### Testing from Cursor

1. Start the test server
2. In Cursor, use the MCP tool `umicp_call` with:
```json
{
  "server": { "port": 8080 },
  "method": "echo",
  "payload": { "test": "Hello, UMICP!" }
}
```
3. You should receive an echo response

See `examples/example-calls.md` for more examples.

## Examples

### Basic Echo Call

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
  "server": { "port": 8080, "timeout": 5000 },
  "method": "longOperation",
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

