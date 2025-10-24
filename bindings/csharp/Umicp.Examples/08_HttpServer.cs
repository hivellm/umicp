using Umicp.SDK.Transport;
using Umicp.SDK.Events;

namespace Umicp.Examples;

/// <summary>
/// HTTP server example with REST API
/// </summary>
public class HttpServerExample
{
    public static async Task Run()
    {
        Console.WriteLine("=== HTTP Server Example ===\n");

        using var server = new HttpServer("localhost", 3000);

        // Subscribe to events
        server.Events.On(EventType.Connect, evt =>
        {
            Console.WriteLine("HTTP server started");
        });

        server.Events.On(EventType.DataReceived, evt =>
        {
            if (evt.Data.TryGetValue("method", out var method) &&
                evt.Data.TryGetValue("path", out var path))
            {
                Console.WriteLine($"{method} {path}");
            }
        });

        // Register routes
        server.Get("/", () =>
        {
            return Task.FromResult<object>(new
            {
                message = "Welcome to UMICP HTTP Server",
                version = "1.0.0",
                endpoints = new[]
                {
                    "GET /",
                    "GET /api/status",
                    "POST /api/echo"
                }
            });
        });

        server.Get("/api/status", () =>
        {
            return Task.FromResult<object>(new
            {
                status = "healthy",
                uptime = DateTime.UtcNow,
                stats = server.Stats.ToDictionary()
            });
        });

        server.Post<EchoRequest>("/api/echo", (request) =>
        {
            return Task.FromResult<object>(new
            {
                echo = request.Message,
                timestamp = DateTime.UtcNow,
                length = request.Message?.Length ?? 0
            });
        });

        try
        {
            // Start server
            Console.WriteLine("Starting HTTP server on http://localhost:3000...");
            await server.StartAsync();
            Console.WriteLine("Server started!\n");

            Console.WriteLine("Available endpoints:");
            Console.WriteLine("  GET  http://localhost:3000/");
            Console.WriteLine("  GET  http://localhost:3000/api/status");
            Console.WriteLine("  POST http://localhost:3000/api/echo\n");

            Console.WriteLine("Test with curl:");
            Console.WriteLine("  curl http://localhost:3000/");
            Console.WriteLine("  curl http://localhost:3000/api/status");
            Console.WriteLine("  curl -X POST http://localhost:3000/api/echo -H \"Content-Type: application/json\" -d \"{\\\"message\\\":\\\"Hello!\\\"}\"");
            Console.WriteLine("\n(Press Ctrl+C to stop or wait 30 seconds)\n");

            // Keep server running
            await Task.Delay(30000); // Run for 30 seconds

            // Get stats
            Console.WriteLine($"\nServer Statistics:");
            Console.WriteLine($"  Requests handled: {server.Stats.MessagesReceived}");
            Console.WriteLine($"  Responses sent: {server.Stats.MessagesSent}");
            Console.WriteLine($"  Errors: {server.Stats.Errors}");

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

    private class EchoRequest
    {
        public string? Message { get; set; }
    }
}

