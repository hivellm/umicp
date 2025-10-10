package peer

import (
	"time"

	"github.com/hivellm/umicp-go/pkg/transport/websocket"
)

// PeerType defines the type of peer connection
type PeerType string

const (
	// PeerTypeIncoming represents an incoming connection
	PeerTypeIncoming PeerType = "incoming"

	// PeerTypeOutgoing represents an outgoing connection
	PeerTypeOutgoing PeerType = "outgoing"
)

// PeerConnection represents a connection to another peer
type PeerConnection struct {
	ID                string
	Type              PeerType
	URL               string // For outgoing connections
	Client            *websocket.Client
	Metadata          map[string]string
	ConnectedAt       time.Time
	HandshakeComplete bool
	PeerInfo          *PeerInfo
}

// PeerInfo contains information about a peer after handshake
type PeerInfo struct {
	PeerID       string
	Metadata     map[string]string
	Capabilities map[string]string
	CompletedAt  time.Time
}

// PeerStats contains statistics about peer connections
type PeerStats struct {
	TotalPeers          int
	IncomingConnections int
	OutgoingConnections int
	MessagesReceived    int64
	MessagesSent        int64
	HandshakesComplete  int
	HandshakesPending   int
}

// String returns string representation of peer type
func (pt PeerType) String() string {
	return string(pt)
}
