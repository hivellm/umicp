package com.hivellm.umicp.compression;

import com.hivellm.umicp.types.CompressionType;
import com.hivellm.umicp.types.UMICPException;
import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

import java.nio.charset.StandardCharsets;
import java.util.Random;

/**
 * Unit tests for Compression
 */
class CompressionTest {

    @Test
    void testCompressDecompressGzip() throws UMICPException {
        // Use longer, repetitive text to ensure compression
        String original = "Hello, UMICP! This is a test message for compression. ".repeat(50);
        byte[] data = original.getBytes(StandardCharsets.UTF_8);

        byte[] compressed = Compression.compress(data, CompressionType.GZIP);
        assertNotNull(compressed);
        assertTrue(compressed.length < data.length,
            String.format("GZIP: compressed (%d) should be < original (%d)", 
                compressed.length, data.length));

        byte[] decompressed = Compression.decompress(compressed, CompressionType.GZIP);
        assertArrayEquals(data, decompressed);

        String result = new String(decompressed, StandardCharsets.UTF_8);
        assertEquals(original, result);
    }

    @Test
    void testCompressDecompressDeflate() throws UMICPException {
        // Use longer, repetitive text to ensure compression
        String original = "Hello, UMICP! This is a test message for deflate compression. ".repeat(50);
        byte[] data = original.getBytes(StandardCharsets.UTF_8);

        byte[] compressed = Compression.compress(data, CompressionType.DEFLATE);
        assertNotNull(compressed);
        assertTrue(compressed.length < data.length,
            String.format("DEFLATE: compressed (%d) should be < original (%d)", 
                compressed.length, data.length));

        byte[] decompressed = Compression.decompress(compressed, CompressionType.DEFLATE);
        assertArrayEquals(data, decompressed);

        String result = new String(decompressed, StandardCharsets.UTF_8);
        assertEquals(original, result);
    }

    @Test
    void testCompressNone() throws UMICPException {
        byte[] data = "Test data".getBytes(StandardCharsets.UTF_8);

        byte[] result = Compression.compress(data, CompressionType.NONE);
        assertArrayEquals(data, result);

        byte[] decompressed = Compression.decompress(result, CompressionType.NONE);
        assertArrayEquals(data, decompressed);
    }

    @Test
    void testCompressLargeData() throws UMICPException {
        // Create large repetitive data (compresses well)
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 1000; i++) {
            sb.append("This is line ").append(i).append(" of test data.\n");
        }
        byte[] data = sb.toString().getBytes(StandardCharsets.UTF_8);

        byte[] compressed = Compression.compress(data, CompressionType.GZIP);
        assertTrue(compressed.length < data.length / 2); // Should compress significantly

        byte[] decompressed = Compression.decompress(compressed, CompressionType.GZIP);
        assertArrayEquals(data, decompressed);
    }

    @Test
    void testCompressRandomData() throws UMICPException {
        // Random data doesn't compress well
        byte[] data = new byte[1000];
        new Random(42).nextBytes(data);

        byte[] compressed = Compression.compress(data, CompressionType.GZIP);
        assertNotNull(compressed);

        byte[] decompressed = Compression.decompress(compressed, CompressionType.GZIP);
        assertArrayEquals(data, decompressed);
    }

    @Test
    void testCompressEmptyData() throws UMICPException {
        byte[] data = new byte[0];

        byte[] compressed = Compression.compress(data, CompressionType.GZIP);
        assertNotNull(compressed);

        byte[] decompressed = Compression.decompress(compressed, CompressionType.GZIP);
        assertEquals(0, decompressed.length);
    }

    @Test
    void testCompressNullData() {
        assertThrows(IllegalArgumentException.class, () -> {
            Compression.compress(null, CompressionType.GZIP);
        });
    }

    @Test
    void testCompressNullType() {
        byte[] data = "test".getBytes();
        assertThrows(IllegalArgumentException.class, () -> {
            Compression.compress(data, null);
        });
    }

    @Test
    void testDecompressNullData() {
        assertThrows(IllegalArgumentException.class, () -> {
            Compression.decompress(null, CompressionType.GZIP);
        });
    }

    @Test
    void testDecompressNullType() {
        byte[] data = "test".getBytes();
        assertThrows(IllegalArgumentException.class, () -> {
            Compression.decompress(data, null);
        });
    }

    @Test
    void testDecompressInvalidData() {
        byte[] invalidData = "not compressed data".getBytes();

        assertThrows(UMICPException.class, () -> {
            Compression.decompress(invalidData, CompressionType.GZIP);
        });
    }

    @Test
    void testCompressLZ4NotImplemented() {
        byte[] data = "test".getBytes();

        assertThrows(UMICPException.class, () -> {
            Compression.compress(data, CompressionType.LZ4);
        });
    }

    @Test
    void testDecompressLZ4NotImplemented() {
        byte[] data = "test".getBytes();

        assertThrows(UMICPException.class, () -> {
            Compression.decompress(data, CompressionType.LZ4);
        });
    }

    @Test
    void testCompressionRatio() {
        double ratio = Compression.getCompressionRatio(1000, 500);
        assertEquals(2.0, ratio, 0.01);

        ratio = Compression.getCompressionRatio(1000, 250);
        assertEquals(4.0, ratio, 0.01);

        ratio = Compression.getCompressionRatio(1000, 0);
        assertEquals(0.0, ratio, 0.01);
    }

    @Test
    void testIsBeneficial() {
        // 50% compression is beneficial
        assertTrue(Compression.isBeneficial(1000, 500));

        // 15% compression is beneficial
        assertTrue(Compression.isBeneficial(1000, 850));

        // 5% compression is not beneficial (threshold is 10%)
        assertFalse(Compression.isBeneficial(1000, 950));

        // No compression is not beneficial
        assertFalse(Compression.isBeneficial(1000, 1000));

        // Expansion is not beneficial
        assertFalse(Compression.isBeneficial(1000, 1100));
    }

    @Test
    void testRoundTripWithSpecialCharacters() throws UMICPException {
        String original = "Special chars: 你好世界 🌍 émojis 😀 symbols: @#$%^&*()";
        byte[] data = original.getBytes(StandardCharsets.UTF_8);

        byte[] compressed = Compression.compress(data, CompressionType.GZIP);
        byte[] decompressed = Compression.decompress(compressed, CompressionType.GZIP);

        String result = new String(decompressed, StandardCharsets.UTF_8);
        assertEquals(original, result);
    }

    @Test
    void testGzipVsDeflate() throws UMICPException {
        // Use longer text for reliable compression comparison
        String text = "This is a test message that will be compressed using different algorithms. ".repeat(20);
        byte[] data = text.getBytes(StandardCharsets.UTF_8);

        byte[] gzipCompressed = Compression.compress(data, CompressionType.GZIP);
        byte[] deflateCompressed = Compression.compress(data, CompressionType.DEFLATE);

        // Both should compress
        assertTrue(gzipCompressed.length < data.length,
            String.format("GZIP (%d) should be < original (%d)", gzipCompressed.length, data.length));
        assertTrue(deflateCompressed.length < data.length,
            String.format("DEFLATE (%d) should be < original (%d)", deflateCompressed.length, data.length));

        // Deflate should be slightly smaller (no GZIP headers)
        assertTrue(deflateCompressed.length < gzipCompressed.length,
            String.format("DEFLATE (%d) should be < GZIP (%d)", deflateCompressed.length, gzipCompressed.length));

        // Both should decompress correctly
        byte[] gzipDecompressed = Compression.decompress(gzipCompressed, CompressionType.GZIP);
        byte[] deflateDecompressed = Compression.decompress(deflateCompressed, CompressionType.DEFLATE);

        assertArrayEquals(data, gzipDecompressed);
        assertArrayEquals(data, deflateDecompressed);
    }

    @Test
    void testMultipleCompressionCycles() throws UMICPException {
        String original = "Test data for multiple compression cycles";
        byte[] data = original.getBytes(StandardCharsets.UTF_8);

        // Compress and decompress multiple times
        for (int i = 0; i < 5; i++) {
            byte[] compressed = Compression.compress(data, CompressionType.GZIP);
            byte[] decompressed = Compression.decompress(compressed, CompressionType.GZIP);
            assertArrayEquals(data, decompressed);
        }
    }
}

