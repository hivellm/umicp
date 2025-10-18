package http

import (
	"context"
	"testing"
	"time"

	"github.com/hivellm/umicp-go/pkg/umicp"
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

func TestClient_ConnectDisconnect(t *testing.T) {
	client := NewClient(ClientConfig{BaseURL: "http://localhost:8080"})

	ctx := context.Background()

	// Should connect successfully
	err := client.Connect(ctx)
	require.NoError(t, err)
	assert.True(t, client.IsConnected())

	// Should disconnect successfully
	err = client.Disconnect(ctx)
	require.NoError(t, err)
	assert.False(t, client.IsConnected())
}

func TestClient_SendNotConnected(t *testing.T) {
	client := NewClient(ClientConfig{BaseURL: "http://localhost:8080"})

	env, _ := umicp.NewEnvelope().
		From("test").
		To("server").
		Operation(umicp.OperationData).
		Build()

	ctx := context.Background()

	// Should fail when not connected
	err := client.Send(ctx, env)
	assert.Error(t, err)
	assert.Contains(t, err.Error(), "not connected")
}

// Test automatic URL path detection
func TestNewClient_AutomaticPathDetection(t *testing.T) {
	tests := []struct {
		name        string
		baseURL     string
		path        string
		expectedURL string
		expectedPath string
	}{
		{
			name:         "URL with custom path",
			baseURL:      "http://localhost:15002/umicp",
			path:         "",
			expectedURL:  "http://localhost:15002",
			expectedPath: "/umicp",
		},
		{
			name:         "URL without path - default",
			baseURL:      "http://localhost:3000",
			path:         "",
			expectedURL:  "http://localhost:3000",
			expectedPath: "/umicp",
		},
		{
			name:         "URL with root path - default",
			baseURL:      "http://localhost:3000/",
			path:         "",
			expectedURL:  "http://localhost:3000",
			expectedPath: "/umicp",
		},
		{
			name:         "URL with port and path",
			baseURL:      "http://127.0.0.1:15002/umicp",
			path:         "",
			expectedURL:  "http://127.0.0.1:15002",
			expectedPath: "/umicp",
		},
		{
			name:         "Explicit path overrides empty URL path",
			baseURL:      "http://localhost:3000",
			path:         "/custom",
			expectedURL:  "http://localhost:3000",
			expectedPath: "/custom",
		},
		{
			name:         "URL path takes precedence over explicit path",
			baseURL:      "http://localhost:3000/umicp",
			path:         "/custom",
			expectedURL:  "http://localhost:3000",
			expectedPath: "/umicp",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			config := ClientConfig{
				BaseURL: tt.baseURL,
				Path:    tt.path,
			}

			client := NewClient(config)

			assert.Equal(t, tt.expectedURL, client.config.BaseURL,
				"Base URL mismatch for test: %s", tt.name)
			assert.Equal(t, tt.expectedPath, client.config.Path,
				"Path mismatch for test: %s", tt.name)
		})
	}
}

func BenchmarkHTTPClient_Send(b *testing.B) {
	// Setup server
	config := ServerConfig{Addr: "127.0.0.1:18082"}
	server := NewServer(config)

	server.OnRequest(func(env *umicp.Envelope) (*umicp.Envelope, error) {
		response, _ := umicp.NewEnvelope().
			From("server").
			To(env.From).
			Operation(umicp.OperationResponse).
			Build()
		return response, nil
	})

	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()

	err := server.Start(ctx)
	require.NoError(b, err)
	defer server.Stop(context.Background())

	time.Sleep(100 * time.Millisecond) // Give server time to start

	// Setup client
	clientConfig := ClientConfig{BaseURL: "http://127.0.0.1:18082"}
	client := NewClient(clientConfig)
	defer client.Close()

	err = client.Connect(context.Background())
	require.NoError(b, err)
	defer client.Disconnect(context.Background())

	// Prepare envelope
	env, _ := umicp.NewEnvelope().
		From("benchmark-client").
		To("benchmark-server").
		Operation(umicp.OperationData).
		Capability("message", "benchmark message").
		Build()

	b.ResetTimer()
	b.ReportAllocs()

	for i := 0; i < b.N; i++ {
		err := client.Send(context.Background(), env)
		if err != nil {
			b.Fatal(err)
		}
	}
}
