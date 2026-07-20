using System.Net.WebSockets;
using System.Text;
using Umicp.SDK.Types;
using Umicp.SDK.Events;
using Umicp.SDK.Exceptions;

namespace Umicp.SDK.Transport;

/// <summary>
/// WebSocket client transport for UMICP
/// </summary>
public class WebSocketClient : ITransport, IDisposable
{
    private readonly Uri _uri;
    private ClientWebSocket? _webSocket;
    private ConnectionState _state = ConnectionState.Disconnected;
    private readonly TransportStats _stats = new();
    private readonly EventEmitter _events = new();
    private CancellationTokenSource? _receiveCts;
    private Task? _receiveTask;

    public ConnectionState State => _state;
    public TransportStats Stats => _stats;
    public EventEmitter Events => _events;

    /// <summary>
    /// Initialize WebSocket client
    /// </summary>
    public WebSocketClient(string url)
    {
        _uri = new Uri(url);
    }

    /// <summary>
    /// Connect to WebSocket server
    /// </summary>
    public async Task ConnectAsync(CancellationToken cancellationToken = default)
    {
        if (_state == ConnectionState.Connected)
            throw new TransportException("Already connected");

        try
        {
            _state = ConnectionState.Connecting;
            _events.Emit(EventType.Connect);

            _webSocket = new ClientWebSocket();
            await _webSocket.ConnectAsync(_uri, cancellationToken);

            _state = ConnectionState.Connected;
            _stats.ConnectedAt = DateTime.UtcNow;
            _stats.LastActivity = DateTime.UtcNow;

            // Start receiving messages
            _receiveCts = new CancellationTokenSource();
            _receiveTask = Task.Run(() => ReceiveLoop(_receiveCts.Token), _receiveCts.Token);

            _events.Emit(EventType.Connect, new Dictionary<string, object>
            {
                ["uri"] = _uri.ToString()
            });
        }
        catch (Exception ex)
        {
            _state = ConnectionState.Error;
            _stats.Errors++;
            _events.Emit(EventType.Error, new Dictionary<string, object>
            {
                ["error"] = ex.Message
            });
            throw new TransportException($"Failed to connect: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Disconnect from WebSocket server
    /// </summary>
    public async Task DisconnectAsync(CancellationToken cancellationToken = default)
    {
        if (_state == ConnectionState.Disconnected)
            return;

        try
        {
            _state = ConnectionState.Disconnecting;

            // Cancel receive loop
            _receiveCts?.Cancel();
            if (_receiveTask != null)
                await _receiveTask;

            if (_webSocket?.State == WebSocketState.Open)
            {
                await _webSocket.CloseAsync(
                    WebSocketCloseStatus.NormalClosure,
                    "Closing",
                    cancellationToken);
            }

            _state = ConnectionState.Disconnected;
            _events.Emit(EventType.Disconnect);
        }
        catch (Exception ex)
        {
            _state = ConnectionState.Error;
            _stats.Errors++;
            throw new TransportException($"Failed to disconnect: {ex.Message}", ex);
        }
        finally
        {
            _webSocket?.Dispose();
            _webSocket = null;
            _receiveCts?.Dispose();
            _receiveCts = null;
        }
    }

    /// <summary>
    /// Send binary data
    /// </summary>
    public async Task SendAsync(byte[] data, CancellationToken cancellationToken = default)
    {
        if (_state != ConnectionState.Connected || _webSocket == null)
            throw new TransportException("Not connected");

        try
        {
            await _webSocket.SendAsync(
                new ArraySegment<byte>(data),
                WebSocketMessageType.Binary,
                true,
                cancellationToken);

            _stats.MessagesSent++;
            _stats.BytesSent += data.Length;
            _stats.LastActivity = DateTime.UtcNow;

            _events.Emit(EventType.DataSent, new Dictionary<string, object>
            {
                ["size"] = data.Length
            });
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            throw new TransportException($"Failed to send data: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Send text data
    /// </summary>
    public async Task SendTextAsync(string text, CancellationToken cancellationToken = default)
    {
        if (_state != ConnectionState.Connected || _webSocket == null)
            throw new TransportException("Not connected");

        try
        {
            var bytes = Encoding.UTF8.GetBytes(text);
            await _webSocket.SendAsync(
                new ArraySegment<byte>(bytes),
                WebSocketMessageType.Text,
                true,
                cancellationToken);

            _stats.MessagesSent++;
            _stats.BytesSent += bytes.Length;
            _stats.LastActivity = DateTime.UtcNow;

            _events.Emit(EventType.DataSent, new Dictionary<string, object>
            {
                ["size"] = bytes.Length,
                ["type"] = "text"
            });
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            throw new TransportException($"Failed to send text: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Receive data (blocking)
    /// </summary>
    public async Task<byte[]> ReceiveAsync(CancellationToken cancellationToken = default)
    {
        if (_state != ConnectionState.Connected || _webSocket == null)
            throw new TransportException("Not connected");

        var buffer = new byte[8192];
        using var ms = new MemoryStream();

        try
        {
            WebSocketReceiveResult result;
            do
            {
                result = await _webSocket.ReceiveAsync(
                    new ArraySegment<byte>(buffer),
                    cancellationToken);

                ms.Write(buffer, 0, result.Count);
            }
            while (!result.EndOfMessage);

            var data = ms.ToArray();
            _stats.MessagesReceived++;
            _stats.BytesReceived += data.Length;
            _stats.LastActivity = DateTime.UtcNow;

            return data;
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            throw new TransportException($"Failed to receive data: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Receive loop (runs in background)
    /// </summary>
    private async Task ReceiveLoop(CancellationToken cancellationToken)
    {
        var buffer = new byte[8192];

        while (!cancellationToken.IsCancellationRequested && _webSocket != null)
        {
            try
            {
                using var ms = new MemoryStream();
                WebSocketReceiveResult result;

                do
                {
                    result = await _webSocket.ReceiveAsync(
                        new ArraySegment<byte>(buffer),
                        cancellationToken);

                    if (result.MessageType == WebSocketMessageType.Close)
                    {
                        await DisconnectAsync();
                        return;
                    }

                    ms.Write(buffer, 0, result.Count);
                }
                while (!result.EndOfMessage);

                var data = ms.ToArray();
                _stats.MessagesReceived++;
                _stats.BytesReceived += data.Length;
                _stats.LastActivity = DateTime.UtcNow;

                _events.Emit(EventType.DataReceived, new Dictionary<string, object>
                {
                    ["data"] = data,
                    ["size"] = data.Length,
                    ["type"] = result.MessageType == WebSocketMessageType.Text ? "text" : "binary"
                });
            }
            catch (OperationCanceledException)
            {
                break;
            }
            catch (WebSocketException ex) when (ex.WebSocketErrorCode == WebSocketError.ConnectionClosedPrematurely)
            {
                _state = ConnectionState.Disconnected;
                _events.Emit(EventType.Disconnect, new Dictionary<string, object>
                {
                    ["reason"] = "Connection closed prematurely"
                });
                break;
            }
            catch (Exception ex)
            {
                _stats.Errors++;
                _events.Emit(EventType.Error, new Dictionary<string, object>
                {
                    ["error"] = ex.Message
                });
            }
        }
    }

    public void Dispose()
    {
        _receiveCts?.Cancel();
        _receiveCts?.Dispose();
        _webSocket?.Dispose();
    }
}

