using Xunit;
using Umicp.Core.Compression;
using System.Text;

namespace Umicp.Tests;

public class CompressionTests
{
    [Fact]
    public void Compress_ShouldCompressData_Gzip()
    {
        // Arrange - use larger data for better compression
        var repeatedText = string.Join("", Enumerable.Repeat("Hello, World! This is a test string for compression. ", 10));
        var data = Encoding.UTF8.GetBytes(repeatedText);

        // Act
        var compressed = Compressor.Compress(data, CompressionType.Gzip);

        // Assert
        Assert.NotNull(compressed);
        Assert.NotEmpty(compressed);
        Assert.True(compressed.Length < data.Length); // Should be smaller with repeated data
    }

    [Fact]
    public void Compress_ShouldCompressData_Deflate()
    {
        // Arrange - use larger data for better compression
        var repeatedText = string.Join("", Enumerable.Repeat("Hello, World! This is a test string for compression. ", 10));
        var data = Encoding.UTF8.GetBytes(repeatedText);

        // Act
        var compressed = Compressor.Compress(data, CompressionType.Deflate);

        // Assert
        Assert.NotNull(compressed);
        Assert.NotEmpty(compressed);
        Assert.True(compressed.Length < data.Length); // Should be smaller with repeated data
    }

    [Fact]
    public void Decompress_ShouldDecompressData_Gzip()
    {
        // Arrange
        var originalData = Encoding.UTF8.GetBytes("Hello, World! This is a test string.");
        var compressed = Compressor.Compress(originalData, CompressionType.Gzip);

        // Act
        var decompressed = Compressor.Decompress(compressed, CompressionType.Gzip);

        // Assert
        Assert.Equal(originalData, decompressed);
    }

    [Fact]
    public void Decompress_ShouldDecompressData_Deflate()
    {
        // Arrange
        var originalData = Encoding.UTF8.GetBytes("Hello, World! This is a test string.");
        var compressed = Compressor.Compress(originalData, CompressionType.Deflate);

        // Act
        var decompressed = Compressor.Decompress(compressed, CompressionType.Deflate);

        // Assert
        Assert.Equal(originalData, decompressed);
    }

    [Fact]
    public void CompressString_ShouldCompressAndDecompressString()
    {
        // Arrange
        var original = "This is a longer test string that should compress well. " +
                      "Compression works better with repeated patterns and longer content.";

        // Act
        var compressed = Compressor.CompressString(original, CompressionType.Gzip);
        var decompressed = Compressor.DecompressString(compressed, CompressionType.Gzip);

        // Assert
        Assert.Equal(original, decompressed);
    }

    [Fact]
    public void Compress_WithNoneType_ShouldReturnOriginalData()
    {
        // Arrange
        var data = Encoding.UTF8.GetBytes("Test data");

        // Act
        var result = Compressor.Compress(data, CompressionType.None);

        // Assert
        Assert.Equal(data, result);
    }

    [Fact]
    public void Compress_WithEmptyData_ShouldReturnEmptyData()
    {
        // Arrange
        var data = Array.Empty<byte>();

        // Act
        var result = Compressor.Compress(data, CompressionType.Gzip);

        // Assert
        Assert.Empty(result);
    }

    [Fact]
    public void GetCompressionRatio_ShouldCalculateCorrectly()
    {
        // Arrange
        var originalSize = 1000;
        var compressedSize = 500;

        // Act
        var ratio = Compressor.GetCompressionRatio(originalSize, compressedSize);

        // Assert
        Assert.Equal(0.5, ratio); // 50% reduction
    }

    [Fact]
    public void IsCompressed_ShouldDetectGzipData()
    {
        // Arrange
        var data = Encoding.UTF8.GetBytes("Test string for compression");
        var compressed = Compressor.Compress(data, CompressionType.Gzip);

        // Act
        var isCompressed = Compressor.IsCompressed(compressed);

        // Assert
        Assert.True(isCompressed);
    }

    [Fact]
    public void IsCompressed_ShouldReturnFalseForUncompressedData()
    {
        // Arrange
        var data = Encoding.UTF8.GetBytes("Test string");

        // Act
        var isCompressed = Compressor.IsCompressed(data);

        // Assert
        Assert.False(isCompressed);
    }

    [Fact]
    public void GetRecommendedType_ShouldReturnNoneForSmallData()
    {
        // Act
        var type = Compressor.GetRecommendedType(100);

        // Assert
        Assert.Equal(CompressionType.None, type);
    }

    [Fact]
    public void GetRecommendedType_ShouldReturnGzipForLargeData()
    {
        // Act
        var type = Compressor.GetRecommendedType(1024);

        // Assert
        Assert.Equal(CompressionType.Gzip, type);
    }

    [Fact]
    public void Compress_LargeData_ShouldAchieveGoodCompression()
    {
        // Arrange
        var repeatedString = string.Join("", Enumerable.Repeat("Lorem ipsum dolor sit amet. ", 100));
        var data = Encoding.UTF8.GetBytes(repeatedString);

        // Act
        var compressed = Compressor.Compress(data, CompressionType.Gzip);
        var ratio = Compressor.GetCompressionRatio(data.Length, compressed.Length);

        // Assert
        Assert.True(ratio > 0.5); // At least 50% compression for repeated data
    }
}

