package integration

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/hivellm/umicp-go/pkg/peer"
	"github.com/hivellm/umicp-go/pkg/umicp"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

// TestE2E_TwoPeerCommunication tests basic 2-peer communication
func TestE2E_TwoPeerCommunication(t *testing.T) {
	if testing.Short() {
		t.Skip("Skipping E2E test in short mode")
	}

	ctx := context.Background()

	// Create peer A with server
	peerA, err := peer.NewPeerWithServer(
		peer.PeerConfig{
			PeerID:       "peer-a",
			AutoProtocol: true,
		},
		peer.ServerConfig{
			Addr: "127.0.0.1:19001",
			Path: "/umicp",
		},
	)
	require.NoError(t, err)

	// Setup handlers
	aReadyCh := make(chan *peer.PeerInfo, 1)
	aDataCh := make(chan *umicp.Envelope, 10)

	peerA.On(peer.EventPeerReady, func(e *peer.Event) {
		aReadyCh <- e.PeerInfo
	})

	peerA.On(peer.EventData, func(e *peer.Event) {
		aDataCh <- e.Envelope
	})

	// Start peer A
	err = peerA.Start(ctx)
	require.NoError(t, err)
	defer peerA.Shutdown(ctx)

	time.Sleep(500 * time.Millisecond)

	// Create peer B (client-only)
	peerB := peer.NewPeer(peer.PeerConfig{
		PeerID:       "peer-b",
		AutoProtocol: true,
	})

	bReadyCh := make(chan *peer.PeerInfo, 1)
	peerB.On(peer.EventPeerReady, func(e *peer.Event) {
		bReadyCh <- e.PeerInfo
	})

	// Connect B to A
	peerBID, err := peerB.ConnectToPeer(ctx, "ws://127.0.0.1:19001/umicp", nil)
	require.NoError(t, err)

	// Wait for handshake (both sides)
	select {
	case info := <-bReadyCh:
		assert.Equal(t, "peer-a", info.PeerID)
	case <-time.After(5 * time.Second):
		t.Fatal("Peer B handshake timeout")
	}

	select {
	case info := <-aReadyCh:
		assert.Equal(t, "peer-b", info.PeerID)
	case <-time.After(5 * time.Second):
		t.Fatal("Peer A handshake timeout")
	}

	// Send message from B to A
	env, _ := umicp.NewEnvelope().
		From("peer-b").
		To("peer-a").
		Operation(umicp.OperationData).
		Capability("message", "Hello from B").
		Build()

	err = peerB.SendToPeer(ctx, peerBID, env)
	require.NoError(t, err)

	// Wait for A to receive
	select {
	case received := <-aDataCh:
		assert.Equal(t, "peer-b", received.From)
		assert.Equal(t, "Hello from B", received.Capabilities["message"])
	case <-time.After(2 * time.Second):
		t.Fatal("Message receive timeout")
	}

	// Verify stats
	statsB := peerB.GetStats()
	assert.Equal(t, 1, statsB.TotalPeers)
	assert.Equal(t, 1, statsB.OutgoingConnections)
	assert.Equal(t, 1, statsB.HandshakesComplete)
}

// TestE2E_ThreePeerMesh tests 3-peer mesh topology
func TestE2E_ThreePeerMesh(t *testing.T) {
	if testing.Short() {
		t.Skip("Skipping E2E test in short mode")
	}

	ctx := context.Background()

	// Create 3 peers with servers
	peers := make([]*peer.Peer, 3)
	ports := []int{19011, 19012, 19013}

	for i := 0; i < 3; i++ {
		p, err := peer.NewPeerWithServer(
			peer.PeerConfig{
				PeerID:       fmt.Sprintf("peer-%d", i+1),
				AutoProtocol: true,
			},
			peer.ServerConfig{
				Addr: fmt.Sprintf("127.0.0.1:%d", ports[i]),
				Path: "/umicp",
			},
		)
		require.NoError(t, err)

		p.Start(ctx)
		peers[i] = p
		defer p.Shutdown(ctx)
	}

	time.Sleep(time.Second)

	// Connect in mesh (each connects to all others)
	for i, p := range peers {
		for j, port := range ports {
			if i != j {
				url := fmt.Sprintf("ws://127.0.0.1:%d/umicp", port)
				_, err := p.ConnectToPeer(ctx, url, nil)
				if err != nil {
					t.Logf("Connect error (expected during mesh): %v", err)
				}
			}
		}
	}

	// Wait for connections
	time.Sleep(2 * time.Second)

	// Verify mesh structure
	for i, p := range peers {
		stats := p.GetStats()
		t.Logf("Peer %d: %d total connections", i+1, stats.TotalPeers)
		assert.GreaterOrEqual(t, stats.TotalPeers, 1, "Should have at least 1 connection")
	}
}
