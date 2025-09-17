# UMICP Installation Guide

/**
 * @fileoverview Complete installation and setup guide for UMICP
 * @description Step-by-step instructions for installing UMICP on various platforms
 * and setting up development environments.
 * 
 * @group Getting Started
 * @order 1
 * @since 1.0.0
 */

## Overview

UMICP (Universal Matrix-based Inter-Component Protocol) is a high-performance communication protocol designed for distributed systems, machine learning applications, and real-time data processing. This guide will walk you through installing UMICP on your system.

## System Requirements

### Minimum Requirements

| Component | Requirement |
|-----------|-------------|
| **Operating System** | Windows 10+, macOS 10.15+, Linux (Ubuntu 18.04+, CentOS 7+) |
| **CPU** | x64 processor with SSE4.2 support |
| **Memory** | 4GB RAM (8GB+ recommended) |
| **Storage** | 500MB free space |
| **Network** | TCP/IP networking capability |

### Recommended Requirements

| Component | Recommendation |
|-----------|----------------|
| **CPU** | Modern x64 processor with AVX2 support for optimal SIMD performance |
| **Memory** | 16GB+ RAM for high-throughput applications |
| **Storage** | SSD storage for better I/O performance |
| **Network** | Gigabit Ethernet for high-bandwidth applications |

### Development Requirements

| Tool | Version | Purpose |
|------|---------|---------|
| **CMake** | 3.16+ | Build system |
| **C++ Compiler** | GCC 9+, Clang 10+, MSVC 2019+ | Core compilation |
| **Node.js** | 16+ | Bindings and tooling |
| **Python** | 3.8+ | Optional Python bindings |
| **Git** | 2.20+ | Source code management |

## Installation Methods

### Method 1: Package Managers (Recommended)

#### Ubuntu/Debian

```bash
# Add UMICP repository
curl -fsSL https://packages.umicp.org/gpg | sudo apt-key add -
echo "deb https://packages.umicp.org/ubuntu $(lsb_release -cs) main" | sudo tee /etc/apt/sources.list.d/umicp.list

# Update package list and install
sudo apt update
sudo apt install umicp-dev umicp-tools

# Install Node.js bindings (optional)
npm install -g umicp
```

#### CentOS/RHEL/Fedora

```bash
# Add UMICP repository
sudo dnf config-manager --add-repo https://packages.umicp.org/rpm/umicp.repo

# Install UMICP
sudo dnf install umicp-devel umicp-tools

# Install Node.js bindings (optional)
npm install -g umicp
```

#### macOS (Homebrew)

```bash
# Add UMICP tap
brew tap umicp/tap

# Install UMICP
brew install umicp

# Install Node.js bindings (optional)
npm install -g umicp
```

#### Windows (Chocolatey)

```powershell
# Install UMICP
choco install umicp

# Install Node.js bindings (optional)
npm install -g umicp
```

### Method 2: Pre-built Binaries

Download pre-built binaries from the [UMICP Releases](https://github.com/umicp/umicp/releases) page:

1. **Download** the appropriate binary for your platform
2. **Extract** the archive to your desired location
3. **Add** the `bin` directory to your system PATH
4. **Set** environment variables (see Configuration section)

#### Linux/macOS

```bash
# Download and extract (replace with latest version)
wget https://github.com/umicp/umicp/releases/download/v1.0.0/umicp-1.0.0-linux-x64.tar.gz
tar -xzf umicp-1.0.0-linux-x64.tar.gz

# Move to installation directory
sudo mv umicp-1.0.0 /opt/umicp

# Add to PATH
echo 'export PATH="/opt/umicp/bin:$PATH"' >> ~/.bashrc
echo 'export UMICP_HOME="/opt/umicp"' >> ~/.bashrc
source ~/.bashrc

# Verify installation
umicp --version
```

#### Windows

```powershell
# Download and extract
Invoke-WebRequest -Uri "https://github.com/umicp/umicp/releases/download/v1.0.0/umicp-1.0.0-windows-x64.zip" -OutFile "umicp.zip"
Expand-Archive -Path "umicp.zip" -DestinationPath "C:\Program Files\UMICP"

# Add to PATH
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\Program Files\UMICP\bin", [EnvironmentVariableTarget]::Machine)
[Environment]::SetEnvironmentVariable("UMICP_HOME", "C:\Program Files\UMICP", [EnvironmentVariableTarget]::Machine)

# Verify installation (restart PowerShell first)
umicp --version
```

### Method 3: Build from Source

#### Prerequisites

First, install the required build tools:

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake git pkg-config \
    libssl-dev libzlib1g-dev libjson-c-dev libcbor-dev \
    libavx2-dev libsimd-dev nodejs npm
```

**CentOS/RHEL/Fedora:**
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git pkg-config openssl-devel \
    zlib-devel json-c-devel libcbor-devel nodejs npm
```

**macOS:**
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install dependencies via Homebrew
brew install cmake git pkg-config openssl zlib json-c libcbor node
```

**Windows:**
```powershell
# Install Visual Studio 2019+ with C++ workload
# Install Git for Windows
# Install CMake

# Install vcpkg for dependencies
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

# Install dependencies
.\vcpkg install openssl zlib nlohmann-json libcbor --triplet x64-windows
```

#### Build Steps

```bash
# Clone the repository
git clone https://github.com/umicp/umicp.git
cd umicp

# Create build directory
mkdir build && cd build

# Configure build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DUMICP_BUILD_TESTS=ON \
         -DUMICP_BUILD_BINDINGS=ON \
         -DUMICP_ENABLE_SIMD=ON

# Build (use appropriate number of cores)
cmake --build . --config Release -j$(nproc)

# Run tests
ctest --output-on-failure

# Install
sudo cmake --install .
```

#### Windows Build

```powershell
# Clone and build
git clone https://github.com/umicp/umicp.git
cd umicp
mkdir build
cd build

# Configure with vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake" `
         -DCMAKE_BUILD_TYPE=Release `
         -DUMICP_BUILD_TESTS=ON `
         -DUMICP_BUILD_BINDINGS=ON

# Build
cmake --build . --config Release

# Run tests
ctest -C Release --output-on-failure

# Install
cmake --install . --config Release
```

## Language Bindings Installation

### Node.js/TypeScript Bindings

#### From npm Registry

```bash
# Install globally
npm install -g umicp

# Or install locally in project
npm install umicp
```

#### From Source

```bash
cd umicp/bindings/nodejs
npm install
npm run build
npm test
npm link  # For global installation
```

#### Usage Example

```typescript
import { Protocol, TransportFactory } from 'umicp';

// Create protocol instance
const protocol = new Protocol('my-node');

// Configure
protocol.configure({
    enableBinary: true,
    maxMessageSize: 1024 * 1024,
    connectionTimeout: 5000
});

// Set transport
const transport = TransportFactory.createWebSocket({
    host: 'localhost',
    port: 8080
});
protocol.setTransport(transport);

// Connect and use
await protocol.connect();
await protocol.sendData('target-node', Buffer.from('Hello, UMICP!'));
```

### Python Bindings

#### From PyPI

```bash
# Install from PyPI
pip install umicp

# Or with conda
conda install -c umicp umicp
```

#### From Source

```bash
cd umicp/bindings/python
pip install -r requirements.txt
python setup.py build
python setup.py install
```

#### Usage Example

```python
import umicp

# Create protocol
protocol = umicp.Protocol('my-node')

# Configure
config = umicp.Config()
config.enable_binary = True
config.max_message_size = 1024 * 1024
protocol.configure(config)

# Set transport
transport = umicp.TransportFactory.create_websocket({
    'host': 'localhost',
    'port': 8080
})
protocol.set_transport(transport)

# Connect and use
protocol.connect()
protocol.send_data('target-node', b'Hello, UMICP!')
```

## Configuration

### Environment Variables

Set these environment variables for optimal UMICP operation:

```bash
# Core configuration
export UMICP_HOME="/opt/umicp"                    # Installation directory
export UMICP_CONFIG_PATH="/etc/umicp/config"     # Configuration files
export UMICP_LOG_LEVEL="INFO"                    # Logging level
export UMICP_LOG_PATH="/var/log/umicp"           # Log directory

# Performance tuning
export UMICP_ENABLE_SIMD="true"                  # Enable SIMD optimizations
export UMICP_THREAD_POOL_SIZE="8"               # Worker thread count
export UMICP_BUFFER_SIZE="65536"                # Default buffer size

# Security
export UMICP_KEY_PATH="/etc/umicp/keys"          # Cryptographic keys
export UMICP_CERT_PATH="/etc/umicp/certs"       # TLS certificates
export UMICP_REQUIRE_TLS="true"                 # Require TLS encryption

# Networking
export UMICP_DEFAULT_PORT="8080"                # Default listening port
export UMICP_MAX_CONNECTIONS="1000"             # Maximum concurrent connections
export UMICP_KEEPALIVE_INTERVAL="30"            # Keepalive interval (seconds)
```

### Configuration Files

#### Main Configuration (`/etc/umicp/umicp.conf`)

```ini
[core]
# Core protocol settings
node_id = "default-node"
max_message_size = 10485760  # 10MB
connection_timeout = 30000   # 30 seconds
enable_binary = true
preferred_format = "binary"

[transport]
# Transport layer settings
default_type = "websocket"
listen_address = "0.0.0.0"
listen_port = 8080
max_connections = 1000
keepalive_interval = 30

[security]
# Security settings
require_auth = false
require_encryption = false
key_file = "/etc/umicp/keys/node.key"
cert_file = "/etc/umicp/certs/node.crt"
ca_file = "/etc/umicp/certs/ca.crt"

[performance]
# Performance tuning
enable_simd = true
thread_pool_size = 8
buffer_pool_size = 1000
compression_enabled = true
compression_level = 6

[logging]
# Logging configuration
level = "INFO"
file = "/var/log/umicp/umicp.log"
max_file_size = "100MB"
max_files = 10
console_output = true
```

#### Transport Configuration (`/etc/umicp/transport.conf`)

```ini
[websocket]
# WebSocket transport settings
enabled = true
port = 8080
max_frame_size = 16777216  # 16MB
ping_interval = 30
pong_timeout = 10

[http2]
# HTTP/2 transport settings
enabled = true
port = 8443
tls_required = true
max_streams = 1000
window_size = 65536

[tcp]
# Raw TCP transport settings
enabled = false
port = 8081
nodelay = true
keepalive = true
```

## Verification

### Basic Functionality Test

Create a simple test to verify your installation:

**C++ Test (`test_installation.cpp`):**

```cpp
#include <iostream>
#include "umicp/protocol.h"
#include "umicp/transport.h"

int main() {
    try {
        // Create protocol
        umicp::Protocol protocol("test-node");
        
        // Configure
        umicp::UMICPConfig config;
        config.max_message_size = 1024;
        config.enable_binary = true;
        
        auto result = protocol.configure(config);
        if (!result.is_success()) {
            std::cerr << "Configuration failed" << std::endl;
            return 1;
        }
        
        std::cout << "UMICP installation verified successfully!" << std::endl;
        std::cout << "Node ID: " << protocol.get_node_id() << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

Compile and run:

```bash
g++ -std=c++17 test_installation.cpp -lumicp -o test_installation
./test_installation
```

**Node.js Test (`test_installation.js`):**

```javascript
const { Protocol } = require('umicp');

async function testInstallation() {
    try {
        // Create protocol
        const protocol = new Protocol('test-node');
        
        // Configure
        const result = protocol.configure({
            maxMessageSize: 1024,
            enableBinary: true
        });
        
        if (!result.success) {
            console.error('Configuration failed');
            process.exit(1);
        }
        
        console.log('UMICP Node.js bindings verified successfully!');
        console.log('Node ID:', protocol.getNodeId());
        
    } catch (error) {
        console.error('Error:', error.message);
        process.exit(1);
    }
}

testInstallation();
```

Run:

```bash
node test_installation.js
```

### Performance Benchmark

Run the included benchmark to verify performance:

```bash
# C++ benchmark
umicp-benchmark --test matrix --size 1000 --iterations 1000

# Node.js benchmark
npm run benchmark

# Expected output:
# Matrix Operations: 12.5 GFLOPS
# Message Throughput: 1.2 GB/s
# Connection Latency: 0.8 ms
```

### Network Connectivity Test

Test network functionality:

```bash
# Start UMICP server
umicp-server --port 8080 --verbose

# In another terminal, test client
umicp-client --host localhost --port 8080 --message "Hello, UMICP!"

# Expected output:
# Connected to localhost:8080
# Sent: Hello, UMICP!
# Received: Echo: Hello, UMICP!
# Connection closed successfully
```

## Troubleshooting

### Common Issues

#### 1. Missing Dependencies

**Problem:** Build fails with missing library errors

**Solution:**
```bash
# Ubuntu/Debian
sudo apt install libssl-dev libzlib1g-dev libjson-c-dev

# CentOS/RHEL
sudo dnf install openssl-devel zlib-devel json-c-devel

# macOS
brew install openssl zlib json-c
```

#### 2. SIMD Not Available

**Problem:** SIMD optimizations not working

**Solution:**
```bash
# Check CPU features
cat /proc/cpuinfo | grep -E "(sse|avx)"

# Rebuild with SIMD disabled if necessary
cmake .. -DUMICP_ENABLE_SIMD=OFF
```

#### 3. Permission Denied

**Problem:** Cannot bind to port or access files

**Solution:**
```bash
# Run with appropriate permissions
sudo umicp-server --port 80

# Or use non-privileged port
umicp-server --port 8080

# Fix file permissions
sudo chown -R $USER:$USER /etc/umicp
sudo chmod -R 644 /etc/umicp/*.conf
```

#### 4. Node.js Binding Issues

**Problem:** Cannot load native module

**Solution:**
```bash
# Rebuild native modules
npm rebuild

# Clear npm cache
npm cache clean --force

# Reinstall
npm uninstall umicp
npm install umicp
```

### Debug Mode

Enable debug mode for detailed troubleshooting:

```bash
# Set debug environment
export UMICP_LOG_LEVEL="DEBUG"
export UMICP_DEBUG="true"

# Run with verbose output
umicp-server --verbose --debug
```

### Log Analysis

Check logs for issues:

```bash
# View recent logs
tail -f /var/log/umicp/umicp.log

# Search for errors
grep -i error /var/log/umicp/umicp.log

# Analyze connection issues
grep -i "connection\|transport" /var/log/umicp/umicp.log
```

## Next Steps

After successful installation:

1. **Read the [Quick Start Guide](quick-start.md)** for basic usage
2. **Explore [API Documentation](../api/)** for detailed reference
3. **Check out [Examples](examples.md)** for real-world usage patterns
4. **Join the [Community](https://github.com/umicp/umicp/discussions)** for support

## Support

If you encounter issues:

- **Documentation**: [https://umicp.github.io/docs/](https://umicp.github.io/docs/)
- **Issues**: [https://github.com/umicp/umicp/issues](https://github.com/umicp/umicp/issues)
- **Discussions**: [https://github.com/umicp/umicp/discussions](https://github.com/umicp/umicp/discussions)
- **Email**: support@umicp.org
