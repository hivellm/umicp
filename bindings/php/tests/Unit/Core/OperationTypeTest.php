<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Core;

use PHPUnit\Framework\TestCase;
use UMICP\Core\OperationType;

/**
 * @covers \UMICP\Core\OperationType
 */
class OperationTypeTest extends TestCase
{
    public function testAllOperationTypes(): void
    {
        $this->assertEquals(0, OperationType::CONTROL->value);
        $this->assertEquals(1, OperationType::DATA->value);
        $this->assertEquals(2, OperationType::ACK->value);
        $this->assertEquals(3, OperationType::ERROR->value);
        $this->assertEquals(4, OperationType::REQUEST->value);
        $this->assertEquals(5, OperationType::RESPONSE->value);
    }

    public function testOperationTypeCount(): void
    {
        $types = OperationType::cases();
        $this->assertCount(6, $types);
    }

    public function testOperationTypeNames(): void
    {
        $this->assertEquals('CONTROL', OperationType::CONTROL->getName());
        $this->assertEquals('DATA', OperationType::DATA->getName());
        $this->assertEquals('ACK', OperationType::ACK->getName());
    }

    public function testOperationTypeHelpers(): void
    {
        $this->assertTrue(OperationType::DATA->isData());
        $this->assertFalse(OperationType::DATA->isControl());

        $this->assertTrue(OperationType::ACK->isAck());
        $this->assertFalse(OperationType::ACK->isError());

        $this->assertTrue(OperationType::ERROR->isError());
        $this->assertTrue(OperationType::CONTROL->isControl());
    }

    public function testOperationTypeToString(): void
    {
        $this->assertEquals('DATA', (string) OperationType::DATA);
        $this->assertEquals('CONTROL', (string) OperationType::CONTROL);
        $this->assertEquals('ACK', (string) OperationType::ACK);
    }

    public function testOperationTypeFromValue(): void
    {
        $op = OperationType::from(1);
        $this->assertEquals(OperationType::DATA, $op);

        $op = OperationType::from(2);
        $this->assertEquals(OperationType::ACK, $op);
    }

    public function testOperationTypeIteration(): void
    {
        $names = [];
        foreach (OperationType::cases() as $type) {
            $names[] = $type->name;
        }

        $this->assertContains('DATA', $names);
        $this->assertContains('CONTROL', $names);
        $this->assertContains('ACK', $names);
        $this->assertContains('ERROR', $names);
        $this->assertContains('REQUEST', $names);
        $this->assertContains('RESPONSE', $names);
    }
}

