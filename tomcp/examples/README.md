# UMICP MCP Bridge Examples

This directory contains examples and test utilities for the UMICP MCP Bridge.

## Contents

- `cursor-config.json` - Example MCP configuration for Cursor
- `example-calls.md` - Comprehensive examples of UMICP calls
- `test-server.ts` - Simple UMICP echo server for testing
- `package.json` - Dependencies for examples

## Quick Start

### 1. Start the Test Server

```bash
npm install
npm run test-server
```

This will start a simple UMICP echo server on port 8080.

### 2. Configure Cursor

Add the following to your Cursor MCP settings:

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

Or use the local version:

```bash
cd ..
npm link
```

Then in Cursor settings:

```json
{
  "mcpServers": {
    "umicp": {
      "command": "umicp2mcp"
    }
  }
}
```

### 3. Test the Connection

In Cursor, ask the AI to use the `umicp_call` tool:

```
Can you call the UMICP test server with an echo request?
```

Or provide the exact call:

```json
{
  "server": { "port": 8080 },
  "method": "echo",
  "payload": { "message": "Hello from Cursor!" }
}
```

## Test Server Details

The test server implements a minimal UMICP protocol handler that:

- Listens on TCP port 8080
- Accepts UMICP protocol messages
- Echoes back the received payload with metadata
- Includes timestamp and method information in response

### Test Server Response Format

```json
{
  "echo": { ...original payload... },
  "receivedMethod": "method name",
  "timestamp": "2025-10-22T..."
}
```

## More Examples

See `example-calls.md` for comprehensive examples including:

- Basic echo calls
- TLS/SSL connections
- Long-running operations
- Multiple server connections
- Statistics and connection management

## Troubleshooting

### Connection Refused

- Make sure the test server is running
- Check that port 8080 is not blocked by firewall
- Verify you're using the correct host (localhost or 127.0.0.1)

### Invalid UMICP Magic Bytes

- Server is not speaking UMICP protocol
- Make sure you're connecting to a UMICP server
- Check server logs for errors

### Request Timeout

- Server is taking too long to respond
- Increase timeout in request
- Check server logs for performance issues

## Protocol Details

The test server and bridge implement UMICP v1 protocol:

- **Magic Bytes**: `0x55 0x4D 0x49 0x43` ("UMIC")
- **Version**: 1
- **Message Types**: REQUEST (0x01), RESPONSE (0x02), ERROR (0x03)
- **Header Size**: 16 bytes
- **Encoding**: UTF-8 JSON for payload and metadata

### Message Format

```
[0-3]   Magic bytes (4 bytes)
[4]     Version (1 byte)
[5]     Message type (1 byte)
[6-7]   Request ID (2 bytes, little-endian)
[8-11]  Payload length (4 bytes, little-endian)
[12-15] Metadata length (4 bytes, little-endian)
[16-]   Payload (JSON)
[-]     Metadata (JSON)
```

