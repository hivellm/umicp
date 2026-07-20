using Umicp.SDK;
using Umicp.SDK.Types;
using Umicp.SDK.Peer;
using Umicp.SDK.Events;

namespace Umicp.Examples;

/// <summary>
/// Multiplexed peer example - peer-to-peer communication
/// </summary>
public class MultiplexedPeerExample
{
    public static async Task Run()
    {
        Console.WriteLine("=== Multiplexed Peer Example ===\n");

        // Create local peer
        var localPeer = new MultiplexedPeer(
            localId: "peer-001",
            capabilities: new Dictionary<string, object>
            {
                ["version"] = "1.0.0",
                ["features"] = new[] { "chat", "file-transfer" }
            }
        );

        // Subscribe to events
        localPeer.Events.On(EventType.PeerConnected, evt =>
        {
            if (evt.Data.TryGetValue("peer_id", out var peerId))
            {
                Console.WriteLine($"Peer connected: {peerId}");
            }
        });

        localPeer.Events.On(EventType.Message, evt =>
        {
            if (evt.Data.TryGetValue("envelope", out var envelopeObj) && envelopeObj is Envelope envelope)
            {
                Console.WriteLine($"Received message from {envelope.FromId}: {envelope.Operation}");
            }
        });

        localPeer.Events.On(EventType.Handshake, evt =>
        {
            if (evt.Data.TryGetValue("type", out var type))
            {
                Console.WriteLine($"Handshake event: {type}");
            }
        });

        // Simulate adding a peer
        var peerInfo = new PeerInfo("peer-002", "ws://localhost:8081");
        peerInfo.Capabilities["version"] = "1.0.0";

        Console.WriteLine($"Adding peer: {peerInfo.Id}");
        var connection = await localPeer.AddPeerAsync(peerInfo);

        Console.WriteLine($"Peer count: {localPeer.PeerCount}");
        Console.WriteLine($"Connected peers: {string.Join(", ", localPeer.GetConnectedPeerIds())}");

        // Create and send a message
        var message = new EnvelopeBuilder()
            .FromId(localPeer.LocalId)
            .ToId("peer-002")
            .Operation(OperationType.Data)
            .Capability("message", "Hello from peer-001!")
            .Build();

        Console.WriteLine("\nSending message to peer-002...");
        // Note: This would fail without actual transport, just for demonstration
        // await localPeer.SendToPeerAsync("peer-002", message);

        // Broadcast example
        var broadcastMessage = new EnvelopeBuilder()
            .FromId(localPeer.LocalId)
            .ToId("broadcast")
            .Operation(OperationType.Data)
            .Capability("message", "Broadcasting to all peers!")
            .Build();

        Console.WriteLine("Broadcasting message to all peers...");
        // await localPeer.BroadcastAsync(broadcastMessage);

        // Get statistics
        var stats = localPeer.GetPeerStats();
        Console.WriteLine($"\nPeer statistics:");
        foreach (var (peerId, peerStats) in stats)
        {
            Console.WriteLine($"  {peerId}:");
            Console.WriteLine($"    Messages sent: {peerStats.MessagesSent}");
            Console.WriteLine($"    Messages received: {peerStats.MessagesReceived}");
        }

        // Cleanup
        localPeer.Dispose();
        Console.WriteLine("\nPeer disposed.\n");
    }
}

