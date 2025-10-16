import XCTest
@testable import UMICP

final class OperationSchemaTests: XCTestCase {
    func testBasicSchemaCreation() {
        let schema = OperationSchema(
            name: "search",
            inputSchema: ["query": AnyCodable("string")]
        )
        
        XCTAssertEqual(schema.name, "search")
        XCTAssertEqual(schema.inputSchema.count, 1)
        XCTAssertNil(schema.title)
        XCTAssertNil(schema.description)
    }
    
    func testSchemaWithAllFields() {
        let schema = OperationSchema(
            name: "search",
            inputSchema: ["query": AnyCodable("string")],
            title: "Search Operation",
            description: "Performs a search query",
            outputSchema: ["results": AnyCodable(["array"])],
            annotations: ["read_only": AnyCodable(true)]
        )
        
        XCTAssertEqual(schema.name, "search")
        XCTAssertEqual(schema.title, "Search Operation")
        XCTAssertEqual(schema.description, "Performs a search query")
        XCTAssertNotNil(schema.outputSchema)
        XCTAssertNotNil(schema.annotations)
    }
    
    func testSchemaBuilderPattern() {
        let schema = OperationSchema(
            name: "search",
            inputSchema: ["query": AnyCodable("string")]
        )
        .withTitle("Search")
        .withDescription("Search operation")
        .withAnnotations(["read_only": AnyCodable(true)])
        
        XCTAssertEqual(schema.title, "Search")
        XCTAssertEqual(schema.description, "Search operation")
        XCTAssertNotNil(schema.annotations)
    }
    
    func testSchemaSerialization() throws {
        let schema = OperationSchema(
            name: "search",
            inputSchema: [
                "type": AnyCodable("object"),
                "properties": AnyCodable([
                    "query": ["type": "string"]
                ])
            ]
        )
        
        let encoded = try JSONEncoder().encode(schema)
        let decoded = try JSONDecoder().decode(OperationSchema.self, from: encoded)
        
        XCTAssertEqual(decoded.name, schema.name)
        XCTAssertEqual(decoded.inputSchema.count, schema.inputSchema.count)
    }
    
    func testSchemaToDictionary() {
        let schema = OperationSchema(
            name: "search",
            inputSchema: ["query": AnyCodable("string")],
            title: "Search",
            description: "Search operation"
        )
        
        let dict = schema.toDictionary()
        XCTAssertEqual(dict["name"] as? String, "search")
        XCTAssertEqual(dict["title"] as? String, "Search")
        XCTAssertEqual(dict["description"] as? String, "Search operation")
        XCTAssertNotNil(dict["input_schema"])
    }
    
    func testMCPCompatibleSchema() throws {
        let schema = OperationSchema(
            name: "get_weather",
            inputSchema: [
                "type": AnyCodable("object"),
                "properties": AnyCodable([
                    "location": ["type": "string"],
                    "units": ["type": "string", "enum": ["celsius", "fahrenheit"]]
                ]),
                "required": AnyCodable(["location"])
            ],
            title: "Get Weather",
            description: "Get current weather for a location"
        )
        
        let dict = schema.toDictionary()
        XCTAssertNotNil(dict["input_schema"])
        
        // Verify MCP JSON Schema structure
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.sortedKeys, .prettyPrinted]
        let data = try encoder.encode(schema)
        let json = String(data: data, encoding: .utf8)!
        
        XCTAssertTrue(json.contains("input_schema"))
        XCTAssertTrue(json.contains("get_weather"))
    }
}

