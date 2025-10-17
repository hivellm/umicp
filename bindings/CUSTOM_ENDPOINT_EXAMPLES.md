# UMICP SDK Custom Endpoint Configuration

All UMICP SDKs now support custom endpoint paths for compatibility with different servers (e.g., Vectorizer uses `/umicp` instead of the default `/message`).

## Examples by Language

### Python

```python
from umicp.transport.http_client import HttpClient
from umicp.envelope import Envelope

# Default endpoint: /message
client = HttpClient("http://localhost:8000")

# Custom endpoint for Vectorizer
client = HttpClient("http://localhost:8000", path="/umicp")

# Send envelope
envelope = Envelope(...)
await client.send(envelope)
```

### Rust

```rust
use umicp_core::transport::http_client::{HttpClient, HttpClientConfig};
use std::time::Duration;

// Default endpoint: /message
let client = HttpClient::new("http://localhost:8000")?;

// Custom endpoint for Vectorizer
let client = HttpClient::new_with_path("http://localhost:8000", "/umicp")?;

// Or with full config
let config = HttpClientConfig {
    base_url: "http://localhost:8000".to_string(),
    path: "/umicp".to_string(),
    timeout: Duration::from_secs(30),
    max_retries: 3,
};
let client = HttpClient::with_config(config)?;
```

### Go

```go
import (
    "github.com/hivellm/umicp-go/pkg/transport/http"
    "time"
)

// Custom endpoint for Vectorizer
config := http.ClientConfig{
    BaseURL:         "http://localhost:8000",
    Path:            "/umicp",  // Custom path
    Timeout:         30 * time.Second,
    MaxIdleConns:    10,
    IdleConnTimeout: 90 * time.Second,
}

client := http.NewClient(config)
```

### TypeScript

```typescript
import { StreamableHTTPClient } from '@hivellm/umicp';

// Custom endpoint for Vectorizer
const client = new StreamableHTTPClient({
  baseUrl: 'http://localhost:8000',
  path: '/umicp',  // Custom path (default: '/umicp')
  timeout: 30000,
});

await client.send(envelope);
```

### PHP

```php
use UMICP\Transport\HttpClient;

// Custom endpoint for Vectorizer
$client = new HttpClient([
    'baseUrl' => 'http://localhost:8000',
    'path' => '/umicp',  // Custom path (default: '/umicp')
    'timeout' => 30,
]);

$response = $client->send($envelope);
```

### C#

```csharp
using Umicp.Core.Transport;

// C# uses path as method parameter
var client = new HttpClient("http://localhost:8000");

// Send to custom endpoint
var response = await client.PostAsync<Envelope>("/umicp", envelope);
```

### Kotlin

```kotlin
import com.hivellm.umicp.transport.UMICPHttpClient

// Kotlin uses endpoint as method parameter
val client = UMICPHttpClient(
    baseUrl = "http://localhost:8000",
    timeout = 30_000
)

// Send to custom endpoint
val response = client.post("/umicp", envelope)
```

### Swift

```swift
import UMICP

// Swift uses endpoint as method parameter
let client = UMICPHTTP2(baseURL: URL(string: "http://localhost:8000")!)

// Send to custom endpoint (default: "/umicp")
let response = try await client.send(envelope: envelope, endpoint: "/umicp")
```

### Java

```java
import com.hivellm.umicp.transport.http.UMICPHttpClient;
import com.hivellm.umicp.transport.http.HttpClientOptions;

// Java uses endpoint as method parameter
HttpClientOptions options = new HttpClientOptions();
options.setBaseUrl("http://localhost:8000");

UMICPHttpClient client = new UMICPHttpClient(options);

// Send to custom endpoint
String response = client.sendEnvelope("/umicp", envelope);
```

## Common Use Cases

### Vectorizer Integration

The Vectorizer service uses `/umicp` as its UMICP endpoint:

```python
# Python
client = HttpClient("http://localhost:8000", path="/umicp")
```

```rust
// Rust
let client = HttpClient::new_with_path("http://localhost:8000", "/umicp")?;
```

```go
// Go
config := http.ClientConfig{
    BaseURL: "http://localhost:8000",
    Path:    "/umicp",
}
client := http.NewClient(config)
```

### Standard UMICP Server

Standard UMICP servers typically use `/message`:

```python
# Python (default)
client = HttpClient("http://localhost:8000")  # Uses /message
```

```rust
// Rust (default)
let client = HttpClient::new("http://localhost:8000")?;  // Uses /message
```

```go
// Go (uses /umicp by default)
config := http.ClientConfig{
    BaseURL: "http://localhost:8000",
}
client := http.NewClient(config)  // Uses /umicp
```

## SDK Default Endpoints

| SDK        | Default Endpoint | Configurable |
|------------|-----------------|--------------|
| Python     | `/message`      | ✅ Constructor param |
| Rust       | `/message`      | ✅ Config field + helper method |
| Go         | `/umicp`        | ✅ Config field |
| TypeScript | `/umicp`        | ✅ Options param |
| PHP        | `/umicp`        | ✅ Options array |
| C#         | Method param    | ✅ Per-method |
| Kotlin     | Method param    | ✅ Per-method |
| Swift      | `/umicp`        | ✅ Method param |
| Java       | Method param    | ✅ Per-method |

