package peer

import (
	"testing"
	"time"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestNewPeer(t *testing.T) {
	config := PeerConfig{
		PeerID: "test-peer",
	}

	peer := NewPeer(config)
	require.NotNil(t, peer)
	assert.Equal(t, "test-peer", peer.config.PeerID)
	assert.NotZero(t, peer.config.HandshakeTimeout)
	assert.NotNil(t, peer.eventBus)
}

func TestNewPeer_AutoGenerateID(t *testing.T) {
	peer := NewPeer(PeerConfig{})
	assert.NotEmpty(t, peer.config.PeerID)
}

func TestNewPeerWithServer(t *testing.T) {
	peerConfig := PeerConfig{
		PeerID: "test-peer",
	}
	serverConfig := ServerConfig{
		Addr: "127.0.0.1:18090",
		Path: "/test",
	}

	peer, err := NewPeerWithServer(peerConfig, serverConfig)
	require.NoError(t, err)
	require.NotNil(t, peer)
	assert.NotNil(t, peer.server)
}

func TestPeer_GetStats(t *testing.T) {
	peer := NewPeer(PeerConfig{})

	stats := peer.GetStats()
	assert.Equal(t, 0, stats.TotalPeers)
	assert.Equal(t, 0, stats.IncomingConnections)
	assert.Equal(t, 0, stats.OutgoingConnections)
}

func TestPeer_GetPeers(t *testing.T) {
	peer := NewPeer(PeerConfig{})

	peers := peer.GetPeers()
	assert.Empty(t, peers)

	// Add mock peer
	mockPeer := &PeerConnection{
		ID:   "test-123",
		Type: PeerTypeIncoming,
	}
	peer.peers.Store(mockPeer.ID, mockPeer)

	peers = peer.GetPeers()
	assert.Len(t, peers, 1)
	assert.Equal(t, "test-123", peers[0].ID)
}

func TestPeer_GetPeersByType(t *testing.T) {
	peer := NewPeer(PeerConfig{})

	// Add mixed peer types
	incoming := &PeerConnection{ID: "inc-1", Type: PeerTypeIncoming}
	outgoing := &PeerConnection{ID: "out-1", Type: PeerTypeOutgoing}

	peer.peers.Store(incoming.ID, incoming)
	peer.peers.Store(outgoing.ID, outgoing)

	incomingPeers := peer.GetPeersByType(PeerTypeIncoming)
	assert.Len(t, incomingPeers, 1)
	assert.Equal(t, PeerTypeIncoming, incomingPeers[0].Type)

	outgoingPeers := peer.GetPeersByType(PeerTypeOutgoing)
	assert.Len(t, outgoingPeers, 1)
	assert.Equal(t, PeerTypeOutgoing, outgoingPeers[0].Type)
}

func TestPeer_EventHandlers(t *testing.T) {
	peer := NewPeer(PeerConfig{})

	messageCalled := false
	peer.On(EventMessage, func(e *Event) {
		messageCalled = true
	})

	// Simulate event emission
	peer.eventBus.Emit(&Event{
		Type: EventMessage,
	})

	// Give time for event to process
	time.Sleep(50 * time.Millisecond)

	// Note: In real test, would use channels to verify
	assert.NotNil(t, peer.eventBus)
	assert.True(t, messageCalled)
}

func TestPeerConnection_Types(t *testing.T) {
	incoming := &PeerConnection{Type: PeerTypeIncoming}
	outgoing := &PeerConnection{Type: PeerTypeOutgoing}

	assert.Equal(t, "incoming", incoming.Type.String())
	assert.Equal(t, "outgoing", outgoing.Type.String())
}

func TestPeerInfo(t *testing.T) {
	info := &PeerInfo{
		PeerID:       "peer-123",
		Metadata:     map[string]string{"key": "value"},
		Capabilities: map[string]string{"version": "1.0"},
		CompletedAt:  time.Now(),
	}

	assert.Equal(t, "peer-123", info.PeerID)
	assert.Equal(t, "value", info.Metadata["key"])
	assert.Equal(t, "1.0", info.Capabilities["version"])
	assert.False(t, info.CompletedAt.IsZero())
}

func TestEventType_String(t *testing.T) {
	tests := []struct {
		eventType EventType
		want      string
	}{
		{EventMessage, "message"},
		{EventData, "data"},
		{EventPeerConnect, "peer:connect"},
		{EventPeerDisconnect, "peer:disconnect"},
		{EventPeerReady, "peer:ready"},
		{EventError, "error"},
	}

	for _, tt := range tests {
		t.Run(tt.want, func(t *testing.T) {
			assert.Equal(t, tt.want, tt.eventType.String())
		})
	}
}
