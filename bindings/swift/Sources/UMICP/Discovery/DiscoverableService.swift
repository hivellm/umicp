import Foundation

/// Protocol for services that support tool discovery (MCP-compatible)
public protocol DiscoverableService {
    /// List all available operations
    func listOperations() -> [OperationSchema]
    
    /// Get schema for a specific operation
    func getSchema(name: String) -> OperationSchema?
    
    /// Get server information
    func serverInfo() -> ServerInfo
}

/// Default implementation for getSchema
public extension DiscoverableService {
    func getSchema(name: String) -> OperationSchema? {
        return listOperations().first { $0.name == name }
    }
}

/// Helper functions for generating discovery responses
public enum DiscoveryHelpers {
    /// Generate JSON response for list operations
    public static func generateOperationsResponse(service: DiscoverableService) throws -> String {
        let operations = service.listOperations().map { $0.toDictionary() }
        let data = try JSONSerialization.data(withJSONObject: operations, options: [.sortedKeys, .prettyPrinted])
        guard let json = String(data: data, encoding: .utf8) else {
            throw UMICPError.serializationError("Failed to generate operations response")
        }
        return json
    }
    
    /// Generate JSON response for get schema
    public static func generateSchemaResponse(service: DiscoverableService, operationName: String) throws -> String {
        if let schema = service.getSchema(name: operationName) {
            let dict = schema.toDictionary()
            let data = try JSONSerialization.data(withJSONObject: dict, options: [.sortedKeys, .prettyPrinted])
            guard let json = String(data: data, encoding: .utf8) else {
                throw UMICPError.serializationError("Failed to generate schema response")
            }
            return json
        } else {
            let error: [String: Any] = [
                "error": "Operation not found",
                "operation": operationName
            ]
            let data = try JSONSerialization.data(withJSONObject: error, options: [.sortedKeys, .prettyPrinted])
            guard let json = String(data: data, encoding: .utf8) else {
                throw UMICPError.serializationError("Failed to generate error response")
            }
            return json
        }
    }
    
    /// Generate JSON response for server info
    public static func generateServerInfoResponse(service: DiscoverableService) throws -> String {
        let info = service.serverInfo()
        let dict = info.toDictionary()
        let data = try JSONSerialization.data(withJSONObject: dict, options: [.sortedKeys, .prettyPrinted])
        guard let json = String(data: data, encoding: .utf8) else {
            throw UMICPError.serializationError("Failed to generate server info response")
        }
        return json
    }
}

