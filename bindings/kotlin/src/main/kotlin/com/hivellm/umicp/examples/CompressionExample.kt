package com.hivellm.umicp.examples

import com.hivellm.umicp.compression.Compression
import com.hivellm.umicp.compression.CompressionAlgorithm
import com.hivellm.umicp.compression.compress
import com.hivellm.umicp.compression.decompress
import com.hivellm.umicp.core.Envelope
import com.hivellm.umicp.types.OperationType

/**
 * Compression Example
 *
 * Demonstrates:
 * - GZIP compression
 * - DEFLATE compression
 * - Compression statistics
 * - Extension functions
 */
fun main() {
    println("=== UMICP Kotlin SDK - Compression Example ===\n")

    // Sample data
    println("1. Basic Compression:")
    val originalText = "Hello, UMICP! This is a test message that demonstrates compression capabilities.".repeat(50)
    val originalData = originalText.toByteArray()

    println("  Original size: ${originalData.size} bytes")
    println("  Original text preview: ${originalText.take(80)}...")
    println()

    // GZIP Compression
    println("2. GZIP Compression:")
    val gzipCompressed = Compression.compress(originalData, CompressionAlgorithm.GZIP)
    val gzipDecompressed = Compression.decompress(gzipCompressed, CompressionAlgorithm.GZIP)

    println("  Compressed size: ${gzipCompressed.size} bytes")
    println("  Decompressed size: ${gzipDecompressed.size} bytes")
    println("  Ratio: ${"%.2f".format(Compression.getCompressionRatio(originalData.size, gzipCompressed.size))}")
    println("  Savings: ${"%.1f".format(Compression.getSavings(originalData.size, gzipCompressed.size))}%")
    println("  Match: ${String(gzipDecompressed, Charsets.UTF_8) == originalText}")
    println()

    // DEFLATE Compression
    println("3. DEFLATE Compression:")
    val deflateCompressed = Compression.compress(originalData, CompressionAlgorithm.DEFLATE)
    val deflateDecompressed = Compression.decompress(deflateCompressed, CompressionAlgorithm.DEFLATE)

    println("  Compressed size: ${deflateCompressed.size} bytes")
    println("  Savings: ${"%.1f".format(Compression.getSavings(originalData.size, deflateCompressed.size))}%")
    println("  Match: ${String(deflateDecompressed, Charsets.UTF_8) == originalText}")
    println()

    // Compression Levels
    println("4. Compression Levels (GZIP):")
    for (level in listOf(1, 6, 9)) {
        val compressed = Compression.compress(originalData, CompressionAlgorithm.GZIP, level)
        println("  Level $level: ${compressed.size} bytes (${("%.1f".format(Compression.getSavings(originalData.size, compressed.size)))}% saved)")
    }
    println()

    // String Compression
    println("5. String Compression:")
    val text = "Quick compression test! ".repeat(100)
    val compressedString = Compression.compressString(text, CompressionAlgorithm.GZIP)
    val decompressedString = Compression.decompressString(compressedString, CompressionAlgorithm.GZIP)

    println("  Original: ${text.length} chars")
    println("  Compressed: ${compressedString.size} bytes")
    println("  Match: ${decompressedString == text}")
    println()

    // Extension Functions
    println("6. Extension Functions:")
    val data = "Test data ".repeat(200).toByteArray()
    val compressed = data.compress(CompressionAlgorithm.GZIP)
    val decompressed = compressed.decompress(CompressionAlgorithm.GZIP)

    println("  Original: ${data.size} bytes")
    println("  Compressed: ${compressed.size} bytes")
    println("  Decompressed: ${decompressed.size} bytes")
    println("  Match: ${decompressed.contentEquals(data)}")
    println()

    // Compressing Envelopes
    println("7. Compressing Envelopes:")
    val envelope = Envelope.build {
        from("client-001")
        to("server-001")
        operation(OperationType.DATA)
        capability("data", "A".repeat(1000))
    }

    val envelopeJson = envelope.serialize()
    val envelopeCompressed = envelopeJson.compress(CompressionAlgorithm.GZIP)

    println("  JSON size: ${envelopeJson.length} bytes")
    println("  Compressed: ${envelopeCompressed.size} bytes")
    println("  Savings: ${"%.1f".format(Compression.getSavings(envelopeJson.length, envelopeCompressed.size))}%")
    println()

    // Highly Compressible Data
    println("8. Highly Repetitive Data:")
    val repetitive = "A".repeat(10000).toByteArray()
    val highlyCompressed = repetitive.compress(CompressionAlgorithm.GZIP)
    val savings = Compression.getSavings(repetitive.size, highlyCompressed.size)

    println("  Original: ${repetitive.size} bytes (all 'A's)")
    println("  Compressed: ${highlyCompressed.size} bytes")
    println("  Savings: ${"%.2f".format(savings)}%")
    println()

    // LZ4 Compression
    println("9. LZ4 Compression (Fast):")
    val lz4Data = "Fast compression test! ".repeat(100).toByteArray()

    val lz4Start = System.nanoTime()
    val lz4Compressed = lz4Data.compress(CompressionAlgorithm.LZ4)
    val lz4Time = (System.nanoTime() - lz4Start) / 1_000_000.0

    val lz4Decompressed = lz4Compressed.decompress(CompressionAlgorithm.LZ4)

    println("  Original: ${lz4Data.size} bytes")
    println("  Compressed: ${lz4Compressed.size} bytes")
    println("  Savings: ${"%.1f".format(Compression.getSavings(lz4Data.size, lz4Compressed.size))}%")
    println("  Time: ${"%.3f".format(lz4Time)}ms")
    println("  Match: ${lz4Decompressed.contentEquals(lz4Data)}")
    println()

    // LZ4 High Compression
    println("10. LZ4 High Compression:")
    val lz4HcCompressed = lz4Data.compress(CompressionAlgorithm.LZ4_HC, 9)
    val lz4HcDecompressed = lz4HcCompressed.decompress(CompressionAlgorithm.LZ4_HC)

    println("  LZ4:    ${lz4Compressed.size} bytes")
    println("  LZ4_HC: ${lz4HcCompressed.size} bytes")
    println("  Improvement: ${lz4Compressed.size - lz4HcCompressed.size} bytes")
    println("  Match: ${lz4HcDecompressed.contentEquals(lz4Data)}")
    println()

    // Comparison
    println("11. Algorithm Comparison:")
    val testData = "Sample data for comparison. ".repeat(100).toByteArray()

    val gzipSize = testData.compress(CompressionAlgorithm.GZIP).size
    val deflateSize = testData.compress(CompressionAlgorithm.DEFLATE).size
    val lz4Size = testData.compress(CompressionAlgorithm.LZ4).size
    val lz4HcSize = testData.compress(CompressionAlgorithm.LZ4_HC, 9).size

    println("  Original: ${testData.size} bytes")
    println("  GZIP:     $gzipSize bytes (${"%.1f".format(Compression.getSavings(testData.size, gzipSize))}% saved)")
    println("  DEFLATE:  $deflateSize bytes (${"%.1f".format(Compression.getSavings(testData.size, deflateSize))}% saved)")
    println("  LZ4:      $lz4Size bytes (${"%.1f".format(Compression.getSavings(testData.size, lz4Size))}% saved) [FASTEST]")
    println("  LZ4_HC:   $lz4HcSize bytes (${"%.1f".format(Compression.getSavings(testData.size, lz4HcSize))}% saved)")
    println()

    // Speed Comparison
    println("12. Speed Comparison (1MB data):")
    val bigData = "X".repeat(1024 * 1024).toByteArray()

    println("  Testing GZIP...")
    val gzipSpeedStart = System.nanoTime()
    bigData.compress(CompressionAlgorithm.GZIP)
    val gzipSpeed = (System.nanoTime() - gzipSpeedStart) / 1_000_000.0

    println("  Testing LZ4...")
    val lz4SpeedStart = System.nanoTime()
    bigData.compress(CompressionAlgorithm.LZ4)
    val lz4Speed = (System.nanoTime() - lz4SpeedStart) / 1_000_000.0

    println("  GZIP: ${"%.1f".format(gzipSpeed)}ms")
    println("  LZ4:  ${"%.1f".format(lz4Speed)}ms")
    println("  LZ4 is ${"%.1f".format(gzipSpeed / lz4Speed)}x faster!")
    println()

    // Use Case: Large Message Transport
    println("13. Use Case - Large Message Transport:")
    val largeMessage = """
        {
            "model": "gpt-4",
            "messages": [
                {"role": "user", "content": "${"This is a large message. ".repeat(500)}"}
            ]
        }
    """.trimIndent()

    val original = largeMessage.toByteArray()

    println("  Original message: ${original.size} bytes")
    println()
    println("  Testing compression algorithms:")

    val gzipComp = original.compress(CompressionAlgorithm.GZIP)
    println("    GZIP:   ${gzipComp.size} bytes (${"%.1f".format(Compression.getSavings(original.size, gzipComp.size))}% saved)")

    val lz4Comp = original.compress(CompressionAlgorithm.LZ4)
    println("    LZ4:    ${lz4Comp.size} bytes (${"%.1f".format(Compression.getSavings(original.size, lz4Comp.size))}% saved) [FASTEST]")

    println()
    println("  Recommendation:")
    println("    - Use LZ4 for real-time applications (fastest)")
    println("    - Use GZIP for bandwidth-constrained networks (best compression)")
    println("    - Use LZ4_HC for balanced approach")
    println()

    println("=== Example Complete ===")
}

