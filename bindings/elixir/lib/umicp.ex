defmodule UMICP do
  @moduledoc """
  UMICP - Universal Matrix Inter-Communication Protocol

  UMICP provides a universal communication framework for distributed systems,
  federated learning, and real-time applications with built-in matrix operations
  and type-safe messaging.

  ## Features

  - **Native JSON Types**: Full support for native Elixir terms in capabilities
  - **Tool Discovery**: MCP-compatible tool discovery with JSON Schema support
  - **Transport Layer**: WebSocket and HTTP/2 transport
  - **Matrix Operations**: High-performance matrix and vector operations
  - **OTP Integration**: GenServer-based connection management
  - **Type Safety**: Struct-based types with `@type` specs

  ## Quick Start

      # Create an envelope
      envelope = UMICP.Envelope.new(
        from: "client-001",
        to: "server-001",
        operation: :data,
        message_id: "msg-001",
        capabilities: %{
          "content-type" => "application/json",
          "max_tokens" => 100,
          "streaming" => true
        }
      )

      # Calculate hash
      envelope = UMICP.Envelope.calculate_hash(envelope)

      # Serialize to JSON
      {:ok, json} = Jason.encode(envelope)

  ## Tool Discovery

      defmodule MyService do
        @behaviour UMICP.Discovery.DiscoverableService

        @impl true
        def list_operations do
          [
            %UMICP.Discovery.OperationSchema{
              name: "search",
              input_schema: %{"type" => "object"},
              title: "Search",
              description: "Perform a search"
            }
          ]
        end

        @impl true
        def get_schema(name) do
          Enum.find(list_operations(), &(&1.name == name))
        end

        @impl true
        def server_info do
          %UMICP.Discovery.ServerInfo{
            server: "my-service",
            version: "1.0.0",
            protocol: "umicp"
          }
        end
      end

  ## Matrix Operations

      ops = UMICP.Matrix.Operations.new()
      {:ok, result} = UMICP.Matrix.Operations.vector_add(ops, [1.0, 2.0], [3.0, 4.0])
      # result: [4.0, 6.0]
  """

  @version "0.2.0"
  @protocol_version "0.2"

  @doc """
  Returns the UMICP version.
  """
  def version, do: @version

  @doc """
  Returns the UMICP protocol version.
  """
  def protocol_version, do: @protocol_version
end

