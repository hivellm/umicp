using Umicp.SDK;

namespace Umicp.Examples;

/// <summary>
/// Matrix operations example with SIMD acceleration
/// </summary>
public class MatrixOperationsExample
{
    public static void Run()
    {
        Console.WriteLine("=== Matrix Operations Example ===\n");

        // Dot product
        var vectorA = new float[] { 1.0f, 2.0f, 3.0f, 4.0f };
        var vectorB = new float[] { 5.0f, 6.0f, 7.0f, 8.0f };

        var dotProduct = Matrix.DotProduct(vectorA, vectorB);
        Console.WriteLine($"Dot Product: {dotProduct}");

        // Cosine similarity
        var similarity = Matrix.CosineSimilarity(vectorA, vectorB);
        Console.WriteLine($"Cosine Similarity: {similarity:F4}");

        // Magnitude
        var magnitude = Matrix.Magnitude(vectorA);
        Console.WriteLine($"Magnitude of A: {magnitude:F4}");

        // Matrix addition
        var sum = Matrix.Add(vectorA, vectorB);
        Console.WriteLine($"Sum: [{string.Join(", ", sum.Select(v => v.ToString("F1")))}]");

        // Matrix scaling
        var scaled = Matrix.Scale(vectorA, 2.0f);
        Console.WriteLine($"Scaled (x2): [{string.Join(", ", scaled.Select(v => v.ToString("F1")))}]");

        // Euclidean distance
        var distance = Matrix.EuclideanDistance(vectorA, vectorB);
        Console.WriteLine($"Euclidean Distance: {distance:F4}");

        // Matrix multiplication (2x2 * 2x2)
        var matrixA = new float[] { 1.0f, 2.0f, 3.0f, 4.0f };
        var matrixB = new float[] { 5.0f, 6.0f, 7.0f, 8.0f };
        var product = Matrix.MatrixMultiply(matrixA, matrixB, 2, 2, 2);
        Console.WriteLine($"Matrix Product: [{string.Join(", ", product.Select(v => v.ToString("F1")))}]");

        // Transpose
        var transposed = Matrix.Transpose(matrixA, 2, 2);
        Console.WriteLine($"Transposed: [{string.Join(", ", transposed.Select(v => v.ToString("F1")))}]");

        Console.WriteLine();
    }
}

