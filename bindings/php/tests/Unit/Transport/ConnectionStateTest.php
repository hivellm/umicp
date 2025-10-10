<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Transport;

use PHPUnit\Framework\TestCase;
use UMICP\Transport\ConnectionState;

/**
 * @covers \UMICP\Transport\ConnectionState
 */
class ConnectionStateTest extends TestCase
{
    private ConnectionState $state;

    protected function setUp(): void
    {
        $this->state = new ConnectionState();
    }

    public function testInitialState(): void
    {
        $this->assertFalse($this->state->isConnected());
        $this->assertEquals(0, $this->state->getMessagesSent());
        $this->assertEquals(0, $this->state->getMessagesReceived());
        $this->assertNull($this->state->getConnectedAt());
    }

    public function testSetConnected(): void
    {
        $this->state->setConnected(true);

        $this->assertTrue($this->state->isConnected());
        $this->assertNotNull($this->state->getConnectedAt());
        $this->assertNull($this->state->getDisconnectedAt());
    }

    public function testSetDisconnected(): void
    {
        $this->state->setConnected(true);
        $this->state->setConnected(false);

        $this->assertFalse($this->state->isConnected());
        $this->assertNotNull($this->state->getDisconnectedAt());
    }

    public function testMessageCounters(): void
    {
        $this->state->incrementMessagesSent();
        $this->state->incrementMessagesSent();
        $this->state->incrementMessagesReceived();

        $this->assertEquals(2, $this->state->getMessagesSent());
        $this->assertEquals(1, $this->state->getMessagesReceived());
    }

    public function testByteCounters(): void
    {
        $this->state->addBytesSent(100);
        $this->state->addBytesSent(50);
        $this->state->addBytesReceived(75);

        $this->assertEquals(150, $this->state->getBytesSent());
        $this->assertEquals(75, $this->state->getBytesReceived());
    }

    public function testUptime(): void
    {
        $this->assertNull($this->state->getUptime());

        $this->state->setConnected(true);
        usleep(10000); // 10ms

        $uptime = $this->state->getUptime();
        $this->assertNotNull($uptime);
        $this->assertGreaterThan(0, $uptime);
    }

    public function testReset(): void
    {
        $this->state->setConnected(true);
        $this->state->incrementMessagesSent();
        $this->state->addBytesSent(100);

        $this->state->reset();

        $this->assertFalse($this->state->isConnected());
        $this->assertEquals(0, $this->state->getMessagesSent());
        $this->assertEquals(0, $this->state->getBytesSent());
        $this->assertNull($this->state->getConnectedAt());
    }

    public function testToArray(): void
    {
        $this->state->setConnected(true);
        $this->state->incrementMessagesSent();

        $array = $this->state->toArray();

        $this->assertIsArray($array);
        $this->assertTrue($array['connected']);
        $this->assertEquals(1, $array['messages_sent']);
        $this->assertArrayHasKey('uptime', $array);
    }
}

