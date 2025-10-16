import Foundation

/// Operation types for UMICP envelopes
public enum OperationType: UInt8, Codable, CaseIterable {
    case data = 0
    case control = 1
    case ack = 2
    case error = 3
    case request = 4
    case response = 5

    public var description: String {
        switch self {
        case .data: return "data"
        case .control: return "control"
        case .ack: return "ack"
        case .error: return "error"
        case .request: return "request"
        case .response: return "response"
        }
    }
}

/// Capabilities type supporting native JSON values
public typealias Capabilities = [String: AnyCodable]

/// UMICP error types
public enum UMICPError: Error {
    case invalidEnvelope(String)
    case serializationError(String)
    case validationError(String)
    case transportError(String)
    case hashMismatch

    public var localizedDescription: String {
        switch self {
        case .invalidEnvelope(let msg):
            return "Invalid envelope: \(msg)"
        case .serializationError(let msg):
            return "Serialization error: \(msg)"
        case .validationError(let msg):
            return "Validation error: \(msg)"
        case .transportError(let msg):
            return "Transport error: \(msg)"
        case .hashMismatch:
            return "Hash mismatch"
        }
    }
}

