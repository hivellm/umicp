<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Core;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Matrix;
use InvalidArgumentException;

/**
 * @covers \UMICP\Core\Matrix
 */
class MatrixTest extends TestCase
{
    private Matrix $matrix;

    protected function setUp(): void
    {
        $this->matrix = new Matrix();
    }

    public function testDotProduct(): void
    {
        $result = $this->matrix->dotProduct([1, 2, 3], [4, 5, 6]);

        // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
        $this->assertEquals(32.0, $result);
    }

    public function testDotProductWithDifferentSizes(): void
    {
        $this->expectException(InvalidArgumentException::class);
        $this->expectExceptionMessage('Vectors must have the same length');

        $this->matrix->dotProduct([1, 2], [4, 5, 6]);
    }

    public function testDotProductWithEmptyVectors(): void
    {
        $this->expectException(InvalidArgumentException::class);
        $this->expectExceptionMessage('Vectors cannot be empty');

        $this->matrix->dotProduct([], []);
    }

    public function testCosineSimilarity(): void
    {
        $vec1 = [1, 2, 3];
        $vec2 = [4, 5, 6];

        $similarity = $this->matrix->cosineSimilarity($vec1, $vec2);

        $this->assertIsFloat($similarity);
        $this->assertGreaterThanOrEqual(-1.0, $similarity);
        $this->assertLessThanOrEqual(1.0, $similarity);
    }

    public function testVectorAdd(): void
    {
        $result = $this->matrix->vectorAdd([1, 2, 3], [4, 5, 6]);

        $this->assertEquals([5, 7, 9], $result);
    }

    public function testVectorSubtract(): void
    {
        $result = $this->matrix->vectorSubtract([10, 8, 6], [1, 2, 3]);

        $this->assertEquals([9, 6, 3], $result);
    }

    public function testVectorScale(): void
    {
        $result = $this->matrix->vectorScale([1, 2, 3], 2.0);

        $this->assertEquals([2, 4, 6], $result);
    }

    public function testVectorScaleWithZero(): void
    {
        $result = $this->matrix->vectorScale([1, 2, 3], 0.0);

        $this->assertEquals([0, 0, 0], $result);
    }

    public function testVectorMagnitude(): void
    {
        // [3, 4] should have magnitude 5 (3-4-5 triangle)
        $magnitude = $this->matrix->vectorMagnitude([3, 4]);

        $this->assertEquals(5.0, $magnitude);
    }

    public function testVectorNormalize(): void
    {
        $normalized = $this->matrix->vectorNormalize([3, 4]);
        $magnitude = $this->matrix->vectorMagnitude($normalized);

        // Normalized vector should have magnitude 1
        $this->assertEqualsWithDelta(1.0, $magnitude, 0.0001);
    }

    public function testMatrixMultiply2x2(): void
    {
        $matA = [1, 2, 3, 4]; // [[1, 2], [3, 4]]
        $matB = [5, 6, 7, 8]; // [[5, 6], [7, 8]]

        $result = $this->matrix->matrixMultiply($matA, $matB, 2, 2, 2);

        // [[1*5+2*7, 1*6+2*8], [3*5+4*7, 3*6+4*8]]
        // [[19, 22], [43, 50]]
        $this->assertEquals([19, 22, 43, 50], $result);
    }

    public function testMatrixMultiplyWithInvalidDimensions(): void
    {
        $this->expectException(InvalidArgumentException::class);

        $this->matrix->matrixMultiply([1, 2], [3, 4], 2, 1, 2);
    }

    public function testMatrixTranspose(): void
    {
        // 2x3 matrix [[1, 2, 3], [4, 5, 6]]
        $matrix = [1, 2, 3, 4, 5, 6];

        $transposed = $this->matrix->matrixTranspose($matrix, 2, 3);

        // Result should be 3x2: [[1, 4], [2, 5], [3, 6]]
        $this->assertEquals([1, 4, 2, 5, 3, 6], $transposed);
    }

    public function testMatrixTransposeSquare(): void
    {
        // 2x2 matrix [[1, 2], [3, 4]]
        $matrix = [1, 2, 3, 4];

        $transposed = $this->matrix->matrixTranspose($matrix, 2, 2);

        // [[1, 3], [2, 4]]
        $this->assertEquals([1, 3, 2, 4], $transposed);
    }
}

