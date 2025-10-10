using Umicp.Core.Types;

namespace Umicp.Core.Peer;

/// <summary>
/// Handshake protocol for peer connections
/// </summary>
public class HandshakeProtocol
{
    private readonly string _localId;
    private readonly Dictionary<string, object> _capabilities;

    public HandshakeProtocol(string localId, Dictionary<string, object>? capabilities = null)
    {
        _localId = localId;
        _capabilities = capabilities ?? new Dictionary<string, object>();
    }

    /// <summary>
    /// Create HELLO message for handshake
    /// </summary>
    public Envelope CreateHelloMessage(string peerId)
    {
        var builder = new EnvelopeBuilder()
            .FromId(_localId)
            .ToId(peerId)
            .Operation(OperationType.Control);

        // Add capabilities
        builder.Capability("command", "HELLO");
        builder.Capability("protocol_version", "1.0.0");

        foreach (var cap in _capabilities)
        {
            builder.Capability(cap.Key, cap.Value);
        }

        return builder.Build();
    }

    /// <summary>
    /// Create ACK message for handshake response
    /// </summary>
    public Envelope CreateAckMessage(Envelope helloMessage)
    {
        var builder = new EnvelopeBuilder()
            .FromId(_localId)
            .ToId(helloMessage.FromId)
            .Operation(OperationType.Ack)
            .CorrelationId(helloMessage.MessageId);

        builder.Capability("command", "HELLO_ACK");
        builder.Capability("protocol_version", "1.0.0");

        foreach (var cap in _capabilities)
        {
            builder.Capability(cap.Key, cap.Value);
        }

        return builder.Build();
    }

    /// <summary>
    /// Validate incoming handshake message
    /// </summary>
    public bool ValidateHandshake(Envelope envelope)
    {
        if (envelope.Operation != OperationType.Control && envelope.Operation != OperationType.Ack)
            return false;

        if (envelope.Capabilities == null)
            return false;

        if (!envelope.Capabilities.TryGetValue("command", out var command))
            return false;

        var commandStr = command.ToString();
        return commandStr == "HELLO" || commandStr == "HELLO_ACK";
    }

    /// <summary>
    /// Extract peer info from handshake message
    /// </summary>
    public PeerInfo ExtractPeerInfo(Envelope envelope)
    {
        var info = new PeerInfo(envelope.FromId);

        if (envelope.Capabilities != null)
        {
            foreach (var cap in envelope.Capabilities)
            {
                info.Capabilities[cap.Key] = cap.Value;
            }

            if (envelope.Capabilities.TryGetValue("protocol_version", out var version))
            {
                info.ProtocolVersion = version.ToString() ?? "1.0.0";
            }
        }

        return info;
    }

    /// <summary>
    /// Check if message is a HELLO message
    /// </summary>
    public bool IsHelloMessage(Envelope envelope)
    {
        return envelope.Operation == OperationType.Control &&
               envelope.Capabilities?.TryGetValue("command", out var cmd) == true &&
               cmd.ToString() == "HELLO";
    }

    /// <summary>
    /// Check if message is a HELLO_ACK message
    /// </summary>
    public bool IsHelloAckMessage(Envelope envelope)
    {
        return envelope.Operation == OperationType.Ack &&
               envelope.Capabilities?.TryGetValue("command", out var cmd) == true &&
               cmd.ToString() == "HELLO_ACK";
    }
}

