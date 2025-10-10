<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Transport;

use PHPUnit\Framework\TestCase;
use UMICP\Transport\PeerInfo;

/**
 * @covers \UMICP\Transport\PeerInfo
 */
class PeerInfoTest extends TestCase
{
    public function testPeerInfoCreation(): void
    {
        $peerInfo = new PeerInfo(
            peerId: 'test-peer',
            metadata: ['role' => 'worker'],
            capabilities: ['feature1' => 'enabled']
        );

        $this->assertEquals('test-peer', $peerInfo->peerId);
        $this->assertEquals(['role' => 'worker'], $peerInfo->metadata);
        $this->assertEquals(['feature1' => 'enabled'], $peerInfo->capabilities);
        $this->assertInstanceOf(\DateTime::class, $peerInfo->handshakeCompletedAt);
    }

    public function testToArray(): void
    {
        $peerInfo = new PeerInfo(
            peerId: 'test-peer-2',
            metadata: ['version' => '1.0.0'],
            capabilities: ['compression' => 'true']
        );

        $array = $peerInfo->toArray();

        $this->assertIsArray($array);
        $this->assertEquals('test-peer-2', $array['peer_id']);
        $this->assertArrayHasKey('metadata', $array);
        $this->assertArrayHasKey('capabilities', $array);
        $this->assertArrayHasKey('handshake_completed_at', $array);
    }

    public function testEmptyMetadataAndCapabilities(): void
    {
        $peerInfo = new PeerInfo(peerId: 'minimal-peer');

        $this->assertEquals([], $peerInfo->metadata);
        $this->assertEquals([], $peerInfo->capabilities);
    }
}

