<?php

declare(strict_types=1);

namespace UMICP\Tests\Integration;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Envelope;
use UMICP\Core\Matrix;
use UMICP\Core\Frame;
use UMICP\Core\OperationType;
use UMICP\FFI\FFIBridge;

/**
 * Integration tests for FFI layer
 *
 * @group integration
 */
class FFIIntegrationTest extends TestCase
{
    public function testFFIBridgeInitialization(): void
    {
        $this->expectNotToPerformAssertions();

        try {
            $ffi = FFIBridge::getInstance();
            $this->assertInstanceOf(FFIBridge::class, $ffi);
        } catch (\UMICP\Exception\FFIException $e) {
            $this->markTestSkipped('FFI not available: ' . $e->getMessage());
        }
    }

    public function testEnvelopeViaFFI(): void
    {
        try {
            $envelope = new Envelope(
                from: 'test-sender',
                to: 'test-receiver',
                operation: OperationType::DATA,
                messageId: 'test-001'
            );

            $this->assertEquals('test-sender', $envelope->getFrom());
            $this->assertEquals('test-receiver', $envelope->getTo());
            $this->assertEquals(OperationType::DATA, $envelope->getOperation());

        } catch (\UMICP\Exception\FFIException $e) {
            $this->markTestSkipped('C++ library not available');
        }
    }

    public function testMatrixViaFFI(): void
    {
        try {
            $matrix = new Matrix();

            $vec1 = [1.0, 2.0, 3.0];
            $vec2 = [4.0, 5.0, 6.0];

            $result = $matrix->dotProduct($vec1, $vec2);

            // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
            $this->assertEquals(32.0, $result);

        } catch (\UMICP\Exception\FFIException $e) {
            $this->markTestSkipped('C++ library not available');
        }
    }

    public function testFrameViaFFI(): void
    {
        try {
            $frame = new Frame(
                type: 1,
                streamId: 100,
                sequence: 42
            );

            $this->assertEquals(1, $frame->getType());
            $this->assertEquals(100, $frame->getStreamId());
            $this->assertEquals(42, $frame->getSequence());

        } catch (\UMICP\Exception\FFIException $e) {
            $this->markTestSkipped('C++ library not available');
        }
    }

    public function testMultipleObjectCreation(): void
    {
        try {
            $objects = [];

            // Create multiple objects to test memory management
            for ($i = 0; $i < 100; $i++) {
                $objects[] = new Envelope(
                    from: "sender-$i",
                    to: "receiver-$i",
                    operation: OperationType::DATA
                );
            }

            $this->assertCount(100, $objects);

            // Objects should clean up automatically
            unset($objects);
            $this->assertTrue(true);

        } catch (\UMICP\Exception\FFIException $e) {
            $this->markTestSkipped('C++ library not available');
        }
    }
}

