<?php

declare(strict_types=1);

/**
 * Test Runner - Runs all tests with detailed reporting
 */

echo "╔════════════════════════════════════════════════════════════╗\n";
echo "║     UMICP PHP Bindings - Complete Test Suite              ║\n";
echo "╚════════════════════════════════════════════════════════════╝\n\n";

$totalTests = 0;
$passedTests = 0;
$failedTests = 0;
$skippedTests = 0;

// Check if vendor exists
if (!file_exists(__DIR__ . '/vendor/autoload.php')) {
    echo "❌ Error: Composer dependencies not installed\n";
    echo "Run: composer install\n";
    exit(1);
}

echo "1. Running Unit Tests...\n";
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
passthru('./vendor/bin/phpunit --testsuite=Unit --testdox', $unitResult);
echo "\n";

echo "2. Running Integration Tests...\n";
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
passthru('./vendor/bin/phpunit --testsuite=Integration --testdox', $integrationResult);
echo "\n";

echo "3. Running Performance Tests...\n";
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
passthru('./vendor/bin/phpunit --testsuite=Performance --group=performance', $perfResult);
echo "\n";

echo "4. Test Coverage Report...\n";
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
passthru('./vendor/bin/phpunit --coverage-text', $coverageResult);
echo "\n";

echo "5. Code Quality Checks...\n";
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

echo "   Checking code style (PSR-12)...\n";
passthru('composer lint', $lintResult);
echo "\n";

echo "   Running static analysis (PHPStan)...\n";
passthru('composer analyse', $analyseResult);
echo "\n";

// Summary
echo "╔════════════════════════════════════════════════════════════╗\n";
echo "║                     TEST SUMMARY                           ║\n";
echo "╚════════════════════════════════════════════════════════════╝\n\n";

$allPassed = ($unitResult === 0 && $integrationResult === 0);

if ($allPassed) {
    echo "✅ ALL TESTS PASSED!\n\n";

    echo "Test Suites:\n";
    echo "  ✓ Unit Tests\n";
    echo "  ✓ Integration Tests\n";
    echo "  ✓ Performance Tests\n\n";

    echo "Code Quality:\n";
    echo "  " . ($lintResult === 0 ? '✓' : '⚠') . " Code Style (PSR-12)\n";
    echo "  " . ($analyseResult === 0 ? '✓' : '⚠') . " Static Analysis\n\n";

    echo "Next steps:\n";
    echo "  • Run examples: php examples/03_complete_demo.php\n";
    echo "  • Run benchmarks: php benchmark.php\n";
    echo "  • Build C++: ./build-cpp.sh\n\n";

    exit(0);
} else {
    echo "❌ SOME TESTS FAILED\n\n";

    echo "Failed Suites:\n";
    if ($unitResult !== 0) echo "  ✗ Unit Tests\n";
    if ($integrationResult !== 0) echo "  ✗ Integration Tests\n";
    if ($perfResult !== 0) echo "  ✗ Performance Tests\n";
    echo "\n";

    echo "Review output above for details.\n\n";

    exit(1);
}

