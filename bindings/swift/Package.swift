// swift-tools-version: 5.9
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "UMICP-SDK",
    platforms: [
        .macOS(.v12),
        .iOS(.v15)
    ],
    products: [
        .library(
            name: "UMICP-SDK",
            targets: ["UMICP-SDK"]),
    ],
    dependencies: [],
    targets: [
        .target(
            name: "UMICP-SDK",
            dependencies: [],
            path: "Sources/UMICP"
        ),
        .testTarget(
            name: "UMICPTests",
            dependencies: ["UMICP-SDK"],
            path: "Tests/UMICPTests"
        ),
    ]
)

