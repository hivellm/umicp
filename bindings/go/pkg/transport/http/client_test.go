package http

import (
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestNewClient(t *testing.T) {
	config := ClientConfig{
		BaseURL: "http://localhost:8080",
	}

	client := NewClient(config)
	require.NotNil(t, client)
	assert.Equal(t, config.BaseURL, client.config.BaseURL)
	assert.NotZero(t, client.config.Timeout)
	assert.NotNil(t, client.stats)
}

func TestDefaultClientConfig(t *testing.T) {
	config := DefaultClientConfig()
	require.NotNil(t, config)

	assert.Equal(t, 30*time.Second, config.Timeout)
	assert.Equal(t, 10, config.MaxIdleConns)
	assert.Equal(t, 90*time.Second, config.IdleConnTimeout)
}

func TestClient_Stats(t *testing.T) {
	client := NewClient(ClientConfig{BaseURL: "http://localhost:8080"})

	stats := client.Stats()
	require.NotNil(t, stats)
	assert.Equal(t, int64(0), stats.MessagesSent)
	assert.Equal(t, int64(0), stats.MessagesReceived)
	assert.NotZero(t, stats.Uptime)
}

func TestClient_Close(t *testing.T) {
	client := NewClient(ClientConfig{BaseURL: "http://localhost:8080"})

	// Should not panic
	client.Close()
}
