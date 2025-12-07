# Example UMICP Calls

This document provides examples of how to use the UMICP MCP bridge in Cursor.

## Basic Echo Call

Call a UMICP echo service:

```json
{
  "server": {
    "host": "localhost",
    "port": 8080
  },
  "method": "echo",
  "payload": {
    "message": "Hello, UMICP!"
  }
}
```

## Call with TLS

Connect to a secure UMICP server:

```json
{
  "server": {
    "host": "api.example.com",
    "port": 443,
    "tls": true,
    "timeout": 10000
  },
  "method": "users.get",
  "payload": {
    "id": "user123"
  },
  "metadata": {
    "authorization": "Bearer token123",
    "client-version": "1.0.0"
  }
}
```

## Long-Running Operation

Execute a long-running operation with custom timeout:

```json
{
  "server": {
    "host": "localhost",
    "port": 8080
  },
  "method": "process.largeDataset",
  "payload": {
    "dataset": "dataset-2024"
  },
  "timeout": 120000
}
```

## Multiple Server Connections

The bridge automatically pools connections. You can call different servers:

### Server 1
```json
{
  "server": { "host": "api1.example.com", "port": 8080 },
  "method": "service1.action"
}
```

### Server 2
```json
{
  "server": { "host": "api2.example.com", "port": 8080 },
  "method": "service2.action"
}
```

## Get Bridge Statistics

Check how many calls have been made:

```json
{}
```

Use the `umicp_stats` tool (no parameters needed).

## List Active Connections

See all active server connections:

```json
{}
```

Use the `umicp_connections` tool.

## Close a Connection

Close connection to a specific server:

```json
{
  "host": "localhost",
  "port": 8080
}
```

Use the `umicp_close_connection` tool.

## Reset Statistics

Reset all bridge statistics:

```json
{}
```

Use the `umicp_reset_stats` tool.

## Testing with a Local UMICP Server

If you want to test the bridge, you can start a simple UMICP echo server and then use these examples to test the connection.

### Example Workflow in Cursor

1. Start your UMICP server on localhost:8080
2. In Cursor, use the MCP tool `umicp_call` with:
   ```json
   {
     "server": { "port": 8080 },
     "method": "echo",
     "payload": { "test": "data" }
   }
   ```
3. Check statistics with `umicp_stats`
4. List connections with `umicp_connections`

## Error Handling

If a call fails, the response will contain an error message:

```json
{
  "status": "error",
  "error": "Connection refused",
  "executionTime": 1234
}
```

Common errors:
- **Connection refused**: Server is not running or wrong port
- **Request timeout**: Server took too long to respond
- **Invalid UMICP magic bytes**: Server is not speaking UMICP protocol

