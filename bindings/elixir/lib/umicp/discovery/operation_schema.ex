defmodule UMICP.Discovery.OperationSchema do
  @moduledoc """
  Operation schema compatible with MCP JSON Schema.
  """

  @enforce_keys [:name, :input_schema]
  defstruct [
    :name,
    :title,
    :description,
    :input_schema,
    :output_schema,
    :annotations
  ]

  @type t :: %__MODULE__{
          name: String.t(),
          title: String.t() | nil,
          description: String.t() | nil,
          input_schema: map(),
          output_schema: map() | nil,
          annotations: map() | nil
        }

  @doc """
  Create a new operation schema.

  ## Examples

      iex> UMICP.Discovery.OperationSchema.new(
      ...>   name: "search",
      ...>   input_schema: %{"type" => "object"}
      ...> )
      %UMICP.Discovery.OperationSchema{...}
  """
  @spec new(keyword()) :: t()
  def new(attrs) do
    struct!(__MODULE__, attrs)
  end

  @doc """
  Set title (pipe-friendly).
  """
  @spec with_title(t(), String.t()) :: t()
  def with_title(schema, title) do
    %{schema | title: title}
  end

  @doc """
  Set description (pipe-friendly).
  """
  @spec with_description(t(), String.t()) :: t()
  def with_description(schema, description) do
    %{schema | description: description}
  end

  @doc """
  Set output schema (pipe-friendly).
  """
  @spec with_output_schema(t(), map()) :: t()
  def with_output_schema(schema, output_schema) do
    %{schema | output_schema: output_schema}
  end

  @doc """
  Set annotations (pipe-friendly).
  """
  @spec with_annotations(t(), map()) :: t()
  def with_annotations(schema, annotations) do
    %{schema | annotations: annotations}
  end

  @doc """
  Convert to map for JSON encoding.
  """
  @spec to_map(t()) :: map()
  def to_map(schema) do
    %{
      "name" => schema.name,
      "input_schema" => schema.input_schema
    }
    |> maybe_put("title", schema.title)
    |> maybe_put("description", schema.description)
    |> maybe_put("output_schema", schema.output_schema)
    |> maybe_put("annotations", schema.annotations)
  end

  defp maybe_put(map, _key, nil), do: map
  defp maybe_put(map, key, value), do: Map.put(map, key, value)
end

