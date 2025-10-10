package websocket

import (
	"context"
	"net/http"
	"sync"
	"time"

	"github.com/google/uuid"
	"github.com/gorilla/websocket"
	"github.com/hivellm/umicp-go/pkg/transport"
	"github.com/hivellm/umicp-go/pkg/umicp"
)

// ServerConfig contains WebSocket server configuration
type ServerConfig struct {
	Addr            string
	Path            string
	MaxPayload      int64
	Compression     bool
	ReadBufferSize  int
	WriteBufferSize int
}

// DefaultServerConfig returns default server configuration
func DefaultServerConfig() *ServerConfig {
	return &ServerConfig{
		Addr:            "127.0.0.1:8080",
		Path:            "/ws",
		MaxPayload:      100 * 1024 * 1024, // 100MB
		Compression:     false,
		ReadBufferSize:  4096,
		WriteBufferSize: 4096,
	}
}

// Connection represents a client connection
type Connection struct {
	ID      string
	Conn    *websocket.Conn
	SendCh  chan *umicp.Envelope
	closeCh chan struct{}
}

// Server represents a WebSocket server
type Server struct {
	config  ServerConfig
	server  *http.Server
	clients sync.Map // map[string]*Connection

	upgrader websocket.Upgrader

	// Stats
	stats *transport.Stats
	mu    sync.RWMutex

	// Callbacks
	onMessage    func(*umicp.Envelope, *Connection)
	onConnect    func(*Connection)
	onDisconnect func(*Connection)
	onError      func(error)
}

// NewServer creates a new WebSocket server
func NewServer(config ServerConfig) *Server {
	if config.MaxPayload == 0 {
		config.MaxPayload = 100 * 1024 * 1024
	}
	if config.ReadBufferSize == 0 {
		config.ReadBufferSize = 4096
	}
	if config.WriteBufferSize == 0 {
		config.WriteBufferSize = 4096
	}
	if config.Path == "" {
		config.Path = "/ws"
	}

	return &Server{
		config: config,
		upgrader: websocket.Upgrader{
			ReadBufferSize:  config.ReadBufferSize,
			WriteBufferSize: config.WriteBufferSize,
			CheckOrigin: func(r *http.Request) bool {
				return true // TODO: Configure properly
			},
		},
		stats: &transport.Stats{
			ConnectedAt: time.Now(),
		},
	}
}

// Start starts the WebSocket server
func (s *Server) Start(ctx context.Context) error {
	mux := http.NewServeMux()
	mux.HandleFunc(s.config.Path, s.handleWebSocket)

	s.server = &http.Server{
		Addr:    s.config.Addr,
		Handler: mux,
	}

	go func() {
		<-ctx.Done()
		s.server.Shutdown(context.Background())
	}()

	return s.server.ListenAndServe()
}

// Send sends an envelope to a specific client
func (s *Server) Send(ctx context.Context, clientID string, env *umicp.Envelope) error {
	val, ok := s.clients.Load(clientID)
	if !ok {
		return umicp.ErrPeerNotFound
	}

	client := val.(*Connection)
	select {
	case client.SendCh <- env:
		return nil
	case <-ctx.Done():
		return ctx.Err()
	case <-client.closeCh:
		return umicp.ErrConnectionClosed
	}
}

// Broadcast sends an envelope to all connected clients
func (s *Server) Broadcast(ctx context.Context, env *umicp.Envelope) int {
	count := 0
	s.clients.Range(func(key, val interface{}) bool {
		client := val.(*Connection)
		select {
		case client.SendCh <- env:
			count++
		default:
			// Skip if buffer is full
		}
		return true
	})
	return count
}

// GetClients returns all connected clients
func (s *Server) GetClients() []*Connection {
	var clients []*Connection
	s.clients.Range(func(key, val interface{}) bool {
		clients = append(clients, val.(*Connection))
		return true
	})
	return clients
}

// Stats returns transport statistics
func (s *Server) Stats() *transport.Stats {
	s.mu.RLock()
	defer s.mu.RUnlock()

	statsCopy := *s.stats
	statsCopy.Uptime = time.Since(s.stats.ConnectedAt)

	return &statsCopy
}

// OnMessage sets the message handler
func (s *Server) OnMessage(fn func(*umicp.Envelope, *Connection)) {
	s.onMessage = fn
}

// OnConnect sets the connect handler
func (s *Server) OnConnect(fn func(*Connection)) {
	s.onConnect = fn
}

// OnDisconnect sets the disconnect handler
func (s *Server) OnDisconnect(fn func(*Connection)) {
	s.onDisconnect = fn
}

// OnError sets the error handler
func (s *Server) OnError(fn func(error)) {
	s.onError = fn
}

// handleWebSocket handles WebSocket upgrade and connection
func (s *Server) handleWebSocket(w http.ResponseWriter, r *http.Request) {
	conn, err := s.upgrader.Upgrade(w, r, nil)
	if err != nil {
		if s.onError != nil {
			s.onError(err)
		}
		return
	}

	clientID := uuid.New().String()
	client := &Connection{
		ID:      clientID,
		Conn:    conn,
		SendCh:  make(chan *umicp.Envelope, 100),
		closeCh: make(chan struct{}),
	}

	s.clients.Store(clientID, client)

	if s.onConnect != nil {
		s.onConnect(client)
	}

	go s.readLoop(client)
	go s.writeLoop(client)
}

// readLoop reads messages from a client connection
func (s *Server) readLoop(client *Connection) {
	defer func() {
		s.clients.Delete(client.ID)
		client.Conn.Close()
		close(client.closeCh)

		if s.onDisconnect != nil {
			s.onDisconnect(client)
		}
	}()

	client.Conn.SetReadLimit(s.config.MaxPayload)

	for {
		_, data, err := client.Conn.ReadMessage()
		if err != nil {
			if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
				if s.onError != nil {
					s.onError(err)
				}
			}
			return
		}

		env, err := umicp.DeserializeEnvelope(data)
		if err != nil {
			if s.onError != nil {
				s.onError(err)
			}
			s.mu.Lock()
			s.stats.Errors++
			s.mu.Unlock()
			continue
		}

		s.mu.Lock()
		s.stats.MessagesReceived++
		s.stats.BytesReceived += int64(len(data))
		s.stats.LastMessageAt = time.Now()
		s.mu.Unlock()

		if s.onMessage != nil {
			go s.onMessage(env, client)
		}
	}
}

// writeLoop writes messages to a client connection
func (s *Server) writeLoop(client *Connection) {
	ticker := time.NewTicker(30 * time.Second)
	defer ticker.Stop()

	for {
		select {
		case <-client.closeCh:
			return
		case env := <-client.SendCh:
			data, err := env.Serialize()
			if err != nil {
				if s.onError != nil {
					s.onError(err)
				}
				s.mu.Lock()
				s.stats.Errors++
				s.mu.Unlock()
				continue
			}

			if err := client.Conn.WriteMessage(websocket.BinaryMessage, data); err != nil {
				if s.onError != nil {
					s.onError(err)
				}
				return
			}

			s.mu.Lock()
			s.stats.MessagesSent++
			s.stats.BytesSent += int64(len(data))
			s.mu.Unlock()

		case <-ticker.C:
			if err := client.Conn.WriteMessage(websocket.PingMessage, nil); err != nil {
				return
			}
		}
	}
}
