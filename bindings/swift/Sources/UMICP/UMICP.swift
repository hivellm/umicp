import Foundation

/// Main UMICP module - Universal Matrix Inter-Communication Protocol
///
/// UMICP provides a universal communication framework for distributed systems,
/// federated learning, and real-time applications with built-in matrix operations
/// and type-safe messaging.
///
/// ## Features
/// - Native JSON type support in capabilities
/// - MCP-compatible tool discovery
/// - WebSocket and HTTP/2 transport
/// - SIMD-optimized matrix operations
/// - Cross-platform (macOS, iOS, Linux)
///
/// ## Quick Start
/// ```swift
/// let envelope = try EnvelopeBuilder()
///     .from("client-001")
///     .to("server-001")
///     .operation(.data)
///     .messageId("msg-\(UUID().uuidString)")
///     .capability("priority", value: "high")
///     .capabilityInt("max_tokens", value: 100)
///     .capabilityBool("streaming", value: true)
///     .buildWithHash()
/// ```
public struct UMICP {
    public static let version = "0.2.0"
    public static let protocolVersion = "0.2"

    /// Create a new envelope builder
    public static func builder() -> EnvelopeBuilder {
        return EnvelopeBuilder()
    }

    /// Create matrix operations instance
    public static func matrixOps() -> MatrixOperations {
        return MatrixOperations()
    }
}

