# Publishing UMICP to vcpkg

This guide explains how to publish UMICP to Microsoft vcpkg registry.

## Prerequisites

- vcpkg installed
- GitHub account
- vcpkg fork

## Steps to Publish

### 1. Test the Port Locally

```bash
# Clone vcpkg (if not already done)
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
./bootstrap-vcpkg.sh  # Linux/macOS
.\bootstrap-vcpkg.bat  # Windows

# Create port directory
mkdir -p ports/umicp

# Copy port files from umicp/vcpkg/
cp path/to/umicp/vcpkg/* ports/umicp/
```

### 2. Calculate SHA512 Hash

```bash
# Download the source tarball
wget https://github.com/hivellm/umicp/archive/refs/tags/v0.1.1.tar.gz

# Calculate SHA512
sha512sum v0.1.1.tar.gz

# Update the SHA512 in portfile.cmake
# Replace "SHA512 0" with the actual hash
```

### 3. Test the Port

```bash
# Test installation
./vcpkg install umicp

# Test with features
./vcpkg install umicp[websocket,http]

# Test on different triplets
./vcpkg install umicp:x64-linux
./vcpkg install umicp:x64-windows
./vcpkg install umicp:x64-osx
```

### 4. Verify the Installation

```bash
# Check installed files
./vcpkg list umicp

# Test with a simple CMake project
cat > test_umicp.cpp << 'EOF'
#include <umicp/envelope.hpp>
#include <iostream>

int main() {
    umicp::Envelope env;
    env.setFrom("test");
    env.setTo("receiver");
    std::cout << "UMICP works!" << std::endl;
    return 0;
}
EOF

# Create CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.15)
project(test_umicp)

find_package(UMICP CONFIG REQUIRED)

add_executable(test_umicp test_umicp.cpp)
target_link_libraries(test_umicp PRIVATE UMICP::UMICP)
EOF

# Build and test
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build build
./build/test_umicp
```

### 5. Fork vcpkg Repository

```bash
# Fork on GitHub
# https://github.com/Microsoft/vcpkg

# Clone your fork
git clone https://github.com/YOUR_USERNAME/vcpkg.git
cd vcpkg

# Add upstream
git remote add upstream https://github.com/Microsoft/vcpkg.git
```

### 6. Create the Port

```bash
# Create branch
git checkout -b add-umicp-0.1.1

# Create port directory
mkdir -p ports/umicp

# Copy files
cp path/to/umicp/vcpkg/portfile.cmake ports/umicp/
cp path/to/umicp/vcpkg/vcpkg.json ports/umicp/
cp path/to/umicp/vcpkg/usage ports/umicp/
```

### 7. Update Baseline and Versions

```bash
# Run vcpkg x-add-version
./vcpkg x-add-version umicp

# This will update:
# - versions/baseline.json
# - versions/u-/umicp.json
```

### 8. Test the Port Again

```bash
# Remove any cached version
./vcpkg remove umicp

# Reinstall from your new port
./vcpkg install umicp

# Run CI checks locally
./vcpkg ci umicp
```

### 9. Commit and Push

```bash
# Stage changes
git add ports/umicp/
git add versions/baseline.json
git add versions/u-/umicp.json

# Commit with proper format
git commit -m "[umicp] new port (0.1.1)"

# Push to your fork
git push origin add-umicp-0.1.1
```

### 10. Create Pull Request

- Go to: https://github.com/YOUR_USERNAME/vcpkg
- Click "New Pull Request"
- Title: `[umicp] new port (0.1.1)`
- Description:

```markdown
### Description
Add UMICP (Universal Matrix Intelligent Communication Protocol) - A high-performance communication protocol for AI model interoperability.

### Features
- Core protocol with envelope/frame support
- Matrix operations for vector processing
- WebSocket transport (optional)
- HTTP/HTTP2 transport (optional)
- Cross-platform (Linux, macOS, Windows)

### Testing
- [x] Tested on x64-linux
- [x] Tested on x64-windows
- [x] Tested on x64-osx
- [x] All features working
- [x] Documentation provided

### Dependencies
- nlohmann-json
- openssl
- websocketpp (feature)
- asio (feature)
- curl (feature)

### License
MIT

### Checklist
- [x] vcpkg.json updated
- [x] portfile.cmake created
- [x] usage file provided
- [x] versions updated (x-add-version)
- [x] Builds on all platforms
```

## Port Structure

```
ports/umicp/
├── portfile.cmake    # Build instructions
├── vcpkg.json        # Package metadata
└── usage             # Usage instructions
```

## vcpkg.json Format

```json
{
  "name": "umicp",
  "version": "0.1.1",
  "description": "Universal Matrix Intelligent Communication Protocol",
  "homepage": "https://github.com/hivellm/umicp",
  "license": "MIT",
  "dependencies": [
    "vcpkg-cmake",
    "vcpkg-cmake-config",
    "nlohmann-json",
    "openssl"
  ],
  "features": {
    "websocket": {
      "description": "Enable WebSocket transport",
      "dependencies": ["websocketpp", "asio"]
    },
    "http": {
      "description": "Enable HTTP transport",
      "dependencies": ["curl"]
    }
  }
}
```

## portfile.cmake Requirements

- Must use `vcpkg_from_github` or `vcpkg_from_git`
- Must include SHA512 hash
- Must use `vcpkg_cmake_configure`, `vcpkg_cmake_install`
- Must use `vcpkg_cmake_config_fixup`
- Must handle copyright file
- Must remove debug includes

## Common Issues

### SHA512 Mismatch

```bash
# Error: SHA512 mismatch
# Solution: Recalculate and update

sha512sum v0.1.1.tar.gz
# Update portfile.cmake with correct hash
```

### Missing Dependencies

```bash
# Error: Could not find package X
# Solution: Add to vcpkg.json dependencies

{
  "dependencies": [
    "missing-package"
  ]
}
```

### Build Failures

```bash
# Check build log
./vcpkg install umicp --debug

# Common fixes:
# 1. Update CMakeLists.txt for vcpkg compatibility
# 2. Add missing find_package() calls
# 3. Fix include paths
```

## CI/CD Pipeline

vcpkg CI will automatically:

1. Build for all platforms (Linux, Windows, macOS)
2. Test all compilers (GCC, Clang, MSVC)
3. Verify dependencies
4. Check license files
5. Validate port format

## Post-Approval

After PR is merged:

- Package available: `vcpkg install umicp`
- Listed on: https://vcpkg.io/en/packages.html
- Searchable: `vcpkg search umicp`

## Usage by Consumers

### With vcpkg.json (Manifest Mode)

```json
{
  "dependencies": [
    "umicp"
  ]
}
```

Or with features:

```json
{
  "dependencies": [
    {
      "name": "umicp",
      "features": ["websocket", "http"]
    }
  ]
}
```

### With CMake

```cmake
find_package(UMICP CONFIG REQUIRED)
target_link_libraries(your_app PRIVATE UMICP::UMICP)
```

### Command Line

```bash
# Basic installation
vcpkg install umicp

# With features
vcpkg install umicp[websocket,http]

# Specific triplet
vcpkg install umicp:x64-linux
vcpkg install umicp:x64-windows
vcpkg install umicp:x64-osx
```

## Version Updates

For future versions:

```bash
# Update portfile.cmake with new REF and SHA512
# Update vcpkg.json with new version
# Run x-add-version
./vcpkg x-add-version umicp --overwrite-version

# Create PR
git commit -m "[umicp] update to 0.2.0"
```

## Support

- vcpkg Issues: https://github.com/Microsoft/vcpkg/issues
- vcpkg Docs: https://vcpkg.io/
- UMICP Issues: https://github.com/hivellm/umicp/issues

---

**Status**: Ready for vcpkg submission  
**Version**: 0.1.1  
**License**: MIT

