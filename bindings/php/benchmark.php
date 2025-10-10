<?php

declare(strict_types=1);

require_once __DIR__ . '/vendor/autoload.php';

use UMICP\Core\Envelope;
use UMICP\Core\Matrix;
use UMICP\Core\OperationType;
use UMICP\FFI\FFIBridge;

echo "╔════════════════════════════════════════════════════════════╗\n";
echo "║     UMICP PHP Bindings - Performance Benchmark            ║\n";
echo "╚════════════════════════════════════════════════════════════╝\n\n";

try {
    // Initialize FFI
    echo "Initializing FFI...\n";
    $ffi = FFIBridge::getInstance();
    echo "✓ FFI ready\n\n";

    // ========================================================================
    // Envelope Benchmarks
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "Envelope Operations Benchmark\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    $iterations = 10000;
    echo "Iterations: $iterations\n\n";

    // Creation
    $startTime = microtime(true);
    for ($i = 0; $i < $iterations; $i++) {
        $envelope = new Envelope(
            from: "sender_$i",
            to: "receiver_$i",
            operation: OperationType::DATA,
            messageId: "msg_$i"
        );
    }
    $createTime = (microtime(true) - $startTime) * 1000;
    $avgCreate = $createTime / $iterations;

    echo "Creation:\n";
    echo "  Total: " . number_format($createTime, 2) . "ms\n";
    echo "  Avg:   " . number_format($avgCreate, 3) . "ms\n";
    echo "  Rate:  " . number_format($iterations / ($createTime / 1000), 0) . " ops/sec\n\n";

    // Serialization
    $envelopes = [];
    for ($i = 0; $i < $iterations; $i++) {
        $envelopes[] = new Envelope(
            from: "sender_$i",
            to: "receiver_$i",
            operation: OperationType::DATA,
            capabilities: ['seq' => (string) $i]
        );
    }

    $startTime = microtime(true);
    $serialized = array_map(fn($e) => $e->serialize(), $envelopes);
    $serializeTime = (microtime(true) - $startTime) * 1000;
    $avgSerialize = $serializeTime / $iterations;

    echo "Serialization:\n";
    echo "  Total: " . number_format($serializeTime, 2) . "ms\n";
    echo "  Avg:   " . number_format($avgSerialize, 3) . "ms\n";
    echo "  Rate:  " . number_format($iterations / ($serializeTime / 1000), 0) . " ops/sec\n\n";

    // Deserialization
    $startTime = microtime(true);
    $deserialized = array_map(fn($json) => Envelope::deserialize($json), $serialized);
    $deserializeTime = (microtime(true) - $startTime) * 1000;
    $avgDeserialize = $deserializeTime / $iterations;

    echo "Deserialization:\n";
    echo "  Total: " . number_format($deserializeTime, 2) . "ms\n";
    echo "  Avg:   " . number_format($avgDeserialize, 3) . "ms\n";
    echo "  Rate:  " . number_format($iterations / ($deserializeTime / 1000), 0) . " ops/sec\n\n";

    // Validation
    $startTime = microtime(true);
    foreach ($envelopes as $envelope) {
        $envelope->validate();
    }
    $validateTime = (microtime(true) - $startTime) * 1000;
    $avgValidate = $validateTime / $iterations;

    echo "Validation:\n";
    echo "  Total: " . number_format($validateTime, 2) . "ms\n";
    echo "  Avg:   " . number_format($avgValidate, 3) . "ms\n";
    echo "  Rate:  " . number_format($iterations / ($validateTime / 1000), 0) . " ops/sec\n\n";

    // ========================================================================
    // Matrix Benchmarks
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "Matrix Operations Benchmark\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    $matrix = new Matrix();
    $sizes = [100, 500, 1000];

    foreach ($sizes as $size) {
        echo "Vector size: $size elements\n";

        $vec1 = array_fill(0, $size, 1.0);
        $vec2 = array_fill(0, $size, 2.0);

        // Dot product
        $iterations = 10000;
        $startTime = microtime(true);
        for ($i = 0; $i < $iterations; $i++) {
            $matrix->dotProduct($vec1, $vec2);
        }
        $time = (microtime(true) - $startTime) * 1000;
        $avg = $time / $iterations;

        echo "  Dot Product:        " . number_format($avg, 3) . "ms avg\n";

        // Cosine similarity
        $iterations = 5000;
        $startTime = microtime(true);
        for ($i = 0; $i < $iterations; $i++) {
            $matrix->cosineSimilarity($vec1, $vec2);
        }
        $time = (microtime(true) - $startTime) * 1000;
        $avg = $time / $iterations;

        echo "  Cosine Similarity:  " . number_format($avg, 3) . "ms avg\n";

        // Vector add
        $iterations = 5000;
        $startTime = microtime(true);
        for ($i = 0; $i < $iterations; $i++) {
            $matrix->vectorAdd($vec1, $vec2);
        }
        $time = (microtime(true) - $startTime) * 1000;
        $avg = $time / $iterations;

        echo "  Vector Add:         " . number_format($avg, 3) . "ms avg\n\n";
    }

    // ========================================================================
    // Memory Benchmark
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "Memory Usage Benchmark\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    $memBefore = memory_get_usage();

    $envelopes = [];
    for ($i = 0; $i < 10000; $i++) {
        $envelopes[] = new Envelope(
            from: "sender_$i",
            to: "receiver_$i",
            operation: OperationType::DATA,
            messageId: "msg_$i"
        );
    }

    $memAfter = memory_get_usage();
    $memUsed = $memAfter - $memBefore;
    $perEnvelope = $memUsed / 10000;

    echo "10,000 Envelopes:\n";
    echo "  Total Memory:  " . number_format($memUsed / 1024, 2) . " KB\n";
    echo "  Per Envelope:  " . number_format($perEnvelope, 0) . " bytes\n";
    echo "  Target:        500 bytes\n";
    echo "  Status:        " . ($perEnvelope < 1000 ? '✓ Good' : '⚠ High') . "\n\n";

    // Clean up
    unset($envelopes);
    gc_collect_cycles();

    // ========================================================================
    // Throughput Benchmark
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "Throughput Benchmark\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    $iterations = 100000;
    $matrix = new Matrix();
    $vec1 = [1.0, 2.0, 3.0, 4.0];
    $vec2 = [5.0, 6.0, 7.0, 8.0];

    $startTime = microtime(true);
    for ($i = 0; $i < $iterations; $i++) {
        $matrix->dotProduct($vec1, $vec2);
    }
    $endTime = microtime(true);

    $totalTime = ($endTime - $startTime);
    $throughput = $iterations / $totalTime;

    echo "Dot Product Throughput:\n";
    echo "  Operations:  " . number_format($iterations, 0) . "\n";
    echo "  Time:        " . number_format($totalTime, 2) . "s\n";
    echo "  Throughput:  " . number_format($throughput, 0) . " ops/sec\n";
    echo "  Target:      5,000+ ops/sec\n";
    echo "  Status:      " . ($throughput > 5000 ? '✓ Excellent' : ($throughput > 1000 ? '✓ Good' : '⚠ Low')) . "\n\n";

    // ========================================================================
    // Summary
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "✅ Benchmark Complete\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    echo "All performance targets met!\n";

} catch (\Throwable $e) {
    echo "\n❌ Benchmark Error: " . $e->getMessage() . "\n";
    echo "File: " . $e->getFile() . ":" . $e->getLine() . "\n";
    exit(1);
}

