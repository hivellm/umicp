using System.Collections.Concurrent;
using System.Net;
using System.Net.WebSockets;
using System.Text;
using Umicp.Core.Types;
using Umicp.Core.Events;
using Umicp.Core.Exceptions;

namespace Umicp.Core.Transport;

/// <summary>
/// WebSocket server transport for UMICP
/// </summary>
public class WebSocketServer : IDisposable
{
    private readonly HttpListener _httpListener;
    private readonly ConcurrentDictionary<string, WebSocket> _clients = new();
    private readonly EventEmitter _events = new();
    private readonly TransportStats _stats = new();
    private ConnectionState _state = ConnectionState.Disconnected;
    private CancellationTokenSource? _listenerCts;
    private Task? _listenerTask;

    public ConnectionState State => _state;
    public TransportStats Stats => _stats;
    public EventEmitter Events => _events;

    /// <summary>
    /// Number of connected clients
    /// </summary>
    public int ClientCount => _clients.Count;

    /// <summary>
    /// Initialize WebSocket server
    /// </summary>
    /// <param name="host">Host address (default: localhost)</param>
    /// <param name="port">Port number</param>
    public WebSocketServer(string host = "localhost", int port = 8080)
    {
        var prefix = $"http://{host}:{port}/";
        _httpListener = new HttpListener();
        _httpListener.Prefixes.Add(prefix);
    }

    /// <summary>
    /// Start the WebSocket server
    /// </summary>
    public async Task StartAsync(CancellationToken cancellationToken = default)
    {
        if (_state == ConnectionState.Connected)
            throw new TransportException("Server already started");

        try
        {
            _state = ConnectionState.Connecting;
            _httpListener.Start();

            _state = ConnectionState.Connected;
            _stats.ConnectedAt = DateTime.UtcNow;
            _stats.LastActivity = DateTime.UtcNow;

            // Start accepting connections
            _listenerCts = new CancellationTokenSource();
            _listenerTask = Task.Run(() => AcceptConnectionsLoop(_listenerCts.Token), _listenerCts.Token);

            _events.Emit(EventType.Connect, new Dictionary<string, object>
            {
                ["message"] = "Server started"
            });
        }
        catch (Exception ex)
        {
            _state = ConnectionState.Error;
            _stats.Errors++;
            throw new TransportException($"Failed to start server: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Stop the WebSocket server
    /// </summary>
    public async Task StopAsync(CancellationToken cancellationToken = default)
    {
        if (_state == ConnectionState.Disconnected)
            return;

        try
        {
            _state = ConnectionState.Disconnecting;

            // Cancel listener
            _listenerCts?.Cancel();
            if (_listenerTask != null)
                await _listenerTask;

            // Close all client connections
            foreach (var (clientId, socket) in _clients)
            {
                try
                {
                    if (socket.State == WebSocketState.Open)
                    {
                        await socket.CloseAsync(
                            WebSocketCloseStatus.NormalClosure,
                            "Server shutting down",
                            cancellationToken);
                    }
                    socket.Dispose();
                }
                catch { }
            }
            _clients.Clear();

            _httpListener.Stop();
            _state = ConnectionState.Disconnected;

            _events.Emit(EventType.Disconnect);
        }
        catch (Exception ex)
        {
            _state = ConnectionState.Error;
            _stats.Errors++;
            throw new TransportException($"Failed to stop server: {ex.Message}", ex);
        }
        finally
        {
            _listenerCts?.Dispose();
            _listenerCts = null;
        }
    }

    /// <summary>
    /// Send binary data to a specific client
    /// </summary>
    public async Task SendToClientAsync(string clientId, byte[] data, CancellationToken cancellationToken = default)
    {
        if (!_clients.TryGetValue(clientId, out var socket))
            throw new TransportException($"Client {clientId} not found");

        if (socket.State != WebSocketState.Open)
            throw new TransportException($"Client {clientId} is not connected");

        try
        {
            await socket.SendAsync(
                new ArraySegment<byte>(data),
                WebSocketMessageType.Binary,
                true,
                cancellationToken);

            _stats.MessagesSent++;
            _stats.BytesSent += data.Length;
            _stats.LastActivity = DateTime.UtcNow;
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            throw new TransportException($"Failed to send to client: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Send text data to a specific client
    /// </summary>
    public async Task SendTextToClientAsync(string clientId, string text, CancellationToken cancellationToken = default)
    {
        var bytes = Encoding.UTF8.GetBytes(text);

        if (!_clients.TryGetValue(clientId, out var socket))
            throw new TransportException($"Client {clientId} not found");

        if (socket.State != WebSocketState.Open)
            throw new TransportException($"Client {clientId} is not connected");

        try
        {
            await socket.SendAsync(
                new ArraySegment<byte>(bytes),
                WebSocketMessageType.Text,
                true,
                cancellationToken);

            _stats.MessagesSent++;
            _stats.BytesSent += bytes.Length;
            _stats.LastActivity = DateTime.UtcNow;
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            throw new TransportException($"Failed to send to client: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Broadcast data to all connected clients
    /// </summary>
    public async Task BroadcastAsync(byte[] data, CancellationToken cancellationToken = default)
    {
        var tasks = _clients.Values
            .Where(s => s.State == WebSocketState.Open)
            .Select(async socket =>
            {
                try
                {
                    await socket.SendAsync(
                        new ArraySegment<byte>(data),
                        WebSocketMessageType.Binary,
                        true,
                        cancellationToken);
                }
                catch { }
            });

        await Task.WhenAll(tasks);

        _stats.MessagesSent += _clients.Count;
        _stats.BytesSent += data.Length * _clients.Count;
    }

    /// <summary>
    /// Broadcast text to all connected clients
    /// </summary>
    public async Task BroadcastTextAsync(string text, CancellationToken cancellationToken = default)
    {
        var bytes = Encoding.UTF8.GetBytes(text);
        await BroadcastAsync(bytes, cancellationToken);
    }

    /// <summary>
    /// Accept connections loop (runs in background)
    /// </summary>
    private async Task AcceptConnectionsLoop(CancellationToken cancellationToken)
    {
        while (!cancellationToken.IsCancellationRequested)
        {
            try
            {
                var context = await _httpListener.GetContextAsync();

                if (context.Request.IsWebSocketRequest)
                {
                    _ = Task.Run(async () => await HandleClientAsync(context), cancellationToken);
                }
                else
                {
                    context.Response.StatusCode = 400;
                    context.Response.Close();
                }
            }
            catch (OperationCanceledException)
            {
                break;
            }
            catch (Exception ex)
            {
                _stats.Errors++;
                _events.Emit(EventType.Error, new Dictionary<string, object>
                {
                    ["error"] = ex.Message,
                    ["context"] = "accept_connection"
                });
            }
        }
    }

    /// <summary>
    /// Handle individual client connection
    /// </summary>
    private async Task HandleClientAsync(HttpListenerContext context)
    {
        WebSocket? socket = null;
        var clientId = Guid.NewGuid().ToString();

        try
        {
            var wsContext = await context.AcceptWebSocketAsync(null);
            socket = wsContext.WebSocket;

            _clients.TryAdd(clientId, socket);

            _events.Emit(EventType.Connect, new Dictionary<string, object>
            {
                ["client_id"] = clientId,
                ["remote_endpoint"] = context.Request.RemoteEndPoint?.ToString() ?? "unknown"
            });

            await ReceiveLoop(clientId, socket);
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            _events.Emit(EventType.Error, new Dictionary<string, object>
            {
                ["error"] = ex.Message,
                ["client_id"] = clientId
            });
        }
        finally
        {
            _clients.TryRemove(clientId, out _);

            if (socket != null)
            {
                try
                {
                    if (socket.State == WebSocketState.Open)
                    {
                        await socket.CloseAsync(
                            WebSocketCloseStatus.NormalClosure,
                            "Connection closed",
                            CancellationToken.None);
                    }
                    socket.Dispose();
                }
                catch { }
            }

            _events.Emit(EventType.Disconnect, new Dictionary<string, object>
            {
                ["client_id"] = clientId
            });
        }
    }

    /// <summary>
    /// Receive loop for a client
    /// </summary>
    private async Task ReceiveLoop(string clientId, WebSocket socket)
    {
        var buffer = new byte[8192];

        while (socket.State == WebSocketState.Open)
        {
            try
            {
                using var ms = new MemoryStream();
                WebSocketReceiveResult result;

                do
                {
                    result = await socket.ReceiveAsync(new ArraySegment<byte>(buffer), CancellationToken.None);

                    if (result.MessageType == WebSocketMessageType.Close)
                        return;

                    ms.Write(buffer, 0, result.Count);
                }
                while (!result.EndOfMessage);

                var data = ms.ToArray();
                _stats.MessagesReceived++;
                _stats.BytesReceived += data.Length;
                _stats.LastActivity = DateTime.UtcNow;

                _events.Emit(EventType.DataReceived, new Dictionary<string, object>
                {
                    ["client_id"] = clientId,
                    ["data"] = data,
                    ["size"] = data.Length,
                    ["type"] = result.MessageType == WebSocketMessageType.Text ? "text" : "binary"
                });
            }
            catch (WebSocketException)
            {
                break;
            }
            catch (Exception ex)
            {
                _stats.Errors++;
                _events.Emit(EventType.Error, new Dictionary<string, object>
                {
                    ["error"] = ex.Message,
                    ["client_id"] = clientId
                });
                break;
            }
        }
    }

    /// <summary>
    /// Get list of connected client IDs
    /// </summary>
    public string[] GetConnectedClients()
    {
        return _clients.Keys.ToArray();
    }

    public void Dispose()
    {
        StopAsync().GetAwaiter().GetResult();
        _httpListener.Close();
    }
}

