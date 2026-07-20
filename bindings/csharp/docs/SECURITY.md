# Security Policy

## Supported Versions

We actively support the following versions of UMICP C# SDK with security updates:

| Version | Supported          |
| ------- | ------------------ |
| 1.0.x   | :white_check_mark: |
| < 1.0   | :x:                |

## Reporting a Vulnerability

We take security seriously. If you discover a security vulnerability in the UMICP C# SDK, please report it responsibly.

### How to Report

**Please DO NOT report security vulnerabilities through public GitHub issues.**

Instead, please report them via one of the following methods:

1. **Email**: Send details to **team@hivellm.org**
2. **GitHub Security Advisory**: Use the [GitHub Security Advisory](https://github.com/hivellm/umicp/security/advisories/new) feature

### What to Include

Please include the following information in your report:

- **Type of vulnerability** (e.g., buffer overflow, injection, authentication bypass)
- **Full paths of source file(s)** related to the vulnerability
- **Location of the affected source code** (tag/branch/commit or direct URL)
- **Step-by-step instructions to reproduce** the issue
- **Proof-of-concept or exploit code** (if possible)
- **Impact of the vulnerability** and how it can be exploited
- **Your name/handle** for acknowledgment (if desired)

### Response Timeline

- **Initial Response**: Within 48 hours of report
- **Status Update**: Within 7 days with assessment
- **Fix Timeline**: Critical issues within 14 days, others within 30 days
- **Disclosure**: Coordinated disclosure after patch release

### Security Best Practices

When using the UMICP C# SDK in production:

1. **Always use TLS/SSL** for WebSocket and HTTP connections in production
2. **Validate all input data** before processing envelopes
3. **Set connection limits** on servers to prevent resource exhaustion
4. **Keep dependencies updated** - run `dotnet list package --vulnerable` regularly
5. **Enable authentication** on production endpoints
6. **Rate limit** client connections and message throughput
7. **Sanitize logging** - avoid logging sensitive payload data
8. **Use strong credentials** for any authentication mechanisms

### Known Security Considerations

#### 1. HttpListener Requires Admin Privileges (Windows)
The `HttpServer` implementation uses `HttpListener`, which requires administrator privileges on Windows to bind to ports < 1024. For production deployments, consider:
- Using ports ≥ 1024
- Running behind a reverse proxy (nginx, IIS)
- Using ASP.NET Core hosting instead

#### 2. No Built-in Authentication
The SDK does not include authentication mechanisms by design. You must implement:
- Token-based authentication in envelope metadata
- TLS client certificates
- Application-level authorization

#### 3. WebSocket Message Size Limits
Configure appropriate message size limits to prevent memory exhaustion:
```csharp
var server = new WebSocketServer("ws://localhost:8080");
// Implement message size checks in your handler
```

#### 4. Compression Bomb Protection
When using compression, be aware of compression ratio attacks. Monitor decompressed sizes:
```csharp
var ratio = Compressor.GetCompressionRatio(original, compressed);
if (ratio > 100) { /* Potential compression bomb */ }
```

## Security Update Policy

- **Critical vulnerabilities**: Immediate patch release
- **High severity**: Patch within 7 days
- **Medium severity**: Included in next minor release
- **Low severity**: Included in next release

## Hall of Fame

We acknowledge security researchers who responsibly disclose vulnerabilities:

<!-- List will be maintained as reports come in -->

## Additional Resources

- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [.NET Security Best Practices](https://docs.microsoft.com/en-us/dotnet/standard/security/)
- [WebSocket Security](https://tools.ietf.org/html/rfc6455#section-10)

---

**Last Updated**: October 10, 2025
**Contact**: team@hivellm.org

