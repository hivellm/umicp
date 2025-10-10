package main

import (
	"context"
	"fmt"
	"time"

	"github.com/hivellm/umicp-go/pkg/pool"
)

func main() {
	fmt.Println("=== UMICP Connection Pool Example ===")

	ctx := context.Background()

	// Create pool configuration
	fmt.Println("1. Creating connection pool...")
	config := pool.PoolConfig{
		Address:     "ws://localhost:8080",
		MinSize:     2,
		MaxSize:     5,
		MaxAge:      5 * time.Minute,
		IdleTimeout: 2 * time.Minute,
	}

	connectionPool := pool.NewConnectionPool(config)
	fmt.Printf("   ✓ Pool created: Min=%d, Max=%d\n", config.MinSize, config.MaxSize)

	// Note: In a real scenario, you would initialize the pool
	// This example demonstrates the pool structure without actual WebSocket connections

	fmt.Println("\n2. Pool Configuration:")
	fmt.Printf("   Address: %s\n", config.Address)
	fmt.Printf("   Min Size: %d connections\n", config.MinSize)
	fmt.Printf("   Max Size: %d connections\n", config.MaxSize)
	fmt.Printf("   Max Age: %v\n", config.MaxAge)
	fmt.Printf("   Idle Timeout: %v\n", config.IdleTimeout)

	// Demonstrate pooled connection
	fmt.Println("\n3. Creating pooled connections...")
	conn1 := pool.NewPooledConnection("ws://localhost:8080")
	conn2 := pool.NewPooledConnection("ws://localhost:8081")
	
	fmt.Printf("   ✓ Connection 1: %s (State: Available)\n", conn1.ID[:8])
	fmt.Printf("   ✓ Connection 2: %s (State: Available)\n", conn2.ID[:8])

	// Acquire connection
	fmt.Println("\n4. Acquiring connections...")
	conn1.Acquire()
	fmt.Printf("   ✓ Connection 1 acquired (UseCount: %d, State: InUse)\n", conn1.UseCount)

	conn2.Acquire()
	fmt.Printf("   ✓ Connection 2 acquired (UseCount: %d, State: InUse)\n", conn2.UseCount)

	// Simulate usage
	fmt.Println("\n5. Simulating connection usage...")
	time.Sleep(100 * time.Millisecond)
	fmt.Println("   [Processing...]")

	// Release connection
	fmt.Println("\n6. Releasing connections...")
	conn1.Release()
	fmt.Printf("   ✓ Connection 1 released (State: Available)\n")

	conn2.Release()
	fmt.Printf("   ✓ Connection 2 released (State: Available)\n")

	// Check connection states
	fmt.Println("\n7. Checking connection properties...")
	
	fmt.Printf("   Connection 1:\n")
	fmt.Printf("     • ID: %s\n", conn1.ID[:16])
	fmt.Printf("     • Address: %s\n", conn1.Address)
	fmt.Printf("     • Use Count: %d\n", conn1.UseCount)
	fmt.Printf("     • Created: %s ago\n", time.Since(conn1.CreatedAt).Round(time.Millisecond))
	fmt.Printf("     • Last Used: %s ago\n", time.Since(conn1.LastUsed).Round(time.Millisecond))
	fmt.Printf("     • Is Stale (5m): %v\n", conn1.IsStale(5*time.Minute))
	fmt.Printf("     • Is Idle (1m): %v\n", conn1.IsIdle(1*time.Minute))

	// Demonstrate multiple acquires
	fmt.Println("\n8. Testing multiple acquires on same connection...")
	conn1.Acquire()
	conn1.Acquire()
	conn1.Acquire()
	fmt.Printf("   ✓ Connection 1 acquired 3 more times (Total UseCount: %d)\n", conn1.UseCount)

	// Test stale detection
	fmt.Println("\n9. Testing stale connection detection...")
	oldConn := pool.NewPooledConnection("ws://localhost:8082")
	oldConn.CreatedAt = time.Now().Add(-10 * time.Minute)
	
	if oldConn.IsStale(5 * time.Minute) {
		fmt.Printf("   ✓ Connection is stale (created %s ago)\n", 
			time.Since(oldConn.CreatedAt).Round(time.Minute))
	}

	// Test idle detection
	fmt.Println("\n10. Testing idle connection detection...")
	idleConn := pool.NewPooledConnection("ws://localhost:8083")
	idleConn.LastUsed = time.Now().Add(-3 * time.Minute)
	
	if idleConn.IsIdle(2 * time.Minute) {
		fmt.Printf("   ✓ Connection is idle (last used %s ago)\n", 
			time.Since(idleConn.LastUsed).Round(time.Minute))
	}

	// Demonstrate pool stats
	fmt.Println("\n11. Pool Statistics:")
	stats := connectionPool.GetStats()
	fmt.Printf("   Total Connections: %d\n", stats.TotalConnections)
	fmt.Printf("   Available: %d\n", stats.AvailableConnections)
	fmt.Printf("   In Use: %d\n", stats.InUseConnections)
	fmt.Printf("   Total Acquires: %d\n", stats.TotalAcquires)
	fmt.Printf("   Total Releases: %d\n", stats.TotalReleases)

	// Demonstrate connection lifecycle
	fmt.Println("\n12. Connection Lifecycle Example:")
	lifecycleConn := pool.NewPooledConnection("ws://localhost:8084")
	
	fmt.Printf("   ① Created: State=%v\n", getStateName(lifecycleConn.GetState()))
	
	lifecycleConn.Acquire()
	fmt.Printf("   ② Acquired: State=%v, UseCount=%d\n", 
		getStateName(lifecycleConn.GetState()), lifecycleConn.UseCount)
	
	lifecycleConn.Release()
	fmt.Printf("   ③ Released: State=%v\n", getStateName(lifecycleConn.GetState()))
	
	lifecycleConn.Close(ctx)
	fmt.Printf("   ④ Closed: State=%v\n", getStateName(lifecycleConn.GetState()))

	// Best practices
	fmt.Println("\n13. Pool Best Practices:")
	fmt.Println("   ✓ Always release connections after use")
	fmt.Println("   ✓ Set appropriate min/max pool sizes for your workload")
	fmt.Println("   ✓ Configure idle timeout to free unused connections")
	fmt.Println("   ✓ Set max age to refresh connections periodically")
	fmt.Println("   ✓ Monitor pool statistics for optimization")
	fmt.Println("   ✓ Use context for timeouts and cancellation")

	// Usage pattern example
	fmt.Println("\n14. Typical Usage Pattern:")
	fmt.Println(`
   // Create pool
   pool := pool.NewConnectionPool(config)
   pool.Initialize(ctx)
   defer pool.Close(ctx)

   // Acquire connection
   conn, err := pool.Acquire(ctx)
   if err != nil {
       return err
   }
   defer pool.Release(conn)

   // Use connection
   // ... send/receive messages ...

   // Connection automatically released via defer
   `)

	// Performance tips
	fmt.Println("\n15. Performance Tips:")
	fmt.Println("   • Pool reduces connection overhead")
	fmt.Println("   • Reuses connections for better performance")
	fmt.Println("   • Automatic cleanup of stale connections")
	fmt.Println("   • Bounded resource usage with max size")
	fmt.Println("   • Graceful handling of connection failures")

	// Close pool
	fmt.Println("\n16. Closing connection pool...")
	err := connectionPool.Close(ctx)
	if err != nil {
		fmt.Printf("   ✗ Error closing pool: %v\n", err)
	} else {
		fmt.Println("   ✓ Pool closed successfully")
	}

	fmt.Println("\n✓ Connection Pool example completed!")
	fmt.Println("\nNote: This example demonstrates pool structure without actual")
	fmt.Println("WebSocket connections. In production, initialize the pool with")
	fmt.Println("pool.Initialize(ctx) to create real connections.")
}

func getStateName(state pool.PoolConnectionState) string {
	switch state {
	case pool.StateAvailable:
		return "Available"
	case pool.StateInUse:
		return "InUse"
	case pool.StateValidating:
		return "Validating"
	case pool.StateClosed:
		return "Closed"
	default:
		return "Unknown"
	}
}

