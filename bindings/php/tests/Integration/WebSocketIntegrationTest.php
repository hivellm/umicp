<?php

declare(strict_types=1);

namespace UMICP\Tests\Integration;

use PHPUnit\Framework\TestCase;
use React\EventLoop\Loop;
use UMICP\Transport\WebSocketClient;
use UMICP\Transport\WebSocketServer;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

/**
 * Integration tests for WebSocket client/server
 *
 * @group integration
 */
class WebSocketIntegrationTest extends TestCase
{
    private const TEST_PORT = 20091;
    private const TEST_TIMEOUT = 5; // seconds

    public function testClientServerCommunication(): void
    {
        $this->markTestSkipped('Requires running event loop - use manual testing');

        // This test demonstrates how to test client/server
        // In practice, use the examples for manual testing
    }

    public function testEnvelopeRoundTrip(): void
    {
        $original = new Envelope(
            from: 'test-client',
            to: 'test-server',
            operation: OperationType::DATA,
            messageId: 'test-msg-001',
            capabilities: [
                'test' => 'true',
                'integration' => 'yes'
            ]
        );

        // Serialize and deserialize
        $json = $original->serialize();
        $received = Envelope::deserialize($json);

        // Verify all fields match
        $this->assertEquals($original->getFrom(), $received->getFrom());
        $this->assertEquals($original->getTo(), $received->getTo());
        $this->assertEquals($original->getOperation(), $received->getOperation());
        $this->assertEquals($original->getMessageId(), $received->getMessageId());
        $this->assertEquals($original->getCapabilities(), $received->getCapabilities());
    }

    public function testMultipleEnvelopeExchange(): void
    {
        $envelopes = [];

        for ($i = 0; $i < 100; $i++) {
            $envelope = new Envelope(
                from: "client-$i",
                to: "server-$i",
                operation: OperationType::DATA,
                messageId: "msg-$i",
                capabilities: ['index' => (string) $i]
            );

            $json = $envelope->serialize();
            $envelopes[] = Envelope::deserialize($json);
        }

        $this->assertCount(100, $envelopes);

        foreach ($envelopes as $i => $envelope) {
            $this->assertEquals("client-$i", $envelope->getFrom());
            $this->assertEquals("server-$i", $envelope->getTo());
            $this->assertEquals((string) $i, $envelope->getCapability('index'));
        }
    }
}

