/*!
# Connection Pool Example

Demonstrates connection pooling for efficient resource management.
*/

#![cfg(feature = "websocket")]

use umicp_core::{ConnectionPool, PoolConfig};
use std::time::Duration;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("=== UMICP Connection Pool Example ===\n");

    // Create custom pool configuration
    let config = PoolConfig {
        min_size: 2,
        max_size: 5,
        idle_timeout: Duration::from_secs(300),
        max_age: Duration::from_secs(3600),
        validation_interval: Duration::from_secs(60),
    };

    println!("Pool Configuration:");
    println!("  Min Size: {}", config.min_size);
    println!("  Max Size: {}", config.max_size);
    println!("  Idle Timeout: {:?}", config.idle_timeout);
    println!("  Max Age: {:?}", config.max_age);

    // Create connection pool
    let pool = ConnectionPool::with_config(config);
    println!("\n✓ Connection pool created\n");

    // Simulate acquiring connections
    let address = "ws://localhost:8080";

    println!("--- Acquiring Connections ---");

    // Try to acquire connections (will fail without actual server, but demonstrates API)
    match pool.acquire(address).await {
        Ok(client) => {
            println!("✓ Acquired connection 1");
            println!("  Connected: {}", client.is_connected());

            // Use the connection...
            // ... do work ...

            // Release back to pool
            pool.release(address, &client);
            println!("✓ Released connection 1 back to pool");
        }
        Err(e) => {
            println!("✗ Failed to acquire connection: {}", e);
            println!("  (This is expected without a running server)");
        }
    }

    // Pool statistics
    println!("\n--- Pool Statistics ---");
    let stats = pool.get_stats();
    println!("Total Connections: {}", stats.total_connections);
    println!("Available: {}", stats.available_connections);
    println!("In Use: {}", stats.in_use_connections);
    println!("Total Requests: {}", stats.total_requests);
    println!("Total Acquisitions: {}", stats.total_acquisitions);
    println!("Total Releases: {}", stats.total_releases);
    println!("Total Creations: {}", stats.total_creations);
    println!("Total Closures: {}", stats.total_closures);

    // Pool size for specific address
    println!("\n--- Address Pool Info ---");
    println!("Pool size for {}: {}", address, pool.get_pool_size(address));
    println!("Available connections: {}", pool.get_available_count(address));

    // Cleanup demonstration
    println!("\n--- Cleanup ---");
    let removed = pool.cleanup().await;
    println!("Removed {} stale/idle connections", removed);

    // Warmup demonstration (would work with real server)
    println!("\n--- Warmup (requires server) ---");
    match pool.warmup(address).await {
        Ok(_) => println!("✓ Pool warmed up with minimum connections"),
        Err(e) => println!("✗ Warmup failed: {}", e),
    }

    // Close all connections
    println!("\n--- Shutdown ---");
    let closed = pool.close_all().await;
    println!("✓ Closed {} connections", closed);

    println!("\n=== Example Complete ===");

    Ok(())
}

#[cfg(not(feature = "websocket"))]
fn main() {
    eprintln!("This example requires the 'websocket' feature.");
    eprintln!("Run with: cargo run --example connection_pool_example --features websocket");
}
