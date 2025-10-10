<?php

declare(strict_types=1);

namespace UMICP\Tests\Performance;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Envelope;
use UMICP\Core\Matrix;
use UMICP\Core\OperationType;

/**
 * Performance benchmark tests
 *
 * @group performance
 * @group slow
 */
class BenchmarkTest extends TestCase
{
    public function testEnvelopeCreationPerformance(): void
    {
        $iterations = 1000;
        $startTime = microtime(true);

        for ($i = 0; $i < $iterations; $i++) {
            $envelope = new Envelope(
                from: "sender_$i",
                to: "receiver_$i",
                operation: OperationType::DATA,
                messageId: "msg_$i"
            );
        }

        $endTime = microtime(true);
        $avgTime = (($endTime - $startTime) / $iterations) * 1000;

        echo "\nEnvelope Creation: " . number_format($avgTime, 3) . "ms average\n";

        // Should be < 5ms per envelope
        $this->assertLessThan(5, $avgTime);
    }

    public function testEnvelopeSerializationPerformance(): void
    {
        $iterations = 1000;
        $envelopes = [];

        // Create envelopes
        for ($i = 0; $i < $iterations; $i++) {
            $envelopes[] = new Envelope(
                from: "sender_$i",
                to: "receiver_$i",
                operation: OperationType::DATA,
                messageId: "msg_$i",
                capabilities: [
                    'sequence' => (string) $i,
                    'batch' => 'true'
                ]
            );
        }

        // Benchmark serialization
        $startTime = microtime(true);
        foreach ($envelopes as $envelope) {
            $envelope->serialize();
        }
        $endTime = microtime(true);

        $avgTime = (($endTime - $startTime) / $iterations) * 1000;
        echo "\nSerialization: " . number_format($avgTime, 3) . "ms average\n";

        // Should be < 20ms per envelope
        $this->assertLessThan(20, $avgTime);
    }

    public function testMatrixDotProductPerformance(): void
    {
        $matrix = new Matrix();
        $iterations = 10000;

        $vec1 = array_fill(0, 100, 1.0);
        $vec2 = array_fill(0, 100, 2.0);

        $startTime = microtime(true);
        for ($i = 0; $i < $iterations; $i++) {
            $matrix->dotProduct($vec1, $vec2);
        }
        $endTime = microtime(true);

        $avgTime = (($endTime - $startTime) / $iterations) * 1000;
        $throughput = 1000 / $avgTime;

        echo "\nDot Product (100-dim): " . number_format($avgTime, 3) . "ms average\n";
        echo "Throughput: " . number_format($throughput, 0) . " ops/sec\n";

        // Should be < 2ms per operation
        $this->assertLessThan(2, $avgTime);
    }

    public function testMatrixCosineSimilarityPerformance(): void
    {
        $matrix = new Matrix();
        $iterations = 5000;

        $vec1 = array_fill(0, 128, 0.5);
        $vec2 = array_fill(0, 128, 0.6);

        $startTime = microtime(true);
        for ($i = 0; $i < $iterations; $i++) {
            $matrix->cosineSimilarity($vec1, $vec2);
        }
        $endTime = microtime(true);

        $avgTime = (($endTime - $startTime) / $iterations) * 1000;

        echo "\nCosine Similarity (128-dim): " . number_format($avgTime, 3) . "ms average\n";

        // Should be < 3ms per operation
        $this->assertLessThan(3, $avgTime);
    }

    public function testEnvelopeValidationPerformance(): void
    {
        $iterations = 5000;
        $envelope = new Envelope(
            from: 'sender',
            to: 'receiver',
            operation: OperationType::DATA,
            messageId: 'msg-123'
        );

        $startTime = microtime(true);
        for ($i = 0; $i < $iterations; $i++) {
            $envelope->validate();
        }
        $endTime = microtime(true);

        $avgTime = (($endTime - $startTime) / $iterations) * 1000;

        echo "\nValidation: " . number_format($avgTime, 3) . "ms average\n";

        // Should be very fast (< 0.1ms)
        $this->assertLessThan(0.1, $avgTime);
    }

    public function testMemoryUsage(): void
    {
        $memBefore = memory_get_usage();

        $envelopes = [];
        for ($i = 0; $i < 1000; $i++) {
            $envelopes[] = new Envelope(
                from: "sender_$i",
                to: "receiver_$i",
                operation: OperationType::DATA
            );
        }

        $memAfter = memory_get_usage();
        $memUsed = $memAfter - $memBefore;
        $perEnvelope = $memUsed / 1000;

        echo "\nMemory per Envelope: " . number_format($perEnvelope, 0) . " bytes\n";

        // Should be < 2KB per envelope (target: 500 bytes)
        $this->assertLessThan(2048, $perEnvelope);

        // Clean up
        unset($envelopes);
        gc_collect_cycles();
    }
}

