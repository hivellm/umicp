import Foundation
import UMICP

// Example service implementing tool discovery
class ExampleSearchService: DiscoverableService {
    func listOperations() -> [OperationSchema] {
        return [
            OperationSchema(
                name: "search",
                inputSchema: [
                    "type": AnyCodable("object"),
                    "properties": AnyCodable([
                        "query": ["type": "string", "description": "Search query"],
                        "max_results": ["type": "integer", "default": 10],
                        "include_metadata": ["type": "boolean", "default": false]
                    ]),
                    "required": AnyCodable(["query"])
                ],
                title: "Search Operation",
                description: "Performs a semantic search across documents"
            ),
            OperationSchema(
                name: "get_document",
                inputSchema: [
                    "type": AnyCodable("object"),
                    "properties": AnyCodable([
                        "document_id": ["type": "string", "description": "Document ID"]
                    ]),
                    "required": AnyCodable(["document_id"])
                ],
                title: "Get Document",
                description: "Retrieves a specific document by ID"
            )
        ]
    }

    func serverInfo() -> ServerInfo {
        return ServerInfo(
            server: "example-search-service",
            version: "1.0.0",
            protocol: "umicp",
            features: ["tool_discovery", "native_types", "mcp_compatible"],
            operationsCount: 2,
            mcpCompatible: true
        )
        .withMetadata([
            "platform": AnyCodable("swift"),
            "sdk_version": AnyCodable(UMICP.version)
        ])
    }
}

// Example usage
func toolDiscoveryExample() throws {
    print("=== Tool Discovery Example ===\n")

    let service = ExampleSearchService()

    // List all operations
    print("Available Operations:")
    let operations = service.listOperations()
    for op in operations {
        print("  - \(op.name): \(op.title ?? "No title")")
        if let desc = op.description {
            print("    Description: \(desc)")
        }
    }

    // Get specific schema
    print("\nDetailed Schema for 'search':")
    if let schema = service.getSchema(name: "search") {
        print("  Name: \(schema.name)")
        print("  Title: \(schema.title ?? "none")")
        print("  Description: \(schema.description ?? "none")")
        print("  Input Schema: \(schema.inputSchema)")
    }

    // Get server info
    print("\nServer Information:")
    let info = service.serverInfo()
    print("  Server: \(info.server)")
    print("  Version: \(info.version)")
    print("  Protocol: \(info.protocol)")
    print("  Features: \(info.features?.joined(separator: ", ") ?? "none")")
    print("  Operations Count: \(info.operationsCount ?? 0)")
    print("  MCP Compatible: \(info.mcpCompatible ?? false)")

    // Generate JSON responses
    print("\nJSON Responses:")

    let opsJson = try DiscoveryHelpers.generateOperationsResponse(service: service)
    print("\nList Operations Response:")
    print(opsJson)

    let schemaJson = try DiscoveryHelpers.generateSchemaResponse(
        service: service,
        operationName: "search"
    )
    print("\nGet Schema Response:")
    print(schemaJson)

    let infoJson = try DiscoveryHelpers.generateServerInfoResponse(service: service)
    print("\nServer Info Response:")
    print(infoJson)
}

// Run example
do {
    try toolDiscoveryExample()
    print("\n✅ Tool discovery example completed!")
} catch {
    print("❌ Error: \(error)")
}

