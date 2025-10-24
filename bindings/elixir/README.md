# UMICP Elixir SDK

[![Hex.pm](https://img.shields.io/hexpm/v/umicp.svg)](https://hex.pm/packages/umicp)
[![Elixir 1.15+](https://img.shields.io/badge/Elixir-1.15+-purple.svg)](https://elixir-lang.org)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Elixir bindings for the Universal Matrix Inter-Communication Protocol (UMICP), providing high-performance communication and matrix operations for distributed systems, federated learning, and real-time applications.

## 🚀 Features

- **Native Elixir Terms**: Full support for native Elixir terms in capabilities (no wrappers needed)
- **MCP-Compatible Tool Discovery**: JSON Schema support for operation definitions
- **OTP Integration**: GenServer-based transport with supervision
- **Type Safety**: Struct-based types with @type specs
- **WebSocket & HTTP/2**: Multiple transport options with Mint/Finch
- **Matrix Operations**: High-performance vector and matrix operations
- **Pipe-Friendly**: Idiomatic Elixir with pipe operator support
- **Well Tested**: Comprehensive ExUnit test suite (100+ tests)

## 📦 Installation

Add `umicp` to your `mix.exs` dependencies:

```elixir
def deps do
  [
    {:umicp_sdk, "~> 0.2.0"}
  ]
end
```

Then run:

```bash
mix deps.get
```

## 🏃 Quick Start

### Basic Envelope Usage

```elixir
alias UMICP.Envelope

# Create an envelope with pipe syntax
{:ok, envelope} =
  Envelope.builder()
  |> Envelope.from("client-001")
  |> Envelope.to("server-001")
  |> Envelope.operation(:data)
  |> Envelope.message_id("msg-#{System.system_time()}")
  |> Envelope.capability("content-type", "application/json")
  |> Envelope.build_with_hash()

# Serialize to JSON
{:ok, json} = Envelope.serialize(envelope)

# Deserialize from JSON
{:ok, restored} = Envelope.deserialize(json)

# Verify hash
Envelope.verify_hash(restored)  # true
```

### Native Elixir Terms in Capabilities

```elixir
{:ok, envelope} =
  Envelope.builder()
  |> Envelope.from("client")
  |> Envelope.to("server")
  |> Envelope.operation(:request)
  |> Envelope.message_id("req-001")
  |> Envelope.capability("action", "search")
  |> Envelope.capability("max_tokens", 100)           # Native integer
  |> Envelope.capability("streaming", true)           # Native boolean
  |> Envelope.capability("temperature", 0.7)          # Native float
  |> Envelope.capability("filters", %{"category" => "tech"})  # Native map
  |> Envelope.capability("tags", ["ai", "ml"])        # Native list
  |> Envelope.build()

# Access capabilities
Envelope.get_capability(envelope, "max_tokens")  # 100
Envelope.get_capability(envelope, "streaming")   # true
```

### Tool Discovery (MCP-Compatible)

```elixir
defmodule MyService do
  use UMICP.Discovery.DiscoverableService

  alias UMICP.Discovery.{OperationSchema, ServerInfo}

  @impl true
  def list_operations do
    [
      OperationSchema.new(
        name: "search",
        input_schema: %{
          "type" => "object",
          "properties" => %{
            "query" => %{"type" => "string"},
            "max_results" => %{"type" => "integer", "default" => 10}
          },
          "required" => ["query"]
        }
      )
      |> OperationSchema.with_title("Search Operation")
      |> OperationSchema.with_description("Performs a semantic search")
    ]
  end

  @impl true
  def server_info do
    ServerInfo.new(
      server: "my-service",
      version: "1.0.0",
      protocol: "umicp"
    )
    |> ServerInfo.with_features(["tool_discovery", "native_types"])
    |> ServerInfo.with_mcp_compatible(true)
  end
end

# Use the service
operations = MyService.list_operations()
info = MyService.server_info()

# Generate JSON responses
{:ok, json} = UMICP.Discovery.DiscoverableService.generate_operations_response(MyService)
```

### WebSocket Transport

```elixir
# Start WebSocket connection
{:ok, ws} = UMICP.Transport.WebSocket.start_link(
  url: "ws://localhost:8080/umicp",
  name: :my_websocket
)

# Set message handler
:ok = UMICP.Transport.WebSocket.on_message(:my_websocket, fn envelope ->
  IO.puts("Received: #{envelope.from}")
end)

# Connect
:ok = UMICP.Transport.WebSocket.connect(:my_websocket)

# Send envelope
{:ok, envelope} =
  Envelope.builder()
  |> Envelope.from("client")
  |> Envelope.to("server")
  |> Envelope.operation(:data)
  |> Envelope.message_id("msg-001")
  |> Envelope.build()

:ok = UMICP.Transport.WebSocket.send_envelope(:my_websocket, envelope)
```

### Matrix Operations

```elixir
alias UMICP.Matrix.Operations

# Vector operations
v1 = [1.0, 2.0, 3.0, 4.0]
v2 = [5.0, 6.0, 7.0, 8.0]

{:ok, sum} = Operations.vector_add(v1, v2)           # [6, 8, 10, 12]
{:ok, dot} = Operations.dot_product(v1, v2)          # 70.0

# Matrix multiplication
m1 = [1.0, 2.0, 3.0, 4.0]  # 2x2 matrix
m2 = [5.0, 6.0, 7.0, 8.0]  # 2x2 matrix
{:ok, result} = Operations.matrix_multiply(m1, m2, 2, 2, 2)

# Vector normalization
{:ok, normalized} = Operations.vector_normalize([3.0, 4.0])  # [0.6, 0.8]
```

## 🧪 Testing

Run tests:

```bash
mix test
```

Run tests with coverage:

```bash
mix coveralls
```

### Test Coverage

- **Core Tests** (30+ tests): Envelope, Types, Validation
- **Discovery Tests** (25+ tests): Tool discovery, MCP compatibility
- **Matrix Tests** (25+ tests): Vector and matrix operations
- **Integration Tests** (20+ tests): End-to-end workflows

Total: **100+ comprehensive tests**

## 📚 API Reference

### Core Types

#### `UMICP.Envelope`

```elixir
@type t :: %UMICP.Envelope{
  from: String.t(),
  to: String.t(),
  operation: OperationType.t(),
  message_id: String.t(),
  capabilities: %{String.t() => term()},
  payload: binary() | nil,
  hash: String.t() | nil
}
```

#### `UMICP.OperationType`

```elixir
@type t :: :data | :control | :ack | :error | :request | :response
```

### Discovery

#### `UMICP.Discovery.DiscoverableService`

Behavior for services with tool discovery:

```elixir
@callback list_operations() :: [OperationSchema.t()]
@callback get_schema(name :: String.t()) :: OperationSchema.t() | nil
@callback server_info() :: ServerInfo.t()
```

### Transport

#### `UMICP.Transport.WebSocket`

GenServer-based WebSocket client:

```elixir
start_link(url: String.t()) :: GenServer.on_start()
connect(server) :: :ok | {:error, term()}
send_envelope(server, Envelope.t()) :: :ok | {:error, term()}
on_message(server, (Envelope.t() -> any())) :: :ok
```

#### `UMICP.Transport.HTTP2`

HTTP/2 transport using Finch:

```elixir
send(base_url, Envelope.t(), keyword()) :: {:ok, Envelope.t()} | {:error, term()}
get(base_url, endpoint) :: {:ok, Envelope.t()} | {:error, term()}
```

### Matrix Operations

```elixir
vector_add([float()], [float()]) :: {:ok, [float()]} | {:error, term()}
vector_subtract([float()], [float()]) :: {:ok, [float()]} | {:error, term()}
vector_scale([float()], float()) :: [float()]
dot_product([float()], [float()]) :: {:ok, float()} | {:error, term()}
matrix_multiply([float()], [float()], pos_integer(), pos_integer(), pos_integer()) ::
  {:ok, [float()]} | {:error, term()}
vector_magnitude([float()]) :: float()
vector_normalize([float()]) :: {:ok, [float()]} | {:error, term()}
```

## 🎯 Use Cases

### Federated Learning

```elixir
weights_envelope =
  Envelope.builder()
  |> Envelope.from("coordinator")
  |> Envelope.to("worker-001")
  |> Envelope.operation(:data)
  |> Envelope.message_id("weights-#{System.system_time()}")
  |> Envelope.capability("model-version", "1.0")
  |> Envelope.capability("layer-count", 12)
  |> Envelope.payload(model_weights_binary)
  |> Envelope.build_with_hash()
```

### IoT Data Streaming

```elixir
sensor_data =
  Envelope.builder()
  |> Envelope.from("sensor-temp-001")
  |> Envelope.to("data-collector")
  |> Envelope.operation(:data)
  |> Envelope.message_id("temp-#{System.system_time()}")
  |> Envelope.capability("sensor-type", "temperature")
  |> Envelope.capability("value", 25.3)
  |> Envelope.capability("unit", "celsius")
  |> Envelope.build()
```

## 📈 Performance

- **Envelope Creation**: <1ms
- **Serialization**: ~1ms for complex envelopes
- **Matrix Operations**: Pure Elixir with optional Nx acceleration
- **OTP Integration**: Supervision, GenServer patterns

## 🛠️ Development

### Build

```bash
mix compile
```

### Run Examples

```bash
mix run examples/basic_envelope.exs
mix run examples/tool_discovery.exs
mix run examples/matrix_operations.exs
```

### Generate Documentation

```bash
mix docs
```

## 📄 License

MIT License - see [LICENSE](LICENSE) for details

## 🤝 Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## 🔗 Related Projects

- **UMICP Core (C++)**: https://github.com/hivellm/umicp/tree/main/cpp
- **UMICP Rust**: https://github.com/hivellm/umicp/tree/main/bindings/rust
- **UMICP Swift**: https://github.com/hivellm/umicp/tree/main/bindings/swift
- **UMICP TypeScript**: https://github.com/hivellm/umicp/tree/main/bindings/typescript

---

**UMICP Elixir v0.2.0** - Universal Matrix Inter-Communication Protocol for Elixir

Built with ❤️ by the HiveLLM AI Collaborative Team

