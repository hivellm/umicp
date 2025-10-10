<?php

require_once __DIR__ . '/../vendor/autoload.php';

use UMICP\Core\Envelope;
use UMICP\Core\OperationType;
use UMICP\Core\CompressionManager;
use UMICP\Core\EventEmitter;
use UMICP\Transport\HttpClient;
use UMICP\Transport\HttpServer;

echo "═══════════════════════════════════════════════════════════\n";
echo " UMICP PHP - HTTP Transport, Compression & Events Demo\n";
echo "═══════════════════════════════════════════════════════════\n\n";

// ============================================================================
// 1. Event System Demo
// ============================================================================

echo "1️⃣  Event System Demo\n";
echo "────────────────────────────────────────────────────────────\n";

$emitter = new EventEmitter();

// Register event listeners
$emitter->on('message', function($data) {
    echo "   📨 Message event: " . json_encode($data) . "\n";
});

$emitter->once('connect', function() {
    echo "   🔌 Connected! (fires once only)\n";
});

$emitter->on('error', function($error) {
    echo "   ❌ Error: $error\n";
});

// Emit events
$emitter->emit('connect');
$emitter->emit('connect'); // Won't fire (once listener)
$emitter->emit('message', ['from' => 'peer-1', 'data' => 'Hello!']);
$emitter->emit('message', ['from' => 'peer-2', 'data' => 'Hi there!']);

echo "   📊 Event stats: " . json_encode($emitter->getStats()) . "\n\n";

// ============================================================================
// 2. Compression Demo
// ============================================================================

echo "2️⃣  Compression Demo\n";
echo "────────────────────────────────────────────────────────────\n";

// Test data - repetitive for good compression
$testData = str_repeat("UMICP is awesome! ", 100);
echo "   📏 Original size: " . strlen($testData) . " bytes\n";

// GZIP Compression
$gzipCompressor = new CompressionManager(CompressionManager::ALGORITHM_GZIP, 9);
$compressed = $gzipCompressor->compress($testData);
echo "   📦 GZIP compressed: " . strlen($compressed) . " bytes\n";
echo "   📉 Compression ratio: " .
    round(CompressionManager::getCompressionRatio($testData, $compressed) * 100, 2) . "%\n";

$decompressed = $gzipCompressor->decompress($compressed);
echo "   ✅ Decompression successful: " . ($testData === $decompressed ? 'YES' : 'NO') . "\n\n";

// DEFLATE Compression
$deflateCompressor = new CompressionManager(CompressionManager::ALGORITHM_DEFLATE, 6);
$compressed2 = $deflateCompressor->compress($testData);
echo "   📦 DEFLATE compressed: " . strlen($compressed2) . " bytes\n";
echo "   📉 Compression ratio: " .
    round(CompressionManager::getCompressionRatio($testData, $compressed2) * 100, 2) . "%\n\n";

// ============================================================================
// 3. HTTP Transport Demo
// ============================================================================

echo "3️⃣  HTTP Transport Demo\n";
echo "────────────────────────────────────────────────────────────\n";

// Create HTTP Client
$httpClient = new HttpClient([
    'baseUrl' => 'http://localhost:9080',
    'path' => '/umicp',
    'timeout' => 10,
]);

echo "   🌐 HTTP Client created\n";
echo "   📍 Base URL: http://localhost:9080/umicp\n";

// Create HTTP Server (for demonstration)
$httpServer = new HttpServer([
    'host' => '0.0.0.0',
    'port' => 9080,
    'path' => '/umicp',
]);

$httpServer->onMessage(function(Envelope $envelope): Envelope {
    echo "   📨 Server received message from: {$envelope->from}\n";

    // Echo back with ACK
    return Envelope::builder()
        ->from('http-server')
        ->to($envelope->from)
        ->operation(OperationType::ACK)
        ->payload(['received' => true, 'timestamp' => time()])
        ->build();
});

echo "   🖥️  HTTP Server configured\n";
echo "   ✅ Message handler registered\n\n";

// Show stats
echo "   📊 Client stats: " . json_encode($httpClient->getStats()) . "\n";
echo "   📊 Server stats: " . json_encode($httpServer->getStats()) . "\n\n";

// ============================================================================
// 4. Combining All Features
// ============================================================================

echo "4️⃣  Combined Features Demo\n";
echo "────────────────────────────────────────────────────────────\n";

// Event emitter for lifecycle events
$lifecycle = new EventEmitter();

$lifecycle->on('envelope:created', function($envelope) {
    echo "   ✨ Envelope created: {$envelope->messageId}\n";
});

$lifecycle->on('envelope:compressed', function($original, $compressed) {
    $ratio = round((strlen($compressed) / strlen($original)) * 100, 2);
    echo "   🗜️  Data compressed: $ratio%\n";
});

$lifecycle->on('envelope:sent', function($envelope) {
    echo "   📤 Envelope sent to: {$envelope->to}\n";
});

// Create envelope
$envelope = Envelope::builder()
    ->from('php-client')
    ->to('http-server')
    ->operation(OperationType::DATA)
    ->payload(['message' => str_repeat('Test data ', 50)])
    ->build();

$lifecycle->emit('envelope:created', $envelope);

// Compress envelope payload
$payload = json_encode($envelope->payload);
$compressor = new CompressionManager();
$compressedPayload = $compressor->compress($payload);

$lifecycle->emit('envelope:compressed', $payload, $compressedPayload);

// Simulate sending (would use httpClient in real scenario)
$lifecycle->emit('envelope:sent', $envelope);

echo "\n";

// ============================================================================
// Summary
// ============================================================================

echo "═══════════════════════════════════════════════════════════\n";
echo " ✅ Demo Complete!\n";
echo "═══════════════════════════════════════════════════════════\n";
echo "\n";
echo "Features Demonstrated:\n";
echo "  ✅ Event System (EventEmitter pattern)\n";
echo "  ✅ Compression (GZIP, DEFLATE)\n";
echo "  ✅ HTTP Transport (Client & Server)\n";
echo "  ✅ Envelope creation & management\n";
echo "  ✅ Statistics tracking\n";
echo "\n";
echo "PHP UMICP is now feature-complete! 🎉\n";
echo "\n";

