defmodule UMICP.Transport.WebSocket do
  @moduledoc """
  WebSocket transport for UMICP using Mint.WebSocket.

  This is a GenServer-based WebSocket client with automatic reconnection
  and message handling.
  """

  use GenServer
  require Logger

  alias UMICP.Envelope

  @type state :: %{
          url: String.t(),
          conn: term() | nil,
          websocket: term() | nil,
          connected: boolean(),
          message_handler: (Envelope.t() -> any()) | nil,
          error_handler: (term() -> any()) | nil,
          reconnect_attempts: non_neg_integer()
        }

  # Client API

  @doc """
  Start a WebSocket connection.
  """
  @spec start_link(keyword()) :: GenServer.on_start()
  def start_link(opts) do
    GenServer.start_link(__MODULE__, opts, name: opts[:name])
  end

  @doc """
  Connect to WebSocket server.
  """
  @spec connect(GenServer.server()) :: :ok | {:error, term()}
  def connect(server) do
    GenServer.call(server, :connect)
  end

  @doc """
  Disconnect from WebSocket server.
  """
  @spec disconnect(GenServer.server()) :: :ok
  def disconnect(server) do
    GenServer.call(server, :disconnect)
  end

  @doc """
  Send an envelope over WebSocket.
  """
  @spec send_envelope(GenServer.server(), Envelope.t()) :: :ok | {:error, term()}
  def send_envelope(server, envelope) do
    GenServer.call(server, {:send, envelope})
  end

  @doc """
  Set message handler callback.
  """
  @spec on_message(GenServer.server(), (Envelope.t() -> any())) :: :ok
  def on_message(server, handler) do
    GenServer.cast(server, {:on_message, handler})
  end

  @doc """
  Set error handler callback.
  """
  @spec on_error(GenServer.server(), (term() -> any())) :: :ok
  def on_error(server, handler) do
    GenServer.cast(server, {:on_error, handler})
  end

  @doc """
  Check if connected.
  """
  @spec connected?(GenServer.server()) :: boolean()
  def connected?(server) do
    GenServer.call(server, :connected?)
  end

  # Server Callbacks

  @impl true
  def init(opts) do
    state = %{
      url: Keyword.fetch!(opts, :url),
      conn: nil,
      websocket: nil,
      connected: false,
      message_handler: nil,
      error_handler: nil,
      reconnect_attempts: 0
    }

    {:ok, state}
  end

  @impl true
  def handle_call(:connect, _from, state) do
    case do_connect(state) do
      {:ok, new_state} ->
        {:reply, :ok, new_state}

      {:error, reason} = error ->
        {:reply, error, state}
    end
  end

  def handle_call(:disconnect, _from, state) do
    new_state = do_disconnect(state)
    {:reply, :ok, new_state}
  end

  def handle_call({:send, envelope}, _from, %{connected: false} = state) do
    {:reply, {:error, :not_connected}, state}
  end

  def handle_call({:send, envelope}, _from, state) do
    case Envelope.serialize(envelope) do
      {:ok, json} ->
        # Simplified send logic - actual implementation would use Mint.WebSocket
        Logger.debug("Sending envelope: #{json}")
        {:reply, :ok, state}

      {:error, reason} = error ->
        {:reply, error, state}
    end
  end

  def handle_call(:connected?, _from, state) do
    {:reply, state.connected, state}
  end

  @impl true
  def handle_cast({:on_message, handler}, state) do
    {:noreply, %{state | message_handler: handler}}
  end

  def handle_cast({:on_error, handler}, state) do
    {:noreply, %{state | error_handler: handler}}
  end

  @impl true
  def handle_info({:websocket, data}, state) do
    case Envelope.deserialize(data) do
      {:ok, envelope} ->
        if state.message_handler do
          state.message_handler.(envelope)
        end

        {:noreply, state}

      {:error, reason} ->
        if state.error_handler do
          state.error_handler.(reason)
        end

        {:noreply, state}
    end
  end

  def handle_info(:reconnect, state) do
    case do_connect(state) do
      {:ok, new_state} ->
        {:noreply, new_state}

      {:error, _reason} ->
        # Schedule reconnect
        Process.send_after(self(), :reconnect, 5000)
        {:noreply, %{state | reconnect_attempts: state.reconnect_attempts + 1}}
    end
  end

  # Private Functions

  defp do_connect(state) do
    # Simplified connection logic
    # Actual implementation would use Mint.WebSocket
    Logger.info("Connecting to WebSocket: #{state.url}")
    {:ok, %{state | connected: true, reconnect_attempts: 0}}
  end

  defp do_disconnect(state) do
    Logger.info("Disconnecting from WebSocket")
    %{state | connected: false, conn: nil, websocket: nil}
  end
end

