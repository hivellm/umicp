<?php

declare(strict_types=1);

require_once __DIR__ . '/../vendor/autoload.php';

use UMICP\Core\Envelope;
use UMICP\Core\OperationType;
use UMICP\FFI\FFIBridge;

echo "UMICP PHP Bindings - Basic Envelope Example\n";
echo "============================================\n\n";

try {
    // Initialize FFI (will load config from config/umicp.php)
    echo "1. Initializing FFI Bridge...\n";
    $ffi = FFIBridge::getInstance();
    echo "   ✓ FFI initialized successfully\n";
    print_r($ffi->getInfo());
    echo "\n";

    // Create an envelope
    echo "2. Creating Envelope...\n";
    $envelope = new Envelope(
        from: 'php-client-001',
        to: 'server-001',
        operation: OperationType::DATA,
        messageId: 'msg-' . uniqid(),
        capabilities: [
            'content-type' => 'application/json',
            'priority' => 'high',
            'timestamp' => (string) time()
        ]
    );
    echo "   ✓ Envelope created\n";
    echo "   From: {$envelope->getFrom()}\n";
    echo "   To: {$envelope->getTo()}\n";
    echo "   Operation: {$envelope->getOperation()->name}\n";
    echo "   Message ID: {$envelope->getMessageId()}\n";
    echo "\n";

    // Serialize envelope
    echo "3. Serializing Envelope...\n";
    $startTime = microtime(true);
    $json = $envelope->serialize();
    $serializeTime = (microtime(true) - $startTime) * 1000;
    echo "   ✓ Serialized in " . number_format($serializeTime, 2) . "ms\n";
    echo "   JSON: " . substr($json, 0, 100) . "...\n";
    echo "   Size: " . strlen($json) . " bytes\n";
    echo "\n";

    // Deserialize envelope
    echo "4. Deserializing Envelope...\n";
    $startTime = microtime(true);
    $deserialized = Envelope::deserialize($json);
    $deserializeTime = (microtime(true) - $startTime) * 1000;
    echo "   ✓ Deserialized in " . number_format($deserializeTime, 2) . "ms\n";
    echo "   From: {$deserialized->getFrom()}\n";
    echo "   To: {$deserialized->getTo()}\n";
    echo "\n";

    // Validate envelope
    echo "5. Validating Envelope...\n";
    $isValid = $envelope->validate();
    echo "   ✓ Envelope is " . ($isValid ? 'valid' : 'invalid') . "\n";
    echo "\n";

    // Get envelope hash
    echo "6. Getting Envelope Hash...\n";
    $hash = $envelope->getHash();
    echo "   Hash: $hash\n";
    echo "\n";

    // Modify capabilities
    echo "7. Modifying Capabilities...\n";
    $envelope->setCapability('status', 'processed');
    $envelope->setCapability('processed_at', (string) time());
    echo "   ✓ Capabilities updated\n";
    $capabilities = $envelope->getCapabilities();
    echo "   Total capabilities: " . count($capabilities) . "\n";
    echo "\n";

    // Convert to array
    echo "8. Converting to Array...\n";
    $array = $envelope->toArray();
    echo "   ✓ Converted to array\n";
    echo "   Array keys: " . implode(', ', array_keys($array)) . "\n";
    echo "\n";

    echo "✅ All operations completed successfully!\n";
    echo "\n";
    echo "Performance Summary:\n";
    echo "  Serialization:   " . number_format($serializeTime, 2) . "ms\n";
    echo "  Deserialization: " . number_format($deserializeTime, 2) . "ms\n";

} catch (\Throwable $e) {
    echo "❌ Error: " . $e->getMessage() . "\n";
    echo "   File: " . $e->getFile() . ":" . $e->getLine() . "\n";
    if ($e->getPrevious()) {
        echo "   Previous: " . $e->getPrevious()->getMessage() . "\n";
    }
    exit(1);
}

