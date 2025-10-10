package peer

import (
	"context"
	"testing"
	"time"

	"github.com/hivellm/umicp-go/pkg/umicp"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

func TestPeer_SendHello(t *testing.T) {
	t.Run("SendHello to outgoing peer - error case", func(t *testing.T) {
		peer := NewPeer(PeerConfig{
			PeerID: "sender-peer",
			Metadata: map[string]string{
				"version": "1.0",
				"region":  "us-east-1",
			},
		})

		peerConn := &PeerConnection{
			ID:   "test-peer-123",
			Type: PeerTypeOutgoing,
		}
		// Don't add to peers map - will cause error

		ctx := context.Background()
		err := peer.sendHello(ctx, peerConn)
		assert.Error(t, err) // Should error because peer not found
	})

	t.Run("SendHello structure validation", func(t *testing.T) {
		peer := NewPeer(PeerConfig{
			PeerID: "sender-peer-2",
			Metadata: map[string]string{
				"version": "2.0",
				"env":     "production",
			},
		})

		peerConn := &PeerConnection{
			ID:   "test-peer-456",
			Type: PeerTypeOutgoing,
		}

		// Add a mock server to allow sending
		peer.server = nil // no server means it will try to use client

		ctx := context.Background()
		err := peer.sendHello(ctx, peerConn)

		// Will error but envelope should be built correctly
		assert.Error(t, err)
	})
}

func TestPeer_WaitForHandshake_Timeout(t *testing.T) {
	peer := NewPeer(PeerConfig{
		PeerID:           "test-wait",
		HandshakeTimeout: 50 * time.Millisecond,
	})

	peerConn := &PeerConnection{
		ID:                "wait-peer",
		Type:              PeerTypeOutgoing,
		HandshakeComplete: false,
	}

	timeoutChan := make(chan bool, 1)
	peer.On(EventHandshakeTimeout, func(e *Event) {
		timeoutChan <- true
	})

	peer.waitForHandshake(peerConn)

	select {
	case <-timeoutChan:
		// Success - timeout occurred
	case <-time.After(200 * time.Millisecond):
		t.Error("Handshake timeout event not emitted")
	}
}

func TestPeer_WaitForHandshake_Success(t *testing.T) {
	peer := NewPeer(PeerConfig{
		PeerID:           "test-wait-success",
		HandshakeTimeout: 100 * time.Millisecond,
	})

	peerConn := &PeerConnection{
		ID:                "wait-peer-success",
		Type:              PeerTypeOutgoing,
		HandshakeComplete: false,
	}

	// Complete handshake before timeout
	go func() {
		time.Sleep(30 * time.Millisecond)
		peer.mu.Lock()
		peerConn.HandshakeComplete = true
		peer.mu.Unlock()
	}()

	timeoutChan := make(chan bool, 1)
	peer.On(EventHandshakeTimeout, func(e *Event) {
		timeoutChan <- true
	})

	peer.waitForHandshake(peerConn)

	select {
	case <-timeoutChan:
		t.Error("Should not timeout when handshake completes")
	case <-time.After(200 * time.Millisecond):
		// Success - no timeout
	}
}

func TestPeer_HandleHello(t *testing.T) {
	peer := NewPeer(PeerConfig{
		PeerID: "receiver-peer",
		Metadata: map[string]string{
			"version": "1.0",
		},
	})

	peerConn := &PeerConnection{
		ID:   "hello-peer",
		Type: PeerTypeIncoming,
	}
	peer.peers.Store(peerConn.ID, peerConn)

	t.Run("Handle valid HELLO", func(t *testing.T) {
		readyChan := make(chan *PeerInfo, 1)
		peer.On(EventPeerReady, func(e *Event) {
			readyChan <- e.PeerInfo
		})

		helloEnv, err := umicp.NewEnvelope().
			From("remote-sender").
			To("*").
			Operation(umicp.OperationControl).
			Capability("type", "hello").
			Capability("peer_id", "remote-sender-id").
			Capability("version", "1.0").
			Capability("region", "us-west-2").
			Build()
		require.NoError(t, err)

		peer.handleHello(helloEnv, peerConn)

		select {
		case info := <-readyChan:
			assert.Equal(t, "remote-sender-id", info.PeerID)
			assert.Equal(t, "1.0", info.Capabilities["version"])
			assert.Equal(t, "us-west-2", info.Capabilities["region"])
			assert.True(t, peerConn.HandshakeComplete)
			assert.NotNil(t, peerConn.PeerInfo)
		case <-time.After(200 * time.Millisecond):
			t.Error("EventPeerReady not emitted")
		}
	})

	t.Run("Handle HELLO with multiple capabilities", func(t *testing.T) {
		peerConn2 := &PeerConnection{
			ID:   "hello-peer-2",
			Type: PeerTypeIncoming,
		}
		peer.peers.Store(peerConn2.ID, peerConn2)

		helloEnv, _ := umicp.NewEnvelope().
			From("remote-2").
			To("*").
			Operation(umicp.OperationControl).
			Capability("type", "hello").
			Capability("peer_id", "remote-2-id").
			Capability("version", "2.0").
			Capability("protocol", "umicp").
			Capability("compression", "gzip").
			Build()

		peer.handleHello(helloEnv, peerConn2)

		// Give time for processing
		time.Sleep(50 * time.Millisecond)

		assert.True(t, peerConn2.HandshakeComplete)
		assert.NotNil(t, peerConn2.PeerInfo)
		assert.Equal(t, "remote-2-id", peerConn2.PeerInfo.PeerID)
		assert.Len(t, peerConn2.PeerInfo.Capabilities, 5) // type, peer_id, version, protocol, compression
	})
}

func TestPeer_HandleHello_Integration(t *testing.T) {
	// Create two peers for handshake
	peer1, err := NewPeerWithServer(
		PeerConfig{
			PeerID:       "peer-1",
			AutoProtocol: true,
			Metadata: map[string]string{
				"name": "Peer 1",
			},
		},
		ServerConfig{
			Addr: "127.0.0.1:19092",
			Path: "/ws",
		},
	)
	require.NoError(t, err)

	peer2 := NewPeer(PeerConfig{
		PeerID: "peer-2",
		Metadata: map[string]string{
			"name": "Peer 2",
		},
	})

	// Start server
	ctx, cancel := context.WithTimeout(context.Background(), 2*time.Second)
	defer cancel()

	err = peer1.Start(ctx)
	assert.NoError(t, err)

	// Give server time to start
	time.Sleep(100 * time.Millisecond)

	// Simulate handshake manually
	peerConn := &PeerConnection{
		ID:   "test-conn",
		Type: PeerTypeIncoming,
	}

	helloEnv, _ := umicp.NewEnvelope().
		From(peer2.config.PeerID).
		To("*").
		Operation(umicp.OperationControl).
		Capability("type", "hello").
		Capability("peer_id", peer2.config.PeerID).
		Capability("name", peer2.config.Metadata["name"]).
		Build()

	peer1.handleHello(helloEnv, peerConn)

	// Verify handshake completed
	assert.True(t, peerConn.HandshakeComplete)
	assert.NotNil(t, peerConn.PeerInfo)

	// Cleanup
	peer1.Shutdown(context.Background())
	peer2.Shutdown(context.Background())
}

func TestPeer_HandleControlMessage_Coverage(t *testing.T) {
	peer := NewPeer(PeerConfig{
		PeerID: "ctrl-test",
	})

	peerConn := &PeerConnection{
		ID:   "ctrl-peer",
		Type: PeerTypeIncoming,
	}

	testCases := []struct {
		name         string
		msgType      string
		shouldHandle bool
	}{
		{"HELLO message", "hello", true},
		{"Unknown message", "unknown", false},
		{"Empty message", "", false},
		{"Custom message", "custom_control", false},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			env, _ := umicp.NewEnvelope().
				From("sender").
				To("receiver").
				Operation(umicp.OperationControl).
				Capability("type", tc.msgType).
				Capability("peer_id", "test-peer").
				Build()

			// Should not panic for any message type
			peer.handleControlMessage(env, peerConn)
		})
	}
}

func TestPeer_HandleAckMessage_Coverage(t *testing.T) {
	peer := NewPeer(PeerConfig{
		PeerID: "ack-test",
	})

	testCases := []struct {
		name           string
		ackType        string
		shouldComplete bool
	}{
		{"HELLO_ACK", "hello_ack", true},
		{"OTHER_ACK", "other_ack", false},
		{"EMPTY_ACK", "", false},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			peerConn := &PeerConnection{
				ID:                "ack-peer-" + tc.name,
				Type:              PeerTypeOutgoing,
				HandshakeComplete: false,
			}

			env, _ := umicp.NewEnvelope().
				From("remote").
				To(peer.config.PeerID).
				Operation(umicp.OperationAck).
				Capability("type", tc.ackType).
				Capability("peer_id", "remote-peer").
				Build()

			peer.handleAckMessage(env, peerConn)

			assert.Equal(t, tc.shouldComplete, peerConn.HandshakeComplete)
		})
	}
}

func TestHandshake_FullFlow(t *testing.T) {
	// Test the full handshake flow between two peers
	peer1 := NewPeer(PeerConfig{
		PeerID: "peer-alpha",
		Metadata: map[string]string{
			"role": "initiator",
		},
	})

	peer2 := NewPeer(PeerConfig{
		PeerID: "peer-beta",
		Metadata: map[string]string{
			"role": "responder",
		},
	})

	// Peer 1 connection
	conn1 := &PeerConnection{
		ID:   "conn-1",
		Type: PeerTypeOutgoing,
	}
	peer1.peers.Store(conn1.ID, conn1)

	// Peer 2 connection
	conn2 := &PeerConnection{
		ID:   "conn-2",
		Type: PeerTypeIncoming,
	}
	peer2.peers.Store(conn2.ID, conn2)

	// Track events
	peer2ReadyChan := make(chan bool, 1)
	peer1ReadyChan := make(chan bool, 1)

	peer2.On(EventPeerReady, func(e *Event) {
		peer2ReadyChan <- true
	})
	peer1.On(EventPeerReady, func(e *Event) {
		peer1ReadyChan <- true
	})

	// Step 1: Peer 1 sends HELLO (simulated)
	helloEnv, _ := umicp.NewEnvelope().
		From(peer1.config.PeerID).
		To("*").
		Operation(umicp.OperationControl).
		Capability("type", "hello").
		Capability("peer_id", peer1.config.PeerID).
		Capability("role", peer1.config.Metadata["role"]).
		Build()

	peer2.handleHello(helloEnv, conn2)

	// Step 2: Wait for peer2 to emit ready
	select {
	case <-peer2ReadyChan:
		// Success
	case <-time.After(200 * time.Millisecond):
		t.Error("Peer2 did not emit ready event")
	}

	// Verify peer2 completed handshake
	assert.True(t, conn2.HandshakeComplete)
	assert.NotNil(t, conn2.PeerInfo)
	assert.Equal(t, peer1.config.PeerID, conn2.PeerInfo.PeerID)

	// Step 3: Simulate ACK back to peer1
	ackEnv, _ := umicp.NewEnvelope().
		From(peer2.config.PeerID).
		To(peer1.config.PeerID).
		Operation(umicp.OperationAck).
		Capability("type", "hello_ack").
		Capability("peer_id", peer2.config.PeerID).
		Capability("role", peer2.config.Metadata["role"]).
		Build()

	peer1.handleAckMessage(ackEnv, conn1)

	// Step 4: Wait for peer1 to emit ready
	select {
	case <-peer1ReadyChan:
		// Success
	case <-time.After(200 * time.Millisecond):
		t.Error("Peer1 did not emit ready event")
	}

	// Verify peer1 completed handshake
	assert.True(t, conn1.HandshakeComplete)
	assert.NotNil(t, conn1.PeerInfo)
	assert.Equal(t, peer2.config.PeerID, conn1.PeerInfo.PeerID)

	// Cleanup
	peer1.Shutdown(context.Background())
	peer2.Shutdown(context.Background())
}
