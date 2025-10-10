package com.hivellm.umicp.compression;

import com.hivellm.umicp.types.CompressionType;
import com.hivellm.umicp.types.UMICPException;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.zip.*;

/**
 * Compression utilities for UMICP
 * Supports GZIP and DEFLATE compression algorithms
 */
public class Compression {

    /**
     * Compress data using specified algorithm
     *
     * @param data Data to compress
     * @param type Compression algorithm
     * @return Compressed data
     * @throws UMICPException if compression fails
     */
    public static byte[] compress(byte[] data, CompressionType type) throws UMICPException {
        if (data == null) {
            throw new IllegalArgumentException("Data cannot be null");
        }
        if (type == null) {
            throw new IllegalArgumentException("Compression type cannot be null");
        }

        if (type == CompressionType.NONE) {
            return data;
        }

        try {
            switch (type) {
                case GZIP:
                    return compressGzip(data);
                case DEFLATE:
                    return compressDeflate(data);
                case LZ4:
                    throw new UMICPException("LZ4 compression not yet implemented");
                default:
                    throw new UMICPException("Unsupported compression type: " + type);
            }
        } catch (IOException e) {
            throw new UMICPException("Compression failed: " + e.getMessage(), e);
        }
    }

    /**
     * Decompress data using specified algorithm
     *
     * @param data Compressed data
     * @param type Compression algorithm used
     * @return Decompressed data
     * @throws UMICPException if decompression fails
     */
    public static byte[] decompress(byte[] data, CompressionType type) throws UMICPException {
        if (data == null) {
            throw new IllegalArgumentException("Data cannot be null");
        }
        if (type == null) {
            throw new IllegalArgumentException("Compression type cannot be null");
        }

        if (type == CompressionType.NONE) {
            return data;
        }

        try {
            switch (type) {
                case GZIP:
                    return decompressGzip(data);
                case DEFLATE:
                    return decompressDeflate(data);
                case LZ4:
                    throw new UMICPException("LZ4 decompression not yet implemented");
                default:
                    throw new UMICPException("Unsupported compression type: " + type);
            }
        } catch (IOException e) {
            throw new UMICPException("Decompression failed: " + e.getMessage(), e);
        }
    }

    /**
     * Compress data using GZIP
     *
     * @param data Data to compress
     * @return Compressed data
     * @throws IOException if compression fails
     */
    private static byte[] compressGzip(byte[] data) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try (GZIPOutputStream gzipOut = new GZIPOutputStream(baos)) {
            gzipOut.write(data);
            gzipOut.finish();
        }
        return baos.toByteArray();
    }

    /**
     * Decompress GZIP data
     *
     * @param data Compressed data
     * @return Decompressed data
     * @throws IOException if decompression fails
     */
    private static byte[] decompressGzip(byte[] data) throws IOException {
        ByteArrayInputStream bais = new ByteArrayInputStream(data);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();

        try (GZIPInputStream gzipIn = new GZIPInputStream(bais)) {
            byte[] buffer = new byte[8192];
            int len;
            while ((len = gzipIn.read(buffer)) > 0) {
                baos.write(buffer, 0, len);
            }
        }

        return baos.toByteArray();
    }

    /**
     * Compress data using DEFLATE
     *
     * @param data Data to compress
     * @return Compressed data
     * @throws IOException if compression fails
     */
    private static byte[] compressDeflate(byte[] data) throws IOException {
        Deflater deflater = new Deflater();
        deflater.setInput(data);
        deflater.finish();

        ByteArrayOutputStream baos = new ByteArrayOutputStream(data.length);
        byte[] buffer = new byte[8192];

        while (!deflater.finished()) {
            int count = deflater.deflate(buffer);
            baos.write(buffer, 0, count);
        }

        deflater.end();
        return baos.toByteArray();
    }

    /**
     * Decompress DEFLATE data
     *
     * @param data Compressed data
     * @return Decompressed data
     * @throws IOException if decompression fails
     */
    private static byte[] decompressDeflate(byte[] data) throws IOException {
        Inflater inflater = new Inflater();
        inflater.setInput(data);

        ByteArrayOutputStream baos = new ByteArrayOutputStream(data.length);
        byte[] buffer = new byte[8192];

        try {
            while (!inflater.finished()) {
                int count = inflater.inflate(buffer);
                baos.write(buffer, 0, count);
            }
        } catch (DataFormatException e) {
            throw new IOException("Invalid compressed data", e);
        } finally {
            inflater.end();
        }

        return baos.toByteArray();
    }

    /**
     * Get compression ratio
     *
     * @param originalSize Original data size
     * @param compressedSize Compressed data size
     * @return Compression ratio (e.g., 2.5 means 2.5x reduction)
     */
    public static double getCompressionRatio(int originalSize, int compressedSize) {
        if (compressedSize == 0) {
            return 0.0;
        }
        return (double) originalSize / compressedSize;
    }

    /**
     * Check if compression is beneficial
     * Returns true if compressed size is at least 10% smaller
     *
     * @param originalSize Original data size
     * @param compressedSize Compressed data size
     * @return true if compression saved at least 10%
     */
    public static boolean isBeneficial(int originalSize, int compressedSize) {
        return compressedSize < (originalSize * 0.9);
    }
}

