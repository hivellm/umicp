#!/usr/bin/env php
<?php
/**
 * Package Validation Script
 * Validates the UMICP PHP package before publishing to Packagist
 */

declare(strict_types=1);

$errors = [];
$warnings = [];

echo "🔍 Validating UMICP PHP Package...\n\n";

// Check composer.json
if (!file_exists('composer.json')) {
    $errors[] = "composer.json not found";
} else {
    $composer = json_decode(file_get_contents('composer.json'), true);

    // Required fields
    $requiredFields = ['name', 'description', 'type', 'license', 'authors', 'require', 'autoload'];
    foreach ($requiredFields as $field) {
        if (!isset($composer[$field])) {
            $errors[] = "Missing required field in composer.json: $field";
        }
    }

    // Check package name
    if (isset($composer['name']) && $composer['name'] !== 'hivellm/umicp') {
        $warnings[] = "Package name should be 'hivellm/umicp', got: {$composer['name']}";
    }

    // Check license
    if (isset($composer['license']) && $composer['license'] !== 'MIT') {
        $warnings[] = "License should be 'MIT', got: {$composer['license']}";
    }

    // Check PHP version
    if (isset($composer['require']['php'])) {
        echo "✅ PHP requirement: {$composer['require']['php']}\n";
    } else {
        $errors[] = "PHP version requirement missing";
    }

    echo "✅ Package: {$composer['name']}\n";
    if (isset($composer['version'])) {
        echo "✅ Version: {$composer['version']}\n";
    } else {
        echo "✅ Version: (from Git tags)\n";
    }
    echo "✅ License: {$composer['license']}\n";
}

// Check required files
$requiredFiles = [
    'README.md' => 'Package documentation',
    'LICENSE' => 'License file',
    'composer.json' => 'Package manifest',
    'src' => 'Source directory'
];

echo "\n📁 Checking required files...\n";
foreach ($requiredFiles as $file => $desc) {
    if (file_exists($file)) {
        echo "✅ $desc ($file)\n";
    } else {
        $errors[] = "Missing $desc: $file";
    }
}

// Check source structure
echo "\n📂 Checking source structure...\n";
$sourceDirs = ['Core', 'Transport', 'FFI', 'Exception', 'Discovery', 'Pool'];
foreach ($sourceDirs as $dir) {
    $path = "src/$dir";
    if (is_dir($path)) {
        $files = glob("$path/*.php");
        echo "✅ $dir: " . count($files) . " files\n";
    } else {
        $warnings[] = "Missing source directory: $path";
    }
}

// Check autoload
echo "\n🔧 Checking autoload...\n";
if (file_exists('vendor/autoload.php')) {
    require 'vendor/autoload.php';
    echo "✅ Autoload working\n";

    // Try to instantiate main classes (FFI is optional for package validation)
    try {
        // Check if classes are defined (don't instantiate if FFI not available)
        if (class_exists(\UMICP\Core\Envelope::class)) {
            echo "✅ Core classes defined\n";
        }

        // Only test instantiation if FFI library is available
        if (file_exists('libumicp_core.so')) {
            new \UMICP\Core\Envelope('test-from', 'test-to', \UMICP\Core\OperationType::DATA);
            echo "✅ Core classes loadable (FFI working)\n";
        } else {
            echo "⚠️  FFI library not found (optional for package validation)\n";
        }
    } catch (\Throwable $e) {
        $warnings[] = "FFI not available (optional): " . $e->getMessage();
    }
} else {
    $errors[] = "Vendor autoload not found. Run: composer install";
}

// Check tests
echo "\n🧪 Checking tests...\n";
if (is_dir('tests')) {
    $testFiles = array_merge(
        glob('tests/Unit/**/*Test.php'),
        glob('tests/Integration/*Test.php'),
        glob('tests/Performance/*Test.php')
    );
    echo "✅ Test files: " . count($testFiles) . "\n";
} else {
    $warnings[] = "Tests directory not found";
}

// Check documentation
echo "\n📚 Checking documentation...\n";
if (is_dir('docs')) {
    $docFiles = glob('docs/*.md');
    echo "✅ Documentation files: " . count($docFiles) . "\n";
} else {
    $warnings[] = "Docs directory not found";
}

// Summary
echo "\n" . str_repeat("=", 60) . "\n";

if (count($errors) > 0) {
    echo "❌ VALIDATION FAILED\n\n";
    echo "Errors:\n";
    foreach ($errors as $error) {
        echo "  ❌ $error\n";
    }
    exit(1);
}

if (count($warnings) > 0) {
    echo "⚠️  VALIDATION PASSED WITH WARNINGS\n\n";
    echo "Warnings:\n";
    foreach ($warnings as $warning) {
        echo "  ⚠️  $warning\n";
    }
    echo "\n";
}

echo "✅ PACKAGE VALIDATION SUCCESSFUL!\n\n";
echo "Package is ready for Packagist publication.\n";
echo "\nNext steps:\n";
echo "  1. Run: ./publish.sh\n";
echo "  2. Or manually create and push a Git tag:\n";
echo "     git tag v0.1.3\n";
echo "     git push origin v0.1.3\n";
echo "\n";

exit(0);

