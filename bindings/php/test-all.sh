#!/bin/bash

# Complete test suite runner

set -e

echo "╔════════════════════════════════════════════════════════════╗"
echo "║     UMICP PHP Bindings - Complete Test Suite              ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

FAILED=0

# Verify implementation
echo -e "${BLUE}Step 1: Verifying Implementation${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
php verify-implementation.php || FAILED=1
echo

# Validate structure
echo -e "${BLUE}Step 2: Validating Structure${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
php validate-structure.php || FAILED=1
echo

# Run unit tests
echo -e "${BLUE}Step 3: Unit Tests${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
./vendor/bin/phpunit --testsuite=Unit --testdox || FAILED=1
echo

# Run integration tests
echo -e "${BLUE}Step 4: Integration Tests${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
./vendor/bin/phpunit --testsuite=Integration --testdox || FAILED=1
echo

# Run performance tests
echo -e "${BLUE}Step 5: Performance Tests${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
./vendor/bin/phpunit --testsuite=Performance --group=performance || FAILED=1
echo

# Code style
echo -e "${BLUE}Step 6: Code Style Check${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
composer lint || FAILED=1
echo

# Static analysis
echo -e "${BLUE}Step 7: Static Analysis${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
composer analyse || FAILED=1
echo

# Summary
echo "╔════════════════════════════════════════════════════════════╗"
echo "║                    FINAL RESULTS                           ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✅ ALL CHECKS PASSED!${NC}"
    echo
    echo "Implementation is production-ready!"
    echo
    echo "Test Coverage:"
    ./vendor/bin/phpunit --coverage-text | grep -A 5 "Code Coverage"
    echo
    exit 0
else
    echo -e "${RED}❌ SOME CHECKS FAILED${NC}"
    echo
    echo "Review output above for details."
    echo
    exit 1
fi

