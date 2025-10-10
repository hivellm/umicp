package com.hivellm.umicp.types;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for CompressionType enum
 */
class CompressionTypeTest {

    @Test
    void testValues() {
        CompressionType[] types = CompressionType.values();
        assertEquals(4, types.length);

        assertEquals(CompressionType.NONE, types[0]);
        assertEquals(CompressionType.GZIP, types[1]);
        assertEquals(CompressionType.DEFLATE, types[2]);
        assertEquals(CompressionType.LZ4, types[3]);
    }

    @Test
    void testValueOf() {
        assertEquals(CompressionType.NONE, CompressionType.valueOf("NONE"));
        assertEquals(CompressionType.GZIP, CompressionType.valueOf("GZIP"));
        assertEquals(CompressionType.DEFLATE, CompressionType.valueOf("DEFLATE"));
        assertEquals(CompressionType.LZ4, CompressionType.valueOf("LZ4"));
    }

    @Test
    void testGetValue() {
        assertEquals(0, CompressionType.NONE.getValue());
        assertEquals(1, CompressionType.GZIP.getValue());
        assertEquals(2, CompressionType.DEFLATE.getValue());
        assertEquals(3, CompressionType.LZ4.getValue());
    }

    @Test
    void testGetAlgorithm() {
        assertEquals("none", CompressionType.NONE.getAlgorithm());
        assertEquals("gzip", CompressionType.GZIP.getAlgorithm());
        assertEquals("deflate", CompressionType.DEFLATE.getAlgorithm());
        assertEquals("lz4", CompressionType.LZ4.getAlgorithm());
    }

    @Test
    void testFromValue() {
        assertEquals(CompressionType.NONE, CompressionType.fromValue(0));
        assertEquals(CompressionType.GZIP, CompressionType.fromValue(1));
        assertEquals(CompressionType.DEFLATE, CompressionType.fromValue(2));
        assertEquals(CompressionType.LZ4, CompressionType.fromValue(3));
    }

    @Test
    void testFromValueInvalid() {
        assertThrows(IllegalArgumentException.class, () -> {
            CompressionType.fromValue(99);
        });

        assertThrows(IllegalArgumentException.class, () -> {
            CompressionType.fromValue(-1);
        });
    }

    @Test
    void testFromAlgorithm() {
        assertEquals(CompressionType.NONE, CompressionType.fromAlgorithm("none"));
        assertEquals(CompressionType.GZIP, CompressionType.fromAlgorithm("gzip"));
        assertEquals(CompressionType.DEFLATE, CompressionType.fromAlgorithm("deflate"));
        assertEquals(CompressionType.LZ4, CompressionType.fromAlgorithm("lz4"));
    }

    @Test
    void testFromAlgorithmCaseInsensitive() {
        assertEquals(CompressionType.GZIP, CompressionType.fromAlgorithm("GZIP"));
        assertEquals(CompressionType.GZIP, CompressionType.fromAlgorithm("Gzip"));
        assertEquals(CompressionType.DEFLATE, CompressionType.fromAlgorithm("DEFLATE"));
        assertEquals(CompressionType.LZ4, CompressionType.fromAlgorithm("LZ4"));
    }

    @Test
    void testFromAlgorithmInvalid() {
        assertThrows(IllegalArgumentException.class, () -> {
            CompressionType.fromAlgorithm("invalid");
        });
    }

    @Test
    void testFromAlgorithmNull() {
        assertThrows(IllegalArgumentException.class, () -> {
            CompressionType.fromAlgorithm(null);
        });
    }

    @Test
    void testToString() {
        assertEquals("none", CompressionType.NONE.toString());
        assertEquals("gzip", CompressionType.GZIP.toString());
        assertEquals("deflate", CompressionType.DEFLATE.toString());
        assertEquals("lz4", CompressionType.LZ4.toString());
    }

    @Test
    void testSwitchStatement() {
        // Test that all enum values can be used in switch
        for (CompressionType type : CompressionType.values()) {
            String result = switch (type) {
                case NONE -> "no compression";
                case GZIP -> "gzip compression";
                case DEFLATE -> "deflate compression";
                case LZ4 -> "lz4 compression";
            };
            assertNotNull(result);
        }
    }
}

