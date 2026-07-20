<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Core;

use PHPUnit\Framework\TestCase;

/**
 * Comprehensive Capabilities Tests (v0.2.0)
 * Testing native JSON types support in PHP
 */
class CapabilitiesComprehensiveTest extends TestCase
{
    // Integer Tests
    public function testCapabilitiesIntegerPositive(): void
    {
        $capabilities = ['max_tokens' => 100];

        $this->assertIsInt($capabilities['max_tokens']);
        $this->assertEquals(100, $capabilities['max_tokens']);
    }

    public function testCapabilitiesIntegerNegative(): void
    {
        $capabilities = ['offset' => -50];
        $this->assertEquals(-50, $capabilities['offset']);
    }

    public function testCapabilitiesIntegerZero(): void
    {
        $capabilities = ['count' => 0];
        $this->assertEquals(0, $capabilities['count']);
    }

    public function testCapabilitiesIntegerLarge(): void
    {
        $capabilities = ['max_value' => 2147483647];
        $this->assertEquals(2147483647, $capabilities['max_value']);
    }

    // Float Tests
    public function testCapabilitiesFloatDecimal(): void
    {
        $capabilities = ['temperature' => 0.7];

        $this->assertIsFloat($capabilities['temperature']);
        $this->assertEqualsWithDelta(0.7, $capabilities['temperature'], 0.001);
    }

    public function testCapabilitiesFloatScientific(): void
    {
        $capabilities = ['learning_rate' => 1.5e-4];
        $this->assertEqualsWithDelta(1.5e-4, $capabilities['learning_rate'], 1e-10);
    }

    public function testCapabilitiesFloatZero(): void
    {
        $capabilities = ['threshold' => 0.0];
        $this->assertEquals(0.0, $capabilities['threshold']);
    }

    // Boolean Tests
    public function testCapabilitiesBooleanTrue(): void
    {
        $capabilities = ['enabled' => true];

        $this->assertIsBool($capabilities['enabled']);
        $this->assertTrue($capabilities['enabled']);
    }

    public function testCapabilitiesBooleanFalse(): void
    {
        $capabilities = ['disabled' => false];
        $this->assertFalse($capabilities['disabled']);
    }

    // String Tests
    public function testCapabilitiesStringSimple(): void
    {
        $capabilities = ['model' => 'gpt-4'];

        $this->assertIsString($capabilities['model']);
        $this->assertEquals('gpt-4', $capabilities['model']);
    }

    public function testCapabilitiesStringEmpty(): void
    {
        $capabilities = ['empty' => ''];
        $this->assertEquals('', $capabilities['empty']);
    }

    public function testCapabilitiesStringUnicode(): void
    {
        $capabilities = ['message' => 'Hello 世界 🌍'];
        $this->assertEquals('Hello 世界 🌍', $capabilities['message']);
    }

    // Array Tests
    public function testCapabilitiesArrayStrings(): void
    {
        $capabilities = ['models' => ['gpt-4', 'claude-3', 'llama-3']];

        $this->assertIsArray($capabilities['models']);
        $this->assertCount(3, $capabilities['models']);
        $this->assertEquals('gpt-4', $capabilities['models'][0]);
    }

    public function testCapabilitiesArrayIntegers(): void
    {
        $capabilities = ['limits' => [10, 20, 30, 40]];

        $this->assertCount(4, $capabilities['limits']);
        $this->assertEquals(30, $capabilities['limits'][2]);
    }

    public function testCapabilitiesArrayMixed(): void
    {
        $capabilities = ['mixed' => [1, 'two', true, 4.5]];

        $this->assertEquals(1, $capabilities['mixed'][0]);
        $this->assertEquals('two', $capabilities['mixed'][1]);
        $this->assertTrue($capabilities['mixed'][2]);
        $this->assertEqualsWithDelta(4.5, $capabilities['mixed'][3], 0.001);
    }

    public function testCapabilitiesArrayEmpty(): void
    {
        $capabilities = ['empty_array' => []];

        $this->assertIsArray($capabilities['empty_array']);
        $this->assertCount(0, $capabilities['empty_array']);
    }

    public function testCapabilitiesArrayNested(): void
    {
        $capabilities = ['nested' => [[1, 2], [3, 4]]];

        $this->assertEquals(2, $capabilities['nested'][0][1]);
        $this->assertEquals(3, $capabilities['nested'][1][0]);
    }

    // Object Tests
    public function testCapabilitiesObjectSimple(): void
    {
        $capabilities = ['config' => ['timeout' => 30, 'retries' => 3]];

        $this->assertIsArray($capabilities['config']);
        $this->assertEquals(30, $capabilities['config']['timeout']);
        $this->assertEquals(3, $capabilities['config']['retries']);
    }

    public function testCapabilitiesObjectNested(): void
    {
        $capabilities = [
            'auth' => [
                'type' => 'oauth',
                'credentials' => [
                    'client_id' => 'abc123',
                    'scope' => ['read', 'write']
                ]
            ]
        ];

        $this->assertEquals('oauth', $capabilities['auth']['type']);
        $this->assertEquals('abc123', $capabilities['auth']['credentials']['client_id']);
    }

    public function testCapabilitiesObjectEmpty(): void
    {
        $capabilities = ['empty_obj' => []];

        $this->assertIsArray($capabilities['empty_obj']);
        $this->assertCount(0, $capabilities['empty_obj']);
    }

    // Null Tests
    public function testCapabilitiesNullValue(): void
    {
        $capabilities = ['optional' => null];
        $this->assertNull($capabilities['optional']);
    }

    // Complex Mixed Types
    public function testCapabilitiesComplexMixed(): void
    {
        $capabilities = [
            'string_val' => 'test',
            'int_val' => 42,
            'float_val' => 3.14,
            'bool_val' => true,
            'null_val' => null,
            'array_val' => [1, 2, 3],
            'object_val' => ['key' => 'value']
        ];

        $this->assertEquals('test', $capabilities['string_val']);
        $this->assertEquals(42, $capabilities['int_val']);
        $this->assertEqualsWithDelta(3.14, $capabilities['float_val'], 0.001);
        $this->assertTrue($capabilities['bool_val']);
        $this->assertNull($capabilities['null_val']);
        $this->assertCount(3, $capabilities['array_val']);
        $this->assertEquals('value', $capabilities['object_val']['key']);
    }

    // Edge Cases
    public function testCapabilitiesSpecialCharactersInKeys(): void
    {
        $capabilities = [
            'key-with-dash' => 1,
            'key_with_underscore' => 2,
            'key.with.dot' => 3,
            'key:with:colon' => 4
        ];

        $this->assertEquals(1, $capabilities['key-with-dash']);
        $this->assertEquals(2, $capabilities['key_with_underscore']);
        $this->assertEquals(3, $capabilities['key.with.dot']);
        $this->assertEquals(4, $capabilities['key:with:colon']);
    }

    public function testCapabilitiesVeryLargeObject(): void
    {
        $capabilities = [];
        for ($i = 0; $i < 100; $i++) {
            $capabilities["key_$i"] = $i;
        }

        $this->assertCount(100, $capabilities);
        $this->assertEquals(50, $capabilities['key_50']);
        $this->assertEquals(99, $capabilities['key_99']);
    }

    public function testCapabilitiesDeeplyNestedObject(): void
    {
        $capabilities = [
            'deep' => [
                'level1' => [
                    'level2' => [
                        'level3' => [
                            'level4' => [
                                'level5' => 'deep_value'
                            ]
                        ]
                    ]
                ]
            ]
        ];

        $this->assertEquals(
            'deep_value',
            $capabilities['deep']['level1']['level2']['level3']['level4']['level5']
        );
    }

    // Type Preservation
    public function testCapabilitiesTypePreservation(): void
    {
        $capabilities = [
            'int' => 42,
            'float' => 3.14,
            'bool' => true,
            'string' => 'text',
            'array' => [1, 2],
            'object' => ['a' => 1]
        ];

        $this->assertIsInt($capabilities['int']);
        $this->assertIsFloat($capabilities['float']);
        $this->assertIsBool($capabilities['bool']);
        $this->assertIsString($capabilities['string']);
        $this->assertIsArray($capabilities['array']);
        $this->assertIsArray($capabilities['object']);
    }

    // Serialization Tests
    public function testCapabilitiesSerializationInteger(): void
    {
        $caps = ['max_tokens' => 100];
        $json = json_encode($caps);
        $decoded = json_decode($json, true);

        $this->assertEquals(100, $decoded['max_tokens']);
    }

    public function testCapabilitiesSerializationFloat(): void
    {
        $caps = ['temperature' => 0.7];
        $json = json_encode($caps);
        $decoded = json_decode($json, true);

        $this->assertEqualsWithDelta(0.7, $decoded['temperature'], 0.001);
    }

    public function testCapabilitiesSerializationBoolean(): void
    {
        $caps = ['enabled' => true];
        $json = json_encode($caps);
        $decoded = json_decode($json, true);

        $this->assertTrue($decoded['enabled']);
    }

    public function testCapabilitiesSerializationArray(): void
    {
        $caps = ['models' => ['gpt-4', 'claude-3']];
        $json = json_encode($caps);
        $decoded = json_decode($json, true);

        $this->assertCount(2, $decoded['models']);
        $this->assertEquals('gpt-4', $decoded['models'][0]);
    }

    public function testCapabilitiesSerializationObject(): void
    {
        $caps = ['config' => ['timeout' => 30]];
        $json = json_encode($caps);
        $decoded = json_decode($json, true);

        $this->assertEquals(30, $decoded['config']['timeout']);
    }

    public function testCapabilitiesSerializationNull(): void
    {
        $caps = ['optional' => null];
        $json = json_encode($caps);
        $decoded = json_decode($json, true);

        $this->assertNull($decoded['optional']);
    }

    // Backward Compatibility
    public function testCapabilitiesBackwardCompatibility(): void
    {
        // Old format: all strings
        $capabilities = [
            'model' => 'gpt-4',
            'count' => '100'  // String in old format
        ];

        $this->assertEquals('gpt-4', $capabilities['model']);
        $this->assertEquals('100', $capabilities['count']);
    }
}

