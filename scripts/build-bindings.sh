#!/bin/bash

# UMICP TypeScript Bindings Build Script
# Builds the native C++ addon for Node.js

set -e

echo "🚀 Building UMICP TypeScript Bindings"
echo "====================================="

# Check if we're in the right directory
if [ ! -d "bindings/typescript" ]; then
    echo "❌ Error: Must be run from UMICP root directory"
    exit 1
fi

cd bindings/typescript

# Check if node-gyp is available
if ! command -v node-gyp &> /dev/null; then
    echo "❌ node-gyp not found. Please install it: npm install -g node-gyp"
    exit 1
fi

# Configure and build
echo "📦 Configuring build..."
node-gyp configure

echo "🔨 Building native addon..."
node-gyp build

# Check if build succeeded
if [ -f "build/Release/umicp_core.node" ]; then
    echo "✅ Build successful!"
    echo "📁 Output: bindings/typescript/build/Release/umicp_core.node"

    # Test the build
    echo "🧪 Testing build..."
    node -e "try { require('./build/Release/umicp_core.node'); console.log('✅ Native addon loads successfully'); } catch(e) { console.error('❌ Failed to load addon:', e.message); process.exit(1); }"

else
    echo "❌ Build failed!"
    exit 1
fi

echo ""
echo "🎉 UMICP TypeScript bindings built successfully!"
echo "=============================================="
echo ""
echo "To use the bindings:"
echo "  const umicp = require('./bindings/typescript/build/Release/umicp_core.node');"
echo "  // or import from the TypeScript wrapper"
echo "  import { Envelope, Matrix } from './bindings/typescript/src/index.js';"
