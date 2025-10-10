package com.hivellm.umicp.transport;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for ServerOptions.
 */
class ServerOptionsTest {

    @Test
    void testBuilder_withDefaults() {
        ServerOptions options = ServerOptions.builder().build();

        assertEquals("/umicp", options.getPath());
        assertEquals(1000, options.getMaxClients());
        assertFalse(options.isCompression());
    }

    @Test
    void testDefaults_returnsDefaultOptions() {
        ServerOptions options = ServerOptions.defaults();

        assertEquals("/umicp", options.getPath());
        assertEquals(1000, options.getMaxClients());
        assertFalse(options.isCompression());
    }

    @Test
    void testBuilder_withCustomPath() {
        ServerOptions options = ServerOptions.builder()
            .path("/custom")
            .build();

        assertEquals("/custom", options.getPath());
    }

    @Test
    void testBuilder_withCustomMaxClients() {
        ServerOptions options = ServerOptions.builder()
            .maxClients(500)
            .build();

        assertEquals(500, options.getMaxClients());
    }

    @Test
    void testBuilder_withCompressionEnabled() {
        ServerOptions options = ServerOptions.builder()
            .compression(true)
            .build();

        assertTrue(options.isCompression());
    }

    @Test
    void testBuilder_withAllOptions() {
        ServerOptions options = ServerOptions.builder()
            .path("/api/ws")
            .maxClients(100)
            .compression(true)
            .build();

        assertEquals("/api/ws", options.getPath());
        assertEquals(100, options.getMaxClients());
        assertTrue(options.isCompression());
    }

    @Test
    void testBuilder_fluent() {
        ServerOptions.Builder builder = ServerOptions.builder();

        // Builder methods should return the builder
        assertSame(builder.getClass(), builder.path("/test").getClass());
        assertSame(builder.getClass(), builder.maxClients(50).getClass());
        assertSame(builder.getClass(), builder.compression(true).getClass());
    }

    @Test
    void testEquals_sameOptions() {
        ServerOptions options1 = ServerOptions.builder()
            .path("/test")
            .maxClients(100)
            .compression(true)
            .build();

        ServerOptions options2 = ServerOptions.builder()
            .path("/test")
            .maxClients(100)
            .compression(true)
            .build();

        assertEquals(options1, options2);
    }

    @Test
    void testEquals_differentPath() {
        ServerOptions options1 = ServerOptions.builder()
            .path("/test1")
            .build();

        ServerOptions options2 = ServerOptions.builder()
            .path("/test2")
            .build();

        assertNotEquals(options1, options2);
    }

    @Test
    void testEquals_differentMaxClients() {
        ServerOptions options1 = ServerOptions.builder()
            .maxClients(100)
            .build();

        ServerOptions options2 = ServerOptions.builder()
            .maxClients(200)
            .build();

        assertNotEquals(options1, options2);
    }

    @Test
    void testEquals_differentCompression() {
        ServerOptions options1 = ServerOptions.builder()
            .compression(true)
            .build();

        ServerOptions options2 = ServerOptions.builder()
            .compression(false)
            .build();

        assertNotEquals(options1, options2);
    }

    @Test
    void testEquals_withNull() {
        ServerOptions options = ServerOptions.builder().build();
        assertNotEquals(null, options);
    }

    @Test
    void testEquals_withSelf() {
        ServerOptions options = ServerOptions.builder().build();
        assertEquals(options, options);
    }

    @Test
    void testHashCode_sameOptions() {
        ServerOptions options1 = ServerOptions.builder()
            .path("/test")
            .maxClients(100)
            .compression(true)
            .build();

        ServerOptions options2 = ServerOptions.builder()
            .path("/test")
            .maxClients(100)
            .compression(true)
            .build();

        assertEquals(options1.hashCode(), options2.hashCode());
    }

    @Test
    void testPath_canBeEmpty() {
        ServerOptions options = ServerOptions.builder()
            .path("")
            .build();

        assertEquals("", options.getPath());
    }

    @Test
    void testMaxClients_canBeZero() {
        ServerOptions options = ServerOptions.builder()
            .maxClients(0)
            .build();

        assertEquals(0, options.getMaxClients());
    }

    @Test
    void testMaxClients_canBeNegative() {
        // Test that negative values are accepted (unlimited)
        ServerOptions options = ServerOptions.builder()
            .maxClients(-1)
            .build();

        assertEquals(-1, options.getMaxClients());
    }
}

