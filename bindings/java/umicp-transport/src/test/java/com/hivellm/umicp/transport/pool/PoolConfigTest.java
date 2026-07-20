package com.hivellm.umicp.transport.pool;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for PoolConfig
 */
class PoolConfigTest {

    @Test
    void testDefaultConstructor() {
        PoolConfig config = new PoolConfig();

        assertEquals(2, config.getMinSize());
        assertEquals(10, config.getMaxSize());
        assertEquals(600, config.getMaxAgeSeconds());
        assertEquals(300, config.getIdleTimeoutSeconds());
        assertEquals(5000, config.getAcquireTimeoutMs());
    }

    @Test
    void testConstructorWithAddress() {
        PoolConfig config = new PoolConfig("ws://test:9000");

        assertEquals("ws://test:9000", config.getAddress());
        assertEquals(2, config.getMinSize());
        assertEquals(10, config.getMaxSize());
    }

    @Test
    void testSetAddress() {
        PoolConfig config = new PoolConfig();
        config.setAddress("ws://localhost:8080");

        assertEquals("ws://localhost:8080", config.getAddress());
    }

    @Test
    void testSetNullAddress() {
        PoolConfig config = new PoolConfig();

        assertThrows(IllegalArgumentException.class, () -> {
            config.setAddress(null);
        });
    }

    @Test
    void testSetEmptyAddress() {
        PoolConfig config = new PoolConfig();

        assertThrows(IllegalArgumentException.class, () -> {
            config.setAddress("");
        });
    }

    @Test
    void testSetMinSize() {
        PoolConfig config = new PoolConfig();
        config.setMinSize(5);

        assertEquals(5, config.getMinSize());
    }

    @Test
    void testSetNegativeMinSize() {
        PoolConfig config = new PoolConfig();

        assertThrows(IllegalArgumentException.class, () -> {
            config.setMinSize(-1);
        });
    }

    @Test
    void testSetMaxSize() {
        PoolConfig config = new PoolConfig();
        config.setMaxSize(20);

        assertEquals(20, config.getMaxSize());
    }

    @Test
    void testSetInvalidMaxSize() {
        PoolConfig config = new PoolConfig();

        assertThrows(IllegalArgumentException.class, () -> {
            config.setMaxSize(0);
        });

        assertThrows(IllegalArgumentException.class, () -> {
            config.setMaxSize(-1);
        });
    }

    @Test
    void testSetMaxAgeSeconds() {
        PoolConfig config = new PoolConfig();
        config.setMaxAgeSeconds(300);

        assertEquals(300, config.getMaxAgeSeconds());
    }

    @Test
    void testSetInvalidMaxAgeSeconds() {
        PoolConfig config = new PoolConfig();

        assertThrows(IllegalArgumentException.class, () -> {
            config.setMaxAgeSeconds(0);
        });
    }

    @Test
    void testSetIdleTimeoutSeconds() {
        PoolConfig config = new PoolConfig();
        config.setIdleTimeoutSeconds(120);

        assertEquals(120, config.getIdleTimeoutSeconds());
    }

    @Test
    void testSetAcquireTimeoutMs() {
        PoolConfig config = new PoolConfig();
        config.setAcquireTimeoutMs(3000);

        assertEquals(3000, config.getAcquireTimeoutMs());
    }

    @Test
    void testValidate() {
        PoolConfig config = new PoolConfig();
        config.setMinSize(20);
        config.setMaxSize(10);

        config.validate();

        // Min should be adjusted to max
        assertEquals(10, config.getMinSize());
        assertEquals(10, config.getMaxSize());
    }

    @Test
    void testFluentAPI() {
        PoolConfig config = new PoolConfig()
                .setAddress("ws://test:9000")
                .setMinSize(3)
                .setMaxSize(15)
                .setMaxAgeSeconds(400)
                .setIdleTimeoutSeconds(200)
                .setAcquireTimeoutMs(2000);

        assertEquals("ws://test:9000", config.getAddress());
        assertEquals(3, config.getMinSize());
        assertEquals(15, config.getMaxSize());
        assertEquals(400, config.getMaxAgeSeconds());
        assertEquals(200, config.getIdleTimeoutSeconds());
        assertEquals(2000, config.getAcquireTimeoutMs());
    }

    @Test
    void testToString() {
        PoolConfig config = new PoolConfig("ws://localhost:8080");
        String str = config.toString();

        assertTrue(str.contains("ws://localhost:8080"));
        assertTrue(str.contains("minSize"));
        assertTrue(str.contains("maxSize"));
    }
}

