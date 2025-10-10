using Xunit;
using Umicp.Core;
using Umicp.Core.Exceptions;

namespace Umicp.Tests;

public class MatrixTests
{
    [Fact]
    public void DotProduct_ShouldCalculateCorrectly()
    {
        // Arrange
        var a = new float[] { 1.0f, 2.0f, 3.0f };
        var b = new float[] { 4.0f, 5.0f, 6.0f };

        // Act
        var result = Matrix.DotProduct(a, b);

        // Assert
        Assert.Equal(32.0f, result); // 1*4 + 2*5 + 3*6 = 32
    }

    [Fact]
    public void CosineSimilarity_ShouldCalculateCorrectly()
    {
        // Arrange
        var a = new float[] { 1.0f, 0.0f, 0.0f };
        var b = new float[] { 1.0f, 0.0f, 0.0f };

        // Act
        var result = Matrix.CosineSimilarity(a, b);

        // Assert
        Assert.Equal(1.0f, result, 5); // Identical vectors should have similarity of 1
    }

    [Fact]
    public void Magnitude_ShouldCalculateCorrectly()
    {
        // Arrange
        var vector = new float[] { 3.0f, 4.0f };

        // Act
        var result = Matrix.Magnitude(vector);

        // Assert
        Assert.Equal(5.0f, result, 5); // sqrt(3^2 + 4^2) = 5
    }

    [Fact]
    public void Add_ShouldAddMatricesCorrectly()
    {
        // Arrange
        var a = new float[] { 1.0f, 2.0f, 3.0f };
        var b = new float[] { 4.0f, 5.0f, 6.0f };

        // Act
        var result = Matrix.Add(a, b);

        // Assert
        Assert.Equal(new float[] { 5.0f, 7.0f, 9.0f }, result);
    }

    [Fact]
    public void Subtract_ShouldSubtractMatricesCorrectly()
    {
        // Arrange
        var a = new float[] { 5.0f, 7.0f, 9.0f };
        var b = new float[] { 1.0f, 2.0f, 3.0f };

        // Act
        var result = Matrix.Subtract(a, b);

        // Assert
        Assert.Equal(new float[] { 4.0f, 5.0f, 6.0f }, result);
    }

    [Fact]
    public void Scale_ShouldScaleMatrixCorrectly()
    {
        // Arrange
        var matrix = new float[] { 1.0f, 2.0f, 3.0f };
        var scalar = 2.0f;

        // Act
        var result = Matrix.Scale(matrix, scalar);

        // Assert
        Assert.Equal(new float[] { 2.0f, 4.0f, 6.0f }, result);
    }

    [Fact]
    public void Transpose_ShouldTransposeMatrixCorrectly()
    {
        // Arrange (2x3 matrix)
        var matrix = new float[] { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f };

        // Act (transpose to 3x2)
        var result = Matrix.Transpose(matrix, rows: 2, cols: 3);

        // Assert
        Assert.Equal(new float[] { 1.0f, 4.0f, 2.0f, 5.0f, 3.0f, 6.0f }, result);
    }

    [Fact]
    public void EuclideanDistance_ShouldCalculateCorrectly()
    {
        // Arrange
        var a = new float[] { 0.0f, 0.0f };
        var b = new float[] { 3.0f, 4.0f };

        // Act
        var result = Matrix.EuclideanDistance(a, b);

        // Assert
        Assert.Equal(5.0f, result, 5); // sqrt(3^2 + 4^2) = 5
    }

    [Fact]
    public void DotProduct_ShouldThrowException_WhenDimensionsMismatch()
    {
        // Arrange
        var a = new float[] { 1.0f, 2.0f };
        var b = new float[] { 3.0f, 4.0f, 5.0f };

        // Act & Assert
        Assert.Throws<MatrixOperationException>(() => Matrix.DotProduct(a, b));
    }
}

