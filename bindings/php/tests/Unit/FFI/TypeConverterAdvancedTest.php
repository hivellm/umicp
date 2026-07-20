<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\FFI;

use PHPUnit\Framework\TestCase;
use UMICP\FFI\TypeConverter;
use InvalidArgumentException;
use RuntimeException;

/**
 * Advanced type converter tests
 *
 * @covers \UMICP\FFI\TypeConverter
 */
class TypeConverterAdvancedTest extends TestCase
{
    public function testLargeArrayConversion(): void
    {
        $largeArray = array_fill(0, 10000, 1.5);

        $cArray = TypeConverter::phpArrayToCFloatArray($largeArray);
        $phpArray = TypeConverter::cFloatArrayToPhpArray($cArray, 10000);

        $this->assertCount(10000, $phpArray);
        $this->assertEquals($largeArray[0], $phpArray[0]);
        $this->assertEquals($largeArray[9999], $phpArray[9999]);
    }

    public function testNegativeNumbers(): void
    {
        $array = [-1.5, -2.5, -3.5];

        $cArray = TypeConverter::phpArrayToCFloatArray($array);
        $result = TypeConverter::cFloatArrayToPhpArray($cArray, 3);

        $this->assertEquals($array, $result);
    }

    public function testMixedNumberTypes(): void
    {
        $array = [1, 2.5, 3, 4.7]; // Mix of int and float

        $cArray = TypeConverter::phpArrayToCFloatArray($array);
        $result = TypeConverter::cFloatArrayToPhpArray($cArray, 4);

        $this->assertEqualsWithDelta(1.0, $result[0], 0.0001);
        $this->assertEqualsWithDelta(2.5, $result[1], 0.0001);
        $this->assertEqualsWithDelta(3.0, $result[2], 0.0001);
        $this->assertEqualsWithDelta(4.7, $result[3], 0.0001);
    }

    public function testJsonWithNestedStructures(): void
    {
        $complex = [
            'simple' => 'value',
            'nested' => [
                'inner' => 'data',
                'number' => 42
            ],
            'array' => [1, 2, 3]
        ];

        $json = TypeConverter::phpArrayToJson($complex);
        $decoded = TypeConverter::jsonToPhpArray($json);

        $this->assertEquals($complex, $decoded);
    }

    public function testJsonWithSpecialCharacters(): void
    {
        $data = [
            'quotes' => 'He said "hello"',
            'backslash' => 'path\\to\\file',
            'unicode' => '世界',
            'newline' => "Line1\nLine2"
        ];

        $json = TypeConverter::phpArrayToJson($data);
        $decoded = TypeConverter::jsonToPhpArray($json);

        $this->assertEquals($data, $decoded);
    }

    public function testEmptyJsonArray(): void
    {
        $empty = [];
        $json = TypeConverter::phpArrayToJson($empty);
        $decoded = TypeConverter::jsonToPhpArray($json);

        $this->assertEquals($empty, $decoded);
    }

    public function testIntArrayConversion(): void
    {
        $phpArray = [10, 20, 30, 40, 50];

        $cArray = TypeConverter::phpArrayToCIntArray($phpArray);
        $result = TypeConverter::cIntArrayToPhpArray($cArray, 5);

        $this->assertEquals($phpArray, $result);
    }

    public function testDoubleArrayConversion(): void
    {
        $phpArray = [1.1, 2.2, 3.3, 4.4];

        $cArray = TypeConverter::phpArrayToCDoubleArray($phpArray);
        $result = TypeConverter::cDoubleArrayToPhpArray($cArray, 4);

        foreach ($phpArray as $i => $value) {
            $this->assertEqualsWithDelta($value, $result[$i], 0.0001);
        }
    }

    public function testJsonWithNullValues(): void
    {
        $data = ['key' => null, 'another' => 'value'];

        $json = TypeConverter::phpArrayToJson($data);
        $decoded = TypeConverter::jsonToPhpArray($json);

        $this->assertArrayHasKey('key', $decoded);
        $this->assertNull($decoded['key']);
        $this->assertEquals('value', $decoded['another']);
    }
}

