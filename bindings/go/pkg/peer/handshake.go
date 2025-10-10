package peer

import (
	"context"
	"fmt"
	"time"

	"github.com/hivellm/umicp-go/pkg/umicp"
)

// sendHello sends a HELLO message to initiate handshake
func (p *Peer) sendHello(ctx context.Context, peerConn *PeerConnection) error {
	hello, err := umicp.NewEnvelope().
		From(p.config.PeerID).
		To("*").
		Operation(umicp.OperationControl).
		MessageID(fmt.Sprintf("hello-%s-%d", peerConn.ID, time.Now().UnixNano())).
		Capability("type", "hello").
		Capability("peer_id", p.config.PeerID).
		Capability("version", "1.0").
		Build()

	if err != nil {
		return err
	}

	// Add metadata to hello
	for k, v := range p.config.Metadata {
		hello.Capabilities[k] = v
	}

	if err := p.SendToPeer(ctx, peerConn.ID, hello); err != nil {
		return err
	}

	// Start handshake timeout
	go p.waitForHandshake(peerConn)

	return nil
}

// waitForHandshake waits for handshake completion or timeout
func (p *Peer) waitForHandshake(peerConn *PeerConnection) {
	timer := time.NewTimer(p.config.HandshakeTimeout)
	defer timer.Stop()

	<-timer.C

	p.mu.RLock()
	completed := peerConn.HandshakeComplete
	p.mu.RUnlock()

	if !completed {
		p.eventBus.Emit(&Event{
			Type:       EventHandshakeTimeout,
			Connection: peerConn,
		})
	}
}

// handleControlMessage handles control messages
func (p *Peer) handleControlMessage(env *umicp.Envelope, peerConn *PeerConnection) {
	msgType := env.Capabilities["type"]

	switch msgType {
	case "hello":
		p.handleHello(env, peerConn)
	default:
		// Unknown control message type
	}
}

// handleHello handles HELLO messages
func (p *Peer) handleHello(env *umicp.Envelope, peerConn *PeerConnection) {
	// Extract peer info from HELLO
	peerInfo := &PeerInfo{
		PeerID:       env.Capabilities["peer_id"],
		Metadata:     make(map[string]string),
		Capabilities: make(map[string]string),
		CompletedAt:  time.Now(),
	}

	// Copy all capabilities
	for k, v := range env.Capabilities {
		peerInfo.Capabilities[k] = v
	}

	p.mu.Lock()
	peerConn.PeerInfo = peerInfo
	p.mu.Unlock()

	// Send ACK response
	ctx := context.Background()
	ack, _ := umicp.NewEnvelope().
		From(p.config.PeerID).
		To(peerInfo.PeerID).
		Operation(umicp.OperationAck).
		MessageID(fmt.Sprintf("ack-%s", env.MessageID)).
		Capability("type", "hello_ack").
		Capability("peer_id", p.config.PeerID).
		Build()

	// Add metadata to ack
	for k, v := range p.config.Metadata {
		ack.Capabilities[k] = v
	}

	p.SendToPeer(ctx, peerConn.ID, ack)

	// Mark handshake complete
	p.mu.Lock()
	peerConn.HandshakeComplete = true
	p.mu.Unlock()

	// Emit peer ready event
	p.eventBus.Emit(&Event{
		Type:       EventPeerReady,
		Connection: peerConn,
		PeerInfo:   peerInfo,
	})
}

// handleAckMessage handles ACK messages
func (p *Peer) handleAckMessage(env *umicp.Envelope, peerConn *PeerConnection) {
	msgType := env.Capabilities["type"]

	if msgType == "hello_ack" {
		// Extract peer info from ACK
		peerInfo := &PeerInfo{
			PeerID:       env.Capabilities["peer_id"],
			Metadata:     make(map[string]string),
			Capabilities: make(map[string]string),
			CompletedAt:  time.Now(),
		}

		// Copy all capabilities
		for k, v := range env.Capabilities {
			peerInfo.Capabilities[k] = v
		}

		p.mu.Lock()
		peerConn.PeerInfo = peerInfo
		peerConn.HandshakeComplete = true
		p.mu.Unlock()

		// Emit events
		p.eventBus.Emit(&Event{
			Type:       EventPeerReady,
			Connection: peerConn,
			PeerInfo:   peerInfo,
		})

		p.eventBus.Emit(&Event{
			Type:       EventConnected,
			Connection: peerConn,
		})
	}
}
