#!/bin/bash

# Build C++ core library for PHP FFI
# This script builds the UMICP C++ core as a shared library

set -e

echo "========================================"
echo "  UMICP C++ Core - Build for PHP FFI"
echo "========================================"
echo

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Navigate to C++ directory
cd ../../cpp

# Create build directory
echo "1. Creating build directory..."
mkdir -p build
cd build

# Configure CMake
echo
echo "2. Configuring CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_TESTS=OFF

# Build
echo
echo "3. Building..."
make -j$(nproc) umicp_core

# Check if library was built
if [ -f "libumicp_core.so" ] || [ -f "libumicp_core.dylib" ] || [ -f "umicp_core.dll" ]; then
    echo
    echo -e "${GREEN}✓ Build successful!${NC}"
    echo

    # Show library info
    if [ -f "libumicp_core.so" ]; then
        echo "Library: libumicp_core.so"
        ls -lh libumicp_core.so
        echo
        echo "Exported symbols:"
        nm -D libumicp_core.so | grep umicp_ | head -n 10
    elif [ -f "libumicp_core.dylib" ]; then
        echo "Library: libumicp_core.dylib"
        ls -lh libumicp_core.dylib
    elif [ -f "umicp_core.dll" ]; then
        echo "Library: umicp_core.dll"
        ls -lh umicp_core.dll
    fi

    echo
    echo "Next steps:"
    echo "  1. Update config/umicp.php with library path"
    echo "  2. Run: composer install"
    echo "  3. Test: php examples/01_basic_envelope.php"
else
    echo
    echo -e "${RED}✗ Build failed!${NC}"
    echo
    echo "Check build output above for errors."
    exit 1
fi

