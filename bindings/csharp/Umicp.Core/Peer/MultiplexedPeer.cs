using System.Collections.Concurrent;
using Umicp.SDK.Types;
using Umicp.SDK.Events;
using Umicp.SDK.Exceptions;

namespace Umicp.SDK.Peer;

/// <summary>
/// Multiplexed peer - acts as both client and server
/// </summary>
public class MultiplexedPeer : IDisposable
{
    private readonly string _localId;
    private readonly Dictionary<string, object> _capabilities;
    private readonly ConcurrentDictionary<string, PeerConnection> _peers = new();
    private readonly EventEmitter _events = new();
    private readonly HandshakeProtocol _handshake;

    /// <summary>
    /// Local peer ID
    /// </summary>
    public string LocalId => _localId;

    /// <summary>
    /// Event emitter
    /// </summary>
    public EventEmitter Events => _events;

    /// <summary>
    /// All connected peers
    /// </summary>
    public IReadOnlyDictionary<string, PeerConnection> Peers => _peers;

    /// <summary>
    /// Peer count
    /// </summary>
    public int PeerCount => _peers.Count;

    /// <summary>
    /// Initialize multiplexed peer
    /// </summary>
    public MultiplexedPeer(string localId, Dictionary<string, object>? capabilities = null)
    {
        _localId = localId;
        _capabilities = capabilities ?? new Dictionary<string, object>();
        _handshake = new HandshakeProtocol(localId, _capabilities);
    }

    /// <summary>
    /// Add a peer connection
    /// </summary>
    public async Task<PeerConnection> AddPeerAsync(PeerInfo peerInfo, CancellationToken cancellationToken = default)
    {
        if (_peers.ContainsKey(peerInfo.Id))
            throw new TransportException($"Peer {peerInfo.Id} already exists");

        var connection = new PeerConnection(peerInfo);

        if (_peers.TryAdd(peerInfo.Id, connection))
        {
            _events.Emit(EventType.PeerConnected, new Dictionary<string, object>
            {
                ["peer_id"] = peerInfo.Id,
                ["peer_info"] = peerInfo
            });

            // Initiate handshake
            if (connection.Transport != null)
            {
                await InitiateHandshakeAsync(connection, cancellationToken);
            }

            return connection;
        }

        throw new TransportException($"Failed to add peer {peerInfo.Id}");
    }

    /// <summary>
    /// Remove a peer connection
    /// </summary>
    public bool RemovePeer(string peerId)
    {
        if (_peers.TryRemove(peerId, out var connection))
        {
            connection.Dispose();

            _events.Emit(EventType.PeerDisconnected, new Dictionary<string, object>
            {
                ["peer_id"] = peerId
            });

            return true;
        }

        return false;
    }

    /// <summary>
    /// Get a peer connection
    /// </summary>
    public PeerConnection? GetPeer(string peerId)
    {
        _peers.TryGetValue(peerId, out var connection);
        return connection;
    }

    /// <summary>
    /// Send envelope to a specific peer
    /// </summary>
    public async Task SendToPeerAsync(string peerId, Envelope envelope, CancellationToken cancellationToken = default)
    {
        var peer = GetPeer(peerId);
        if (peer == null)
            throw new TransportException($"Peer {peerId} not found");

        if (peer.State != ConnectionState.Connected)
            throw new TransportException($"Peer {peerId} is not connected");

        await peer.SendEnvelopeAsync(envelope, cancellationToken);
    }

    /// <summary>
    /// Broadcast envelope to all peers
    /// </summary>
    public async Task BroadcastAsync(Envelope envelope, CancellationToken cancellationToken = default)
    {
        var tasks = _peers.Values
            .Where(p => p.State == ConnectionState.Connected && p.HandshakeCompleted)
            .Select(p => p.SendEnvelopeAsync(envelope, cancellationToken));

        await Task.WhenAll(tasks);
    }

    /// <summary>
    /// Broadcast to all peers except one
    /// </summary>
    public async Task BroadcastExceptAsync(string excludePeerId, Envelope envelope, CancellationToken cancellationToken = default)
    {
        var tasks = _peers.Values
            .Where(p => p.Info.Id != excludePeerId &&
                       p.State == ConnectionState.Connected &&
                       p.HandshakeCompleted)
            .Select(p => p.SendEnvelopeAsync(envelope, cancellationToken));

        await Task.WhenAll(tasks);
    }

    /// <summary>
    /// Initiate handshake with a peer
    /// </summary>
    private async Task InitiateHandshakeAsync(PeerConnection connection, CancellationToken cancellationToken)
    {
        try
        {
            var helloMessage = _handshake.CreateHelloMessage(connection.Info.Id);
            await connection.SendEnvelopeAsync(helloMessage, cancellationToken);

            _events.Emit(EventType.Handshake, new Dictionary<string, object>
            {
                ["peer_id"] = connection.Info.Id,
                ["type"] = "HELLO"
            });
        }
        catch (Exception ex)
        {
            _events.Emit(EventType.Error, new Dictionary<string, object>
            {
                ["error"] = ex.Message,
                ["peer_id"] = connection.Info.Id,
                ["context"] = "handshake"
            });
            throw;
        }
    }

    /// <summary>
    /// Handle incoming envelope
    /// </summary>
    public async Task HandleEnvelopeAsync(Envelope envelope, CancellationToken cancellationToken = default)
    {
        // Check if it's a handshake message
        if (_handshake.IsHelloMessage(envelope))
        {
            await HandleHelloMessageAsync(envelope, cancellationToken);
            return;
        }

        if (_handshake.IsHelloAckMessage(envelope))
        {
            await HandleHelloAckMessageAsync(envelope, cancellationToken);
            return;
        }

        // Regular message - emit to listeners
        _events.Emit(EventType.Message, new Dictionary<string, object>
        {
            ["envelope"] = envelope,
            ["from"] = envelope.FromId
        });
    }

    /// <summary>
    /// Handle HELLO message
    /// </summary>
    private async Task HandleHelloMessageAsync(Envelope envelope, CancellationToken cancellationToken)
    {
        var peerInfo = _handshake.ExtractPeerInfo(envelope);

        // Add peer if not exists
        if (!_peers.ContainsKey(peerInfo.Id))
        {
            var connection = new PeerConnection(peerInfo)
            {
                State = ConnectionState.Connected
            };
            _peers.TryAdd(peerInfo.Id, connection);
        }

        // Send ACK
        var peer = GetPeer(peerInfo.Id);
        if (peer != null)
        {
            var ackMessage = _handshake.CreateAckMessage(envelope);
            await peer.SendEnvelopeAsync(ackMessage, cancellationToken);

            peer.HandshakeCompleted = true;
            peer.UpdateLastSeen();

            _events.Emit(EventType.Handshake, new Dictionary<string, object>
            {
                ["peer_id"] = peerInfo.Id,
                ["type"] = "HELLO_ACK_SENT"
            });
        }
    }

    /// <summary>
    /// Handle HELLO_ACK message
    /// </summary>
    private Task HandleHelloAckMessageAsync(Envelope envelope, CancellationToken cancellationToken)
    {
        var peer = GetPeer(envelope.FromId);
        if (peer != null)
        {
            peer.HandshakeCompleted = true;
            peer.UpdateLastSeen();

            _events.Emit(EventType.Handshake, new Dictionary<string, object>
            {
                ["peer_id"] = peer.Info.Id,
                ["type"] = "HELLO_ACK_RECEIVED"
            });
        }

        return Task.CompletedTask;
    }

    /// <summary>
    /// Get all connected peer IDs
    /// </summary>
    public string[] GetConnectedPeerIds()
    {
        return _peers.Values
            .Where(p => p.State == ConnectionState.Connected && p.HandshakeCompleted)
            .Select(p => p.Info.Id)
            .ToArray();
    }

    /// <summary>
    /// Get statistics for all peers
    /// </summary>
    public Dictionary<string, TransportStats> GetPeerStats()
    {
        return _peers.ToDictionary(
            kvp => kvp.Key,
            kvp => kvp.Value.Stats
        );
    }

    public void Dispose()
    {
        foreach (var peer in _peers.Values)
        {
            peer.Dispose();
        }
        _peers.Clear();
    }
}

