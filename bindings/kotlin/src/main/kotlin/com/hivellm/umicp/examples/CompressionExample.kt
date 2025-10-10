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
    
    // Comparison
    println("9. GZIP vs DEFLATE Comparison:")
    val testData = "Sample data for comparison. ".repeat(100).toByteArray()
    
    val gzipSize = testData.compress(CompressionAlgorithm.GZIP).size
    val deflateSize = testData.compress(CompressionAlgorithm.DEFLATE).size
    
    println("  Original: ${testData.size} bytes")
    println("  GZIP: $gzipSize bytes")
    println("  DEFLATE: $deflateSize bytes")
    println("  Winner: ${if (gzipSize < deflateSize) "GZIP" else "DEFLATE"}")
    println()
    
    // Use Case: Large Message Transport
    println("10. Use Case - Large Message Transport:")
    val largeMessage = """
        {
            "model": "gpt-4",
            "messages": [
                {"role": "user", "content": "${"This is a large message. ".repeat(500)}"}
            ]
        }
    """.trimIndent()
    
    val original = largeMessage.toByteArray()
    val compressed = original.compress(CompressionAlgorithm.GZIP)
    
    println("  Original message: ${original.size} bytes")
    println("  Compressed: ${compressed.size} bytes")
    println("  Bandwidth savings: ${"%.1f".format(Compression.getSavings(original.size, compressed.size))}%")
    println("  ✓ Recommended for messages > 1KB")
    println()
    
    println("=== Example Complete ===")
}

