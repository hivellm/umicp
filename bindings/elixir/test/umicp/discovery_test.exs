defmodule UMICP.DiscoveryTest do
  use ExUnit.Case, async: true

  alias UMICP.Discovery.{OperationSchema, ServerInfo, DiscoverableService}

  # Mock service for testing
  defmodule MockService do
    use DiscoverableService

    @impl true
    def list_operations do
      [
        OperationSchema.new(
          name: "search",
          input_schema: %{"type" => "object"},
          title: "Search",
          description: "Perform search"
        ),
        OperationSchema.new(
          name: "create",
          input_schema: %{"type" => "object"},
          title: "Create",
          description: "Create item"
        )
      ]
    end

    @impl true
    def server_info do
      ServerInfo.new(
        server: "mock-server",
        version: "1.0.0",
        protocol: "umicp",
        features: ["discovery"],
        operations_count: 2,
        mcp_compatible: true
      )
    end
  end

  describe "OperationSchema" do
    test "new/1 creates schema" do
      schema = OperationSchema.new(
        name: "search",
        input_schema: %{"type" => "object"}
      )

      assert schema.name == "search"
      assert schema.input_schema == %{"type" => "object"}
      assert schema.title == nil
    end

    test "with_title/2 sets title" do
      schema =
        OperationSchema.new(
          name: "search",
          input_schema: %{"type" => "object"}
        )
        |> OperationSchema.with_title("Search Operation")

      assert schema.title == "Search Operation"
    end

    test "with_description/2 sets description" do
      schema =
        OperationSchema.new(
          name: "search",
          input_schema: %{"type" => "object"}
        )
        |> OperationSchema.with_description("Performs a search")

      assert schema.description == "Performs a search"
    end

    test "to_map/1 converts to map" do
      schema = OperationSchema.new(
        name: "search",
        input_schema: %{"type" => "object"},
        title: "Search",
        description: "Search operation"
      )

      map = OperationSchema.to_map(schema)

      assert map["name"] == "search"
      assert map["title"] == "Search"
      assert map["description"] == "Search operation"
      assert map["input_schema"] == %{"type" => "object"}
    end
  end

  describe "ServerInfo" do
    test "new/1 creates server info" do
      info = ServerInfo.new(
        server: "my-service",
        version: "1.0.0",
        protocol: "umicp"
      )

      assert info.server == "my-service"
      assert info.version == "1.0.0"
      assert info.protocol == "umicp"
    end

    test "with_features/2 sets features" do
      info =
        ServerInfo.new(
          server: "my-service",
          version: "1.0.0",
          protocol: "umicp"
        )
        |> ServerInfo.with_features(["discovery", "matrix"])

      assert info.features == ["discovery", "matrix"]
    end

    test "with_operations_count/2 sets count" do
      info =
        ServerInfo.new(
          server: "my-service",
          version: "1.0.0",
          protocol: "umicp"
        )
        |> ServerInfo.with_operations_count(10)

      assert info.operations_count == 10
    end

    test "with_mcp_compatible/2 sets compatibility" do
      info =
        ServerInfo.new(
          server: "my-service",
          version: "1.0.0",
          protocol: "umicp"
        )
        |> ServerInfo.with_mcp_compatible(true)

      assert info.mcp_compatible == true
    end

    test "to_map/1 converts to map" do
      info = ServerInfo.new(
        server: "my-service",
        version: "1.0.0",
        protocol: "umicp",
        features: ["discovery"],
        mcp_compatible: true
      )

      map = ServerInfo.to_map(info)

      assert map["server"] == "my-service"
      assert map["version"] == "1.0.0"
      assert map["features"] == ["discovery"]
      assert map["mcp_compatible"] == true
    end
  end

  describe "DiscoverableService" do
    test "list_operations/0 returns operations" do
      operations = MockService.list_operations()

      assert length(operations) == 2
      assert Enum.at(operations, 0).name == "search"
      assert Enum.at(operations, 1).name == "create"
    end

    test "get_schema/1 finds schema by name" do
      schema = MockService.get_schema("search")

      assert schema != nil
      assert schema.name == "search"
      assert schema.title == "Search"
    end

    test "get_schema/1 returns nil for unknown operation" do
      schema = MockService.get_schema("nonexistent")

      assert schema == nil
    end

    test "server_info/0 returns info" do
      info = MockService.server_info()

      assert info.server == "mock-server"
      assert info.version == "1.0.0"
      assert info.operations_count == 2
      assert info.mcp_compatible == true
    end

    test "generate_operations_response/1 returns JSON" do
      {:ok, json} = DiscoverableService.generate_operations_response(MockService)

      assert is_binary(json)
      assert String.contains?(json, "search")
      assert String.contains?(json, "create")
    end

    test "generate_schema_response/2 returns schema JSON" do
      {:ok, json} = DiscoverableService.generate_schema_response(MockService, "search")

      assert is_binary(json)
      assert String.contains?(json, "search")
      assert String.contains?(json, "Search")
    end

    test "generate_schema_response/2 returns error for unknown operation" do
      {:ok, json} = DiscoverableService.generate_schema_response(MockService, "unknown")

      assert String.contains?(json, "error")
      assert String.contains?(json, "Operation not found")
    end

    test "generate_server_info_response/1 returns JSON" do
      {:ok, json} = DiscoverableService.generate_server_info_response(MockService)

      assert is_binary(json)
      assert String.contains?(json, "mock-server")
      assert String.contains?(json, "1.0.0")
    end
  end
end

