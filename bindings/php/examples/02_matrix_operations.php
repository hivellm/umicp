<?php

declare(strict_types=1);

require_once __DIR__ . '/../vendor/autoload.php';

use UMICP\Core\Matrix;

echo "UMICP PHP Bindings - Matrix Operations Example\n";
echo "===============================================\n\n";

try {
    // Create matrix instance
    echo "1. Creating Matrix Instance...\n";
    $matrix = new Matrix();
    echo "   ✓ Matrix created\n\n";

    // Define test vectors
    $vec1 = [1.0, 2.0, 3.0, 4.0];
    $vec2 = [5.0, 6.0, 7.0, 8.0];

    echo "2. Test Vectors:\n";
    echo "   Vector 1: [" . implode(', ', $vec1) . "]\n";
    echo "   Vector 2: [" . implode(', ', $vec2) . "]\n\n";

    // Dot product
    echo "3. Dot Product:\n";
    $startTime = microtime(true);
    $dotProduct = $matrix->dotProduct($vec1, $vec2);
    $time = (microtime(true) - $startTime) * 1000;
    echo "   Result: $dotProduct\n";
    echo "   Time: " . number_format($time, 3) . "ms\n\n";

    // Cosine similarity
    echo "4. Cosine Similarity:\n";
    $startTime = microtime(true);
    $similarity = $matrix->cosineSimilarity($vec1, $vec2);
    $time = (microtime(true) - $startTime) * 1000;
    echo "   Result: " . number_format($similarity, 6) . "\n";
    echo "   Time: " . number_format($time, 3) . "ms\n\n";

    // Vector addition
    echo "5. Vector Addition:\n";
    $startTime = microtime(true);
    $sum = $matrix->vectorAdd($vec1, $vec2);
    $time = (microtime(true) - $startTime) * 1000;
    echo "   Result: [" . implode(', ', $sum) . "]\n";
    echo "   Time: " . number_format($time, 3) . "ms\n\n";

    // Vector subtraction
    echo "6. Vector Subtraction:\n";
    $startTime = microtime(true);
    $diff = $matrix->vectorSubtract($vec2, $vec1);
    $time = (microtime(true) - $startTime) * 1000;
    echo "   Result: [" . implode(', ', $diff) . "]\n";
    echo "   Time: " . number_format($time, 3) . "ms\n\n";

    // Vector scaling
    echo "7. Vector Scaling (x2):\n";
    $startTime = microtime(true);
    $scaled = $matrix->vectorScale($vec1, 2.0);
    $time = (microtime(true) - $startTime) * 1000;
    echo "   Result: [" . implode(', ', $scaled) . "]\n";
    echo "   Time: " . number_format($time, 3) . "ms\n\n";

    // Vector magnitude
    echo "8. Vector Magnitude:\n";
    $startTime = microtime(true);
    $magnitude = $matrix->vectorMagnitude($vec1);
    $time = (microtime(true) - $startTime) * 1000;
    echo "   Result: " . number_format($magnitude, 6) . "\n";
    echo "   Time: " . number_format($time, 3) . "ms\n\n";

    // Vector normalization
    echo "9. Vector Normalization:\n";
    $startTime = microtime(true);
    $normalized = $matrix->vectorNormalize($vec1);
    $time = (microtime(true) - $startTime) * 1000;
    echo "   Result: [" . implode(', ', array_map(fn($v) => number_format($v, 4), $normalized)) . "]\n";
    echo "   Magnitude of normalized: " . number_format($matrix->vectorMagnitude($normalized), 6) . "\n";
    echo "   Time: " . number_format($time, 3) . "ms\n\n";

    // Matrix multiplication
    echo "10. Matrix Multiplication (2x2):\n";
    $matrixA = [1, 2, 3, 4]; // [[1, 2], [3, 4]]
    $matrixB = [5, 6, 7, 8]; // [[5, 6], [7, 8]]
    $startTime = microtime(true);
    $result = $matrix->matrixMultiply($matrixA, $matrixB, 2, 2, 2);
    $time = (microtime(true) - $startTime) * 1000;
    echo "   Matrix A: [[1, 2], [3, 4]]\n";
    echo "   Matrix B: [[5, 6], [7, 8]]\n";
    echo "   Result: [[{$result[0]}, {$result[1]}], [{$result[2]}, {$result[3]}]]\n";
    echo "   Time: " . number_format($time, 3) . "ms\n\n";

    // Matrix transpose
    echo "11. Matrix Transpose (2x3):\n";
    $matrixC = [1, 2, 3, 4, 5, 6]; // [[1, 2, 3], [4, 5, 6]]
    $startTime = microtime(true);
    $transposed = $matrix->matrixTranspose($matrixC, 2, 3);
    $time = (microtime(true) - $startTime) * 1000;
    echo "   Original (2x3): [[1, 2, 3], [4, 5, 6]]\n";
    echo "   Transposed (3x2): [[{$transposed[0]}, {$transposed[1]}], [{$transposed[2]}, {$transposed[3]}], [{$transposed[4]}, {$transposed[5]}]]\n";
    echo "   Time: " . number_format($time, 3) . "ms\n\n";

    echo "✅ All matrix operations completed successfully!\n";

} catch (\Throwable $e) {
    echo "❌ Error: " . $e->getMessage() . "\n";
    echo "   File: " . $e->getFile() . ":" . $e->getLine() . "\n";
    exit(1);
}

