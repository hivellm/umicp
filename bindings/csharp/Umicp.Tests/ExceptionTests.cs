using Xunit;
using Umicp.SDK.Exceptions;

namespace Umicp.Tests;

public class ExceptionTests
{
    [Fact]
    public void UmicpException_ShouldCreateWithMessage()
    {
        // Arrange & Act
        var ex = new UmicpException("Test error");

        // Assert
        Assert.Equal("Test error", ex.Message);
        Assert.Null(ex.Context);
    }

    [Fact]
    public void UmicpException_ShouldCreateWithMessageAndContext()
    {
        // Arrange
        var context = new Dictionary<string, object>
        {
            ["key1"] = "value1",
            ["key2"] = 42
        };

        // Act
        var ex = new UmicpException("Test error", context);

        // Assert
        Assert.Equal("Test error", ex.Message);
        Assert.NotNull(ex.Context);
        Assert.Equal("value1", ex.Context["key1"]);
        Assert.Equal(42, ex.Context["key2"]);
    }

    [Fact]
    public void UmicpException_ShouldCreateWithInnerException()
    {
        // Arrange
        var inner = new InvalidOperationException("Inner error");

        // Act
        var ex = new UmicpException("Outer error", inner);

        // Assert
        Assert.Equal("Outer error", ex.Message);
        Assert.NotNull(ex.InnerException);
        Assert.Equal("Inner error", ex.InnerException.Message);
    }

    [Fact]
    public void ValidationException_ShouldInheritFromUmicpException()
    {
        // Arrange & Act
        var ex = new ValidationException("Validation failed");

        // Assert
        Assert.IsType<ValidationException>(ex);
        Assert.IsAssignableFrom<UmicpException>(ex);
        Assert.Equal("Validation failed", ex.Message);
    }

    [Fact]
    public void SerializationException_ShouldInheritFromUmicpException()
    {
        // Arrange & Act
        var ex = new SerializationException("Serialization failed");

        // Assert
        Assert.IsType<SerializationException>(ex);
        Assert.IsAssignableFrom<UmicpException>(ex);
        Assert.Equal("Serialization failed", ex.Message);
    }

    [Fact]
    public void TransportException_ShouldInheritFromUmicpException()
    {
        // Arrange & Act
        var ex = new TransportException("Transport failed");

        // Assert
        Assert.IsType<TransportException>(ex);
        Assert.IsAssignableFrom<UmicpException>(ex);
        Assert.Equal("Transport failed", ex.Message);
    }

    [Fact]
    public void MatrixOperationException_ShouldInheritFromUmicpException()
    {
        // Arrange & Act
        var ex = new MatrixOperationException("Matrix operation failed");

        // Assert
        Assert.IsType<MatrixOperationException>(ex);
        Assert.IsAssignableFrom<UmicpException>(ex);
        Assert.Equal("Matrix operation failed", ex.Message);
    }

    [Fact]
    public void AllExceptions_ShouldSupportInnerException()
    {
        // Arrange
        var inner = new Exception("Inner");

        // Act
        var validation = new ValidationException("Test", inner);
        var serialization = new SerializationException("Test", inner);
        var transport = new TransportException("Test", inner);
        var matrix = new MatrixOperationException("Test", inner);

        // Assert
        Assert.Equal(inner, validation.InnerException);
        Assert.Equal(inner, serialization.InnerException);
        Assert.Equal(inner, transport.InnerException);
        Assert.Equal(inner, matrix.InnerException);
    }
}

