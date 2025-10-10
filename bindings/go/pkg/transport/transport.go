package transport

import (
	"context"
	"time"

	"github.com/hivellm/umicp-go/pkg/umicp"
)

// Transport defines the interface for all transport implementations
type Transport interface {
	// Connect establishes the connection
	Connect(ctx context.Context) error

	// Disconnect closes the connection
	Disconnect(ctx context.Context) error

	// Send sends an envelope
	Send(ctx context.Context, env *umicp.Envelope) error

	// IsConnected returns true if connected
	IsConnected() bool

	// Stats returns transport statistics
	Stats() *Stats
}

// Stats contains transport statistics
type Stats struct {
	MessagesSent     int64
	MessagesReceived int64
	BytesSent        int64
	BytesReceived    int64
	Errors           int64
	ConnectedAt      time.Time
	LastMessageAt    time.Time
	Uptime           time.Duration
}

// Config contains common transport configuration
type Config struct {
	// ReadTimeout for read operations
	ReadTimeout time.Duration

	// WriteTimeout for write operations
	WriteTimeout time.Duration

	// MaxMessageSize in bytes
	MaxMessageSize int64

	// BufferSize for send/receive buffers
	BufferSize int
}

// DefaultConfig returns default transport configuration
func DefaultConfig() *Config {
	return &Config{
		ReadTimeout:    30 * time.Second,
		WriteTimeout:   10 * time.Second,
		MaxMessageSize: 10 * 1024 * 1024, // 10MB
		BufferSize:     100,
	}
}
