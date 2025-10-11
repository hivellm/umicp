# UMICP PHP Bindings - Technical Guide

> **📝 TECHNICAL GUIDE**

**Version**: 0.1.0  
**Last Updated**: October 11, 2025

---

## 🚀 Quick Start

### Installation

```bash
# Clone & setup
git clone https://github.com/hivellm/umicp.git
cd umicp/bindings/php
./setup.sh
```

### Basic Usage

```php
<?php
require_once 'vendor/autoload.php';

use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

// Create envelope
$envelope = new Envelope(
    from: 'client-001',
    to: 'server-001',
    operation: OperationType::DATA,
    payload: 'Hello UMICP!'
);

// Serialize
$json = $envelope->toJson();

// Deserialize
$received = Envelope::fromJson($json);
echo "From: {$received->from}\n";
```

---

## 📝 Core Classes

### Envelope

```php
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;
use UMICP\Core\PayloadType;

$envelope = new Envelope(
    from: 'client',
    to: 'server',
    operation: OperationType::DATA,
    payload: 'data',
    payloadType: PayloadType::TEXT
);
```

### Matrix

```php
use UMICP\Core\Matrix;

$a = [1.0, 2.0, 3.0];
$b = [4.0, 5.0, 6.0];

// Operations
$dot = Matrix::dotProduct($a, $b);  // 32.0
$cos = Matrix::cosineSimilarity($a, $b);
$norm = Matrix::normalize($a);
```

---

## 🌐 WebSocket Transport

### Client

```php
use UMICP\Transport\WebSocketClient;

$client = new WebSocketClient('ws://localhost:8080');

// Events
$client->on('connected', function() {
    echo "Connected!\n";
});

$client->on('message', function($envelope) {
    echo "Received from: {$envelope->from}\n";
});

// Connect & send
$client->connect();
$client->send($envelope);
```

### Server

```php
use UMICP\Transport\WebSocketServer;

$server = new WebSocketServer('127.0.0.1', 8080);

$server->on('clientConnected', function($clientId) {
    echo "Client $clientId connected\n";
});

$server->on('message', function($clientId, $envelope) {
    // Echo back
    $response = new Envelope(/* ... */);
    $server->sendToClient($clientId, $response);
});

$server->start();
```

---

## 🔥 HTTP/2 Transport

### Client

```php
use UMICP\Transport\HttpClient;

$client = new HttpClient('http://localhost:8080');

// POST request
$response = $client->post('/api/messages', $envelope);

// GET request
$data = $client->get('/api/messages/123');
```

### Server

```php
use UMICP\Transport\HttpServer;

$server = new HttpServer('127.0.0.1', 8080);

$server->on('request', function($request, $response) {
    // Handle request
    $envelope = Envelope::fromJson($request->getBody());
    
    // Send response
    $reply = new Envelope(/* ... */);
    $response->send($reply->toJson());
});

$server->start();
```

---

## ⚡ Advanced Features

### Compression

```php
use UMICP\Core\CompressionManager;

$data = str_repeat("Large text...", 100);

// Compress
$compressed = CompressionManager::compress($data, 'gzip');

// Decompress
$decompressed = CompressionManager::decompress($compressed, 'gzip');
```

### Service Discovery

```php
use UMICP\Discovery\ServiceDiscovery;
use UMICP\Discovery\ServiceInfo;

$discovery = new ServiceDiscovery();

// Register
$service = new ServiceInfo(
    name: 'my-service',
    url: 'ws://localhost:8080',
    type: 'websocket'
);
$discovery->register($service);

// Find
$services = $discovery->findByName('my-service');
```

### Connection Pooling

```php
use UMICP\Pool\ConnectionPool;

$pool = new ConnectionPool(
    minSize: 2,
    maxSize: 10,
    factory: fn() => new WebSocketClient('ws://localhost:8080')
);

// Use connection
$conn = $pool->acquire();
$conn->send($envelope);
$pool->release($conn);
```

---

## 💡 Best Practices

### 1. Use Strict Types
```php
<?php declare(strict_types=1);
```

### 2. Type Hints
```php
function send(Envelope $envelope): void { }
```

### 3. Error Handling
```php
try {
    $client->connect();
} catch (ConnectionException $e) {
    // Handle
}
```

---

## 📚 Resources

- [README.md](./README.md)
- [STATUS.md](./STATUS.md)
- [ROADMAP.md](./ROADMAP.md)
- [REVIEWS.md](./REVIEWS.md)
- [Examples](../examples/)

---

*Last Updated: October 11, 2025*

