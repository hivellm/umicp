<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Core;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;
use UMICP\Exception\ValidationException;
use UMICP\Exception\SerializationException;

/**
 * @covers \UMICP\Core\Envelope
 */
class EnvelopeTest extends TestCase
{
    public function testEnvelopeCreation(): void
    {
        $envelope = new Envelope(
            from: 'sender',
            to: 'receiver',
            operation: OperationType::DATA,
            messageId: 'msg-123'
        );

        $this->assertEquals('sender', $envelope->getFrom());
        $this->assertEquals('receiver', $envelope->getTo());
        $this->assertEquals(OperationType::DATA, $envelope->getOperation());
        $this->assertEquals('msg-123', $envelope->getMessageId());
    }

    public function testFluentInterface(): void
    {
        $envelope = (new Envelope())
            ->setFrom('sender')
            ->setTo('receiver')
            ->setOperation(OperationType::ACK)
            ->setMessageId('msg-456');

        $this->assertEquals('sender', $envelope->getFrom());
        $this->assertEquals('receiver', $envelope->getTo());
        $this->assertEquals(OperationType::ACK, $envelope->getOperation());
    }

    public function testCapabilities(): void
    {
        $envelope = new Envelope();

        $envelope->setCapabilities([
            'key1' => 'value1',
            'key2' => 'value2'
        ]);

        $this->assertEquals('value1', $envelope->getCapability('key1'));
        $this->assertEquals('value2', $envelope->getCapability('key2'));
        $this->assertNull($envelope->getCapability('nonexistent'));
        $this->assertEquals('default', $envelope->getCapability('nonexistent', 'default'));
    }

    public function testSingleCapability(): void
    {
        $envelope = new Envelope();

        $envelope->setCapability('test', 'value');

        $this->assertTrue($envelope->hasCapability('test'));
        $this->assertEquals('value', $envelope->getCapability('test'));
        $this->assertFalse($envelope->hasCapability('nonexistent'));
    }

    public function testRemoveCapability(): void
    {
        $envelope = new Envelope(
            capabilities: ['key1' => 'value1', 'key2' => 'value2']
        );

        $this->assertTrue($envelope->hasCapability('key1'));

        $envelope->removeCapability('key1');

        $this->assertFalse($envelope->hasCapability('key1'));
        $this->assertTrue($envelope->hasCapability('key2'));
    }

    public function testToArray(): void
    {
        $envelope = new Envelope(
            from: 'sender',
            to: 'receiver',
            operation: OperationType::DATA,
            messageId: 'msg-789',
            capabilities: ['key' => 'value']
        );

        $array = $envelope->toArray();

        $this->assertIsArray($array);
        $this->assertEquals('sender', $array['from']);
        $this->assertEquals('receiver', $array['to']);
        $this->assertEquals(OperationType::DATA->value, $array['operation']);
        $this->assertEquals('msg-789', $array['messageId']);
        $this->assertArrayHasKey('capabilities', $array);
    }

    public function testFromArray(): void
    {
        $data = [
            'from' => 'sender',
            'to' => 'receiver',
            'operation' => OperationType::ACK->value,
            'messageId' => 'msg-abc',
            'capabilities' => ['key' => 'value']
        ];

        $envelope = Envelope::fromArray($data);

        $this->assertEquals('sender', $envelope->getFrom());
        $this->assertEquals('receiver', $envelope->getTo());
        $this->assertEquals(OperationType::ACK, $envelope->getOperation());
    }

    public function testOperationTypeHelpers(): void
    {
        $envelope = new Envelope(operation: OperationType::DATA);
        $this->assertTrue($envelope->getOperation()->isData());
        $this->assertFalse($envelope->getOperation()->isControl());

        $envelope->setOperation(OperationType::CONTROL);
        $this->assertTrue($envelope->getOperation()->isControl());
        $this->assertFalse($envelope->getOperation()->isData());
    }
}

