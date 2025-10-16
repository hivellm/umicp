#!/bin/bash
# Build script for UMICP Swift bindings

set -e

echo "🔨 Building UMICP Swift..."

# Check Swift version
swift --version

# Clean previous build
echo "🧹 Cleaning previous build..."
swift package clean

# Build for release
echo "📦 Building release..."
swift build -c release

# Run tests
echo "🧪 Running tests..."
swift test

echo "✅ Build completed successfully!"
echo ""
echo "📊 Build artifacts:"
swift build --show-bin-path

echo ""
echo "🚀 To run examples:"
echo "  swift run BasicEnvelope"
echo "  swift run ToolDiscovery"
echo "  swift run MatrixOperations"

