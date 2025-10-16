import Foundation

/// Operation schema compatible with MCP JSON Schema
public struct OperationSchema: Codable, Equatable {
    public let name: String
    public var title: String?
    public var description: String?
    public let inputSchema: [String: AnyCodable]
    public var outputSchema: [String: AnyCodable]?
    public var annotations: [String: AnyCodable]?
    
    enum CodingKeys: String, CodingKey {
        case name, title, description
        case inputSchema = "input_schema"
        case outputSchema = "output_schema"
        case annotations
    }
    
    public init(
        name: String,
        inputSchema: [String: AnyCodable],
        title: String? = nil,
        description: String? = nil,
        outputSchema: [String: AnyCodable]? = nil,
        annotations: [String: AnyCodable]? = nil
    ) {
        self.name = name
        self.inputSchema = inputSchema
        self.title = title
        self.description = description
        self.outputSchema = outputSchema
        self.annotations = annotations
    }
    
    /// Builder pattern for title
    public func withTitle(_ title: String) -> OperationSchema {
        var copy = self
        copy.title = title
        return copy
    }
    
    /// Builder pattern for description
    public func withDescription(_ description: String) -> OperationSchema {
        var copy = self
        copy.description = description
        return copy
    }
    
    /// Builder pattern for output schema
    public func withOutputSchema(_ schema: [String: AnyCodable]) -> OperationSchema {
        var copy = self
        copy.outputSchema = schema
        return copy
    }
    
    /// Builder pattern for annotations
    public func withAnnotations(_ annotations: [String: AnyCodable]) -> OperationSchema {
        var copy = self
        copy.annotations = annotations
        return copy
    }
    
    /// Convert to dictionary representation
    public func toDictionary() -> [String: Any] {
        var dict: [String: Any] = [
            "name": name,
            "input_schema": inputSchema.mapValues { $0.value }
        ]
        
        if let title = title {
            dict["title"] = title
        }
        if let description = description {
            dict["description"] = description
        }
        if let outputSchema = outputSchema {
            dict["output_schema"] = outputSchema.mapValues { $0.value }
        }
        if let annotations = annotations {
            dict["annotations"] = annotations.mapValues { $0.value }
        }
        
        return dict
    }
}

