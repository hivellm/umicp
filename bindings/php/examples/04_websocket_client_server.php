<?php

declare(strict_types=1);

require_once __DIR__ . '/../vendor/autoload.php';

use React\EventLoop\Loop;
use UMICP\Transport\WebSocketClient;
use UMICP\Transport\WebSocketServer;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

echo "UMICP PHP - WebSocket Client/Server Example\n";
echo "============================================\n\n";

$loop = Loop::get();

// Create server
echo "1. Creating WebSocket Server...\n";
$server = new WebSocketServer($loop, [
    'port' => 20081,
    'host' => '0.0.0.0',
    'path' => '/umicp'
]);

$server->on('listening', function ($host, $port) {
    echo "   ✓ Server listening on $host:$port\n\n";
});

$server->on('client_connected', function ($client) {
    echo "   ✓ Client connected: {$client['id']}\n";
});

$server->on('message', function (Envelope $envelope, $client) use ($server) {
    $caps = $envelope->getCapabilities();
    echo "   📨 Message from {$client['id']}: " . json_encode($caps) . "\n";

    // Echo response
    $response = new Envelope(
        from: 'server',
        to: $envelope->getFrom(),
        operation: OperationType::ACK,
        messageId: 'ack-' . uniqid(),
        capabilities: [
            'status' => 'received',
            'original_message' => $envelope->getMessageId()
        ]
    );

    $server->sendToClient($client['id'], $response);
    echo "   📤 Sent ACK to {$client['id']}\n";
});

$server->on('client_disconnected', function ($client) {
    echo "   ✗ Client disconnected: {$client['id']}\n";
});

// Start server
$server->start();

// Wait for server to start
$loop->addTimer(1, function () use ($loop) {
    echo "2. Creating WebSocket Client...\n";

    $client = new WebSocketClient($loop, [
        'url' => 'ws://localhost:20081/umicp'
    ]);

    $client->on('connected', function () use ($client) {
        echo "   ✓ Client connected\n\n";

        // Send test messages
        echo "3. Sending test messages...\n";

        $messages = [
            ['message' => 'Hello Server!', 'sequence' => '1'],
            ['message' => 'How are you?', 'sequence' => '2'],
            ['message' => 'Testing UMICP!', 'sequence' => '3'],
        ];

        foreach ($messages as $msg) {
            $envelope = new Envelope(
                from: 'php-client',
                to: 'server',
                operation: OperationType::DATA,
                messageId: 'msg-' . uniqid(),
                capabilities: $msg
            );

            if ($client->send($envelope)) {
                echo "   📤 Sent: {$msg['message']}\n";
            }
        }
    });

    $client->on('message', function (Envelope $envelope) {
        $caps = $envelope->getCapabilities();
        echo "   📨 Response: " . json_encode($caps) . "\n";
    });

    $client->on('error', function ($error) {
        echo "   ❌ Client error: {$error->getMessage()}\n";
    });

    $client->connect();
});

// Stop after 10 seconds
$loop->addTimer(10, function () use ($loop) {
    echo "\n4. Shutting down...\n";
    echo "   ✓ Example complete\n";
    $loop->stop();
});

echo "Starting event loop...\n\n";
Loop::run();

