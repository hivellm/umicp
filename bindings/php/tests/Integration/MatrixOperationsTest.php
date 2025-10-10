<?php

declare(strict_types=1);

namespace UMICP\Tests\Integration;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Matrix;

/**
 * Integration tests for matrix operations
 *
 * @group integration
 */
class MatrixOperationsTest extends TestCase
{
    private Matrix $matrix;

    protected function setUp(): void
    {
        $this->matrix = new Matrix();
    }

    public function testVectorOperationsPipeline(): void
    {
        // Create two vectors
        $vec1 = [1.0, 2.0, 3.0, 4.0];
        $vec2 = [5.0, 6.0, 7.0, 8.0];

        // Add them
        $sum = $this->matrix->vectorAdd($vec1, $vec2);

        // Scale the result
        $scaled = $this->matrix->vectorScale($sum, 0.5);

        // Normalize
        $normalized = $this->matrix->vectorNormalize($scaled);

        // Check magnitude is 1
        $magnitude = $this->matrix->vectorMagnitude($normalized);

        $this->assertEqualsWithDelta(1.0, $magnitude, 0.0001);
    }

    public function testMatrixMultiplicationChain(): void
    {
        // Matrix multiplication chain: A * B * C
        $matA = [1, 2, 3, 4]; // 2x2
        $matB = [2, 0, 0, 2]; // 2x2 (scale by 2)
        $matC = [1, 1, 1, 1]; // 2x2

        // A * B
        $ab = $this->matrix->matrixMultiply($matA, $matB, 2, 2, 2);

        // (A * B) * C
        $result = $this->matrix->matrixMultiply($ab, $matC, 2, 2, 2);

        $this->assertIsArray($result);
        $this->assertCount(4, $result);
    }

    public function testDotProductAndSimilarity(): void
    {
        $vec1 = [1.0, 0.0, 0.0];
        $vec2 = [0.0, 1.0, 0.0];

        // Orthogonal vectors
        $dotProduct = $this->matrix->dotProduct($vec1, $vec2);
        $this->assertEquals(0.0, $dotProduct);

        $similarity = $this->matrix->cosineSimilarity($vec1, $vec2);
        $this->assertEqualsWithDelta(0.0, $similarity, 0.0001);

        // Same vector
        $similarity = $this->matrix->cosineSimilarity($vec1, $vec1);
        $this->assertEqualsWithDelta(1.0, $similarity, 0.0001);
    }

    public function testLargeVectorOperations(): void
    {
        // Test with 1000-element vectors
        $vec1 = array_fill(0, 1000, 1.0);
        $vec2 = array_fill(0, 1000, 2.0);

        $startTime = microtime(true);
        $dotProduct = $this->matrix->dotProduct($vec1, $vec2);
        $endTime = microtime(true);

        $this->assertEquals(2000.0, $dotProduct);

        // Should be fast (< 10ms)
        $duration = ($endTime - $startTime) * 1000;
        $this->assertLessThan(10, $duration);
    }

    public function testMatrixTransposeAndMultiply(): void
    {
        // Matrix: 2x3
        $mat = [1, 2, 3, 4, 5, 6];

        // Transpose to 3x2
        $transposed = $this->matrix->matrixTranspose($mat, 2, 3);
        $this->assertCount(6, $transposed);

        // Multiply original (2x3) by transposed (3x2) = 2x2
        $result = $this->matrix->matrixMultiply($mat, $transposed, 2, 3, 2);
        $this->assertCount(4, $result);
    }
}

