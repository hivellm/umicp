<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\FFI;

use PHPUnit\Framework\TestCase;
use UMICP\FFI\Config;

/**
 * @covers \UMICP\FFI\Config
 */
class ConfigTest extends TestCase
{
    protected function tearDown(): void
    {
        Config::clear();
    }

    public function testGetWithDotNotation(): void
    {
        // Assumes config file exists
        try {
            $libPath = Config::get('ffi.lib_path');
            $this->assertIsString($libPath);
        } catch (\RuntimeException $e) {
            $this->markTestSkipped('Config file not found: ' . $e->getMessage());
        }
    }

    public function testGetWithDefault(): void
    {
        try {
            Config::load();
            $value = Config::get('nonexistent.key', 'default_value');
            $this->assertEquals('default_value', $value);
        } catch (\RuntimeException $e) {
            $this->markTestSkipped('Config file not found');
        }
    }

    public function testHas(): void
    {
        try {
            Config::load();
            $this->assertTrue(Config::has('ffi'));
            $this->assertFalse(Config::has('nonexistent.key'));
        } catch (\RuntimeException $e) {
            $this->markTestSkipped('Config file not found');
        }
    }

    public function testSetAndGet(): void
    {
        try {
            Config::load();
            Config::set('test.key', 'test_value');

            $this->assertEquals('test_value', Config::get('test.key'));
        } catch (\RuntimeException $e) {
            $this->markTestSkipped('Config file not found');
        }
    }

    public function testAll(): void
    {
        try {
            $all = Config::all();

            $this->assertIsArray($all);
            $this->assertArrayHasKey('ffi', $all);
        } catch (\RuntimeException $e) {
            $this->markTestSkipped('Config file not found');
        }
    }
}

