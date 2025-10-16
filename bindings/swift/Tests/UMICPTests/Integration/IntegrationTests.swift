import XCTest
@testable import UMICP

final class IntegrationTests: XCTestCase {
    func testFullEnvelopeWorkflow() throws {
        // Create envelope
        let envelope = try UMICP.builder()
            .from("client-001")
            .to("server-001")
            .operation(.request)
            .messageId("req-\(UUID().uuidString)")
            .capabilityString("action", value: "search")
            .capabilityInt("max_results", value: 10)
            .capabilityBool("include_metadata", value: true)
            .buildWithHash()
        
        // Validate
        try envelope.validate()
        XCTAssertTrue(try envelope.verifyHash())
        
        // Serialize
        let json = try envelope.serialize()
        XCTAssertFalse(json.isEmpty)
        
        // Deserialize
        let restored = try Envelope.deserialize(from: json)
        XCTAssertEqual(restored.from, envelope.from)
        XCTAssertEqual(restored.hash, envelope.hash)
        
        // Verify capabilities
        XCTAssertEqual(restored.getCapability("action") as? String, "search")
        XCTAssertEqual(restored.getCapability("max_results") as? Int, 10)
        XCTAssertEqual(restored.getCapability("include_metadata") as? Bool, true)
    }
    
    func testDiscoveryWorkflow() throws {
        let service = MockDiscoverableService()
        
        // List operations
        let operations = service.listOperations()
        XCTAssertEqual(operations.count, 2)
        
        // Get specific schema
        let schema = service.getSchema(name: "search")
        XCTAssertNotNil(schema)
        
        // Get server info
        let info = service.serverInfo()
        XCTAssertEqual(info.mcpCompatible, true)
        
        // Generate responses
        let opsJson = try DiscoveryHelpers.generateOperationsResponse(service: service)
        XCTAssertTrue(opsJson.contains("search"))
        
        let schemaJson = try DiscoveryHelpers.generateSchemaResponse(
            service: service,
            operationName: "search"
        )
        XCTAssertTrue(schemaJson.contains("Search"))
        
        let infoJson = try DiscoveryHelpers.generateServerInfoResponse(service: service)
        XCTAssertTrue(infoJson.contains("mock-server"))
    }
    
    func testMatrixOperationsWorkflow() throws {
        let ops = UMICP.matrixOps()
        
        // Vector operations
        let v1 = [1.0, 2.0, 3.0]
        let v2 = [4.0, 5.0, 6.0]
        
        let sum = try ops.vectorAdd(v1, v2)
        XCTAssertEqual(sum[0], 5.0, accuracy: 0.001)
        
        let scaled = ops.vectorScale(v1, scalar: 2.0)
        XCTAssertEqual(scaled[0], 2.0, accuracy: 0.001)
        
        let dot = try ops.dotProduct(v1, v2)
        XCTAssertEqual(dot, 32.0, accuracy: 0.001)
        
        // Matrix multiplication
        let m1 = [1.0, 2.0, 3.0, 4.0]
        let m2 = [5.0, 6.0, 7.0, 8.0]
        let result = try ops.matrixMultiply(m1, m2, rows: 2, cols: 2, k: 2)
        XCTAssertEqual(result.count, 4)
    }
    
    func testErrorHandling() {
        // Invalid envelope
        XCTAssertThrowsError(try EnvelopeBuilder()
            .from("")
            .to("server")
            .operation(.data)
            .messageId("msg")
            .build()
        )
        
        // Invalid deserialization
        XCTAssertThrowsError(try Envelope.deserialize(from: "invalid json"))
        
        // Invalid matrix operations
        let ops = MatrixOperations()
        XCTAssertThrowsError(try ops.vectorAdd([1.0], [1.0, 2.0]))
        XCTAssertThrowsError(try ops.vectorNormalize([0.0, 0.0]))
    }
    
    func testVersionInformation() {
        XCTAssertEqual(UMICP.version, "0.2.0")
        XCTAssertEqual(UMICP.protocolVersion, "0.2")
    }
}

