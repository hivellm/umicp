<?php

declare(strict_types=1);

require_once __DIR__ . '/../vendor/autoload.php';

use React\EventLoop\Loop;
use UMICP\Transport\MultiplexedPeer;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

echo "UMICP PHP - Multiplexed Peer Example\n";
echo "=====================================\n\n";

$loop = Loop::get();

// Create Peer A (server + client)
echo "1. Creating Peer A (port 20081)...\n";
$peerA = new MultiplexedPeer(
    peerId: 'peer-alpha',
    loop: $loop,
    serverConfig: [
        'port' => 20081,
        'path' => '/umicp'
    ],
    metadata: [
        'type' => 'coordinator',
        'version' => '1.0.0'
    ]
);

$peerA->on('server:ready', function () {
    echo "   ✓ Peer A server ready on port 20081\n\n";
});

$peerA->on('peer:connect', function ($peer) {
    echo "   🔌 Peer A: Connection from {$peer->id} ({$peer->type})\n";
});

$peerA->on('peer:ready', function ($peer, $peerInfo) {
    echo "   ✅ Peer A: Handshake complete with {$peerInfo->peerId}\n";
    echo "      Metadata: " . json_encode($peerInfo->metadata) . "\n";
});

$peerA->on('data', function (Envelope $envelope, $peer) use ($peerA) {
    $caps = $envelope->getCapabilities();
    echo "   📨 Peer A received: " . json_encode($caps) . "\n";

    // Respond
    $response = new Envelope(
        from: 'peer-alpha',
        to: $envelope->getFrom(),
        operation: OperationType::ACK,
        messageId: 'resp-' . uniqid(),
        capabilities: [
            'status' => 'processed',
            'original' => $envelope->getMessageId()
        ]
    );

    $peerA->sendToPeer($peer->id, $response);
    echo "   📤 Peer A sent response\n";
});

// Create Peer B after A is ready
$loop->addTimer(2, function () use ($loop) {
    echo "2. Creating Peer B...\n";

    $peerB = new MultiplexedPeer(
        peerId: 'peer-beta',
        loop: $loop,
        serverConfig: [
            'port' => 20082,
            'path' => '/umicp'
        ],
        metadata: [
            'type' => 'worker',
            'version' => '1.0.0'
        ]
    );

    $peerB->on('server:ready', function () {
        echo "   ✓ Peer B server ready on port 20082\n\n";
    });

    $peerB->on('peer:ready', function ($peer, $peerInfo) {
        echo "   ✅ Peer B: Handshake complete with {$peerInfo->peerId}\n";
    });

    $peerB->on('data', function (Envelope $envelope, $peer) {
        $caps = $envelope->getCapabilities();
        echo "   📨 Peer B received: " . json_encode($caps) . "\n";
    });

    $peerB->on('ack', function (Envelope $envelope, $peer) {
        $caps = $envelope->getCapabilities();
        echo "   ✅ Peer B received ACK: " . json_encode($caps) . "\n";
    });

    // Connect B to A
    $loop->addTimer(1, function () use ($peerB, $loop) {
        echo "3. Connecting Peer B to Peer A...\n";

        $peerB->connectToPeer('ws://localhost:20081/umicp', [
            'role' => 'worker'
        ])->then(
            function ($peerId) use ($peerB, $loop) {
                echo "   ✓ Peer B connected: $peerId\n\n";

                // Send test message after handshake completes
                $loop->addTimer(2, function () use ($peerB, $peerId) {
                    echo "4. Peer B sending message to Peer A...\n";

                    $envelope = new Envelope(
                        from: 'peer-beta',
                        to: 'peer-alpha',
                        operation: OperationType::DATA,
                        messageId: 'msg-' . uniqid(),
                        capabilities: [
                            'action' => 'hello',
                            'message' => 'Hello from Peer B!',
                            'timestamp' => (string) time()
                        ]
                    );

                    $peerB->sendToPeer($peerId, $envelope);
                });
            },
            function ($error) {
                echo "   ❌ Connection failed: {$error->getMessage()}\n";
            }
        );
    });
});

// Show stats periodically
$loop->addPeriodicTimer(5, function () use ($peerA) {
    echo "\n📊 Peer A Stats:\n";
    $stats = $peerA->getStats();
    echo "   Total Peers: {$stats['total_peers']}\n";
    echo "   Incoming: {$stats['incoming_connections']}\n";
    echo "   Outgoing: {$stats['outgoing_connections']}\n";
});

// Stop after 15 seconds
$loop->addTimer(15, function () use ($loop, $peerA) {
    echo "\n5. Shutting down...\n";

    $peerA->shutdown()->then(
        function () use ($loop) {
            echo "   ✓ Shutdown complete\n";
            $loop->stop();
        }
    );
});

echo "Starting multiplexed peer network...\n\n";
Loop::run();

