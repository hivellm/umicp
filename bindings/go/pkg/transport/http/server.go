package http

import (
	"context"
	"fmt"
	"io"
	"net/http"
	"sync"
	"sync/atomic"
	"time"

	"github.com/hivellm/umicp-go/pkg/transport"
	"github.com/hivellm/umicp-go/pkg/umicp"
	"golang.org/x/net/http2"
	"golang.org/x/net/http2/h2c"
)

// ServerConfig contains HTTP/2 server configuration
type ServerConfig struct {
	Addr           string
	Path           string
	ReadTimeout    time.Duration
	WriteTimeout   time.Duration
	MaxHeaderBytes int
}

// DefaultServerConfig returns default HTTP server configuration
func DefaultServerConfig() *ServerConfig {
	return &ServerConfig{
		Addr:           "127.0.0.1:8080",
		Path:           "/",
		ReadTimeout:    30 * time.Second,
		WriteTimeout:   30 * time.Second,
		MaxHeaderBytes: 1 << 20, // 1MB
	}
}

// Server represents an HTTP/2 streaming server
type Server struct {
	config  ServerConfig
	server  *http.Server
	handler func(*umicp.Envelope) (*umicp.Envelope, error)
	mu      sync.RWMutex
	stats   *transport.Stats
	running int32    // atomic boolean
	conns   sync.Map // map[string]*Connection
}

// NewServer creates a new HTTP/2 server
func NewServer(config ServerConfig) *Server {
	if config.ReadTimeout == 0 {
		config.ReadTimeout = 30 * time.Second
	}
	if config.WriteTimeout == 0 {
		config.WriteTimeout = 30 * time.Second
	}
	if config.MaxHeaderBytes == 0 {
		config.MaxHeaderBytes = 1 << 20
	}
	if config.Path == "" {
		config.Path = "/"
	}

	return &Server{
		config: config,
		stats: &transport.Stats{
			ConnectedAt: time.Now(),
		},
	}
}

// OnRequest sets the request handler
func (s *Server) OnRequest(handler func(*umicp.Envelope) (*umicp.Envelope, error)) {
	s.handler = handler
}

// Start starts the HTTP/2 server
func (s *Server) Start(ctx context.Context) error {
	if s.IsRunning() {
		return fmt.Errorf("server already running")
	}

	h2s := &http2.Server{}

	mux := http.NewServeMux()
	mux.HandleFunc(s.config.Path, s.handleRequest)

	s.server = &http.Server{
		Addr:           s.config.Addr,
		Handler:        h2c.NewHandler(mux, h2s),
		ReadTimeout:    s.config.ReadTimeout,
		WriteTimeout:   s.config.WriteTimeout,
		MaxHeaderBytes: s.config.MaxHeaderBytes,
	}

	atomic.StoreInt32(&s.running, 1)

	go func() {
		if err := s.server.ListenAndServe(); err != http.ErrServerClosed {
			atomic.StoreInt32(&s.running, 0)
		}
	}()

	go func() {
		<-ctx.Done()
		s.Stop(context.Background())
	}()

	return nil
}

// Stop gracefully stops the HTTP/2 server
func (s *Server) Stop(ctx context.Context) error {
	if !s.IsRunning() {
		return nil
	}

	atomic.StoreInt32(&s.running, 0)

	if s.server != nil {
		return s.server.Shutdown(ctx)
	}

	return nil
}

// IsRunning returns true if server is running
func (s *Server) IsRunning() bool {
	return atomic.LoadInt32(&s.running) == 1
}

// handleRequest handles incoming HTTP requests
func (s *Server) handleRequest(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	data, err := io.ReadAll(r.Body)
	if err != nil {
		http.Error(w, fmt.Sprintf("Read error: %v", err), http.StatusBadRequest)
		s.mu.Lock()
		s.stats.Errors++
		s.mu.Unlock()
		return
	}

	env, err := umicp.DeserializeEnvelope(data)
	if err != nil {
		http.Error(w, fmt.Sprintf("Deserialization error: %v", err), http.StatusBadRequest)
		s.mu.Lock()
		s.stats.Errors++
		s.mu.Unlock()
		return
	}

	s.mu.Lock()
	s.stats.MessagesReceived++
	s.stats.BytesReceived += int64(len(data))
	s.stats.LastMessageAt = time.Now()
	s.mu.Unlock()

	if s.handler != nil {
		resp, err := s.handler(env)
		if err != nil {
			http.Error(w, fmt.Sprintf("Handler error: %v", err), http.StatusInternalServerError)
			s.mu.Lock()
			s.stats.Errors++
			s.mu.Unlock()
			return
		}

		respData, err := resp.Serialize()
		if err != nil {
			http.Error(w, fmt.Sprintf("Serialization error: %v", err), http.StatusInternalServerError)
			s.mu.Lock()
			s.stats.Errors++
			s.mu.Unlock()
			return
		}

		s.mu.Lock()
		s.stats.MessagesSent++
		s.stats.BytesSent += int64(len(respData))
		s.mu.Unlock()

		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(http.StatusOK)
		w.Write(respData)
	} else {
		w.WriteHeader(http.StatusNoContent)
	}
}

// Stats returns transport statistics
func (s *Server) Stats() *transport.Stats {
	s.mu.RLock()
	defer s.mu.RUnlock()

	statsCopy := *s.stats
	statsCopy.Uptime = time.Since(s.stats.ConnectedAt)

	return &statsCopy
}

// GetConnections returns the number of active connections
func (s *Server) GetConnections() int {
	count := 0
	s.conns.Range(func(key, value interface{}) bool {
		count++
		return true
	})
	return count
}
