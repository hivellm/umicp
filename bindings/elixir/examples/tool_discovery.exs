#!/usr/bin/env elixir

# Tool Discovery Example for UMICP Elixir SDK

Mix.install([{:jason, "~> 1.4"}])

Code.prepend_path("_build/dev/lib/umicp/ebin")

defmodule ExampleSearchService do
  @moduledoc """
  Example service implementing tool discovery.
  """

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
            "query" => %{"type" => "string", "description" => "Search query"},
            "max_results" => %{"type" => "integer", "default" => 10}
          },
          "required" => ["query"]
        }
      )
      |> OperationSchema.with_title("Search Operation")
      |> OperationSchema.with_description("Performs a semantic search"),
      OperationSchema.new(
        name: "get_document",
        input_schema: %{
          "type" => "object",
          "properties" => %{
            "document_id" => %{"type" => "string"}
          },
          "required" => ["document_id"]
        }
      )
      |> OperationSchema.with_title("Get Document")
    ]
  end

  @impl true
  def server_info do
    ServerInfo.new(
      server: "example-search-service",
      version: "1.0.0",
      protocol: "umicp"
    )
    |> ServerInfo.with_features(["tool_discovery", "native_types", "mcp_compatible"])
    |> ServerInfo.with_operations_count(2)
    |> ServerInfo.with_mcp_compatible(true)
    |> ServerInfo.with_metadata(%{
      "platform" => "elixir",
      "sdk_version" => UMICP.version()
    })
  end
end

defmodule ToolDiscoveryExample do
  alias UMICP.Discovery.DiscoverableService

  def run do
    IO.puts("\n=== Tool Discovery Example ===\n")

    service = ExampleSearchService

    # List all operations
    IO.puts("Available Operations:")
    operations = service.list_operations()

    for op <- operations do
      IO.puts("  - #{op.name}: #{op.title || "No title"}")

      if op.description do
        IO.puts("    Description: #{op.description}")
      end
    end

    # Get specific schema
    IO.puts("\nDetailed Schema for 'search':")
    schema = service.get_schema("search")
    IO.puts("  Name: #{schema.name}")
    IO.puts("  Title: #{schema.title}")
    IO.puts("  Description: #{schema.description}")

    # Get server info
    IO.puts("\nServer Information:")
    info = service.server_info()
    IO.puts("  Server: #{info.server}")
    IO.puts("  Version: #{info.version}")
    IO.puts("  Protocol: #{info.protocol}")
    IO.puts("  Features: #{Enum.join(info.features, ", ")}")
    IO.puts("  Operations Count: #{info.operations_count}")
    IO.puts("  MCP Compatible: #{info.mcp_compatible}")

    # Generate JSON responses
    IO.puts("\nJSON Responses:")

    {:ok, ops_json} = DiscoverableService.generate_operations_response(service)
    IO.puts("\nList Operations Response:")
    IO.puts(ops_json)

    {:ok, info_json} = DiscoverableService.generate_server_info_response(service)
    IO.puts("\nServer Info Response:")
    IO.puts(info_json)

    IO.puts("\n✅ Tool discovery example completed!")
  end
end

ToolDiscoveryExample.run()

