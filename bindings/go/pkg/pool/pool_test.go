package pool

import (
	"context"
	"testing"
	"time"
)

func TestPooledConnection(t *testing.T) {
	conn := NewPooledConnection("ws://localhost:8080")

	if conn.ID == "" {
		t.Error("Connection ID should not be empty")
	}

	if conn.Address != "ws://localhost:8080" {
		t.Errorf("Expected address 'ws://localhost:8080', got '%s'", conn.Address)
	}

	if conn.State != StateAvailable {
		t.Error("New connection should be available")
	}

	if conn.UseCount != 0 {
		t.Errorf("UseCount should be 0, got %d", conn.UseCount)
	}
}

func TestConnectionAcquireRelease(t *testing.T) {
	conn := NewPooledConnection("ws://localhost:8080")

	// Acquire
	conn.Acquire()
	if conn.State != StateInUse {
		t.Error("Connection should be in use after acquire")
	}
	if conn.UseCount != 1 {
		t.Errorf("UseCount should be 1, got %d", conn.UseCount)
	}

	// Release
	conn.Release()
	if conn.State != StateAvailable {
		t.Error("Connection should be available after release")
	}
}

func TestConnectionStale(t *testing.T) {
	conn := NewPooledConnection("ws://localhost:8080")

	// Fresh connection should not be stale
	if conn.IsStale(1 * time.Minute) {
		t.Error("Fresh connection should not be stale")
	}

	// Set old creation time
	conn.CreatedAt = time.Now().Add(-2 * time.Minute)
	if !conn.IsStale(1 * time.Minute) {
		t.Error("Old connection should be stale")
	}
}

func TestConnectionIdle(t *testing.T) {
	conn := NewPooledConnection("ws://localhost:8080")

	// Fresh connection should not be idle
	if conn.IsIdle(1 * time.Minute) {
		t.Error("Fresh connection should not be idle")
	}

	// Set old last used time
	conn.LastUsed = time.Now().Add(-2 * time.Minute)
	if !conn.IsIdle(1 * time.Minute) {
		t.Error("Unused connection should be idle")
	}
}

func TestDefaultPoolConfig(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:8080")

	if config.Address != "ws://localhost:8080" {
		t.Errorf("Expected address 'ws://localhost:8080', got '%s'", config.Address)
	}

	if config.MinSize != 2 {
		t.Errorf("Expected min size 2, got %d", config.MinSize)
	}

	if config.MaxSize != 10 {
		t.Errorf("Expected max size 10, got %d", config.MaxSize)
	}

	if config.MaxAge != 10*time.Minute {
		t.Errorf("Expected max age 10m, got %v", config.MaxAge)
	}
}

func TestNewConnectionPool(t *testing.T) {
	config := PoolConfig{
		Address:     "ws://localhost:8080",
		MinSize:     3,
		MaxSize:     15,
		MaxAge:      5 * time.Minute,
		IdleTimeout: 2 * time.Minute,
	}

	pool := NewConnectionPool(config)

	if pool.address != "ws://localhost:8080" {
		t.Errorf("Expected address 'ws://localhost:8080', got '%s'", pool.address)
	}

	if pool.minSize != 3 {
		t.Errorf("Expected min size 3, got %d", pool.minSize)
	}

	if pool.maxSize != 15 {
		t.Errorf("Expected max size 15, got %d", pool.maxSize)
	}
}

func TestPoolConfigValidation(t *testing.T) {
	// Test with invalid min/max
	config := PoolConfig{
		Address: "ws://localhost:8080",
		MinSize: 0,
		MaxSize: 0,
	}

	pool := NewConnectionPool(config)

	if pool.minSize != 2 {
		t.Errorf("Expected min size to default to 2, got %d", pool.minSize)
	}

	if pool.maxSize != 10 {
		t.Errorf("Expected max size to default to 10, got %d", pool.maxSize)
	}

	// Test with min > max
	config = PoolConfig{
		Address: "ws://localhost:8080",
		MinSize: 20,
		MaxSize: 10,
	}

	pool = NewConnectionPool(config)

	if pool.minSize != 10 {
		t.Errorf("Expected min size to be capped at max (10), got %d", pool.minSize)
	}
}

func TestPoolClosedError(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:8080")
	pool := NewConnectionPool(config)

	ctx := context.Background()

	// Close the pool
	pool.Close(ctx)

	// Try to initialize closed pool
	err := pool.Initialize(ctx)
	if err != ErrPoolClosed {
		t.Errorf("Expected ErrPoolClosed, got %v", err)
	}

	// Try to acquire from closed pool
	_, err = pool.Acquire(ctx)
	if err != ErrPoolClosed {
		t.Errorf("Expected ErrPoolClosed, got %v", err)
	}
}

func TestPoolStats(t *testing.T) {
	config := PoolConfig{
		Address: "ws://localhost:8080",
		MinSize: 2,
		MaxSize: 5,
		MaxAge:  10 * time.Minute,
	}

	pool := NewConnectionPool(config)

	stats := pool.GetStats()

	if stats.TotalConnections != 0 {
		t.Errorf("Expected 0 total connections, got %d", stats.TotalConnections)
	}

	if stats.AvailableConnections != 0 {
		t.Errorf("Expected 0 available connections, got %d", stats.AvailableConnections)
	}
}

func TestRemoveConnection(t *testing.T) {
	config := DefaultPoolConfig("ws://localhost:8080")
	pool := NewConnectionPool(config)

	// Create and add a connection manually
	conn := NewPooledConnection("ws://localhost:8080")
	pool.mu.Lock()
	pool.connections[conn.ID] = conn
	pool.stats.TotalConnections++
	pool.mu.Unlock()

	// Remove it
	err := pool.Remove(conn.ID)
	if err != nil {
		t.Errorf("Failed to remove connection: %v", err)
	}

	// Verify it's gone
	pool.mu.RLock()
	if _, exists := pool.connections[conn.ID]; exists {
		t.Error("Connection should be removed")
	}
	if pool.stats.TotalConnections != 0 {
		t.Errorf("Expected 0 connections, got %d", pool.stats.TotalConnections)
	}
	pool.mu.RUnlock()

	// Try to remove non-existent connection
	err = pool.Remove("non-existent")
	if err == nil {
		t.Error("Expected error when removing non-existent connection")
	}
}

func TestCleanupIdleConnections(t *testing.T) {
	config := PoolConfig{
		Address:     "ws://localhost:8080",
		MinSize:     1,
		MaxSize:     5,
		IdleTimeout: 100 * time.Millisecond,
	}

	pool := NewConnectionPool(config)

	// Add connections - all idle
	for i := 0; i < 4; i++ {
		conn := NewPooledConnection("ws://localhost:8080")
		conn.LastUsed = time.Now().Add(-200 * time.Millisecond)
		pool.mu.Lock()
		pool.connections[conn.ID] = conn
		pool.stats.TotalConnections++
		pool.stats.AvailableConnections++
		pool.mu.Unlock()
	}

	// Cleanup - should remove all but minSize (1)
	removed := pool.CleanupIdleConnections()

	// Should keep minSize connections
	if removed != 3 {
		t.Errorf("Expected 3 connections removed, got %d", removed)
	}

	stats := pool.GetStats()
	if stats.TotalConnections != 1 {
		t.Errorf("Expected 1 connection remaining, got %d", stats.TotalConnections)
	}
}

func TestCleanupStaleConnections(t *testing.T) {
	config := PoolConfig{
		Address: "ws://localhost:8080",
		MinSize: 1,
		MaxSize: 5,
		MaxAge:  100 * time.Millisecond,
	}

	pool := NewConnectionPool(config)

	// Add stale connection
	stale := NewPooledConnection("ws://localhost:8080")
	stale.CreatedAt = time.Now().Add(-200 * time.Millisecond)
	pool.mu.Lock()
	pool.connections[stale.ID] = stale
	pool.stats.TotalConnections++
	pool.stats.AvailableConnections++
	pool.mu.Unlock()

	// Add fresh connection
	fresh := NewPooledConnection("ws://localhost:8080")
	pool.mu.Lock()
	pool.connections[fresh.ID] = fresh
	pool.stats.TotalConnections++
	pool.stats.AvailableConnections++
	pool.mu.Unlock()

	// Cleanup
	removed := pool.CleanupStaleConnections()

	if removed != 1 {
		t.Errorf("Expected 1 stale connection removed, got %d", removed)
	}

	stats := pool.GetStats()
	if stats.TotalConnections != 1 {
		t.Errorf("Expected 1 connection remaining, got %d", stats.TotalConnections)
	}
}

func TestConnectionStateTransitions(t *testing.T) {
	conn := NewPooledConnection("ws://localhost:8080")

	// Initial state
	if conn.GetState() != StateAvailable {
		t.Error("Initial state should be Available")
	}

	// Acquire
	conn.Acquire()
	if conn.GetState() != StateInUse {
		t.Error("State should be InUse after acquire")
	}

	// Release
	conn.Release()
	if conn.GetState() != StateAvailable {
		t.Error("State should be Available after release")
	}

	// Close
	conn.Close(context.Background())
	if conn.GetState() != StateClosed {
		t.Error("State should be Closed after close")
	}
}

func TestMultipleAcquires(t *testing.T) {
	conn := NewPooledConnection("ws://localhost:8080")

	// Multiple acquires
	conn.Acquire()
	conn.Acquire()
	conn.Acquire()

	if conn.UseCount != 3 {
		t.Errorf("Expected UseCount 3, got %d", conn.UseCount)
	}
}
