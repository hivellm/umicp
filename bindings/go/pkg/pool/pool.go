package pool

import (
	"context"
	"errors"
	"sync"
	"time"

	"github.com/google/uuid"
	"github.com/hivellm/umicp-go/pkg/transport/websocket"
)

var (
	// ErrPoolClosed is returned when trying to use a closed pool
	ErrPoolClosed = errors.New("connection pool is closed")
	// ErrNoAvailableConnections is returned when no connections are available
	ErrNoAvailableConnections = errors.New("no available connections")
	// ErrConnectionClosed is returned when a connection is closed
	ErrConnectionClosed = errors.New("connection is closed")
)

// PoolConnectionState represents the state of a pooled connection
type PoolConnectionState int

const (
	// StateAvailable means connection is available for use
	StateAvailable PoolConnectionState = iota
	// StateInUse means connection is currently in use
	StateInUse
	// StateValidating means connection is being validated
	StateValidating
	// StateClosed means connection is closed
	StateClosed
)

// PooledConnection represents a connection in the pool
type PooledConnection struct {
	ID        string
	Address   string
	Client    *websocket.Client
	State     PoolConnectionState
	LastUsed  time.Time
	CreatedAt time.Time
	UseCount  int
	mu        sync.RWMutex
}

// NewPooledConnection creates a new pooled connection
func NewPooledConnection(address string) *PooledConnection {
	config := websocket.ClientConfig{
		URL:                  address,
		HeartbeatInterval:    30 * time.Second,
		ReconnectDelay:       5 * time.Second,
		MaxReconnectAttempts: 5,
	}

	return &PooledConnection{
		ID:        uuid.New().String(),
		Address:   address,
		Client:    websocket.NewClient(config),
		State:     StateAvailable,
		LastUsed:  time.Now(),
		CreatedAt: time.Now(),
		UseCount:  0,
	}
}

// Acquire marks the connection as in use
func (pc *PooledConnection) Acquire() {
	pc.mu.Lock()
	defer pc.mu.Unlock()

	pc.State = StateInUse
	pc.LastUsed = time.Now()
	pc.UseCount++
}

// Release marks the connection as available
func (pc *PooledConnection) Release() {
	pc.mu.Lock()
	defer pc.mu.Unlock()

	pc.State = StateAvailable
	pc.LastUsed = time.Now()
}

// Close closes the connection
func (pc *PooledConnection) Close(ctx context.Context) error {
	pc.mu.Lock()
	defer pc.mu.Unlock()

	pc.State = StateClosed
	return pc.Client.Disconnect(ctx)
}

// IsStale checks if connection is older than max age
func (pc *PooledConnection) IsStale(maxAge time.Duration) bool {
	pc.mu.RLock()
	defer pc.mu.RUnlock()

	return time.Since(pc.CreatedAt) > maxAge
}

// IsIdle checks if connection has been idle for too long
func (pc *PooledConnection) IsIdle(idleTimeout time.Duration) bool {
	pc.mu.RLock()
	defer pc.mu.RUnlock()

	return time.Since(pc.LastUsed) > idleTimeout
}

// IsConnected checks if the underlying client is connected
func (pc *PooledConnection) IsConnected() bool {
	pc.mu.RLock()
	defer pc.mu.RUnlock()

	return pc.Client.IsConnected()
}

// GetState returns the current state
func (pc *PooledConnection) GetState() PoolConnectionState {
	pc.mu.RLock()
	defer pc.mu.RUnlock()

	return pc.State
}

// ConnectionPool manages a pool of reusable connections
type ConnectionPool struct {
	mu            sync.RWMutex
	connections   map[string]*PooledConnection
	available     chan *PooledConnection
	address       string
	minSize       int
	maxSize       int
	maxAge        time.Duration
	idleTimeout   time.Duration
	closed        bool
	stats         PoolStats
	cleanupTicker *time.Ticker
	stopCleanup   chan struct{}
}

// PoolConfig contains connection pool configuration
type PoolConfig struct {
	Address     string
	MinSize     int
	MaxSize     int
	MaxAge      time.Duration
	IdleTimeout time.Duration
}

// DefaultPoolConfig returns default pool configuration
func DefaultPoolConfig(address string) PoolConfig {
	return PoolConfig{
		Address:     address,
		MinSize:     2,
		MaxSize:     10,
		MaxAge:      10 * time.Minute,
		IdleTimeout: 5 * time.Minute,
	}
}

// NewConnectionPool creates a new connection pool
func NewConnectionPool(config PoolConfig) *ConnectionPool {
	if config.MinSize <= 0 {
		config.MinSize = 2
	}
	if config.MaxSize <= 0 {
		config.MaxSize = 10
	}
	if config.MinSize > config.MaxSize {
		config.MinSize = config.MaxSize
	}

	pool := &ConnectionPool{
		connections: make(map[string]*PooledConnection),
		available:   make(chan *PooledConnection, config.MaxSize),
		address:     config.Address,
		minSize:     config.MinSize,
		maxSize:     config.MaxSize,
		maxAge:      config.MaxAge,
		idleTimeout: config.IdleTimeout,
		stopCleanup: make(chan struct{}),
	}

	return pool
}

// Initialize initializes the pool with minimum connections
func (cp *ConnectionPool) Initialize(ctx context.Context) error {
	cp.mu.Lock()
	defer cp.mu.Unlock()

	if cp.closed {
		return ErrPoolClosed
	}

	for i := 0; i < cp.minSize; i++ {
		conn := NewPooledConnection(cp.address)
		
		if err := conn.Client.Connect(ctx); err != nil {
			return err
		}

		cp.connections[conn.ID] = conn
		cp.available <- conn
		cp.stats.TotalConnections++
		cp.stats.AvailableConnections++
	}

	return nil
}

// Acquire gets a connection from the pool
func (cp *ConnectionPool) Acquire(ctx context.Context) (*PooledConnection, error) {
	cp.mu.RLock()
	if cp.closed {
		cp.mu.RUnlock()
		return nil, ErrPoolClosed
	}
	cp.mu.RUnlock()

	select {
	case conn := <-cp.available:
		// Got available connection
		cp.mu.Lock()
		conn.Acquire()
		cp.stats.AvailableConnections--
		cp.stats.InUseConnections++
		cp.stats.TotalAcquires++
		cp.mu.Unlock()

		// Validate connection
		if !conn.IsConnected() {
			// Try to reconnect
			if err := conn.Client.Connect(ctx); err != nil {
				cp.Remove(conn.ID)
				return nil, err
			}
		}

		return conn, nil

	case <-ctx.Done():
		return nil, ctx.Err()

	default:
		// Try to create new connection if under max size
		cp.mu.Lock()
		if len(cp.connections) < cp.maxSize {
			conn := NewPooledConnection(cp.address)
			
			if err := conn.Client.Connect(ctx); err != nil {
				cp.mu.Unlock()
				return nil, err
			}

			conn.Acquire()
			cp.connections[conn.ID] = conn
			cp.stats.TotalConnections++
			cp.stats.InUseConnections++
			cp.stats.TotalAcquires++
			cp.mu.Unlock()

			return conn, nil
		}
		cp.mu.Unlock()

		return nil, ErrNoAvailableConnections
	}
}

// Release returns a connection to the pool
func (cp *ConnectionPool) Release(conn *PooledConnection) error {
	cp.mu.Lock()
	defer cp.mu.Unlock()

	if cp.closed {
		return ErrPoolClosed
	}

	if _, exists := cp.connections[conn.ID]; !exists {
		return errors.New("connection not in pool")
	}

	// Check if connection should be discarded
	if conn.IsStale(cp.maxAge) || !conn.IsConnected() {
		delete(cp.connections, conn.ID)
		cp.stats.TotalConnections--
		cp.stats.InUseConnections--
		conn.Close(context.Background())
		return nil
	}

	conn.Release()
	cp.stats.InUseConnections--
	cp.stats.AvailableConnections++
	cp.stats.TotalReleases++

	select {
	case cp.available <- conn:
		return nil
	default:
		// Channel full, close connection
		delete(cp.connections, conn.ID)
		cp.stats.TotalConnections--
		cp.stats.AvailableConnections--
		conn.Close(context.Background())
		return nil
	}
}

// Remove removes a connection from the pool
func (cp *ConnectionPool) Remove(connID string) error {
	cp.mu.Lock()
	defer cp.mu.Unlock()

	conn, exists := cp.connections[connID]
	if !exists {
		return errors.New("connection not found")
	}

	delete(cp.connections, connID)
	
	if conn.GetState() == StateInUse {
		cp.stats.InUseConnections--
	} else {
		cp.stats.AvailableConnections--
	}
	cp.stats.TotalConnections--

	conn.Close(context.Background())
	return nil
}

// CleanupIdleConnections removes idle connections
func (cp *ConnectionPool) CleanupIdleConnections() int {
	cp.mu.Lock()
	defer cp.mu.Unlock()

	if cp.closed {
		return 0
	}

	// Collect idle connections to remove
	toRemove := make([]string, 0)
	for id, conn := range cp.connections {
		if conn.GetState() == StateAvailable && conn.IsIdle(cp.idleTimeout) {
			toRemove = append(toRemove, id)
		}
	}

	// Remove connections while keeping minimum
	removed := 0
	for _, id := range toRemove {
		// Keep minimum connections
		if len(cp.connections) <= cp.minSize {
			break
		}

		conn := cp.connections[id]
		delete(cp.connections, id)
		cp.stats.TotalConnections--
		cp.stats.AvailableConnections--
		conn.Close(context.Background())
		removed++
	}

	return removed
}

// CleanupStaleConnections removes stale connections
func (cp *ConnectionPool) CleanupStaleConnections() int {
	cp.mu.Lock()
	defer cp.mu.Unlock()

	if cp.closed {
		return 0
	}

	removed := 0
	for id, conn := range cp.connections {
		if conn.IsStale(cp.maxAge) {
			delete(cp.connections, id)
			
			if conn.GetState() == StateInUse {
				cp.stats.InUseConnections--
			} else {
				cp.stats.AvailableConnections--
			}
			cp.stats.TotalConnections--
			
			conn.Close(context.Background())
			removed++
		}
	}

	return removed
}

// StartAutoCleanup starts automatic cleanup of idle/stale connections
func (cp *ConnectionPool) StartAutoCleanup(ctx context.Context, interval time.Duration) {
	cp.cleanupTicker = time.NewTicker(interval)

	go func() {
		for {
			select {
			case <-cp.cleanupTicker.C:
				cp.CleanupIdleConnections()
				cp.CleanupStaleConnections()
			case <-cp.stopCleanup:
				return
			case <-ctx.Done():
				return
			}
		}
	}()
}

// StopAutoCleanup stops automatic cleanup
func (cp *ConnectionPool) StopAutoCleanup() {
	if cp.cleanupTicker != nil {
		cp.cleanupTicker.Stop()
		close(cp.stopCleanup)
	}
}

// GetStats returns pool statistics
func (cp *ConnectionPool) GetStats() PoolStats {
	cp.mu.RLock()
	defer cp.mu.RUnlock()

	return cp.stats
}

// Close closes all connections in the pool
func (cp *ConnectionPool) Close(ctx context.Context) error {
	cp.mu.Lock()
	defer cp.mu.Unlock()

	if cp.closed {
		return ErrPoolClosed
	}

	cp.closed = true
	cp.StopAutoCleanup()

	// Close all connections
	for _, conn := range cp.connections {
		conn.Close(ctx)
	}

	cp.connections = make(map[string]*PooledConnection)
	close(cp.available)

	return nil
}

// PoolStats contains pool statistics
type PoolStats struct {
	TotalConnections     int `json:"total_connections"`
	AvailableConnections int `json:"available_connections"`
	InUseConnections     int `json:"in_use_connections"`
	TotalAcquires        int `json:"total_acquires"`
	TotalReleases        int `json:"total_releases"`
}

