using Umicp.Core.Types;
using Umicp.Core.Transport;
using Umicp.Core.Events;

namespace Umicp.Core.Peer;

/// <summary>
/// Represents a connection to a peer
/// </summary>
public class PeerConnection : IDisposable
{
    /// <summary>
    /// Peer information
    /// </summary>
    public PeerInfo Info { get; }

    /// <summary>
    /// Transport for this connection
    /// </summary>
    public ITransport? Transport { get; set; }

    /// <summary>
    /// Connection state
    /// </summary>
    public ConnectionState State { get; set; } = ConnectionState.Disconnected;

    /// <summary>
    /// Statistics for this connection
    /// </summary>
    public TransportStats Stats { get; } = new();

    /// <summary>
    /// Event emitter for connection events
    /// </summary>
    public EventEmitter Events { get; } = new();

    /// <summary>
    /// Whether handshake has completed
    /// </summary>
    public bool HandshakeCompleted { get; set; }

    /// <summary>
    /// Last heartbeat time
    /// </summary>
    public DateTime LastHeartbeat { get; set; } = DateTime.UtcNow;

    public PeerConnection(PeerInfo info)
    {
        Info = info;
    }

    /// <summary>
    /// Send envelope to peer
    /// </summary>
    public async Task SendEnvelopeAsync(Envelope envelope, CancellationToken cancellationToken = default)
    {
        if (Transport == null)
            throw new InvalidOperationException("No transport available");

        var json = envelope.ToJson();
        await Transport.SendTextAsync(json, cancellationToken);

        Stats.MessagesSent++;
        Stats.BytesSent += System.Text.Encoding.UTF8.GetByteCount(json);
        Stats.LastActivity = DateTime.UtcNow;
    }

    /// <summary>
    /// Send data to peer
    /// </summary>
    public async Task SendAsync(byte[] data, CancellationToken cancellationToken = default)
    {
        if (Transport == null)
            throw new InvalidOperationException("No transport available");

        await Transport.SendAsync(data, cancellationToken);

        Stats.MessagesSent++;
        Stats.BytesSent += data.Length;
        Stats.LastActivity = DateTime.UtcNow;
    }

    /// <summary>
    /// Update last seen timestamp
    /// </summary>
    public void UpdateLastSeen()
    {
        Info.LastSeen = DateTime.UtcNow;
        LastHeartbeat = DateTime.UtcNow;
    }

    public void Dispose()
    {
        if (Transport is IDisposable disposable)
            disposable.Dispose();
    }

    public override string ToString()
    {
        return $"PeerConnection(id={Info.Id}, state={State}, handshake={HandshakeCompleted})";
    }
}

