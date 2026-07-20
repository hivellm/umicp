package com.hivellm.umicp.types;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Test class for PayloadType enum.
 */
public class PayloadTypeTest {

    @Test
    public void testPayloadType_Values() {
        PayloadType[] types = PayloadType.values();
        
        assertNotNull(types);
        assertEquals(4, types.length);
        
        // Verify all expected types exist
        boolean hasVector = false;
        boolean hasText = false;
        boolean hasMetadata = false;
        boolean hasBinary = false;
        
        for (PayloadType type : types) {
            if (type == PayloadType.VECTOR) hasVector = true;
            if (type == PayloadType.TEXT) hasText = true;
            if (type == PayloadType.METADATA) hasMetadata = true;
            if (type == PayloadType.BINARY) hasBinary = true;
        }
        
        assertTrue(hasVector, "VECTOR type should exist");
        assertTrue(hasText, "TEXT type should exist");
        assertTrue(hasMetadata, "METADATA type should exist");
        assertTrue(hasBinary, "BINARY type should exist");
    }

    @Test
    public void testPayloadType_ValueOf() {
        assertEquals(PayloadType.VECTOR, PayloadType.valueOf("VECTOR"));
        assertEquals(PayloadType.TEXT, PayloadType.valueOf("TEXT"));
        assertEquals(PayloadType.METADATA, PayloadType.valueOf("METADATA"));
        assertEquals(PayloadType.BINARY, PayloadType.valueOf("BINARY"));
    }

    @Test
    public void testPayloadType_ValueOf_Invalid() {
        assertThrows(IllegalArgumentException.class, () -> {
            PayloadType.valueOf("INVALID");
        });
    }

    @Test
    public void testPayloadType_ToString() {
        assertEquals("VECTOR", PayloadType.VECTOR.toString());
        assertEquals("TEXT", PayloadType.TEXT.toString());
        assertEquals("METADATA", PayloadType.METADATA.toString());
        assertEquals("BINARY", PayloadType.BINARY.toString());
    }

    @Test
    public void testPayloadType_Equality() {
        PayloadType type1 = PayloadType.VECTOR;
        PayloadType type2 = PayloadType.VECTOR;
        PayloadType type3 = PayloadType.TEXT;
        
        assertEquals(type1, type2);
        assertNotEquals(type1, type3);
        assertSame(type1, type2); // Enums are singletons
    }

    @Test
    public void testPayloadType_Switch() {
        // Test that we can use in switch statements
        for (PayloadType type : PayloadType.values()) {
            String result = getPayloadDescription(type);
            assertNotNull(result);
            assertFalse(result.isEmpty());
        }
    }

    private String getPayloadDescription(PayloadType type) {
        switch (type) {
            case VECTOR:
                return "Vector/Embedding data";
            case TEXT:
                return "Text data";
            case METADATA:
                return "Metadata";
            case BINARY:
                return "Binary data";
            default:
                return "Unknown";
        }
    }

    @Test
    public void testPayloadType_Ordinal() {
        // Test ordinal values are consistent
        assertTrue(PayloadType.VECTOR.ordinal() >= 0);
        assertTrue(PayloadType.TEXT.ordinal() >= 0);
        assertTrue(PayloadType.METADATA.ordinal() >= 0);
        assertTrue(PayloadType.BINARY.ordinal() >= 0);
        
        // Verify ordinals are unique
        int vectorOrd = PayloadType.VECTOR.ordinal();
        int textOrd = PayloadType.TEXT.ordinal();
        int metadataOrd = PayloadType.METADATA.ordinal();
        int binaryOrd = PayloadType.BINARY.ordinal();
        
        assertNotEquals(vectorOrd, textOrd);
        assertNotEquals(vectorOrd, metadataOrd);
        assertNotEquals(vectorOrd, binaryOrd);
    }

    @Test
    public void testPayloadType_CompareTo() {
        // Enums are comparable
        PayloadType[] types = PayloadType.values();
        
        for (int i = 0; i < types.length - 1; i++) {
            assertTrue(types[i].compareTo(types[i + 1]) < 0);
            assertTrue(types[i + 1].compareTo(types[i]) > 0);
            assertEquals(0, types[i].compareTo(types[i]));
        }
    }
}

