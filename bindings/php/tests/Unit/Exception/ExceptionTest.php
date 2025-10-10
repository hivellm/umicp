<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Exception;

use PHPUnit\Framework\TestCase;
use UMICP\Exception\UMICPException;
use UMICP\Exception\FFIException;
use UMICP\Exception\TransportException;
use UMICP\Exception\SerializationException;
use UMICP\Exception\ValidationException;

/**
 * @covers \UMICP\Exception\UMICPException
 * @covers \UMICP\Exception\FFIException
 */
class ExceptionTest extends TestCase
{
    public function testUMICPExceptionWithContext(): void
    {
        $context = ['key' => 'value', 'number' => 42];
        $exception = new UMICPException(
            'Test exception',
            100,
            null,
            $context
        );

        $this->assertEquals('Test exception', $exception->getMessage());
        $this->assertEquals(100, $exception->getCode());
        $this->assertEquals($context, $exception->getContext());
        $this->assertTrue($exception->hasContext());
        $this->assertEquals('value', $exception->getContextValue('key'));
    }

    public function testUMICPExceptionWithoutContext(): void
    {
        $exception = new UMICPException('Simple exception');

        $this->assertFalse($exception->hasContext());
        $this->assertNull($exception->getContext());
        $this->assertNull($exception->getContextValue('nonexistent'));
        $this->assertEquals('default', $exception->getContextValue('nonexistent', 'default'));
    }

    public function testUMICPExceptionToArray(): void
    {
        $exception = new UMICPException(
            'Test',
            100,
            null,
            ['key' => 'value']
        );

        $array = $exception->toArray();

        $this->assertIsArray($array);
        $this->assertArrayHasKey('message', $array);
        $this->assertArrayHasKey('code', $array);
        $this->assertArrayHasKey('file', $array);
        $this->assertArrayHasKey('line', $array);
        $this->assertArrayHasKey('context', $array);
    }

    public function testFFIExceptionWithLibraryPath(): void
    {
        $exception = new FFIException(
            'FFI error',
            0,
            null,
            'C error message',
            '/path/to/lib.so'
        );

        $this->assertEquals('FFI error', $exception->getMessage());
        $this->assertEquals('C error message', $exception->getFFIError());
        $this->assertEquals('/path/to/lib.so', $exception->getLibraryPath());
        $this->assertTrue($exception->hasFFIError());
    }

    public function testFFIExceptionToArray(): void
    {
        $exception = new FFIException(
            'Test',
            0,
            null,
            'FFI error',
            '/lib.so'
        );

        $array = $exception->toArray();

        $this->assertArrayHasKey('ffi_error', $array);
        $this->assertArrayHasKey('library_path', $array);
        $this->assertEquals('FFI error', $array['ffi_error']);
        $this->assertEquals('/lib.so', $array['library_path']);
    }

    public function testExceptionHierarchy(): void
    {
        $transport = new TransportException('Transport error');
        $this->assertInstanceOf(UMICPException::class, $transport);

        $serialization = new SerializationException('Serialization error');
        $this->assertInstanceOf(UMICPException::class, $serialization);

        $validation = new ValidationException('Validation error');
        $this->assertInstanceOf(UMICPException::class, $validation);

        $ffi = new FFIException('FFI error');
        $this->assertInstanceOf(UMICPException::class, $ffi);
    }
}

