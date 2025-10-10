using Xunit;
using Umicp.Core.Types;

namespace Umicp.Tests;

public class TypesTests
{
    [Fact]
    public void PayloadHint_ShouldInitialize()
    {
        // Arrange & Act
        var hint = new PayloadHint
        {
            Type = PayloadType.Json,
            Size = 1024,
            Encoding = EncodingType.Utf8,
            Count = 10,
            Compression = "gzip"
        };

        // Assert
        Assert.Equal(PayloadType.Json, hint.Type);
        Assert.Equal(1024, hint.Size);
        Assert.Equal(EncodingType.Utf8, hint.Encoding);
        Assert.Equal(10, hint.Count);
        Assert.Equal("gzip", hint.Compression);
    }

    [Fact]
    public void PayloadHint_ShouldConvertToDictionary()
    {
        // Arrange
        var hint = new PayloadHint
        {
            Type = PayloadType.Vector,
            Size = 512,
            Encoding = EncodingType.Float32,
            Count = 128
        };

        // Act
        var dict = hint.ToDictionary();

        // Assert
        Assert.Equal("vector", dict["type"]);
        Assert.Equal(512, dict["size"]);
        Assert.Equal("float32", dict["encoding"]);
        Assert.Equal(128, dict["count"]);
    }

    [Fact]
    public void PayloadHint_ShouldCreateFromDictionary()
    {
        // Arrange
        var dict = new Dictionary<string, object>
        {
            ["type"] = "matrix",
            ["size"] = 2048,
            ["encoding"] = "float64",
            ["count"] = 256,
            ["compression"] = "lz4"
        };

        // Act
        var hint = PayloadHint.FromDictionary(dict);

        // Assert
        Assert.Equal(PayloadType.Matrix, hint.Type);
        Assert.Equal(2048, hint.Size);
        Assert.Equal(EncodingType.Float64, hint.Encoding);
        Assert.Equal(256, hint.Count);
        Assert.Equal("lz4", hint.Compression);
    }

    [Fact]
    public void TransportStats_ShouldInitializeWithDefaults()
    {
        // Arrange & Act
        var stats = new TransportStats();

        // Assert
        Assert.Equal(0, stats.MessagesSent);
        Assert.Equal(0, stats.MessagesReceived);
        Assert.Equal(0, stats.BytesSent);
        Assert.Equal(0, stats.BytesReceived);
        Assert.Equal(0, stats.Errors);
        Assert.Equal(0, stats.Reconnections);
        Assert.Null(stats.ConnectedAt);
        Assert.Null(stats.LastActivity);
    }

    [Fact]
    public void TransportStats_ShouldTrackStatistics()
    {
        // Arrange
        var stats = new TransportStats
        {
            MessagesSent = 100,
            MessagesReceived = 95,
            BytesSent = 10240,
            BytesReceived = 9600,
            Errors = 2,
            Reconnections = 1,
            ConnectedAt = DateTime.UtcNow,
            LastActivity = DateTime.UtcNow
        };

        // Act & Assert
        Assert.Equal(100, stats.MessagesSent);
        Assert.Equal(95, stats.MessagesReceived);
        Assert.Equal(10240, stats.BytesSent);
        Assert.Equal(9600, stats.BytesReceived);
        Assert.Equal(2, stats.Errors);
        Assert.Equal(1, stats.Reconnections);
        Assert.NotNull(stats.ConnectedAt);
        Assert.NotNull(stats.LastActivity);
    }

    [Fact]
    public void TransportStats_ShouldConvertToDictionary()
    {
        // Arrange
        var now = DateTime.UtcNow;
        var stats = new TransportStats
        {
            MessagesSent = 50,
            MessagesReceived = 45,
            BytesSent = 5120,
            BytesReceived = 4800,
            Errors = 1,
            Reconnections = 0,
            ConnectedAt = now,
            LastActivity = now
        };

        // Act
        var dict = stats.ToDictionary();

        // Assert
        Assert.Equal(50L, dict["messages_sent"]);
        Assert.Equal(45L, dict["messages_received"]);
        Assert.Equal(5120L, dict["bytes_sent"]);
        Assert.Equal(4800L, dict["bytes_received"]);
        Assert.Equal(1L, dict["errors"]);
        Assert.Equal(0L, dict["reconnections"]);
        Assert.NotNull(dict["connected_at"]);
        Assert.NotNull(dict["last_activity"]);
    }

    [Theory]
    [InlineData(OperationType.Control)]
    [InlineData(OperationType.Data)]
    [InlineData(OperationType.Ack)]
    [InlineData(OperationType.Error)]
    [InlineData(OperationType.Request)]
    [InlineData(OperationType.Response)]
    public void OperationType_ShouldHaveAllValues(OperationType type)
    {
        // Assert - just verify enum values exist
        Assert.True(Enum.IsDefined(typeof(OperationType), type));
    }

    [Theory]
    [InlineData(PayloadType.Vector)]
    [InlineData(PayloadType.Text)]
    [InlineData(PayloadType.Metadata)]
    [InlineData(PayloadType.Binary)]
    [InlineData(PayloadType.Json)]
    [InlineData(PayloadType.Matrix)]
    public void PayloadType_ShouldHaveAllValues(PayloadType type)
    {
        // Assert
        Assert.True(Enum.IsDefined(typeof(PayloadType), type));
    }

    [Theory]
    [InlineData(EncodingType.Float32)]
    [InlineData(EncodingType.Float64)]
    [InlineData(EncodingType.Int32)]
    [InlineData(EncodingType.Int64)]
    [InlineData(EncodingType.Utf8)]
    [InlineData(EncodingType.Base64)]
    [InlineData(EncodingType.Hex)]
    public void EncodingType_ShouldHaveAllValues(EncodingType type)
    {
        // Assert
        Assert.True(Enum.IsDefined(typeof(EncodingType), type));
    }

    [Theory]
    [InlineData(ConnectionState.Disconnected)]
    [InlineData(ConnectionState.Connecting)]
    [InlineData(ConnectionState.Connected)]
    [InlineData(ConnectionState.Reconnecting)]
    [InlineData(ConnectionState.Disconnecting)]
    [InlineData(ConnectionState.Error)]
    public void ConnectionState_ShouldHaveAllValues(ConnectionState state)
    {
        // Assert
        Assert.True(Enum.IsDefined(typeof(ConnectionState), state));
    }
}

