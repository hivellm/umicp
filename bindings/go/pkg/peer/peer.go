package peer

import (
	"context"
	"fmt"
	"sync"
	"time"

	"github.com/google/uuid"
	"github.com/hivellm/umicp-go/pkg/transport/websocket"
	"github.com/hivellm/umicp-go/pkg/umicp"
)

// PeerConfig contains peer configuration
type PeerConfig struct {
	PeerID           string
	AutoProtocol     bool
	HandshakeTimeout time.Duration
	Metadata         map[string]string
}

// ServerConfig contains server configuration for peer
type ServerConfig struct {
	Addr        string
	Path        string
	Compression bool
}

// Peer represents a multiplexed peer (server + multiple clients)
type Peer struct {
	config   PeerConfig
	server   *websocket.Server
	peers    sync.Map // map[string]*PeerConnection
	eventBus *EventBus
	mu       sync.RWMutex
}

// NewPeer creates a new peer without server
func NewPeer(config PeerConfig) *Peer {
	if config.PeerID == "" {
		config.PeerID = uuid.New().String()
	}
	if config.HandshakeTimeout == 0 {
		config.HandshakeTimeout = 10 * time.Second
	}
	if config.Metadata == nil {
		config.Metadata = make(map[string]string)
	}

	return &Peer{
		config:   config,
		eventBus: NewEventBus(),
	}
}

// NewPeerWithServer creates a new peer with server component
func NewPeerWithServer(config PeerConfig, serverConfig ServerConfig) (*Peer, error) {
	peer := NewPeer(config)

	// Create server
	wsConfig := websocket.ServerConfig{
		Addr:        serverConfig.Addr,
		Path:        serverConfig.Path,
		Compression: serverConfig.Compression,
	}
	server := websocket.NewServer(wsConfig)

	// Setup server event handlers
	server.OnConnect(func(conn *websocket.Connection) {
		peerConn := &PeerConnection{
			ID:          conn.ID,
			Type:        PeerTypeIncoming,
			ConnectedAt: time.Now(),
			Metadata:    make(map[string]string),
		}
		peer.peers.Store(conn.ID, peerConn)

		peer.eventBus.Emit(&Event{
			Type:       EventPeerConnect,
			Connection: peerConn,
		})

		// Auto handshake for incoming
		if peer.config.AutoProtocol {
			go peer.waitForHelloFromIncoming(peerConn)
		}
	})

	server.OnDisconnect(func(conn *websocket.Connection) {
		if val, ok := peer.peers.Load(conn.ID); ok {
			peerConn := val.(*PeerConnection)
			peer.peers.Delete(conn.ID)

			peer.eventBus.Emit(&Event{
				Type:       EventPeerDisconnect,
				Connection: peerConn,
			})
		}
	})

	server.OnMessage(func(env *umicp.Envelope, conn *websocket.Connection) {
		if val, ok := peer.peers.Load(conn.ID); ok {
			peerConn := val.(*PeerConnection)
			peer.handleMessage(env, peerConn)
		}
	})

	peer.server = server
	return peer, nil
}

// Start starts the peer (and server if present)
func (p *Peer) Start(ctx context.Context) error {
	if p.server != nil {
		go func() {
			if err := p.server.Start(ctx); err != nil {
				p.eventBus.Emit(&Event{
					Type:  EventError,
					Error: err,
				})
			}
		}()

		p.eventBus.Emit(&Event{
			Type: EventServerReady,
		})
	}
	return nil
}

// ConnectToPeer connects to a remote peer
func (p *Peer) ConnectToPeer(ctx context.Context, url string, metadata map[string]string) (string, error) {
	clientConfig := websocket.DefaultClientConfig()
	clientConfig.URL = url

	client := websocket.NewClient(*clientConfig)

	if err := client.Connect(ctx); err != nil {
		return "", err
	}

	peerID := uuid.New().String()
	peerConn := &PeerConnection{
		ID:          peerID,
		Type:        PeerTypeOutgoing,
		URL:         url,
		Client:      client,
		Metadata:    metadata,
		ConnectedAt: time.Now(),
	}

	// Setup client event handlers
	client.OnMessage(func(env *umicp.Envelope) {
		p.handleMessage(env, peerConn)
	})

	client.OnDisconnect(func(err error) {
		p.peers.Delete(peerID)
		p.eventBus.Emit(&Event{
			Type:       EventPeerDisconnect,
			Connection: peerConn,
			Error:      err,
		})
	})

	p.peers.Store(peerID, peerConn)

	p.eventBus.Emit(&Event{
		Type:       EventPeerConnect,
		Connection: peerConn,
	})

	// Auto handshake for outgoing
	if p.config.AutoProtocol {
		if err := p.sendHello(ctx, peerConn); err != nil {
			return "", err
		}
	}

	return peerID, nil
}

// SendToPeer sends an envelope to a specific peer
func (p *Peer) SendToPeer(ctx context.Context, peerID string, env *umicp.Envelope) error {
	val, ok := p.peers.Load(peerID)
	if !ok {
		return umicp.ErrPeerNotFound
	}

	peerConn := val.(*PeerConnection)

	if peerConn.Type == PeerTypeOutgoing {
		return peerConn.Client.Send(ctx, env)
	} else {
		// Incoming connection - send via server
		if p.server != nil {
			return p.server.Send(ctx, peerConn.ID, env)
		}
		return fmt.Errorf("no server available for incoming connection")
	}
}

// Broadcast sends an envelope to all peers
func (p *Peer) Broadcast(ctx context.Context, env *umicp.Envelope, excludePeerID string) int {
	count := 0
	p.peers.Range(func(key, val interface{}) bool {
		peerID := key.(string)
		if peerID != excludePeerID {
			if err := p.SendToPeer(ctx, peerID, env); err == nil {
				count++
			}
		}
		return true
	})
	return count
}

// BroadcastToType sends to peers of a specific type
func (p *Peer) BroadcastToType(ctx context.Context, env *umicp.Envelope, peerType PeerType, excludePeerID string) int {
	count := 0
	p.peers.Range(func(key, val interface{}) bool {
		peerID := key.(string)
		peerConn := val.(*PeerConnection)
		if peerID != excludePeerID && peerConn.Type == peerType {
			if err := p.SendToPeer(ctx, peerID, env); err == nil {
				count++
			}
		}
		return true
	})
	return count
}

// GetPeers returns all peer connections
func (p *Peer) GetPeers() []*PeerConnection {
	var peers []*PeerConnection
	p.peers.Range(func(key, val interface{}) bool {
		peers = append(peers, val.(*PeerConnection))
		return true
	})
	return peers
}

// GetPeersByType returns peers of a specific type
func (p *Peer) GetPeersByType(peerType PeerType) []*PeerConnection {
	var peers []*PeerConnection
	p.peers.Range(func(key, val interface{}) bool {
		peer := val.(*PeerConnection)
		if peer.Type == peerType {
			peers = append(peers, peer)
		}
		return true
	})
	return peers
}

// GetStats returns peer statistics
func (p *Peer) GetStats() PeerStats {
	stats := PeerStats{}
	p.peers.Range(func(key, val interface{}) bool {
		peer := val.(*PeerConnection)
		stats.TotalPeers++
		if peer.Type == PeerTypeIncoming {
			stats.IncomingConnections++
		} else {
			stats.OutgoingConnections++
		}
		if peer.HandshakeComplete {
			stats.HandshakesComplete++
		} else {
			stats.HandshakesPending++
		}
		return true
	})
	return stats
}

// On registers an event handler
func (p *Peer) On(eventType EventType, handler EventHandler) {
	p.eventBus.On(eventType, handler)
}

// Shutdown shuts down the peer and all connections
func (p *Peer) Shutdown(ctx context.Context) error {
	// Close all peer connections
	p.peers.Range(func(key, val interface{}) bool {
		peer := val.(*PeerConnection)
		if peer.Type == PeerTypeOutgoing && peer.Client != nil {
			peer.Client.Disconnect(ctx)
		}
		return true
	})

	// Close event bus
	p.eventBus.Close()

	return nil
}

// handleMessage handles incoming messages
func (p *Peer) handleMessage(env *umicp.Envelope, peerConn *PeerConnection) {
	// Emit raw message event
	p.eventBus.Emit(&Event{
		Type:       EventMessage,
		Envelope:   env,
		Connection: peerConn,
	})

	// Handle protocol messages
	switch env.Operation {
	case umicp.OperationControl:
		p.handleControlMessage(env, peerConn)
	case umicp.OperationAck:
		p.handleAckMessage(env, peerConn)
	case umicp.OperationData:
		p.eventBus.Emit(&Event{
			Type:       EventData,
			Envelope:   env,
			Connection: peerConn,
		})
	case umicp.OperationError:
		p.eventBus.Emit(&Event{
			Type:       EventError,
			Envelope:   env,
			Connection: peerConn,
		})
	}
}

// waitForHelloFromIncoming waits for HELLO from incoming connection
func (p *Peer) waitForHelloFromIncoming(peerConn *PeerConnection) {
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
