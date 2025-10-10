#!/bin/bash

# UMICP PHP Bindings - Complete Setup Script
# This script sets up the entire PHP bindings environment

set -e

echo "╔════════════════════════════════════════════════════════════╗"
echo "║     UMICP PHP Bindings - Complete Setup                   ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check PHP version
echo -e "${BLUE}1. Checking PHP version...${NC}"
PHP_VERSION=$(php -r 'echo PHP_VERSION;')
PHP_MAJOR=$(php -r 'echo PHP_MAJOR_VERSION;')
PHP_MINOR=$(php -r 'echo PHP_MINOR_VERSION;')

echo "   PHP Version: $PHP_VERSION"

if [ "$PHP_MAJOR" -lt 8 ] || ([ "$PHP_MAJOR" -eq 8 ] && [ "$PHP_MINOR" -lt 1 ]); then
    echo -e "   ${RED}✗ PHP 8.1+ required${NC}"
    exit 1
fi
echo -e "   ${GREEN}✓ PHP version OK${NC}"
echo

# Check FFI extension
echo -e "${BLUE}2. Checking FFI extension...${NC}"
if php -m | grep -q FFI; then
    echo -e "   ${GREEN}✓ FFI extension loaded${NC}"
else
    echo -e "   ${RED}✗ FFI extension not loaded${NC}"
    echo "   Install: apt-get install php-ffi (Ubuntu/Debian)"
    echo "   Enable in php.ini: extension=ffi"
    exit 1
fi

# Check FFI enabled
FFI_ENABLED=$(php -r 'echo ini_get("ffi.enable");')
if [ "$FFI_ENABLED" == "1" ] || [ "$FFI_ENABLED" == "preload" ]; then
    echo -e "   ${GREEN}✓ FFI enabled${NC}"
else
    echo -e "   ${YELLOW}⚠ FFI not enabled in php.ini${NC}"
    echo "   Add to php.ini: ffi.enable=1"
fi
echo

# Install Composer dependencies
echo -e "${BLUE}3. Installing Composer dependencies...${NC}"
if command -v composer &> /dev/null; then
    composer install
    echo -e "   ${GREEN}✓ Dependencies installed${NC}"
else
    echo -e "   ${YELLOW}⚠ Composer not found, skipping${NC}"
fi
echo

# Build C++ core
echo -e "${BLUE}4. Building C++ core library...${NC}"
if [ -f "build-cpp.sh" ]; then
    chmod +x build-cpp.sh
    ./build-cpp.sh
else
    echo -e "   ${YELLOW}⚠ build-cpp.sh not found${NC}"
fi
echo

# Copy config
echo -e "${BLUE}5. Setting up configuration...${NC}"
if [ ! -f "config/umicp.php" ]; then
    cp config/umicp.example.php config/umicp.php
    echo -e "   ${GREEN}✓ Created config/umicp.php${NC}"
    echo -e "   ${YELLOW}⚠ Review and update library path in config/umicp.php${NC}"
else
    echo -e "   ${GREEN}✓ config/umicp.php exists${NC}"
fi
echo

# Run validation
echo -e "${BLUE}6. Validating structure...${NC}"
if [ -f "validate-structure.php" ]; then
    php validate-structure.php
else
    echo -e "   ${YELLOW}⚠ validate-structure.php not found${NC}"
fi
echo

# Summary
echo "╔════════════════════════════════════════════════════════════╗"
echo "║                    Setup Complete                          ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo
echo "Next steps:"
echo "  1. Run examples:"
echo "     php examples/01_basic_envelope.php"
echo "     php examples/02_matrix_operations.php"
echo "     php examples/03_complete_demo.php"
echo
echo "  2. Run tests:"
echo "     ./vendor/bin/phpunit"
echo
echo "  3. Check code quality:"
echo "     composer lint"
echo "     composer analyse"
echo

