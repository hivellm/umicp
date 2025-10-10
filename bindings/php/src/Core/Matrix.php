<?php

declare(strict_types=1);

namespace UMICP\Core;

use FFI;
use FFI\CData;
use InvalidArgumentException;
use UMICP\FFI\FFIBridge;
use UMICP\FFI\Traits\AutoCleanup;
use UMICP\FFI\TypeConverter;

/**
 * UMICP Matrix - High-performance matrix and vector operations
 *
 * @package UMICP\Core
 */
class Matrix
{
    use AutoCleanup;

    /**
     * Native C matrix instance
     *
     * @var CData
     */
    private CData $nativeMatrix;

    /**
     * FFI bridge instance
     *
     * @var FFIBridge
     */
    private FFIBridge $ffi;

    /**
     * Create a new matrix instance
     */
    public function __construct()
    {
        $this->ffi = FFIBridge::getInstance();
        $this->nativeMatrix = $this->ffi->createMatrix();

        // Register cleanup
        $this->registerCleanup(fn() => $this->ffi->destroyMatrix($this->nativeMatrix));
    }

    /**
     * Calculate dot product of two vectors
     *
     * @param array<float|int> $a First vector
     * @param array<float|int> $b Second vector
     * @return float Dot product result
     * @throws InvalidArgumentException If vectors have different lengths
     */
    public function dotProduct(array $a, array $b): float
    {
        $sizeA = count($a);
        $sizeB = count($b);

        if ($sizeA !== $sizeB) {
            throw new InvalidArgumentException(
                "Vectors must have the same length. Got {$sizeA} and {$sizeB}"
            );
        }

        if ($sizeA === 0) {
            throw new InvalidArgumentException('Vectors cannot be empty');
        }

        $cArrayA = TypeConverter::phpArrayToCFloatArray($a);
        $cArrayB = TypeConverter::phpArrayToCFloatArray($b);

        $result = $this->ffi->getFFI()->umicp_matrix_dot_product(
            $this->nativeMatrix,
            $cArrayA,
            $cArrayB,
            $sizeA
        );

        return $result;
    }

    /**
     * Calculate cosine similarity between two vectors
     *
     * @param array<float|int> $a First vector
     * @param array<float|int> $b Second vector
     * @return float Cosine similarity (-1 to 1)
     * @throws InvalidArgumentException If vectors have different lengths
     */
    public function cosineSimilarity(array $a, array $b): float
    {
        $sizeA = count($a);
        $sizeB = count($b);

        if ($sizeA !== $sizeB) {
            throw new InvalidArgumentException(
                "Vectors must have the same length. Got {$sizeA} and {$sizeB}"
            );
        }

        if ($sizeA === 0) {
            throw new InvalidArgumentException('Vectors cannot be empty');
        }

        $cArrayA = TypeConverter::phpArrayToCFloatArray($a);
        $cArrayB = TypeConverter::phpArrayToCFloatArray($b);

        $result = $this->ffi->getFFI()->umicp_matrix_cosine_similarity(
            $this->nativeMatrix,
            $cArrayA,
            $cArrayB,
            $sizeA
        );

        return $result;
    }

    /**
     * Add two vectors element-wise
     *
     * @param array<float|int> $a First vector
     * @param array<float|int> $b Second vector
     * @return array<float> Result vector
     * @throws InvalidArgumentException If vectors have different lengths
     */
    public function vectorAdd(array $a, array $b): array
    {
        $sizeA = count($a);
        $sizeB = count($b);

        if ($sizeA !== $sizeB) {
            throw new InvalidArgumentException(
                "Vectors must have the same length. Got {$sizeA} and {$sizeB}"
            );
        }

        if ($sizeA === 0) {
            throw new InvalidArgumentException('Vectors cannot be empty');
        }

        $cArrayA = TypeConverter::phpArrayToCFloatArray($a);
        $cArrayB = TypeConverter::phpArrayToCFloatArray($b);
        $cResult = FFI::new("float[$sizeA]");

        $this->ffi->getFFI()->umicp_matrix_vector_add(
            $this->nativeMatrix,
            $cArrayA,
            $cArrayB,
            $cResult,
            $sizeA
        );

        return TypeConverter::cFloatArrayToPhpArray($cResult, $sizeA);
    }

    /**
     * Subtract two vectors element-wise
     *
     * @param array<float|int> $a First vector
     * @param array<float|int> $b Second vector
     * @return array<float> Result vector
     * @throws InvalidArgumentException If vectors have different lengths
     */
    public function vectorSubtract(array $a, array $b): array
    {
        $sizeA = count($a);
        $sizeB = count($b);

        if ($sizeA !== $sizeB) {
            throw new InvalidArgumentException(
                "Vectors must have the same length. Got {$sizeA} and {$sizeB}"
            );
        }

        if ($sizeA === 0) {
            throw new InvalidArgumentException('Vectors cannot be empty');
        }

        $cArrayA = TypeConverter::phpArrayToCFloatArray($a);
        $cArrayB = TypeConverter::phpArrayToCFloatArray($b);
        $cResult = FFI::new("float[$sizeA]");

        $this->ffi->getFFI()->umicp_matrix_vector_subtract(
            $this->nativeMatrix,
            $cArrayA,
            $cArrayB,
            $cResult,
            $sizeA
        );

        return TypeConverter::cFloatArrayToPhpArray($cResult, $sizeA);
    }

    /**
     * Multiply vector by scalar
     *
     * @param array<float|int> $vector Vector
     * @param float $scalar Scalar value
     * @return array<float> Result vector
     * @throws InvalidArgumentException If vector is empty
     */
    public function vectorScale(array $vector, float $scalar): array
    {
        $size = count($vector);

        if ($size === 0) {
            throw new InvalidArgumentException('Vector cannot be empty');
        }

        $cVector = TypeConverter::phpArrayToCFloatArray($vector);
        $cResult = FFI::new("float[$size]");

        $this->ffi->getFFI()->umicp_matrix_vector_scale(
            $this->nativeMatrix,
            $cVector,
            $scalar,
            $cResult,
            $size
        );

        return TypeConverter::cFloatArrayToPhpArray($cResult, $size);
    }

    /**
     * Calculate vector magnitude (L2 norm)
     *
     * @param array<float|int> $vector Vector
     * @return float Magnitude
     * @throws InvalidArgumentException If vector is empty
     */
    public function vectorMagnitude(array $vector): float
    {
        $size = count($vector);

        if ($size === 0) {
            throw new InvalidArgumentException('Vector cannot be empty');
        }

        $cVector = TypeConverter::phpArrayToCFloatArray($vector);

        $result = $this->ffi->getFFI()->umicp_matrix_vector_magnitude(
            $this->nativeMatrix,
            $cVector,
            $size
        );

        return $result;
    }

    /**
     * Normalize vector to unit length
     *
     * @param array<float|int> $vector Vector
     * @return array<float> Normalized vector
     * @throws InvalidArgumentException If vector is empty or zero
     */
    public function vectorNormalize(array $vector): array
    {
        $size = count($vector);

        if ($size === 0) {
            throw new InvalidArgumentException('Vector cannot be empty');
        }

        $cVector = TypeConverter::phpArrayToCFloatArray($vector);
        $cResult = FFI::new("float[$size]");

        $this->ffi->getFFI()->umicp_matrix_vector_normalize(
            $this->nativeMatrix,
            $cVector,
            $cResult,
            $size
        );

        return TypeConverter::cFloatArrayToPhpArray($cResult, $size);
    }

    /**
     * Multiply two matrices
     *
     * @param array<float|int> $a First matrix (flat array, row-major)
     * @param array<float|int> $b Second matrix (flat array, row-major)
     * @param int $m Rows of A
     * @param int $n Columns of A / Rows of B
     * @param int $p Columns of B
     * @return array<float> Result matrix (flat array, row-major)
     * @throws InvalidArgumentException If dimensions are incompatible
     */
    public function matrixMultiply(array $a, array $b, int $m, int $n, int $p): array
    {
        $sizeA = count($a);
        $sizeB = count($b);
        $expectedA = $m * $n;
        $expectedB = $n * $p;

        if ($sizeA !== $expectedA) {
            throw new InvalidArgumentException(
                "Matrix A size mismatch. Expected {$expectedA} ({$m}x{$n}), got {$sizeA}"
            );
        }

        if ($sizeB !== $expectedB) {
            throw new InvalidArgumentException(
                "Matrix B size mismatch. Expected {$expectedB} ({$n}x{$p}), got {$sizeB}"
            );
        }

        $cArrayA = TypeConverter::phpArrayToCFloatArray($a);
        $cArrayB = TypeConverter::phpArrayToCFloatArray($b);
        $resultSize = $m * $p;
        $cResult = FFI::new("float[$resultSize]");

        $this->ffi->getFFI()->umicp_matrix_multiply(
            $this->nativeMatrix,
            $cArrayA,
            $cArrayB,
            $cResult,
            $m,
            $n,
            $p
        );

        return TypeConverter::cFloatArrayToPhpArray($cResult, $resultSize);
    }

    /**
     * Transpose matrix
     *
     * @param array<float|int> $matrix Matrix (flat array, row-major)
     * @param int $rows Number of rows
     * @param int $cols Number of columns
     * @return array<float> Transposed matrix
     * @throws InvalidArgumentException If dimensions don't match
     */
    public function matrixTranspose(array $matrix, int $rows, int $cols): array
    {
        $size = count($matrix);
        $expected = $rows * $cols;

        if ($size !== $expected) {
            throw new InvalidArgumentException(
                "Matrix size mismatch. Expected {$expected} ({$rows}x{$cols}), got {$size}"
            );
        }

        $cMatrix = TypeConverter::phpArrayToCFloatArray($matrix);
        $cResult = FFI::new("float[$size]");

        $this->ffi->getFFI()->umicp_matrix_transpose(
            $this->nativeMatrix,
            $cMatrix,
            $cResult,
            $rows,
            $cols
        );

        return TypeConverter::cFloatArrayToPhpArray($cResult, $size);
    }
}

