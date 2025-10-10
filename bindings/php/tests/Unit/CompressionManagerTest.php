<?php

namespace UMICP\Tests\Unit;

use PHPUnit\Framework\TestCase;
use UMICP\Core\CompressionManager;

class CompressionManagerTest extends TestCase
{
    public function testConstructorWithDefaults(): void
    {
        $compressor = new CompressionManager();
        $this->assertInstanceOf(CompressionManager::class, $compressor);
        $this->assertEquals('gzip', $compressor->getAlgorithm());
        $this->assertEquals(6, $compressor->getLevel());
    }

    public function testConstructorWithOptions(): void
    {
        $compressor = new CompressionManager(CompressionManager::ALGORITHM_DEFLATE, 9);
        $this->assertEquals('deflate', $compressor->getAlgorithm());
        $this->assertEquals(9, $compressor->getLevel());
    }

    public function testGzipCompressionDecompression(): void
    {
        $compressor = new CompressionManager(CompressionManager::ALGORITHM_GZIP);
        $original = 'Hello, UMICP! This is test data that should compress well. ' . str_repeat('A', 1000);

        $compressed = $compressor->compress($original);
        $this->assertNotEquals($original, $compressed);
        $this->assertLessThan(strlen($original), strlen($compressed));

        $decompressed = $compressor->decompress($compressed);
        $this->assertEquals($original, $decompressed);
    }

    public function testDeflateCompressionDecompression(): void
    {
        $compressor = new CompressionManager(CompressionManager::ALGORITHM_DEFLATE);
        $original = 'Test data ' . str_repeat('B', 500);

        $compressed = $compressor->compress($original);
        $decompressed = $compressor->decompress($compressed);

        $this->assertEquals($original, $decompressed);
    }

    public function testNoneAlgorithm(): void
    {
        $compressor = new CompressionManager(CompressionManager::ALGORITHM_NONE);
        $original = 'No compression';

        $compressed = $compressor->compress($original);
        $this->assertEquals($original, $compressed);

        $decompressed = $compressor->decompress($compressed);
        $this->assertEquals($original, $decompressed);
    }

    public function testEmptyData(): void
    {
        $compressor = new CompressionManager();

        $compressed = $compressor->compress('');
        $this->assertEquals('', $compressed);

        $decompressed = $compressor->decompress('');
        $this->assertEquals('', $decompressed);
    }

    public function testCompressionRatio(): void
    {
        $original = str_repeat('A', 1000);
        $compressed = gzencode($original);

        $ratio = CompressionManager::getCompressionRatio($original, $compressed);
        $this->assertLessThan(1.0, $ratio); // Should be compressed
        $this->assertGreaterThan(0.0, $ratio);
    }
}

