using Umicp.SDK;
using Umicp.SDK.Types;
using Umicp.SDK.Transport;
using Umicp.SDK.Events;

namespace Umicp.Examples;

/// <summary>
/// WebSocket client example
/// </summary>
public class WebSocketClientExample
{
    public static async Task Run()
    {
        Console.WriteLine("=== WebSocket Client Example ===\n");

        // Note: This example requires a running WebSocket server
        var url = "ws://localhost:8080";

        using var client = new WebSocketClient(url);

        // Subscribe to events
        client.Events.On(EventType.Connect, evt =>
        {
            Console.WriteLine("Connected to server!");
        });

        client.Events.On(EventType.DataReceived, evt =>
        {
            if (evt.Data.TryGetValue("data", out var data))
            {
                Console.WriteLine($"Received data: {data}");
            }
        });

        client.Events.On(EventType.Error, evt =>
        {
            if (evt.Data.TryGetValue("error", out var error))
            {
                Console.WriteLine($"Error: {error}");
            }
        });

        try
        {
            // Connect
            Console.WriteLine($"Connecting to {url}...");
            await client.ConnectAsync();

            // Create and send envelope
            var envelope = new EnvelopeBuilder()
                .FromId("client-001")
                .ToId("server-001")
                .Operation(OperationType.Data)
                .Capability("message", "Hello from C#!")
                .Build();

            Console.WriteLine("Sending envelope...");
            await client.SendTextAsync(envelope.ToJson());

            // Wait a bit for response
            await Task.Delay(2000);

            // Check stats
            Console.WriteLine($"\nStatistics:");
            Console.WriteLine($"  Messages sent: {client.Stats.MessagesSent}");
            Console.WriteLine($"  Messages received: {client.Stats.MessagesReceived}");
            Console.WriteLine($"  Bytes sent: {client.Stats.BytesSent}");
            Console.WriteLine($"  Bytes received: {client.Stats.BytesReceived}");

            // Disconnect
            await client.DisconnectAsync();
            Console.WriteLine("\nDisconnected.");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error: {ex.Message}");
            Console.WriteLine("Make sure a WebSocket server is running on ws://localhost:8080");
        }

        Console.WriteLine();
    }
}

