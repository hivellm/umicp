# UMICP PHP Bindings - API Specification

[![PHP](https://img.shields.io/badge/PHP-8.1%2B-777BB4.svg)](https://www.php.net/)
[![PSR-12](https://img.shields.io/badge/PSR--12-Compliant-green.svg)](https://www.php-fig.org/psr/psr-12/)

> **API Version**: 1.0.0  
> **Based on**: TypeScript Implementation  
> **Namespace**: `UMICP\`

## Table of Contents

- [Core Classes](#core-classes)
  - [Envelope](#envelope)
  - [Matrix](#matrix)
  - [Frame](#frame)
- [Enums](#enums)
- [Transport Layer](#transport-layer)
  - [WebSocketClient](#websocketclient)
  - [WebSocketServer](#websocketserver)
  - [MultiplexedPeer](#multiplexedpeer)
- [FFI Layer](#ffi-layer)
- [Exceptions](#exceptions)
- [Configuration](#configuration)
- [Events](#events)

---

## Core Classes

### Envelope

Message container with metadata and capabilities.

#### Namespace
```php
namespace UMICP\Core;
```

#### Class Definition

```php
class Envelope
{
    /**
     * Create a new envelope
     *
     * @param string|null $from Source identifier
     * @param string|null $to Destination identifier
     * @param OperationType $operation Operation type
     * @param string|null $messageId Unique message identifier
     * @param array<string, string> $capabilities Message metadata
     * @param PayloadHint|null $payloadHint Payload type hint
     */
    public function __construct(
        ?string $from = null,
        ?string $to = null,
        OperationType $operation = OperationType::DATA,
        ?string $messageId = null,
        array $capabilities = [],
        ?PayloadHint $payloadHint = null
    );
    
    /**
     * Set the sender identifier
     *
     * @param string $from Sender identifier
     * @return self Fluent interface
     */
    public function setFrom(string $from): self;
    
    /**
     * Get the sender identifier
     *
     * @return string|null Sender identifier or null if not set
     */
    public function getFrom(): ?string;
    
    /**
     * Set the recipient identifier
     *
     * @param string $to Recipient identifier
     * @return self Fluent interface
     */
    public function setTo(string $to): self;
    
    /**
     * Get the recipient identifier
     *
     * @return string|null Recipient identifier or null if not set
     */
    public function getTo(): ?string;
    
    /**
     * Set the operation type
     *
     * @param OperationType $operation Operation type
     * @return self Fluent interface
     */
    public function setOperation(OperationType $operation): self;
    
    /**
     * Get the operation type
     *
     * @return OperationType Operation type
     */
    public function getOperation(): OperationType;
    
    /**
     * Set the message identifier
     *
     * @param string $messageId Unique message identifier
     * @return self Fluent interface
     */
    public function setMessageId(string $messageId): self;
    
    /**
     * Get the message identifier
     *
     * @return string|null Message identifier or null if not set
     */
    public function getMessageId(): ?string;
    
    /**
     * Set message capabilities (metadata)
     *
     * @param array<string, string> $capabilities Key-value metadata
     * @return self Fluent interface
     */
    public function setCapabilities(array $capabilities): self;
    
    /**
     * Get message capabilities
     *
     * @return array<string, string> Capabilities array
     */
    public function getCapabilities(): array;
    
    /**
     * Get a specific capability value
     *
     * @param string $key Capability key
     * @param string|null $default Default value if key not found
     * @return string|null Capability value or default
     */
    public function getCapability(string $key, ?string $default = null): ?string;
    
    /**
     * Set a single capability
     *
     * @param string $key Capability key
     * @param string $value Capability value
     * @return self Fluent interface
     */
    public function setCapability(string $key, string $value): self;
    
    /**
     * Set payload hint
     *
     * @param PayloadHint $hint Payload hint
     * @return self Fluent interface
     */
    public function setPayloadHint(PayloadHint $hint): self;
    
    /**
     * Get payload hint
     *
     * @return PayloadHint|null Payload hint or null
     */
    public function getPayloadHint(): ?PayloadHint;
    
    /**
     * Serialize envelope to JSON string
     *
     * @return string JSON representation
     * @throws SerializationException If serialization fails
     */
    public function serialize(): string;
    
    /**
     * Deserialize envelope from JSON string
     *
     * @param string $json JSON string
     * @return self New envelope instance
     * @throws SerializationException If deserialization fails
     */
    public static function deserialize(string $json): self;
    
    /**
     * Validate envelope structure
     *
     * @return bool True if valid, false otherwise
     */
    public function validate(): bool;
    
    /**
     * Generate envelope hash
     *
     * @return string SHA-256 hash of envelope
     */
    public function getHash(): string;
    
    /**
     * Convert to array representation
     *
     * @return array<string, mixed> Array representation
     */
    public function toArray(): array;
    
    /**
     * Create envelope from array
     *
     * @param array<string, mixed> $data Array data
     * @return self New envelope instance
     */
    public static function fromArray(array $data): self;
}
```

#### Usage Examples

```php
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

// Create envelope
$envelope = new Envelope(
    from: 'sender-001',
    to: 'receiver-001',
    operation: OperationType::DATA,
    messageId: 'msg-' . uniqid(),
    capabilities: [
        'content-type' => 'application/json',
        'priority' => 'high',
        'timestamp' => (string) time()
    ]
);

// Fluent API
$envelope->setFrom('sender-002')
         ->setTo('receiver-002')
         ->setCapability('retry-count', '0');

// Serialization
$json = $envelope->serialize();
echo $json;

// Deserialization
$received = Envelope::deserialize($json);
echo $received->getFrom(); // 'sender-002'

// Validation
if ($envelope->validate()) {
    echo "Valid envelope";
}
```

---

### Matrix

High-performance matrix and vector operations.

#### Namespace
```php
namespace UMICP\Core;
```

#### Class Definition

```php
class Matrix
{
    /**
     * Create a new matrix instance
     */
    public function __construct();
    
    /**
     * Calculate dot product of two vectors
     *
     * @param array<float> $a First vector
     * @param array<float> $b Second vector
     * @return float Dot product result
     * @throws \InvalidArgumentException If vectors have different lengths
     */
    public function dotProduct(array $a, array $b): float;
    
    /**
     * Calculate cosine similarity between two vectors
     *
     * @param array<float> $a First vector
     * @param array<float> $b Second vector
     * @return float Cosine similarity (-1 to 1)
     * @throws \InvalidArgumentException If vectors have different lengths
     */
    public function cosineSimilarity(array $a, array $b): float;
    
    /**
     * Add two vectors element-wise
     *
     * @param array<float> $a First vector
     * @param array<float> $b Second vector
     * @return array<float> Result vector
     * @throws \InvalidArgumentException If vectors have different lengths
     */
    public function vectorAdd(array $a, array $b): array;
    
    /**
     * Subtract two vectors element-wise
     *
     * @param array<float> $a First vector
     * @param array<float> $b Second vector
     * @return array<float> Result vector
     * @throws \InvalidArgumentException If vectors have different lengths
     */
    public function vectorSubtract(array $a, array $b): array;
    
    /**
     * Multiply vector by scalar
     *
     * @param array<float> $vector Vector
     * @param float $scalar Scalar value
     * @return array<float> Result vector
     */
    public function vectorScale(array $vector, float $scalar): array;
    
    /**
     * Calculate vector magnitude (L2 norm)
     *
     * @param array<float> $vector Vector
     * @return float Magnitude
     */
    public function vectorMagnitude(array $vector): float;
    
    /**
     * Normalize vector to unit length
     *
     * @param array<float> $vector Vector
     * @return array<float> Normalized vector
     */
    public function vectorNormalize(array $vector): array;
    
    /**
     * Multiply two matrices
     *
     * @param array<float> $a First matrix (flat array)
     * @param array<float> $b Second matrix (flat array)
     * @param int $m Rows of A
     * @param int $n Columns of A / Rows of B
     * @param int $p Columns of B
     * @return array<float> Result matrix (flat array)
     * @throws \InvalidArgumentException If dimensions incompatible
     */
    public function matrixMultiply(array $a, array $b, int $m, int $n, int $p): array;
    
    /**
     * Transpose matrix
     *
     * @param array<float> $matrix Matrix (flat array)
     * @param int $rows Number of rows
     * @param int $cols Number of columns
     * @return array<float> Transposed matrix
     */
    public function matrixTranspose(array $matrix, int $rows, int $cols): array;
}
```

#### Usage Examples

```php
use UMICP\Core\Matrix;

$matrix = new Matrix();

// Vector operations
$v1 = [1.0, 2.0, 3.0, 4.0];
$v2 = [5.0, 6.0, 7.0, 8.0];

$dotProduct = $matrix->dotProduct($v1, $v2);
echo "Dot product: $dotProduct\n"; // 70.0

$similarity = $matrix->cosineSimilarity($v1, $v2);
echo "Cosine similarity: $similarity\n"; // ~0.968

$sum = $matrix->vectorAdd($v1, $v2);
print_r($sum); // [6.0, 8.0, 10.0, 12.0]

// Normalize vector
$normalized = $matrix->vectorNormalize($v1);
$magnitude = $matrix->vectorMagnitude($normalized);
echo "Magnitude: $magnitude\n"; // 1.0

// Matrix multiplication
$matrixA = [1, 2, 3, 4]; // 2x2 matrix
$matrixB = [5, 6, 7, 8]; // 2x2 matrix
$result = $matrix->matrixMultiply($matrixA, $matrixB, 2, 2, 2);
print_r($result); // [19, 22, 43, 50]
```

---

### Frame

Low-level frame structure for protocol communication.

#### Namespace
```php
namespace UMICP\Core;
```

#### Class Definition

```php
class Frame
{
    /**
     * Create a new frame
     *
     * @param int $type Frame type
     * @param int $streamId Stream identifier
     * @param int $sequence Sequence number
     * @param int $flags Frame flags
     * @param bool $compressed Compression flag
     * @param bool $encrypted Encryption flag
     */
    public function __construct(
        int $type = 0,
        int $streamId = 0,
        int $sequence = 0,
        int $flags = 0,
        bool $compressed = false,
        bool $encrypted = false
    );
    
    public function getType(): int;
    public function setType(int $type): self;
    
    public function getStreamId(): int;
    public function setStreamId(int $streamId): self;
    
    public function getSequence(): int;
    public function setSequence(int $sequence): self;
    
    public function getFlags(): int;
    public function setFlags(int $flags): self;
    
    public function isCompressed(): bool;
    public function setCompressed(bool $compressed): self;
    
    public function isEncrypted(): bool;
    public function setEncrypted(bool $encrypted): self;
    
    public function serialize(): string;
    public static function deserialize(string $data): self;
}
```

---

## Enums

### OperationType

Message operation types.

```php
namespace UMICP\Core;

enum OperationType: int
{
    /**
     * Control message
     */
    case CONTROL = 0;
    
    /**
     * Data message
     */
    case DATA = 1;
    
    /**
     * Acknowledgment message
     */
    case ACK = 2;
    
    /**
     * Error message
     */
    case ERROR = 3;
    
    /**
     * Request message
     */
    case REQUEST = 4;
    
    /**
     * Response message
     */
    case RESPONSE = 5;
}
```

### PayloadType

Payload data types.

```php
namespace UMICP\Core;

enum PayloadType: int
{
    /**
     * Vector/embedding data
     */
    case VECTOR = 0;
    
    /**
     * Text data
     */
    case TEXT = 1;
    
    /**
     * Metadata
     */
    case METADATA = 2;
    
    /**
     * Binary data
     */
    case BINARY = 3;
}
```

### EncodingType

Data encoding types.

```php
namespace UMICP\Core;

enum EncodingType: int
{
    case FLOAT32 = 0;
    case FLOAT64 = 1;
    case INT32 = 2;
    case INT64 = 3;
    case UINT8 = 4;
    case UINT16 = 5;
    case UINT32 = 6;
    case UINT64 = 7;
}
```

---

## Transport Layer

### WebSocketClient

WebSocket client for connecting to remote peers.

#### Namespace
```php
namespace UMICP\Transport;
```

#### Class Definition

```php
use React\EventLoop\LoopInterface;
use UMICP\Core\Envelope;
use Evenement\EventEmitter;

class WebSocketClient extends EventEmitter
{
    /**
     * Create a new WebSocket client
     *
     * @param LoopInterface $loop Event loop
     * @param array<string, mixed> $config Client configuration
     */
    public function __construct(LoopInterface $loop, array $config = []);
    
    /**
     * Connect to WebSocket server
     *
     * @return \React\Promise\PromiseInterface<WebSocket> Promise resolving to connection
     */
    public function connect(): \React\Promise\PromiseInterface;
    
    /**
     * Disconnect from server
     *
     * @return void
     */
    public function disconnect(): void;
    
    /**
     * Send envelope to server
     *
     * @param Envelope $envelope Envelope to send
     * @return bool True if sent successfully
     */
    public function send(Envelope $envelope): bool;
    
    /**
     * Check if connected
     *
     * @return bool Connection status
     */
    public function isConnected(): bool;
    
    /**
     * Get connection statistics
     *
     * @return array<string, mixed> Statistics
     */
    public function getStats(): array;
    
    /**
     * Send and wait for response
     *
     * @param Envelope $envelope Request envelope
     * @param int $timeout Timeout in milliseconds
     * @return \React\Promise\PromiseInterface<Envelope> Promise resolving to response
     */
    public function sendAndWait(Envelope $envelope, int $timeout = 30000): \React\Promise\PromiseInterface;
}
```

#### Configuration

```php
$config = [
    'url' => 'ws://localhost:8080',
    'compression' => true,
    'auto_reconnect' => true,
    'reconnect_delay' => 5000,
    'max_reconnect_attempts' => 5,
    'heartbeat_interval' => 30000,
    'connection_timeout' => 10000,
];
```

#### Events

- **`connected`**: Fired when connection established
- **`disconnected`**: Fired when connection closed
  - Parameters: `(int $code, string $reason)`
- **`message`**: Fired when envelope received
  - Parameters: `(Envelope $envelope)`
- **`error`**: Fired on error
  - Parameters: `(\Throwable $error)`

#### Usage Example

```php
use React\EventLoop\Loop;
use UMICP\Transport\WebSocketClient;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

$loop = Loop::get();

$client = new WebSocketClient($loop, [
    'url' => 'ws://localhost:20081/umicp',
    'compression' => true,
    'auto_reconnect' => true,
]);

$client->on('connected', function () use ($client) {
    echo "Connected!\n";
    
    $envelope = new Envelope(
        from: 'client-001',
        to: 'server-001',
        operation: OperationType::DATA,
        messageId: 'msg-' . uniqid(),
        capabilities: ['message' => 'Hello Server!']
    );
    
    $client->send($envelope);
});

$client->on('message', function (Envelope $envelope) {
    echo "Received: " . json_encode($envelope->getCapabilities()) . "\n";
});

$client->on('error', function (\Throwable $error) {
    echo "Error: " . $error->getMessage() . "\n";
});

$client->connect();

Loop::run();
```

---

### WebSocketServer

WebSocket server for accepting client connections.

#### Namespace
```php
namespace UMICP\Transport;
```

#### Class Definition

```php
use React\EventLoop\LoopInterface;
use UMICP\Core\Envelope;
use Evenement\EventEmitter;

class WebSocketServer extends EventEmitter
{
    /**
     * Create a new WebSocket server
     *
     * @param LoopInterface $loop Event loop
     * @param array<string, mixed> $config Server configuration
     */
    public function __construct(LoopInterface $loop, array $config = []);
    
    /**
     * Start server
     *
     * @return \React\Promise\PromiseInterface Promise resolving when started
     */
    public function start(): \React\Promise\PromiseInterface;
    
    /**
     * Stop server
     *
     * @return \React\Promise\PromiseInterface Promise resolving when stopped
     */
    public function stop(): \React\Promise\PromiseInterface;
    
    /**
     * Send envelope to specific client
     *
     * @param string $clientId Client identifier
     * @param Envelope $envelope Envelope to send
     * @return bool True if sent successfully
     */
    public function sendToClient(string $clientId, Envelope $envelope): bool;
    
    /**
     * Broadcast envelope to all connected clients
     *
     * @param Envelope $envelope Envelope to broadcast
     * @param string|null $excludeClientId Optional client ID to exclude
     * @return int Number of clients message was sent to
     */
    public function broadcast(Envelope $envelope, ?string $excludeClientId = null): int;
    
    /**
     * Get connected clients
     *
     * @return array<string, ClientConnection> Connected clients
     */
    public function getClients(): array;
    
    /**
     * Get server statistics
     *
     * @return array<string, mixed> Statistics
     */
    public function getStats(): array;
    
    /**
     * Disconnect client
     *
     * @param string $clientId Client identifier
     * @param int $code Close code
     * @param string $reason Close reason
     * @return bool True if disconnected
     */
    public function disconnectClient(string $clientId, int $code = 1000, string $reason = ''): bool;
}
```

#### Configuration

```php
$config = [
    'port' => 20081,
    'host' => '0.0.0.0',
    'path' => '/umicp',
    'compression' => true,
    'max_payload' => 100 * 1024 * 1024, // 100MB
    'heartbeat_interval' => 30000,
];
```

#### Events

- **`listening`**: Fired when server starts listening
  - Parameters: `(string $address, int $port)`
- **`client_connected`**: Fired when client connects
  - Parameters: `(ClientConnection $client)`
- **`client_disconnected`**: Fired when client disconnects
  - Parameters: `(ClientConnection $client, int $code, string $reason)`
- **`message`**: Fired when message received from client
  - Parameters: `(Envelope $envelope, ClientConnection $client)`
- **`error`**: Fired on error
  - Parameters: `(\Throwable $error, ClientConnection|null $client)`

#### Usage Example

```php
use React\EventLoop\Loop;
use UMICP\Transport\WebSocketServer;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

$loop = Loop::get();

$server = new WebSocketServer($loop, [
    'port' => 20081,
    'path' => '/umicp',
    'compression' => true,
]);

$server->on('listening', function (string $address, int $port) {
    echo "Server listening on $address:$port\n";
});

$server->on('client_connected', function ($client) {
    echo "Client connected: {$client->id}\n";
});

$server->on('message', function (Envelope $envelope, $client) use ($server) {
    echo "Message from {$client->id}: " . json_encode($envelope->getCapabilities()) . "\n";
    
    // Echo response
    $response = new Envelope(
        from: 'server',
        to: $envelope->getFrom(),
        operation: OperationType::ACK,
        messageId: 'ack-' . uniqid(),
        capabilities: ['status' => 'received']
    );
    
    $server->sendToClient($client->id, $response);
});

$server->start();

Loop::run();
```

---

### MultiplexedPeer

Multiplexed peer architecture - each peer is both server AND client.

#### Namespace
```php
namespace UMICP\Transport;
```

#### Class Definition

```php
use React\EventLoop\LoopInterface;
use UMICP\Core\Envelope;
use Evenement\EventEmitter;

class MultiplexedPeer extends EventEmitter
{
    /**
     * Create a new multiplexed peer
     *
     * @param string $peerId Unique peer identifier
     * @param LoopInterface $loop Event loop
     * @param array<string, mixed>|null $serverConfig Server configuration (null = no server)
     * @param array<string, mixed> $metadata Peer metadata
     */
    public function __construct(
        string $peerId,
        LoopInterface $loop,
        ?array $serverConfig = null,
        array $metadata = []
    );
    
    /**
     * Connect to a remote peer
     *
     * @param string $url WebSocket URL
     * @param array<string, mixed> $metadata Connection metadata
     * @return \React\Promise\PromiseInterface<string|null> Promise resolving to peer ID or null
     */
    public function connectToPeer(string $url, array $metadata = []): \React\Promise\PromiseInterface;
    
    /**
     * Disconnect from a peer
     *
     * @param string $peerId Peer identifier
     * @return bool True if disconnected
     */
    public function disconnectPeer(string $peerId): bool;
    
    /**
     * Send envelope to specific peer
     *
     * @param string $peerId Target peer ID
     * @param Envelope $envelope Envelope to send
     * @return bool True if sent successfully
     */
    public function sendToPeer(string $peerId, Envelope $envelope): bool;
    
    /**
     * Broadcast envelope to all connected peers
     *
     * @param Envelope $envelope Envelope to broadcast
     * @param string|null $excludePeerId Peer ID to exclude
     * @return int Number of peers message was sent to
     */
    public function broadcast(Envelope $envelope, ?string $excludePeerId = null): int;
    
    /**
     * Broadcast to peers of specific type
     *
     * @param Envelope $envelope Envelope to broadcast
     * @param string $type 'incoming' or 'outgoing'
     * @param string|null $excludePeerId Peer ID to exclude
     * @return int Number of peers message was sent to
     */
    public function broadcastToType(Envelope $envelope, string $type, ?string $excludePeerId = null): int;
    
    /**
     * Send and wait for response from peer
     *
     * @param string $peerId Target peer ID
     * @param Envelope $envelope Request envelope
     * @param int $timeout Timeout in milliseconds
     * @return \React\Promise\PromiseInterface<Envelope> Promise resolving to response
     */
    public function sendAndWait(string $peerId, Envelope $envelope, int $timeout = 30000): \React\Promise\PromiseInterface;
    
    /**
     * Get all connected peers
     *
     * @return array<PeerConnection> Peer connections
     */
    public function getPeers(): array;
    
    /**
     * Get peers by type
     *
     * @param string $type 'incoming' or 'outgoing'
     * @return array<PeerConnection> Filtered peer connections
     */
    public function getPeersByType(string $type): array;
    
    /**
     * Get specific peer by ID
     *
     * @param string $peerId Peer identifier
     * @return PeerConnection|null Peer connection or null
     */
    public function getPeer(string $peerId): ?PeerConnection;
    
    /**
     * Find peer by metadata
     *
     * @param string $key Metadata key
     * @param mixed $value Metadata value
     * @return PeerConnection|null First matching peer or null
     */
    public function findPeerByMetadata(string $key, mixed $value): ?PeerConnection;
    
    /**
     * Get peer statistics
     *
     * @return array<string, mixed> Statistics
     */
    public function getStats(): array;
    
    /**
     * Shutdown peer and close all connections
     *
     * @return \React\Promise\PromiseInterface Promise resolving when shutdown complete
     */
    public function shutdown(): \React\Promise\PromiseInterface;
}
```

#### Configuration

```php
// Server configuration (optional)
$serverConfig = [
    'port' => 20081,
    'path' => '/umicp',
    'compression' => true,
];

// Peer metadata
$metadata = [
    'type' => 'agent',
    'version' => '1.0.0',
    'capabilities' => 'nlp,vision',
];
```

#### Events

- **`server:ready`**: Fired when server component is ready
- **`peer:connect`**: Fired when peer connects (incoming or outgoing)
  - Parameters: `(PeerConnection $peer)`
- **`peer:disconnect`**: Fired when peer disconnects
  - Parameters: `(PeerConnection $peer)`
- **`peer:ready`**: Fired when handshake completes and peer is ready
  - Parameters: `(PeerConnection $peer, PeerInfo $peerInfo)`
- **`message`**: Fired when any message received
  - Parameters: `(Envelope $envelope, PeerConnection $peer)`
- **`data`**: Fired when DATA operation received
  - Parameters: `(Envelope $envelope, PeerConnection $peer)`
- **`control`**: Fired when CONTROL operation received
  - Parameters: `(Envelope $envelope, PeerConnection $peer)`
- **`error`**: Fired on error
  - Parameters: `(\Throwable $error, PeerConnection|null $peer)`

#### Usage Example

```php
use React\EventLoop\Loop;
use UMICP\Transport\MultiplexedPeer;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

$loop = Loop::get();

// Create peer A (server + client)
$peerA = new MultiplexedPeer(
    peerId: 'agent-alpha',
    loop: $loop,
    serverConfig: [
        'port' => 20081,
        'path' => '/umicp'
    ],
    metadata: [
        'type' => 'coordinator',
        'version' => '1.0.0'
    ]
);

$peerA->on('peer:ready', function ($peer, $peerInfo) {
    echo "Peer ready: {$peerInfo->peerId}\n";
    echo "Metadata: " . json_encode($peerInfo->metadata) . "\n";
});

$peerA->on('data', function (Envelope $envelope, $peer) use ($peerA) {
    echo "Data from {$peer->id}: " . json_encode($envelope->getCapabilities()) . "\n";
    
    // Respond
    $response = new Envelope(
        from: 'agent-alpha',
        to: $envelope->getFrom(),
        operation: OperationType::ACK,
        messageId: 'ack-' . uniqid()
    );
    
    $peerA->sendToPeer($peer->id, $response);
});

// Connect to other peers
$peerA->connectToPeer('ws://localhost:20082/umicp')->then(
    function ($peerId) {
        echo "Connected to peer: $peerId\n";
    }
);

$peerA->connectToPeer('ws://localhost:20083/umicp')->then(
    function ($peerId) {
        echo "Connected to peer: $peerId\n";
    }
);

Loop::run();
```

---

## FFI Layer

### FFIBridge

Main FFI interface to C++ core.

```php
namespace UMICP\FFI;

use FFI;
use FFI\CData;

class FFIBridge
{
    public static function getInstance(?string $libPath = null, ?string $headerPath = null): self;
    public function getFFI(): FFI;
    
    // Envelope operations
    public function createEnvelope(): CData;
    public function destroyEnvelope(CData $envelope): void;
    
    // Matrix operations
    public function createMatrix(): CData;
    public function destroyMatrix(CData $matrix): void;
}
```

### TypeConverter

Type conversion utilities.

```php
namespace UMICP\FFI;

use FFI\CData;

class TypeConverter
{
    public static function phpArrayToCFloatArray(array $phpArray): CData;
    public static function cFloatArrayToPhpArray(CData $cArray, int $size): array;
    public static function phpStringToCString(string $phpString): CData;
    public static function cStringToPhpString(CData $cString): string;
    public static function phpArrayToJsonCString(array $array): CData;
}
```

---

## Exceptions

```php
namespace UMICP\Exception;

// Base exception
class UMICPException extends \Exception {}

// Specific exceptions
class FFIException extends UMICPException {}
class TransportException extends UMICPException {}
class SerializationException extends UMICPException {}
class ValidationException extends UMICPException {}
class ConnectionException extends TransportException {}
class TimeoutException extends TransportException {}
```

---

## Configuration

### Config File Example

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
        'reconnect_delay' => 5000,
    ],
    'server' => [
        'default_port' => 20081,
        'default_path' => '/umicp',
        'compression' => true,
        'max_payload' => 100 * 1024 * 1024,
    ],
];
```

### Loading Configuration

```php
namespace UMICP\FFI;

class Config
{
    private static ?array $config = null;
    
    public static function load(?string $configPath = null): array
    {
        if (self::$config === null) {
            $configPath ??= __DIR__ . '/../../config/umicp.php';
            self::$config = require $configPath;
        }
        
        return self::$config;
    }
    
    public static function get(string $key, mixed $default = null): mixed
    {
        $config = self::load();
        
        return $config[$key] ?? $default;
    }
}
```

---

## Complete Examples

### Federated Learning

```php
use React\EventLoop\Loop;
use UMICP\Transport\MultiplexedPeer;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;
use UMICP\Core\Matrix;

$loop = Loop::get();

// Coordinator peer
$coordinator = new MultiplexedPeer(
    peerId: 'fl-coordinator',
    loop: $loop,
    serverConfig: ['port' => 20081, 'path' => '/umicp']
);

$coordinator->on('peer:ready', function ($peer, $peerInfo) use ($coordinator) {
    echo "Worker ready: {$peerInfo->peerId}\n";
    
    // Send model weights
    $weights = [0.5, 0.8, 0.3, 0.9];
    
    $envelope = new Envelope(
        from: 'fl-coordinator',
        to: $peerInfo->peerId,
        operation: OperationType::DATA,
        messageId: 'weights-' . uniqid(),
        capabilities: [
            'action' => 'update_weights',
            'weights' => json_encode($weights),
            'epoch' => '1'
        ]
    );
    
    $coordinator->sendToPeer($peer->id, $envelope);
});

$coordinator->on('data', function (Envelope $envelope, $peer) {
    $caps = $envelope->getCapabilities();
    
    if ($caps['action'] === 'gradient_update') {
        $gradients = json_decode($caps['gradients'], true);
        echo "Received gradients from {$envelope->getFrom()}\n";
        
        // Aggregate gradients (simplified)
        $matrix = new Matrix();
        // ... aggregation logic
    }
});

Loop::run();
```

### Multi-Agent System

```php
use React\EventLoop\Loop;
use UMICP\Transport\MultiplexedPeer;
use UMICP\Core\Envelope;
use UMICP\Core\OperationType;

$loop = Loop::get();

// Create 3 agents
$agents = [];

foreach (['alpha', 'beta', 'gamma'] as $i => $name) {
    $port = 20081 + $i;
    
    $agent = new MultiplexedPeer(
        peerId: "agent-$name",
        loop: $loop,
        serverConfig: ['port' => $port, 'path' => '/umicp'],
        metadata: ['role' => $name]
    );
    
    $agent->on('data', function (Envelope $envelope, $peer) use ($agent, $name) {
        echo "[$name] Message from {$envelope->getFrom()}: "
           . json_encode($envelope->getCapabilities()) . "\n";
        
        // Broadcast to other agents
        $response = new Envelope(
            from: "agent-$name",
            to: 'all',
            operation: OperationType::DATA,
            messageId: 'resp-' . uniqid(),
            capabilities: ['action' => 'acknowledge', 'original_from' => $envelope->getFrom()]
        );
        
        $agent->broadcast($response, $peer->id);
    });
    
    $agents[$name] = $agent;
}

// Connect agents in mesh topology
$agents['alpha']->connectToPeer('ws://localhost:20082/umicp');
$agents['alpha']->connectToPeer('ws://localhost:20083/umicp');
$agents['beta']->connectToPeer('ws://localhost:20083/umicp');

Loop::run();
```

---

**Next Document**: [FFI_INTEGRATION_GUIDE.md](./FFI_INTEGRATION_GUIDE.md)

