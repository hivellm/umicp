# Build Guide - UMICP PHP Bindings

## Quick Build

### Linux/macOS

```bash
# Complete setup
./setup.sh

# Or manual:
./build-cpp.sh
composer install
php examples/03_complete_demo.php
```

### Windows

```powershell
# Build C++
.\build-cpp.ps1

# Install PHP dependencies
composer install

# Run demo
php examples\03_complete_demo.php
```

## Step-by-Step

### 1. Build C++ Core

**Linux/macOS**:
```bash
cd umicp/cpp
mkdir -p build && cd build
cmake .. -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release
make -j$(nproc) umicp_core
# Output: libumicp_core.so (Linux) or libumicp_core.dylib (macOS)
```

**Windows**:
```powershell
cd umicp\cpp
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=ON
cmake --build . --config Release --target umicp_core
# Output: Release\umicp_core.dll
```

### 2. Install PHP Dependencies

```bash
cd umicp/bindings/php
composer install
```

### 3. Configure

```bash
# Copy config
cp config/umicp.example.php config/umicp.php

# Edit library path
vim config/umicp.php
# Update 'lib_path' to point to built library
```

**Linux**:
```php
'lib_path' => __DIR__ . '/../../../cpp/build/libumicp_core.so',
```

**macOS**:
```php
'lib_path' => __DIR__ . '/../../../cpp/build/libumicp_core.dylib',
```

**Windows**:
```php
'lib_path' => __DIR__ . '/../../../cpp/build/Release/umicp_core.dll',
```

### 4. Verify

```bash
# Validate structure
php validate-structure.php

# Run tests
./vendor/bin/phpunit

# Run examples
php examples/01_basic_envelope.php
php examples/02_matrix_operations.php
php examples/03_complete_demo.php
```

## Troubleshooting

### FFI Not Enabled

```bash
# Check
php -m | grep FFI

# If missing, enable in php.ini
echo "extension=ffi" >> /etc/php/8.1/cli/php.ini
echo "ffi.enable=1" >> /etc/php/8.1/cli/php.ini
```

### Library Not Found

```bash
# Check library exists
ls -la umicp/cpp/build/libumicp_core.*

# Check symbols
nm -D umicp/cpp/build/libumicp_core.so | grep umicp_

# Update config
vim config/umicp.php
```

### Build Errors

```bash
# Install build tools
sudo apt-get install build-essential cmake

# Install dependencies
sudo apt-get install libjson-c-dev libssl-dev zlib1g-dev

# Clean and rebuild
cd umicp/cpp/build
rm -rf *
cmake .. -DBUILD_SHARED_LIBS=ON
make
```

## Success Indicators

✅ `php validate-structure.php` passes  
✅ Examples run without errors  
✅ Tests pass  
✅ No memory leaks

---

**Next**: Run examples and tests!

