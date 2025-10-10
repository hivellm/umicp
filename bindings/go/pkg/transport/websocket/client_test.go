package websocket

import (
	"testing"
	"time"

	"github.com/hivellm/umicp-go/pkg/umicp"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestNewClient(t *testing.T) {
	config := ClientConfig{
		URL: "ws://localhost:8080/ws",
	}

	client := NewClient(config)
	require.NotNil(t, client)
	assert.Equal(t, config.URL, client.config.URL)
	assert.NotZero(t, client.config.HeartbeatInterval)
	assert.NotZero(t, client.config.MaxReconnectAttempts)
	assert.NotNil(t, client.sendCh)
	assert.NotNil(t, client.stats)
}

func TestDefaultClientConfig(t *testing.T) {
	config := DefaultClientConfig()
	require.NotNil(t, config)

	assert.Equal(t, 30*time.Second, config.HeartbeatInterval)
	assert.Equal(t, 5, config.MaxReconnectAttempts)
	assert.Equal(t, time.Second, config.ReconnectDelay)
	assert.Equal(t, 4096, config.ReadBufferSize)
	assert.Equal(t, 4096, config.WriteBufferSize)
	assert.Equal(t, 10*time.Second, config.HandshakeTimeout)
}

func TestClient_IsConnected(t *testing.T) {
	client := NewClient(ClientConfig{URL: "ws://localhost:8080"})
	assert.False(t, client.IsConnected())

	// Simulate connection
	client.mu.Lock()
	client.connected = true
	client.mu.Unlock()

	assert.True(t, client.IsConnected())
}

func TestClient_Stats(t *testing.T) {
	client := NewClient(ClientConfig{URL: "ws://localhost:8080"})

	stats := client.Stats()
	require.NotNil(t, stats)
	assert.Equal(t, int64(0), stats.MessagesSent)
	assert.Equal(t, int64(0), stats.MessagesReceived)
}

func TestClient_CallbacksRegistration(t *testing.T) {
	client := NewClient(ClientConfig{URL: "ws://localhost:8080"})

	messageCalled := false
	client.OnMessage(func(env *umicp.Envelope) {
		messageCalled = true
	})
	assert.NotNil(t, client.onMessage)

	connectCalled := false
	client.OnConnect(func() {
		connectCalled = true
	})
	assert.NotNil(t, client.onConnect)

	disconnectCalled := false
	client.OnDisconnect(func(err error) {
		disconnectCalled = true
	})
	assert.NotNil(t, client.onDisconnect)

	errorCalled := false
	client.OnError(func(err error) {
		errorCalled = true
	})
	assert.NotNil(t, client.onError)
}
