using System.IO.Compression;
using Umicp.SDK.Exceptions;

namespace Umicp.SDK.Compression;

/// <summary>
/// Compression and decompression utilities for UMICP
/// </summary>
public static class Compressor
{
    /// <summary>
    /// Compress data using specified algorithm
    /// </summary>
    /// <param name="data">Data to compress</param>
    /// <param name="type">Compression algorithm</param>
    /// <returns>Compressed data</returns>
    public static byte[] Compress(byte[] data, CompressionType type = CompressionType.Gzip)
    {
        if (data == null || data.Length == 0)
            return data ?? Array.Empty<byte>();

        if (type == CompressionType.None)
            return data;

        try
        {
            using var outputStream = new MemoryStream();

            Stream compressionStream = type switch
            {
                CompressionType.Gzip => new GZipStream(outputStream, CompressionLevel.Optimal),
                CompressionType.Deflate => new DeflateStream(outputStream, CompressionLevel.Optimal),
                _ => throw new ArgumentException($"Unsupported compression type: {type}")
            };

            using (compressionStream)
            {
                compressionStream.Write(data, 0, data.Length);
            }

            return outputStream.ToArray();
        }
        catch (Exception ex)
        {
            throw new UmicpException($"Compression failed: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Decompress data using specified algorithm
    /// </summary>
    /// <param name="compressedData">Compressed data</param>
    /// <param name="type">Compression algorithm used</param>
    /// <returns>Decompressed data</returns>
    public static byte[] Decompress(byte[] compressedData, CompressionType type = CompressionType.Gzip)
    {
        if (compressedData == null || compressedData.Length == 0)
            return compressedData ?? Array.Empty<byte>();

        if (type == CompressionType.None)
            return compressedData;

        try
        {
            using var inputStream = new MemoryStream(compressedData);
            using var outputStream = new MemoryStream();

            Stream decompressionStream = type switch
            {
                CompressionType.Gzip => new GZipStream(inputStream, CompressionMode.Decompress),
                CompressionType.Deflate => new DeflateStream(inputStream, CompressionMode.Decompress),
                _ => throw new ArgumentException($"Unsupported compression type: {type}")
            };

            using (decompressionStream)
            {
                decompressionStream.CopyTo(outputStream);
            }

            return outputStream.ToArray();
        }
        catch (Exception ex)
        {
            throw new UmicpException($"Decompression failed: {ex.Message}", ex);
        }
    }

    /// <summary>
    /// Compress string data using specified algorithm
    /// </summary>
    /// <param name="text">Text to compress</param>
    /// <param name="type">Compression algorithm</param>
    /// <returns>Compressed data</returns>
    public static byte[] CompressString(string text, CompressionType type = CompressionType.Gzip)
    {
        if (string.IsNullOrEmpty(text))
            return Array.Empty<byte>();

        var bytes = System.Text.Encoding.UTF8.GetBytes(text);
        return Compress(bytes, type);
    }

    /// <summary>
    /// Decompress data to string using specified algorithm
    /// </summary>
    /// <param name="compressedData">Compressed data</param>
    /// <param name="type">Compression algorithm used</param>
    /// <returns>Decompressed string</returns>
    public static string DecompressString(byte[] compressedData, CompressionType type = CompressionType.Gzip)
    {
        if (compressedData == null || compressedData.Length == 0)
            return string.Empty;

        var bytes = Decompress(compressedData, type);
        return System.Text.Encoding.UTF8.GetString(bytes);
    }

    /// <summary>
    /// Calculate compression ratio
    /// </summary>
    /// <param name="originalSize">Original size in bytes</param>
    /// <param name="compressedSize">Compressed size in bytes</param>
    /// <returns>Compression ratio (0-1, where 0.5 = 50% reduction)</returns>
    public static double GetCompressionRatio(int originalSize, int compressedSize)
    {
        if (originalSize == 0)
            return 0;

        return 1.0 - ((double)compressedSize / originalSize);
    }

    /// <summary>
    /// Detect if data is likely compressed
    /// </summary>
    /// <param name="data">Data to check</param>
    /// <returns>True if data appears to be GZIP or Deflate compressed</returns>
    public static bool IsCompressed(byte[] data)
    {
        if (data == null || data.Length < 2)
            return false;

        // Check for GZIP magic number (1F 8B)
        if (data[0] == 0x1F && data[1] == 0x8B)
            return true;

        // Check for Deflate (zlib) magic number (78 01, 78 9C, 78 DA)
        if (data[0] == 0x78 && (data[1] == 0x01 || data[1] == 0x9C || data[1] == 0xDA))
            return true;

        return false;
    }

    /// <summary>
    /// Get recommended compression type based on data size
    /// </summary>
    /// <param name="dataSize">Size of data in bytes</param>
    /// <returns>Recommended compression type</returns>
    public static CompressionType GetRecommendedType(int dataSize)
    {
        // Don't compress small data (overhead not worth it)
        if (dataSize < 128)
            return CompressionType.None;

        // GZIP for general use (better compression ratio)
        return CompressionType.Gzip;
    }
}

