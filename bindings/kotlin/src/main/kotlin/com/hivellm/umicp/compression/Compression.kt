package com.hivellm.umicp.compression

import com.hivellm.umicp.types.UMICPException
import net.jpountz.lz4.LZ4Factory
import net.jpountz.lz4.LZ4Compressor
import net.jpountz.lz4.LZ4FastDecompressor
import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import java.nio.ByteBuffer
import java.util.zip.GZIPInputStream
import java.util.zip.GZIPOutputStream
import java.util.zip.Deflater
import java.util.zip.Inflater

/**
 * Compression algorithm types
 */
enum class CompressionAlgorithm {
    /** No compression */
    NONE,

    /** GZIP compression */
    GZIP,

    /** DEFLATE compression */
    DEFLATE,

    /** LZ4 compression (fast) */
    LZ4,

    /** LZ4 High compression */
    LZ4_HC;

    companion object {
        fun fromString(value: String): CompressionAlgorithm? =
            entries.find { it.name.equals(value, ignoreCase = true) }
    }
}

/**
 * Exception thrown during compression/decompression operations
 */
class CompressionException(message: String, cause: Throwable? = null) : UMICPException(message, cause)

/**
 * Compression utilities for UMICP
 *
 * Provides GZIP and DEFLATE compression/decompression for message payloads.
 */
object Compression {

    /**
     * Compress data using specified algorithm
     *
     * @param data Data to compress
     * @param algorithm Compression algorithm
     * @param level Compression level (0-9, default 6)
     * @return Compressed data
     * @throws CompressionException if compression fails
     */
    fun compress(
        data: ByteArray,
        algorithm: CompressionAlgorithm = CompressionAlgorithm.GZIP,
        level: Int = 6
    ): ByteArray {
        if (algorithm == CompressionAlgorithm.NONE) {
            return data
        }

        return try {
            when (algorithm) {
                CompressionAlgorithm.GZIP -> compressGzip(data, level)
                CompressionAlgorithm.DEFLATE -> compressDeflate(data, level)
                CompressionAlgorithm.LZ4 -> compressLz4(data)
                CompressionAlgorithm.LZ4_HC -> compressLz4Hc(data, level)
                CompressionAlgorithm.NONE -> data
            }
        } catch (e: Exception) {
            throw CompressionException("Failed to compress data with $algorithm", e)
        }
    }

    /**
     * Decompress data using specified algorithm
     *
     * @param data Compressed data
     * @param algorithm Compression algorithm used
     * @return Decompressed data
     * @throws CompressionException if decompression fails
     */
    fun decompress(
        data: ByteArray,
        algorithm: CompressionAlgorithm = CompressionAlgorithm.GZIP
    ): ByteArray {
        if (algorithm == CompressionAlgorithm.NONE) {
            return data
        }

        return try {
            when (algorithm) {
                CompressionAlgorithm.GZIP -> decompressGzip(data)
                CompressionAlgorithm.DEFLATE -> decompressDeflate(data)
                CompressionAlgorithm.LZ4 -> decompressLz4(data)
                CompressionAlgorithm.LZ4_HC -> decompressLz4(data) // Same decompressor
                CompressionAlgorithm.NONE -> data
            }
        } catch (e: Exception) {
            throw CompressionException("Failed to decompress data with $algorithm", e)
        }
    }

    /**
     * Compress string using specified algorithm
     *
     * @param text Text to compress
     * @param algorithm Compression algorithm
     * @param level Compression level
     * @return Compressed data
     */
    fun compressString(
        text: String,
        algorithm: CompressionAlgorithm = CompressionAlgorithm.GZIP,
        level: Int = 6
    ): ByteArray {
        return compress(text.toByteArray(Charsets.UTF_8), algorithm, level)
    }

    /**
     * Decompress to string using specified algorithm
     *
     * @param data Compressed data
     * @param algorithm Compression algorithm used
     * @return Decompressed string
     */
    fun decompressString(
        data: ByteArray,
        algorithm: CompressionAlgorithm = CompressionAlgorithm.GZIP
    ): String {
        return String(decompress(data, algorithm), Charsets.UTF_8)
    }

    /**
     * Get compression ratio
     *
     * @param original Original data size
     * @param compressed Compressed data size
     * @return Compression ratio (0.0 to 1.0, lower is better)
     */
    fun getCompressionRatio(original: Int, compressed: Int): Double {
        if (original == 0) return 0.0
        return compressed.toDouble() / original.toDouble()
    }

    /**
     * Calculate space saved by compression
     *
     * @param original Original data size
     * @param compressed Compressed data size
     * @return Percentage of space saved (0-100)
     */
    fun getSavings(original: Int, compressed: Int): Double {
        if (original == 0) return 0.0
        return ((original - compressed).toDouble() / original.toDouble()) * 100.0
    }

    // Private compression methods

    private fun compressGzip(data: ByteArray, level: Int): ByteArray {
        val output = ByteArrayOutputStream()
        // `def` is the protected Deflater declared on DeflaterOutputStream (the
        // superclass), so it is reachable from an anonymous subclass without the
        // reflection that used to fail with NoSuchFieldException on newer JDKs.
        val gzip = object : GZIPOutputStream(output) {
            init { def.setLevel(level) }
        }
        gzip.use { it.write(data) }
        return output.toByteArray()
    }

    private fun decompressGzip(data: ByteArray): ByteArray {
        val input = ByteArrayInputStream(data)
        val output = ByteArrayOutputStream()

        GZIPInputStream(input).use { gzip ->
            val buffer = ByteArray(4096)
            var len: Int
            while (gzip.read(buffer).also { len = it } > 0) {
                output.write(buffer, 0, len)
            }
        }

        return output.toByteArray()
    }

    private fun compressDeflate(data: ByteArray, level: Int): ByteArray {
        val deflater = Deflater(level)
        deflater.setInput(data)
        deflater.finish()

        val output = ByteArrayOutputStream()
        val buffer = ByteArray(4096)

        while (!deflater.finished()) {
            val count = deflater.deflate(buffer)
            output.write(buffer, 0, count)
        }

        deflater.end()
        return output.toByteArray()
    }

    private fun decompressDeflate(data: ByteArray): ByteArray {
        val inflater = Inflater()
        inflater.setInput(data)

        val output = ByteArrayOutputStream()
        val buffer = ByteArray(4096)

        while (!inflater.finished()) {
            val count = inflater.inflate(buffer)
            output.write(buffer, 0, count)
        }

        inflater.end()
        return output.toByteArray()
    }

    private fun compressLz4(data: ByteArray): ByteArray {
        val factory = LZ4Factory.fastestInstance()
        val compressor: LZ4Compressor = factory.fastCompressor()

        val maxCompressedLength = compressor.maxCompressedLength(data.size)
        val compressed = ByteArray(maxCompressedLength)

        val compressedLength = compressor.compress(data, 0, data.size, compressed, 0, maxCompressedLength)

        // Store original size + compressed data
        val result = ByteBuffer.allocate(4 + compressedLength)
        result.putInt(data.size)
        result.put(compressed, 0, compressedLength)

        return result.array()
    }

    private fun compressLz4Hc(data: ByteArray, level: Int): ByteArray {
        val factory = LZ4Factory.fastestInstance()
        val compressor: LZ4Compressor = factory.highCompressor(level)

        val maxCompressedLength = compressor.maxCompressedLength(data.size)
        val compressed = ByteArray(maxCompressedLength)

        val compressedLength = compressor.compress(data, 0, data.size, compressed, 0, maxCompressedLength)

        // Store original size + compressed data
        val result = ByteBuffer.allocate(4 + compressedLength)
        result.putInt(data.size)
        result.put(compressed, 0, compressedLength)

        return result.array()
    }

    private fun decompressLz4(data: ByteArray): ByteArray {
        val factory = LZ4Factory.fastestInstance()
        val decompressor: LZ4FastDecompressor = factory.fastDecompressor()

        // Read original size
        val buffer = ByteBuffer.wrap(data)
        val originalSize = buffer.getInt()

        val compressed = ByteArray(data.size - 4)
        buffer.get(compressed)

        val restored = ByteArray(originalSize)
        decompressor.decompress(compressed, 0, restored, 0, originalSize)

        return restored
    }
}

/**
 * Extension function to compress byte array
 */
fun ByteArray.compress(
    algorithm: CompressionAlgorithm = CompressionAlgorithm.GZIP,
    level: Int = 6
): ByteArray = Compression.compress(this, algorithm, level)

/**
 * Extension function to decompress byte array
 */
fun ByteArray.decompress(
    algorithm: CompressionAlgorithm = CompressionAlgorithm.GZIP
): ByteArray = Compression.decompress(this, algorithm)

/**
 * Extension function to compress string
 */
fun String.compress(
    algorithm: CompressionAlgorithm = CompressionAlgorithm.GZIP,
    level: Int = 6
): ByteArray = Compression.compressString(this, algorithm, level)

