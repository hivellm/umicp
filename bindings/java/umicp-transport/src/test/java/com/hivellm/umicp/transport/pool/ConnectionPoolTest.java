package com.hivellm.umicp.transport.pool;

import com.hivellm.umicp.types.ConnectionException;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.AfterEach;
import static org.junit.jupiter.api.Assertions.*;

/**
 * Unit tests for ConnectionPool
 */
class ConnectionPoolTest {
    private PoolConfig config;
    private ConnectionPool pool;

    @BeforeEach
    void setUp() {
        config = new PoolConfig("ws://localhost:8080");
        config.setMinSize(2);
        config.setMaxSize(5);
        config.setMaxAgeSeconds(10);
        config.setIdleTimeoutSeconds(5);
        config.setAcquireTimeoutMs(1000);

        pool = new ConnectionPool(config);
    }

    @AfterEach
    void tearDown() {
        if (pool != null && !pool.isClosed()) {
            pool.close();
        }
    }

    @Test
    void testCreation() {
        assertNotNull(pool);
        assertEquals("ws://localhost:8080", pool.getConfig().getAddress());
        assertEquals(2, pool.getConfig().getMinSize());
        assertEquals(5, pool.getConfig().getMaxSize());
        assertFalse(pool.isClosed());
    }

    @Test
    void testCreationWithNullConfig() {
        assertThrows(IllegalArgumentException.class, () -> {
            new ConnectionPool(null);
        });
    }

    @Test
    void testConfigValidation() {
        PoolConfig badConfig = new PoolConfig("ws://localhost:8080");
        badConfig.setMinSize(10);
        badConfig.setMaxSize(5);

        ConnectionPool testPool = new ConnectionPool(badConfig);

        // Min should be clamped to max
        assertTrue(testPool.getConfig().getMinSize() <= testPool.getConfig().getMaxSize());

        testPool.close();
    }

    @Test
    void testInitialStats() {
        PoolStats stats = pool.getStats();

        assertEquals(0, stats.getTotalConnections());
        assertEquals(0, stats.getAvailableConnections());
        assertEquals(0, stats.getInUseConnections());
        assertEquals(0, stats.getTotalAcquires());
        assertEquals(0, stats.getTotalReleases());
        assertEquals(0, stats.getTotalCreates());
        assertEquals(0, stats.getTotalCloses());
        assertEquals(0, stats.getFailedAcquires());
    }

    @Test
    void testShutdown() {
        assertFalse(pool.isClosed());

        pool.shutdown();

        assertTrue(pool.isClosed());

        PoolStats stats = pool.getStats();
        assertEquals(0, stats.getTotalConnections());
        assertEquals(0, stats.getAvailableConnections());
        assertEquals(0, stats.getInUseConnections());
    }

    @Test
    void testDoubleShutdown() {
        pool.shutdown();
        assertTrue(pool.isClosed());

        // Second shutdown should be safe
        pool.shutdown();
        assertTrue(pool.isClosed());
    }

    @Test
    void testCleanup() {
        int removed = pool.cleanup();
        // On empty pool, nothing should be removed
        assertEquals(0, removed);
    }

    @Test
    void testGetConfig() {
        PoolConfig retrievedConfig = pool.getConfig();
        assertNotNull(retrievedConfig);
        assertEquals(config.getAddress(), retrievedConfig.getAddress());
        assertEquals(config.getMinSize(), retrievedConfig.getMinSize());
        assertEquals(config.getMaxSize(), retrievedConfig.getMaxSize());
    }

    @Test
    void testStartStopCleanupTask() {
        pool.startCleanupTask(60);

        // Should be safe to call multiple times
        pool.startCleanupTask(60);

        pool.stopCleanupTask();

        // Should be safe to call multiple times
        pool.stopCleanupTask();
    }

    @Test
    void testCloseViaAutoCloseable() throws Exception {
        ConnectionPool testPool = new ConnectionPool(config);
        assertFalse(testPool.isClosed());

        testPool.close();
        assertTrue(testPool.isClosed());
    }

    @Test
    void testMultiplePoolInstances() {
        PoolConfig config1 = new PoolConfig("ws://localhost:8080");
        PoolConfig config2 = new PoolConfig("ws://localhost:8081");

        ConnectionPool pool1 = new ConnectionPool(config1);
        ConnectionPool pool2 = new ConnectionPool(config2);

        assertEquals("ws://localhost:8080", pool1.getConfig().getAddress());
        assertEquals("ws://localhost:8081", pool2.getConfig().getAddress());

        pool1.close();
        pool2.close();
    }

    // Note: Full integration tests would require a real WebSocket server
    // These tests validate the API structure and basic logic
}

