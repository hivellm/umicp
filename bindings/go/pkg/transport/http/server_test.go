package http

import (
	"context"
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
	assert.False(t, handlerCalled)
}

func TestServer_Stats(t *testing.T) {
	server := NewServer(ServerConfig{})

	stats := server.Stats()
	require.NotNil(t, stats)
	assert.Equal(t, int64(0), stats.MessagesSent)
	assert.Equal(t, int64(0), stats.MessagesReceived)
	assert.NotZero(t, stats.Uptime)
}

func TestServer_StartStop(t *testing.T) {
	config := ServerConfig{Addr: "127.0.0.1:0"} // Use port 0 for auto-assignment
	server := NewServer(config)

	ctx, cancel := context.WithCancel(context.Background())

	// Should start successfully
	err := server.Start(ctx)
	require.NoError(t, err)
	assert.True(t, server.IsRunning())

	// Should stop successfully
	cancel()
	time.Sleep(100 * time.Millisecond) // Give time for shutdown
	assert.False(t, server.IsRunning())
}

func TestServer_GetConnections(t *testing.T) {
	server := NewServer(ServerConfig{})

	connections := server.GetConnections()
	assert.Equal(t, 0, connections)
}

func TestHTTPServerClientIntegration(t *testing.T) {
	if testing.Short() {
		t.Skip("Skipping integration test in short mode")
	}

	// Setup server with specific port
	config := ServerConfig{Addr: "127.0.0.1:18081"} // Use a specific test port
	server := NewServer(config)

	// Setup handler
	server.OnRequest(func(env *umicp.Envelope) (*umicp.Envelope, error) {
		response, _ := umicp.NewEnvelope().
			From("server").
			To(env.From).
			Operation(umicp.OperationResponse).
			Capability("status", "received").
			Build()
		return response, nil
	})

	// Start server
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	err := server.Start(ctx)
	require.NoError(t, err)
	defer server.Stop(context.Background())

	time.Sleep(200 * time.Millisecond) // Give server time to start

	// Setup client
	clientConfig := ClientConfig{
		BaseURL: "http://127.0.0.1:18081",
	}
	client := NewClient(clientConfig)
	defer client.Close()

	// Connect
	err = client.Connect(context.Background())
	require.NoError(t, err)
	defer client.Disconnect(context.Background())

	// Send message
	env, _ := umicp.NewEnvelope().
		From("client").
		To("server").
		Operation(umicp.OperationData).
		Capability("message", "test message").
		Build()

	err = client.Send(context.Background(), env)
	require.NoError(t, err)

	// Verify statistics
	stats := client.Stats()
	assert.Equal(t, int64(1), stats.MessagesSent)
	assert.True(t, stats.BytesSent > 0)
	assert.Equal(t, int64(0), stats.Errors)
}
