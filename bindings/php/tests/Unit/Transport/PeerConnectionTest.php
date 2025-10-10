<?php

declare(strict_types=1);

namespace UMICP\Tests\Unit\Transport;

use PHPUnit\Framework\TestCase;
use UMICP\Transport\PeerConnection;
use UMICP\Transport\PeerInfo;
use DateTime;

/**
 * @covers \UMICP\Transport\PeerConnection
 */
class PeerConnectionTest extends TestCase
{
    public function testPeerConnectionCreation(): void
    {
        $mockClient = new \stdClass();

        $peer = new PeerConnection(
            id: 'peer-123',
            type: 'outgoing',
            url: 'ws://localhost:8080',
            client: $mockClient,
            metadata: ['role' => 'worker']
        );

        $this->assertEquals('peer-123', $peer->id);
        $this->assertEquals('outgoing', $peer->type);
        $this->assertEquals('ws://localhost:8080', $peer->url);
        $this->assertSame($mockClient, $peer->client);
        $this->assertEquals(['role' => 'worker'], $peer->metadata);
        $this->assertFalse($peer->handshakeComplete);
        $this->assertNull($peer->peerInfo);
    }

    public function testPeerConnectionWithPeerInfo(): void
    {
        $peerInfo = new PeerInfo(
            peerId: 'remote-peer',
            metadata: ['version' => '1.0.0'],
            capabilities: ['feature' => 'test']
        );

        $peer = new PeerConnection(
            id: 'peer-456',
            type: 'incoming',
            url: null,
            client: new \stdClass(),
            handshakeComplete: true,
            peerInfo: $peerInfo
        );

        $this->assertTrue($peer->handshakeComplete);
        $this->assertSame($peerInfo, $peer->peerInfo);
    }

    public function testToArray(): void
    {
        $peer = new PeerConnection(
            id: 'peer-789',
            type: 'outgoing',
            url: 'ws://localhost:8080',
            client: new \stdClass(),
            metadata: ['key' => 'value']
        );

        $array = $peer->toArray();

        $this->assertIsArray($array);
        $this->assertEquals('peer-789', $array['id']);
        $this->assertEquals('outgoing', $array['type']);
        $this->assertEquals('ws://localhost:8080', $array['url']);
        $this->assertArrayHasKey('metadata', $array);
        $this->assertArrayHasKey('connected_at', $array);
        $this->assertFalse($array['handshake_complete']);
    }
}

