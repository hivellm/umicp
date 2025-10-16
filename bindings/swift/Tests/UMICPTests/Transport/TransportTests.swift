import XCTest
@testable import UMICP

@available(macOS 12.0, iOS 15.0, *)
final class TransportTests: XCTestCase {
    func testWebSocketInitialization() {
        let url = URL(string: "ws://localhost:8080/umicp")!
        let ws = UMICPWebSocket(url: url)

        Task {
            let isConnected = await ws.getIsConnected()
            XCTAssertFalse(isConnected)
        }
    }

    func testHTTP2Initialization() {
        let url = URL(string: "http://localhost:8080")!
        let http = UMICPHTTP2(baseURL: url)

        XCTAssertNotNil(http)
    }

    func testEnvelopeRoundTrip() throws {
        let original = try EnvelopeBuilder()
            .from("client")
            .to("server")
            .operation(.data)
            .messageId("test-001")
            .capabilityString("type", value: "test")
            .buildWithHash()

        let json = try original.serialize()
        let deserialized = try Envelope.deserialize(from: json)

        XCTAssertEqual(original.from, deserialized.from)
        XCTAssertEqual(original.to, deserialized.to)
        XCTAssertEqual(original.messageId, deserialized.messageId)
    }
}

