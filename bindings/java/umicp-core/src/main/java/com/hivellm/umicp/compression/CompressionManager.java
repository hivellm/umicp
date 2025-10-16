package com.hivellm.umicp.compression;

import com.hivellm.umicp.types.CompressionType;
import com.hivellm.umicp.types.UMICPException;
import org.jetbrains.annotations.NotNull;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.*;
import java.util.zip.*;

/**
 * Compression manager for UMICP messages.
 * Supports GZIP and DEFLATE compression algorithms.
 *
 * @author HiveLLM Team
 * @version 0.1.3
 */
public class CompressionManager {
    private static final Logger logger = LoggerFactory.getLogger(CompressionManager.class);

    private static final int DEFAULT_BUFFER_SIZE = 8192;
    private static final int DEFAULT_COMPRESSION_LEVEL = Deflater.DEFAULT_COMPRESSION;

    private final int compressionLevel;
    private final int bufferSize;

    /**
     * Create compression manager with default settings.
     */
    public CompressionManager() {
        this(DEFAULT_COMPRESSION_LEVEL, DEFAULT_BUFFER_SIZE);
    }

    /**
     * Create compression manager with custom settings.
     *
     * @param compressionLevel Compression level (0-9, where 9 is maximum compression)
     * @param bufferSize Buffer size for compression operations
     */
    public CompressionManager(int compressionLevel, int bufferSize) {
        if (compressionLevel < 0 || compressionLevel > 9) {
            throw new IllegalArgumentException("Compression level must be between 0 and 9");
        }
        if (bufferSize <= 0) {
            throw new IllegalArgumentException("Buffer size must be positive");
        }

        this.compressionLevel = compressionLevel;
        this.bufferSize = bufferSize;
    }

    /**
     * Compress data using specified compression type.
     *
     * @param data Data to compress
     * @param type Compression type
     * @return Compressed data
     * @throws UMICPException if compression fails
     */
    @NotNull
    public byte[] compress(@NotNull byte[] data, @NotNull CompressionType type) throws UMICPException {
        if (data.length == 0) {
            return data;
        }

        try {
            return switch (type) {
                case GZIP -> compressGzip(data);
                case DEFLATE -> compressDeflate(data);
                case NONE -> data;
                default -> throw new IllegalArgumentException("Unknown compression type: " + type);
            };
        } catch (IOException e) {
            throw new UMICPException("Compression failed: " + e.getMessage(), e);
        }
    }

    /**
     * Decompress data using specified compression type.
     *
     * @param data Compressed data
     * @param type Compression type used
     * @return Decompressed data
     * @throws UMICPException if decompression fails
     */
    @NotNull
    public byte[] decompress(@NotNull byte[] data, @NotNull CompressionType type) throws UMICPException {
        if (data.length == 0) {
            return data;
        }

        try {
            return switch (type) {
                case GZIP -> decompressGzip(data);
                case DEFLATE -> decompressDeflate(data);
                case NONE -> data;
                default -> throw new IllegalArgumentException("Unknown compression type: " + type);
            };
        } catch (IOException e) {
            throw new UMICPException("Decompression failed: " + e.getMessage(), e);
        }
    }

    /**
     * Compress data using GZIP.
     *
     * @param data Input data
     * @return GZIP compressed data
     * @throws IOException if compression fails
     */
    @NotNull
    private byte[] compressGzip(@NotNull byte[] data) throws IOException {
        try (ByteArrayOutputStream outputStream = new ByteArrayOutputStream(data.length);
             GZIPOutputStream gzipStream = new GZIPOutputStream(outputStream) {
                 {
                     def.setLevel(compressionLevel);
                 }
             }) {

            gzipStream.write(data);
            gzipStream.finish();

            byte[] compressed = outputStream.toByteArray();
            logger.debug("GZIP compressed {} bytes to {} bytes (ratio: {:.2f}%)",
                    data.length, compressed.length,
                    (compressed.length * 100.0 / data.length));

            return compressed;
        }
    }

    /**
     * Decompress GZIP data.
     *
     * @param data GZIP compressed data
     * @return Decompressed data
     * @throws IOException if decompression fails
     */
    @NotNull
    private byte[] decompressGzip(@NotNull byte[] data) throws IOException {
        try (ByteArrayInputStream inputStream = new ByteArrayInputStream(data);
             GZIPInputStream gzipStream = new GZIPInputStream(inputStream);
             ByteArrayOutputStream outputStream = new ByteArrayOutputStream()) {

            byte[] buffer = new byte[bufferSize];
            int bytesRead;

            while ((bytesRead = gzipStream.read(buffer)) != -1) {
                outputStream.write(buffer, 0, bytesRead);
            }

            return outputStream.toByteArray();
        }
    }

    /**
     * Compress data using DEFLATE.
     *
     * @param data Input data
     * @return DEFLATE compressed data
     * @throws IOException if compression fails
     */
    @NotNull
    private byte[] compressDeflate(@NotNull byte[] data) throws IOException {
        Deflater deflater = new Deflater(compressionLevel);
        deflater.setInput(data);
        deflater.finish();

        try (ByteArrayOutputStream outputStream = new ByteArrayOutputStream(data.length)) {
            byte[] buffer = new byte[bufferSize];

            while (!deflater.finished()) {
                int count = deflater.deflate(buffer);
                outputStream.write(buffer, 0, count);
            }

            byte[] compressed = outputStream.toByteArray();
            logger.debug("DEFLATE compressed {} bytes to {} bytes (ratio: {:.2f}%)",
                    data.length, compressed.length,
                    (compressed.length * 100.0 / data.length));

            return compressed;
        } finally {
            deflater.end();
        }
    }

    /**
     * Decompress DEFLATE data.
     *
     * @param data DEFLATE compressed data
     * @return Decompressed data
     * @throws IOException if decompression fails
     */
    @NotNull
    private byte[] decompressDeflate(@NotNull byte[] data) throws IOException {
        Inflater inflater = new Inflater();
        inflater.setInput(data);

        try (ByteArrayOutputStream outputStream = new ByteArrayOutputStream(data.length)) {
            byte[] buffer = new byte[bufferSize];

            while (!inflater.finished()) {
                int count = inflater.inflate(buffer);
                outputStream.write(buffer, 0, count);
            }

            return outputStream.toByteArray();
        } catch (DataFormatException e) {
            throw new IOException("Invalid DEFLATE data", e);
        } finally {
            inflater.end();
        }
    }

    /**
     * Calculate compression ratio.
     *
     * @param originalSize Original data size
     * @param compressedSize Compressed data size
     * @return Compression ratio as percentage
     */
    public static double getCompressionRatio(int originalSize, int compressedSize) {
        if (originalSize == 0) {
            return 0.0;
        }
        return (compressedSize * 100.0) / originalSize;
    }

    /**
     * Check if compression would be beneficial.
     * Returns true if data size is above threshold (typically 1KB).
     *
     * @param dataSize Data size in bytes
     * @return true if compression is recommended
     */
    public static boolean shouldCompress(int dataSize) {
        return dataSize > 1024; // 1KB threshold
    }

    /**
     * Get compression level.
     *
     * @return Compression level (0-9)
     */
    public int getCompressionLevel() {
        return compressionLevel;
    }

    /**
     * Get buffer size.
     *
     * @return Buffer size in bytes
     */
    public int getBufferSize() {
        return bufferSize;
    }
}

