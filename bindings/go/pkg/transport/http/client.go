package http

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"net/http"
	"sync"
	"time"

	"github.com/hivellm/umicp-go/pkg/transport"
	"github.com/hivellm/umicp-go/pkg/umicp"
	"golang.org/x/net/http2"
)

// ClientConfig contains HTTP/2 client configuration
type ClientConfig struct {
	BaseURL         string
	Timeout         time.Duration
	MaxIdleConns    int
	IdleConnTimeout time.Duration
}

// DefaultClientConfig returns default HTTP client configuration
func DefaultClientConfig() *ClientConfig {
	return &ClientConfig{
		Timeout:         30 * time.Second,
		MaxIdleConns:    10,
		IdleConnTimeout: 90 * time.Second,
	}
}

// Client represents an HTTP/2 streaming client
type Client struct {
	config ClientConfig
	client *http.Client
	mu     sync.RWMutex
	stats  *transport.Stats
}

// NewClient creates a new HTTP/2 client
func NewClient(config ClientConfig) *Client {
	if config.Timeout == 0 {
		config.Timeout = 30 * time.Second
	}
	if config.MaxIdleConns == 0 {
		config.MaxIdleConns = 10
	}
	if config.IdleConnTimeout == 0 {
		config.IdleConnTimeout = 90 * time.Second
	}

	httpTransport := &http.Transport{
		MaxIdleConns:    config.MaxIdleConns,
		IdleConnTimeout: config.IdleConnTimeout,
	}

	http2.ConfigureTransport(httpTransport)

	return &Client{
		config: config,
		client: &http.Client{
			Transport: httpTransport,
			Timeout:   config.Timeout,
		},
		stats: &transport.Stats{
			ConnectedAt: time.Now(),
		},
	}
}

// Send sends an envelope and waits for response
func (c *Client) Send(ctx context.Context, env *umicp.Envelope) (*umicp.Envelope, error) {
	data, err := env.Serialize()
	if err != nil {
		return nil, fmt.Errorf("serialization failed: %w", err)
	}

	req, err := http.NewRequestWithContext(ctx, "POST", c.config.BaseURL, bytes.NewReader(data))
	if err != nil {
		return nil, fmt.Errorf("request creation failed: %w", err)
	}

	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Accept", "application/json")

	resp, err := c.client.Do(req)
	if err != nil {
		c.mu.Lock()
		c.stats.Errors++
		c.mu.Unlock()
		return nil, fmt.Errorf("request failed: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		c.mu.Lock()
		c.stats.Errors++
		c.mu.Unlock()
		return nil, fmt.Errorf("unexpected status code: %d", resp.StatusCode)
	}

	respData, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("response read failed: %w", err)
	}

	c.mu.Lock()
	c.stats.MessagesSent++
	c.stats.BytesSent += int64(len(data))
	c.stats.MessagesReceived++
	c.stats.BytesReceived += int64(len(respData))
	c.stats.LastMessageAt = time.Now()
	c.mu.Unlock()

	return umicp.DeserializeEnvelope(respData)
}

// Stats returns transport statistics
func (c *Client) Stats() *transport.Stats {
	c.mu.RLock()
	defer c.mu.RUnlock()

	statsCopy := *c.stats
	statsCopy.Uptime = time.Since(c.stats.ConnectedAt)

	return &statsCopy
}

// Close closes the client
func (c *Client) Close() {
	c.client.CloseIdleConnections()
}
