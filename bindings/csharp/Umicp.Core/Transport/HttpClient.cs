using System.Net.Http.Json;
using System.Text;
using System.Text.Json;
using Umicp.Core.Types;
using Umicp.Core.Events;
using Umicp.Core.Exceptions;

namespace Umicp.Core.Transport;

/// <summary>
/// HTTP client transport for UMICP
/// </summary>
public class HttpClient : IDisposable
{
    private readonly System.Net.Http.HttpClient _httpClient;
    private readonly string _baseUrl;
    private readonly TransportStats _stats = new();
    private readonly EventEmitter _events = new();
    private ConnectionState _state = ConnectionState.Disconnected;

    public ConnectionState State => _state;
    public TransportStats Stats => _stats;
    public EventEmitter Events => _events;

    /// <summary>
    /// Initialize HTTP client
    /// </summary>
    public HttpClient(string baseUrl, TimeSpan? timeout = null)
    {
        _baseUrl = baseUrl.TrimEnd('/');
        _httpClient = new System.Net.Http.HttpClient
        {
            Timeout = timeout ?? TimeSpan.FromSeconds(30)
        };
        _state = ConnectionState.Connected; // HTTP is always "connected"
        _stats.ConnectedAt = DateTime.UtcNow;
    }

    /// <summary>
    /// Send POST request with JSON data
    /// </summary>
    public async Task<T?> PostAsync<T>(string path, object data, CancellationToken cancellationToken = default)
    {
        try
        {
            var url = $"{_baseUrl}/{path.TrimStart('/')}";
            var json = JsonSerializer.Serialize(data);
            var content = new StringContent(json, Encoding.UTF8, "application/json");

            _stats.BytesSent += Encoding.UTF8.GetByteCount(json);
            _stats.MessagesSent++;

            var response = await _httpClient.PostAsync(url, content, cancellationToken);
            response.EnsureSuccessStatusCode();

            var responseData = await response.Content.ReadFromJsonAsync<T>(cancellationToken: cancellationToken);

            if (response.Content.Headers.ContentLength.HasValue)
                _stats.BytesReceived += response.Content.Headers.ContentLength.Value;

            _stats.MessagesReceived++;
            _stats.LastActivity = DateTime.UtcNow;

            _events.Emit(EventType.DataReceived, new Dictionary<string, object>
            {
                ["url"] = url,
                ["method"] = "POST"
            });

            return responseData;
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            _events.Emit(EventType.Error, new Dictionary<string, object>
            {
                ["error"] = ex.Message,
                ["path"] = path
            });
            throw new TransportException($"HTTP POST failed: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Send GET request
    /// </summary>
    public async Task<T?> GetAsync<T>(string path, CancellationToken cancellationToken = default)
    {
        try
        {
            var url = $"{_baseUrl}/{path.TrimStart('/')}";
            _stats.MessagesSent++;

            var response = await _httpClient.GetAsync(url, cancellationToken);
            response.EnsureSuccessStatusCode();

            var data = await response.Content.ReadFromJsonAsync<T>(cancellationToken: cancellationToken);

            if (response.Content.Headers.ContentLength.HasValue)
                _stats.BytesReceived += response.Content.Headers.ContentLength.Value;

            _stats.MessagesReceived++;
            _stats.LastActivity = DateTime.UtcNow;

            _events.Emit(EventType.DataReceived, new Dictionary<string, object>
            {
                ["url"] = url,
                ["method"] = "GET"
            });

            return data;
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            _events.Emit(EventType.Error, new Dictionary<string, object>
            {
                ["error"] = ex.Message,
                ["path"] = path
            });
            throw new TransportException($"HTTP GET failed: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Send binary data via POST
    /// </summary>
    public async Task<byte[]> PostBinaryAsync(string path, byte[] data, CancellationToken cancellationToken = default)
    {
        try
        {
            var url = $"{_baseUrl}/{path.TrimStart('/')}";
            var content = new ByteArrayContent(data);
            content.Headers.ContentType = new System.Net.Http.Headers.MediaTypeHeaderValue("application/octet-stream");

            _stats.BytesSent += data.Length;
            _stats.MessagesSent++;

            var response = await _httpClient.PostAsync(url, content, cancellationToken);
            response.EnsureSuccessStatusCode();

            var responseData = await response.Content.ReadAsByteArrayAsync(cancellationToken);

            _stats.BytesReceived += responseData.Length;
            _stats.MessagesReceived++;
            _stats.LastActivity = DateTime.UtcNow;

            _events.Emit(EventType.DataReceived, new Dictionary<string, object>
            {
                ["url"] = url,
                ["method"] = "POST",
                ["type"] = "binary"
            });

            return responseData;
        }
        catch (Exception ex)
        {
            _stats.Errors++;
            throw new TransportException($"HTTP POST binary failed: {ex.Message}", ex);
        }
    }

    public void Dispose()
    {
        _httpClient.Dispose();
    }
}

