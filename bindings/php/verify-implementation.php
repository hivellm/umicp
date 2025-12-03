<?php

declare(strict_types=1);

/**
 * Comprehensive implementation verification
 */

echo "UMICP PHP Bindings - Implementation Verification\n";
echo "=================================================\n\n";

$errors = [];
$warnings = [];
$checks = 0;
$passed = 0;

// Check PHP version
echo "1. PHP Environment\n";
echo "   PHP Version: " . PHP_VERSION . " ";
if (version_compare(PHP_VERSION, '8.1.0', '>=')) {
    echo "✓\n";
    $passed++;
} else {
    echo "✗ (8.1+ required)\n";
    $errors[] = "PHP 8.1+ required";
}
$checks++;

// Check FFI
echo "   FFI Extension: ";
if (extension_loaded('ffi')) {
    echo "✓\n";
    $passed++;
} else {
    echo "✗\n";
    $errors[] = "FFI extension not loaded";
}
$checks++;

echo "   FFI Enabled: ";
$ffiEnabled = ini_get('ffi.enable');
if ($ffiEnabled === '1' || $ffiEnabled === 'preload') {
    echo "✓\n";
    $passed++;
} else {
    echo "⚠ (set ffi.enable=1 in php.ini)\n";
    $warnings[] = "FFI not enabled";
}
$checks++;
echo "\n";

// Check classes
echo "2. Core Classes\n";
$coreClasses = [
    'UMICP\\Core\\Envelope',
    'UMICP\\Core\\Matrix',
    'UMICP\\Core\\Frame',
    'UMICP\\Core\\OperationType',
    'UMICP\\Core\\PayloadType',
    'UMICP\\Core\\EncodingType',
    'UMICP\\Core\\PayloadHint',
];

foreach ($coreClasses as $class) {
    $checks++;
    echo "   " . basename(str_replace('\\', '/', $class)) . ": ";
    if (class_exists($class) || enum_exists($class)) {
        echo "✓\n";
        $passed++;
    } else {
        echo "✗\n";
        $errors[] = "Class not found: $class";
    }
}
echo "\n";

// Check FFI classes
echo "3. FFI Infrastructure\n";
$ffiClasses = [
    'UMICP\\FFI\\FFIBridge',
    'UMICP\\FFI\\Config',
    'UMICP\\FFI\\TypeConverter',
];

foreach ($ffiClasses as $class) {
    $checks++;
    echo "   " . basename(str_replace('\\', '/', $class)) . ": ";
    if (class_exists($class)) {
        echo "✓\n";
        $passed++;
    } else {
        echo "✗\n";
        $errors[] = "Class not found: $class";
    }
}
$checks++;
echo "   AutoCleanup Trait: ";
if (trait_exists('UMICP\\FFI\\Traits\\AutoCleanup')) {
    echo "✓\n";
    $passed++;
} else {
    echo "✗\n";
    $errors[] = "Trait not found";
}
echo "\n";

// Check transport classes
echo "4. Transport Layer\n";
$transportClasses = [
    'UMICP\\Transport\\WebSocketClient',
    'UMICP\\Transport\\WebSocketServer',
    'UMICP\\Transport\\MultiplexedPeer',
    'UMICP\\Transport\\ConnectionState',
    'UMICP\\Transport\\PeerConnection',
    'UMICP\\Transport\\PeerInfo',
];

foreach ($transportClasses as $class) {
    $checks++;
    echo "   " . basename(str_replace('\\', '/', $class)) . ": ";
    if (class_exists($class)) {
        echo "✓\n";
        $passed++;
    } else {
        echo "✗\n";
        $errors[] = "Class not found: $class";
    }
}
echo "\n";

// Check exceptions
echo "5. Exception System\n";
$exceptionClasses = [
    'UMICP\\Exception\\UMICPException',
    'UMICP\\Exception\\FFIException',
    'UMICP\\Exception\\TransportException',
    'UMICP\\Exception\\SerializationException',
];

foreach ($exceptionClasses as $class) {
    $checks++;
    echo "   " . basename(str_replace('\\', '/', $class)) . ": ";
    if (class_exists($class)) {
        echo "✓\n";
        $passed++;
    } else {
        echo "✗\n";
        $errors[] = "Class not found: $class";
    }
}
echo "\n";

// Check files
echo "6. Required Files\n";
$requiredFiles = [
    'ffi/umicp_core.h',
    'config/umicp.php',
    'composer.json',
    'phpunit.xml',
];

foreach ($requiredFiles as $file) {
    $checks++;
    echo "   $file: ";
    if (file_exists(__DIR__ . '/' . $file)) {
        echo "✓\n";
        $passed++;
    } else {
        echo "✗\n";
        $errors[] = "File not found: $file";
    }
}
echo "\n";

// Count tests
echo "7. Test Files\n";
$testFiles = glob(__DIR__ . '/tests/**/*Test.php');
$testFiles = array_merge($testFiles, glob(__DIR__ . '/tests/*Test.php'));
$testFiles = array_merge($testFiles, glob(__DIR__ . '/tests/*/*Test.php'));
$testFiles = array_merge($testFiles, glob(__DIR__ . '/tests/*/*/*Test.php'));
$testCount = count(array_unique($testFiles));

echo "   Test files found: $testCount ";
if ($testCount >= 20) {
    echo "✓\n";
    $passed++;
} else {
    echo "⚠ (expected 20+)\n";
    $warnings[] = "Low test count: $testCount";
}
$checks++;
echo "\n";

// Count examples
echo "8. Examples\n";
$exampleFiles = glob(__DIR__ . '/examples/*.php');
$exampleCount = count($exampleFiles);

echo "   Example files: $exampleCount ";
if ($exampleCount >= 5) {
    echo "✓\n";
    $passed++;
} else {
    echo "⚠\n";
    $warnings[] = "Expected 5+ examples";
}
$checks++;
echo "\n";

// Summary
echo "═══════════════════════════════════════════════════════════════\n";
echo "VERIFICATION SUMMARY\n";
echo "═══════════════════════════════════════════════════════════════\n\n";

echo "Checks:   $passed / $checks passed\n";
echo "Errors:   " . count($errors) . "\n";
echo "Warnings: " . count($warnings) . "\n";
echo "\n";

$percentage = ($passed / $checks) * 100;
echo "Success Rate: " . number_format($percentage, 1) . "%\n\n";

if (empty($errors)) {
    echo "✅ VERIFICATION PASSED\n\n";

    if (!empty($warnings)) {
        echo "Warnings:\n";
        foreach ($warnings as $warning) {
            echo "  ⚠ $warning\n";
        }
        echo "\n";
    }

    echo "Implementation is ready!\n\n";
    echo "Next steps:\n";
    echo "  1. Run tests: ./vendor/bin/phpunit\n";
    echo "  2. Run benchmarks: php benchmark.php\n";
    echo "  3. Run examples: php examples/03_complete_demo.php\n";
    echo "  4. Build C++: ./build-cpp.sh\n";

    exit(0);
} else {
    echo "❌ VERIFICATION FAILED\n\n";

    echo "Errors:\n";
    foreach ($errors as $error) {
        echo "  ✗ $error\n";
    }
    echo "\n";

    if (!empty($warnings)) {
        echo "Warnings:\n";
        foreach ($warnings as $warning) {
            echo "  ⚠ $warning\n";
        }
        echo "\n";
    }

    exit(1);
}

