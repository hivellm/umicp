using Xunit;
using Umicp.Core;
using Umicp.Core.Types;
using Umicp.Core.Peer;
using Umicp.Core.Events;

namespace Umicp.Tests;

public class PeerTests
{
    [Fact]
    public void PeerInfo_ShouldCreateWithBasicInfo()
    {
        // Arrange & Act
        var info = new PeerInfo("peer-001", "ws://localhost:8080");

        // Assert
        Assert.Equal("peer-001", info.Id);
        Assert.Equal("ws://localhost:8080", info.Address);
        Assert.NotNull(info.Capabilities);
        Assert.NotNull(info.Metadata);
        Assert.Equal("1.0.0", info.ProtocolVersion);
    }

    [Fact]
    public void PeerInfo_ShouldTrackTimestamps()
    {
        // Arrange
        var before = DateTime.UtcNow;
        
        // Act
        var info = new PeerInfo("peer-001");
        var after = DateTime.UtcNow;

        // Assert
        Assert.True(info.ConnectedAt >= before && info.ConnectedAt <= after);
        Assert.True(info.LastSeen >= before && info.LastSeen <= after);
    }

    [Fact]
    public void PeerConnection_ShouldInitializeWithPeerInfo()
    {
        // Arrange
        var peerInfo = new PeerInfo("peer-001", "ws://localhost:8080");

        // Act
        var connection = new PeerConnection(peerInfo);

        // Assert
        Assert.Equal(peerInfo, connection.Info);
        Assert.Equal(ConnectionState.Disconnected, connection.State);
        Assert.False(connection.HandshakeCompleted);
        Assert.NotNull(connection.Stats);
        Assert.NotNull(connection.Events);
    }

    [Fact]
    public void PeerConnection_ShouldUpdateLastSeen()
    {
        // Arrange
        var peerInfo = new PeerInfo("peer-001");
        var connection = new PeerConnection(peerInfo);
        var originalLastSeen = peerInfo.LastSeen;
        
        System.Threading.Thread.Sleep(10);

        // Act
        connection.UpdateLastSeen();

        // Assert
        Assert.True(peerInfo.LastSeen > originalLastSeen);
        Assert.True(connection.LastHeartbeat > originalLastSeen);
    }

    [Fact]
    public void HandshakeProtocol_ShouldCreateHelloMessage()
    {
        // Arrange
        var protocol = new HandshakeProtocol("local-001", new Dictionary<string, object>
        {
            ["version"] = "1.0.0",
            ["features"] = "test"
        });

        // Act
        var hello = protocol.CreateHelloMessage("remote-001");

        // Assert
        Assert.Equal("local-001", hello.FromId);
        Assert.Equal("remote-001", hello.ToId);
        Assert.Equal(OperationType.Control, hello.Operation);
        Assert.NotNull(hello.Capabilities);
        Assert.Equal("HELLO", hello.Capabilities["command"]);
        Assert.Equal("1.0.0", hello.Capabilities["protocol_version"]);
        Assert.Equal("1.0.0", hello.Capabilities["version"]);
    }

    [Fact]
    public void HandshakeProtocol_ShouldCreateAckMessage()
    {
        // Arrange
        var protocol = new HandshakeProtocol("local-001");
        var helloMessage = new Envelope(
            fromId: "remote-001",
            toId: "local-001",
            operation: OperationType.Control,
            messageId: "msg-123"
        );

        // Act
        var ack = protocol.CreateAckMessage(helloMessage);

        // Assert
        Assert.Equal("local-001", ack.FromId);
        Assert.Equal("remote-001", ack.ToId);
        Assert.Equal(OperationType.Ack, ack.Operation);
        Assert.Equal("msg-123", ack.CorrelationId);
        Assert.Equal("HELLO_ACK", ack.Capabilities!["command"]);
    }

    [Fact]
    public void HandshakeProtocol_ShouldValidateHandshakeMessage()
    {
        // Arrange
        var protocol = new HandshakeProtocol("local-001");
        var validMessage = new Envelope(
            fromId: "remote-001",
            toId: "local-001",
            operation: OperationType.Control,
            capabilities: new Dictionary<string, object> { ["command"] = "HELLO" }
        );
        var invalidMessage = new Envelope(
            fromId: "remote-001",
            toId: "local-001",
            operation: OperationType.Data
        );

        // Act & Assert
        Assert.True(protocol.ValidateHandshake(validMessage));
        Assert.False(protocol.ValidateHandshake(invalidMessage));
    }

    [Fact]
    public void HandshakeProtocol_ShouldExtractPeerInfo()
    {
        // Arrange
        var protocol = new HandshakeProtocol("local-001");
        var message = new Envelope(
            fromId: "remote-001",
            toId: "local-001",
            operation: OperationType.Control,
            capabilities: new Dictionary<string, object>
            {
                ["command"] = "HELLO",
                ["protocol_version"] = "1.5.0",
                ["feature1"] = "value1"
            }
        );

        // Act
        var peerInfo = protocol.ExtractPeerInfo(message);

        // Assert
        Assert.Equal("remote-001", peerInfo.Id);
        Assert.Equal("1.5.0", peerInfo.ProtocolVersion);
        Assert.Equal("HELLO", peerInfo.Capabilities["command"]);
        Assert.Equal("value1", peerInfo.Capabilities["feature1"]);
    }

    [Fact]
    public void HandshakeProtocol_ShouldIdentifyHelloMessage()
    {
        // Arrange
        var protocol = new HandshakeProtocol("local-001");
        var helloMessage = protocol.CreateHelloMessage("remote-001");
        var dataMessage = new Envelope(
            fromId: "remote-001",
            toId: "local-001",
            operation: OperationType.Data
        );

        // Act & Assert
        Assert.True(protocol.IsHelloMessage(helloMessage));
        Assert.False(protocol.IsHelloMessage(dataMessage));
    }

    [Fact]
    public void HandshakeProtocol_ShouldIdentifyHelloAckMessage()
    {
        // Arrange
        var protocol = new HandshakeProtocol("local-001");
        var hello = protocol.CreateHelloMessage("remote-001");
        var ack = protocol.CreateAckMessage(hello);
        var dataMessage = new Envelope(
            fromId: "remote-001",
            toId: "local-001",
            operation: OperationType.Data
        );

        // Act & Assert
        Assert.True(protocol.IsHelloAckMessage(ack));
        Assert.False(protocol.IsHelloAckMessage(dataMessage));
    }

    [Fact]
    public void MultiplexedPeer_ShouldInitializeWithLocalId()
    {
        // Arrange & Act
        var peer = new MultiplexedPeer("peer-001", new Dictionary<string, object>
        {
            ["version"] = "1.0.0"
        });

        // Assert
        Assert.Equal("peer-001", peer.LocalId);
        Assert.Equal(0, peer.PeerCount);
        Assert.NotNull(peer.Events);
        Assert.NotNull(peer.Peers);
    }

    [Fact]
    public async Task MultiplexedPeer_ShouldAddPeer()
    {
        // Arrange
        var peer = new MultiplexedPeer("peer-001");
        var peerInfo = new PeerInfo("peer-002", "ws://localhost:8081");
        var eventFired = false;

        peer.Events.On(EventType.PeerConnected, evt => eventFired = true);

        // Act
        var connection = await peer.AddPeerAsync(peerInfo);

        // Assert
        Assert.NotNull(connection);
        Assert.Equal(1, peer.PeerCount);
        Assert.True(eventFired);
    }

    [Fact]
    public async Task MultiplexedPeer_ShouldNotAddDuplicatePeer()
    {
        // Arrange
        var peer = new MultiplexedPeer("peer-001");
        var peerInfo = new PeerInfo("peer-002");
        await peer.AddPeerAsync(peerInfo);

        // Act & Assert
        await Assert.ThrowsAsync<Umicp.Core.Exceptions.TransportException>(
            async () => await peer.AddPeerAsync(peerInfo)
        );
    }

    [Fact]
    public async Task MultiplexedPeer_ShouldRemovePeer()
    {
        // Arrange
        var peer = new MultiplexedPeer("peer-001");
        var peerInfo = new PeerInfo("peer-002");
        await peer.AddPeerAsync(peerInfo);
        var eventFired = false;

        peer.Events.On(EventType.PeerDisconnected, evt => eventFired = true);

        // Act
        var removed = peer.RemovePeer("peer-002");

        // Assert
        Assert.True(removed);
        Assert.Equal(0, peer.PeerCount);
        Assert.True(eventFired);
    }

    [Fact]
    public async Task MultiplexedPeer_ShouldGetPeer()
    {
        // Arrange
        var peer = new MultiplexedPeer("peer-001");
        var peerInfo = new PeerInfo("peer-002");
        await peer.AddPeerAsync(peerInfo);

        // Act
        var retrieved = peer.GetPeer("peer-002");

        // Assert
        Assert.NotNull(retrieved);
        Assert.Equal("peer-002", retrieved.Info.Id);
    }

    [Fact]
    public void MultiplexedPeer_ShouldReturnNullForNonExistentPeer()
    {
        // Arrange
        var peer = new MultiplexedPeer("peer-001");

        // Act
        var retrieved = peer.GetPeer("non-existent");

        // Assert
        Assert.Null(retrieved);
    }

    [Fact]
    public async Task MultiplexedPeer_ShouldGetConnectedPeerIds()
    {
        // Arrange
        var peer = new MultiplexedPeer("peer-001");
        var peerInfo1 = new PeerInfo("peer-002");
        var peerInfo2 = new PeerInfo("peer-003");
        
        var conn1 = await peer.AddPeerAsync(peerInfo1);
        conn1.State = ConnectionState.Connected;
        conn1.HandshakeCompleted = true;
        
        var conn2 = await peer.AddPeerAsync(peerInfo2);
        conn2.State = ConnectionState.Connecting;

        // Act
        var connectedIds = peer.GetConnectedPeerIds();

        // Assert
        Assert.Single(connectedIds);
        Assert.Contains("peer-002", connectedIds);
    }

    [Fact]
    public async Task MultiplexedPeer_ShouldGetPeerStats()
    {
        // Arrange
        var peer = new MultiplexedPeer("peer-001");
        await peer.AddPeerAsync(new PeerInfo("peer-002"));
        await peer.AddPeerAsync(new PeerInfo("peer-003"));

        // Act
        var stats = peer.GetPeerStats();

        // Assert
        Assert.Equal(2, stats.Count);
        Assert.Contains("peer-002", stats.Keys);
        Assert.Contains("peer-003", stats.Keys);
    }
}

