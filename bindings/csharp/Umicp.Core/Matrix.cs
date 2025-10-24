using System.Numerics;
using Umicp.SDK.Exceptions;

namespace Umicp.SDK;

/// <summary>
/// Result of matrix operations
/// </summary>
public class MatrixResult
{
    /// <summary>
    /// Array result for vector/matrix operations
    /// </summary>
    public float[]? Result { get; set; }

    /// <summary>
    /// Scalar result for dot product and similar operations
    /// </summary>
    public float ScalarResult { get; set; }

    /// <summary>
    /// Number of rows in the result matrix
    /// </summary>
    public int Rows { get; set; }

    /// <summary>
    /// Number of columns in the result matrix
    /// </summary>
    public int Cols { get; set; }
}

/// <summary>
/// Result of dot product operation
/// </summary>
public class DotProductResult
{
    /// <summary>
    /// Dot product result value
    /// </summary>
    public float Result { get; set; }
}

/// <summary>
/// Result of cosine similarity operation
/// </summary>
public class CosineSimilarityResult
{
    /// <summary>
    /// Cosine similarity value (range: -1 to 1)
    /// </summary>
    public float Similarity { get; set; }
}

/// <summary>
/// UMICP Matrix operations with SIMD acceleration
/// </summary>
public static class Matrix
{
    /// <summary>
    /// Compute dot product of two vectors using SIMD acceleration
    /// </summary>
    /// <param name="a">First vector</param>
    /// <param name="b">Second vector</param>
    /// <returns>Dot product result</returns>
    /// <exception cref="MatrixOperationException">Thrown when vector dimensions mismatch</exception>
    public static float DotProduct(float[] a, float[] b)
    {
        if (a.Length != b.Length)
            throw new MatrixOperationException($"Vector dimensions mismatch: {a.Length} vs {b.Length}");

        float sum = 0f;
        int vectorSize = Vector<float>.Count;
        int i = 0;

        // SIMD accelerated loop
        for (; i <= a.Length - vectorSize; i += vectorSize)
        {
            var va = new Vector<float>(a, i);
            var vb = new Vector<float>(b, i);
            sum += Vector.Dot(va, vb);
        }

        // Handle remaining elements
        for (; i < a.Length; i++)
        {
            sum += a[i] * b[i];
        }

        return sum;
    }

    /// <summary>
    /// Compute cosine similarity between two vectors
    /// </summary>
    /// <param name="a">First vector</param>
    /// <param name="b">Second vector</param>
    /// <returns>Cosine similarity value between -1 and 1</returns>
    /// <exception cref="MatrixOperationException">Thrown when vector dimensions mismatch or vectors have zero magnitude</exception>
    public static float CosineSimilarity(float[] a, float[] b)
    {
        if (a.Length != b.Length)
            throw new MatrixOperationException($"Vector dimensions mismatch: {a.Length} vs {b.Length}");

        float dotProduct = DotProduct(a, b);
        float magnitudeA = Magnitude(a);
        float magnitudeB = Magnitude(b);

        if (magnitudeA == 0f || magnitudeB == 0f)
            throw new MatrixOperationException("Cannot compute cosine similarity with zero-magnitude vector");

        return dotProduct / (magnitudeA * magnitudeB);
    }

    /// <summary>
    /// Compute magnitude (L2 norm) of a vector using SIMD acceleration
    /// </summary>
    /// <param name="vector">Input vector</param>
    /// <returns>Vector magnitude (L2 norm)</returns>
    public static float Magnitude(float[] vector)
    {
        float sumOfSquares = 0f;
        int vectorSize = Vector<float>.Count;
        int i = 0;

        // SIMD accelerated loop
        for (; i <= vector.Length - vectorSize; i += vectorSize)
        {
            var v = new Vector<float>(vector, i);
            sumOfSquares += Vector.Dot(v, v);
        }

        // Handle remaining elements
        for (; i < vector.Length; i++)
        {
            sumOfSquares += vector[i] * vector[i];
        }

        return MathF.Sqrt(sumOfSquares);
    }

    /// <summary>
    /// Multiply two matrices
    /// </summary>
    /// <param name="a">First matrix (m x n) in row-major order</param>
    /// <param name="b">Second matrix (n x p) in row-major order</param>
    /// <param name="m">Number of rows in matrix a</param>
    /// <param name="n">Number of columns in matrix a (and rows in matrix b)</param>
    /// <param name="p">Number of columns in matrix b</param>
    /// <returns>Result matrix (m x p) in row-major order</returns>
    public static float[] MatrixMultiply(float[] a, float[] b, int m, int n, int p)
    {
        // a is m x n, b is n x p, result is m x p
        var result = new float[m * p];

        for (int i = 0; i < m; i++)
        {
            for (int j = 0; j < p; j++)
            {
                float sum = 0f;
                for (int k = 0; k < n; k++)
                {
                    sum += a[i * n + k] * b[k * p + j];
                }
                result[i * p + j] = sum;
            }
        }

        return result;
    }

    /// <summary>
    /// Add two matrices element-wise using SIMD acceleration
    /// </summary>
    /// <param name="a">First matrix/vector</param>
    /// <param name="b">Second matrix/vector</param>
    /// <returns>Result of element-wise addition</returns>
    /// <exception cref="MatrixOperationException">Thrown when matrix dimensions mismatch</exception>
    public static float[] Add(float[] a, float[] b)
    {
        if (a.Length != b.Length)
            throw new MatrixOperationException($"Matrix dimensions mismatch: {a.Length} vs {b.Length}");

        var result = new float[a.Length];
        int vectorSize = Vector<float>.Count;
        int i = 0;

        // SIMD accelerated loop
        for (; i <= a.Length - vectorSize; i += vectorSize)
        {
            var va = new Vector<float>(a, i);
            var vb = new Vector<float>(b, i);
            (va + vb).CopyTo(result, i);
        }

        // Handle remaining elements
        for (; i < a.Length; i++)
        {
            result[i] = a[i] + b[i];
        }

        return result;
    }

    /// <summary>
    /// Subtract two matrices element-wise using SIMD acceleration
    /// </summary>
    /// <param name="a">First matrix/vector</param>
    /// <param name="b">Second matrix/vector</param>
    /// <returns>Result of element-wise subtraction (a - b)</returns>
    /// <exception cref="MatrixOperationException">Thrown when matrix dimensions mismatch</exception>
    public static float[] Subtract(float[] a, float[] b)
    {
        if (a.Length != b.Length)
            throw new MatrixOperationException($"Matrix dimensions mismatch: {a.Length} vs {b.Length}");

        var result = new float[a.Length];
        int vectorSize = Vector<float>.Count;
        int i = 0;

        // SIMD accelerated loop
        for (; i <= a.Length - vectorSize; i += vectorSize)
        {
            var va = new Vector<float>(a, i);
            var vb = new Vector<float>(b, i);
            (va - vb).CopyTo(result, i);
        }

        // Handle remaining elements
        for (; i < a.Length; i++)
        {
            result[i] = a[i] - b[i];
        }

        return result;
    }

    /// <summary>
    /// Transpose a matrix
    /// </summary>
    public static float[] Transpose(float[] matrix, int rows, int cols)
    {
        var result = new float[rows * cols];

        for (int i = 0; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                result[j * rows + i] = matrix[i * cols + j];
            }
        }

        return result;
    }

    /// <summary>
    /// Normalize a matrix (L2 normalization)
    /// </summary>
    public static float[] Normalize(float[] matrix, int rows, int cols)
    {
        var result = new float[matrix.Length];

        for (int i = 0; i < rows; i++)
        {
            var row = new float[cols];
            Array.Copy(matrix, i * cols, row, 0, cols);

            float magnitude = Magnitude(row);
            if (magnitude == 0f)
                throw new MatrixOperationException($"Cannot normalize row {i} with zero magnitude");

            for (int j = 0; j < cols; j++)
            {
                result[i * cols + j] = matrix[i * cols + j] / magnitude;
            }
        }

        return result;
    }

    /// <summary>
    /// Scale a matrix by a scalar value
    /// </summary>
    public static float[] Scale(float[] matrix, float scalar)
    {
        var result = new float[matrix.Length];
        int vectorSize = Vector<float>.Count;
        int i = 0;

        var scalarVector = new Vector<float>(scalar);

        // SIMD accelerated loop
        for (; i <= matrix.Length - vectorSize; i += vectorSize)
        {
            var v = new Vector<float>(matrix, i);
            (v * scalarVector).CopyTo(result, i);
        }

        // Handle remaining elements
        for (; i < matrix.Length; i++)
        {
            result[i] = matrix[i] * scalar;
        }

        return result;
    }

    /// <summary>
    /// Calculate Euclidean distance between two vectors
    /// </summary>
    public static float EuclideanDistance(float[] a, float[] b)
    {
        if (a.Length != b.Length)
            throw new MatrixOperationException($"Vector dimensions mismatch: {a.Length} vs {b.Length}");

        float sumOfSquares = 0f;
        int vectorSize = Vector<float>.Count;
        int i = 0;

        // SIMD accelerated loop
        for (; i <= a.Length - vectorSize; i += vectorSize)
        {
            var va = new Vector<float>(a, i);
            var vb = new Vector<float>(b, i);
            var diff = va - vb;
            sumOfSquares += Vector.Dot(diff, diff);
        }

        // Handle remaining elements
        for (; i < a.Length; i++)
        {
            float diff = a[i] - b[i];
            sumOfSquares += diff * diff;
        }

        return MathF.Sqrt(sumOfSquares);
    }
}

