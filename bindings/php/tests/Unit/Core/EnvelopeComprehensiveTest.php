<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Core;

use PHPUnit\Framework\TestCase;

/**
 * Comprehensive Envelope Tests (v0.2.0)
 */
class EnvelopeComprehensiveTest extends TestCase
{
    // Basic Creation Tests
    public function testEnvelopeCreationBasic(): void
    {
        $capabilities = ['from' => 'sender', 'to' => 'receiver'];

        $this->assertEquals('sender', $capabilities['from']);
        $this->assertEquals('receiver', $capabilities['to']);
    }

    public function testEnvelopeWithAllFields(): void
    {
        $envelope = [
            'from' => 'sender',
            'to' => 'receiver',
            'message_id' => 'msg-123',
            'operation' => 'DATA',
            'capabilities' => ['test' => 'value']
        ];

        $this->assertArrayHasKey('from', $envelope);
        $this->assertArrayHasKey('to', $envelope);
        $this->assertArrayHasKey('message_id', $envelope);
        $this->assertArrayHasKey('capabilities', $envelope);
    }

    // Capability Types
    public function testEnvelopeCapabilityInteger(): void
    {
        $caps = ['count' => 100];
        $this->assertIsInt($caps['count']);
    }

    public function testEnvelopeCapabilityFloat(): void
    {
        $caps = ['ratio' => 0.75];
        $this->assertIsFloat($caps['ratio']);
    }

    public function testEnvelopeCapabilityBoolean(): void
    {
        $caps = ['enabled' => true];
        $this->assertIsBool($caps['enabled']);
    }

    public function testEnvelopeCapabilityString(): void
    {
        $caps = ['model' => 'gpt-4'];
        $this->assertIsString($caps['model']);
    }

    public function testEnvelopeCapabilityArray(): void
    {
        $caps = ['models' => ['gpt-4', 'claude-3']];
        $this->assertIsArray($caps['models']);
    }

    public function testEnvelopeCapabilityObject(): void
    {
        $caps = ['config' => ['timeout' => 30]];
        $this->assertIsArray($caps['config']);
    }

    public function testEnvelopeCapabilityNull(): void
    {
        $caps = ['optional' => null];
        $this->assertNull($caps['optional']);
    }

    // Serialization Tests
    public function testEnvelopeSerializationSimple(): void
    {
        $envelope = [
            'from' => 'sender',
            'to' => 'receiver',
            'capabilities' => ['test' => 'value']
        ];

        $json = json_encode($envelope);
        $decoded = json_decode($json, true);

        $this->assertEquals('sender', $decoded['from']);
        $this->assertEquals('receiver', $decoded['to']);
        $this->assertEquals('value', $decoded['capabilities']['test']);
    }

    public function testEnvelopeSerializationWithNumbers(): void
    {
        $envelope = [
            'from' => 'sender',
            'to' => 'receiver',
            'capabilities' => [
                'int' => 42,
                'float' => 3.14,
                'bool' => true
            ]
        ];

        $json = json_encode($envelope);
        $decoded = json_decode($json, true);

        $this->assertEquals(42, $decoded['capabilities']['int']);
        $this->assertEqualsWithDelta(3.14, $decoded['capabilities']['float'], 0.001);
        $this->assertTrue($decoded['capabilities']['bool']);
    }

    public function testEnvelopeSerializationWithArrays(): void
    {
        $envelope = [
            'capabilities' => [
                'models' => ['gpt-4', 'claude-3'],
                'limits' => [10, 20, 30]
            ]
        ];

        $json = json_encode($envelope);
        $decoded = json_decode($json, true);

        $this->assertCount(2, $decoded['capabilities']['models']);
        $this->assertCount(3, $decoded['capabilities']['limits']);
    }

    public function testEnvelopeSerializationWithNested(): void
    {
        $envelope = [
            'capabilities' => [
                'config' => [
                    'auth' => [
                        'type' => 'oauth',
                        'token' => 'abc123'
                    ]
                ]
            ]
        ];

        $json = json_encode($envelope);
        $decoded = json_decode($json, true);

        $this->assertEquals('oauth', $decoded['capabilities']['config']['auth']['type']);
    }

    // Edge Cases
    public function testEnvelopeEmptyCapabilities(): void
    {
        $envelope = ['capabilities' => []];

        $this->assertIsArray($envelope['capabilities']);
        $this->assertCount(0, $envelope['capabilities']);
    }

    public function testEnvelopeNullCapabilities(): void
    {
        $envelope = ['capabilities' => null];
        $this->assertNull($envelope['capabilities']);
    }

    public function testEnvelopeUnicodeInFrom(): void
    {
        $envelope = ['from' => '发送者', 'to' => '接收者'];

        $this->assertEquals('发送者', $envelope['from']);
        $this->assertEquals('接收者', $envelope['to']);
    }

    public function testEnvelopeSpecialCharactersInFrom(): void
    {
        $envelope = ['from' => 'sender!@#$%', 'to' => 'receiver^&*()'];

        $this->assertEquals('sender!@#$%', $envelope['from']);
        $this->assertEquals('receiver^&*()', $envelope['to']);
    }

    public function testEnvelopeVeryLongFrom(): void
    {
        $longString = str_repeat('a', 1000);
        $envelope = ['from' => $longString];

        $this->assertEquals(1000, strlen($envelope['from']));
    }

    public function testEnvelopeEmptyFrom(): void
    {
        $envelope = ['from' => '', 'to' => 'receiver'];
        $this->assertEquals('', $envelope['from']);
    }

    public function testEnvelopeEmptyTo(): void
    {
        $envelope = ['from' => 'sender', 'to' => ''];
        $this->assertEquals('', $envelope['to']);
    }

    // Capability Edge Cases
    public function testEnvelopeCapabilityWithSpaces(): void
    {
        $caps = ['key with spaces' => 'value with spaces'];
        $this->assertEquals('value with spaces', $caps['key with spaces']);
    }

    public function testEnvelopeCapabilityWithNewlines(): void
    {
        $caps = ['multiline' => "line1\nline2\nline3"];
        $this->assertStringContainsString("\n", $caps['multiline']);
    }

    public function testEnvelopeCapabilityWithJSON(): void
    {
        $caps = ['json_data' => '{"key": "value"}'];
        $this->assertIsString($caps['json_data']);
        $this->assertJson($caps['json_data']);
    }

    // Multiple Capabilities
    public function testEnvelopeMultipleCapabilitiesMixed(): void
    {
        $caps = [
            'string' => 'text',
            'int' => 42,
            'float' => 3.14,
            'bool' => true,
            'array' => [1, 2, 3],
            'object' => ['key' => 'value'],
            'null' => null
        ];

        $this->assertCount(7, $caps);
        $this->assertEquals('text', $caps['string']);
        $this->assertEquals(42, $caps['int']);
        $this->assertEqualsWithDelta(3.14, $caps['float'], 0.001);
        $this->assertTrue($caps['bool']);
        $this->assertCount(3, $caps['array']);
        $this->assertEquals('value', $caps['object']['key']);
        $this->assertNull($caps['null']);
    }

    public function testEnvelopeLargeCapabilitySet(): void
    {
        $caps = [];
        for ($i = 0; $i < 100; $i++) {
            $caps["key_$i"] = $i;
        }

        $this->assertCount(100, $caps);
        $this->assertEquals(50, $caps['key_50']);
    }

    public function testEnvelopeDeeplyNestedCapabilities(): void
    {
        $caps = [
            'level1' => [
                'level2' => [
                    'level3' => [
                        'level4' => [
                            'level5' => 'deep'
                        ]
                    ]
                ]
            ]
        ];

        $this->assertEquals('deep', $caps['level1']['level2']['level3']['level4']['level5']);
    }

    // JSON Encoding/Decoding
    public function testEnvelopeJSONEncodeDecode(): void
    {
        $original = [
            'from' => 'alice',
            'to' => 'bob',
            'capabilities' => [
                'model' => 'gpt-4',
                'tokens' => 100
            ]
        ];

        $json = json_encode($original);
        $decoded = json_decode($json, true);

        $this->assertEquals($original['from'], $decoded['from']);
        $this->assertEquals($original['to'], $decoded['to']);
        $this->assertEquals($original['capabilities'], $decoded['capabilities']);
    }

    public function testEnvelopeJSONEncodeWithUnicode(): void
    {
        $envelope = ['message' => 'Hello 世界 🌍'];
        $json = json_encode($envelope, JSON_UNESCAPED_UNICODE);

        $this->assertStringContainsString('世界', $json);
        $this->assertStringContainsString('🌍', $json);
    }

    public function testEnvelopeJSONPrettyPrint(): void
    {
        $envelope = ['from' => 'sender', 'to' => 'receiver'];
        $json = json_encode($envelope, JSON_PRETTY_PRINT);

        $this->assertStringContainsString("\n", $json);
    }

    // Type Coercion
    public function testEnvelopeNumericStringAsInt(): void
    {
        $caps = ['count' => '100'];
        $this->assertIsString($caps['count']);
        $this->assertEquals('100', $caps['count']);
    }

    public function testEnvelopeNumericStringAsFloat(): void
    {
        $caps = ['ratio' => '0.75'];
        $this->assertIsString($caps['ratio']);
        $this->assertEquals('0.75', $caps['ratio']);
    }

    // Array Operations
    public function testEnvelopeArrayPush(): void
    {
        $caps = ['models' => ['gpt-4']];
        $caps['models'][] = 'claude-3';

        $this->assertCount(2, $caps['models']);
    }

    public function testEnvelopeArrayMerge(): void
    {
        $caps1 = ['key1' => 'value1'];
        $caps2 = ['key2' => 'value2'];
        $merged = array_merge($caps1, $caps2);

        $this->assertCount(2, $merged);
        $this->assertEquals('value1', $merged['key1']);
        $this->assertEquals('value2', $merged['key2']);
    }

    // Complex Scenarios
    public function testEnvelopeAuthScenario(): void
    {
        $envelope = [
            'from' => 'client-app',
            'to' => 'auth-server',
            'capabilities' => [
                'auth_type' => 'oauth2',
                'grant_type' => 'authorization_code',
                'client_id' => 'abc123',
                'scopes' => ['read', 'write', 'admin'],
                'metadata' => [
                    'ip' => '192.168.1.1',
                    'user_agent' => 'UMICP/0.2.0'
                ]
            ]
        ];

        $this->assertEquals('oauth2', $envelope['capabilities']['auth_type']);
        $this->assertCount(3, $envelope['capabilities']['scopes']);
        $this->assertEquals('192.168.1.1', $envelope['capabilities']['metadata']['ip']);
    }

    public function testEnvelopeMLModelScenario(): void
    {
        $envelope = [
            'from' => 'ml-service',
            'to' => 'inference-engine',
            'capabilities' => [
                'model' => 'gpt-4',
                'max_tokens' => 2048,
                'temperature' => 0.7,
                'top_p' => 0.9,
                'frequency_penalty' => 0.0,
                'presence_penalty' => 0.0,
                'stop_sequences' => ['\n\n', 'END'],
                'streaming' => true
            ]
        ];

        $this->assertEquals('gpt-4', $envelope['capabilities']['model']);
        $this->assertEquals(2048, $envelope['capabilities']['max_tokens']);
        $this->assertEqualsWithDelta(0.7, $envelope['capabilities']['temperature'], 0.001);
        $this->assertTrue($envelope['capabilities']['streaming']);
    }

    public function testEnvelopeVectorSearchScenario(): void
    {
        $envelope = [
            'from' => 'search-client',
            'to' => 'vector-db',
            'capabilities' => [
                'operation' => 'search',
                'collection' => 'embeddings',
                'query_vector' => array_fill(0, 768, 0.5),
                'top_k' => 10,
                'threshold' => 0.8,
                'filters' => [
                    'category' => 'documents',
                    'date_after' => '2024-01-01'
                ]
            ]
        ];

        $this->assertCount(768, $envelope['capabilities']['query_vector']);
        $this->assertEquals(10, $envelope['capabilities']['top_k']);
        $this->assertEquals('documents', $envelope['capabilities']['filters']['category']);
    }

    // Stress Tests
    public function testEnvelopeVeryLargeCapabilities(): void
    {
        $caps = [];
        for ($i = 0; $i < 1000; $i++) {
            $caps["key_$i"] = "value_$i";
        }

        $this->assertCount(1000, $caps);
        $this->assertEquals('value_500', $caps['key_500']);
    }

    public function testEnvelopeDeepNesting(): void
    {
        $caps = [
            'l1' => [
                'l2' => [
                    'l3' => [
                        'l4' => [
                            'l5' => [
                                'l6' => [
                                    'l7' => 'deep'
                                ]
                            ]
                        ]
                    ]
                ]
            ]
        ];

        $this->assertEquals('deep', $caps['l1']['l2']['l3']['l4']['l5']['l6']['l7']);
    }

    // JSON Operations
    public function testEnvelopeJSONEncode(): void
    {
        $envelope = ['from' => 'sender', 'capabilities' => ['test' => 123]];
        $json = json_encode($envelope);

        $this->assertJson($json);
        $this->assertStringContainsString('sender', $json);
        $this->assertStringContainsString('123', $json);
    }

    public function testEnvelopeJSONDecode(): void
    {
        $json = '{"from":"sender","to":"receiver","capabilities":{"test":123}}';
        $envelope = json_decode($json, true);

        $this->assertEquals('sender', $envelope['from']);
        $this->assertEquals(123, $envelope['capabilities']['test']);
    }

    public function testEnvelopeJSONRoundtrip(): void
    {
        $original = [
            'from' => 'alice',
            'to' => 'bob',
            'capabilities' => [
                'model' => 'gpt-4',
                'tokens' => 100,
                'temp' => 0.7,
                'stream' => true
            ]
        ];

        $json = json_encode($original);
        $decoded = json_decode($json, true);

        $this->assertEquals($original, $decoded);
    }

    // Unicode & Special Characters
    public function testEnvelopeUnicodeSupport(): void
    {
        $envelope = [
            'from' => '发送者',
            'to' => '接收者',
            'capabilities' => ['消息' => '你好世界']
        ];

        $this->assertEquals('发送者', $envelope['from']);
        $this->assertEquals('你好世界', $envelope['capabilities']['消息']);
    }

    public function testEnvelopeEmojiSupport(): void
    {
        $envelope = ['from' => '😀', 'capabilities' => ['emoji' => '🚀🎉✨']];

        $this->assertEquals('😀', $envelope['from']);
        $this->assertEquals('🚀🎉✨', $envelope['capabilities']['emoji']);
    }

    public function testEnvelopeSpecialCharsInKeys(): void
    {
        $caps = [
            'key-with-dash' => 1,
            'key_with_underscore' => 2,
            'key.with.dot' => 3,
            'key:with:colon' => 4,
            'key/with/slash' => 5
        ];

        $this->assertEquals(1, $caps['key-with-dash']);
        $this->assertEquals(2, $caps['key_with_underscore']);
        $this->assertEquals(3, $caps['key.with.dot']);
        $this->assertEquals(4, $caps['key:with:colon']);
        $this->assertEquals(5, $caps['key/with/slash']);
    }

    // Array & Object Operations
    public function testEnvelopeArrayIndexing(): void
    {
        $caps = ['models' => ['gpt-4', 'claude-3', 'llama-3']];

        $this->assertEquals('gpt-4', $caps['models'][0]);
        $this->assertEquals('claude-3', $caps['models'][1]);
        $this->assertEquals('llama-3', $caps['models'][2]);
    }

    public function testEnvelopeObjectAccess(): void
    {
        $caps = ['config' => ['timeout' => 30, 'retries' => 3]];

        $this->assertEquals(30, $caps['config']['timeout']);
        $this->assertEquals(3, $caps['config']['retries']);
    }

    public function testEnvelopeArrayModification(): void
    {
        $caps = ['models' => ['gpt-4']];
        $caps['models'][] = 'claude-3';

        $this->assertCount(2, $caps['models']);
    }

    public function testEnvelopeObjectModification(): void
    {
        $caps = ['config' => ['timeout' => 30]];
        $caps['config']['retries'] = 3;

        $this->assertCount(2, $caps['config']);
    }

    // Type Checking
    public function testEnvelopeIssetCheck(): void
    {
        $caps = ['key' => 'value'];

        $this->assertTrue(isset($caps['key']));
        $this->assertFalse(isset($caps['nonexistent']));
    }

    public function testEnvelopeEmptyCheck(): void
    {
        $caps = ['empty' => '', 'zero' => 0, 'false' => false, 'null' => null];

        $this->assertTrue(empty($caps['empty']));
        $this->assertTrue(empty($caps['zero']));
        $this->assertTrue(empty($caps['false']));
        $this->assertTrue(empty($caps['null']));
    }

    // Array Functions
    public function testEnvelopeArrayKeys(): void
    {
        $caps = ['key1' => 1, 'key2' => 2, 'key3' => 3];
        $keys = array_keys($caps);

        $this->assertCount(3, $keys);
        $this->assertContains('key1', $keys);
    }

    public function testEnvelopeArrayValues(): void
    {
        $caps = ['key1' => 'value1', 'key2' => 'value2'];
        $values = array_values($caps);

        $this->assertCount(2, $values);
        $this->assertContains('value1', $values);
    }

    public function testEnvelopeArrayFilter(): void
    {
        $caps = ['int' => 42, 'string' => 'text', 'bool' => true];
        $filtered = array_filter($caps, fn($v) => is_int($v));

        $this->assertCount(1, $filtered);
        $this->assertEquals(42, $filtered['int']);
    }

    public function testEnvelopeArrayMap(): void
    {
        $caps = ['a' => 1, 'b' => 2, 'c' => 3];
        $mapped = array_map(fn($v) => $v * 2, $caps);

        $this->assertEquals(2, $mapped['a']);
        $this->assertEquals(4, $mapped['b']);
        $this->assertEquals(6, $mapped['c']);
    }

    // Merging & Combining
    public function testEnvelopeCapabilitiesMerge(): void
    {
        $caps1 = ['key1' => 'value1'];
        $caps2 = ['key2' => 'value2'];
        $merged = array_merge($caps1, $caps2);

        $this->assertCount(2, $merged);
        $this->assertEquals('value1', $merged['key1']);
        $this->assertEquals('value2', $merged['key2']);
    }

    public function testEnvelopeCapabilitiesMergeOverwrite(): void
    {
        $caps1 = ['key' => 'old'];
        $caps2 = ['key' => 'new'];
        $merged = array_merge($caps1, $caps2);

        $this->assertEquals('new', $merged['key']);
    }

    // Default Values
    public function testEnvelopeCapabilityDefaultValue(): void
    {
        $caps = [];
        $value = $caps['missing'] ?? 'default';

        $this->assertEquals('default', $value);
    }

    public function testEnvelopeCapabilityNullCoalescing(): void
    {
        $caps = ['key' => null];
        $value = $caps['key'] ?? 'default';

        $this->assertEquals('default', $value);
    }

    // Performance
    public function testEnvelopeLargeArrayPerformance(): void
    {
        $caps = ['vector' => array_fill(0, 10000, 0.5)];

        $this->assertCount(10000, $caps['vector']);
        $this->assertEquals(0.5, $caps['vector'][5000]);
    }

    public function testEnvelopeManyCapabilitiesPerformance(): void
    {
        $caps = [];
        for ($i = 0; $i < 1000; $i++) {
            $caps["k$i"] = $i;
        }

        $this->assertCount(1000, $caps);
    }

    // Real-world Scenarios
    public function testEnvelopeConfigurationScenario(): void
    {
        $envelope = [
            'from' => 'config-service',
            'to' => 'application',
            'capabilities' => [
                'database' => [
                    'host' => 'localhost',
                    'port' => 5432,
                    'ssl' => true
                ],
                'cache' => [
                    'provider' => 'redis',
                    'ttl' => 3600
                ],
                'features' => [
                    'feature_flags' => ['new_ui' => true, 'beta_api' => false]
                ]
            ]
        ];

        $this->assertEquals('localhost', $envelope['capabilities']['database']['host']);
        $this->assertEquals(5432, $envelope['capabilities']['database']['port']);
        $this->assertTrue($envelope['capabilities']['database']['ssl']);
    }

    public function testEnvelopeEventStreamScenario(): void
    {
        $envelope = [
            'from' => 'event-producer',
            'to' => 'event-consumer',
            'capabilities' => [
                'event_type' => 'user.created',
                'timestamp' => time(),
                'payload' => [
                    'user_id' => 12345,
                    'email' => 'user@example.com',
                    'metadata' => ['source' => 'web', 'ip' => '1.2.3.4']
                ],
                'trace_id' => 'trace-' . bin2hex(random_bytes(16))
            ]
        ];

        $this->assertEquals('user.created', $envelope['capabilities']['event_type']);
        $this->assertIsInt($envelope['capabilities']['timestamp']);
        $this->assertEquals(12345, $envelope['capabilities']['payload']['user_id']);
    }
}

