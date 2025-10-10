<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Core;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;
use UMICP\Exception\SerializationException;
use UMICP\Exception\ValidationException;

/**
 * Advanced envelope tests (edge cases, error conditions)
 *
 * @covers \UMICP\Core\Envelope
 */
class EnvelopeAdvancedTest extends TestCase
{
    public function testEmptyEnvelope(): void
    {
        $envelope = new Envelope();

        $this->assertNull($envelope->getFrom());
        $this->assertNull($envelope->getTo());
        $this->assertEquals(OperationType::DATA, $envelope->getOperation());
        $this->assertNull($envelope->getMessageId());
        $this->assertEquals([], $envelope->getCapabilities());
    }

    public function testEnvelopeWithSpecialCharacters(): void
    {
        $envelope = new Envelope(
            from: 'sender@domain.com',
            to: 'receiver/path/to/resource',
            capabilities: [
                'special-chars' => '!@#$%^&*()',
                'unicode' => 'Hello 世界 🌍',
                'newlines' => "Line1\nLine2\nLine3"
            ]
        );

        $json = $envelope->serialize();
        $deserialized = Envelope::deserialize($json);

        $this->assertEquals('sender@domain.com', $deserialized->getFrom());
        $this->assertEquals('Hello 世界 🌍', $deserialized->getCapability('unicode'));
    }

    public function testEnvelopeWithLargePayload(): void
    {
        $largeData = str_repeat('A', 10000);

        $envelope = new Envelope(
            from: 'sender',
            to: 'receiver',
            capabilities: ['large_data' => $largeData]
        );

        $json = $envelope->serialize();
        $this->assertGreaterThan(10000, strlen($json));

        $deserialized = Envelope::deserialize($json);
        $this->assertEquals($largeData, $deserialized->getCapability('large_data'));
    }

    public function testEnvelopeCloning(): void
    {
        $original = new Envelope(
            from: 'original-sender',
            to: 'original-receiver',
            capabilities: ['key' => 'value']
        );

        $json = $original->serialize();
        $clone = Envelope::deserialize($json);

        // Modify clone
        $clone->setFrom('cloned-sender');
        $clone->setCapability('modified', 'true');

        // Original should be unchanged
        $this->assertEquals('original-sender', $original->getFrom());
        $this->assertFalse($original->hasCapability('modified'));
    }

    public function testCapabilityOverwrite(): void
    {
        $envelope = new Envelope();

        $envelope->setCapability('key', 'value1');
        $this->assertEquals('value1', $envelope->getCapability('key'));

        $envelope->setCapability('key', 'value2');
        $this->assertEquals('value2', $envelope->getCapability('key'));
    }

    public function testMultipleCapabilityRemovals(): void
    {
        $envelope = new Envelope(
            capabilities: [
                'key1' => 'value1',
                'key2' => 'value2',
                'key3' => 'value3'
            ]
        );

        $envelope->removeCapability('key1')
                 ->removeCapability('key3');

        $caps = $envelope->getCapabilities();
        $this->assertCount(1, $caps);
        $this->assertTrue($envelope->hasCapability('key2'));
        $this->assertFalse($envelope->hasCapability('key1'));
        $this->assertFalse($envelope->hasCapability('key3'));
    }

    public function testEnvelopeWithNumericCapabilities(): void
    {
        $envelope = new Envelope(
            capabilities: [
                'count' => '42',
                'price' => '99.99',
                'id' => '12345'
            ]
        );

        $this->assertEquals('42', $envelope->getCapability('count'));
        $this->assertEquals('99.99', $envelope->getCapability('price'));
    }

    public function testEnvelopeHashConsistency(): void
    {
        $envelope1 = new Envelope(
            from: 'sender',
            to: 'receiver',
            operation: OperationType::DATA,
            messageId: 'msg-001'
        );

        $envelope2 = new Envelope(
            from: 'sender',
            to: 'receiver',
            operation: OperationType::DATA,
            messageId: 'msg-001'
        );

        // Same data should produce same hash (via serialization)
        $json1 = $envelope1->serialize();
        $json2 = $envelope2->serialize();

        $this->assertEquals($json1, $json2);
    }

    public function testEnvelopeArrayRoundTrip(): void
    {
        $envelope = new Envelope(
            from: 'sender',
            to: 'receiver',
            operation: OperationType::REQUEST,
            messageId: 'msg-abc',
            capabilities: ['key' => 'value']
        );

        $array = $envelope->toArray();
        $reconstructed = Envelope::fromArray($array);

        $this->assertEquals($envelope->getFrom(), $reconstructed->getFrom());
        $this->assertEquals($envelope->getTo(), $reconstructed->getTo());
        $this->assertEquals($envelope->getOperation(), $reconstructed->getOperation());
        $this->assertEquals($envelope->getMessageId(), $reconstructed->getMessageId());
    }

    public function testEnvelopeToString(): void
    {
        $envelope = new Envelope(
            from: 'sender',
            to: 'receiver'
        );

        $string = (string) $envelope;

        $this->assertIsString($string);
        $this->assertNotEmpty($string);
        $this->assertJson($string);
    }
}

