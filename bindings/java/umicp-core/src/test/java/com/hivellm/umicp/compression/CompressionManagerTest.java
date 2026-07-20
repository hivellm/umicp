package com.hivellm.umicp.compression;

import com.hivellm.umicp.types.CompressionType;
import com.hivellm.umicp.types.UMICPException;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.nio.charset.StandardCharsets;
import java.util.Random;

import static org.junit.jupiter.api.Assertions.*;

class CompressionManagerTest {

    private CompressionManager manager;

    @BeforeEach
    void setUp() {
        manager = new CompressionManager();
    }

    @Test
    void testGzipCompressDecompress() throws UMICPException {
        String original = "Hello, World! This is a test message for GZIP compression.";
        byte[] data = original.getBytes(StandardCharsets.UTF_8);

        byte[] compressed = manager.compress(data, CompressionType.GZIP);
        assertNotNull(compressed);
        assertTrue(compressed.length > 0);

        byte[] decompressed = manager.decompress(compressed, CompressionType.GZIP);
        assertNotNull(decompressed);
        assertArrayEquals(data, decompressed);

        String result = new String(decompressed, StandardCharsets.UTF_8);
        assertEquals(original, result);
    }

    @Test
    void testDeflateCompressDecompress() throws UMICPException {
        String original = "Hello, World! This is a test message for DEFLATE compression.";
        byte[] data = original.getBytes(StandardCharsets.UTF_8);

        byte[] compressed = manager.compress(data, CompressionType.DEFLATE);
        assertNotNull(compressed);
        assertTrue(compressed.length > 0);

        byte[] decompressed = manager.decompress(compressed, CompressionType.DEFLATE);
        assertNotNull(decompressed);
        assertArrayEquals(data, decompressed);

        String result = new String(decompressed, StandardCharsets.UTF_8);
        assertEquals(original, result);
    }

    @Test
    void testNoneCompression() throws UMICPException {
        String original = "No compression test";
        byte[] data = original.getBytes(StandardCharsets.UTF_8);

        byte[] compressed = manager.compress(data, CompressionType.NONE);
        assertArrayEquals(data, compressed);

        byte[] decompressed = manager.decompress(compressed, CompressionType.NONE);
        assertArrayEquals(data, decompressed);
    }

    @Test
    void testEmptyData() throws UMICPException {
        byte[] empty = new byte[0];

        byte[] compressedGzip = manager.compress(empty, CompressionType.GZIP);
        assertArrayEquals(empty, compressedGzip);

        byte[] compressedDeflate = manager.compress(empty, CompressionType.DEFLATE);
        assertArrayEquals(empty, compressedDeflate);
    }

    @Test
    void testLargeData() throws UMICPException {
        // Generate 1MB of repetitive data (compressible)
        byte[] data = new byte[1024 * 1024];
        for (int i = 0; i < data.length; i++) {
            data[i] = (byte)(i % 256);
        }

        byte[] compressed = manager.compress(data, CompressionType.GZIP);
        assertNotNull(compressed);
        assertTrue(compressed.length < data.length, 
            String.format("Compressed size (%d) should be less than original (%d)", 
                compressed.length, data.length));

        byte[] decompressed = manager.decompress(compressed, CompressionType.GZIP);
        assertArrayEquals(data, decompressed);
    }

    @Test
    void testRepetitiveData() throws UMICPException {
        // Highly compressible data
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 1000; i++) {
            sb.append("AAAAAAAAAA");
        }
        byte[] data = sb.toString().getBytes(StandardCharsets.UTF_8);

        byte[] compressedGzip = manager.compress(data, CompressionType.GZIP);
        assertTrue(compressedGzip.length < data.length * 0.1); // Should compress very well

        byte[] compressedDeflate = manager.compress(data, CompressionType.DEFLATE);
        assertTrue(compressedDeflate.length < data.length * 0.1);

        byte[] decompressedGzip = manager.decompress(compressedGzip, CompressionType.GZIP);
        assertArrayEquals(data, decompressedGzip);

        byte[] decompressedDeflate = manager.decompress(compressedDeflate, CompressionType.DEFLATE);
        assertArrayEquals(data, decompressedDeflate);
    }

    @Test
    void testCompressionRatio() {
        double ratio = CompressionManager.getCompressionRatio(1000, 500);
        assertEquals(50.0, ratio, 0.01);

        ratio = CompressionManager.getCompressionRatio(0, 500);
        assertEquals(0.0, ratio, 0.01);
    }

    @Test
    void testShouldCompress() {
        assertFalse(CompressionManager.shouldCompress(100));
        assertFalse(CompressionManager.shouldCompress(1024));
        assertTrue(CompressionManager.shouldCompress(1025));
        assertTrue(CompressionManager.shouldCompress(10000));
    }

    @Test
    void testCustomCompressionLevel() throws UMICPException {
        CompressionManager fastCompression = new CompressionManager(1, 8192);
        CompressionManager maxCompression = new CompressionManager(9, 8192);

        String original = "Test data for compression level comparison. ".repeat(100);
        byte[] data = original.getBytes(StandardCharsets.UTF_8);

        byte[] compressedFast = fastCompression.compress(data, CompressionType.GZIP);
        byte[] compressedMax = maxCompression.compress(data, CompressionType.GZIP);

        // Max compression should produce smaller output
        assertTrue(compressedMax.length <= compressedFast.length);

        // Both should decompress correctly
        byte[] decompressedFast = fastCompression.decompress(compressedFast, CompressionType.GZIP);
        byte[] decompressedMax = maxCompression.decompress(compressedMax, CompressionType.GZIP);

        assertArrayEquals(data, decompressedFast);
        assertArrayEquals(data, decompressedMax);
    }

    @Test
    void testInvalidCompressionLevel() {
        assertThrows(IllegalArgumentException.class, () -> new CompressionManager(-2, 8192));
        assertThrows(IllegalArgumentException.class, () -> new CompressionManager(10, 8192));
    }

    @Test
    void testInvalidBufferSize() {
        assertThrows(IllegalArgumentException.class, () -> new CompressionManager(5, 0));
        assertThrows(IllegalArgumentException.class, () -> new CompressionManager(5, -100));
    }

    @Test
    void testGetters() {
        CompressionManager custom = new CompressionManager(7, 16384);
        assertEquals(7, custom.getCompressionLevel());
        assertEquals(16384, custom.getBufferSize());
    }

    @Test
    void testJsonCompression() throws UMICPException {
        String json = "{\"from\":\"client-1\",\"to\":\"server-1\",\"operation\":\"data\",\"message_id\":\"msg-123\",\"timestamp\":\"2025-10-10T12:00:00Z\",\"capabilities\":{\"key1\":\"value1\",\"key2\":\"value2\"}}";
        byte[] data = json.getBytes(StandardCharsets.UTF_8);

        byte[] compressed = manager.compress(data, CompressionType.DEFLATE);
        assertTrue(compressed.length < data.length);

        byte[] decompressed = manager.decompress(compressed, CompressionType.DEFLATE);
        assertEquals(json, new String(decompressed, StandardCharsets.UTF_8));
    }
}

