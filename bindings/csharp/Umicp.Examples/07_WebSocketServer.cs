using Umicp.SDK;
using Umicp.SDK.Types;
using Umicp.SDK.Transport;
using Umicp.SDK.Events;

namespace Umicp.Examples;

/// <summary>
/// WebSocket server example
/// </summary>
public class WebSocketServerExample
{
    public static async Task Run()
    {
        Console.WriteLine("=== WebSocket Server Example ===\n");

        using var server = new WebSocketServer("localhost", 8080);

        // Subscribe to events
        server.Events.On(EventType.Connect, evt =>
        {
            if (evt.Data.TryGetValue("client_id", out var clientId))
            {
                Console.WriteLine($"Client connected: {clientId}");
            }
        });

        server.Events.On(EventType.Disconnect, evt =>
        {
            if (evt.Data.TryGetValue("client_id", out var clientId))
            {
                Console.WriteLine($"Client disconnected: {clientId}");
            }
        });

        server.Events.On(EventType.DataReceived, evt =>
        {
            if (evt.Data.TryGetValue("client_id", out var clientId) &&
                evt.Data.TryGetValue("data", out var dataObj) &&
                dataObj is byte[] data)
            {
                var text = System.Text.Encoding.UTF8.GetString(data);
                Console.WriteLine($"Received from {clientId}: {text}");

                // Echo back to client
                var response = $"Echo: {text}";
                _ = server.SendTextToClientAsync(clientId.ToString()!, response);
            }
        });

        server.Events.On(EventType.Error, evt =>
        {
            if (evt.Data.TryGetValue("error", out var error))
            {
                Console.WriteLine($"Error: {error}");
            }
        });

        try
        {
            // Start server
            Console.WriteLine("Starting WebSocket server on ws://localhost:8080...");
            await server.StartAsync();
            Console.WriteLine("Server started! Waiting for connections...");
            Console.WriteLine("(Press Ctrl+C to stop or wait 30 seconds)\n");

            // Keep server running
            await Task.Delay(30000); // Run for 30 seconds

            // Get stats
            Console.WriteLine($"\nServer Statistics:");
            Console.WriteLine($"  Connected clients: {server.ClientCount}");
            Console.WriteLine($"  Messages sent: {server.Stats.MessagesSent}");
            Console.WriteLine($"  Messages received: {server.Stats.MessagesReceived}");
            Console.WriteLine($"  Bytes sent: {server.Stats.BytesSent}");
            Console.WriteLine($"  Bytes received: {server.Stats.BytesReceived}");

            // Stop server
            await server.StopAsync();
            Console.WriteLine("\nServer stopped.\n");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error: {ex.Message}");
            Console.WriteLine("Note: This example requires administrative privileges on Windows.");
        }
    }
}

