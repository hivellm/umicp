import XCTest
@testable import UMICP

final class MatrixOperationsTests: XCTestCase {
    var ops: MatrixOperations!

    override func setUp() {
        super.setUp()
        ops = MatrixOperations()
    }

    func testVectorAdd() throws {
        let a = [1.0, 2.0, 3.0]
        let b = [4.0, 5.0, 6.0]
        let result = try ops.vectorAdd(a, b)

        XCTAssertEqual(result.count, 3)
        XCTAssertEqual(result[0], 5.0, accuracy: 0.0001)
        XCTAssertEqual(result[1], 7.0, accuracy: 0.0001)
        XCTAssertEqual(result[2], 9.0, accuracy: 0.0001)
    }

    func testVectorSubtract() throws {
        let a = [5.0, 7.0, 9.0]
        let b = [1.0, 2.0, 3.0]
        let result = try ops.vectorSubtract(a, b)

        XCTAssertEqual(result.count, 3)
        XCTAssertEqual(result[0], 4.0, accuracy: 0.0001)
        XCTAssertEqual(result[1], 5.0, accuracy: 0.0001)
        XCTAssertEqual(result[2], 6.0, accuracy: 0.0001)
    }

    func testVectorScale() {
        let vector = [1.0, 2.0, 3.0]
        let result = ops.vectorScale(vector, scalar: 2.0)

        XCTAssertEqual(result.count, 3)
        XCTAssertEqual(result[0], 2.0, accuracy: 0.0001)
        XCTAssertEqual(result[1], 4.0, accuracy: 0.0001)
        XCTAssertEqual(result[2], 6.0, accuracy: 0.0001)
    }

    func testDotProduct() throws {
        let a = [1.0, 2.0, 3.0]
        let b = [4.0, 5.0, 6.0]
        let result = try ops.dotProduct(a, b)

        // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
        XCTAssertEqual(result, 32.0, accuracy: 0.0001)
    }

    func testMatrixMultiply2x2() throws {
        let a = [1.0, 2.0, 3.0, 4.0] // 2x2 matrix
        let b = [5.0, 6.0, 7.0, 8.0] // 2x2 matrix
        let result = try ops.matrixMultiply(a, b, rows: 2, cols: 2, k: 2)

        // [[1,2], [3,4]] * [[5,6], [7,8]]
        // = [[1*5+2*7, 1*6+2*8], [3*5+4*7, 3*6+4*8]]
        // = [[19, 22], [43, 50]]
        XCTAssertEqual(result.count, 4)
        XCTAssertEqual(result[0], 19.0, accuracy: 0.0001)
        XCTAssertEqual(result[1], 22.0, accuracy: 0.0001)
        XCTAssertEqual(result[2], 43.0, accuracy: 0.0001)
        XCTAssertEqual(result[3], 50.0, accuracy: 0.0001)
    }

    func testVectorMagnitude() {
        let vector = [3.0, 4.0]
        let magnitude = ops.vectorMagnitude(vector)

        // sqrt(3^2 + 4^2) = sqrt(9 + 16) = sqrt(25) = 5
        XCTAssertEqual(magnitude, 5.0, accuracy: 0.0001)
    }

    func testVectorNormalize() throws {
        let vector = [3.0, 4.0]
        let normalized = try ops.vectorNormalize(vector)

        XCTAssertEqual(normalized.count, 2)
        XCTAssertEqual(normalized[0], 0.6, accuracy: 0.0001)
        XCTAssertEqual(normalized[1], 0.8, accuracy: 0.0001)

        // Verify unit length
        let magnitude = ops.vectorMagnitude(normalized)
        XCTAssertEqual(magnitude, 1.0, accuracy: 0.0001)
    }

    func testVectorDimensionMismatch() {
        let a = [1.0, 2.0]
        let b = [1.0, 2.0, 3.0]

        XCTAssertThrowsError(try ops.vectorAdd(a, b))
        XCTAssertThrowsError(try ops.vectorSubtract(a, b))
        XCTAssertThrowsError(try ops.dotProduct(a, b))
    }

    func testMatrixInvalidDimensions() {
        let a = [1.0, 2.0, 3.0]
        let b = [4.0, 5.0]

        XCTAssertThrowsError(try ops.matrixMultiply(a, b, rows: 2, cols: 2, k: 2))
    }

    func testNormalizeZeroVector() {
        let zero = [0.0, 0.0, 0.0]
        XCTAssertThrowsError(try ops.vectorNormalize(zero))
    }
}

