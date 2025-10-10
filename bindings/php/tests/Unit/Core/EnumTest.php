<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Core;

use PHPUnit\Framework\TestCase;
use UMICP\Core\OperationType;
use UMICP\Core\PayloadType;
use UMICP\Core\EncodingType;

/**
 * @covers \UMICP\Core\OperationType
 * @covers \UMICP\Core\PayloadType
 * @covers \UMICP\Core\EncodingType
 */
class EnumTest extends TestCase
{
    public function testOperationTypeValues(): void
    {
        $this->assertEquals(0, OperationType::CONTROL->value);
        $this->assertEquals(1, OperationType::DATA->value);
        $this->assertEquals(2, OperationType::ACK->value);
        $this->assertEquals(3, OperationType::ERROR->value);
        $this->assertEquals(4, OperationType::REQUEST->value);
        $this->assertEquals(5, OperationType::RESPONSE->value);
    }

    public function testOperationTypeHelpers(): void
    {
        $this->assertTrue(OperationType::DATA->isData());
        $this->assertFalse(OperationType::DATA->isControl());
        $this->assertFalse(OperationType::DATA->isAck());
        $this->assertFalse(OperationType::DATA->isError());

        $this->assertTrue(OperationType::ACK->isAck());
        $this->assertTrue(OperationType::ERROR->isError());
    }

    public function testOperationTypeName(): void
    {
        $this->assertEquals('DATA', OperationType::DATA->getName());
        $this->assertEquals('CONTROL', OperationType::CONTROL->getName());
    }

    public function testOperationTypeToString(): void
    {
        $this->assertEquals('DATA', (string) OperationType::DATA);
        $this->assertEquals('ACK', (string) OperationType::ACK);
    }

    public function testPayloadTypeValues(): void
    {
        $this->assertEquals(0, PayloadType::VECTOR->value);
        $this->assertEquals(1, PayloadType::TEXT->value);
        $this->assertEquals(2, PayloadType::METADATA->value);
        $this->assertEquals(3, PayloadType::BINARY->value);
    }

    public function testEncodingTypeValues(): void
    {
        $this->assertEquals(0, EncodingType::FLOAT32->value);
        $this->assertEquals(1, EncodingType::FLOAT64->value);
        $this->assertEquals(2, EncodingType::INT32->value);
        $this->assertEquals(7, EncodingType::UINT64->value);
    }

    public function testEncodingTypeSize(): void
    {
        $this->assertEquals(4, EncodingType::FLOAT32->getSize());
        $this->assertEquals(8, EncodingType::FLOAT64->getSize());
        $this->assertEquals(4, EncodingType::INT32->getSize());
        $this->assertEquals(8, EncodingType::INT64->getSize());
        $this->assertEquals(1, EncodingType::UINT8->getSize());
        $this->assertEquals(2, EncodingType::UINT16->getSize());
    }

    public function testEncodingTypeIsFloat(): void
    {
        $this->assertTrue(EncodingType::FLOAT32->isFloat());
        $this->assertTrue(EncodingType::FLOAT64->isFloat());
        $this->assertFalse(EncodingType::INT32->isFloat());
        $this->assertFalse(EncodingType::UINT8->isFloat());
    }

    public function testEncodingTypeIsInteger(): void
    {
        $this->assertFalse(EncodingType::FLOAT32->isInteger());
        $this->assertTrue(EncodingType::INT32->isInteger());
        $this->assertTrue(EncodingType::UINT8->isInteger());
    }
}

