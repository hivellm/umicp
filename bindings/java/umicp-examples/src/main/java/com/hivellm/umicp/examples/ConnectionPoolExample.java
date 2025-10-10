package com.hivellm.umicp.examples;

import com.hivellm.umicp.transport.pool.*;

/**
 * Example demonstrating UMICP Connection Pool
 */
public class ConnectionPoolExample {
    public static void main(String[] args) throws Exception {
        System.out.println("=== UMICP Connection Pool Example ===\n");

        // Create pool configuration
        System.out.println("1. Creating connection pool configuration...");
        PoolConfig config = new PoolConfig()
                .setAddress("ws://localhost:8080")
                .setMinSize(2)
                .setMaxSize(5)
                .setMaxAgeSeconds(300)        // 5 minutes
                .setIdleTimeoutSeconds(120)   // 2 minutes
                .setAcquireTimeoutMs(5000);   // 5 seconds

        System.out.println("   Configuration:");
        System.out.println("   - Address: " + config.getAddress());
        System.out.println("   - Min size: " + config.getMinSize());
        System.out.println("   - Max size: " + config.getMaxSize());
        System.out.println("   - Max age: " + config.getMaxAgeSeconds() + " seconds");
        System.out.println("   - Idle timeout: " + config.getIdleTimeoutSeconds() + " seconds");
        System.out.println("   - Acquire timeout: " + config.getAcquireTimeoutMs() + " ms");
        System.out.println();

        // Create connection pool
        System.out.println("2. Creating connection pool...");
        try (ConnectionPool pool = new ConnectionPool(config)) {
            System.out.println("   ✓ Pool created successfully");
            System.out.println("   Is closed: " + pool.isClosed());
            System.out.println();

            // Show initial statistics
            System.out.println("3. Initial pool statistics:");
            PoolStats stats = pool.getStats();
            printStats(stats);

            // Pool initialization
            System.out.println("4. Pool initialization:");
            System.out.println("   NOTE: Actual initialization requires a running WebSocket server");
            System.out.println("   This example demonstrates the API structure.");
            System.out.println();

            // Demonstrate configuration validation
            System.out.println("5. Testing configuration validation:");
            PoolConfig badConfig = new PoolConfig("ws://test:9000");
            badConfig.setMinSize(20);
            badConfig.setMaxSize(10);

            System.out.println("   Original: min=" + badConfig.getMinSize() + ", max=" + badConfig.getMaxSize());

            badConfig.validate();

            System.out.println("   Adjusted: min=" + badConfig.getMinSize() + ", max=" + badConfig.getMaxSize());
            System.out.println("   ✓ Configuration automatically validated");
            System.out.println();

            // Demonstrate cleanup
            System.out.println("6. Testing pool cleanup:");
            int removed = pool.cleanup();
            System.out.println("   ✓ Cleaned up " + removed + " connection(s)");
            System.out.println();

            // Demonstrate cleanup task
            System.out.println("7. Background cleanup task:");
            pool.startCleanupTask(60);
            System.out.println("   ✓ Cleanup task started (60 second interval)");

            pool.stopCleanupTask();
            System.out.println("   ✓ Cleanup task stopped");
            System.out.println();

            // Multiple pool instances
            System.out.println("8. Testing multiple pool instances:");
            PoolConfig config1 = new PoolConfig("ws://localhost:8080");
            PoolConfig config2 = new PoolConfig("ws://localhost:8081");

            try (ConnectionPool pool1 = new ConnectionPool(config1);
                 ConnectionPool pool2 = new ConnectionPool(config2)) {

                System.out.println("   Pool 1: " + pool1.getConfig().getAddress());
                System.out.println("   Pool 2: " + pool2.getConfig().getAddress());
                System.out.println("   ✓ Multiple pools can coexist");
            }
            System.out.println();

            // Shutdown
            System.out.println("9. Shutting down pool...");
            pool.shutdown();
            System.out.println("   ✓ Pool shut down");
            System.out.println("   Is closed: " + pool.isClosed());
            System.out.println();

            // Final statistics
            System.out.println("10. Final pool statistics:");
            stats = pool.getStats();
            printStats(stats);

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }

        System.out.println("=== Connection Pool Example Complete ===");
        System.out.println("\nNOTE: This example demonstrates the API structure.");
        System.out.println("For full functionality, connect to a running WebSocket server.");
    }

    private static void printStats(PoolStats stats) {
        System.out.println("   - Total connections: " + stats.getTotalConnections());
        System.out.println("   - Available: " + stats.getAvailableConnections());
        System.out.println("   - In use: " + stats.getInUseConnections());
        System.out.println("   - Total acquires: " + stats.getTotalAcquires());
        System.out.println("   - Total releases: " + stats.getTotalReleases());
        System.out.println("   - Total creates: " + stats.getTotalCreates());
        System.out.println("   - Total closes: " + stats.getTotalCloses());
        System.out.println("   - Failed acquires: " + stats.getFailedAcquires());
        System.out.println();
    }
}

