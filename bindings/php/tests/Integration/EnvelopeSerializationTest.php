<?php

declare(strict_types=1);

namespace UMICP\Tests\Integration;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;
use UMICP\Core\PayloadHint;
use UMICP\Core\PayloadType;
use UMICP\Core\EncodingType;

/**
 * Integration tests for envelope serialization/deserialization
 *
 * @group integration
 */
class EnvelopeSerializationTest extends TestCase
{
    public function testRoundTripSerialization(): void
    {
        $original = new Envelope(
            from: 'client-001',
            to: 'server-001',
            operation: OperationType::DATA,
            messageId: 'msg-12345',
            capabilities: [
                'content-type' => 'application/json',
                'priority' => 'high',
                'timestamp' => '1697000000'
            ]
        );

        $json = $original->serialize();
        $deserialized = Envelope::deserialize($json);

        $this->assertEquals($original->getFrom(), $deserialized->getFrom());
        $this->assertEquals($original->getTo(), $deserialized->getTo());
        $this->assertEquals($original->getOperation(), $deserialized->getOperation());
        $this->assertEquals($original->getMessageId(), $deserialized->getMessageId());
        $this->assertEquals($original->getCapabilities(), $deserialized->getCapabilities());
    }

    public function testMultipleRoundTrips(): void
    {
        $envelope = new Envelope(
            from: 'sender',
            to: 'receiver',
            operation: OperationType::ACK
        );

        for ($i = 0; $i < 100; $i++) {
            $json = $envelope->serialize();
            $envelope = Envelope::deserialize($json);
        }

        $this->assertEquals('sender', $envelope->getFrom());
        $this->assertEquals('receiver', $envelope->getTo());
        $this->assertEquals(OperationType::ACK, $envelope->getOperation());
    }

    public function testLargeCapabilities(): void
    {
        $capabilities = [];
        for ($i = 0; $i < 100; $i++) {
            $capabilities["key_$i"] = "value_$i";
        }

        $envelope = new Envelope(
            from: 'sender',
            to: 'receiver',
            capabilities: $capabilities
        );

        $json = $envelope->serialize();
        $deserialized = Envelope::deserialize($json);

        $this->assertEquals($capabilities, $deserialized->getCapabilities());
    }

    public function testEnvelopeWithPayloadHint(): void
    {
        $hint = new PayloadHint(
            type: PayloadType::VECTOR,
            size: 1024,
            encoding: EncodingType::FLOAT32,
            count: 256
        );

        $envelope = new Envelope(
            from: 'sender',
            to: 'receiver',
            payloadHint: $hint
        );

        $this->assertNotNull($envelope->getPayloadHint());
        $this->assertEquals(PayloadType::VECTOR, $envelope->getPayloadHint()->getType());
    }

    public function testAllOperationTypes(): void
    {
        foreach (OperationType::cases() as $opType) {
            $envelope = new Envelope(
                from: 'sender',
                to: 'receiver',
                operation: $opType
            );

            $json = $envelope->serialize();
            $deserialized = Envelope::deserialize($json);

            $this->assertEquals($opType, $deserialized->getOperation());
        }
    }
}

