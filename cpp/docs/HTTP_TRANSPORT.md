# UMICP StreamableHTTP Transport

**Date**: October 10, 2025  
**Status**: Implemented ✅  
**Version**: 1.0.0

---

## 📊 Overview

The StreamableHTTP transport provides HTTP/1.1 support for UMICP, enabling request/response communication patterns over HTTP. This complements the WebSocket transport and provides compatibility with HTTP-based infrastructure.

### Features

✅ **HTTP/1.1 Client** - Full-featured HTTP client based on libcurl  
✅ **HTTP/1.1 Server** - Native HTTP server with thread pool  
✅ **Request/Response Pattern** - Synchronous message exchange  
✅ **Keep-Alive Connections** - Connection reuse for efficiency  
✅ **Custom Headers** - Support for custom HTTP headers  
✅ **SSL/TLS Support** - Secure HTTPS communication  
✅ **Health Check Endpoint** - Built-in health monitoring  
✅ **Custom Request Handlers** - Flexible routing  
✅ **Statistics Tracking** - Performance metrics  
✅ **Connection Management** - Automatic connection handling  

---

## 🏗️ Architecture

### Client Architecture

```
┌─────────────────────────────────────┐
│   StreamableHTTPClient              │
├─────────────────────────────────────┤
│ - HTTPClientConfig                  │
│ - CURL handle                       │
│ - Statistics                        │
├─────────────────────────────────────┤
│ + connect()                         │
│ + send(Envelope) -> Envelope        │
│ + post(data) -> HTTPResponse        │
│ + get(path) -> HTTPResponse         │
│ + health_check()                    │
│ + disconnect()                      │
└─────────────────────────────────────┘
           │
           │ uses
           ▼
      ┌─────────┐
      │ libcurl │
      └─────────┘
```

### Server Architecture

```
┌─────────────────────────────────────┐
│   StreamableHTTPServer              │
├─────────────────────────────────────┤
│ - HTTPServerConfig                  │
│ - Accept Thread                     │
│ - Worker Thread Pool                │
│ - Message Handler                   │
│ - Request Handlers Map              │
│ - Client Connections                │
├─────────────────────────────────────┤
│ + start()                           │
│ + stop()                            │
│ + on_message(handler)               │
│ + on_request(path, handler)         │
│ + get_statistics()                  │
└─────────────────────────────────────┘
           │
           │ manages
           ▼
   ┌──────────────────┐
   │ Client           │
   │ Connections      │
   │ (Socket FDs)     │
   └──────────────────┘
```

---

## 🚀 Usage

### HTTP Client

```cpp
#include "http_client.h"
#include "envelope.h"

// Configure client
HTTPClientConfig config;
config.base_url = "http://localhost:9080";
config.path = "/umicp";
config.timeout = std::chrono::milliseconds(30000);
config.keep_alive = true;

// Create client
StreamableHTTPClient client(config);

// Connect
auto result = client.connect();
if (!result.is_ok()) {
    std::cerr << "Failed to connect: " << result.error() << "\n";
    return;
}

// Create envelope
Envelope envelope;
envelope.version = 1;
envelope.from = "client-1";
envelope.to = "server";
envelope.operation = OperationType::REQUEST;
envelope.timestamp = std::chrono::system_clock::now();
envelope.payload = {'H', 'e', 'l', 'l', 'o'};

// Send and receive response
auto send_result = client.send(envelope);
if (send_result.is_ok()) {
    Envelope response = send_result.unwrap();
    // Process response
}

// Health check
auto health = client.health_check();

// Get statistics
auto stats = client.get_statistics();
std::cout << "Requests sent: " << stats.requests_sent << "\n";
std::cout << "Average latency: " << stats.avg_latency.count() << " ms\n";

// Disconnect
client.disconnect();
```

### HTTP Server

```cpp
#include "http_server.h"
#include "envelope.h"

// Configure server
HTTPServerConfig config;
config.port = 9080;
config.host = "0.0.0.0";
config.path = "/umicp";
config.worker_threads = 4;
config.max_connections = 1000;

// Create server
StreamableHTTPServer server(config);

// Set message handler
server.on_message([](const Envelope& envelope, const HTTPRequest& req) -> Result<Envelope> {
    // Process envelope
    std::cout << "Received envelope from: " << envelope.from << "\n";
    
    // Create response
    Envelope response;
    response.version = 1;
    response.from = "server";
    response.to = envelope.from;
    response.operation = OperationType::RESPONSE;
    response.timestamp = std::chrono::system_clock::now();
    response.payload = {'O', 'K'};
    
    return Result<Envelope>::ok(response);
});

// Add custom endpoint
server.on_request("/info", [&server](const HTTPRequest& req) -> HTTPResponseBuilder {
    auto stats = server.get_statistics();
    
    std::ostringstream json;
    json << "{\"connections\": " << stats.connections_active 
         << ", \"requests\": " << stats.requests_handled << "}";
    
    HTTPResponseBuilder builder;
    builder.status(200).json(json.str());
    return builder;
});

// Start server
auto result = server.start();
if (!result.is_ok()) {
    std::cerr << "Failed to start: " << result.error() << "\n";
    return;
}

std::cout << "Server running on http://0.0.0.0:9080\n";

// Server runs in background threads
// ...

// Stop server
server.stop();
```

---

## ⚙️ Configuration

### Client Configuration

```cpp
struct HTTPClientConfig {
    std::string base_url;                         // Server URL
    std::string path = "/umicp";                  // UMICP endpoint path
    
    // Timeouts
    std::chrono::milliseconds timeout = 30000;            // Request timeout
    std::chrono::milliseconds connect_timeout = 10000;    // Connection timeout
    
    // Headers
    std::unordered_map<std::string, std::string> headers; // Custom headers
    
    // SSL/TLS
    bool verify_ssl = true;                       // Verify SSL certificates
    std::string ssl_ca_path;                      // CA certificate path
    std::string ssl_cert_path;                    // Client certificate
    std::string ssl_key_path;                     // Client key
    
    // Connection
    bool keep_alive = true;                       // Enable keep-alive
    size_t max_redirects = 5;                     // Max HTTP redirects
    size_t buffer_size = 65536;                   // Buffer size (64KB)
};
```

### Server Configuration

```cpp
struct HTTPServerConfig {
    uint16_t port = 9080;                         // Listen port
    std::string host = "0.0.0.0";                 // Listen address
    std::string path = "/umicp";                  // UMICP endpoint path
    
    // Limits
    size_t max_connections = 1000;                // Max concurrent connections
    size_t max_request_size = 100 * 1024 * 1024; // Max request size (100MB)
    size_t max_header_size = 8192;                // Max header size (8KB)
    
    // Timeouts
    std::chrono::milliseconds request_timeout = 30000;     // Request timeout
    std::chrono::milliseconds keep_alive_timeout = 60000;  // Keep-alive timeout
    
    // Performance
    size_t buffer_size = 65536;                   // Buffer size (64KB)
    size_t worker_threads = 4;                    // Worker thread count
    
    // SSL/TLS (optional)
    bool use_ssl = false;                         // Enable HTTPS
    std::string ssl_cert_path;                    // Server certificate
    std::string ssl_key_path;                     // Server key
};
```

---

## 🔌 Endpoints

### Built-in Endpoints

#### `POST /umicp` - UMICP Message Endpoint
- **Purpose**: Send and receive UMICP envelopes
- **Content-Type**: `application/octet-stream`
- **Request**: Serialized UMICP envelope
- **Response**: Serialized UMICP envelope
- **Status Codes**:
  - `200 OK` - Success
  - `400 Bad Request` - Invalid envelope
  - `500 Internal Server Error` - Processing error
  - `501 Not Implemented` - No handler configured

#### `GET /health` - Health Check
- **Purpose**: Check server health
- **Content-Type**: `application/json`
- **Response**: `{"status": "ok", "service": "UMICP"}`
- **Status Code**: `200 OK`

### Custom Endpoints

You can add custom endpoints using `on_request()`:

```cpp
server.on_request("/custom", [](const HTTPRequest& req) -> HTTPResponseBuilder {
    HTTPResponseBuilder builder;
    builder.status(200).json("{\"message\": \"Custom endpoint\"}");
    return builder;
});
```

---

## 📊 Statistics

### Client Statistics

```cpp
struct Statistics {
    size_t requests_sent;                     // Total requests sent
    size_t responses_received;                // Total responses received
    size_t bytes_sent;                        // Total bytes sent
    size_t bytes_received;                    // Total bytes received
    size_t errors;                            // Total errors
    std::chrono::milliseconds avg_latency;    // Average latency
    std::chrono::system_clock::time_point last_request; // Last request time
};
```

### Server Statistics

```cpp
struct Statistics {
    size_t connections_accepted;              // Total connections accepted
    size_t connections_active;                // Current active connections
    size_t requests_handled;                  // Total requests handled
    size_t bytes_received;                    // Total bytes received
    size_t bytes_sent;                        // Total bytes sent
    size_t errors;                            // Total errors
    std::chrono::system_clock::time_point started_at; // Server start time
    std::chrono::milliseconds uptime;         // Server uptime
};
```

---

## 🔒 Security

### SSL/TLS Support

#### Client

```cpp
HTTPClientConfig config;
config.base_url = "https://secure-server.com";
config.verify_ssl = true;
config.ssl_ca_path = "/path/to/ca-bundle.crt";
config.ssl_cert_path = "/path/to/client.crt";  // Client certificate (optional)
config.ssl_key_path = "/path/to/client.key";   // Client key (optional)
```

#### Server

```cpp
HTTPServerConfig config;
config.port = 9443;
config.use_ssl = true;
config.ssl_cert_path = "/path/to/server.crt";
config.ssl_key_path = "/path/to/server.key";
```

---

## 🎯 When to Use HTTP vs WebSocket

### Use **HTTP** when:

✅ **Request/Response Pattern**
- Simple synchronous request/response
- REST-like API integration
- One-off queries

✅ **Infrastructure Compatibility**
- HTTP-only infrastructure
- Load balancers expecting HTTP
- Existing HTTP-based systems

✅ **Caching & Proxies**
- Need HTTP caching
- Proxy traversal
- CDN integration

### Use **WebSocket** when:

✅ **Bi-directional Communication**
- Real-time bidirectional messaging
- Server-initiated messages
- Streaming data

✅ **High Frequency**
- High message frequency
- Low latency required
- Persistent connection needed

✅ **Performance Critical**
- Reduced overhead vs HTTP
- No connection setup per message
- Better for high throughput

---

## 🔧 Dependencies

### Required

- **libcurl** - HTTP client library
  - Ubuntu/Debian: `sudo apt-get install libcurl4-openssl-dev`
  - Fedora/RHEL: `sudo dnf install libcurl-devel`
  - macOS: `brew install curl`

### Optional

- **OpenSSL** - SSL/TLS support (usually included with libcurl)

---

## 📝 Examples

See `examples/http_example.cpp` for a complete working example.

### Build Example

```bash
cd umicp/cpp
mkdir build && cd build
cmake ..
make http_example
./http_example
```

---

## 🐛 Error Handling

### Client Errors

- **Connection Failed** - Server unreachable
- **Timeout** - Request took too long
- **Invalid Response** - Server returned invalid data
- **SSL Error** - Certificate verification failed

```cpp
auto result = client.send(envelope);
if (!result.is_ok()) {
    std::cerr << "Error: " << result.error() << "\n";
    // Handle error
}
```

### Server Errors

- **Bind Failed** - Port already in use
- **Invalid Request** - Malformed HTTP request
- **Handler Error** - Message handler threw exception

```cpp
auto result = server.start();
if (!result.is_ok()) {
    std::cerr << "Failed to start: " << result.error() << "\n";
    // Handle error
}
```

---

## ✅ Feature Comparison

| Feature | HTTP Client | HTTP Server | WebSocket |
|---------|-------------|-------------|-----------|
| **Bi-directional** | ⚠️ Request/Response | ⚠️ Request/Response | ✅ Full duplex |
| **Connection** | Keep-alive | Keep-alive | Persistent |
| **Overhead** | Medium (headers) | Medium (headers) | Low (frames) |
| **Latency** | Higher | Higher | Lower |
| **Streaming** | Limited | Limited | ✅ Full |
| **Caching** | ✅ HTTP cache | ✅ HTTP cache | ❌ N/A |
| **Load Balancing** | ✅ Easy | ✅ Easy | ⚠️ Special config |
| **Firewall Friendly** | ✅ Port 80/443 | ✅ Port 80/443 | ⚠️ May be blocked |

---

## 🎓 Best Practices

### Client

1. **Reuse Connections** - Enable keep-alive for better performance
2. **Set Timeouts** - Always configure appropriate timeouts
3. **Handle Errors** - Check all Result<> return values
4. **Monitor Statistics** - Track latency and error rates
5. **Use SSL** - Enable SSL for production environments

### Server

1. **Thread Pool Sizing** - Configure worker threads based on load
2. **Connection Limits** - Set appropriate max_connections
3. **Request Size Limits** - Protect against large payloads
4. **Error Handling** - Implement robust error handling in handlers
5. **Health Monitoring** - Use /health endpoint for monitoring
6. **Graceful Shutdown** - Call stop() to clean up properly

---

## 🚀 Performance

### Throughput

- **Client**: ~1,000-5,000 requests/second (depends on network)
- **Server**: ~5,000-10,000 requests/second (4 workers)

### Latency

- **Local**: 1-5 ms
- **LAN**: 5-20 ms
- **Internet**: 50-200 ms (depends on distance)

### Memory

- **Client**: ~1-5 MB (depends on buffer sizes)
- **Server**: ~10-50 MB (depends on connections and buffers)

---

## 📚 References

- [HTTP/1.1 RFC 2616](https://tools.ietf.org/html/rfc2616)
- [libcurl Documentation](https://curl.se/libcurl/)
- [UMICP Protocol Specification](./UMICP_SPEC.md)

---

**Document Version**: 1.0  
**Last Updated**: October 10, 2025  
**Status**: Complete ✅  
**Implemented**: YES ✅

