# UMICP PHP Bindings - FFI Integration Guide

[![PHP](https://img.shields.io/badge/PHP-8.1%2B-777BB4.svg)](https://www.php.net/)
[![FFI](https://img.shields.io/badge/FFI-Enabled-blue.svg)](https://www.php.net/manual/en/book.ffi.php)

> **Complete guide to integrating PHP with C++ core via FFI**

## Table of Contents

- [Introduction](#introduction)
- [Prerequisites](#prerequisites)
- [C++ Core Preparation](#c-core-preparation)
- [FFI Header Creation](#ffi-header-creation)
- [PHP FFI Bridge](#php-ffi-bridge)
- [Type Conversion](#type-conversion)
- [Memory Management](#memory-management)
- [Error Handling](#error-handling)
- [Performance Optimization](#performance-optimization)
- [Testing Strategy](#testing-strategy)
- [Troubleshooting](#troubleshooting)

---

## Introduction

PHP Foreign Function Interface (FFI) allows PHP to call C functions and manipulate C data structures directly. This guide explains how to integrate PHP with the UMICP C++ core library.

### Why FFI?

**Advantages**:
- ✅ Direct C++ integration without writing C extensions
- ✅ No PHP compilation required
- ✅ Rapid development and iteration
- ✅ Native C++ performance for critical operations
- ✅ Easy to maintain and update

**Disadvantages**:
- ⚠️ Requires FFI extension (PHP 7.4+, stable in 8.0+)
- ⚠️ Manual memory management
- ⚠️ Type safety is developer's responsibility
- ⚠️ Small performance overhead vs native extensions

### Architecture Overview

```
┌──────────────────────────────────────────────────┐
│              PHP Application                      │
└──────────────────┬───────────────────────────────┘
                   │
┌──────────────────▼───────────────────────────────┐
│          PHP FFI Bridge Classes                   │
│   (Envelope, Matrix, Transport wrappers)         │
└──────────────────┬───────────────────────────────┘
                   │
┌──────────────────▼───────────────────────────────┐
│          FFI Type Conversion Layer                │
│   (PHP ↔ C type conversion)                      │
└──────────────────┬───────────────────────────────┘
                   │
┌──────────────────▼───────────────────────────────┐
│          PHP FFI Extension                        │
│   (Built into PHP 8.1+)                          │
└──────────────────┬───────────────────────────────┘
                   │
┌──────────────────▼───────────────────────────────┐
│          C FFI Header (umicp_core.h)             │
│   (C-compatible API definitions)                 │
└──────────────────┬───────────────────────────────┘
                   │
┌──────────────────▼───────────────────────────────┐
│     C++ Core Library (libumicp_core.so/.dylib)   │
│   (Native C++ implementation with C wrappers)    │
└──────────────────────────────────────────────────┘
```

---

## Prerequisites

### System Requirements

#### PHP Requirements

```bash
# Check PHP version
php -v
# Required: PHP 8.1 or higher

# Check if FFI extension is loaded
php -m | grep FFI
# Should output: FFI

# Check FFI configuration
php -i | grep ffi
# ffi.enable => 1 or "preload"
```

#### Enable FFI in php.ini

```ini
; php.ini configuration
[FFI]
ffi.enable = 1              ; Enable in all contexts (development)
; ffi.enable = "preload"    ; Production: only in preloaded files

; Optional: increase memory limit for large FFI operations
memory_limit = 512M
```

#### C++ Build Tools

```bash
# Linux (Ubuntu/Debian)
sudo apt-get install build-essential cmake gcc g++

# macOS
xcode-select --install
brew install cmake

# Windows
# Install Visual Studio 2019+ with C++ tools
# Install CMake from https://cmake.org/download/
```

---

## C++ Core Preparation

### Step 1: Create C API Wrapper

The C++ core needs C-compatible wrapper functions for FFI.

**File**: `umicp/cpp/src/ffi_wrapper.cpp`

```cpp
#include "../include/envelope.hpp"
#include "../include/matrix.hpp"
#include <cstring>
#include <memory>

// Opaque pointer types for FFI
struct UMICPEnvelope {
    umicp::Envelope* impl;
};

struct UMICPMatrix {
    umicp::Matrix* impl;
};

// Export C functions
extern "C" {

// ============================================================================
// Envelope API
// ============================================================================

/**
 * Create a new envelope instance
 * @return Pointer to envelope or NULL on failure
 */
UMICPEnvelope* umicp_envelope_create() {
    try {
        auto* wrapper = new UMICPEnvelope();
        wrapper->impl = new umicp::Envelope();
        return wrapper;
    } catch (...) {
        return nullptr;
    }
}

/**
 * Destroy envelope instance
 * @param envelope Envelope to destroy
 */
void umicp_envelope_destroy(UMICPEnvelope* envelope) {
    if (envelope) {
        delete envelope->impl;
        delete envelope;
    }
}

/**
 * Set envelope sender
 * @param envelope Target envelope
 * @param from Sender identifier (null-terminated string)
 */
void umicp_envelope_set_from(UMICPEnvelope* envelope, const char* from) {
    if (envelope && from) {
        envelope->impl->setFrom(std::string(from));
    }
}

/**
 * Get envelope sender
 * @param envelope Source envelope
 * @return Sender identifier (caller must not free)
 */
const char* umicp_envelope_get_from(UMICPEnvelope* envelope) {
    if (!envelope) return nullptr;
    
    static thread_local std::string result;
    result = envelope->impl->getFrom();
    return result.c_str();
}

/**
 * Set envelope recipient
 * @param envelope Target envelope
 * @param to Recipient identifier
 */
void umicp_envelope_set_to(UMICPEnvelope* envelope, const char* to) {
    if (envelope && to) {
        envelope->impl->setTo(std::string(to));
    }
}

/**
 * Get envelope recipient
 * @param envelope Source envelope
 * @return Recipient identifier
 */
const char* umicp_envelope_get_to(UMICPEnvelope* envelope) {
    if (!envelope) return nullptr;
    
    static thread_local std::string result;
    result = envelope->impl->getTo();
    return result.c_str();
}

/**
 * Set operation type
 * @param envelope Target envelope
 * @param operation Operation type (0=CONTROL, 1=DATA, 2=ACK, 3=ERROR)
 */
void umicp_envelope_set_operation(UMICPEnvelope* envelope, int operation) {
    if (envelope) {
        envelope->impl->setOperation(static_cast<umicp::OperationType>(operation));
    }
}

/**
 * Get operation type
 * @param envelope Source envelope
 * @return Operation type as integer
 */
int umicp_envelope_get_operation(UMICPEnvelope* envelope) {
    if (!envelope) return -1;
    return static_cast<int>(envelope->impl->getOperation());
}

/**
 * Set message identifier
 * @param envelope Target envelope
 * @param messageId Message ID
 */
void umicp_envelope_set_message_id(UMICPEnvelope* envelope, const char* messageId) {
    if (envelope && messageId) {
        envelope->impl->setMessageId(std::string(messageId));
    }
}

/**
 * Set capabilities (as JSON string)
 * @param envelope Target envelope
 * @param json JSON string with capabilities
 */
void umicp_envelope_set_capabilities(UMICPEnvelope* envelope, const char* json) {
    if (envelope && json) {
        envelope->impl->setCapabilitiesFromJson(std::string(json));
    }
}

/**
 * Serialize envelope to JSON
 * @param envelope Source envelope
 * @return JSON string (caller must not free, thread-local storage)
 */
const char* umicp_envelope_serialize(UMICPEnvelope* envelope) {
    if (!envelope) return nullptr;
    
    static thread_local std::string result;
    result = envelope->impl->serialize();
    return result.c_str();
}

/**
 * Deserialize envelope from JSON
 * @param json JSON string
 * @return New envelope instance or NULL on failure
 */
UMICPEnvelope* umicp_envelope_deserialize(const char* json) {
    if (!json) return nullptr;
    
    try {
        auto* wrapper = new UMICPEnvelope();
        wrapper->impl = new umicp::Envelope();
        wrapper->impl->deserialize(std::string(json));
        return wrapper;
    } catch (...) {
        return nullptr;
    }
}

/**
 * Validate envelope
 * @param envelope Envelope to validate
 * @return 1 if valid, 0 if invalid
 */
int umicp_envelope_validate(UMICPEnvelope* envelope) {
    if (!envelope) return 0;
    return envelope->impl->validate() ? 1 : 0;
}

// ============================================================================
// Matrix API
// ============================================================================

/**
 * Create matrix instance
 */
UMICPMatrix* umicp_matrix_create() {
    try {
        auto* wrapper = new UMICPMatrix();
        wrapper->impl = new umicp::Matrix();
        return wrapper;
    } catch (...) {
        return nullptr;
    }
}

/**
 * Destroy matrix instance
 */
void umicp_matrix_destroy(UMICPMatrix* matrix) {
    if (matrix) {
        delete matrix->impl;
        delete matrix;
    }
}

/**
 * Calculate dot product
 * @param matrix Matrix instance
 * @param a First vector
 * @param b Second vector
 * @param size Vector size
 * @return Dot product result
 */
double umicp_matrix_dot_product(UMICPMatrix* matrix, const float* a, const float* b, int size) {
    if (!matrix || !a || !b || size <= 0) return 0.0;
    
    return matrix->impl->dotProduct(a, b, size);
}

/**
 * Calculate cosine similarity
 * @param matrix Matrix instance
 * @param a First vector
 * @param b Second vector
 * @param size Vector size
 * @return Cosine similarity (-1 to 1)
 */
double umicp_matrix_cosine_similarity(UMICPMatrix* matrix, const float* a, const float* b, int size) {
    if (!matrix || !a || !b || size <= 0) return 0.0;
    
    return matrix->impl->cosineSimilarity(a, b, size);
}

/**
 * Vector addition
 * @param matrix Matrix instance
 * @param a First vector
 * @param b Second vector
 * @param result Result vector (must be pre-allocated)
 * @param size Vector size
 */
void umicp_matrix_vector_add(UMICPMatrix* matrix, const float* a, const float* b, float* result, int size) {
    if (!matrix || !a || !b || !result || size <= 0) return;
    
    matrix->impl->vectorAdd(a, b, result, size);
}

/**
 * Matrix multiplication
 * @param matrix Matrix instance
 * @param a First matrix
 * @param b Second matrix
 * @param result Result matrix (must be pre-allocated)
 * @param m Rows of A
 * @param n Columns of A / Rows of B
 * @param p Columns of B
 */
void umicp_matrix_multiply(UMICPMatrix* matrix, const float* a, const float* b, float* result, int m, int n, int p) {
    if (!matrix || !a || !b || !result || m <= 0 || n <= 0 || p <= 0) return;
    
    matrix->impl->matrixMultiply(a, b, result, m, n, p);
}

} // extern "C"
```

### Step 2: Build Shared Library

**File**: `umicp/cpp/CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.15)
project(umicp_core VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build as shared library for FFI
add_library(umicp_core SHARED
    src/envelope.cpp
    src/matrix.cpp
    src/ffi_wrapper.cpp
    # ... other source files
)

target_include_directories(umicp_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Enable position-independent code
set_target_properties(umicp_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Install library
install(TARGETS umicp_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
```

**Build Commands**:

```bash
cd umicp/cpp
mkdir -p build && cd build

# Build shared library
cmake .. -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Output: libumicp_core.so (Linux), libumicp_core.dylib (macOS), umicp_core.dll (Windows)
# Location: build/libumicp_core.so

# Verify library
file build/libumicp_core.so
nm -D build/libumicp_core.so | grep umicp_  # List exported symbols
```

---

## FFI Header Creation

### Step 3: Create C Header for FFI

**File**: `umicp/bindings/php/ffi/umicp_core.h`

```c
#ifndef UMICP_FFI_H
#define UMICP_FFI_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Opaque Types
// ============================================================================

typedef struct UMICPEnvelope UMICPEnvelope;
typedef struct UMICPMatrix UMICPMatrix;

// ============================================================================
// Envelope Functions
// ============================================================================

UMICPEnvelope* umicp_envelope_create(void);
void umicp_envelope_destroy(UMICPEnvelope* envelope);

void umicp_envelope_set_from(UMICPEnvelope* envelope, const char* from);
const char* umicp_envelope_get_from(UMICPEnvelope* envelope);

void umicp_envelope_set_to(UMICPEnvelope* envelope, const char* to);
const char* umicp_envelope_get_to(UMICPEnvelope* envelope);

void umicp_envelope_set_operation(UMICPEnvelope* envelope, int operation);
int umicp_envelope_get_operation(UMICPEnvelope* envelope);

void umicp_envelope_set_message_id(UMICPEnvelope* envelope, const char* messageId);
void umicp_envelope_set_capabilities(UMICPEnvelope* envelope, const char* json);

const char* umicp_envelope_serialize(UMICPEnvelope* envelope);
UMICPEnvelope* umicp_envelope_deserialize(const char* json);

int umicp_envelope_validate(UMICPEnvelope* envelope);

// ============================================================================
// Matrix Functions
// ============================================================================

UMICPMatrix* umicp_matrix_create(void);
void umicp_matrix_destroy(UMICPMatrix* matrix);

double umicp_matrix_dot_product(UMICPMatrix* matrix, const float* a, const float* b, int size);
double umicp_matrix_cosine_similarity(UMICPMatrix* matrix, const float* a, const float* b, int size);

void umicp_matrix_vector_add(UMICPMatrix* matrix, const float* a, const float* b, float* result, int size);
void umicp_matrix_multiply(UMICPMatrix* matrix, const float* a, const float* b, float* result, int m, int n, int p);

#ifdef __cplusplus
}
#endif

#endif // UMICP_FFI_H
```

---

## PHP FFI Bridge

### Step 4: Implement FFI Bridge

**File**: `src/FFI/FFIBridge.php`

```php
<?php

namespace UMICP\FFI;

use FFI;
use FFI\CData;
use UMICP\Exception\FFIException;

/**
 * FFI Bridge to C++ core library
 */
class FFIBridge
{
    private static ?FFIBridge $instance = null;
    private FFI $ffi;
    private string $libPath;
    private string $headerPath;
    
    /**
     * Private constructor for singleton
     */
    private function __construct(string $libPath, string $headerPath)
    {
        $this->libPath = $libPath;
        $this->headerPath = $headerPath;
        
        $this->validateEnvironment();
        $this->initializeFFI();
    }
    
    /**
     * Get singleton instance
     */
    public static function getInstance(?string $libPath = null, ?string $headerPath = null): self
    {
        if (self::$instance === null) {
            // Load from config if not provided
            if ($libPath === null || $headerPath === null) {
                $config = Config::load();
                $libPath ??= $config['ffi']['lib_path'];
                $headerPath ??= $config['ffi']['header_path'];
            }
            
            self::$instance = new self($libPath, $headerPath);
        }
        
        return self::$instance;
    }
    
    /**
     * Validate FFI environment
     */
    private function validateEnvironment(): void
    {
        if (!extension_loaded('ffi')) {
            throw new FFIException('FFI extension is not loaded. Enable it in php.ini');
        }
        
        if (!file_exists($this->libPath)) {
            throw new FFIException("C++ library not found: {$this->libPath}");
        }
        
        if (!file_exists($this->headerPath)) {
            throw new FFIException("FFI header not found: {$this->headerPath}");
        }
        
        // Check if FFI is enabled
        $ffiEnabled = ini_get('ffi.enable');
        if ($ffiEnabled !== '1' && $ffiEnabled !== 'preload') {
            throw new FFIException('FFI is disabled. Set ffi.enable=1 in php.ini');
        }
    }
    
    /**
     * Initialize FFI interface
     */
    private function initializeFFI(): void
    {
        try {
            $header = file_get_contents($this->headerPath);
            
            if ($header === false) {
                throw new FFIException("Failed to read FFI header: {$this->headerPath}");
            }
            
            $this->ffi = FFI::cdef($header, $this->libPath);
            
        } catch (\FFI\Exception $e) {
            throw new FFIException(
                "Failed to initialize FFI: " . $e->getMessage(),
                $e->getCode(),
                $e
            );
        }
    }
    
    /**
     * Get FFI instance
     */
    public function getFFI(): FFI
    {
        return $this->ffi;
    }
    
    // ========================================================================
    // Envelope Operations
    // ========================================================================
    
    /**
     * Create envelope
     */
    public function createEnvelope(): CData
    {
        $envelope = $this->ffi->umicp_envelope_create();
        
        if ($envelope === null) {
            throw new FFIException('Failed to create envelope');
        }
        
        return $envelope;
    }
    
    /**
     * Destroy envelope
     */
    public function destroyEnvelope(CData $envelope): void
    {
        $this->ffi->umicp_envelope_destroy($envelope);
    }
    
    // ========================================================================
    // Matrix Operations
    // ========================================================================
    
    /**
     * Create matrix
     */
    public function createMatrix(): CData
    {
        $matrix = $this->ffi->umicp_matrix_create();
        
        if ($matrix === null) {
            throw new FFIException('Failed to create matrix');
        }
        
        return $matrix;
    }
    
    /**
     * Destroy matrix
     */
    public function destroyMatrix(CData $matrix): void
    {
        $this->ffi->umicp_matrix_destroy($matrix);
    }
    
    /**
     * Get library information
     */
    public function getInfo(): array
    {
        return [
            'lib_path' => $this->libPath,
            'header_path' => $this->headerPath,
            'ffi_version' => phpversion('ffi'),
            'php_version' => PHP_VERSION,
        ];
    }
}
```

### Step 5: Configuration

**File**: `src/FFI/Config.php`

```php
<?php

namespace UMICP\FFI;

class Config
{
    private static ?array $config = null;
    private static ?string $configPath = null;
    
    /**
     * Load configuration
     */
    public static function load(?string $configPath = null): array
    {
        if (self::$config === null || ($configPath !== null && $configPath !== self::$configPath)) {
            self::$configPath = $configPath ?? self::findConfigFile();
            
            if (!file_exists(self::$configPath)) {
                throw new \RuntimeException("Config file not found: " . self::$configPath);
            }
            
            self::$config = require self::$configPath;
        }
        
        return self::$config;
    }
    
    /**
     * Get configuration value
     */
    public static function get(string $key, mixed $default = null): mixed
    {
        $config = self::load();
        
        // Support dot notation: 'ffi.lib_path'
        $keys = explode('.', $key);
        $value = $config;
        
        foreach ($keys as $k) {
            if (!isset($value[$k])) {
                return $default;
            }
            $value = $value[$k];
        }
        
        return $value;
    }
    
    /**
     * Find config file
     */
    private static function findConfigFile(): string
    {
        $possiblePaths = [
            __DIR__ . '/../../config/umicp.php',
            __DIR__ . '/../../../config/umicp.php',
            getcwd() . '/config/umicp.php',
        ];
        
        foreach ($possiblePaths as $path) {
            if (file_exists($path)) {
                return $path;
            }
        }
        
        throw new \RuntimeException('Config file not found in standard locations');
    }
}
```

**File**: `config/umicp.php`

```php
<?php

return [
    'ffi' => [
        'lib_path' => __DIR__ . '/../../../cpp/build/libumicp_core.so',
        'header_path' => __DIR__ . '/../ffi/umicp_core.h',
    ],
    
    'transport' => [
        'default_timeout' => 10000,
        'max_reconnect_attempts' => 3,
        'heartbeat_interval' => 30000,
        'reconnect_delay' => 5000,
    ],
    
    'server' => [
        'default_port' => 20081,
        'default_path' => '/umicp',
        'compression' => true,
        'max_payload' => 100 * 1024 * 1024, // 100MB
    ],
    
    'performance' => [
        'enable_opcache_preload' => false,
        'batch_size' => 1000,
    ],
];
```

---

## Type Conversion

### Step 6: Type Converter Implementation

**File**: `src/FFI/TypeConverter.php`

```php
<?php

namespace UMICP\FFI;

use FFI;
use FFI\CData;

/**
 * Type conversion between PHP and C
 */
class TypeConverter
{
    /**
     * Convert PHP array to C float array
     *
     * @param array<float> $phpArray PHP array
     * @return CData C float array
     */
    public static function phpArrayToCFloatArray(array $phpArray): CData
    {
        $size = count($phpArray);
        
        if ($size === 0) {
            throw new \InvalidArgumentException('Array cannot be empty');
        }
        
        $cArray = FFI::new("float[$size]");
        
        foreach ($phpArray as $i => $value) {
            $cArray[$i] = (float) $value;
        }
        
        return $cArray;
    }
    
    /**
     * Convert C float array to PHP array
     *
     * @param CData $cArray C array
     * @param int $size Array size
     * @return array<float> PHP array
     */
    public static function cFloatArrayToPhpArray(CData $cArray, int $size): array
    {
        $result = [];
        
        for ($i = 0; $i < $size; $i++) {
            $result[] = $cArray[$i];
        }
        
        return $result;
    }
    
    /**
     * Convert PHP string to C string
     *
     * @param string $phpString PHP string
     * @return CData C string
     */
    public static function phpStringToCString(string $phpString): CData
    {
        $len = strlen($phpString);
        $cString = FFI::new("char[$len + 1]", false);
        
        FFI::memcpy($cString, $phpString, $len);
        $cString[$len] = "\0";
        
        return $cString;
    }
    
    /**
     * Convert C string to PHP string
     *
     * @param CData $cString C string
     * @return string PHP string
     */
    public static function cStringToPhpString(CData $cString): string
    {
        return FFI::string($cString);
    }
    
    /**
     * Convert PHP array to JSON C string
     *
     * @param array $array PHP associative array
     * @return string JSON string
     */
    public static function phpArrayToJsonCString(array $array): string
    {
        $json = json_encode($array);
        
        if ($json === false) {
            throw new \RuntimeException('JSON encoding failed: ' . json_last_error_msg());
        }
        
        return $json;
    }
    
    /**
     * Convert JSON C string to PHP array
     *
     * @param string $json JSON string
     * @return array PHP array
     */
    public static function jsonCStringToPhpArray(string $json): array
    {
        $array = json_decode($json, true);
        
        if ($array === null && json_last_error() !== JSON_ERROR_NONE) {
            throw new \RuntimeException('JSON decoding failed: ' . json_last_error_msg());
        }
        
        return $array ?? [];
    }
}
```

---

## Memory Management

### Step 7: Memory Management with Auto-Cleanup

**File**: `src/FFI/Traits/AutoCleanup.php`

```php
<?php

namespace UMICP\FFI\Traits;

/**
 * Automatic resource cleanup on destruction
 */
trait AutoCleanup
{
    /** @var array<callable> */
    private array $cleanupCallbacks = [];
    
    /**
     * Register cleanup callback
     *
     * @param callable $callback Cleanup function
     */
    protected function registerCleanup(callable $callback): void
    {
        $this->cleanupCallbacks[] = $callback;
    }
    
    /**
     * Destructor - execute all cleanup callbacks
     */
    public function __destruct()
    {
        foreach ($this->cleanupCallbacks as $callback) {
            try {
                $callback();
            } catch (\Throwable $e) {
                // Log error but don't throw in destructor
                error_log(sprintf(
                    '[UMICP] Cleanup error in %s: %s',
                    static::class,
                    $e->getMessage()
                ));
            }
        }
        
        $this->cleanupCallbacks = [];
    }
    
    /**
     * Manually trigger cleanup (useful for testing)
     */
    public function cleanup(): void
    {
        $this->__destruct();
    }
}
```

**Usage Example**:

```php
<?php

namespace UMICP\Core;

use UMICP\FFI\FFIBridge;
use UMICP\FFI\Traits\AutoCleanup;
use FFI\CData;

class Envelope
{
    use AutoCleanup;
    
    private CData $nativeEnvelope;
    private FFIBridge $ffi;
    
    public function __construct()
    {
        $this->ffi = FFIBridge::getInstance();
        $this->nativeEnvelope = $this->ffi->createEnvelope();
        
        // Register automatic cleanup
        $this->registerCleanup(function () {
            $this->ffi->destroyEnvelope($this->nativeEnvelope);
        });
    }
    
    // ... rest of implementation
}
```

---

## Error Handling

### Step 8: Exception Hierarchy

**File**: `src/Exception/UMICPException.php`

```php
<?php

namespace UMICP\Exception;

/**
 * Base UMICP exception
 */
class UMICPException extends \Exception
{
    protected ?array $context = null;
    
    public function __construct(
        string $message = "",
        int $code = 0,
        ?\Throwable $previous = null,
        ?array $context = null
    ) {
        parent::__construct($message, $code, $previous);
        $this->context = $context;
    }
    
    public function getContext(): ?array
    {
        return $this->context;
    }
}
```

**File**: `src/Exception/FFIException.php`

```php
<?php

namespace UMICP\Exception;

/**
 * FFI-related exception
 */
class FFIException extends UMICPException
{
    private ?string $ffiError = null;
    private ?string $libraryPath = null;
    
    public function __construct(
        string $message = "",
        int $code = 0,
        ?\Throwable $previous = null,
        ?string $ffiError = null,
        ?string $libraryPath = null
    ) {
        parent::__construct($message, $code, $previous);
        $this->ffiError = $ffiError;
        $this->libraryPath = $libraryPath;
    }
    
    public function getFFIError(): ?string
    {
        return $this->ffiError;
    }
    
    public function getLibraryPath(): ?string
    {
        return $this->libraryPath;
    }
}
```

---

## Performance Optimization

### OPcache Preloading (PHP 7.4+)

**File**: `preload.php`

```php
<?php

// preload.php - Preload FFI for production performance

opcache_compile_file(__DIR__ . '/src/FFI/FFIBridge.php');
opcache_compile_file(__DIR__ . '/src/FFI/TypeConverter.php');
opcache_compile_file(__DIR__ . '/src/FFI/Config.php');
opcache_compile_file(__DIR__ . '/src/Core/Envelope.php');
opcache_compile_file(__DIR__ . '/src/Core/Matrix.php');

// Initialize FFI in preload (if ffi.enable="preload")
require_once __DIR__ . '/vendor/autoload.php';

$ffi = \UMICP\FFI\FFIBridge::getInstance();
```

**php.ini**:

```ini
[opcache]
opcache.enable=1
opcache.enable_cli=1
opcache.preload=/path/to/umicp/bindings/php/preload.php
opcache.preload_user=www-data
```

### Performance Tips

1. **Batch Operations**: Group FFI calls
2. **Reuse Objects**: Cache `Matrix` and `Envelope` instances
3. **Minimize Conversions**: Work with FFI types directly when possible
4. **Use JIT**: Enable PHP 8.1+ JIT for hot code paths

---

## Testing Strategy

### Unit Tests

**File**: `tests/Unit/FFI/FFIBridgeTest.php`

```php
<?php

namespace UMICP\Tests\Unit\FFI;

use PHPUnit\Framework\TestCase;
use UMICP\FFI\FFIBridge;
use UMICP\Exception\FFIException;

class FFIBridgeTest extends TestCase
{
    public function testFFIInitialization(): void
    {
        $ffi = FFIBridge::getInstance();
        
        $this->assertInstanceOf(FFIBridge::class, $ffi);
        $this->assertInstanceOf(\FFI::class, $ffi->getFFI());
    }
    
    public function testEnvelopeCreation(): void
    {
        $ffi = FFIBridge::getInstance();
        $envelope = $ffi->createEnvelope();
        
        $this->assertNotNull($envelope);
        
        $ffi->destroyEnvelope($envelope);
    }
    
    public function testMatrixCreation(): void
    {
        $ffi = FFIBridge::getInstance();
        $matrix = $ffi->createMatrix();
        
        $this->assertNotNull($matrix);
        
        $ffi->destroyMatrix($matrix);
    }
    
    public function testInvalidLibraryPath(): void
    {
        $this->expectException(FFIException::class);
        
        new FFIBridge('/invalid/path/lib.so', '/invalid/header.h');
    }
}
```

---

## Troubleshooting

### Common Issues

#### Issue 1: FFI Extension Not Loaded

```
Error: FFI extension is not loaded
```

**Solution**:
```bash
# Check if FFI is available
php -m | grep FFI

# If not available, install/enable it
# Ubuntu/Debian
sudo apt-get install php8.1-ffi

# Enable in php.ini
echo "extension=ffi" >> /etc/php/8.1/cli/php.ini
echo "ffi.enable=1" >> /etc/php/8.1/cli/php.ini
```

#### Issue 2: Library Not Found

```
Error: C++ library not found: /path/to/libumicp_core.so
```

**Solution**:
```bash
# Verify library exists
ls -la /path/to/libumicp_core.so

# Check library dependencies
ldd /path/to/libumicp_core.so

# Add library path to LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/path/to/lib:$LD_LIBRARY_PATH

# Or update config
cp config/umicp.example.php config/umicp.php
# Edit lib_path in config/umicp.php
```

#### Issue 3: Symbol Not Found

```
Error: undefined symbol: umicp_envelope_create
```

**Solution**:
```bash
# Check exported symbols
nm -D /path/to/libumicp_core.so | grep umicp_

# Rebuild with proper export
cd umicp/cpp/build
rm -rf *
cmake .. -DBUILD_SHARED_LIBS=ON
make clean && make
```

#### Issue 4: Memory Leaks

```
PHP Fatal error: Out of memory
```

**Solution**:
- Ensure cleanup callbacks are registered
- Check for circular references
- Use `cleanup()` method explicitly if needed
- Run with `valgrind` to detect C++ leaks

```bash
valgrind --leak-check=full php test.php
```

---

## Complete Example

**File**: `examples/01_basic_ffi.php`

```php
<?php

require_once __DIR__ . '/../vendor/autoload.php';

use UMICP\FFI\FFIBridge;
use UMICP\Core\Envelope;
use UMICP\Core\Matrix;
use UMICP\Core\OperationType;

echo "UMICP PHP FFI Example\n";
echo "=====================\n\n";

// Initialize FFI
echo "1. Initializing FFI...\n";
$ffi = FFIBridge::getInstance();
print_r($ffi->getInfo());
echo "\n";

// Test Envelope
echo "2. Testing Envelope...\n";
$envelope = new Envelope(
    from: 'sender-001',
    to: 'receiver-001',
    operation: OperationType::DATA,
    messageId: 'msg-' . uniqid(),
    capabilities: [
        'content-type' => 'application/json',
        'priority' => 'high'
    ]
);

echo "   Envelope created: {$envelope->getFrom()} → {$envelope->getTo()}\n";

$json = $envelope->serialize();
echo "   Serialized: " . substr($json, 0, 100) . "...\n";

$deserialized = Envelope::deserialize($json);
echo "   Deserialized: {$deserialized->getFrom()} → {$deserialized->getTo()}\n";
echo "   Valid: " . ($envelope->validate() ? 'Yes' : 'No') . "\n\n";

// Test Matrix
echo "3. Testing Matrix operations...\n";
$matrix = new Matrix();

$vec1 = [1.0, 2.0, 3.0, 4.0];
$vec2 = [5.0, 6.0, 7.0, 8.0];

$dotProduct = $matrix->dotProduct($vec1, $vec2);
echo "   Dot product: $dotProduct\n";

$similarity = $matrix->cosineSimilarity($vec1, $vec2);
echo "   Cosine similarity: $similarity\n";

$sum = $matrix->vectorAdd($vec1, $vec2);
echo "   Vector sum: [" . implode(', ', $sum) . "]\n\n";

echo "✅ FFI integration working correctly!\n";
```

**Run**:
```bash
php examples/01_basic_ffi.php
```

---

**Status**: FFI integration guide complete  
**Next**: Begin implementation following the roadmap

