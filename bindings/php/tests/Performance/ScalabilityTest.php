<?php

declare(strict_types=1);

namespace UMICP\Tests\Performance;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Envelope;
use UMICP\Core\Matrix;
use UMICP\Core\OperationType;

/**
 * Scalability and stress tests
 *
 * @group performance
 * @group stress
 * @group slow
 */
class ScalabilityTest extends TestCase
{
    public function testMassiveEnvelopeCreation(): void
    {
        $count = 100000;
        $envelopes = [];

        $startTime = microtime(true);
        $memBefore = memory_get_usage();

        for ($i = 0; $i < $count; $i++) {
            $envelopes[] = new Envelope(
                from: "sender_$i",
                to: "receiver_$i",
                operation: OperationType::DATA
            );
        }

        $duration = microtime(true) - $startTime;
        $memAfter = memory_get_usage();

        $throughput = $count / $duration;
        $avgMemory = ($memAfter - $memBefore) / $count;

        echo "\n";
        echo "Massive Envelope Creation:\n";
        echo "  Count:      " . number_format($count) . "\n";
        echo "  Duration:   " . number_format($duration, 2) . "s\n";
        echo "  Throughput: " . number_format($throughput, 0) . " ops/sec\n";
        echo "  Avg Memory: " . number_format($avgMemory, 0) . " bytes\n";

        $this->assertGreaterThan(1000, $throughput);
        $this->assertLessThan(2048, $avgMemory);

        unset($envelopes);
        gc_collect_cycles();
    }

    public function testConcurrentMatrixOperations(): void
    {
        $matrix = new Matrix();
        $operations = 50000;

        $vec1 = array_fill(0, 256, 0.5);
        $vec2 = array_fill(0, 256, 0.6);

        $results = [
            'dotProduct' => 0,
            'cosineSimilarity' => 0,
            'vectorAdd' => 0
        ];

        $startTime = microtime(true);

        for ($i = 0; $i < $operations; $i++) {
            $op = $i % 3;

            if ($op === 0) {
                $matrix->dotProduct($vec1, $vec2);
                $results['dotProduct']++;
            } elseif ($op === 1) {
                $matrix->cosineSimilarity($vec1, $vec2);
                $results['cosineSimilarity']++;
            } else {
                $matrix->vectorAdd($vec1, $vec2);
                $results['vectorAdd']++;
            }
        }

        $duration = microtime(true) - $startTime;
        $throughput = $operations / $duration;

        echo "\n";
        echo "Concurrent Matrix Operations:\n";
        echo "  Total Ops:  " . number_format($operations) . "\n";
        echo "  Duration:   " . number_format($duration, 2) . "s\n";
        echo "  Throughput: " . number_format($throughput, 0) . " ops/sec\n";

        $this->assertGreaterThan(5000, $throughput);
    }

    public function testSerializationUnderLoad(): void
    {
        $count = 50000;
        $envelopes = [];

        // Create envelopes
        for ($i = 0; $i < $count; $i++) {
            $envelopes[] = new Envelope(
                from: "sender_$i",
                to: "receiver_$i",
                operation: OperationType::DATA,
                capabilities: [
                    'index' => (string) $i,
                    'timestamp' => (string) time()
                ]
            );
        }

        // Serialize all
        $startTime = microtime(true);
        $serialized = array_map(fn($e) => $e->serialize(), $envelopes);
        $serializeDuration = microtime(true) - $startTime;

        // Deserialize all
        $startTime = microtime(true);
        $deserialized = array_map(fn($j) => Envelope::deserialize($j), $serialized);
        $deserializeDuration = microtime(true) - $startTime;

        $serializeThroughput = $count / $serializeDuration;
        $deserializeThroughput = $count / $deserializeDuration;

        echo "\n";
        echo "Serialization Under Load:\n";
        echo "  Count:                " . number_format($count) . "\n";
        echo "  Serialize Duration:   " . number_format($serializeDuration, 2) . "s\n";
        echo "  Deserialize Duration: " . number_format($deserializeDuration, 2) . "s\n";
        echo "  Serialize Rate:       " . number_format($serializeThroughput, 0) . " ops/sec\n";
        echo "  Deserialize Rate:     " . number_format($deserializeThroughput, 0) . " ops/sec\n";

        $this->assertGreaterThan(500, $serializeThroughput);
        $this->assertGreaterThan(500, $deserializeThroughput);
        $this->assertCount($count, $deserialized);
    }

    public function testMatrixWithVariousDimensions(): void
    {
        $matrix = new Matrix();
        $dimensions = [10, 50, 100, 500, 1000];
        $results = [];

        foreach ($dimensions as $dim) {
            $vec1 = array_fill(0, $dim, 1.0);
            $vec2 = array_fill(0, $dim, 2.0);

            $iterations = max(100, 10000 / $dim);

            $startTime = microtime(true);
            for ($i = 0; $i < $iterations; $i++) {
                $matrix->dotProduct($vec1, $vec2);
            }
            $duration = microtime(true) - $startTime;

            $avgTime = ($duration / $iterations) * 1000;
            $results[$dim] = $avgTime;
        }

        echo "\n";
        echo "Matrix Operations by Dimension:\n";
        foreach ($results as $dim => $time) {
            echo sprintf("  %4d-dim: %6.3fms\n", $dim, $time);
        }

        // All should be reasonably fast
        foreach ($results as $time) {
            $this->assertLessThan(10, $time);
        }
    }
}

