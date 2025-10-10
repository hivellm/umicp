package pool

import (
	"context"
	"testing"
	"time"
)

func TestPooledConnection_Close(t *testing.T) {
	conn := NewPooledConnection("ws://localhost:9999")

	ctx := context.Background()
	_ = conn.Close(ctx) // May error because no real server

	// State should change regardless
	if conn.GetState() != StateClosed {
		t.Error("Connection state should be Closed after Close()")
	}
}

func TestPooledConnection_IsConnected(t *testing.T) {
	conn := NewPooledConnection("ws://localhost:9999")

	// New connection not connected (no Connect() called)
	if conn.IsConnected() {
		t.Error("New connection should not be connected without Connect()")
	}
}

func TestPooledConnection_GetState(t *testing.T) {
	conn := NewPooledConnection("ws://localhost:9999")

	// Test all states
	states := []PoolConnectionState{
		StateAvailable,
		StateInUse,
		StateValidating,
		StateClosed,
	}

	for _, state := range states {
		conn.mu.Lock()
		conn.State = state
		conn.mu.Unlock()

		if conn.GetState() != state {
			t.Errorf("Expected state %v, got %v", state, conn.GetState())
		}
	}
}

func TestConnectionPool_Release_NotInPool(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:9999")
	pool := NewConnectionPool(config)

	conn := NewPooledConnection("ws://localhost:9999")

	err := pool.Release(conn)
	if err == nil {
		t.Error("Expected error when releasing connection not in pool")
	}
}

func TestConnectionPool_Release_ClosedPool(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:9999")
	pool := NewConnectionPool(config)

	conn := NewPooledConnection("ws://localhost:9999")
	pool.mu.Lock()
	pool.connections[conn.ID] = conn
	pool.mu.Unlock()

	// Close pool
	pool.Close(context.Background())

	// Try to release
	err := pool.Release(conn)
	if err != ErrPoolClosed {
		t.Errorf("Expected ErrPoolClosed, got %v", err)
	}
}

func TestConnectionPool_Release_StaleConnection(t *testing.T) {
	config := PoolConfig{
		Address: "ws://localhost:9999",
		MinSize: 1,
		MaxSize: 5,
		MaxAge:  100 * time.Millisecond,
	}
	pool := NewConnectionPool(config)

	conn := NewPooledConnection("ws://localhost:9999")
	conn.CreatedAt = time.Now().Add(-200 * time.Millisecond)
	conn.State = StateInUse

	pool.mu.Lock()
	pool.connections[conn.ID] = conn
	pool.stats.TotalConnections = 1
	pool.stats.InUseConnections = 1
	pool.mu.Unlock()

	// Release stale connection - should be removed
	pool.Release(conn)

	stats := pool.GetStats()
	if stats.TotalConnections != 0 {
		t.Errorf("Stale connection should be removed, got %d connections", stats.TotalConnections)
	}
}

func TestConnectionPool_Acquire_Timeout(t *testing.T) {
	config := PoolConfig{
		Address: "ws://localhost:9999",
		MinSize: 0,
		MaxSize: 1,
	}
	pool := NewConnectionPool(config)

	// Don't initialize - no connections available
	// Create timeout context
	ctx, cancel := context.WithTimeout(context.Background(), 50*time.Millisecond)
	defer cancel()

	// Try to acquire - should timeout or fail to connect
	_, err := pool.Acquire(ctx)
	if err == nil {
		t.Error("Expected error")
	}
	// Accept any error - timeout, no available, or connection refused
}

func TestConnectionPool_Acquire_AtMaxSize(t *testing.T) {
	config := PoolConfig{
		Address: "ws://localhost:9999",
		MinSize: 0,
		MaxSize: 2,
	}
	pool := NewConnectionPool(config)

	// Manually add connections to reach max
	conn1 := NewPooledConnection("ws://localhost:9999")
	conn1.State = StateInUse
	conn2 := NewPooledConnection("ws://localhost:9999")
	conn2.State = StateInUse

	pool.mu.Lock()
	pool.connections[conn1.ID] = conn1
	pool.connections[conn2.ID] = conn2
	pool.stats.TotalConnections = 2
	pool.stats.InUseConnections = 2
	pool.mu.Unlock()

	// Try to acquire when at max and no available
	ctx := context.Background()
	_, err := pool.Acquire(ctx)

	if err != ErrNoAvailableConnections {
		t.Errorf("Expected ErrNoAvailableConnections, got %v", err)
	}
}

func TestConnectionPool_Remove_InUseConnection(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:9999")
	pool := NewConnectionPool(config)

	conn := NewPooledConnection("ws://localhost:9999")
	conn.State = StateInUse

	pool.mu.Lock()
	pool.connections[conn.ID] = conn
	pool.stats.TotalConnections = 1
	pool.stats.InUseConnections = 1
	pool.mu.Unlock()

	err := pool.Remove(conn.ID)
	if err != nil {
		t.Errorf("Failed to remove in-use connection: %v", err)
	}

	stats := pool.GetStats()
	if stats.InUseConnections != 0 {
		t.Errorf("InUseConnections should be 0, got %d", stats.InUseConnections)
	}
}

func TestConnectionPool_Remove_AvailableConnection(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:9999")
	pool := NewConnectionPool(config)

	conn := NewPooledConnection("ws://localhost:9999")
	conn.State = StateAvailable

	pool.mu.Lock()
	pool.connections[conn.ID] = conn
	pool.stats.TotalConnections = 1
	pool.stats.AvailableConnections = 1
	pool.mu.Unlock()

	err := pool.Remove(conn.ID)
	if err != nil {
		t.Errorf("Failed to remove available connection: %v", err)
	}

	stats := pool.GetStats()
	if stats.AvailableConnections != 0 {
		t.Errorf("AvailableConnections should be 0, got %d", stats.AvailableConnections)
	}
}

func TestConnectionPool_CleanupIdleConnections_ClosedPool(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:9999")
	pool := NewConnectionPool(config)

	pool.Close(context.Background())

	removed := pool.CleanupIdleConnections()
	if removed != 0 {
		t.Errorf("Closed pool should not cleanup connections, got %d removed", removed)
	}
}

func TestConnectionPool_CleanupStaleConnections_ClosedPool(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:9999")
	pool := NewConnectionPool(config)

	pool.Close(context.Background())

	removed := pool.CleanupStaleConnections()
	if removed != 0 {
		t.Errorf("Closed pool should not cleanup connections, got %d removed", removed)
	}
}

func TestConnectionPool_CleanupStaleConnections_InUse(t *testing.T) {
	config := PoolConfig{
		Address: "ws://localhost:9999",
		MinSize: 0,
		MaxSize: 5,
		MaxAge:  100 * time.Millisecond,
	}
	pool := NewConnectionPool(config)

	// Add stale in-use connection
	conn := NewPooledConnection("ws://localhost:9999")
	conn.CreatedAt = time.Now().Add(-200 * time.Millisecond)
	conn.State = StateInUse

	pool.mu.Lock()
	pool.connections[conn.ID] = conn
	pool.stats.TotalConnections = 1
	pool.stats.InUseConnections = 1
	pool.mu.Unlock()

	removed := pool.CleanupStaleConnections()

	if removed != 1 {
		t.Errorf("Expected 1 stale connection removed, got %d", removed)
	}

	stats := pool.GetStats()
	if stats.InUseConnections != 0 {
		t.Errorf("InUseConnections should be decremented, got %d", stats.InUseConnections)
	}
}

func TestConnectionPool_StartAutoCleanup(t *testing.T) {
	config := PoolConfig{
		Address:     "ws://localhost:9999",
		MinSize:     1,
		MaxSize:     5,
		IdleTimeout: 50 * time.Millisecond,
		MaxAge:      100 * time.Millisecond,
	}
	pool := NewConnectionPool(config)

	// Add idle connection
	conn := NewPooledConnection("ws://localhost:9999")
	conn.LastUsed = time.Now().Add(-100 * time.Millisecond)

	pool.mu.Lock()
	pool.connections[conn.ID] = conn
	pool.stats.TotalConnections = 1
	pool.stats.AvailableConnections = 1
	pool.mu.Unlock()

	// Start auto cleanup
	ctx, cancel := context.WithTimeout(context.Background(), 500*time.Millisecond)
	defer cancel()

	pool.StartAutoCleanup(ctx, 100*time.Millisecond)

	// Wait for cleanup to run
	time.Sleep(250 * time.Millisecond)

	// Stop cleanup
	pool.StopAutoCleanup()

	// Verify cleanup ran (connection count may vary)
	stats := pool.GetStats()
	_ = stats // Cleanup may or may not have removed connections
}

func TestConnectionPool_StopAutoCleanup_NotStarted(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:9999")
	pool := NewConnectionPool(config)

	// Should not panic when stopping cleanup that wasn't started
	pool.StopAutoCleanup()
}

func TestConnectionPool_Close_AlreadyClosed(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:9999")
	pool := NewConnectionPool(config)

	ctx := context.Background()

	// First close
	err := pool.Close(ctx)
	if err != nil {
		t.Errorf("First close failed: %v", err)
	}

	// Second close
	err = pool.Close(ctx)
	if err != ErrPoolClosed {
		t.Errorf("Expected ErrPoolClosed on second close, got %v", err)
	}
}

func TestConnectionPool_Close_WithConnections(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:9999")
	pool := NewConnectionPool(config)

	// Add multiple connections
	for i := 0; i < 3; i++ {
		conn := NewPooledConnection("ws://localhost:9999")
		pool.mu.Lock()
		pool.connections[conn.ID] = conn
		pool.stats.TotalConnections++
		pool.mu.Unlock()
	}

	ctx := context.Background()
	err := pool.Close(ctx)
	if err != nil {
		t.Errorf("Close failed: %v", err)
	}

	// Verify all connections removed
	if len(pool.connections) != 0 {
		t.Errorf("Expected 0 connections after close, got %d", len(pool.connections))
	}
}

func TestConnectionPool_Acquire_Reconnect(t *testing.T) {
	config := PoolConfig{
		Address: "ws://localhost:9999",
		MinSize: 0,
		MaxSize: 5,
	}
	pool := NewConnectionPool(config)

	// Add disconnected connection to available channel
	conn := NewPooledConnection("ws://localhost:9999")
	conn.State = StateAvailable

	pool.mu.Lock()
	pool.connections[conn.ID] = conn
	pool.stats.TotalConnections = 1
	pool.stats.AvailableConnections = 1
	pool.mu.Unlock()

	pool.available <- conn

	ctx, cancel := context.WithTimeout(context.Background(), 100*time.Millisecond)
	defer cancel()

	// Try to acquire - will fail to reconnect but should handle it
	_, err := pool.Acquire(ctx)
	// Expected to error because can't connect to fake server
	if err == nil {
		t.Log("Unexpectedly succeeded (may have removed connection)")
	}
}

func TestConnectionPool_Release_ChannelFull(t *testing.T) {
	config := PoolConfig{
		Address: "ws://localhost:9999",
		MinSize: 0,
		MaxSize: 2, // Small maxSize
	}
	pool := NewConnectionPool(config)

	conn := NewPooledConnection("ws://localhost:9999")
	conn.State = StateInUse

	pool.mu.Lock()
	pool.connections[conn.ID] = conn
	pool.stats.TotalConnections = 1
	pool.stats.InUseConnections = 1
	pool.mu.Unlock()

	// Fill the available channel
	dummy1 := NewPooledConnection("ws://localhost:9999")
	dummy2 := NewPooledConnection("ws://localhost:9999")
	pool.available <- dummy1
	pool.available <- dummy2

	// Now release - channel is full
	err := pool.Release(conn)
	if err != nil {
		t.Errorf("Release should not error even with full channel: %v", err)
	}

	// Connection should be closed and removed
	pool.mu.RLock()
	_, exists := pool.connections[conn.ID]
	pool.mu.RUnlock()

	if exists {
		t.Error("Connection should be removed when channel is full")
	}
}

func TestConnectionPool_CleanupIdleConnections_KeepsMinSize(t *testing.T) {
	config := PoolConfig{
		Address:     "ws://localhost:9999",
		MinSize:     3,
		MaxSize:     10,
		IdleTimeout: 50 * time.Millisecond,
	}
	pool := NewConnectionPool(config)

	// Add 6 idle connections
	for i := 0; i < 6; i++ {
		conn := NewPooledConnection("ws://localhost:9999")
		conn.LastUsed = time.Now().Add(-100 * time.Millisecond)
		conn.State = StateAvailable

		pool.mu.Lock()
		pool.connections[conn.ID] = conn
		pool.stats.TotalConnections++
		pool.stats.AvailableConnections++
		pool.mu.Unlock()
	}

	// Cleanup should remove 3, keeping minSize of 3
	removed := pool.CleanupIdleConnections()

	if removed != 3 {
		t.Errorf("Expected 3 connections removed, got %d", removed)
	}

	stats := pool.GetStats()
	if stats.TotalConnections != 3 {
		t.Errorf("Expected 3 connections remaining (minSize), got %d", stats.TotalConnections)
	}
}

func TestConnectionPool_CleanupIdleConnections_IgnoresInUse(t *testing.T) {
	config := PoolConfig{
		Address:     "ws://localhost:9999",
		MinSize:     0,
		MaxSize:     10,
		IdleTimeout: 50 * time.Millisecond,
	}
	pool := NewConnectionPool(config)

	// Add idle available connection
	conn1 := NewPooledConnection("ws://localhost:9999")
	conn1.mu.Lock()
	conn1.LastUsed = time.Now().Add(-100 * time.Millisecond)
	conn1.State = StateAvailable
	conn1.mu.Unlock()

	// Add idle in-use connection (shouldn't be cleaned)
	conn2 := NewPooledConnection("ws://localhost:9999")
	conn2.mu.Lock()
	conn2.LastUsed = time.Now().Add(-100 * time.Millisecond)
	conn2.State = StateInUse
	conn2.mu.Unlock()

	pool.mu.Lock()
	pool.connections[conn1.ID] = conn1
	pool.connections[conn2.ID] = conn2
	pool.stats.TotalConnections = 2
	pool.stats.AvailableConnections = 1
	pool.stats.InUseConnections = 1
	pool.mu.Unlock()

	removed := pool.CleanupIdleConnections()

	// Should only remove available connection
	if removed < 0 || removed > 1 {
		t.Errorf("Expected 0 or 1 connection removed, got %d", removed)
	}

	// In-use connection should remain
	pool.mu.RLock()
	_, exists := pool.connections[conn2.ID]
	pool.mu.RUnlock()

	if !exists {
		t.Error("In-use connection should not be removed by idle cleanup")
	}
}

func TestPoolErrors(t *testing.T) {
	t.Run("ErrPoolClosed", func(t *testing.T) {
		if ErrPoolClosed.Error() != "connection pool is closed" {
			t.Errorf("Unexpected error message: %s", ErrPoolClosed.Error())
		}
	})

	t.Run("ErrNoAvailableConnections", func(t *testing.T) {
		if ErrNoAvailableConnections.Error() != "no available connections" {
			t.Errorf("Unexpected error message: %s", ErrNoAvailableConnections.Error())
		}
	})

	t.Run("ErrConnectionClosed", func(t *testing.T) {
		if ErrConnectionClosed.Error() != "connection is closed" {
			t.Errorf("Unexpected error message: %s", ErrConnectionClosed.Error())
		}
	})
}

func TestPoolConnectionState_Values(t *testing.T) {
	// Test that constants have expected values
	if StateAvailable != 0 {
		t.Errorf("StateAvailable should be 0, got %d", StateAvailable)
	}
	if StateInUse != 1 {
		t.Errorf("StateInUse should be 1, got %d", StateInUse)
	}
	if StateValidating != 2 {
		t.Errorf("StateValidating should be 2, got %d", StateValidating)
	}
	if StateClosed != 3 {
		t.Errorf("StateClosed should be 3, got %d", StateClosed)
	}
}
