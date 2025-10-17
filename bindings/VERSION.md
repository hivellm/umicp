# UMICP SDK Versions

All UMICP SDK bindings are now at version **0.2.2**.

## Current Versions

| SDK        | Version | Package/Module Name            | Repository/Registry                          |
|------------|---------|--------------------------------|----------------------------------------------|
| C++        | 0.2.2   | `umicp`                       | Conan/vcpkg                                  |
| Rust       | 0.2.2   | `umicp-core`                  | crates.io                                    |
| Python     | 0.2.2   | `umicp-python`                | PyPI                                         |
| TypeScript | 0.2.2   | `@hivellm/umicp`              | npm                                          |
| Go         | latest  | `github.com/hivellm/umicp-go` | Go Modules                                   |
| Java       | 0.2.2   | `com.hivellm:umicp-*`         | Maven Central                                |
| Kotlin     | 0.2.2   | `com.hivellm.org:umicp`       | Maven Central                                |
| C#         | 0.2.2   | `HiveLLM.Umicp`               | NuGet                                        |
| PHP        | latest  | `hivellm/umicp`               | Packagist                                    |
| Swift      | latest  | `UMICP`                       | Swift Package Manager                        |
| Elixir     | 0.2.2   | `umicp`                       | Hex.pm                                       |

## Version 0.2.2 Highlights

### Custom Endpoint Support
All SDKs now support configurable endpoint paths:

- **Python**: `HttpClient(base_url, path="/umicp")`
- **Rust**: `HttpClient::new_with_path(base_url, "/umicp")` or `HttpClientConfig { path: "/umicp", ... }`
- **Go**: `ClientConfig { Path: "/umicp", ... }`
- **TypeScript**: Already supported via `path` option (default: `/umicp`)
- **PHP**: Already supported via `path` option (default: `/umicp`)
- **C#/Kotlin/Java/Swift**: Endpoint passed as method parameter

### Benefits
- ✅ Full compatibility with Vectorizer service (`/umicp`)
- ✅ Compatible with standard UMICP servers (`/message`)
- ✅ Backward compatible - no breaking changes
- ✅ Consistent API across all SDKs

## Version History

### 0.2.2 (2025-10-17)
- Added custom endpoint path configuration
- Updated all SDKs to consistent version
- Added integration examples for Vectorizer
- Documentation improvements

### 0.2.1 (Previous)
- Various bug fixes and improvements
- Enhanced HTTP/2 support

### 0.2.0 (Previous)
- Tool Discovery + Native Types
- MCP-compatible tool discovery
- JSON Schema support
- Operation introspection

## Installation

### C++

**CMake (FetchContent)**:
```cmake
include(FetchContent)
FetchContent_Declare(
    umicp
    GIT_REPOSITORY https://github.com/hivellm/umicp.git
    GIT_TAG v0.2.2
)
FetchContent_MakeAvailable(umicp)

target_link_libraries(your_target PRIVATE umicp::umicp)
```

**Conan**:
```bash
conan install umicp/0.2.2@
```

### Rust
```toml
[dependencies]
umicp-core = "0.2.2"
```

### Python
```bash
pip install umicp-python==0.2.2
```

### TypeScript/JavaScript
```bash
npm install @hivellm/umicp@0.2.2
```

### Go
```bash
go get github.com/hivellm/umicp-go@latest
```

### Java (Maven)
```xml
<dependency>
    <groupId>com.hivellm</groupId>
    <artifactId>umicp-core</artifactId>
    <version>0.2.2</version>
</dependency>
```

### Kotlin (Gradle)
```kotlin
implementation("com.hivellm.org:umicp:0.2.2")
```

### C# (NuGet)
```bash
dotnet add package HiveLLM.Umicp --version 0.2.2
```

### PHP (Composer)
```bash
composer require hivellm/umicp
```

### Swift
```swift
dependencies: [
    .package(url: "https://github.com/hivellm/umicp.git", from: "0.2.2")
]
```

### Elixir
```elixir
def deps do
  [
    {:umicp, "~> 0.2.2"}
  ]
end
```

## Compatibility Matrix

All 0.2.2 SDKs are fully compatible with each other and support:

- ✅ UMICP Protocol v1.0
- ✅ HTTP/1.1 and HTTP/2
- ✅ WebSocket transport
- ✅ Custom endpoint paths
- ✅ Tool discovery
- ✅ Matrix operations
- ✅ Compression support
- ✅ P2P multiplexing

## Support

- **Documentation**: https://github.com/hivellm/umicp
- **Issues**: https://github.com/hivellm/umicp/issues
- **Discord**: https://discord.gg/hivellm

