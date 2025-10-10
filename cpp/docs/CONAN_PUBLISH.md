# Publishing UMICP to Conan Center

This guide explains how to publish UMICP to Conan Center Index.

## Prerequisites

- Conan 2.x installed (`pip install conan`)
- GitHub account
- Conan Center Index fork

## Steps to Publish

### 1. Test the Recipe Locally

```bash
cd umicp/cpp

# Create and test the package
conan create . --build=missing

# Test with different configurations
conan create . -s build_type=Debug --build=missing
conan create . -o with_websocket=False --build=missing
conan create . -o shared=True --build=missing
```

### 2. Test Package Installation

```bash
# Test the package in a separate directory
cd test_package
conan install .. --build=missing
conan build .
./build/Release/test_umicp
```

### 3. Fork Conan Center Index

```bash
# Fork the repository on GitHub
# https://github.com/conan-io/conan-center-index

# Clone your fork
git clone https://github.com/YOUR_USERNAME/conan-center-index.git
cd conan-center-index
```

### 4. Add UMICP Recipe

```bash
# Create recipe directory
mkdir -p recipes/umicp/all

# Copy files
cp path/to/umicp/cpp/conanfile.py recipes/umicp/all/
cp -r path/to/umicp/cpp/test_package recipes/umicp/all/

# Create config.yml
cat > recipes/umicp/config.yml << 'EOF'
versions:
  "0.1.1":
    folder: all
EOF
```

### 5. Test with Conan Center Hooks

```bash
# Install hooks
conan config install https://github.com/conan-io/hooks.git -tf hooks -sf hooks

# Test the recipe
cd recipes/umicp/all
conan create . umicp/0.1.1@ --build=missing
```

### 6. Submit Pull Request

```bash
# Create a branch
git checkout -b umicp-0.1.1

# Add files
git add recipes/umicp/

# Commit
git commit -m "Add umicp/0.1.1 recipe"

# Push
git push origin umicp-0.1.1
```

### 7. Create Pull Request on GitHub

- Go to: https://github.com/YOUR_USERNAME/conan-center-index
- Click "New Pull Request"
- Title: `(umicp/0.1.1) Add UMICP - Universal Matrix Intelligent Communication Protocol`
- Description: Include features, dependencies, and test results

## Recipe Guidelines

The recipe follows Conan Center guidelines:

- ✅ CMake-based build system
- ✅ Modern CMake targets (UMICP::UMICP)
- ✅ Options for shared/static builds
- ✅ Feature flags (websocket, http, tests)
- ✅ Cross-platform support (Linux, macOS, Windows)
- ✅ Test package included
- ✅ All dependencies from Conan Center

## Package Options

- `shared`: Build shared library (default: False)
- `fPIC`: Position Independent Code (default: True)
- `with_websocket`: Enable WebSocket support (default: True)
- `with_http`: Enable HTTP transport (default: True)
- `with_tests`: Build tests (default: False)

## Dependencies

- nlohmann_json/3.11.3
- openssl/3.2.0
- websocketpp/0.8.2 (optional)
- asio/1.28.0 (optional)
- libcurl/8.4.0 (optional)
- gtest/1.14.0 (test only)

## CI/CD Pipeline

The Conan Center CI will automatically:

1. Build for multiple configurations
2. Test on Linux, macOS, Windows
3. Verify all compilers (GCC, Clang, MSVC)
4. Check code standards
5. Validate dependencies

## Post-Approval

After PR is merged:

- Package available at: `conan install --requires=umicp/0.1.1`
- Listed on: https://conan.io/center/umicp
- Searchable via: `conan search umicp --remote=conancenter`

## Usage by Consumers

```bash
# Add to conanfile.txt
[requires]
umicp/0.1.1

[generators]
CMakeDeps
CMakeToolchain

[options]
umicp:with_websocket=True
umicp:with_http=True
```

Or in CMakeLists.txt:

```cmake
find_package(UMICP REQUIRED CONFIG)
target_link_libraries(your_app PRIVATE UMICP::UMICP)
```

## Troubleshooting

### Recipe Test Failures

```bash
# Clean cache
conan remove "umicp/*" -c

# Rebuild
conan create . --build=missing -vv
```

### Dependency Issues

```bash
# Check available versions
conan search nlohmann_json --remote=conancenter

# Update dependencies in conanfile.py
```

### Build Errors

```bash
# Enable verbose output
conan create . --build=missing -vv

# Check CMakeLists.txt compatibility
```

## Support

- GitHub Issues: https://github.com/hivellm/umicp/issues
- Conan Center: https://github.com/conan-io/conan-center-index
- Documentation: https://docs.conan.io/

---

**Status**: Ready for Conan Center submission  
**Version**: 0.1.1  
**License**: MIT

