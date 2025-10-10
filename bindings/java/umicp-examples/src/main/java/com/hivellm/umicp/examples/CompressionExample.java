package com.hivellm.umicp.examples;

import com.hivellm.umicp.compression.Compression;
import com.hivellm.umicp.types.CompressionType;
import com.hivellm.umicp.types.UMICPException;

import java.nio.charset.StandardCharsets;
import java.util.Random;

/**
 * Example demonstrating UMICP Compression
 */
public class CompressionExample {
    public static void main(String[] args) {
        System.out.println("=== UMICP Compression Example ===\n");

        try {
            // Example 1: Compress and decompress text
            System.out.println("1. Text Compression (GZIP):");
            String text = "Hello, UMICP! This is a test message for compression. " +
                         "Compression is very useful for reducing network bandwidth.";
            byte[] originalData = text.getBytes(StandardCharsets.UTF_8);

            System.out.println("   Original text: " + text);
            System.out.println("   Original size: " + originalData.length + " bytes");

            byte[] compressed = Compression.compress(originalData, CompressionType.GZIP);
            System.out.println("   Compressed size: " + compressed.length + " bytes");
            System.out.println("   Compression ratio: " +
                String.format("%.2f", Compression.getCompressionRatio(originalData.length, compressed.length)) + "x");
            System.out.println("   Beneficial: " +
                Compression.isBeneficial(originalData.length, compressed.length));

            byte[] decompressed = Compression.decompress(compressed, CompressionType.GZIP);
            String result = new String(decompressed, StandardCharsets.UTF_8);

            System.out.println("   Decompressed: " + result);
            System.out.println("   ✓ Compression successful!");
            System.out.println();

            // Example 2: Large repetitive data
            System.out.println("2. Large Repetitive Data (GZIP):");
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < 100; i++) {
                sb.append("This is line ").append(i).append(" of repetitive test data.\n");
            }
            byte[] largeData = sb.toString().getBytes(StandardCharsets.UTF_8);

            System.out.println("   Original size: " + largeData.length + " bytes");

            byte[] largeCompressed = Compression.compress(largeData, CompressionType.GZIP);
            System.out.println("   Compressed size: " + largeCompressed.length + " bytes");
            System.out.println("   Compression ratio: " +
                String.format("%.2f", Compression.getCompressionRatio(largeData.length, largeCompressed.length)) + "x");
            System.out.println("   Savings: " +
                String.format("%.1f%%", (1.0 - (double)largeCompressed.length/largeData.length) * 100));
            System.out.println();

            // Example 3: GZIP vs DEFLATE
            System.out.println("3. GZIP vs DEFLATE:");
            byte[] testData = "Test data for comparing compression algorithms".getBytes(StandardCharsets.UTF_8);

            byte[] gzipCompressed = Compression.compress(testData, CompressionType.GZIP);
            byte[] deflateCompressed = Compression.compress(testData, CompressionType.DEFLATE);

            System.out.println("   Original: " + testData.length + " bytes");
            System.out.println("   GZIP: " + gzipCompressed.length + " bytes");
            System.out.println("   DEFLATE: " + deflateCompressed.length + " bytes");
            System.out.println("   DEFLATE is smaller: " + (deflateCompressed.length < gzipCompressed.length));
            System.out.println();

            // Example 4: Random data (doesn't compress well)
            System.out.println("4. Random Data Compression:");
            byte[] randomData = new byte[1000];
            new Random(42).nextBytes(randomData);

            System.out.println("   Original size: " + randomData.length + " bytes");

            byte[] randomCompressed = Compression.compress(randomData, CompressionType.GZIP);
            System.out.println("   Compressed size: " + randomCompressed.length + " bytes");
            System.out.println("   Compression ratio: " +
                String.format("%.2f", Compression.getCompressionRatio(randomData.length, randomCompressed.length)) + "x");
            System.out.println("   Beneficial: " +
                Compression.isBeneficial(randomData.length, randomCompressed.length));
            System.out.println("   Note: Random data compresses poorly");
            System.out.println();

            // Example 5: No compression
            System.out.println("5. No Compression:");
            byte[] noneData = "test".getBytes();
            byte[] noneResult = Compression.compress(noneData, CompressionType.NONE);

            System.out.println("   Original size: " + noneData.length);
            System.out.println("   Result size: " + noneResult.length);
            System.out.println("   Same data: " + (noneData == noneResult));
            System.out.println();

            // Example 6: Special characters
            System.out.println("6. Unicode and Special Characters:");
            String unicode = "Special chars: 你好世界 🌍 émojis 😀 symbols: @#$%^&*()";
            byte[] unicodeData = unicode.getBytes(StandardCharsets.UTF_8);

            System.out.println("   Original: " + unicode);
            System.out.println("   Original size: " + unicodeData.length + " bytes");

            byte[] unicodeCompressed = Compression.compress(unicodeData, CompressionType.GZIP);
            byte[] unicodeDecompressed = Compression.decompress(unicodeCompressed, CompressionType.GZIP);
            String unicodeResult = new String(unicodeDecompressed, StandardCharsets.UTF_8);

            System.out.println("   Compressed size: " + unicodeCompressed.length + " bytes");
            System.out.println("   Decompressed: " + unicodeResult);
            System.out.println("   Match: " + unicode.equals(unicodeResult));
            System.out.println();

            System.out.println("=== Compression Example Complete ===");

        } catch (UMICPException e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}

