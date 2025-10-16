defmodule UMICP.OperationType do
  @moduledoc """
  Operation types for UMICP envelopes.
  """

  @type t :: :data | :control | :ack | :error | :request | :response

  @doc """
  Convert operation type to integer.
  """
  @spec to_int(t()) :: 0..5
  def to_int(:data), do: 0
  def to_int(:control), do: 1
  def to_int(:ack), do: 2
  def to_int(:error), do: 3
  def to_int(:request), do: 4
  def to_int(:response), do: 5

  @doc """
  Convert integer to operation type.
  """
  @spec from_int(integer()) :: {:ok, t()} | {:error, :invalid_operation_type}
  def from_int(0), do: {:ok, :data}
  def from_int(1), do: {:ok, :control}
  def from_int(2), do: {:ok, :ack}
  def from_int(3), do: {:ok, :error}
  def from_int(4), do: {:ok, :request}
  def from_int(5), do: {:ok, :response}
  def from_int(_), do: {:error, :invalid_operation_type}

  @doc """
  Get all operation types.
  """
  @spec all() :: [t()]
  def all do
    [:data, :control, :ack, :error, :request, :response]
  end
end

defmodule UMICP.Error do
  @moduledoc """
  UMICP error types.
  """

  defexception [:message, :type]

  @type t :: %__MODULE__{
          message: String.t(),
          type: atom()
        }

  @doc """
  Create a new error.
  """
  def new(type, message) do
    %__MODULE__{type: type, message: message}
  end

  @doc """
  Create an invalid envelope error.
  """
  def invalid_envelope(message) do
    new(:invalid_envelope, message)
  end

  @doc """
  Create a validation error.
  """
  def validation_error(message) do
    new(:validation_error, message)
  end

  @doc """
  Create a serialization error.
  """
  def serialization_error(message) do
    new(:serialization_error, message)
  end

  @doc """
  Create a transport error.
  """
  def transport_error(message) do
    new(:transport_error, message)
  end
end

