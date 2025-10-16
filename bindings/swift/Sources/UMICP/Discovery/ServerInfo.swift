import Foundation

/// Server information for tool discovery
public struct ServerInfo: Codable, Equatable {
    public let server: String
    public let version: String
    public let `protocol`: String
    public var features: [String]?
    public var operationsCount: Int?
    public var mcpCompatible: Bool?
    public var metadata: [String: AnyCodable]?

    enum CodingKeys: String, CodingKey {
        case server, version
        case `protocol` = "protocol"
        case features
        case operationsCount = "operations_count"
        case mcpCompatible = "mcp_compatible"
        case metadata
    }

    public init(
        server: String,
        version: String,
        protocol: String,
        features: [String]? = nil,
        operationsCount: Int? = nil,
        mcpCompatible: Bool? = nil,
        metadata: [String: AnyCodable]? = nil
    ) {
        self.server = server
        self.version = version
        self.protocol = `protocol`
        self.features = features
        self.operationsCount = operationsCount
        self.mcpCompatible = mcpCompatible
        self.metadata = metadata
    }

    /// Builder pattern for features
    public func withFeatures(_ features: [String]) -> ServerInfo {
        var copy = self
        copy.features = features
        return copy
    }

    /// Builder pattern for operations count
    public func withOperationsCount(_ count: Int) -> ServerInfo {
        var copy = self
        copy.operationsCount = count
        return copy
    }

    /// Builder pattern for MCP compatibility
    public func withMcpCompatible(_ compatible: Bool) -> ServerInfo {
        var copy = self
        copy.mcpCompatible = compatible
        return copy
    }

    /// Builder pattern for metadata
    public func withMetadata(_ metadata: [String: AnyCodable]) -> ServerInfo {
        var copy = self
        copy.metadata = metadata
        return copy
    }

    /// Convert to dictionary representation
    public func toDictionary() -> [String: Any] {
        var dict: [String: Any] = [
            "server": server,
            "version": version,
            "protocol": `protocol`
        ]

        if let features = features {
            dict["features"] = features
        }
        if let operationsCount = operationsCount {
            dict["operations_count"] = operationsCount
        }
        if let mcpCompatible = mcpCompatible {
            dict["mcp_compatible"] = mcpCompatible
        }
        if let metadata = metadata {
            dict["metadata"] = metadata.mapValues { $0.value }
        }

        return dict
    }
}

