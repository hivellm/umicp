<?php

declare(strict_types=1);

namespace UMICP\Tests\Performance;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Envelope;
use UMICP\Core\Matrix;
use UMICP\Core\Frame;
use UMICP\Core\OperationType;

/**
 * Memory leak detection tests
 *
 * @group performance
 * @group memory
 */
class MemoryLeakTest extends TestCase
{
    public function testEnvelopeMemoryLeak(): void
    {
        gc_collect_cycles();
        $memBefore = memory_get_usage();

        // Create and destroy many envelopes
        for ($round = 0; $round < 10; $round++) {
            $envelopes = [];

            for ($i = 0; $i < 1000; $i++) {
                $envelopes[] = new Envelope(
                    from: "sender_$i",
                    to: "receiver_$i",
                    operation: OperationType::DATA,
                    messageId: "msg_$i"
                );
            }

            // Serialize all
            foreach ($envelopes as $envelope) {
                $envelope->serialize();
            }

            // Destroy
            unset($envelopes);
            gc_collect_cycles();
        }

        $memAfter = memory_get_usage();
        $memGrowth = $memAfter - $memBefore;

        echo "\nEnvelope Memory Growth: " . number_format($memGrowth / 1024, 2) . " KB\n";

        // Memory growth should be minimal (< 1MB)
        $this->assertLessThan(1024 * 1024, $memGrowth, 'Memory leak detected in Envelope');
    }

    public function testMatrixMemoryLeak(): void
    {
        gc_collect_cycles();
        $memBefore = memory_get_usage();

        // Create and use many matrix instances
        for ($round = 0; $round < 10; $round++) {
            $matrices = [];

            for ($i = 0; $i < 100; $i++) {
                $matrices[] = new Matrix();
            }

            // Perform operations
            $vec1 = array_fill(0, 100, 1.0);
            $vec2 = array_fill(0, 100, 2.0);

            foreach ($matrices as $matrix) {
                $matrix->dotProduct($vec1, $vec2);
                $matrix->cosineSimilarity($vec1, $vec2);
            }

            // Destroy
            unset($matrices);
            gc_collect_cycles();
        }

        $memAfter = memory_get_usage();
        $memGrowth = $memAfter - $memBefore;

        echo "\nMatrix Memory Growth: " . number_format($memGrowth / 1024, 2) . " KB\n";

        // Memory growth should be minimal (< 512KB)
        $this->assertLessThan(512 * 1024, $memGrowth, 'Memory leak detected in Matrix');
    }

    public function testFrameMemoryLeak(): void
    {
        gc_collect_cycles();
        $memBefore = memory_get_usage();

        for ($round = 0; $round < 10; $round++) {
            $frames = [];

            for ($i = 0; $i < 1000; $i++) {
                $frames[] = new Frame(
                    type: $i % 4,
                    streamId: $i,
                    sequence: $i * 2
                );
            }

            unset($frames);
            gc_collect_cycles();
        }

        $memAfter = memory_get_usage();
        $memGrowth = $memAfter - $memBefore;

        echo "\nFrame Memory Growth: " . number_format($memGrowth / 1024, 2) . " KB\n";

        $this->assertLessThan(512 * 1024, $memGrowth, 'Memory leak detected in Frame');
    }

    public function testMixedObjectsMemoryLeak(): void
    {
        gc_collect_cycles();
        $memBefore = memory_get_usage();

        for ($round = 0; $round < 5; $round++) {
            $objects = [];

            for ($i = 0; $i < 100; $i++) {
                $objects[] = new Envelope(from: "s$i", to: "r$i");
                $objects[] = new Matrix();
                $objects[] = new Frame(type: $i % 4);
            }

            // Use objects
            foreach ($objects as $obj) {
                if ($obj instanceof Envelope) {
                    $obj->validate();
                } elseif ($obj instanceof Matrix) {
                    $obj->dotProduct([1,2], [3,4]);
                }
            }

            unset($objects);
            gc_collect_cycles();
        }

        $memAfter = memory_get_usage();
        $memGrowth = $memAfter - $memBefore;

        echo "\nMixed Objects Memory Growth: " . number_format($memGrowth / 1024, 2) . " KB\n";

        $this->assertLessThan(1024 * 1024, $memGrowth, 'Memory leak detected');
    }
}

