import Foundation
import CryptoKit

/// UMICP Envelope for message passing with native JSON type support
public struct Envelope: Codable {
    public let from: String
    public let to: String
    public let operation: OperationType
    public let messageId: String
    public var capabilities: Capabilities
    public var payload: Data?
    public var hash: String?
    
    enum CodingKeys: String, CodingKey {
        case from, to, operation
        case messageId = "message_id"
        case capabilities, payload, hash
    }
    
    public init(
        from: String,
        to: String,
        operation: OperationType,
        messageId: String,
        capabilities: Capabilities = [:],
        payload: Data? = nil
    ) {
        self.from = from
        self.to = to
        self.operation = operation
        self.messageId = messageId
        self.capabilities = capabilities
        self.payload = payload
        self.hash = nil
    }
    
    /// Calculate SHA-256 hash of envelope
    public mutating func calculateHash() throws {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.sortedKeys]
        
        // Create a copy without hash for hashing
        var hashableEnvelope = self
        hashableEnvelope.hash = nil
        
        let data = try encoder.encode(hashableEnvelope)
        let digest = SHA256.hash(data: data)
        self.hash = digest.compactMap { String(format: "%02x", $0) }.joined()
    }
    
    /// Validate envelope fields
    public func validate() throws {
        if from.isEmpty {
            throw UMICPError.validationError("'from' field cannot be empty")
        }
        if to.isEmpty {
            throw UMICPError.validationError("'to' field cannot be empty")
        }
        if messageId.isEmpty {
            throw UMICPError.validationError("'message_id' field cannot be empty")
        }
    }
    
    /// Verify hash matches envelope content
    public func verifyHash() throws -> Bool {
        guard let currentHash = hash else {
            return false
        }
        
        var copy = self
        try copy.calculateHash()
        
        return copy.hash == currentHash
    }
    
    /// Serialize to JSON string
    public func serialize() throws -> String {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.sortedKeys]
        let data = try encoder.encode(self)
        guard let json = String(data: data, encoding: .utf8) else {
            throw UMICPError.serializationError("Failed to convert to UTF-8")
        }
        return json
    }
    
    /// Deserialize from JSON string
    public static func deserialize(from json: String) throws -> Envelope {
        guard let data = json.data(using: .utf8) else {
            throw UMICPError.serializationError("Invalid UTF-8 string")
        }
        let decoder = JSONDecoder()
        return try decoder.decode(Envelope.self, from: data)
    }
    
    /// Get capability value
    public func getCapability(_ key: String) -> Any? {
        return capabilities[key]?.value
    }
    
    /// Set capability value
    public mutating func setCapability(_ key: String, value: Any) {
        capabilities[key] = AnyCodable(value)
    }
}

/// Builder pattern for creating envelopes
public class EnvelopeBuilder {
    private var from: String = ""
    private var to: String = ""
    private var operation: OperationType = .data
    private var messageId: String = ""
    private var capabilities: Capabilities = [:]
    private var payload: Data?
    
    public init() {}
    
    @discardableResult
    public func from(_ value: String) -> Self {
        self.from = value
        return self
    }
    
    @discardableResult
    public func to(_ value: String) -> Self {
        self.to = value
        return self
    }
    
    @discardableResult
    public func operation(_ value: OperationType) -> Self {
        self.operation = value
        return self
    }
    
    @discardableResult
    public func messageId(_ value: String) -> Self {
        self.messageId = value
        return self
    }
    
    @discardableResult
    public func capabilities(_ value: Capabilities) -> Self {
        self.capabilities = value
        return self
    }
    
    @discardableResult
    public func capability(_ key: String, value: Any) -> Self {
        self.capabilities[key] = AnyCodable(value)
        return self
    }
    
    @discardableResult
    public func capabilityString(_ key: String, value: String) -> Self {
        self.capabilities[key] = AnyCodable(value)
        return self
    }
    
    @discardableResult
    public func capabilityInt(_ key: String, value: Int) -> Self {
        self.capabilities[key] = AnyCodable(value)
        return self
    }
    
    @discardableResult
    public func capabilityBool(_ key: String, value: Bool) -> Self {
        self.capabilities[key] = AnyCodable(value)
        return self
    }
    
    @discardableResult
    public func capabilityDouble(_ key: String, value: Double) -> Self {
        self.capabilities[key] = AnyCodable(value)
        return self
    }
    
    @discardableResult
    public func payload(_ value: Data?) -> Self {
        self.payload = value
        return self
    }
    
    public func build() throws -> Envelope {
        let envelope = Envelope(
            from: from,
            to: to,
            operation: operation,
            messageId: messageId,
            capabilities: capabilities,
            payload: payload
        )
        
        try envelope.validate()
        return envelope
    }
    
    public func buildWithHash() throws -> Envelope {
        var envelope = try build()
        try envelope.calculateHash()
        return envelope
    }
}

