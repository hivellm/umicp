# GitHub Actions Workflows

This directory contains GitHub Actions workflows for automated builds, tests, and releases.

## Workflows

### 🚀 Release Build (`release.yml`)

Automatically builds and publishes **library packages** when you push a version tag.

#### What This Workflow Produces

**UMICP is a C++ library, not an executable.** The workflow generates development packages containing:

- 📚 **Shared Libraries**: `.so` (Linux), `.dylib` (macOS), `.dll` (Windows)
- 📚 **Static Libraries**: `.a` (Linux/macOS), `.lib` (Windows)
- 📄 **Header Files**: All public C++ headers
- ⚙️ **CMake Config**: `UMICPConfig.cmake` for easy integration

#### Trigger

```bash
# Create and push a version tag
git tag v0.1.1
git push origin v0.1.1
```

#### Build Process

1. **Multi-platform builds**:
   - Linux (Ubuntu) - x64
   - macOS (latest) - x64  
   - Windows (latest) - x64

2. **Compilation**:
   - Builds in Release mode with optimizations
   - Generates shared libraries
   - Runs all tests to ensure quality

3. **Packaging**:
   - **Linux**: `umicp-dev-linux-x64.tar.gz`
   - **macOS**: `umicp-dev-macos-x64.tar.gz`
   - **Windows**: `umicp-dev-windows-x64.zip`

4. **GitHub Release**:
   - Creates release automatically
   - Uploads development packages
   - Includes integration guide

#### Package Contents

Each package contains:

```
umicp-dev/
├── include/
│   └── umicp/
│       ├── envelope.hpp
│       ├── matrix.hpp
│       ├── frame.hpp
│       └── ... (all headers)
├── lib/
│   ├── libumicp.so      (Linux shared)
│   ├── libumicp.a       (Linux static)
│   ├── libumicp.dylib   (macOS shared)
│   ├── umicp.dll        (Windows shared)
│   ├── umicp.lib        (Windows import)
│   └── cmake/
│       └── UMICP/
│           └── UMICPConfig.cmake
└── share/
    └── doc/
        └── umicp/
            └── README.md
```

#### Using the Library

After downloading and extracting:

**1. System-wide Installation (Linux/macOS)**:
```bash
tar -xzf umicp-dev-linux-x64.tar.gz
cd umicp-dev
sudo cp -r include/* /usr/local/include/
sudo cp -r lib/* /usr/local/lib/
sudo ldconfig  # Linux only
```

**2. Local Installation (All platforms)**:
```bash
# Extract to your project
tar -xzf umicp-dev-linux-x64.tar.gz
mv umicp-dev /path/to/your/libs/
```

**3. CMake Integration**:
```cmake
# In your CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(MyApp)

# Point to UMICP
set(UMICP_DIR "/path/to/umicp-dev/lib/cmake/UMICP")
find_package(UMICP REQUIRED CONFIG)

# Link library
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE UMICP::UMICP)
```

**4. Manual Compilation**:
```bash
# Linux/macOS
g++ -std=c++17 -I/path/to/umicp-dev/include \
    main.cpp -L/path/to/umicp-dev/lib -lumicp -o my_app

# Windows (MSVC)
cl /std:c++17 /I"C:\path\to\umicp-dev\include" \
   main.cpp /link /LIBPATH:"C:\path\to\umicp-dev\lib" umicp.lib
```

#### Example Application

```cpp
#include <umicp/envelope.hpp>
#include <umicp/matrix.hpp>
#include <iostream>

int main() {
    // Create communication envelope
    umicp::Envelope env;
    env.setFrom("system-a");
    env.setTo("system-b");
    env.setOperation(umicp::OperationType::DATA);
    
    // Matrix operations
    umicp::Matrix mat(3, 3);
    mat.fill(1.0);
    double similarity = mat.cosineSimilarity(mat);
    
    std::cout << "UMICP library working!" << std::endl;
    std::cout << "Cosine similarity: " << similarity << std::endl;
    
    return 0;
}
```

## Usage Example

```bash
# 1. Develop and commit changes
git add .
git commit -m "Release library version 0.1.1"
git push origin main

# 2. Create version tag
git tag v0.1.1
git push origin v0.1.1

# 3. GitHub Actions automatically:
#    ✓ Builds library on Linux, macOS, Windows
#    ✓ Runs all tests
#    ✓ Packages libraries + headers
#    ✓ Creates GitHub Release
#    ✓ Uploads development packages

# 4. Users can download from:
#    https://github.com/hivellm/umicp/releases/tag/v0.1.1
```

## Platform-Specific Notes

### Linux
- Generates `.so` (shared) and `.a` (static) libraries
- Includes pkg-config files
- Creates `.deb` package for Debian/Ubuntu

### macOS
- Generates `.dylib` (shared) and `.a` (static) libraries
- Sets proper install names and rpath
- Compatible with Homebrew-style installation

### Windows
- Generates `.dll` (shared) and `.lib` (import/static) libraries
- Compatible with Visual Studio 2017+
- Includes debug symbols (.pdb)

## Requirements

### For Release Workflow

The workflow uses:
- `actions/checkout@v4` - Checkout code
- `actions/upload-artifact@v4` - Upload packages
- `softprops/action-gh-release@v1` - Create release
- `lukka/run-vcpkg@v11` - Windows dependencies

### Secrets (Optional)

For Conan Center publishing:
- `CONAN_LOGIN_USERNAME` - Conan username
- `CONAN_PASSWORD` - Conan password

## Troubleshooting

### Build Fails

Check that CMakeLists.txt properly defines:
- `BUILD_SHARED_LIBS` option
- Install targets for libraries and headers
- Export targets for CMake config

### Library Not Found at Runtime

**Linux/macOS**:
```bash
# Add library path
export LD_LIBRARY_PATH=/path/to/umicp-dev/lib:$LD_LIBRARY_PATH

# Or run ldconfig
sudo ldconfig
```

**Windows**:
```powershell
# Copy DLL to executable directory
copy C:\path\to\umicp-dev\lib\umicp.dll .

# Or add to PATH
$env:PATH += ";C:\path\to\umicp-dev\lib"
```

### CMake Can't Find Package

```cmake
# Explicitly set UMICP directory
set(UMICP_DIR "/path/to/umicp-dev/lib/cmake/UMICP")
find_package(UMICP REQUIRED CONFIG)
```

## CI/CD Best Practices

1. **Library Versioning**: Use semantic versioning (v0.1.1)
2. **ABI Stability**: Document breaking changes
3. **Test Coverage**: Run tests before release
4. **Documentation**: Update API docs with release
5. **Backward Compatibility**: Maintain when possible

## Support

- GitHub Issues: https://github.com/hivellm/umicp/issues
- Documentation: https://github.com/hivellm/umicp/tree/main/cpp/docs
- Examples: https://github.com/hivellm/umicp/tree/main/cpp/examples

---

**Last Updated**: 2025-10-10  
**Package Type**: Development Library (headers + libs)  
**Target**: Developers integrating UMICP into their projects
