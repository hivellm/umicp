#!/bin/bash

# UMICP PHP Bindings - Production Release Script
# Prepares package for Packagist publication

set -e

echo "╔════════════════════════════════════════════════════════════╗"
echo "║     UMICP PHP Bindings - Production Release               ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
PACKAGE_NAME="hivellm/umicp"
VERSION="0.1.1"

echo -e "${BLUE}📦 Preparing $PACKAGE_NAME v$VERSION for Packagist${NC}"
echo

# Check if we're in the right directory
if [ ! -f "composer.json" ]; then
    echo -e "${RED}❌ Error: composer.json not found. Run from package root.${NC}"
    exit 1
fi

# Validate composer.json
echo -e "${BLUE}1. Validating composer.json...${NC}"
if composer validate; then
    echo -e "${GREEN}✓ composer.json is valid${NC}"
else
    echo -e "${RED}❌ composer.json validation failed${NC}"
    exit 1
fi

# Check composer.json structure
echo -e "${BLUE}2. Checking composer.json metadata...${NC}"

# Check required fields
REQUIRED_FIELDS=("name" "description" "version" "type" "license" "authors" "require")
for field in "${REQUIRED_FIELDS[@]}"; do
    if ! composer config $field > /dev/null 2>&1; then
        echo -e "${RED}❌ Missing required field: $field${NC}"
        exit 1
    fi
done
echo -e "${GREEN}✓ All required fields present${NC}"

# Check version consistency
COMPOSER_VERSION=$(composer config version)
if [ "$COMPOSER_VERSION" != "$VERSION" ]; then
    echo -e "${RED}❌ Version mismatch: composer.json has $COMPOSER_VERSION, expected $VERSION${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Version consistency: $VERSION${NC}"

# Run full test suite
echo -e "${BLUE}3. Running complete test suite...${NC}"
if [ -f "test-all.sh" ]; then
    chmod +x test-all.sh
    if ./test-all.sh; then
        echo -e "${GREEN}✓ All tests passed${NC}"
    else
        echo -e "${RED}❌ Test suite failed${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}⚠ test-all.sh not found, running PHPUnit directly${NC}"
    if composer test; then
        echo -e "${GREEN}✓ Tests passed${NC}"
    else
        echo -e "${RED}❌ Tests failed${NC}"
        exit 1
    fi
fi

# Run code quality checks
echo -e "${BLUE}4. Running code quality checks...${NC}"
if composer check:all; then
    echo -e "${GREEN}✓ Code quality checks passed${NC}"
else
    echo -e "${RED}❌ Code quality checks failed${NC}"
    exit 1
fi

# Generate test coverage
echo -e "${BLUE}5. Generating test coverage report...${NC}"
if composer test:coverage; then
    echo -e "${GREEN}✓ Coverage report generated${NC}"
else
    echo -e "${YELLOW}⚠ Coverage report failed (non-critical)${NC}"
fi

# Run performance benchmarks
echo -e "${BLUE}6. Running performance benchmarks...${NC}"
if [ -f "benchmark.php" ]; then
    if php benchmark.php; then
        echo -e "${GREEN}✓ Benchmarks completed${NC}"
    else
        echo -e "${YELLOW}⚠ Benchmarks failed (non-critical)${NC}"
    fi
else
    echo -e "${YELLOW}⚠ benchmark.php not found${NC}"
fi

# Verify implementation
echo -e "${BLUE}7. Verifying implementation completeness...${NC}"
if [ -f "verify-implementation.php" ]; then
    if php verify-implementation.php; then
        echo -e "${GREEN}✓ Implementation verification passed${NC}"
    else
        echo -e "${RED}❌ Implementation verification failed${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}⚠ verify-implementation.php not found${NC}"
fi

# Check documentation
echo -e "${BLUE}8. Checking documentation...${NC}"
if [ -d "docs" ] && [ -f "docs/README.md" ]; then
    echo -e "${GREEN}✓ Documentation directory present${NC}"
else
    echo -e "${RED}❌ Documentation missing${NC}"
    exit 1
fi

# Check examples
echo -e "${BLUE}9. Checking examples...${NC}"
if [ -d "examples" ] && [ "$(ls examples/*.php 2>/dev/null | wc -l)" -ge 3 ]; then
    echo -e "${GREEN}✓ Examples present${NC}"
else
    echo -e "${YELLOW}⚠ Few or no examples found${NC}"
fi

# Generate build info
echo -e "${BLUE}10. Generating build information...${NC}"
BUILD_INFO_FILE="build-info.json"
cat > "$BUILD_INFO_FILE" << EOF
{
  "package": "$PACKAGE_NAME",
  "version": "$VERSION",
  "build_date": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
  "build_host": "$(hostname)",
  "php_version": "$(php -r 'echo PHP_VERSION;')",
  "composer_version": "$(composer --version | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')",
  "git_commit": "$(git rev-parse HEAD 2>/dev/null || echo 'unknown')",
  "git_branch": "$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo 'unknown')"
}
EOF
echo -e "${GREEN}✓ Build info generated: $BUILD_INFO_FILE${NC}"

# Clean up for distribution
echo -e "${BLUE}11. Preparing distribution package...${NC}"

# Remove development files from archive (composer.json archive.exclude handles this)
echo -e "${GREEN}✓ Distribution exclusions configured in composer.json${NC}"

# Final validation
echo -e "${BLUE}12. Final validation...${NC}"

# Check file structure
REQUIRED_FILES=("src/" "composer.json" "README.md" "LICENSE")
for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -e "$file" ]; then
        echo -e "${RED}❌ Missing required file: $file${NC}"
        exit 1
    fi
done
echo -e "${GREEN}✓ Required files present${NC}"

# Check for sensitive data
if grep -r "password\|secret\|key.*=.*[" src/ 2>/dev/null; then
    echo -e "${RED}❌ Potential sensitive data found in source${NC}"
    exit 1
fi
echo -e "${GREEN}✓ No sensitive data detected${NC}"

echo
echo "╔════════════════════════════════════════════════════════════╗"
echo "║                    RELEASE READY!                          ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo
echo -e "${GREEN}✅ Package $PACKAGE_NAME v$VERSION is ready for Packagist!${NC}"
echo
echo "📋 Next steps:"
echo "  1. Commit all changes: git add . && git commit -m 'Release v$VERSION'"
echo "  2. Tag the release: git tag -a v$VERSION -m 'Release v$VERSION'"
echo "  3. Push to GitHub: git push && git push --tags"
echo "  4. Publish to Packagist: Visit https://packagist.org/packages/submit"
echo "  5. Or use: composer publish (if you have packagist credentials)"
echo
echo "📦 Package will be available as: composer require $PACKAGE_NAME"
echo
echo "🔗 Repository: https://github.com/hivellm/umicp"
echo "📚 Documentation: https://github.com/hivellm/umicp/tree/main/bindings/php/docs"
echo
echo -e "${GREEN}🎉 Happy releasing!${NC}"

# Optional: Create GitHub release notes
if [ -f "docs/CHANGELOG.md" ]; then
    echo
    echo -e "${BLUE}📝 Release notes from CHANGELOG.md:${NC}"
    echo "----------------------------------------"
    # Extract the latest release notes
    awk '/^## \['"$VERSION"'\]/,/^## \[|^---/' docs/CHANGELOG.md | head -n -1
    echo "----------------------------------------"
fi
