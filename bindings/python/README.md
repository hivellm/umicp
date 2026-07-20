# UMICP Python SDK

[![PyPI version](https://badge.fury.io/py/umicp-sdk.svg)](https://pypi.org/project/umicp-sdk/)
[![Python Versions](https://img.shields.io/pypi/pyversions/umicp-sdk.svg)](https://pypi.org/project/umicp-sdk/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

High-performance Python SDK for the Universal Matrix Inter-Communication Protocol (UMICP).

## 🚀 Status: **Production Release** (v0.3.0)

| Component | Status | Description |
|-----------|--------|-------------|
| **Envelope System** | ✅ Complete | Message envelope with JSON serialization |
| **Matrix Operations** | ✅ Complete | NumPy-powered matrix operations |
| **WebSocket Client** | ✅ Complete | Async WebSocket client |
| **WebSocket Server** | ✅ Complete | Async WebSocket server |
| **HTTP/2 Client** | ✅ Complete | HTTP/2 client with httpx |
| **HTTP/2 Server** | ✅ Complete | HTTP/2 server with aiohttp |
| **Multiplexed Peer** | ✅ Complete | Server + client architecture |
| **Event System** | ✅ Complete | Async event emitter |
| **Service Discovery** | ✅ Complete | Service registration and discovery |
| **Connection Pooling** | ✅ Complete | Generic connection pooling |
| **Compression** | ✅ Complete | GZIP/DEFLATE compression |

**Current Progress**: 100% Feature Complete ✅  
**Python Version**: 3.9+ required  
**Dependencies**: Modern async stack (asyncio, websockets, httpx, aiohttp)

---

## 📦 Installation

```bash
# Install from PyPI (recommended)
pip install umicp-sdk

# Or install specific version
pip install umicp-sdk==0.3.0

# Install from source
git clone https://github.com/hivellm/umicp
cd umicp/bindings/python
pip install -e .

# Install with dev dependencies
pip install -e ".[dev]"
```

**PyPI Package**: [https://pypi.org/project/umicp-sdk/](https://pypi.org/project/umicp-sdk/)

**Note**: Package name follows PEP 625 (normalized as `umicp_sdk` internally)

### Requirements

- Python 3.9+
- NumPy >= 1.24.0
- websockets >= 12.0
- httpx >= 0.27.0
- aiohttp >= 3.9.0

---

## 🎯 Quick Start

### Basic Envelope Usage

```python
from umicp_sdk import Envelope, EnvelopeBuilder, OperationType

# Create envelope
envelope = EnvelopeBuilder() \
    .from_id("client-001") \
    .to_id("server-001") \
    .operation(OperationType.DATA) \
    .capability("content-type", "application/json") \
    .build()

# Serialize
json_str = envelope.to_json()
print(f"Envelope: {json_str}")

# Deserialize
received = Envelope.from_json(json_str)
print(f"From: {received.from_id}, To: {received.to_id}")
```

### Matrix Operations

```python
import numpy as np
from umicp_sdk import Matrix

matrix = Matrix()

# Vector operations
v1 = np.array([1.0, 2.0, 3.0, 4.0], dtype=np.float32)
v2 = np.array([5.0, 6.0, 7.0, 8.0], dtype=np.float32)

# Dot product
dot_result = matrix.dot_product(v1, v2)
print(f"Dot product: {dot_result.result}")  # 70.0

# Cosine similarity
similarity = matrix.cosine_similarity(v1, v2)
print(f"Similarity: {similarity.similarity}")

# Matrix operations
m1 = np.array([[1, 2], [3, 4]], dtype=np.float32)
m2 = np.array([[5, 6], [7, 8]], dtype=np.float32)

result = matrix.multiply(m1, m2)
print(f"Product:\n{result.result}")
```

### WebSocket Client

```python
import asyncio
from umicp_sdk import WebSocketClient, Envelope, OperationType, EnvelopeBuilder

async def main():
    # Create client
    client = WebSocketClient("ws://localhost:8080")
    
    # Connect
    await client.connect()
    print("Connected!")
    
    # Send message
    envelope = EnvelopeBuilder() \
        .from_id("python-client") \
        .to_id("server") \
        .operation(OperationType.DATA) \
        .capability("message", "Hello from Python!") \
        .build()
    
    await client.send(envelope)
    print("Message sent!")
    
    # Receive message
    received = await client.receive()
    if received:
        print(f"Received: {received}")
    
    # Disconnect
    await client.disconnect()

asyncio.run(main())
```

### Multiplexed Peer

```python
import asyncio
from umicp_sdk import WebSocketPeer, EnvelopeBuilder, OperationType

async def main():
    # Create peer
    peer = WebSocketPeer("peer-001", port=8080)
    
    # Start server
    await peer.start()
    print("Peer server started")
    
    # Connect to another peer
    await peer.connect_to_peer("ws://localhost:8081", "peer-002")
    
    # Send to specific peer
    envelope = EnvelopeBuilder() \
        .from_id("peer-001") \
        .to_id("peer-002") \
        .operation(OperationType.DATA) \
        .capability("message", "Hello Peer!") \
        .build()
    
    await peer.send_to_peer("peer-002", envelope)
    
    # Broadcast to all peers
    await peer.broadcast(envelope)
    
    # Cleanup
    await peer.disconnect()

asyncio.run(main())
```

### Service Discovery

```python
import asyncio
from umicp_sdk import ServiceDiscovery, ServiceInfo

async def main():
    discovery = ServiceDiscovery()
    await discovery.start()
    
    # Register service
    service = ServiceInfo(
        id="my-service-001",
        name="MyService",
        version="1.0.0",
        capabilities={"type": "processor", "language": "python"},
        metadata={"description": "Example service"}
    )
    await discovery.register(service)
    
    # Find services
    services = await discovery.find_by_capability("type", "processor")
    print(f"Found {len(services)} services")
    
    # Cleanup
    await discovery.stop()

asyncio.run(main())
```

### Compression

```python
from umicp_sdk import Compression, CompressionType

# Compress data
data = b"Hello, UMICP! " * 100
compressed = Compression.compress(data, CompressionType.GZIP)
print(f"Original: {len(data)} bytes, Compressed: {len(compressed)} bytes")

# Decompress
decompressed = Compression.decompress(compressed, CompressionType.GZIP)
assert decompressed == data

# Check if compression is beneficial
if Compression.is_beneficial(len(data), len(compressed)):
    print("Compression saved space!")

# Get compression ratio
ratio = Compression.get_compression_ratio(len(data), len(compressed))
print(f"Compression ratio: {ratio:.2f}x")
```

---

## ✅ Features

### Core Features
- ✅ **Envelope System**: Type-safe message envelopes with JSON serialization
- ✅ **Builder Pattern**: Fluent API for envelope construction
- ✅ **Hash Generation**: SHA-256 envelope integrity
- ✅ **Validation**: Comprehensive field validation

### Matrix Operations
- ✅ **NumPy Integration**: High-performance operations
- ✅ **Vector Operations**: Add, subtract, scale, dot product
- ✅ **Matrix Operations**: Add, multiply, transpose, determinant, inverse
- ✅ **Similarity**: Cosine similarity, L2 normalization
- ✅ **Type Safety**: Full type hints

### Transport Layer
- ✅ **WebSocket**: Async client and server with websockets library
- ✅ **HTTP/2**: Client (httpx) and server (aiohttp)
- ✅ **Auto-reconnect**: Configurable reconnection logic
- ✅ **Statistics**: Message and byte counters

### Advanced Features
- ✅ **Multiplexed Peer**: Server + multiple clients in one
- ✅ **Auto-Handshake**: HELLO → ACK protocol
- ✅ **Event System**: Async event emitter with type-safe events
- ✅ **Service Discovery**: Registration, health tracking, capability matching
- ✅ **Connection Pooling**: Generic async pool with timeouts
- ✅ **Compression**: GZIP/DEFLATE with automatic size detection

### Developer Experience
- ✅ **Type Hints**: Full type annotations for IDE support
- ✅ **Async/Await**: Modern asyncio patterns
- ✅ **Error Handling**: Custom exception hierarchy
- ✅ **Documentation**: Comprehensive docstrings

---

## 🧪 Testing

### Test Suite

**133 comprehensive tests** with **97% code coverage**:

- ✅ **Unit Tests** (100+): All core modules
- ✅ **Integration Tests** (10+): End-to-end workflows
- ✅ **Async Tests** (30+): Full asyncio coverage
- ✅ **Advanced Tests** (15+): Edge cases and error handling
- ✅ **Compression Tests** (18): GZIP/DEFLATE compression

### Test Coverage by Module

| Module | Tests | Coverage |
|--------|-------|----------|
| types.py | 12 | 100% |
| error.py | 7 | 100% |
| envelope.py | 24 | 94% |
| envelope_advanced.py | 17 | 95% |
| matrix.py | 27 | 99% |
| matrix_advanced.py | 20 | 98% |
| compression.py | 18 | 100% |
| events.py | 8 | 98% |
| discovery.py | 8 | 100% |
| pool.py | 10 | 90% |
| peer/* | 8 | 100% |
| transport/* | 10 | 95% |
| integration | 7 | N/A |
| **Total** | **133** | **97%** |

### Running Tests

```bash
# Install dev dependencies
pip install -e ".[dev]"

# Run all tests
pytest

# Run with coverage report
pytest --cov=umicp --cov-report=html
pytest --cov=umicp --cov-report=term-missing

# Run specific test file
pytest tests/test_envelope.py
pytest tests/test_matrix.py
pytest tests/test_integration.py

# Run by category
pytest -m "not integration"  # Unit tests only
pytest -m integration        # Integration tests only

# Verbose output
pytest -v
pytest -vv  # Extra verbose

# Type checking
mypy umicp/

# Linting
ruff check umicp/

# Formatting
black umicp/
```

See [TEST_REPORT.md](TEST_REPORT.md) for detailed test coverage report.

---

## 📚 API Reference

### Core Classes

- **`Envelope`**: Message envelope
- **`EnvelopeBuilder`**: Fluent envelope builder
- **`Matrix`**: Matrix operations
- **`OperationType`**: Operation enum (CONTROL, DATA, ACK, ERROR, REQUEST, RESPONSE)
- **`PayloadType`**: Payload type enum
- **`EncodingType`**: Encoding type enum

### Transport

- **`WebSocketClient`**: Async WebSocket client
- **`WebSocketServer`**: Async WebSocket server
- **`HttpClient`**: HTTP/2 client
- **`HttpServer`**: HTTP/2 server

### Peer

- **`WebSocketPeer`**: Multiplexed peer
- **`PeerConnection`**: Peer connection management
- **`PeerInfo`**: Peer metadata
- **`HandshakeProtocol`**: Handshake utilities

### Advanced

- **`EventEmitter`**: Async event system
- **`ServiceDiscovery`**: Service registry
- **`ConnectionPool`**: Connection pooling
- **`Compression`**: Data compression utilities
- **`CompressionType`**: Compression algorithm enum (NONE, GZIP, DEFLATE, LZ4)
- **`CompressionError`**: Compression-specific exceptions

---

## 🔧 Development

```bash
# Clone repository
git clone https://github.com/hivellm/umicp
cd umicp/bindings/python

# Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install in editable mode with dev dependencies
pip install -e ".[dev]"

# Run tests
pytest

# Format code
black umicp/ tests/

# Type check
mypy umicp/

# Lint
ruff check umicp/
```

---

## 📋 Roadmap

### Completed ✅
- ✅ Core envelope system with JSON serialization
- ✅ Matrix operations with NumPy (SIMD-accelerated)
- ✅ WebSocket transport (client/server) with auto-reconnect
- ✅ HTTP/2 transport (client/server)
- ✅ Multiplexed peer architecture
- ✅ Event system with async handlers
- ✅ Service discovery with capability matching
- ✅ Connection pooling with health checks
- ✅ **Compression (GZIP/DEFLATE)** with automatic size detection ⭐ v0.1.3
- ✅ Full type hints (PEP 561)
- ✅ Comprehensive error handling
- ✅ 133 tests with 97% coverage

### Planned 📋
- 📋 LZ4 compression support
- 📋 TLS/SSL transport security
- 📋 Additional ML framework integrations (TensorFlow, PyTorch)
- 📋 Load balancing strategies
- 📋 Performance benchmarks
- 📋 Additional examples and tutorials

---

## 🔗 Part of HiveHub Ecosystem

UMICP Python bindings integrate with the HiveHub ecosystem:

- **Vectorizer**: Semantic search with UMICP communication
- **Task Queue**: Workflow orchestration
- **Agent Framework**: Multi-agent systems
- **Voxa**: Voice AI with agent coordination

---

## 📄 License

MIT License - See [LICENSE](LICENSE) file for details.

---

## 🤝 Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](../../CONTRIBUTING.md) for guidelines.

---

**Version**: 0.2.2  
**Status**: Production Release  
**Released**: October 17, 2025  
**Python**: 3.9+  
**PyPI**: https://pypi.org/project/umicp-python/  
**Repository**: https://github.com/hivellm/umicp

## 🆕 Custom Endpoint Support (v0.2.2)

```python
from umicp_sdk import HttpClient

# Method 1: Default endpoint (/message)
client = HttpClient("http://localhost:8000")

# Method 2: Custom endpoint (e.g., Vectorizer)
client = HttpClient("http://localhost:8000", path="/umicp")

# Method 3: Explicit path parameter
client = HttpClient(
    base_url="http://localhost:8000",
    path="/custom/endpoint"
)
```

**Compatibility:**
- Vectorizer: Use `path="/umicp"`
- Standard UMICP servers: Use default `path="/message"` (or omit)
- No breaking changes - fully backward compatible

