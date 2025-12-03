#!/bin/bash
# Build script for UMICP PHP SDK
# Validates and prepares the package for Packagist/Composer

set -e

echo "🔨 Building UMICP PHP SDK..."

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Check if PHP is installed
if ! command -v php &> /dev/null; then
    echo -e "${RED}❌ PHP not found. Please install PHP 8.1 or higher.${NC}"
    exit 1
fi

# Check PHP version
echo -e "${BLUE}📋 Checking PHP version...${NC}"
PHP_VERSION=$(php -r "echo PHP_VERSION;")
echo "PHP version: $PHP_VERSION"

# Check required extensions
echo -e "${BLUE}🔧 Checking required extensions...${NC}"
REQUIRED_EXTS=("ffi" "json")
for ext in "${REQUIRED_EXTS[@]}"; do
    if ! php -m | grep -qi "^$ext$"; then
        echo -e "${RED}❌ Required extension '$ext' not found.${NC}"
        exit 1
    fi
    echo -e "${GREEN}✅ $ext${NC}"
done

# Check if composer is installed
if ! command -v composer &> /dev/null; then
    echo -e "${RED}❌ Composer not found. Please install Composer.${NC}"
    echo "Download from: https://getcomposer.org/download/"
    exit 1
fi

# Validate composer.json
echo -e "${BLUE}📦 Validating composer.json...${NC}"
composer validate --strict
if [ $? -ne 0 ]; then
    echo -e "${RED}❌ composer.json validation failed!${NC}"
    exit 1
fi

# Install dependencies
echo -e "${BLUE}📦 Installing dependencies...${NC}"
composer install --no-dev --optimize-autoloader

# Install dev dependencies for testing
echo -e "${BLUE}📦 Installing dev dependencies...${NC}"
composer install --optimize-autoloader

# Run linting
echo -e "${BLUE}🔍 Running code style check (PSR-12)...${NC}"
./vendor/bin/phpcs --standard=PSR12 src/ --report=summary || {
    echo -e "${YELLOW}⚠️  Linting warnings found (non-blocking)${NC}"
}

# Run static analysis
echo -e "${BLUE}🔬 Running static analysis (PHPStan)...${NC}"
./vendor/bin/phpstan analyse src/ --level=8 --no-progress || {
    echo -e "${YELLOW}⚠️  Static analysis warnings found (non-blocking)${NC}"
}

# Note: Tests require compiled FFI library (libumicp_core.so)
# For Packagist publication, we only validate the package structure
echo -e "${YELLOW}⚠️  Skipping FFI-dependent tests (library not compiled)${NC}"
echo -e "${BLUE}ℹ️  For full test suite, compile C++ library first: ./build-cpp.sh${NC}"

# Run validation
echo -e "${BLUE}✅ Validating package structure...${NC}"
php validate-package.php
if [ $? -ne 0 ]; then
    echo -e "${RED}❌ Package validation failed!${NC}"
    exit 1
fi

# Get package version from Git tags
VERSION=$(git describe --tags --abbrev=0 2>/dev/null | sed 's/^v//' || echo "dev-main")

# Success message
echo ""
echo -e "${GREEN}✅ Build completed successfully!${NC}"
echo ""
echo "Package: hivellm/umicp"
echo "Version: $VERSION (from Git tags)"
echo ""
echo "Package is ready for Packagist!"
echo ""
echo "Next steps:"
echo "  1. Commit changes: git add . && git commit -m 'Release v$VERSION'"
echo "  2. Create tag: git tag v$VERSION"
echo "  3. Push with tags: git push origin main --tags"
echo "  4. Packagist will auto-update (if webhook configured)"
echo ""
echo "Manual publish: ./publish.sh"
echo ""

