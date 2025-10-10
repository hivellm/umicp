using Umicp.Core.Compression;
using System.Text;

namespace Umicp.Examples;

/// <summary>
/// Compression example - GZIP and Deflate
/// </summary>
public class CompressionExample
{
    public static void Run()
    {
        Console.WriteLine("=== Compression Example ===\n");

        // Original data
        var originalText = "Hello, World! This is a test message for compression. " +
                          "Compression works best with repeated patterns and larger data. " +
                          "Let's add more content to see better compression ratios!";

        var originalBytes = Encoding.UTF8.GetBytes(originalText);
        Console.WriteLine($"Original size: {originalBytes.Length} bytes");
        Console.WriteLine($"Original text: {originalText}\n");

        // GZIP Compression
        Console.WriteLine("--- GZIP Compression ---");
        var gzipCompressed = Compressor.Compress(originalBytes, CompressionType.Gzip);
        var gzipRatio = Compressor.GetCompressionRatio(originalBytes.Length, gzipCompressed.Length);

        Console.WriteLine($"Compressed size: {gzipCompressed.Length} bytes");
        Console.WriteLine($"Compression ratio: {gzipRatio:P2} ({(gzipRatio * 100):F2}% reduction)");
        Console.WriteLine($"Is compressed: {Compressor.IsCompressed(gzipCompressed)}\n");

        // Decompress
        var gzipDecompressed = Compressor.Decompress(gzipCompressed, CompressionType.Gzip);
        var decompressedText = Encoding.UTF8.GetString(gzipDecompressed);
        Console.WriteLine($"Decompressed: {decompressedText == originalText} ✓\n");

        // Deflate Compression
        Console.WriteLine("--- Deflate Compression ---");
        var deflateCompressed = Compressor.Compress(originalBytes, CompressionType.Deflate);
        var deflateRatio = Compressor.GetCompressionRatio(originalBytes.Length, deflateCompressed.Length);

        Console.WriteLine($"Compressed size: {deflateCompressed.Length} bytes");
        Console.WriteLine($"Compression ratio: {deflateRatio:P2}\n");

        // String compression
        Console.WriteLine("--- String Compression ---");
        var compressedString = Compressor.CompressString(originalText, CompressionType.Gzip);
        var decompressedString = Compressor.DecompressString(compressedString, CompressionType.Gzip);

        Console.WriteLine($"Original: {originalText.Length} chars");
        Console.WriteLine($"Compressed: {compressedString.Length} bytes");
        Console.WriteLine($"Match: {decompressedString == originalText} ✓\n");

        // Recommended compression type
        Console.WriteLine("--- Recommendations ---");
        Console.WriteLine($"Small data (100 bytes): {Compressor.GetRecommendedType(100)}");
        Console.WriteLine($"Medium data (1KB): {Compressor.GetRecommendedType(1024)}");
        Console.WriteLine($"Large data (10KB): {Compressor.GetRecommendedType(10240)}\n");

        // Large data compression test
        Console.WriteLine("--- Large Data Test ---");
        var largeText = string.Join("", Enumerable.Repeat("Lorem ipsum dolor sit amet, consectetur adipiscing elit. ", 100));
        var largeBytes = Encoding.UTF8.GetBytes(largeText);
        var largeCompressed = Compressor.Compress(largeBytes, CompressionType.Gzip);
        var largeRatio = Compressor.GetCompressionRatio(largeBytes.Length, largeCompressed.Length);

        Console.WriteLine($"Original: {largeBytes.Length:N0} bytes");
        Console.WriteLine($"Compressed: {largeCompressed.Length:N0} bytes");
        Console.WriteLine($"Ratio: {largeRatio:P2} reduction\n");
    }
}

