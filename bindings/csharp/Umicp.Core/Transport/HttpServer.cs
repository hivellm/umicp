using System.Collections.Concurrent;
using System.Net;
using System.Text;
using System.Text.Json;
using Umicp.SDK.Events;
using Umicp.SDK.Types;
using Umicp.SDK.Exceptions;

namespace Umicp.SDK.Transport;

/// <summary>
/// HTTP server transport for UMICP
/// </summary>
public class HttpServer : IDisposable
{
    private readonly HttpListener _httpListener;
    private readonly ConcurrentDictionary<string, Func<HttpListenerContext, Task>> _routes = new();
    private readonly EventEmitter _events = new();
    private readonly TransportStats _stats = new();
    private ConnectionState _state = ConnectionState.Disconnected;
    private CancellationTokenSource? _listenerCts;
    private Task? _listenerTask;

    public ConnectionState State => _state;
    public TransportStats Stats => _stats;
    public EventEmitter Events => _events;

    /// <summary>
    /// Initialize HTTP server
    /// </summary>
    /// <param name="host">Host address (default: localhost)</param>
    /// <param name="port">Port number</param>
    public HttpServer(string host = "localhost", int port = 3000)
    {
        var prefix = $"http://{host}:{port}/";
        _httpListener = new HttpListener();
        _httpListener.Prefixes.Add(prefix);
    }

    /// <summary>
    /// Start the HTTP server
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

            // Start handling requests
            _listenerCts = new CancellationTokenSource();
            _listenerTask = Task.Run(() => HandleRequestsLoop(_listenerCts.Token), _listenerCts.Token);

            _events.Emit(EventType.Connect, new Dictionary<string, object>
            {
                ["message"] = "HTTP server started"
            });
        }
        catch (Exception ex)
        {
            _state = ConnectionState.Error;
            _stats.Errors++;
            throw new TransportException($"Failed to start HTTP server: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Stop the HTTP server
    /// </summary>
    public async Task StopAsync(CancellationToken cancellationToken = default)
    {
        if (_state == ConnectionState.Disconnected)
            return;

        try
        {
            _state = ConnectionState.Disconnecting;

            _listenerCts?.Cancel();
            if (_listenerTask != null)
                await _listenerTask;

            _httpListener.Stop();
            _state = ConnectionState.Disconnected;

            _events.Emit(EventType.Disconnect);
        }
        catch (Exception ex)
        {
            _state = ConnectionState.Error;
            _stats.Errors++;
            throw new TransportException($"Failed to stop HTTP server: {ex.Message}", ex);
        }
        finally
        {
            _listenerCts?.Dispose();
            _listenerCts = null;
        }
    }

    /// <summary>
    /// Register a route handler
    /// </summary>
    /// <param name="path">URL path (e.g., "/api/data")</param>
    /// <param name="handler">Request handler function</param>
    public void AddRoute(string path, Func<HttpListenerContext, Task> handler)
    {
        _routes[path.ToLower()] = handler;
    }

    /// <summary>
    /// Register a GET route with JSON response
    /// </summary>
    public void Get(string path, Func<Task<object>> handler)
    {
        AddRoute(path, async context =>
        {
            if (context.Request.HttpMethod != "GET")
            {
                await SendErrorAsync(context, 405, "Method not allowed");
                return;
            }

            try
            {
                var result = await handler();
                await SendJsonAsync(context, result);
            }
            catch (Exception ex)
            {
                await SendErrorAsync(context, 500, ex.Message);
            }
        });
    }

    /// <summary>
    /// Register a POST route with JSON request/response
    /// </summary>
    public void Post<TRequest>(string path, Func<TRequest, Task<object>> handler)
    {
        AddRoute(path, async context =>
        {
            if (context.Request.HttpMethod != "POST")
            {
                await SendErrorAsync(context, 405, "Method not allowed");
                return;
            }

            try
            {
                // Read request body
                using var reader = new StreamReader(context.Request.InputStream, context.Request.ContentEncoding);
                var body = await reader.ReadToEndAsync();

                var request = JsonSerializer.Deserialize<TRequest>(body, new JsonSerializerOptions
                {
                    PropertyNameCaseInsensitive = true
                });

                if (request == null)
                {
                    await SendErrorAsync(context, 400, "Invalid request body");
                    return;
                }

                var result = await handler(request);
                await SendJsonAsync(context, result);
            }
            catch (JsonException ex)
            {
                await SendErrorAsync(context, 400, $"Invalid JSON: {ex.Message}");
            }
            catch (Exception ex)
            {
                await SendErrorAsync(context, 500, ex.Message);
            }
        });
    }

    /// <summary>
    /// Handle requests loop
    /// </summary>
    private async Task HandleRequestsLoop(CancellationToken cancellationToken)
    {
        while (!cancellationToken.IsCancellationRequested)
        {
            try
            {
                var context = await _httpListener.GetContextAsync();
                _ = Task.Run(() => HandleRequest(context), cancellationToken);
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
                    ["context"] = "handle_requests"
                });
            }
        }
    }

    /// <summary>
    /// Handle individual HTTP request
    /// </summary>
    private async Task HandleRequest(HttpListenerContext context)
    {
        try
        {
            _stats.MessagesReceived++;
            _stats.LastActivity = DateTime.UtcNow;

            var path = context.Request.Url?.AbsolutePath.ToLower() ?? "/";

            _events.Emit(EventType.DataReceived, new Dictionary<string, object>
            {
                ["method"] = context.Request.HttpMethod,
                ["path"] = path,
                ["remote_endpoint"] = context.Request.RemoteEndPoint?.ToString() ?? "unknown"
            });

            if (_routes.TryGetValue(path, out var handler))
            {
                await handler(context);
            }
            else
            {
                await SendErrorAsync(context, 404, "Not found");
            }
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            _events.Emit(EventType.Error, new Dictionary<string, object>
            {
                ["error"] = ex.Message
            });

            try
            {
                await SendErrorAsync(context, 500, "Internal server error");
            }
            catch { }
        }
    }

    /// <summary>
    /// Send JSON response
    /// </summary>
    public async Task SendJsonAsync(HttpListenerContext context, object data, int statusCode = 200, CancellationToken cancellationToken = default)
    {
        try
        {
            var json = JsonSerializer.Serialize(data, new JsonSerializerOptions
            {
                PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
                WriteIndented = false
            });

            var bytes = Encoding.UTF8.GetBytes(json);

            context.Response.StatusCode = statusCode;
            context.Response.ContentType = "application/json";
            context.Response.ContentLength64 = bytes.Length;
            context.Response.Headers.Add("Access-Control-Allow-Origin", "*");

            await context.Response.OutputStream.WriteAsync(bytes, 0, bytes.Length, cancellationToken).ConfigureAwait(false);
            context.Response.Close();

            _stats.MessagesSent++;
            _stats.BytesSent += bytes.Length;
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            throw new TransportException($"Failed to send JSON response: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Send error response
    /// </summary>
    public async Task SendErrorAsync(HttpListenerContext context, int statusCode, string message, CancellationToken cancellationToken = default)
    {
        try
        {
            var error = new
            {
                error = message,
                statusCode = statusCode,
                timestamp = DateTime.UtcNow
            };

            await SendJsonAsync(context, error, statusCode, cancellationToken).ConfigureAwait(false);
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            Console.Error.WriteLine($"Failed to send error response: {ex.Message}");
        }
    }

    /// <summary>
    /// Send binary response
    /// </summary>
    public async Task SendBinaryAsync(HttpListenerContext context, byte[] data, string contentType = "application/octet-stream", CancellationToken cancellationToken = default)
    {
        try
        {
            context.Response.StatusCode = 200;
            context.Response.ContentType = contentType;
            context.Response.ContentLength64 = data.Length;

            await context.Response.OutputStream.WriteAsync(data, 0, data.Length, cancellationToken).ConfigureAwait(false);
            context.Response.Close();

            _stats.MessagesSent++;
            _stats.BytesSent += data.Length;
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            throw new TransportException($"Failed to send binary response: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Get registered routes
    /// </summary>
    public string[] GetRoutes()
    {
        return _routes.Keys.ToArray();
    }

    public void Dispose()
    {
        StopAsync().GetAwaiter().GetResult();
        _httpListener.Close();
    }
}

