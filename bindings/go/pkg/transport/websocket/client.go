package websocket

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/gorilla/websocket"
	"github.com/hivellm/umicp-go/pkg/transport"
	"github.com/hivellm/umicp-go/pkg/umicp"
)

// ClientConfig contains WebSocket client configuration
type ClientConfig struct {
	URL                  string
	HeartbeatInterval    time.Duration
	MaxReconnectAttempts int
	ReconnectDelay       time.Duration
	ReadBufferSize       int
	WriteBufferSize      int
	HandshakeTimeout     time.Duration
}

// DefaultClientConfig returns default client configuration
func DefaultClientConfig() *ClientConfig {
	return &ClientConfig{
		HeartbeatInterval:    30 * time.Second,
		MaxReconnectAttempts: 5,
		ReconnectDelay:       time.Second,
		ReadBufferSize:       4096,
		WriteBufferSize:      4096,
		HandshakeTimeout:     10 * time.Second,
	}
}

// Client represents a WebSocket client
type Client struct {
	config ClientConfig
	conn   *websocket.Conn
	mu     sync.RWMutex

	// Channels
	sendCh    chan *umicp.Envelope
	receiveCh chan *umicp.Envelope
	closeCh   chan struct{}
	doneCh    chan struct{}

	// State
	connected bool
	stats     *transport.Stats

	// Callbacks
	onMessage    func(*umicp.Envelope)
	onConnect    func()
	onDisconnect func(error)
	onError      func(error)
}

// NewClient creates a new WebSocket client
func NewClient(config ClientConfig) *Client {
	if config.HeartbeatInterval == 0 {
		config.HeartbeatInterval = 30 * time.Second
	}
	if config.MaxReconnectAttempts == 0 {
		config.MaxReconnectAttempts = 5
	}
	if config.ReconnectDelay == 0 {
		config.ReconnectDelay = time.Second
	}
	if config.ReadBufferSize == 0 {
		config.ReadBufferSize = 4096
	}
	if config.WriteBufferSize == 0 {
		config.WriteBufferSize = 4096
	}
	if config.HandshakeTimeout == 0 {
		config.HandshakeTimeout = 10 * time.Second
	}

	return &Client{
		config:    config,
		sendCh:    make(chan *umicp.Envelope, 100),
		receiveCh: make(chan *umicp.Envelope, 100),
		closeCh:   make(chan struct{}),
		doneCh:    make(chan struct{}),
		stats: &transport.Stats{
			ConnectedAt: time.Time{},
		},
	}
}

// Connect establishes connection to the WebSocket server
func (c *Client) Connect(ctx context.Context) error {
	c.mu.Lock()
	defer c.mu.Unlock()

	if c.connected {
		return umicp.ErrAlreadyConnected
	}

	dialer := websocket.Dialer{
		ReadBufferSize:   c.config.ReadBufferSize,
		WriteBufferSize:  c.config.WriteBufferSize,
		HandshakeTimeout: c.config.HandshakeTimeout,
	}

	conn, _, err := dialer.DialContext(ctx, c.config.URL, nil)
	if err != nil {
		return fmt.Errorf("failed to connect: %w", err)
	}

	c.conn = conn
	c.connected = true
	c.stats.ConnectedAt = time.Now()

	// Start goroutines
	go c.readLoop()
	go c.writeLoop()
	go c.heartbeatLoop()

	if c.onConnect != nil {
		go c.onConnect()
	}

	return nil
}

// Disconnect closes the connection
func (c *Client) Disconnect(ctx context.Context) error {
	c.mu.Lock()
	defer c.mu.Unlock()

	if !c.connected {
		return nil
	}

	close(c.closeCh)
	c.connected = false

	if c.conn != nil {
		return c.conn.Close()
	}

	return nil
}

// Send sends an envelope to the server
func (c *Client) Send(ctx context.Context, env *umicp.Envelope) error {
	if !c.IsConnected() {
		return umicp.ErrNotConnected
	}

	select {
	case c.sendCh <- env:
		return nil
	case <-ctx.Done():
		return ctx.Err()
	case <-c.closeCh:
		return umicp.ErrConnectionClosed
	}
}

// IsConnected returns true if connected
func (c *Client) IsConnected() bool {
	c.mu.RLock()
	defer c.mu.RUnlock()
	return c.connected
}

// Stats returns transport statistics
func (c *Client) Stats() *transport.Stats {
	c.mu.RLock()
	defer c.mu.RUnlock()

	statsCopy := *c.stats
	if !c.stats.ConnectedAt.IsZero() {
		statsCopy.Uptime = time.Since(c.stats.ConnectedAt)
	}

	return &statsCopy
}

// OnMessage sets the message handler
func (c *Client) OnMessage(fn func(*umicp.Envelope)) {
	c.onMessage = fn
}

// OnConnect sets the connect handler
func (c *Client) OnConnect(fn func()) {
	c.onConnect = fn
}

// OnDisconnect sets the disconnect handler
func (c *Client) OnDisconnect(fn func(error)) {
	c.onDisconnect = fn
}

// OnError sets the error handler
func (c *Client) OnError(fn func(error)) {
	c.onError = fn
}

// readLoop reads messages from the WebSocket connection
func (c *Client) readLoop() {
	defer func() {
		if c.onDisconnect != nil {
			c.onDisconnect(nil)
		}
		close(c.doneCh)
	}()

	for {
		select {
		case <-c.closeCh:
			return
		default:
			_, data, err := c.conn.ReadMessage()
			if err != nil {
				if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
					if c.onError != nil {
						c.onError(err)
					}
				}
				return
			}

			env, err := umicp.DeserializeEnvelope(data)
			if err != nil {
				if c.onError != nil {
					c.onError(err)
				}
				c.stats.Errors++
				continue
			}

			c.stats.MessagesReceived++
			c.stats.BytesReceived += int64(len(data))
			c.stats.LastMessageAt = time.Now()

			if c.onMessage != nil {
				go c.onMessage(env)
			}
		}
	}
}

// writeLoop writes messages to the WebSocket connection
func (c *Client) writeLoop() {
	ticker := time.NewTicker(time.Second)
	defer ticker.Stop()

	for {
		select {
		case <-c.closeCh:
			return
		case env := <-c.sendCh:
			data, err := env.Serialize()
			if err != nil {
				if c.onError != nil {
					c.onError(err)
				}
				c.stats.Errors++
				continue
			}

			if err := c.conn.WriteMessage(websocket.BinaryMessage, data); err != nil {
				if c.onError != nil {
					c.onError(err)
				}
				c.stats.Errors++
				return
			}

			c.stats.MessagesSent++
			c.stats.BytesSent += int64(len(data))
		}
	}
}

// heartbeatLoop sends periodic ping messages
func (c *Client) heartbeatLoop() {
	ticker := time.NewTicker(c.config.HeartbeatInterval)
	defer ticker.Stop()

	for {
		select {
		case <-c.closeCh:
			return
		case <-ticker.C:
			if err := c.conn.WriteMessage(websocket.PingMessage, nil); err != nil {
				if c.onError != nil {
					c.onError(err)
				}
				return
			}
		}
	}
}
