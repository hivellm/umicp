package websocket

import (
	"testing"

	"github.com/hivellm/umicp-go/pkg/umicp"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestNewServer(t *testing.T) {
	config := ServerConfig{
		Addr: "127.0.0.1:8080",
		Path: "/ws",
	}

	server := NewServer(config)
	require.NotNil(t, server)
	assert.Equal(t, config.Addr, server.config.Addr)
	assert.Equal(t, config.Path, server.config.Path)
	assert.NotZero(t, server.config.MaxPayload)
	assert.NotNil(t, server.stats)
}

func TestDefaultServerConfig(t *testing.T) {
	config := DefaultServerConfig()
	require.NotNil(t, config)

	assert.Equal(t, "127.0.0.1:8080", config.Addr)
	assert.Equal(t, "/ws", config.Path)
	assert.Equal(t, int64(100*1024*1024), config.MaxPayload)
	assert.Equal(t, 4096, config.ReadBufferSize)
	assert.Equal(t, 4096, config.WriteBufferSize)
}

func TestServer_GetClients(t *testing.T) {
	server := NewServer(ServerConfig{Addr: "127.0.0.1:8080"})

	clients := server.GetClients()
	assert.Empty(t, clients)

	// Add mock client
	mockConn := &Connection{
		ID: "test-client",
	}
	server.clients.Store(mockConn.ID, mockConn)

	clients = server.GetClients()
	assert.Len(t, clients, 1)
	assert.Equal(t, "test-client", clients[0].ID)
}

func TestServer_Stats(t *testing.T) {
	server := NewServer(ServerConfig{Addr: "127.0.0.1:8080"})

	stats := server.Stats()
	require.NotNil(t, stats)
	assert.Equal(t, int64(0), stats.MessagesSent)
	assert.Equal(t, int64(0), stats.MessagesReceived)
	assert.NotZero(t, stats.Uptime)
}

func TestServer_CallbacksRegistration(t *testing.T) {
	server := NewServer(ServerConfig{Addr: "127.0.0.1:8080"})

	messageCalled := false
	server.OnMessage(func(env *umicp.Envelope, conn *Connection) {
		messageCalled = true
	})
	assert.NotNil(t, server.onMessage)
	assert.False(t, messageCalled)

	connectCalled := false
	server.OnConnect(func(conn *Connection) {
		connectCalled = true
	})
	assert.NotNil(t, server.onConnect)
	assert.False(t, connectCalled)

	disconnectCalled := false
	server.OnDisconnect(func(conn *Connection) {
		disconnectCalled = true
	})
	assert.NotNil(t, server.onDisconnect)
	assert.False(t, disconnectCalled)

	errorCalled := false
	server.OnError(func(err error) {
		errorCalled = true
	})
	assert.NotNil(t, server.onError)
	assert.False(t, errorCalled)
}

func TestConnection(t *testing.T) {
	conn := &Connection{
		ID:     "test-123",
		SendCh: make(chan *umicp.Envelope, 10),
	}

	assert.Equal(t, "test-123", conn.ID)
	assert.NotNil(t, conn.SendCh)
}
