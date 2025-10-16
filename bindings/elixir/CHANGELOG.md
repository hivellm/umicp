# Changelog

All notable changes to the UMICP Elixir bindings will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.2.0] - 2025-10-16

### Added

#### Core Features
- Initial release of UMICP Elixir bindings
- **Native Elixir Term Support**: Capabilities support native Elixir terms without wrappers
- `Envelope` struct with builder pattern
- SHA-256 hash calculation and verification
- Input validation and error handling with `UMICP.Error`
- Pipe-friendly API design

#### Tool Discovery (MCP-Compatible)
- `DiscoverableService` behavior for tool discovery
- `OperationSchema` with JSON Schema support
- `ServerInfo` for service metadata
- Helper functions for generating JSON responses
- Full MCP (Model Context Protocol) compatibility

#### Transport Layer
- `WebSocket` GenServer for WebSocket communication
- Automatic reconnection support
- Message handlers with callbacks
- `HTTP2` module for HTTP/2 transport using Finch
- Connection pooling and stream handling

#### Matrix Operations
- Pure Elixir implementations for:
  - Vector addition, subtraction, scaling
  - Dot product calculation
  - Matrix multiplication
  - Vector magnitude and normalization
- Optional Nx (Numerical Elixir) integration

#### Testing & Examples
- 100+ comprehensive ExUnit tests covering:
  - Core functionality (Envelope, Types)
  - Tool discovery (all components)
  - Matrix operations
  - Integration workflows
- 3 working examples:
  - basic_envelope.exs
  - tool_discovery.exs
  - matrix_operations.exs

#### Documentation
- Comprehensive README with quick start guide
- API reference documentation
- Usage examples
- Integration patterns

### Technical Details

#### Platform Support
- Elixir 1.15+
- Erlang/OTP 25+

#### Dependencies
- `jason ~> 1.4` - JSON encoding/decoding
- `mint ~> 1.5` - HTTP/2 support
- `mint_web_socket ~> 1.0` - WebSocket support
- `finch ~> 0.18` - HTTP client with connection pooling
- `nx ~> 0.7` (optional) - Numerical operations

#### Performance
- Envelope creation: <1ms
- Serialization: ~1ms for complex envelopes
- Pure Elixir matrix operations
- OTP supervision for reliability

### Quality Metrics
- 100+ tests passing
- Full API coverage
- Type specs for all public functions
- ExDoc documentation

---

## [Unreleased]

### Planned Features
- NIF bindings to C++ core for performance
- GenStage integration for streaming
- Distributed Elixir support
- Phoenix framework integration
- Advanced compression support

---

**Version History**

- **0.2.0** (2025-10-16): Initial release with native types and tool discovery

