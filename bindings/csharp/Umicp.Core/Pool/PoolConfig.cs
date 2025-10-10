namespace Umicp.Core.Pool;

/// <summary>
/// Configuration for connection pool
/// </summary>
public class PoolConfig
{
    /// <summary>
    /// Minimum number of connections to maintain
    /// </summary>
    public int MinConnections { get; set; } = 1;

    /// <summary>
    /// Maximum number of connections allowed
    /// </summary>
    public int MaxConnections { get; set; } = 10;

    /// <summary>
    /// Connection timeout
    /// </summary>
    public TimeSpan ConnectionTimeout { get; set; } = TimeSpan.FromSeconds(30);

    /// <summary>
    /// Idle timeout before closing connection
    /// </summary>
    public TimeSpan IdleTimeout { get; set; } = TimeSpan.FromMinutes(5);

    /// <summary>
    /// Maximum time to wait for available connection
    /// </summary>
    public TimeSpan AcquireTimeout { get; set; } = TimeSpan.FromSeconds(10);

    /// <summary>
    /// Enable connection validation before use
    /// </summary>
    public bool ValidateOnAcquire { get; set; } = true;

    /// <summary>
    /// Enable automatic reconnection
    /// </summary>
    public bool AutoReconnect { get; set; } = true;

    /// <summary>
    /// Validate configuration
    /// </summary>
    public void Validate()
    {
        if (MinConnections < 0)
            throw new ArgumentException("MinConnections must be >= 0");

        if (MaxConnections < MinConnections)
            throw new ArgumentException("MaxConnections must be >= MinConnections");

        if (ConnectionTimeout <= TimeSpan.Zero)
            throw new ArgumentException("ConnectionTimeout must be > 0");
    }
}

