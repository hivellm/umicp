<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Core;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Frame;

/**
 * @covers \UMICP\Core\Frame
 */
class FrameTest extends TestCase
{
    public function testFrameCreation(): void
    {
        $frame = new Frame(
            type: 1,
            streamId: 100,
            sequence: 42
        );

        $this->assertEquals(1, $frame->getType());
        $this->assertEquals(100, $frame->getStreamId());
        $this->assertEquals(42, $frame->getSequence());
    }

    public function testFrameWithCompression(): void
    {
        $frame = new Frame(
            type: 1,
            streamId: 100,
            sequence: 1,
            compressed: true
        );

        $this->assertTrue($frame->isCompressed());
        $this->assertFalse($frame->isEncrypted());
    }

    public function testFrameWithEncryption(): void
    {
        $frame = new Frame(
            type: 1,
            streamId: 100,
            sequence: 1,
            encrypted: true
        );

        $this->assertFalse($frame->isCompressed());
        $this->assertTrue($frame->isEncrypted());
    }

    public function testSetCompressed(): void
    {
        $frame = new Frame();

        $this->assertFalse($frame->isCompressed());

        $frame->setCompressed(true);
        $this->assertTrue($frame->isCompressed());

        $frame->setCompressed(false);
        $this->assertFalse($frame->isCompressed());
    }

    public function testSetEncrypted(): void
    {
        $frame = new Frame();

        $this->assertFalse($frame->isEncrypted());

        $frame->setEncrypted(true);
        $this->assertTrue($frame->isEncrypted());

        $frame->setEncrypted(false);
        $this->assertFalse($frame->isEncrypted());
    }

    public function testFluentInterface(): void
    {
        $frame = (new Frame())
            ->setType(2)
            ->setStreamId(200)
            ->setSequence(10)
            ->setCompressed(true);

        $this->assertEquals(2, $frame->getType());
        $this->assertEquals(200, $frame->getStreamId());
        $this->assertEquals(10, $frame->getSequence());
        $this->assertTrue($frame->isCompressed());
    }

    public function testToArray(): void
    {
        $frame = new Frame(
            type: 1,
            streamId: 100,
            sequence: 5,
            compressed: true,
            encrypted: false
        );

        $array = $frame->toArray();

        $this->assertIsArray($array);
        $this->assertEquals(1, $array['type']);
        $this->assertEquals(100, $array['streamId']);
        $this->assertEquals(5, $array['sequence']);
        $this->assertTrue($array['compressed']);
        $this->assertFalse($array['encrypted']);
    }
}

