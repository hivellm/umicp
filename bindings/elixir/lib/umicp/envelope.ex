defmodule UMICP.Envelope do
  @moduledoc """
  UMICP Envelope for message passing with native Elixir term support.

  Elixir maps naturally support any term as values, providing native JSON
  type support without additional wrappers.
  """

  @enforce_keys [:from, :to, :operation, :message_id]
  @derive {Jason.Encoder, except: [:hash]}
  defstruct [
    :from,
    :to,
    :operation,
    :message_id,
    :hash,
    :payload,
    capabilities: %{}
  ]

  @type capabilities :: %{String.t() => term()}
  @type t :: %__MODULE__{
          from: String.t(),
          to: String.t(),
          operation: UMICP.OperationType.t(),
          message_id: String.t(),
          capabilities: capabilities(),
          payload: binary() | nil,
          hash: String.t() | nil
        }

  @doc """
  Create a new envelope.

  ## Examples

      iex> UMICP.Envelope.new(
      ...>   from: "client",
      ...>   to: "server",
      ...>   operation: :data,
      ...>   message_id: "msg-001"
      ...> )
      %UMICP.Envelope{
        from: "client",
        to: "server",
        operation: :data,
        message_id: "msg-001",
        capabilities: %{},
        payload: nil,
        hash: nil
      }
  """
  @spec new(keyword()) :: t()
  def new(attrs) do
    struct!(__MODULE__, attrs)
  end

  @doc """
  Calculate SHA-256 hash for the envelope.
  """
  @spec calculate_hash(t()) :: t()
  def calculate_hash(envelope) do
    # Create envelope without hash for hashing
    hashable = %{envelope | hash: nil}

    # Convert to JSON with sorted keys
    {:ok, json} = Jason.encode(hashable, pretty: false)

    # Calculate SHA-256
    hash =
      :crypto.hash(:sha256, json)
      |> Base.encode16(case: :lower)

    %{envelope | hash: hash}
  end

  @doc """
  Validate envelope fields.
  """
  @spec validate(t()) :: :ok | {:error, UMICP.Error.t()}
  def validate(envelope) do
    cond do
      envelope.from == "" ->
        {:error, UMICP.Error.validation_error("'from' field cannot be empty")}

      envelope.to == "" ->
        {:error, UMICP.Error.validation_error("'to' field cannot be empty")}

      envelope.message_id == "" ->
        {:error, UMICP.Error.validation_error("'message_id' field cannot be empty")}

      true ->
        :ok
    end
  end

  @doc """
  Verify that the hash matches the envelope content.
  """
  @spec verify_hash(t()) :: boolean()
  def verify_hash(%{hash: nil}), do: false

  def verify_hash(envelope) do
    current_hash = envelope.hash
    recalculated = calculate_hash(envelope)
    recalculated.hash == current_hash
  end

  @doc """
  Serialize envelope to JSON string.
  """
  @spec serialize(t()) :: {:ok, String.t()} | {:error, Jason.EncodeError.t()}
  def serialize(envelope) do
    Jason.encode(envelope)
  end

  @doc """
  Deserialize envelope from JSON string.
  """
  @spec deserialize(String.t()) :: {:ok, t()} | {:error, Jason.DecodeError.t()}
  def deserialize(json) do
    case Jason.decode(json, keys: :atoms) do
      {:ok, data} -> {:ok, struct(__MODULE__, data)}
      error -> error
    end
  end

  @doc """
  Get a capability value.
  """
  @spec get_capability(t(), String.t()) :: term() | nil
  def get_capability(envelope, key) do
    Map.get(envelope.capabilities, key)
  end

  @doc """
  Set a capability value.
  """
  @spec set_capability(t(), String.t(), term()) :: t()
  def set_capability(envelope, key, value) do
    capabilities = Map.put(envelope.capabilities, key, value)
    %{envelope | capabilities: capabilities}
  end

  @doc """
  Builder pattern for creating envelopes.

  ## Examples

      iex> UMICP.Envelope.builder()
      ...> |> UMICP.Envelope.from("client")
      ...> |> UMICP.Envelope.to("server")
      ...> |> UMICP.Envelope.operation(:data)
      ...> |> UMICP.Envelope.message_id("msg-001")
      ...> |> UMICP.Envelope.capability("key", "value")
      ...> |> UMICP.Envelope.build()
      {:ok, %UMICP.Envelope{...}}
  """
  @spec builder() :: map()
  def builder do
    %{
      from: "",
      to: "",
      operation: :data,
      message_id: "",
      capabilities: %{},
      payload: nil
    }
  end

  @spec from(map(), String.t()) :: map()
  def from(builder, value), do: Map.put(builder, :from, value)

  @spec to(map(), String.t()) :: map()
  def to(builder, value), do: Map.put(builder, :to, value)

  @spec operation(map(), UMICP.OperationType.t()) :: map()
  def operation(builder, value), do: Map.put(builder, :operation, value)

  @spec message_id(map(), String.t()) :: map()
  def message_id(builder, value), do: Map.put(builder, :message_id, value)

  @spec capability(map(), String.t(), term()) :: map()
  def capability(builder, key, value) do
    capabilities = Map.put(builder.capabilities, key, value)
    Map.put(builder, :capabilities, capabilities)
  end

  @spec payload(map(), binary() | nil) :: map()
  def payload(builder, value), do: Map.put(builder, :payload, value)

  @doc """
  Build the envelope from builder.
  """
  @spec build(map()) :: {:ok, t()} | {:error, UMICP.Error.t()}
  def build(builder) do
    envelope = new(builder)

    case validate(envelope) do
      :ok -> {:ok, envelope}
      error -> error
    end
  end

  @doc """
  Build the envelope with hash calculation.
  """
  @spec build_with_hash(map()) :: {:ok, t()} | {:error, UMICP.Error.t()}
  def build_with_hash(builder) do
    case build(builder) do
      {:ok, envelope} -> {:ok, calculate_hash(envelope)}
      error -> error
    end
  end
end

