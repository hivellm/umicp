<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Transport;

use PHPUnit\Framework\TestCase;
use React\EventLoop\Loop;
use UMICP\Transport\MultiplexedPeer;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

/**
 * @covers \UMICP\Transport\MultiplexedPeer
 */
class MultiplexedPeerTest extends TestCase
{
    public function testPeerCreationWithoutServer(): void
    {
        $loop = Loop::get();

        $peer = new MultiplexedPeer(
            peerId: 'test-peer',
            loop: $loop,
            serverConfig: null
        );

        $stats = $peer->getStats();

        $this->assertEquals('test-peer', $stats['peer_id']);
        $this->assertEquals(0, $stats['total_peers']);
        $this->assertFalse($stats['server_active']);
    }

    public function testPeerCreationWithServer(): void
    {
        $loop = Loop::get();

        $peer = new MultiplexedPeer(
            peerId: 'test-peer-server',
            loop: $loop,
            serverConfig: ['port' => 30001, 'host' => '127.0.0.1']
        );

        $stats = $peer->getStats();

        $this->assertEquals('test-peer-server', $stats['peer_id']);
        $this->assertTrue($stats['server_active']);
    }

    public function testPeerWithMetadata(): void
    {
        $loop = Loop::get();

        $metadata = [
            'role' => 'coordinator',
            'version' => '1.0.0',
            'capabilities' => 'nlp,vision'
        ];

        $peer = new MultiplexedPeer(
            peerId: 'meta-peer',
            loop: $loop,
            serverConfig: null,
            metadata: $metadata
        );

        // Metadata is internal, verify via stats
        $stats = $peer->getStats();
        $this->assertEquals('meta-peer', $stats['peer_id']);
    }

    public function testGetPeersByType(): void
    {
        $loop = Loop::get();

        $peer = new MultiplexedPeer('test', $loop);

        $incoming = $peer->getPeersByType('incoming');
        $outgoing = $peer->getPeersByType('outgoing');

        $this->assertIsArray($incoming);
        $this->assertIsArray($outgoing);
        $this->assertEmpty($incoming);
        $this->assertEmpty($outgoing);
    }

    public function testGetPeerWithNonexistentId(): void
    {
        $loop = Loop::get();
        $peer = new MultiplexedPeer('test', $loop);

        $result = $peer->getPeer('nonexistent-id');

        $this->assertNull($result);
    }

    public function testFindPeerByMetadataNoMatch(): void
    {
        $loop = Loop::get();
        $peer = new MultiplexedPeer('test', $loop);

        $result = $peer->findPeerByMetadata('role', 'worker');

        $this->assertNull($result);
    }

    public function testDisconnectNonexistentPeer(): void
    {
        $loop = Loop::get();
        $peer = new MultiplexedPeer('test', $loop);

        $result = $peer->disconnectPeer('nonexistent-id');

        $this->assertFalse($result);
    }
}

