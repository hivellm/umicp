import XCTest
@testable import UMICP

final class EnvelopeTests: XCTestCase {
    func testBasicEnvelopeCreation() throws {
        let envelope = try EnvelopeBuilder()
            .from("client-001")
            .to("server-001")
            .operation(.data)
            .messageId("msg-001")
            .build()
        
        XCTAssertEqual(envelope.from, "client-001")
        XCTAssertEqual(envelope.to, "server-001")
        XCTAssertEqual(envelope.operation, .data)
        XCTAssertEqual(envelope.messageId, "msg-001")
    }
    
    func testEnvelopeWithCapabilities() throws {
        let envelope = try EnvelopeBuilder()
            .from("client-001")
            .to("server-001")
            .operation(.data)
            .messageId("msg-001")
            .capabilityString("type", value: "request")
            .capabilityInt("timeout", value: 5000)
            .capabilityBool("streaming", value: true)
            .build()
        
        XCTAssertEqual(envelope.capabilities.count, 3)
        XCTAssertEqual(envelope.getCapability("type") as? String, "request")
        XCTAssertEqual(envelope.getCapability("timeout") as? Int, 5000)
        XCTAssertEqual(envelope.getCapability("streaming") as? Bool, true)
    }
    
    func testEnvelopeValidation() {
        XCTAssertThrowsError(try EnvelopeBuilder()
            .from("")
            .to("server")
            .operation(.data)
            .messageId("msg")
            .build()
        )
        
        XCTAssertThrowsError(try EnvelopeBuilder()
            .from("client")
            .to("")
            .operation(.data)
            .messageId("msg")
            .build()
        )
        
        XCTAssertThrowsError(try EnvelopeBuilder()
            .from("client")
            .to("server")
            .operation(.data)
            .messageId("")
            .build()
        )
    }
    
    func testEnvelopeSerialization() throws {
        let envelope = try EnvelopeBuilder()
            .from("client-001")
            .to("server-001")
            .operation(.data)
            .messageId("msg-001")
            .capabilityString("key", value: "value")
            .build()
        
        let json = try envelope.serialize()
        XCTAssertFalse(json.isEmpty)
        XCTAssertTrue(json.contains("client-001"))
        XCTAssertTrue(json.contains("server-001"))
    }
    
    func testEnvelopeDeserialization() throws {
        let json = """
        {
            "from": "client-001",
            "to": "server-001",
            "operation": 0,
            "message_id": "msg-001",
            "capabilities": {"key": "value"}
        }
        """
        
        let envelope = try Envelope.deserialize(from: json)
        XCTAssertEqual(envelope.from, "client-001")
        XCTAssertEqual(envelope.to, "server-001")
        XCTAssertEqual(envelope.operation, .data)
        XCTAssertEqual(envelope.messageId, "msg-001")
    }
    
    func testEnvelopeHashCalculation() throws {
        var envelope = try EnvelopeBuilder()
            .from("client-001")
            .to("server-001")
            .operation(.data)
            .messageId("msg-001")
            .build()
        
        try envelope.calculateHash()
        XCTAssertNotNil(envelope.hash)
        XCTAssertEqual(envelope.hash?.count, 64) // SHA-256 hex string
    }
    
    func testEnvelopeHashVerification() throws {
        let envelope = try EnvelopeBuilder()
            .from("client-001")
            .to("server-001")
            .operation(.data)
            .messageId("msg-001")
            .buildWithHash()
        
        XCTAssertTrue(try envelope.verifyHash())
    }
    
    func testEnvelopeWithPayload() throws {
        let payload = "Hello UMICP!".data(using: .utf8)!
        let envelope = try EnvelopeBuilder()
            .from("client")
            .to("server")
            .operation(.data)
            .messageId("msg")
            .payload(payload)
            .build()
        
        XCTAssertEqual(envelope.payload, payload)
    }
    
    func testAllOperationTypes() throws {
        for opType in OperationType.allCases {
            let envelope = try EnvelopeBuilder()
                .from("client")
                .to("server")
                .operation(opType)
                .messageId("msg-\(opType.rawValue)")
                .build()
            
            XCTAssertEqual(envelope.operation, opType)
        }
    }
    
    func testNativeTypesInCapabilities() throws {
        let envelope = try EnvelopeBuilder()
            .from("client")
            .to("server")
            .operation(.data)
            .messageId("msg")
            .capability("string", value: "test")
            .capability("int", value: 42)
            .capability("bool", value: true)
            .capability("double", value: 3.14)
            .capability("array", value: [1, 2, 3])
            .capability("dict", value: ["key": "value"])
            .build()
        
        XCTAssertEqual(envelope.capabilities.count, 6)
        XCTAssertNotNil(envelope.getCapability("string"))
        XCTAssertNotNil(envelope.getCapability("int"))
        XCTAssertNotNil(envelope.getCapability("bool"))
        XCTAssertNotNil(envelope.getCapability("double"))
        XCTAssertNotNil(envelope.getCapability("array"))
        XCTAssertNotNil(envelope.getCapability("dict"))
    }
}

