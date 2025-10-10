<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Core;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Matrix;
use InvalidArgumentException;

/**
 * Advanced matrix tests (edge cases, large operations)
 *
 * @covers \UMICP\Core\Matrix
 */
class MatrixAdvancedTest extends TestCase
{
    private Matrix $matrix;

    protected function setUp(): void
    {
        $this->matrix = new Matrix();
    }

    public function testZeroVector(): void
    {
        $zero = [0.0, 0.0, 0.0];
        $vec = [1.0, 2.0, 3.0];

        $dotProduct = $this->matrix->dotProduct($zero, $vec);
        $this->assertEquals(0.0, $dotProduct);

        $sum = $this->matrix->vectorAdd($zero, $vec);
        $this->assertEquals($vec, $sum);
    }

    public function testNegativeVectors(): void
    {
        $vec1 = [-1.0, -2.0, -3.0];
        $vec2 = [4.0, 5.0, 6.0];

        $dotProduct = $this->matrix->dotProduct($vec1, $vec2);
        // -1*4 + -2*5 + -3*6 = -4 + -10 + -18 = -32
        $this->assertEquals(-32.0, $dotProduct);

        $sum = $this->matrix->vectorAdd($vec1, $vec2);
        $this->assertEquals([3.0, 3.0, 3.0], $sum);
    }

    public function testVectorScaleNegative(): void
    {
        $vec = [1.0, 2.0, 3.0];
        $scaled = $this->matrix->vectorScale($vec, -2.0);

        $this->assertEquals([-2.0, -4.0, -6.0], $scaled);
    }

    public function testVectorNormalizationIdempotent(): void
    {
        $vec = [3.0, 4.0];

        $normalized1 = $this->matrix->vectorNormalize($vec);
        $normalized2 = $this->matrix->vectorNormalize($normalized1);

        // Normalizing a normalized vector should be idempotent
        $this->assertEqualsWithDelta($normalized1[0], $normalized2[0], 0.0001);
        $this->assertEqualsWithDelta($normalized1[1], $normalized2[1], 0.0001);
    }

    public function testLargeMatrixMultiplication(): void
    {
        // 10x10 matrix multiplication
        $size = 10;
        $matA = array_fill(0, $size * $size, 1.0);
        $matB = array_fill(0, $size * $size, 2.0);

        $result = $this->matrix->matrixMultiply($matA, $matB, $size, $size, $size);

        $this->assertCount($size * $size, $result);

        // Each element should be 2 * size (1*2 summed size times)
        $expected = 2.0 * $size;
        foreach ($result as $value) {
            $this->assertEqualsWithDelta($expected, $value, 0.001);
        }
    }

    public function testIdentityMatrix(): void
    {
        // 2x2 identity matrix
        $identity = [1, 0, 0, 1];
        $matrix = [5, 6, 7, 8];

        $result = $this->matrix->matrixMultiply($identity, $matrix, 2, 2, 2);

        // Identity * Matrix = Matrix
        $this->assertEquals($matrix, $result);
    }

    public function testMatrixTransposeTwice(): void
    {
        $matrix = [1, 2, 3, 4, 5, 6]; // 2x3

        $transposed = $this->matrix->matrixTranspose($matrix, 2, 3); // 3x2
        $restored = $this->matrix->matrixTranspose($transposed, 3, 2); // 2x3

        // Double transpose should restore original
        $this->assertEquals($matrix, $restored);
    }

    public function testCosineSimilarityOppositeVectors(): void
    {
        $vec1 = [1.0, 0.0];
        $vec2 = [-1.0, 0.0];

        $similarity = $this->matrix->cosineSimilarity($vec1, $vec2);

        // Opposite vectors should have similarity -1
        $this->assertEqualsWithDelta(-1.0, $similarity, 0.0001);
    }

    public function testCosineSimilarityParallelVectors(): void
    {
        $vec1 = [1.0, 2.0, 3.0];
        $vec2 = [2.0, 4.0, 6.0]; // 2x vec1

        $similarity = $this->matrix->cosineSimilarity($vec1, $vec2);

        // Parallel vectors should have similarity 1
        $this->assertEqualsWithDelta(1.0, $similarity, 0.0001);
    }

    public function testVectorOperationsChain(): void
    {
        $vec1 = [1.0, 2.0, 3.0];
        $vec2 = [4.0, 5.0, 6.0];

        // Chain operations
        $sum = $this->matrix->vectorAdd($vec1, $vec2);      // [5, 7, 9]
        $scaled = $this->matrix->vectorScale($sum, 0.5);    // [2.5, 3.5, 4.5]
        $normalized = $this->matrix->vectorNormalize($scaled);

        $magnitude = $this->matrix->vectorMagnitude($normalized);
        $this->assertEqualsWithDelta(1.0, $magnitude, 0.0001);
    }

    public function testMixedIntegerAndFloatVectors(): void
    {
        $vec1 = [1, 2, 3];    // Integers
        $vec2 = [1.5, 2.5, 3.5]; // Floats

        $dotProduct = $this->matrix->dotProduct($vec1, $vec2);
        // 1*1.5 + 2*2.5 + 3*3.5 = 1.5 + 5.0 + 10.5 = 17.0
        $this->assertEqualsWithDelta(17.0, $dotProduct, 0.0001);
    }

    public function testSingleElementVectors(): void
    {
        $vec1 = [5.0];
        $vec2 = [3.0];

        $dotProduct = $this->matrix->dotProduct($vec1, $vec2);
        $this->assertEquals(15.0, $dotProduct);

        $sum = $this->matrix->vectorAdd($vec1, $vec2);
        $this->assertEquals([8.0], $sum);
    }

    public function testVeryLargeVectors(): void
    {
        // Test with 10,000 element vectors
        $vec1 = array_fill(0, 10000, 0.5);
        $vec2 = array_fill(0, 10000, 0.5);

        $dotProduct = $this->matrix->dotProduct($vec1, $vec2);
        // 0.5 * 0.5 * 10000 = 2500
        $this->assertEquals(2500.0, $dotProduct);

        $similarity = $this->matrix->cosineSimilarity($vec1, $vec2);
        $this->assertEqualsWithDelta(1.0, $similarity, 0.0001);
    }
}

