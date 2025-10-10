using Xunit;
using Umicp.Core;
using Umicp.Core.Types;
using Umicp.Core.Exceptions;

namespace Umicp.Tests;

public class EnvelopeTests
{
    [Fact]
    public void Envelope_ShouldCreateWithRequiredFields()
    {
        // Arrange & Act
        var envelope = new Envelope(
            fromId: "client-001",
            toId: "server-001",
            operation: OperationType.Data
        );

        // Assert
        Assert.Equal("client-001", envelope.FromId);
        Assert.Equal("server-001", envelope.ToId);
        Assert.Equal(OperationType.Data, envelope.Operation);
        Assert.NotNull(envelope.MessageId);
        Assert.NotEqual(default, envelope.Timestamp);
    }

    [Fact]
    public void Envelope_ShouldThrowValidationException_WhenFromIdIsEmpty()
    {
        // Arrange, Act & Assert
        Assert.Throws<ValidationException>(() =>
            new Envelope(
                fromId: "",
                toId: "server-001",
                operation: OperationType.Data
            )
        );
    }

    [Fact]
    public void Envelope_ShouldSerializeToJson()
    {
        // Arrange
        var envelope = new Envelope(
            fromId: "client-001",
            toId: "server-001",
            operation: OperationType.Data,
            messageId: "msg-123"
        );

        // Act
        var json = envelope.ToJson();

        // Assert
        Assert.Contains("\"from\":\"client-001\"", json);
        Assert.Contains("\"to\":\"server-001\"", json);
        Assert.Contains("\"operation\":\"data\"", json);
        Assert.Contains("\"message_id\":\"msg-123\"", json);
    }

    [Fact]
    public void Envelope_ShouldDeserializeFromJson()
    {
        // Arrange
        var json = @"{
            ""from"": ""client-001"",
            ""to"": ""server-001"",
            ""operation"": ""data"",
            ""message_id"": ""msg-123"",
            ""timestamp"": ""2024-01-01T00:00:00.000Z""
        }";

        // Act
        var envelope = Envelope.FromJson(json);

        // Assert
        Assert.Equal("client-001", envelope.FromId);
        Assert.Equal("server-001", envelope.ToId);
        Assert.Equal(OperationType.Data, envelope.Operation);
        Assert.Equal("msg-123", envelope.MessageId);
    }

    [Fact]
    public void Envelope_ShouldComputeHash()
    {
        // Arrange
        var envelope = new Envelope(
            fromId: "client-001",
            toId: "server-001",
            operation: OperationType.Data,
            messageId: "msg-123"
        );

        // Act
        var hash = envelope.ComputeHash();

        // Assert
        Assert.NotNull(hash);
        Assert.NotEmpty(hash);
        Assert.Equal(64, hash.Length); // SHA-256 hex string
    }

    [Fact]
    public void EnvelopeBuilder_ShouldBuildValidEnvelope()
    {
        // Arrange & Act
        var envelope = new EnvelopeBuilder()
            .FromId("client-001")
            .ToId("server-001")
            .Operation(OperationType.Request)
            .Capability("version", "1.0.0")
            .Build();

        // Assert
        Assert.Equal("client-001", envelope.FromId);
        Assert.Equal("server-001", envelope.ToId);
        Assert.Equal(OperationType.Request, envelope.Operation);
        Assert.NotNull(envelope.Capabilities);
        Assert.Equal("1.0.0", envelope.Capabilities["version"]);
        Assert.NotNull(envelope.Hash);
    }

    [Fact]
    public void EnvelopeBuilder_ShouldThrowValidationException_WhenRequiredFieldsMissing()
    {
        // Arrange
        var builder = new EnvelopeBuilder()
            .FromId("client-001");

        // Act & Assert
        Assert.Throws<ValidationException>(() => builder.Build());
    }
}

