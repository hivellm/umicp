<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Core;

use PHPUnit\Framework\TestCase;
use UMICP\Core\PayloadHint;
use UMICP\Core\PayloadType;
use UMICP\Core\EncodingType;

/**
 * @covers \UMICP\Core\PayloadHint
 */
class PayloadHintTest extends TestCase
{
    public function testPayloadHintCreation(): void
    {
        $hint = new PayloadHint(
            type: PayloadType::VECTOR,
            size: 1024,
            encoding: EncodingType::FLOAT32,
            count: 256
        );

        $this->assertEquals(PayloadType::VECTOR, $hint->getType());
        $this->assertEquals(1024, $hint->getSize());
        $this->assertEquals(EncodingType::FLOAT32, $hint->getEncoding());
        $this->assertEquals(256, $hint->getCount());
    }

    public function testPayloadHintWithNullValues(): void
    {
        $hint = new PayloadHint();

        $this->assertNull($hint->getType());
        $this->assertNull($hint->getSize());
        $this->assertNull($hint->getEncoding());
        $this->assertNull($hint->getCount());
    }

    public function testFluentInterface(): void
    {
        $hint = (new PayloadHint())
            ->setType(PayloadType::TEXT)
            ->setSize(512)
            ->setEncoding(EncodingType::UINT8)
            ->setCount(512);

        $this->assertEquals(PayloadType::TEXT, $hint->getType());
        $this->assertEquals(512, $hint->getSize());
        $this->assertEquals(EncodingType::UINT8, $hint->getEncoding());
        $this->assertEquals(512, $hint->getCount());
    }

    public function testToArray(): void
    {
        $hint = new PayloadHint(
            type: PayloadType::BINARY,
            size: 2048,
            encoding: EncodingType::UINT8,
            count: 2048
        );

        $array = $hint->toArray();

        $this->assertIsArray($array);
        $this->assertEquals(PayloadType::BINARY->value, $array['type']);
        $this->assertEquals(2048, $array['size']);
        $this->assertEquals(EncodingType::UINT8->value, $array['encoding']);
        $this->assertEquals(2048, $array['count']);
    }

    public function testFromArray(): void
    {
        $data = [
            'type' => PayloadType::VECTOR->value,
            'size' => 768,
            'encoding' => EncodingType::FLOAT64->value,
            'count' => 96
        ];

        $hint = PayloadHint::fromArray($data);

        $this->assertEquals(PayloadType::VECTOR, $hint->getType());
        $this->assertEquals(768, $hint->getSize());
        $this->assertEquals(EncodingType::FLOAT64, $hint->getEncoding());
        $this->assertEquals(96, $hint->getCount());
    }

    public function testFromArrayWithMissingValues(): void
    {
        $data = ['type' => PayloadType::TEXT->value];

        $hint = PayloadHint::fromArray($data);

        $this->assertEquals(PayloadType::TEXT, $hint->getType());
        $this->assertNull($hint->getSize());
        $this->assertNull($hint->getEncoding());
        $this->assertNull($hint->getCount());
    }
}

