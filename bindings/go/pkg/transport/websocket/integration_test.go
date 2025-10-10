package websocket

import (
	"context"
	"testing"
	"time"

	"github.com/hivellm/umicp-go/pkg/umicp"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

// TestClientServerIntegration tests client-server communication
func TestClientServerIntegration(t *testing.T) {
	// Skip if running short tests
	if testing.Short() {
		t.Skip("Skipping integration test in short mode")
	}

	ctx := context.Background()

	// Start server
	serverConfig := DefaultServerConfig()
	serverConfig.Addr = "127.0.0.1:18080"
	serverConfig.Path = "/test"

	server := NewServer(*serverConfig)

	serverReceived := make(chan *umicp.Envelope, 10)
	server.OnMessage(func(env *umicp.Envelope, conn *Connection) {
		serverReceived <- env
	})

	go func() {
		if err := server.Start(ctx); err != nil {
			t.Logf("Server error: %v", err)
		}
	}()

	// Give server time to start
	time.Sleep(500 * time.Millisecond)

	// Create client
	clientConfig := DefaultClientConfig()
	clientConfig.URL = "ws://127.0.0.1:18080/test"

	client := NewClient(*clientConfig)

	clientReceived := make(chan *umicp.Envelope, 10)
	client.OnMessage(func(env *umicp.Envelope) {
		clientReceived <- env
	})

	// Connect client
	err := client.Connect(ctx)
	require.NoError(t, err)
	defer client.Disconnect(ctx)

	// Give connection time to establish
	time.Sleep(100 * time.Millisecond)

	// Send message from client to server
	envelope, err := umicp.NewEnvelope().
		From("client").
		To("server").
		Operation(umicp.OperationData).
		Capability("test", "message").
		Build()
	require.NoError(t, err)

	err = client.Send(ctx, envelope)
	require.NoError(t, err)

	// Wait for server to receive
	select {
	case received := <-serverReceived:
		assert.Equal(t, "client", received.From)
		assert.Equal(t, "server", received.To)
		assert.Equal(t, "message", received.Capabilities["test"])
	case <-time.After(2 * time.Second):
		t.Fatal("Timeout waiting for server to receive message")
	}

	// Check client stats
	stats := client.Stats()
	assert.Equal(t, int64(1), stats.MessagesSent)

	// Check server stats
	serverStats := server.Stats()
	assert.Equal(t, int64(1), serverStats.MessagesReceived)
}

// TestServerBroadcast tests broadcast functionality
func TestServerBroadcast(t *testing.T) {
	if testing.Short() {
		t.Skip("Skipping integration test in short mode")
	}

	ctx := context.Background()

	// Start server
	serverConfig := DefaultServerConfig()
	serverConfig.Addr = "127.0.0.1:18081"
	server := NewServer(*serverConfig)

	go func() {
		if err := server.Start(ctx); err != nil {
			t.Logf("Server error: %v", err)
		}
	}()

	time.Sleep(500 * time.Millisecond)

	// Create 3 clients
	clients := make([]*Client, 3)
	received := make([]chan *umicp.Envelope, 3)

	for i := 0; i < 3; i++ {
		clientConfig := DefaultClientConfig()
		clientConfig.URL = "ws://127.0.0.1:18081/ws"

		clients[i] = NewClient(*clientConfig)
		received[i] = make(chan *umicp.Envelope, 10)

		// Capture i in closure
		idx := i
		clients[i].OnMessage(func(env *umicp.Envelope) {
			received[idx] <- env
		})

		err := clients[i].Connect(ctx)
		require.NoError(t, err)
		defer clients[i].Disconnect(ctx)
	}

	time.Sleep(200 * time.Millisecond)

	// Broadcast message
	envelope, _ := umicp.NewEnvelope().
		From("server").
		To("all").
		Operation(umicp.OperationData).
		Capability("broadcast", "test").
		Build()

	count := server.Broadcast(ctx, envelope)
	assert.Equal(t, 3, count, "Should broadcast to 3 clients")

	// Verify all clients received
	for i := 0; i < 3; i++ {
		select {
		case env := <-received[i]:
			assert.Equal(t, "test", env.Capabilities["broadcast"])
		case <-time.After(2 * time.Second):
			t.Fatalf("Client %d timeout", i)
		}
	}
}
