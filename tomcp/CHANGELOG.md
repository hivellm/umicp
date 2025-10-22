# Changelog

All notable changes to @hivellm/umicp2mcp will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2025-10-22

### Added
- Initial implementation of UMICP to MCP bridge
- MCP server with stdio transport
- **HTTP transport support** for UMICP servers (in addition to TCP sockets)
- **Auto-detection** of HTTP vs TCP transport based on port
- UMICP client connection pooling
- Five MCP tools: umicp_call, umicp_stats, umicp_connections, umicp_close_connection, umicp_reset_stats
- Support for custom timeouts and TLS/SSL
- Request/response metadata handling
- Bridge statistics tracking
- Graceful shutdown handling
- TypeScript type definitions
- **UMICP v2.0 envelope format** with proper Rust compatibility
- **Tested with Synap** (localhost:15500) - all 8 operations working
- Comprehensive README documentation

### Features
- ✅ HTTP transport for web-based UMICP servers
- ✅ TCP socket transport for native UMICP servers
- ✅ Automatic transport selection (ports 10000-19999 use HTTP)
- ✅ Compatible with UMICP v2.0 protocol
- ✅ ISO 8601 timestamps
- ✅ Proper envelope validation

