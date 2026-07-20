defmodule UMICP.Discovery.DiscoverableService do
  @moduledoc """
  Behavior for services that support tool discovery (MCP-compatible).
  """

  alias UMICP.Discovery.{OperationSchema, ServerInfo}

  @doc """
  List all available operations.
  """
  @callback list_operations() :: [OperationSchema.t()]

  @doc """
  Get schema for a specific operation.
  """
  @callback get_schema(name :: String.t()) :: OperationSchema.t() | nil

  @doc """
  Get server information.
  """
  @callback server_info() :: ServerInfo.t()

  @doc """
  Generate JSON response for list operations.
  """
  @spec generate_operations_response(module()) :: {:ok, String.t()} | {:error, term()}
  def generate_operations_response(service) do
    operations =
      service.list_operations()
      |> Enum.map(&OperationSchema.to_map/1)

    Jason.encode(operations, pretty: true)
  end

  @doc """
  Generate JSON response for get schema.
  """
  @spec generate_schema_response(module(), String.t()) ::
          {:ok, String.t()} | {:error, term()}
  def generate_schema_response(service, operation_name) do
    case service.get_schema(operation_name) do
      nil ->
        Jason.encode(
          %{
            "error" => "Operation not found",
            "operation" => operation_name
          },
          pretty: true
        )

      schema ->
        Jason.encode(OperationSchema.to_map(schema), pretty: true)
    end
  end

  @doc """
  Generate JSON response for server info.
  """
  @spec generate_server_info_response(module()) :: {:ok, String.t()} | {:error, term()}
  def generate_server_info_response(service) do
    info = service.server_info()
    Jason.encode(ServerInfo.to_map(info), pretty: true)
  end

  @doc """
  Default implementation for get_schema (can be overridden).
  """
  defmacro __using__(_opts) do
    quote do
      @behaviour UMICP.Discovery.DiscoverableService

      @impl true
      def get_schema(name) do
        list_operations()
        |> Enum.find(&(&1.name == name))
      end

      defoverridable get_schema: 1
    end
  end
end

