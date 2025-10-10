<?php

declare(strict_types=1);

/**
 * Validate UMICP PHP Bindings Structure
 *
 * This script validates that all required files and classes are in place
 */

echo "UMICP PHP Bindings - Structure Validation\n";
echo "==========================================\n\n";

$errors = [];
$warnings = [];
$success = 0;

// Required files
$requiredFiles = [
    // Configuration
    'composer.json' => 'Composer configuration',
    'config/umicp.example.php' => 'Configuration template',
    'ffi/umicp_core.h' => 'FFI header',

    // Core classes
    'src/Core/Envelope.php' => 'Envelope class',
    'src/Core/Matrix.php' => 'Matrix class',
    'src/Core/OperationType.php' => 'OperationType enum',
    'src/Core/PayloadType.php' => 'PayloadType enum',
    'src/Core/EncodingType.php' => 'EncodingType enum',
    'src/Core/PayloadHint.php' => 'PayloadHint class',

    // FFI layer
    'src/FFI/FFIBridge.php' => 'FFI Bridge',
    'src/FFI/Config.php' => 'Config class',
    'src/FFI/TypeConverter.php' => 'Type converter',
    'src/FFI/Traits/AutoCleanup.php' => 'AutoCleanup trait',

    // Exceptions
    'src/Exception/UMICPException.php' => 'Base exception',
    'src/Exception/FFIException.php' => 'FFI exception',
    'src/Exception/TransportException.php' => 'Transport exception',
    'src/Exception/SerializationException.php' => 'Serialization exception',
    'src/Exception/ValidationException.php' => 'Validation exception',
    'src/Exception/ConnectionException.php' => 'Connection exception',
    'src/Exception/TimeoutException.php' => 'Timeout exception',

    // Documentation
    'README.md' => 'Main README',
    'docs/ARCHITECTURE.md' => 'Architecture documentation',
    'docs/IMPLEMENTATION_ROADMAP.md' => 'Implementation roadmap',
    'docs/API_SPECIFICATION.md' => 'API specification',
    'docs/FFI_INTEGRATION_GUIDE.md' => 'FFI integration guide',

    // Examples
    'examples/01_basic_envelope.php' => 'Basic envelope example',
    'examples/02_matrix_operations.php' => 'Matrix operations example',
];

echo "1. Checking Required Files...\n";
foreach ($requiredFiles as $file => $description) {
    $path = __DIR__ . '/' . $file;
    if (file_exists($path)) {
        echo "   ✓ $description\n";
        $success++;
    } else {
        echo "   ✗ $description (missing: $file)\n";
        $errors[] = "Missing file: $file";
    }
}

echo "\n2. Checking Directory Structure...\n";
$requiredDirs = [
    'src/Core',
    'src/FFI',
    'src/FFI/Traits',
    'src/Exception',
    'src/Transport',
    'ffi',
    'config',
    'docs',
    'examples',
    'tests',
];

foreach ($requiredDirs as $dir) {
    $path = __DIR__ . '/' . $dir;
    if (is_dir($path)) {
        echo "   ✓ $dir/\n";
    } else {
        echo "   ✗ $dir/ (missing)\n";
        $errors[] = "Missing directory: $dir";
    }
}

echo "\n3. Validating PHP Syntax...\n";
$phpFiles = glob(__DIR__ . '/src/**/*.php', GLOB_BRACE);
$phpFiles = array_merge($phpFiles, glob(__DIR__ . '/src/*/*.php'));
$phpFiles = array_merge($phpFiles, glob(__DIR__ . '/src/*/*/*.php'));
$phpFiles = array_merge($phpFiles, glob(__DIR__ . '/examples/*.php'));

$syntaxErrors = 0;
foreach (array_unique($phpFiles) as $file) {
    $output = [];
    $returnVar = 0;
    exec("php -l " . escapeshellarg($file) . " 2>&1", $output, $returnVar);

    if ($returnVar === 0) {
        // Syntax OK
    } else {
        echo "   ✗ Syntax error in: $file\n";
        $syntaxErrors++;
        $errors[] = "Syntax error in: $file";
    }
}

if ($syntaxErrors === 0) {
    echo "   ✓ All PHP files have valid syntax\n";
}

echo "\n4. Checking Class Definitions...\n";
$classes = [
    'UMICP\\Core\\Envelope',
    'UMICP\\Core\\Matrix',
    'UMICP\\Core\\OperationType',
    'UMICP\\Core\\PayloadType',
    'UMICP\\Core\\EncodingType',
    'UMICP\\Core\\PayloadHint',
    'UMICP\\FFI\\FFIBridge',
    'UMICP\\FFI\\Config',
    'UMICP\\FFI\\TypeConverter',
    'UMICP\\Exception\\UMICPException',
    'UMICP\\Exception\\FFIException',
];

foreach ($classes as $class) {
    $file = str_replace('UMICP\\', 'src/', $class) . '.php';
    $file = str_replace('\\', '/', $file);

    if (file_exists(__DIR__ . '/' . $file)) {
        // Check if class/enum/trait is defined
        $content = file_get_contents(__DIR__ . '/' . $file);
        $basename = basename($class);

        if (str_contains($content, "class $basename") ||
            str_contains($content, "enum $basename") ||
            str_contains($content, "trait $basename")) {
            echo "   ✓ $class\n";
        } else {
            echo "   ✗ $class (definition not found)\n";
            $warnings[] = "Class definition not found: $class";
        }
    }
}

echo "\n5. Checking Documentation...\n";
$docFiles = [
    'docs/ARCHITECTURE.md',
    'docs/IMPLEMENTATION_ROADMAP.md',
    'docs/API_SPECIFICATION.md',
    'docs/FFI_INTEGRATION_GUIDE.md',
];

$totalPages = 0;
foreach ($docFiles as $doc) {
    $path = __DIR__ . '/' . $doc;
    if (file_exists($path)) {
        $lines = count(file($path));
        $pages = (int) ceil($lines / 50);
        $totalPages += $pages;
        echo "   ✓ " . basename($doc) . " (~$pages pages, $lines lines)\n";
    }
}

echo "\n" . str_repeat("=", 60) . "\n";
echo "VALIDATION SUMMARY\n";
echo str_repeat("=", 60) . "\n\n";

echo "Files Validated:     " . $success . "/" . count($requiredFiles) . "\n";
echo "Errors:              " . count($errors) . "\n";
echo "Warnings:            " . count($warnings) . "\n";
echo "Documentation:       ~$totalPages pages\n";
echo "PHP Files:           " . count(array_unique($phpFiles)) . "\n";

if (count($errors) === 0 && $syntaxErrors === 0) {
    echo "\n✅ Structure validation PASSED!\n";
    echo "\nNext steps:\n";
    echo "  1. Implement C++ FFI wrapper (cpp/src/ffi_wrapper.cpp)\n";
    echo "  2. Build C++ core as shared library\n";
    echo "  3. Run examples: php examples/01_basic_envelope.php\n";
    exit(0);
} else {
    echo "\n❌ Structure validation FAILED!\n";

    if (!empty($errors)) {
        echo "\nErrors:\n";
        foreach ($errors as $error) {
            echo "  - $error\n";
        }
    }

    if (!empty($warnings)) {
        echo "\nWarnings:\n";
        foreach ($warnings as $warning) {
            echo "  - $warning\n";
        }
    }

    exit(1);
}

