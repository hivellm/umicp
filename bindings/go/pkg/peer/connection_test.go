package peer

import (
	"testing"
	"time"
)

func TestPeerType(t *testing.T) {
	t.Run("Incoming type", func(t *testing.T) {
		peerType := PeerTypeIncoming
		if peerType != "incoming" {
			t.Errorf("Expected 'incoming', got %s", peerType)
		}
		if peerType.String() != "incoming" {
			t.Errorf("String() failed, got %s", peerType.String())
		}
	})

	t.Run("Outgoing type", func(t *testing.T) {
		peerType := PeerTypeOutgoing
		if peerType != "outgoing" {
			t.Errorf("Expected 'outgoing', got %s", peerType)
		}
		if peerType.String() != "outgoing" {
			t.Errorf("String() failed, got %s", peerType.String())
		}
	})
}

func TestPeerConnection(t *testing.T) {
	t.Run("Create peer connection", func(t *testing.T) {
		conn := &PeerConnection{
			ID:                "peer-001",
			Type:              PeerTypeOutgoing,
			URL:               "ws://localhost:8080",
			Metadata:          make(map[string]string),
			ConnectedAt:       time.Now(),
			HandshakeComplete: false,
		}

		if conn.ID != "peer-001" {
			t.Errorf("Expected ID 'peer-001', got %s", conn.ID)
		}
		if conn.Type != PeerTypeOutgoing {
			t.Errorf("Expected type outgoing, got %s", conn.Type)
		}
		if conn.HandshakeComplete {
			t.Error("Expected handshake to be incomplete")
		}
	})

	t.Run("Connection with metadata", func(t *testing.T) {
		metadata := map[string]string{
			"region": "us-east-1",
			"env":    "production",
		}

		conn := &PeerConnection{
			ID:       "peer-002",
			Type:     PeerTypeIncoming,
			Metadata: metadata,
		}

		if len(conn.Metadata) != 2 {
			t.Errorf("Expected 2 metadata entries, got %d", len(conn.Metadata))
		}
		if conn.Metadata["region"] != "us-east-1" {
			t.Error("Metadata not set correctly")
		}
	})

	t.Run("Connection with peer info", func(t *testing.T) {
		peerInfo := &PeerInfo{
			PeerID:       "remote-peer",
			Metadata:     make(map[string]string),
			Capabilities: make(map[string]string),
			CompletedAt:  time.Now(),
		}

		conn := &PeerConnection{
			ID:                "peer-003",
			Type:              PeerTypeOutgoing,
			PeerInfo:          peerInfo,
			HandshakeComplete: true,
		}

		if conn.PeerInfo.PeerID != "remote-peer" {
			t.Error("PeerInfo not set correctly")
		}
		if !conn.HandshakeComplete {
			t.Error("Expected handshake to be complete")
		}
	})
}

func TestPeerInfoStruct(t *testing.T) {
	t.Run("Create peer info", func(t *testing.T) {
		info := &PeerInfo{
			PeerID:       "peer-001",
			Metadata:     make(map[string]string),
			Capabilities: make(map[string]string),
			CompletedAt:  time.Now(),
		}

		if info.PeerID != "peer-001" {
			t.Errorf("Expected PeerID 'peer-001', got %s", info.PeerID)
		}
		if info.Metadata == nil {
			t.Error("Metadata should be initialized")
		}
		if info.Capabilities == nil {
			t.Error("Capabilities should be initialized")
		}
	})

	t.Run("Peer info with metadata", func(t *testing.T) {
		info := &PeerInfo{
			PeerID: "peer-002",
			Metadata: map[string]string{
				"version": "1.0.0",
				"type":    "processor",
			},
			Capabilities: map[string]string{
				"matrix_ops": "true",
				"websocket":  "true",
			},
			CompletedAt: time.Now(),
		}

		if len(info.Metadata) != 2 {
			t.Errorf("Expected 2 metadata entries, got %d", len(info.Metadata))
		}
		if len(info.Capabilities) != 2 {
			t.Errorf("Expected 2 capabilities, got %d", len(info.Capabilities))
		}
		if info.Metadata["version"] != "1.0.0" {
			t.Error("Metadata not set correctly")
		}
	})
}

func TestPeerStats(t *testing.T) {
	t.Run("Create peer stats", func(t *testing.T) {
		stats := &PeerStats{
			TotalPeers:          5,
			IncomingConnections: 3,
			OutgoingConnections: 2,
			MessagesReceived:    100,
			MessagesSent:        50,
			HandshakesComplete:  4,
			HandshakesPending:   1,
		}

		if stats.TotalPeers != 5 {
			t.Errorf("Expected 5 total peers, got %d", stats.TotalPeers)
		}
		if stats.IncomingConnections != 3 {
			t.Errorf("Expected 3 incoming, got %d", stats.IncomingConnections)
		}
		if stats.HandshakesComplete != 4 {
			t.Errorf("Expected 4 complete handshakes, got %d", stats.HandshakesComplete)
		}
	})

	t.Run("Update stats", func(t *testing.T) {
		stats := &PeerStats{}

		stats.TotalPeers++
		stats.MessagesReceived += 10
		stats.MessagesSent += 5

		if stats.TotalPeers != 1 {
			t.Error("Stats not incremented correctly")
		}
		if stats.MessagesReceived != 10 {
			t.Error("Messages received not incremented")
		}
	})
}
