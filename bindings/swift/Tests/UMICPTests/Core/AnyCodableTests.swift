import XCTest
@testable import UMICP

final class AnyCodableTests: XCTestCase {
    func testEncodeBool() throws {
        let value = AnyCodable(true)
        let encoded = try JSONEncoder().encode(value)
        let decoded = try JSONDecoder().decode(AnyCodable.self, from: encoded)
        XCTAssertEqual(decoded.value as? Bool, true)
    }

    func testEncodeInt() throws {
        let value = AnyCodable(42)
        let encoded = try JSONEncoder().encode(value)
        let decoded = try JSONDecoder().decode(AnyCodable.self, from: encoded)
        XCTAssertEqual(decoded.value as? Int, 42)
    }

    func testEncodeDouble() throws {
        let value = AnyCodable(3.14)
        let encoded = try JSONEncoder().encode(value)
        let decoded = try JSONDecoder().decode(AnyCodable.self, from: encoded)
        XCTAssertEqual(try XCTUnwrap(decoded.value as? Double), 3.14, accuracy: 0.001)
    }

    func testEncodeString() throws {
        let value = AnyCodable("hello")
        let encoded = try JSONEncoder().encode(value)
        let decoded = try JSONDecoder().decode(AnyCodable.self, from: encoded)
        XCTAssertEqual(decoded.value as? String, "hello")
    }

    func testEncodeArray() throws {
        let value = AnyCodable([1, 2, 3])
        let encoded = try JSONEncoder().encode(value)
        let decoded = try JSONDecoder().decode(AnyCodable.self, from: encoded)
        let array = decoded.value as? [Any]
        XCTAssertEqual(array?.count, 3)
    }

    func testEncodeDictionary() throws {
        let dict: [String: Any] = ["key": "value", "count": 42]
        let value = AnyCodable(dict)
        let encoded = try JSONEncoder().encode(value)
        let decoded = try JSONDecoder().decode(AnyCodable.self, from: encoded)
        let decodedDict = decoded.value as? [String: Any]
        XCTAssertNotNil(decodedDict)
    }

    func testEquality() {
        XCTAssertEqual(AnyCodable(42), AnyCodable(42))
        XCTAssertEqual(AnyCodable("test"), AnyCodable("test"))
        XCTAssertEqual(AnyCodable(true), AnyCodable(true))
        XCTAssertNotEqual(AnyCodable(42), AnyCodable(43))
    }
}

