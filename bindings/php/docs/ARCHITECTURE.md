# UMICP PHP Bindings - Architecture

[![PHP](https://img.shields.io/badge/PHP-8.1%2B-777BB4.svg)](https://www.php.net/)
[![FFI](https://img.shields.io/badge/FFI-Enabled-blue.svg)](https://www.php.net/manual/en/book.ffi.php)
[![Status](https://img.shields.io/badge/Status-Planning-orange.svg)](#)

> **Status**: 📋 Planning Phase - Documentation and Architecture Design  
> **Target PHP Version**: 8.1+  
> **Core Integration**: C++ via FFI  
> **Based on**: TypeScript Implementation (Production-Ready)

## Overview

The UMICP PHP bindings provide high-performance inter-model communication capabilities for PHP applications, enabling:

- **🚀 High Performance**: Sub-millisecond latency through C++ core via FFI
- **🔒 Secure Communication**: Envelope-based messaging with capability negotiation
- **📦 Binary Protocol**: Efficient serialization with optional compression
- **⚡ Real-time**: WebSocket transport with async support
- **🤝 Peer-to-Peer**: Multiplexed architecture - each peer is both server AND client
- **🌐 PHP Ecosystem**: Native integration with PHP standards and frameworks

## Architecture Layers

```
┌──────────────────────────────────────────────────────────────┐
│                    PHP Application Layer                      │
│            (Symfony, Laravel, Custom Apps)                   │
└──────────────────────────────────────────────────────────────┘
                              ▼
┌──────────────────────────────────────────────────────────────┐
│                  UMICP PHP Bindings API                       │
│  ┌────────────────┬─────────────────┬─────────────────────┐  │
│  │  Core Classes  │   Transport     │   Async Runtime     │  │
│  │  - Envelope    │   - WebSocket   │   - ReactPHP        │  │
│  │  - Matrix      │   - HTTP/2      │   - Swoole          │  │
│  │  - Frame       │   - MultiplexPeer│  - Amp             │  │
│  └────────────────┴─────────────────┴─────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
                              ▼
┌──────────────────────────────────────────────────────────────┐
│                      PHP FFI Layer                            │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  FFI Wrapper Classes                                   │  │
│  │  - EnvelopeFFI    - MatrixFFI    - TransportFFI      │  │
│  │  - Automatic type conversion PHP ↔ C++                │  │
│  │  - Memory management and lifecycle                    │  │
│  └────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
                              ▼
┌──────────────────────────────────────────────────────────────┐
│               C++ Core Library (umicp_core)                   │
│  ┌────────────────────────────────────────────────────────┐  │
│  │  • Native C++ Implementation                          │  │
│  │  • SIMD-optimized matrix operations                   │  │
│  │  • Binary protocol serialization                      │  │
│  │  • High-performance transport layer                   │  │
│  └────────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────────┘
```

## Core Components

### 1. FFI Integration Layer

**Purpose**: Bridge between PHP and C++ core using PHP FFI extension

**Key Components**:
- `FFIBridge`: Main FFI initialization and management
- `TypeConverter`: Automatic conversion between PHP and C types
- `MemoryManager`: Safe memory allocation and cleanup
- `ErrorHandler`: FFI error translation to PHP exceptions

**Example Structure**:
```php
namespace UMICP\FFI;

class FFIBridge {
    private FFI $ffi;
    private string $headerPath;
    
    public function __construct(string $libPath, string $headerPath) {
        $this->ffi = FFI::cdef(
            file_get_contents($headerPath),
            $libPath
        );
    }
    
    public function createEnvelope(): CData { /* ... */ }
    public function destroyEnvelope(CData $envelope): void { /* ... */ }
}
```

### 2. Core Classes

**Envelope**: Message container with metadata and capabilities

```php
namespace UMICP\Core;

class Envelope {
    public function __construct(
        private ?string $from = null,
        private ?string $to = null,
        private OperationType $operation = OperationType::DATA,
        private ?string $messageId = null,
        private array $capabilities = []
    ) {}
    
    public function setFrom(string $from): self;
    public function setTo(string $to): self;
    public function setOperation(OperationType $op): self;
    public function serialize(): string;
    public static function deserialize(string $json): self;
    public function validate(): bool;
}
```

**Matrix**: High-performance matrix operations via FFI

```php
namespace UMICP\Core;

class Matrix {
    public function dotProduct(array $a, array $b): float;
    public function cosineSimilarity(array $a, array $b): float;
    public function vectorAdd(array $a, array $b): array;
    public function matrixMultiply(array $a, array $b, int $m, int $n, int $p): array;
}
```

### 3. Transport Layer

#### WebSocket Client

```php
namespace UMICP\Transport;

class WebSocketClient {
    public function __construct(array $config);
    public function connect(): Promise;
    public function disconnect(): Promise;
    public function send(Envelope $envelope): bool;
    public function isConnected(): bool;
    public function on(string $event, callable $handler): void;
    public function getStats(): array;
}
```

#### WebSocket Server

```php
namespace UMICP\Transport;

class WebSocketServer {
    public function __construct(array $config);
    public function start(): Promise;
    public function stop(): Promise;
    public function broadcast(Envelope $envelope): int;
    public function sendToClient(string $clientId, Envelope $envelope): bool;
    public function on(string $event, callable $handler): void;
    public function getStats(): array;
}
```

#### Multiplexed Peer (Key Feature)

```php
namespace UMICP\Transport;

class MultiplexedPeer {
    public function __construct(
        private string $peerId,
        private ?array $serverConfig = null,
        private array $metadata = []
    ) {}
    
    // Dual functionality: Server + Multiple Clients
    public function connectToPeer(string $url, array $metadata = []): Promise;
    public function disconnectPeer(string $peerId): bool;
    public function sendToPeer(string $peerId, Envelope $envelope): bool;
    public function broadcast(Envelope $envelope, ?string $excludePeerId = null): int;
    public function getPeers(): array;
    public function shutdown(): Promise;
    
    // Event handlers
    public function on(string $event, callable $handler): void;
    // Events: 'peer:connect', 'peer:disconnect', 'peer:ready', 'message', 'data', 'error'
}
```

## Async Runtime Support

### Option 1: ReactPHP (Recommended)

**Advantages**:
- Pure PHP, no extensions required
- Large ecosystem
- Well-documented
- PSR-7/PSR-15 compatible

**Example**:
```php
use React\EventLoop\Loop;
use UMICP\Transport\MultiplexedPeer;

$peer = new MultiplexedPeer(
    peerId: 'php-agent-1',
    serverConfig: ['port' => 20081, 'path' => '/umicp']
);

$peer->on('message', function(Envelope $envelope, PeerConnection $peer) {
    echo "Message from {$peer->id}: " . json_encode($envelope->getCapabilities());
});

Loop::run();
```

### Option 2: Swoole

**Advantages**:
- Native async/coroutines
- Higher performance
- Built-in WebSocket support

**Example**:
```php
use Swoole\Coroutine;
use UMICP\Transport\MultiplexedPeer;

Coroutine\run(function() {
    $peer = new MultiplexedPeer('php-agent-1');
    
    $peer->on('message', function(Envelope $envelope, PeerConnection $peer) {
        // Handle message
    });
    
    $peer->connectToPeer('ws://localhost:20082/umicp');
});
```

### Option 3: Amp

**Advantages**:
- Modern PHP 8+ syntax
- Fiber-based concurrency
- Clean API

## Design Principles

### 1. PHP Idioms

- **PSR Standards**: Follow PSR-4 (autoloading), PSR-12 (coding style), PSR-7 (HTTP)
- **Type Safety**: Use PHP 8.1+ strict types, enums, and attributes
- **Composer**: Standard package management
- **Namespaces**: `UMICP\*` namespace hierarchy

### 2. Performance

- **FFI for Core**: Leverage C++ performance for critical operations
- **Lazy Loading**: Initialize FFI only when needed
- **Memory Management**: Proper cleanup of FFI resources
- **Connection Pooling**: Reuse WebSocket connections

### 3. Developer Experience

- **Fluent API**: Method chaining where appropriate
- **Clear Errors**: Descriptive exceptions with context
- **Documentation**: PHPDoc comments for all public APIs
- **Examples**: Comprehensive usage examples

### 4. Framework Agnostic

- **No Framework Lock-in**: Works with any PHP application
- **Integration Helpers**: Optional adapters for Symfony, Laravel, etc.
- **PSR Compliance**: Easy integration with PSR-compliant frameworks

## Comparison with TypeScript Implementation

| Feature | TypeScript | PHP (Planned) |
|---------|-----------|---------------|
| **Core Language** | JavaScript/TypeScript | PHP 8.1+ |
| **C++ Integration** | N-API Native Modules | FFI |
| **Async Model** | Promises/async-await | ReactPHP/Swoole/Amp |
| **Package Manager** | npm | Composer |
| **WebSocket** | `ws` library | Ratchet/Swoole |
| **Type System** | TypeScript types | PHP 8.1+ types + attributes |
| **Performance** | ~10-50ms latency | Target: ~20-100ms latency |
| **Memory Overhead** | ~200 bytes/peer | Target: ~500 bytes/peer |
| **Multiplexed Peer** | ✅ Implemented | 📋 Planned |
| **Auto Protocol** | ✅ Handshake/ACK | 📋 Planned |
| **Event System** | EventEmitter | PSR-14 Events or custom |

## PHP-Specific Considerations

### 1. FFI Requirements

- **PHP Version**: PHP 8.1+ with FFI extension enabled
- **php.ini**: `ffi.enable=1` or `ffi.enable="preload"`
- **Preloading**: Optional OPcache preloading for performance
- **Platform**: Linux, macOS, Windows (with compatible C++ compiler)

### 2. Memory Management

```php
class EnvelopeFFI {
    private CData $nativeEnvelope;
    
    public function __construct() {
        $this->nativeEnvelope = $this->ffi->createEnvelope();
    }
    
    public function __destruct() {
        // Explicit cleanup
        $this->ffi->destroyEnvelope($this->nativeEnvelope);
    }
}
```

### 3. Error Handling

```php
namespace UMICP\Exception;

class UMICPException extends \Exception {}
class FFIException extends UMICPException {}
class TransportException extends UMICPException {}
class SerializationException extends UMICPException {}
class ValidationException extends UMICPException {}
```

### 4. Configuration

```php
// config/umicp.php
return [
    'ffi' => [
        'lib_path' => __DIR__ . '/../build/Release/umicp_core.so',
        'header_path' => __DIR__ . '/../ffi/umicp_core.h',
    ],
    'transport' => [
        'default_timeout' => 10000,
        'max_reconnect_attempts' => 3,
        'heartbeat_interval' => 30000,
    ],
    'server' => [
        'default_port' => 20081,
        'default_path' => '/umicp',
        'compression' => true,
    ],
];
```

## Integration Examples

### Symfony Integration

```php
// src/Service/UMICPService.php
namespace App\Service;

use UMICP\Transport\MultiplexedPeer;
use Symfony\Contracts\EventDispatcher\EventDispatcherInterface;

class UMICPService {
    public function __construct(
        private EventDispatcherInterface $dispatcher,
        private array $umicpConfig
    ) {}
    
    public function createPeer(string $peerId): MultiplexedPeer {
        return new MultiplexedPeer($peerId, $this->umicpConfig['server']);
    }
}
```

### Laravel Integration

```php
// app/Providers/UMICPServiceProvider.php
namespace App\Providers;

use Illuminate\Support\ServiceProvider;
use UMICP\Transport\MultiplexedPeer;

class UMICPServiceProvider extends ServiceProvider {
    public function register(): void {
        $this->app->singleton(MultiplexedPeer::class, function ($app) {
            return new MultiplexedPeer(
                config('umicp.peer_id'),
                config('umicp.server')
            );
        });
    }
}
```

## Performance Targets

### Benchmarks (Estimated)

Based on TypeScript implementation and FFI overhead:

| Operation | TypeScript | PHP Target | Notes |
|-----------|-----------|------------|-------|
| Envelope Creation | ~1ms | ~2-3ms | FFI overhead |
| Serialization | ~8ms | ~10-15ms | JSON + FFI |
| Deserialization | ~8ms | ~10-15ms | JSON parsing |
| WebSocket Handshake | ~10-50ms | ~20-100ms | Network bound |
| Message Throughput | 10,000/sec | 5,000-8,000/sec | CPU bound |
| Matrix Dot Product | <1ms | ~1-2ms | FFI call overhead |
| Memory per Peer | ~200 bytes | ~500 bytes | PHP overhead |

### Optimization Strategies

1. **FFI Preloading**: Use OPcache preload to reduce FFI initialization
2. **Connection Pooling**: Reuse WebSocket connections
3. **Batch Operations**: Group FFI calls when possible
4. **JIT Compilation**: Enable PHP 8.1+ JIT for hot paths
5. **Message Batching**: Send multiple envelopes in one WebSocket frame

## Testing Strategy

### Unit Tests

```php
namespace UMICP\Tests\Unit;

use PHPUnit\Framework\TestCase;
use UMICP\Core\Envelope;

class EnvelopeTest extends TestCase {
    public function testEnvelopeCreation(): void {
        $envelope = new Envelope(from: 'sender', to: 'receiver');
        $this->assertEquals('sender', $envelope->getFrom());
        $this->assertEquals('receiver', $envelope->getTo());
    }
}
```

### Integration Tests

```php
namespace UMICP\Tests\Integration;

use PHPUnit\Framework\TestCase;
use UMICP\Transport\WebSocketClient;
use UMICP\Transport\WebSocketServer;

class WebSocketIntegrationTest extends TestCase {
    public function testClientServerCommunication(): void {
        // Test client-server message exchange
    }
}
```

### Performance Tests

```php
namespace UMICP\Tests\Performance;

use UMICP\Core\Envelope;

class EnvelopePerformanceTest extends TestCase {
    public function testSerializationPerformance(): void {
        $start = microtime(true);
        
        for ($i = 0; $i < 10000; $i++) {
            $envelope = new Envelope();
            $serialized = $envelope->serialize();
        }
        
        $duration = (microtime(true) - $start) * 1000;
        $this->assertLessThan(100, $duration / 10000);
    }
}
```

## Documentation Plan

1. **API Reference**: Complete PHPDoc documentation
2. **Getting Started Guide**: Installation and first steps
3. **Examples**: Practical usage scenarios
4. **Framework Integration**: Symfony, Laravel, etc.
5. **Performance Guide**: Optimization tips
6. **Troubleshooting**: Common issues and solutions

## Next Steps

1. ✅ Complete architecture documentation
2. 📋 Create detailed implementation roadmap
3. 📋 Write API specification
4. 📋 Create FFI integration guide
5. 📋 Implement FFI bridge
6. 📋 Implement core classes
7. 📋 Implement transport layer
8. 📋 Write comprehensive tests
9. 📋 Create examples
10. 📋 Performance benchmarking

---

**Status**: Architecture design complete, ready for implementation planning  
**Next Document**: [IMPLEMENTATION_ROADMAP.md](./IMPLEMENTATION_ROADMAP.md)

