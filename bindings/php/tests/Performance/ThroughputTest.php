<?php

declare(strict_types=1);

namespace UMICP\Tests\Performance;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Envelope;
use UMICP\Core\Matrix;
use UMICP\Core\OperationType;

/**
 * Throughput performance tests
 *
 * @group performance
 * @group slow
 */
class ThroughputTest extends TestCase
{
    public function testEnvelopeCreationThroughput(): void
    {
        $iterations = 10000;
        $startTime = microtime(true);

        for ($i = 0; $i < $iterations; $i++) {
            new Envelope(
                from: "sender_$i",
                to: "receiver_$i",
                operation: OperationType::DATA,
                messageId: "msg_$i"
            );
        }

        $duration = microtime(true) - $startTime;
        $throughput = $iterations / $duration;

        echo "\nEnvelope Creation Throughput: " . number_format($throughput, 0) . " ops/sec\n";

        // Target: > 1,000 ops/sec
        $this->assertGreaterThan(1000, $throughput);
    }

    public function testSerializationThroughput(): void
    {
        $iterations = 5000;
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
        foreach ($envelopes as $envelope) {
            $envelope->serialize();
        }
        $duration = microtime(true) - $startTime;
        $throughput = $iterations / $duration;

        echo "\nSerialization Throughput: " . number_format($throughput, 0) . " ops/sec\n";

        // Target: > 500 ops/sec
        $this->assertGreaterThan(500, $throughput);
    }

    public function testDotProductThroughput(): void
    {
        $matrix = new Matrix();
        $iterations = 100000;

        $vec1 = [1.0, 2.0, 3.0, 4.0];
        $vec2 = [5.0, 6.0, 7.0, 8.0];

        $startTime = microtime(true);
        for ($i = 0; $i < $iterations; $i++) {
            $matrix->dotProduct($vec1, $vec2);
        }
        $duration = microtime(true) - $startTime;
        $throughput = $iterations / $duration;

        echo "\nDot Product Throughput: " . number_format($throughput, 0) . " ops/sec\n";

        // Target: > 10,000 ops/sec
        $this->assertGreaterThan(10000, $throughput);
    }

    public function testCosineSimilarityThroughput(): void
    {
        $matrix = new Matrix();
        $iterations = 50000;

        $vec1 = array_fill(0, 128, 0.5);
        $vec2 = array_fill(0, 128, 0.6);

        $startTime = microtime(true);
        for ($i = 0; $i < $iterations; $i++) {
            $matrix->cosineSimilarity($vec1, $vec2);
        }
        $duration = microtime(true) - $startTime;
        $throughput = $iterations / $duration;

        echo "\nCosine Similarity Throughput (128-dim): " . number_format($throughput, 0) . " ops/sec\n";

        // Target: > 5,000 ops/sec
        $this->assertGreaterThan(5000, $throughput);
    }

    public function testVectorAddThroughput(): void
    {
        $matrix = new Matrix();
        $iterations = 50000;

        $vec1 = array_fill(0, 100, 1.0);
        $vec2 = array_fill(0, 100, 2.0);

        $startTime = microtime(true);
        for ($i = 0; $i < $iterations; $i++) {
            $matrix->vectorAdd($vec1, $vec2);
        }
        $duration = microtime(true) - $startTime;
        $throughput = $iterations / $duration;

        echo "\nVector Add Throughput: " . number_format($throughput, 0) . " ops/sec\n";

        // Target: > 5,000 ops/sec
        $this->assertGreaterThan(5000, $throughput);
    }
}

