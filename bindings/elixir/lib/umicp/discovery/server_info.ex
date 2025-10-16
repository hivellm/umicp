defmodule UMICP.Discovery.ServerInfo do
  @moduledoc """
  Server information for tool discovery.
  """

  @enforce_keys [:server, :version, :protocol]
  defstruct [
    :server,
    :version,
    :protocol,
    :features,
    :operations_count,
    :mcp_compatible,
    :metadata
  ]

  @type t :: %__MODULE__{
          server: String.t(),
          version: String.t(),
          protocol: String.t(),
          features: [String.t()] | nil,
          operations_count: non_neg_integer() | nil,
          mcp_compatible: boolean() | nil,
          metadata: map() | nil
        }

  @doc """
  Create a new server info.

  ## Examples

      iex> UMICP.Discovery.ServerInfo.new(
      ...>   server: "my-service",
      ...>   version: "1.0.0",
      ...>   protocol: "umicp"
      ...> )
      %UMICP.Discovery.ServerInfo{...}
  """
  @spec new(keyword()) :: t()
  def new(attrs) do
    struct!(__MODULE__, attrs)
  end

  @doc """
  Set features (pipe-friendly).
  """
  @spec with_features(t(), [String.t()]) :: t()
  def with_features(info, features) do
    %{info | features: features}
  end

  @doc """
  Set operations count (pipe-friendly).
  """
  @spec with_operations_count(t(), non_neg_integer()) :: t()
  def with_operations_count(info, count) do
    %{info | operations_count: count}
  end

  @doc """
  Set MCP compatibility (pipe-friendly).
  """
  @spec with_mcp_compatible(t(), boolean()) :: t()
  def with_mcp_compatible(info, compatible) do
    %{info | mcp_compatible: compatible}
  end

  @doc """
  Set metadata (pipe-friendly).
  """
  @spec with_metadata(t(), map()) :: t()
  def with_metadata(info, metadata) do
    %{info | metadata: metadata}
  end

  @doc """
  Convert to map for JSON encoding.
  """
  @spec to_map(t()) :: map()
  def to_map(info) do
    %{
      "server" => info.server,
      "version" => info.version,
      "protocol" => info.protocol
    }
    |> maybe_put("features", info.features)
    |> maybe_put("operations_count", info.operations_count)
    |> maybe_put("mcp_compatible", info.mcp_compatible)
    |> maybe_put("metadata", info.metadata)
  end

  defp maybe_put(map, _key, nil), do: map
  defp maybe_put(map, key, value), do: Map.put(map, key, value)
end

