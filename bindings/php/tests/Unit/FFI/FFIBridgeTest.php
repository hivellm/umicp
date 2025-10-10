<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\FFI;

use PHPUnit\Framework\TestCase;
use UMICP\FFI\FFIBridge;
use UMICP\Exception\FFIException;

/**
 * @covers \UMICP\FFI\FFIBridge
 */
class FFIBridgeTest extends TestCase
{
    protected function tearDown(): void
    {
        FFIBridge::reset();
    }

    public function testGetInstance(): void
    {
        try {
            $ffi = FFIBridge::getInstance();

            $this->assertInstanceOf(FFIBridge::class, $ffi);
            $this->assertTrue($ffi->isInitialized());

        } catch (FFIException $e) {
            $this->markTestSkipped('FFI not available: ' . $e->getMessage());
        }
    }

    public function testSingletonPattern(): void
    {
        try {
            $ffi1 = FFIBridge::getInstance();
            $ffi2 = FFIBridge::getInstance();

            $this->assertSame($ffi1, $ffi2);

        } catch (FFIException $e) {
            $this->markTestSkipped('FFI not available');
        }
    }

    public function testGetInfo(): void
    {
        try {
            $ffi = FFIBridge::getInstance();
            $info = $ffi->getInfo();

            $this->assertIsArray($info);
            $this->assertArrayHasKey('lib_path', $info);
            $this->assertArrayHasKey('header_path', $info);
            $this->assertArrayHasKey('ffi_version', $info);
            $this->assertArrayHasKey('php_version', $info);
            $this->assertArrayHasKey('initialized', $info);

        } catch (FFIException $e) {
            $this->markTestSkipped('FFI not available');
        }
    }

    public function testCreateEnvelope(): void
    {
        try {
            $ffi = FFIBridge::getInstance();
            $envelope = $ffi->createEnvelope();

            $this->assertInstanceOf(\FFI\CData::class, $envelope);

            $ffi->destroyEnvelope($envelope);

        } catch (FFIException $e) {
            $this->markTestSkipped('C++ library not available');
        }
    }

    public function testCreateMatrix(): void
    {
        try {
            $ffi = FFIBridge::getInstance();
            $matrix = $ffi->createMatrix();

            $this->assertInstanceOf(\FFI\CData::class, $matrix);

            $ffi->destroyMatrix($matrix);

        } catch (FFIException $e) {
            $this->markTestSkipped('C++ library not available');
        }
    }

    public function testCreateFrame(): void
    {
        try {
            $ffi = FFIBridge::getInstance();
            $frame = $ffi->createFrame();

            $this->assertInstanceOf(\FFI\CData::class, $frame);

            $ffi->destroyFrame($frame);

        } catch (FFIException $e) {
            $this->markTestSkipped('C++ library not available');
        }
    }
}

