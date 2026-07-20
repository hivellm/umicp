namespace Umicp.Examples;

class Program
{
    static async Task Main(string[] args)
    {
        Console.WriteLine("╔════════════════════════════════════════╗");
        Console.WriteLine("║   UMICP C# SDK Examples                ║");
        Console.WriteLine("╚════════════════════════════════════════╝\n");

        if (args.Length > 0 && int.TryParse(args[0], out int example))
        {
            await RunExample(example);
        }
        else
        {
            await RunAllExamples();
        }
    }

    static async Task RunAllExamples()
    {
        BasicEnvelopeExample.Run();
        await Task.Delay(500);

        MatrixOperationsExample.Run();
        await Task.Delay(500);

        CompressionExample.Run();
        await Task.Delay(500);

        Console.WriteLine("=== WebSocket Client Example ===");
        Console.WriteLine("(Skipped - requires running server)\n");
        // await WebSocketClientExample.Run();

        Console.WriteLine("=== WebSocket Server Example ===");
        Console.WriteLine("(Skipped - requires administrative privileges)\n");
        // await WebSocketServerExample.Run();

        Console.WriteLine("=== HTTP Server Example ===");
        Console.WriteLine("(Skipped - requires administrative privileges)\n");
        // await HttpServerExample.Run();

        await MultiplexedPeerExample.Run();
        await Task.Delay(500);

        await ServiceDiscoveryExample.Run();

        Console.WriteLine("╔════════════════════════════════════════╗");
        Console.WriteLine("║   All Examples Completed!              ║");
        Console.WriteLine("╚════════════════════════════════════════╝");
    }

    static async Task RunExample(int number)
    {
        switch (number)
        {
            case 1:
                BasicEnvelopeExample.Run();
                break;
            case 2:
                MatrixOperationsExample.Run();
                break;
            case 3:
                await WebSocketClientExample.Run();
                break;
            case 4:
                await MultiplexedPeerExample.Run();
                break;
            case 5:
                await ServiceDiscoveryExample.Run();
                break;
            case 6:
                CompressionExample.Run();
                break;
            case 7:
                await WebSocketServerExample.Run();
                break;
            case 8:
                await HttpServerExample.Run();
                break;
            default:
                Console.WriteLine($"Unknown example number: {number}");
                Console.WriteLine("Available examples: 1-8");
                break;
        }
    }
}

