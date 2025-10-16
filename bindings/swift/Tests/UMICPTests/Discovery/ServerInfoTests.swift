import XCTest
@testable import UMICP

final class ServerInfoTests: XCTestCase {
    func testBasicServerInfo() {
        let info = ServerInfo(
            server: "umicp-swift",
            version: "0.2.0",
            protocol: "umicp"
        )

        XCTAssertEqual(info.server, "umicp-swift")
        XCTAssertEqual(info.version, "0.2.0")
        XCTAssertEqual(info.protocol, "umicp")
    }

    func testServerInfoWithFeatures() {
        let info = ServerInfo(
            server: "umicp-swift",
            version: "0.2.0",
            protocol: "umicp",
            features: ["tool_discovery", "native_types", "websocket"],
            operationsCount: 10,
            mcpCompatible: true
        )

        XCTAssertEqual(info.features?.count, 3)
        XCTAssertEqual(info.operationsCount, 10)
        XCTAssertEqual(info.mcpCompatible, true)
    }

    func testServerInfoBuilderPattern() {
        let info = ServerInfo(
            server: "umicp-swift",
            version: "0.2.0",
            protocol: "umicp"
        )
        .withFeatures(["discovery", "matrix"])
        .withOperationsCount(5)
        .withMcpCompatible(true)
        .withMetadata(["platform": AnyCodable("swift")])

        XCTAssertEqual(info.features?.count, 2)
        XCTAssertEqual(info.operationsCount, 5)
        XCTAssertEqual(info.mcpCompatible, true)
        XCTAssertNotNil(info.metadata)
    }

    func testServerInfoSerialization() throws {
        let info = ServerInfo(
            server: "umicp-swift",
            version: "0.2.0",
            protocol: "umicp",
            features: ["discovery"],
            operationsCount: 5
        )

        let encoded = try JSONEncoder().encode(info)
        let decoded = try JSONDecoder().decode(ServerInfo.self, from: encoded)

        XCTAssertEqual(decoded.server, info.server)
        XCTAssertEqual(decoded.version, info.version)
        XCTAssertEqual(decoded.features?.count, info.features?.count)
    }

    func testServerInfoToDictionary() {
        let info = ServerInfo(
            server: "umicp-swift",
            version: "0.2.0",
            protocol: "umicp",
            features: ["discovery"],
            operationsCount: 5,
            mcpCompatible: true
        )

        let dict = info.toDictionary()
        XCTAssertEqual(dict["server"] as? String, "umicp-swift")
        XCTAssertEqual(dict["version"] as? String, "0.2.0")
        XCTAssertEqual(dict["operations_count"] as? Int, 5)
        XCTAssertEqual(dict["mcp_compatible"] as? Bool, true)
    }
}

