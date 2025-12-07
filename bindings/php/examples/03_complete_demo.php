<?php

declare(strict_types=1);

require_once __DIR__ . '/../vendor/autoload.php';

use UMICP\Core\Envelope;
use UMICP\Core\Matrix;
use UMICP\Core\Frame;
use UMICP\Core\OperationType;
use UMICP\Core\PayloadType;
use UMICP\Core\EncodingType;
use UMICP\Core\PayloadHint;
use UMICP\FFI\FFIBridge;
use UMICP\FFI\Config;

echo "╔════════════════════════════════════════════════════════════╗\n";
echo "║     UMICP PHP Bindings - Complete Demonstration           ║\n";
echo "╚════════════════════════════════════════════════════════════╝\n\n";

try {
    // ========================================================================
    // 1. FFI Initialization
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "1. FFI Bridge Initialization\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    $ffi = FFIBridge::getInstance();
    $info = $ffi->getInfo();

    echo "Library Path:  {$info['lib_path']}\n";
    echo "Header Path:   {$info['header_path']}\n";
    echo "FFI Version:   {$info['ffi_version']}\n";
    echo "PHP Version:   {$info['php_version']}\n";
    echo "UMICP Version: {$info['umicp_version']}\n";
    echo "Build Info:    {$info['build_info']}\n";
    echo "\n✓ FFI initialized successfully\n\n";

    // ========================================================================
    // 2. Envelope Operations
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "2. Envelope Operations\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    // Create envelope
    $envelope = new Envelope(
        from: 'php-client-001',
        to: 'server-001',
        operation: OperationType::DATA,
        messageId: 'msg-' . uniqid(),
        capabilities: [
            'content-type' => 'application/json',
            'priority' => 'high',
            'timestamp' => (string) time(),
            'version' => '1.0.0'
        ]
    );

    echo "Created Envelope:\n";
    echo "  From:       {$envelope->getFrom()}\n";
    echo "  To:         {$envelope->getTo()}\n";
    echo "  Operation:  {$envelope->getOperation()->name}\n";
    echo "  Message ID: {$envelope->getMessageId()}\n";
    echo "  Capabilities: " . count($envelope->getCapabilities()) . " items\n\n";

    // Serialization
    $startTime = microtime(true);
    $json = $envelope->serialize();
    $serializeTime = (microtime(true) - $startTime) * 1000;

    echo "Serialization:\n";
    echo "  Time:  " . number_format($serializeTime, 2) . "ms\n";
    echo "  Size:  " . strlen($json) . " bytes\n";
    echo "  JSON:  " . substr($json, 0, 80) . "...\n\n";

    // Deserialization
    $startTime = microtime(true);
    $received = Envelope::deserialize($json);
    $deserializeTime = (microtime(true) - $startTime) * 1000;

    echo "Deserialization:\n";
    echo "  Time:  " . number_format($deserializeTime, 2) . "ms\n";
    echo "  Match: " . ($received->getFrom() === $envelope->getFrom() ? 'Yes' : 'No') . "\n\n";

    // Validation
    $isValid = $envelope->validate();
    echo "Validation: " . ($isValid ? '✓ Valid' : '✗ Invalid') . "\n\n";

    // Hash
    $hash = $envelope->getHash();
    echo "Hash: " . substr($hash, 0, 16) . "...\n\n";

    // ========================================================================
    // 3. Matrix Operations
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "3. Matrix Operations\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    $matrix = new Matrix();

    $vec1 = [1.0, 2.0, 3.0, 4.0];
    $vec2 = [5.0, 6.0, 7.0, 8.0];

    echo "Test Vectors:\n";
    echo "  Vec1: [" . implode(', ', $vec1) . "]\n";
    echo "  Vec2: [" . implode(', ', $vec2) . "]\n\n";

    // Dot product
    $startTime = microtime(true);
    $dotProduct = $matrix->dotProduct($vec1, $vec2);
    $time = (microtime(true) - $startTime) * 1000;
    echo "Dot Product:        $dotProduct (in " . number_format($time, 3) . "ms)\n";

    // Cosine similarity
    $startTime = microtime(true);
    $similarity = $matrix->cosineSimilarity($vec1, $vec2);
    $time = (microtime(true) - $startTime) * 1000;
    echo "Cosine Similarity:  " . number_format($similarity, 6) . " (in " . number_format($time, 3) . "ms)\n";

    // Vector operations
    $sum = $matrix->vectorAdd($vec1, $vec2);
    echo "Vector Add:         [" . implode(', ', $sum) . "]\n";

    $diff = $matrix->vectorSubtract($vec2, $vec1);
    echo "Vector Subtract:    [" . implode(', ', $diff) . "]\n";

    $scaled = $matrix->vectorScale($vec1, 2.0);
    echo "Vector Scale (x2):  [" . implode(', ', $scaled) . "]\n";

    $magnitude = $matrix->vectorMagnitude($vec1);
    echo "Vector Magnitude:   " . number_format($magnitude, 6) . "\n";

    $normalized = $matrix->vectorNormalize($vec1);
    echo "Vector Normalized:  [" . implode(', ', array_map(fn($v) => number_format($v, 4), $normalized)) . "]\n";

    echo "\n";

    // Matrix operations
    echo "Matrix Operations:\n";
    $matA = [1, 2, 3, 4]; // 2x2
    $matB = [5, 6, 7, 8]; // 2x2

    $result = $matrix->matrixMultiply($matA, $matB, 2, 2, 2);
    echo "  Multiply (2x2):   [[{$result[0]}, {$result[1]}], [{$result[2]}, {$result[3]}]]\n";

    $matC = [1, 2, 3, 4, 5, 6]; // 2x3
    $transposed = $matrix->matrixTranspose($matC, 2, 3);
    echo "  Transpose (2x3):  Result 3x2 matrix\n";

    echo "\n";

    // ========================================================================
    // 4. Frame Operations
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "4. Frame Operations\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    $frame = new Frame(
        type: 1,
        streamId: 100,
        sequence: 42,
        flags: 0,
        compressed: true,
        encrypted: false
    );

    echo "Created Frame:\n";
    echo "  Type:       {$frame->getType()}\n";
    echo "  Stream ID:  {$frame->getStreamId()}\n";
    echo "  Sequence:   {$frame->getSequence()}\n";
    echo "  Flags:      {$frame->getFlags()}\n";
    echo "  Compressed: " . ($frame->isCompressed() ? 'Yes' : 'No') . "\n";
    echo "  Encrypted:  " . ($frame->isEncrypted() ? 'Yes' : 'No') . "\n\n";

    $frameJson = $frame->serialize();
    echo "Serialized: " . substr($frameJson, 0, 60) . "...\n\n";

    // ========================================================================
    // 5. Configuration
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "5. Configuration System\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    echo "Config values:\n";
    echo "  FFI lib path: " . Config::get('ffi.lib_path') . "\n";
    echo "  Default port: " . Config::get('server.default_port') . "\n";
    echo "  Compression:  " . (Config::get('transport.compression') ? 'Enabled' : 'Disabled') . "\n";
    echo "  Timeout:      " . Config::get('transport.default_timeout') . "ms\n\n";

    // ========================================================================
    // 6. Enums Demonstration
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "6. Enums (PHP 8.1+)\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    echo "OperationType:\n";
    foreach (OperationType::cases() as $op) {
        echo "  - {$op->name} = {$op->value}\n";
    }
    echo "\n";

    echo "PayloadType:\n";
    foreach (PayloadType::cases() as $type) {
        echo "  - {$type->name} = {$type->value}\n";
    }
    echo "\n";

    echo "EncodingType:\n";
    foreach (EncodingType::cases() as $enc) {
        echo "  - {$enc->name} = {$enc->value} ({$enc->getSize()} bytes)\n";
    }
    echo "\n";

    // ========================================================================
    // 7. Performance Summary
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "7. Performance Summary\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    echo "Envelope Operations:\n";
    echo "  Serialization:   " . number_format($serializeTime, 2) . "ms\n";
    echo "  Deserialization: " . number_format($deserializeTime, 2) . "ms\n";
    echo "  Size:            " . strlen($json) . " bytes\n\n";

    // Benchmark matrix operations
    $iterations = 1000;
    $vec1 = array_fill(0, 100, 1.0);
    $vec2 = array_fill(0, 100, 2.0);

    $startTime = microtime(true);
    for ($i = 0; $i < $iterations; $i++) {
        $matrix->dotProduct($vec1, $vec2);
    }
    $avgTime = ((microtime(true) - $startTime) / $iterations) * 1000;

    echo "Matrix Operations (100-dim vectors, $iterations iterations):\n";
    echo "  Avg Dot Product: " . number_format($avgTime, 3) . "ms\n";
    echo "  Throughput:      " . number_format(1000 / $avgTime, 0) . " ops/sec\n\n";

    // ========================================================================
    // 8. Use Cases
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "8. Real-World Use Cases\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    // Use case 1: Federated Learning
    echo "Use Case 1: Federated Learning\n";
    $weightsEnvelope = new Envelope(
        from: 'fl-coordinator',
        to: 'worker-001',
        operation: OperationType::DATA,
        capabilities: [
            'action' => 'update_weights',
            'epoch' => '5',
            'weights' => json_encode([0.5, 0.8, 0.3, 0.9])
        ]
    );
    echo "  ✓ Model weights envelope created\n";
    echo "  Action: {$weightsEnvelope->getCapability('action')}\n\n";

    // Use case 2: Similarity search
    echo "Use Case 2: Vector Similarity Search\n";
    $embedding1 = array_fill(0, 128, 0.5);
    $embedding2 = array_fill(0, 128, 0.6);
    $similarity = $matrix->cosineSimilarity($embedding1, $embedding2);
    echo "  ✓ Embedding similarity: " . number_format($similarity, 4) . "\n\n";

    // Use case 3: Agent communication
    echo "Use Case 3: Agent-to-Agent Communication\n";
    $agentMsg = new Envelope(
        from: 'agent-alpha',
        to: 'agent-beta',
        operation: OperationType::REQUEST,
        capabilities: [
            'action' => 'collaborate',
            'task' => 'analyze_data',
            'priority' => 'urgent'
        ]
    );
    echo "  ✓ Agent message created\n";
    echo "  Task: {$agentMsg->getCapability('task')}\n\n";

    // ========================================================================
    // Final Summary
    // ========================================================================

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    echo "✅ DEMONSTRATION COMPLETE\n";
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    echo "Summary:\n";
    echo "  ✓ FFI Bridge:        Working\n";
    echo "  ✓ Envelope:          Working\n";
    echo "  ✓ Matrix:            Working\n";
    echo "  ✓ Frame:             Working\n";
    echo "  ✓ Configuration:     Working\n";
    echo "  ✓ Enums:             Working\n\n";

    echo "Next steps:\n";
    echo "  • Implement WebSocket transport\n";
    echo "  • Add MultiplexedPeer\n";
    echo "  • Create unit tests\n";
    echo "  • Performance benchmarks\n\n";

} catch (\UMICP\Exception\FFIException $e) {
    echo "\n❌ FFI Error: " . $e->getMessage() . "\n";
    echo "Library: " . ($e->getLibraryPath() ?? 'unknown') . "\n";
    echo "FFI Error: " . ($e->getFFIError() ?? 'none') . "\n\n";

    echo "Possible solutions:\n";
    echo "  1. Build C++ core: ./build-cpp.sh\n";
    echo "  2. Update config: config/umicp.php\n";
    echo "  3. Check FFI enabled: php -m | grep FFI\n";
    exit(1);

} catch (\Throwable $e) {
    echo "\n❌ Error: " . $e->getMessage() . "\n";
    echo "File: " . $e->getFile() . ":" . $e->getLine() . "\n";
    if ($e->getPrevious()) {
        echo "Previous: " . $e->getPrevious()->getMessage() . "\n";
    }
    exit(1);
}

