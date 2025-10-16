import XCTest
@testable import UMICP

// Mock implementation of DiscoverableService for testing
class MockDiscoverableService: DiscoverableService {
    func listOperations() -> [OperationSchema] {
        return [
            OperationSchema(
                name: "search",
                inputSchema: ["query": AnyCodable("string")],
                title: "Search",
                description: "Perform a search"
            ),
            OperationSchema(
                name: "create",
                inputSchema: ["data": AnyCodable("object")],
                title: "Create",
                description: "Create a new item"
            )
        ]
    }
    
    func serverInfo() -> ServerInfo {
        return ServerInfo(
            server: "mock-server",
            version: "1.0.0",
            protocol: "umicp",
            features: ["discovery"],
            operationsCount: 2,
            mcpCompatible: true
        )
    }
}

final class DiscoverableServiceTests: XCTestCase {
    var service: MockDiscoverableService!
    
    override func setUp() {
        super.setUp()
        service = MockDiscoverableService()
    }
    
    func testListOperations() {
        let operations = service.listOperations()
        XCTAssertEqual(operations.count, 2)
        XCTAssertEqual(operations[0].name, "search")
        XCTAssertEqual(operations[1].name, "create")
    }
    
    func testGetSchema() {
        let schema = service.getSchema(name: "search")
        XCTAssertNotNil(schema)
        XCTAssertEqual(schema?.name, "search")
        XCTAssertEqual(schema?.title, "Search")
        
        let notFound = service.getSchema(name: "nonexistent")
        XCTAssertNil(notFound)
    }
    
    func testServerInfo() {
        let info = service.serverInfo()
        XCTAssertEqual(info.server, "mock-server")
        XCTAssertEqual(info.version, "1.0.0")
        XCTAssertEqual(info.operationsCount, 2)
        XCTAssertEqual(info.mcpCompatible, true)
    }
    
    func testGenerateOperationsResponse() throws {
        let json = try DiscoveryHelpers.generateOperationsResponse(service: service)
        XCTAssertFalse(json.isEmpty)
        XCTAssertTrue(json.contains("search"))
        XCTAssertTrue(json.contains("create"))
    }
    
    func testGenerateSchemaResponse() throws {
        let json = try DiscoveryHelpers.generateSchemaResponse(
            service: service,
            operationName: "search"
        )
        XCTAssertFalse(json.isEmpty)
        XCTAssertTrue(json.contains("search"))
        XCTAssertTrue(json.contains("Search"))
    }
    
    func testGenerateSchemaResponseNotFound() throws {
        let json = try DiscoveryHelpers.generateSchemaResponse(
            service: service,
            operationName: "nonexistent"
        )
        XCTAssertTrue(json.contains("error"))
        XCTAssertTrue(json.contains("Operation not found"))
    }
    
    func testGenerateServerInfoResponse() throws {
        let json = try DiscoveryHelpers.generateServerInfoResponse(service: service)
        XCTAssertFalse(json.isEmpty)
        XCTAssertTrue(json.contains("mock-server"))
        XCTAssertTrue(json.contains("1.0.0"))
    }
}

