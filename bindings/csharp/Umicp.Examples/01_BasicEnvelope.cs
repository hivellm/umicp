using Umicp.SDK;
using Umicp.SDK.Types;

namespace Umicp.Examples;

/// <summary>
/// Basic envelope creation and serialization example
/// </summary>
public class BasicEnvelopeExample
{
    public static void Run()
    {
        Console.WriteLine("=== Basic Envelope Example ===\n");

        // Create an envelope
        var envelope = new Envelope(
            fromId: "client-001",
            toId: "server-001",
            operation: OperationType.Data
        );

        Console.WriteLine($"Created envelope: {envelope}");
        Console.WriteLine($"Message ID: {envelope.MessageId}");
        Console.WriteLine($"Timestamp: {envelope.Timestamp:O}\n");

        // Serialize to JSON
        var json = envelope.ToJson();
        Console.WriteLine($"JSON:\n{json}\n");

        // Compute hash
        var hash = envelope.ComputeHash();
        Console.WriteLine($"Hash: {hash}\n");

        // Using EnvelopeBuilder
        var builtEnvelope = new EnvelopeBuilder()
            .FromId("client-002")
            .ToId("server-001")
            .Operation(OperationType.Request)
            .Capability("version", "1.0.0")
            .Capability("protocol", "UMICP")
            .PayloadHint(new PayloadHint
            {
                Type = PayloadType.Json,
                Size = 1024,
                Encoding = EncodingType.Utf8
            })
            .Build();

        Console.WriteLine($"Built envelope: {builtEnvelope}");
        var capabilities = builtEnvelope.Capabilities?.Keys.ToArray() ?? Array.Empty<string>();
        Console.WriteLine($"Capabilities: {string.Join(", ", capabilities)}\n");
    }
}

