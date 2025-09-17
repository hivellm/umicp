#!/bin/bash

# UMICP Dependencies Installation Script
# Installs libwebsockets and nghttp2 for WebSocket and HTTP/2 support

set -e

echo "🚀 Installing UMICP Dependencies..."

# Update package lists
echo "📦 Updating package lists..."
sudo apt-get update

# Install build dependencies
echo "🔧 Installing build dependencies..."
sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libssl-dev \
    libjson-c-dev \
    zlib1g-dev \
    libcbor-dev

# Install libwebsockets
echo "🌐 Installing libwebsockets..."
sudo apt-get install -y libwebsockets-dev

# Install nghttp2
echo "📡 Installing nghttp2..."
sudo apt-get install -y libnghttp2-dev

# Install optional dependencies
echo "📚 Installing optional dependencies..."
sudo apt-get install -y \
    libmsgpack-dev \
    libcbor-dev

# Verify installations
echo "✅ Verifying installations..."

echo "Checking libwebsockets..."
pkg-config --exists libwebsockets && echo "  ✓ libwebsockets found" || echo "  ❌ libwebsockets not found"

echo "Checking nghttp2..."
pkg-config --exists libnghttp2 && echo "  ✓ nghttp2 found" || echo "  ❌ nghttp2 not found"

echo "Checking OpenSSL..."
pkg-config --exists openssl && echo "  ✓ OpenSSL found" || echo "  ❌ OpenSSL not found"

echo "Checking json-c..."
pkg-config --exists json-c && echo "  ✓ json-c found" || echo "  ❌ json-c not found"

echo "Checking zlib..."
pkg-config --exists zlib && echo "  ✓ zlib found" || echo "  ❌ zlib not found"

echo ""
echo "🎉 Dependencies installation complete!"
echo ""
echo "You can now build UMICP with:"
echo "  cd build"
echo "  cmake .."
echo "  make -j\$(nproc)"
echo ""
