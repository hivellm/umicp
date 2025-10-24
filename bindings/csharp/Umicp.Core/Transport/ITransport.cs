using Umicp.SDK.Types;
using Umicp.SDK.Events;

namespace Umicp.SDK.Transport;

/// <summary>
/// Base interface for UMICP transports
/// </summary>
public interface ITransport
{
    /// <summary>
    /// Connection state
    /// </summary>
    ConnectionState State { get; }

    /// <summary>
    /// Transport statistics
    /// </summary>
    TransportStats Stats { get; }

    /// <summary>
    /// Event emitter for transport events
    /// </summary>
    EventEmitter Events { get; }

    /// <summary>
    /// Connect to the transport
    /// </summary>
    Task ConnectAsync(CancellationToken cancellationToken = default);

    /// <summary>
    /// Disconnect from the transport
    /// </summary>
    Task DisconnectAsync(CancellationToken cancellationToken = default);

    /// <summary>
    /// Send data through the transport
    /// </summary>
    Task SendAsync(byte[] data, CancellationToken cancellationToken = default);

    /// <summary>
    /// Send text through the transport
    /// </summary>
    Task SendTextAsync(string text, CancellationToken cancellationToken = default);

    /// <summary>
    /// Receive data from the transport
    /// </summary>
    Task<byte[]> ReceiveAsync(CancellationToken cancellationToken = default);
}

