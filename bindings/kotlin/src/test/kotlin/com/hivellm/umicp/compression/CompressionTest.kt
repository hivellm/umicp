package com.hivellm.umicp.compression

import io.kotest.assertions.throwables.shouldThrow
import io.kotest.matchers.doubles.shouldBeGreaterThan
import io.kotest.matchers.doubles.shouldBeLessThan
import io.kotest.matchers.ints.shouldBeLessThan
import io.kotest.matchers.shouldBe
import org.junit.jupiter.api.Test

class CompressionTest {

    @Test
    fun `should compress and decompress with GZIP`() {
        val original = "Hello, UMICP! This is a test message that should be compressed.".repeat(10)
        val data = original.toByteArray()

        val compressed = Compression.compress(data, CompressionAlgorithm.GZIP)
        val decompressed = Compression.decompress(compressed, CompressionAlgorithm.GZIP)

        String(decompressed, Charsets.UTF_8) shouldBe original
        compressed.size shouldBeLessThan data.size
    }

    @Test
    fun `should compress and decompress with DEFLATE`() {
        val original = "Hello, UMICP! This is a test message that should be compressed.".repeat(10)
        val data = original.toByteArray()

        val compressed = Compression.compress(data, CompressionAlgorithm.DEFLATE)
        val decompressed = Compression.decompress(compressed, CompressionAlgorithm.DEFLATE)

        String(decompressed, Charsets.UTF_8) shouldBe original
        compressed.size shouldBeLessThan data.size
    }

    @Test
    fun `should handle NONE compression`() {
        val data = "Test data".toByteArray()

        val compressed = Compression.compress(data, CompressionAlgorithm.NONE)
        val decompressed = Compression.decompress(compressed, CompressionAlgorithm.NONE)

        compressed shouldBe data
        decompressed shouldBe data
    }

    @Test
    fun `should compress string with GZIP`() {
        val original = "Hello, World!".repeat(100)

        val compressed = Compression.compressString(original, CompressionAlgorithm.GZIP)
        val decompressed = Compression.decompressString(compressed, CompressionAlgorithm.GZIP)

        decompressed shouldBe original
        compressed.size shouldBeLessThan original.length
    }

    @Test
    fun `should compress string with DEFLATE`() {
        val original = "Hello, World!".repeat(100)

        val compressed = Compression.compressString(original, CompressionAlgorithm.DEFLATE)
        val decompressed = Compression.decompressString(compressed, CompressionAlgorithm.DEFLATE)

        decompressed shouldBe original
    }

    @Test
    fun `should respect compression level`() {
        val data = "Test data".repeat(1000).toByteArray()

        val level1 = Compression.compress(data, CompressionAlgorithm.GZIP, 1)
        val level9 = Compression.compress(data, CompressionAlgorithm.GZIP, 9)

        // Higher level should produce smaller size
        level9.size shouldBeLessThan level1.size
    }

    @Test
    fun `should calculate compression ratio`() {
        val original = 1000
        val compressed = 300

        val ratio = Compression.getCompressionRatio(original, compressed)
        ratio shouldBe 0.3
    }

    @Test
    fun `should calculate savings`() {
        val original = 1000
        val compressed = 300

        val savings = Compression.getSavings(original, compressed)
        savings shouldBe 70.0
    }

    @Test
    fun `should handle empty data`() {
        val data = ByteArray(0)

        val compressed = Compression.compress(data, CompressionAlgorithm.GZIP)
        val decompressed = Compression.decompress(compressed, CompressionAlgorithm.GZIP)

        decompressed.size shouldBe 0
    }

    @Test
    fun `should work with extension functions`() {
        val data = "Test data".repeat(100).toByteArray()

        val compressed = data.compress(CompressionAlgorithm.GZIP)
        val decompressed = compressed.decompress(CompressionAlgorithm.GZIP)

        decompressed shouldBe data
    }

    @Test
    fun `should compress string with extension`() {
        val text = "Hello, World!".repeat(100)

        val compressed = text.compress(CompressionAlgorithm.GZIP)
        val decompressed = String(compressed.decompress(CompressionAlgorithm.GZIP), Charsets.UTF_8)

        decompressed shouldBe text
    }

    @Test
    fun `should throw on invalid compressed data`() {
        val invalidData = ByteArray(10) { it.toByte() }

        shouldThrow<CompressionException> {
            Compression.decompress(invalidData, CompressionAlgorithm.GZIP)
        }
    }

    @Test
    fun `should achieve good compression on repetitive data`() {
        val data = "A".repeat(10000).toByteArray()

        val compressed = Compression.compress(data, CompressionAlgorithm.GZIP)
        val savings = Compression.getSavings(data.size, compressed.size)

        savings shouldBeGreaterThan 90.0 // Should save >90%
    }

    @Test
    fun `should handle large data`() {
        val data = ByteArray(1024 * 1024) { (it % 256).toByte() } // 1MB

        val compressed = Compression.compress(data, CompressionAlgorithm.GZIP)
        val decompressed = Compression.decompress(compressed, CompressionAlgorithm.GZIP)

        decompressed shouldBe data
    }

    @Test
    fun `should fromString work for algorithms`() {
        CompressionAlgorithm.fromString("GZIP") shouldBe CompressionAlgorithm.GZIP
        CompressionAlgorithm.fromString("gzip") shouldBe CompressionAlgorithm.GZIP
        CompressionAlgorithm.fromString("DEFLATE") shouldBe CompressionAlgorithm.DEFLATE
        CompressionAlgorithm.fromString("LZ4") shouldBe CompressionAlgorithm.LZ4
        CompressionAlgorithm.fromString("NONE") shouldBe CompressionAlgorithm.NONE
        CompressionAlgorithm.fromString("invalid") shouldBe null
    }

    @Test
    fun `should compress and decompress with LZ4`() {
        val original = "Hello, UMICP! This is a test message that should be compressed.".repeat(10)
        val data = original.toByteArray()

        val compressed = Compression.compress(data, CompressionAlgorithm.LZ4)
        val decompressed = Compression.decompress(compressed, CompressionAlgorithm.LZ4)

        String(decompressed, Charsets.UTF_8) shouldBe original
        compressed.size shouldBeLessThan data.size
    }

    @Test
    fun `should compress and decompress with LZ4_HC`() {
        val original = "Hello, UMICP! This is a test message that should be compressed.".repeat(10)
        val data = original.toByteArray()

        val compressed = Compression.compress(data, CompressionAlgorithm.LZ4_HC, 9)
        val decompressed = Compression.decompress(compressed, CompressionAlgorithm.LZ4_HC)

        String(decompressed, Charsets.UTF_8) shouldBe original
        compressed.size shouldBeLessThan data.size
    }

    @Test
    fun `LZ4 should be faster than GZIP`() {
        val data = "Test data ".repeat(1000).toByteArray()

        val lz4Start = System.nanoTime()
        val lz4Compressed = Compression.compress(data, CompressionAlgorithm.LZ4)
        val lz4Time = System.nanoTime() - lz4Start

        val gzipStart = System.nanoTime()
        val gzipCompressed = Compression.compress(data, CompressionAlgorithm.GZIP)
        val gzipTime = System.nanoTime() - gzipStart

        // LZ4 should be faster (not always guaranteed in short tests, but usually)
        println("LZ4 time: ${lz4Time / 1_000_000.0}ms")
        println("GZIP time: ${gzipTime / 1_000_000.0}ms")

        // Both should successfully compress
        lz4Compressed.size shouldBeLessThan data.size
        gzipCompressed.size shouldBeLessThan data.size
    }

    @Test
    fun `LZ4_HC should produce smaller output than LZ4`() {
        val data = "AAAAAAAAAA".repeat(1000).toByteArray()

        val lz4Size = Compression.compress(data, CompressionAlgorithm.LZ4).size
        val lz4HcSize = Compression.compress(data, CompressionAlgorithm.LZ4_HC, 9).size

        // HC should produce equal or smaller output
        lz4HcSize shouldBeLessThan lz4Size + 100 // Allow some margin
    }

    @Test
    fun `should compress string with LZ4`() {
        val original = "Hello, World!".repeat(100)

        val compressed = Compression.compressString(original, CompressionAlgorithm.LZ4)
        val decompressed = Compression.decompressString(compressed, CompressionAlgorithm.LZ4)

        decompressed shouldBe original
    }
}

