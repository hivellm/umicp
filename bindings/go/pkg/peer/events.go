package peer

import (
	"sync"

	"github.com/hivellm/umicp-go/pkg/umicp"
)

// EventType defines the type of peer event
type EventType string

const (
	// EventMessage fires when any message is received
	EventMessage EventType = "message"

	// EventData fires when a data message is received
	EventData EventType = "data"

	// EventPeerConnect fires when a peer connects
	EventPeerConnect EventType = "peer:connect"

	// EventPeerDisconnect fires when a peer disconnects
	EventPeerDisconnect EventType = "peer:disconnect"

	// EventPeerReady fires when handshake completes
	EventPeerReady EventType = "peer:ready"

	// EventHandshakeTimeout fires when handshake times out
	EventHandshakeTimeout EventType = "handshake:timeout"

	// EventError fires when an error occurs
	EventError EventType = "error"

	// EventServerReady fires when server starts
	EventServerReady EventType = "server:ready"

	// EventConnected fires after successful connection
	EventConnected EventType = "connected"
)

// Event represents a peer event
type Event struct {
	Type       EventType
	Envelope   *umicp.Envelope
	Connection *PeerConnection
	PeerInfo   *PeerInfo
	Error      error
	Data       interface{}
}

// EventHandler is a function that handles events
type EventHandler func(*Event)

// EventBus manages event subscriptions and emissions
type EventBus struct {
	handlers map[EventType][]EventHandler
	mu       sync.RWMutex
	eventCh  chan *Event
	closeCh  chan struct{}
}

// NewEventBus creates a new event bus
func NewEventBus() *EventBus {
	bus := &EventBus{
		handlers: make(map[EventType][]EventHandler),
		eventCh:  make(chan *Event, 1000),
		closeCh:  make(chan struct{}),
	}
	go bus.processEvents()
	return bus
}

// On registers an event handler
func (eb *EventBus) On(eventType EventType, handler EventHandler) {
	eb.mu.Lock()
	defer eb.mu.Unlock()
	eb.handlers[eventType] = append(eb.handlers[eventType], handler)
}

// Emit sends an event to all registered handlers
func (eb *EventBus) Emit(event *Event) {
	select {
	case eb.eventCh <- event:
	default:
		// Event buffer full, drop event
	}
}

// Close closes the event bus
func (eb *EventBus) Close() {
	close(eb.closeCh)
}

// processEvents processes events from the channel
func (eb *EventBus) processEvents() {
	for {
		select {
		case <-eb.closeCh:
			return
		case event := <-eb.eventCh:
			eb.mu.RLock()
			handlers, ok := eb.handlers[event.Type]
			eb.mu.RUnlock()

			if ok {
				for _, handler := range handlers {
					go handler(event) // Run handlers concurrently
				}
			}
		}
	}
}

// String returns string representation of event type
func (et EventType) String() string {
	return string(et)
}
