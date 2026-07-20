<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\FFI;

use PHPUnit\Framework\TestCase;
use UMICP\FFI\TypeConverter;
use FFI;
use InvalidArgumentException;
use RuntimeException;

/**
 * @covers \UMICP\FFI\TypeConverter
 */
class TypeConverterTest extends TestCase
{
    public function testPhpArrayToCFloatArray(): void
    {
        $phpArray = [1.0, 2.0, 3.0];
        $cArray = TypeConverter::phpArrayToCFloatArray($phpArray);

        $this->assertInstanceOf(FFI\CData::class, $cArray);
        $this->assertEquals(1.0, $cArray[0]);
        $this->assertEquals(2.0, $cArray[1]);
        $this->assertEquals(3.0, $cArray[2]);
    }

    public function testPhpArrayToCFloatArrayWithEmptyArray(): void
    {
        $this->expectException(InvalidArgumentException::class);
        $this->expectExceptionMessage('Array cannot be empty');

        TypeConverter::phpArrayToCFloatArray([]);
    }

    public function testCFloatArrayToPhpArray(): void
    {
        $phpArray = [1.5, 2.5, 3.5];
        $cArray = TypeConverter::phpArrayToCFloatArray($phpArray);
        $result = TypeConverter::cFloatArrayToPhpArray($cArray, 3);

        $this->assertEquals($phpArray, $result);
    }

    public function testCFloatArrayToPhpArrayWithInvalidSize(): void
    {
        $this->expectException(InvalidArgumentException::class);
        $this->expectExceptionMessage('Size must be greater than 0');

        $cArray = FFI::new("float[3]");
        TypeConverter::cFloatArrayToPhpArray($cArray, 0);
    }

    public function testPhpArrayToJson(): void
    {
        $array = ['key1' => 'value1', 'key2' => 'value2'];
        $json = TypeConverter::phpArrayToJson($array);

        $this->assertJson($json);
        $decoded = json_decode($json, true);
        $this->assertEquals($array, $decoded);
    }

    public function testJsonToPhpArray(): void
    {
        $json = '{"key1":"value1","key2":"value2"}';
        $array = TypeConverter::jsonToPhpArray($json);

        $this->assertIsArray($array);
        $this->assertEquals('value1', $array['key1']);
        $this->assertEquals('value2', $array['key2']);
    }

    public function testJsonToPhpArrayWithInvalidJson(): void
    {
        $this->expectException(RuntimeException::class);
        $this->expectExceptionMessage('JSON decoding failed');

        TypeConverter::jsonToPhpArray('invalid json {');
    }

    public function testPhpArrayToCIntArray(): void
    {
        $phpArray = [1, 2, 3];
        $cArray = TypeConverter::phpArrayToCIntArray($phpArray);

        $this->assertInstanceOf(FFI\CData::class, $cArray);
        $this->assertEquals(1, $cArray[0]);
        $this->assertEquals(2, $cArray[1]);
        $this->assertEquals(3, $cArray[2]);
    }

    public function testCIntArrayToPhpArray(): void
    {
        $phpArray = [10, 20, 30];
        $cArray = TypeConverter::phpArrayToCIntArray($phpArray);
        $result = TypeConverter::cIntArrayToPhpArray($cArray, 3);

        $this->assertEquals($phpArray, $result);
    }
}

