using System.Collections.Concurrent;
using Umicp.Core.Transport;
using Umicp.Core.Events;
using Umicp.Core.Exceptions;

namespace Umicp.Core.Pool;

/// <summary>
/// Connection pool for managing UMICP transports
/// </summary>
public class ConnectionPool : IDisposable
{
    private readonly string _endpoint;
    private readonly PoolConfig _config;
    private readonly ConcurrentBag<WebSocketClient> _availableConnections = new();
    private readonly ConcurrentDictionary<WebSocketClient, DateTime> _activeConnections = new();
    private readonly EventEmitter _events = new();
    private readonly SemaphoreSlim _connectionSemaphore;
    private readonly object _lock = new();
    private int _totalConnections;
    private bool _disposed;

    /// <summary>
    /// Event emitter
    /// </summary>
    public EventEmitter Events => _events;

    /// <summary>
    /// Total connection count
    /// </summary>
    public int TotalConnections => _totalConnections;

    /// <summary>
    /// Available connection count
    /// </summary>
    public int AvailableConnections => _availableConnections.Count;

    /// <summary>
    /// Active connection count
    /// </summary>
    public int ActiveConnections => _activeConnections.Count;

    /// <summary>
    /// Initialize connection pool
    /// </summary>
    public ConnectionPool(string endpoint, PoolConfig? config = null)
    {
        _endpoint = endpoint;
        _config = config ?? new PoolConfig();
        _config.Validate();

        _connectionSemaphore = new SemaphoreSlim(_config.MaxConnections, _config.MaxConnections);
    }

    /// <summary>
    /// Initialize the pool with minimum connections
    /// </summary>
    public async Task InitializeAsync(CancellationToken cancellationToken = default)
    {
        for (int i = 0; i < _config.MinConnections; i++)
        {
            var connection = await CreateConnectionAsync(cancellationToken);
            _availableConnections.Add(connection);
        }
    }

    /// <summary>
    /// Acquire a connection from the pool
    /// </summary>
    public async Task<WebSocketClient> AcquireAsync(CancellationToken cancellationToken = default)
    {
        if (_disposed)
            throw new ObjectDisposedException(nameof(ConnectionPool));

        // Wait for available slot
        var acquired = await _connectionSemaphore.WaitAsync(_config.AcquireTimeout, cancellationToken);
        if (!acquired)
            throw new TransportException("Timeout waiting for available connection");

        try
        {
            // Try to get existing connection
            if (_availableConnections.TryTake(out var connection))
            {
                // Validate if required
                if (_config.ValidateOnAcquire && !await ValidateConnectionAsync(connection))
                {
                    // Connection invalid, create new one
                    connection.Dispose();
                    Interlocked.Decrement(ref _totalConnections);
                    connection = await CreateConnectionAsync(cancellationToken);
                }

                _activeConnections[connection] = DateTime.UtcNow;
                return connection;
            }

            // Create new connection if under max limit
            lock (_lock)
            {
                if (_totalConnections < _config.MaxConnections)
                {
                    var newConnection = CreateConnectionAsync(cancellationToken).GetAwaiter().GetResult();
                    _activeConnections[newConnection] = DateTime.UtcNow;
                    return newConnection;
                }
            }

            throw new TransportException("Unable to acquire connection");
        }
        catch
        {
            _connectionSemaphore.Release();
            throw;
        }
    }

    /// <summary>
    /// Release a connection back to the pool
    /// </summary>
    public void Release(WebSocketClient connection)
    {
        if (_disposed)
        {
            connection.Dispose();
            return;
        }

        _activeConnections.TryRemove(connection, out _);

        // Check if connection is still valid
        if (connection.State == Types.ConnectionState.Connected)
        {
            _availableConnections.Add(connection);
            _connectionSemaphore.Release();
        }
        else
        {
            // Connection invalid, dispose and decrease count
            connection.Dispose();
            Interlocked.Decrement(ref _totalConnections);
            _connectionSemaphore.Release();

            // Try to maintain minimum connections
            _ = Task.Run(async () =>
            {
                if (_totalConnections < _config.MinConnections)
                {
                    try
                    {
                        var newConnection = await CreateConnectionAsync();
                        _availableConnections.Add(newConnection);
                    }
                    catch (Exception ex)
                    {
                        _events.Emit(EventType.Error, new Dictionary<string, object>
                        {
                            ["error"] = ex.Message,
                            ["context"] = "maintain_minimum"
                        });
                    }
                }
            });
        }
    }

    /// <summary>
    /// Create a new connection
    /// </summary>
    private async Task<WebSocketClient> CreateConnectionAsync(CancellationToken cancellationToken = default)
    {
        var connection = new WebSocketClient(_endpoint);

        try
        {
            await connection.ConnectAsync(cancellationToken);
            Interlocked.Increment(ref _totalConnections);

            _events.Emit(EventType.Connect, new Dictionary<string, object>
            {
                ["endpoint"] = _endpoint,
                ["total_connections"] = _totalConnections
            });

            return connection;
        }
        catch (Exception ex)
        {
            connection.Dispose();
            _events.Emit(EventType.Error, new Dictionary<string, object>
            {
                ["error"] = ex.Message,
                ["context"] = "create_connection"
            });
            throw new TransportException($"Failed to create connection: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Validate a connection
    /// </summary>
    private Task<bool> ValidateConnectionAsync(WebSocketClient connection)
    {
        try
        {
            return Task.FromResult(connection.State == Types.ConnectionState.Connected);
        }
        catch
        {
            return Task.FromResult(false);
        }
    }

    /// <summary>
    /// Execute action with a pooled connection
    /// </summary>
    public async Task<T> ExecuteAsync<T>(
        Func<WebSocketClient, Task<T>> action,
        CancellationToken cancellationToken = default)
    {
        var connection = await AcquireAsync(cancellationToken);
        try
        {
            return await action(connection);
        }
        finally
        {
            Release(connection);
        }
    }

    /// <summary>
    /// Execute action with a pooled connection
    /// </summary>
    public async Task ExecuteAsync(
        Func<WebSocketClient, Task> action,
        CancellationToken cancellationToken = default)
    {
        var connection = await AcquireAsync(cancellationToken);
        try
        {
            await action(connection);
        }
        finally
        {
            Release(connection);
        }
    }

    /// <summary>
    /// Close all connections and cleanup
    /// </summary>
    public async Task CloseAsync()
    {
        _disposed = true;

        // Close all active connections
        foreach (var connection in _activeConnections.Keys)
        {
            try
            {
                await connection.DisconnectAsync();
                connection.Dispose();
            }
            catch { }
        }
        _activeConnections.Clear();

        // Close all available connections
        while (_availableConnections.TryTake(out var connection))
        {
            try
            {
                await connection.DisconnectAsync();
                connection.Dispose();
            }
            catch { }
        }

        _totalConnections = 0;
    }

    public void Dispose()
    {
        if (!_disposed)
        {
            CloseAsync().GetAwaiter().GetResult();
            _connectionSemaphore.Dispose();
        }
    }
}

