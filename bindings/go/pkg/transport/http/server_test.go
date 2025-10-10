package http

import (
	"testing"
	"time"

	"github.com/hivellm/umicp-go/pkg/umicp"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestNewServer(t *testing.T) {
	config := ServerConfig{
		Addr: "127.0.0.1:8080",
		Path: "/api",
	}

	server := NewServer(config)
	require.NotNil(t, server)
	assert.Equal(t, config.Addr, server.config.Addr)
	assert.Equal(t, config.Path, server.config.Path)
	assert.NotNil(t, server.stats)
}

func TestDefaultServerConfig(t *testing.T) {
	config := DefaultServerConfig()
	require.NotNil(t, config)

	assert.Equal(t, "127.0.0.1:8080", config.Addr)
	assert.Equal(t, "/", config.Path)
	assert.Equal(t, 30*time.Second, config.ReadTimeout)
	assert.Equal(t, 30*time.Second, config.WriteTimeout)
}

func TestServer_OnRequest(t *testing.T) {
	server := NewServer(ServerConfig{})

	handlerCalled := false
	server.OnRequest(func(env *umicp.Envelope) (*umicp.Envelope, error) {
		handlerCalled = true
		return env, nil
	})

	assert.NotNil(t, server.handler)
}

func TestServer_Stats(t *testing.T) {
	server := NewServer(ServerConfig{})

	stats := server.Stats()
	require.NotNil(t, stats)
	assert.Equal(t, int64(0), stats.MessagesSent)
	assert.Equal(t, int64(0), stats.MessagesReceived)
	assert.NotZero(t, stats.Uptime)
}
