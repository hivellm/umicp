package peer

import (
	"sync"
	"testing"
	"time"

	"github.com/hivellm/umicp-go/pkg/umicp"
)

func TestEventType(t *testing.T) {
	t.Run("All event types", func(t *testing.T) {
		eventTypes := []EventType{
			EventMessage,
			EventData,
			EventPeerConnect,
			EventPeerDisconnect,
			EventPeerReady,
			EventHandshakeTimeout,
			EventError,
			EventServerReady,
			EventConnected,
		}

		for _, et := range eventTypes {
			if et.String() == "" {
				t.Errorf("EventType %v has empty string", et)
			}
		}
	})

	t.Run("Event type strings", func(t *testing.T) {
		if EventMessage.String() != "message" {
			t.Error("EventMessage string incorrect")
		}
		if EventPeerConnect.String() != "peer:connect" {
			t.Error("EventPeerConnect string incorrect")
		}
		if EventError.String() != "error" {
			t.Error("EventError string incorrect")
		}
	})
}

func TestEvent(t *testing.T) {
	t.Run("Create message event", func(t *testing.T) {
		env, _ := umicp.NewEnvelope().
			From("test").
			To("target").
			Operation(umicp.OperationData).
			Build()

		event := &Event{
			Type:     EventMessage,
			Envelope: env,
		}

		if event.Type != EventMessage {
			t.Error("Event type not set correctly")
		}
		if event.Envelope == nil {
			t.Error("Envelope should not be nil")
		}
	})

	t.Run("Create peer connect event", func(t *testing.T) {
		conn := &PeerConnection{
			ID:   "peer-001",
			Type: PeerTypeIncoming,
		}

		event := &Event{
			Type:       EventPeerConnect,
			Connection: conn,
		}

		if event.Connection == nil {
			t.Error("Connection should not be nil")
		}
		if event.Connection.ID != "peer-001" {
			t.Error("Connection ID incorrect")
		}
	})

	t.Run("Create error event", func(t *testing.T) {
		event := &Event{
			Type:  EventError,
			Error: &GenericError{Message: "test error"},
		}

		if event.Error == nil {
			t.Error("Error should not be nil")
		}
	})
}

func TestEventBus(t *testing.T) {
	t.Run("Create event bus", func(t *testing.T) {
		bus := NewEventBus()
		defer bus.Close()

		if bus == nil {
			t.Fatal("Event bus should not be nil")
		}
		if bus.handlers == nil {
			t.Error("Handlers map should be initialized")
		}
	})

	t.Run("Register handler", func(t *testing.T) {
		bus := NewEventBus()
		defer bus.Close()

		handler := func(e *Event) {
			// Handler function
		}

		bus.On(EventMessage, handler)

		// Verify handler was registered
		bus.mu.RLock()
		handlers := bus.handlers[EventMessage]
		bus.mu.RUnlock()

		if len(handlers) != 1 {
			t.Errorf("Expected 1 handler, got %d", len(handlers))
		}
	})

	t.Run("Multiple handlers for same event", func(t *testing.T) {
		bus := NewEventBus()
		defer bus.Close()

		count := 0
		var mu sync.Mutex

		handler1 := func(e *Event) {
			mu.Lock()
			count++
			mu.Unlock()
		}

		handler2 := func(e *Event) {
			mu.Lock()
			count++
			mu.Unlock()
		}

		bus.On(EventMessage, handler1)
		bus.On(EventMessage, handler2)

		// Emit event
		event := &Event{Type: EventMessage}
		bus.Emit(event)

		// Give time for handlers to run
		time.Sleep(100 * time.Millisecond)

		mu.Lock()
		finalCount := count
		mu.Unlock()

		if finalCount != 2 {
			t.Errorf("Expected 2 handler calls, got %d", finalCount)
		}
	})

	t.Run("Different event types", func(t *testing.T) {
		bus := NewEventBus()
		defer bus.Close()

		messageReceived := false
		errorReceived := false

		bus.On(EventMessage, func(e *Event) {
			messageReceived = true
		})

		bus.On(EventError, func(e *Event) {
			errorReceived = true
		})

		// Emit message event
		bus.Emit(&Event{Type: EventMessage})
		time.Sleep(50 * time.Millisecond)

		if !messageReceived {
			t.Error("Message handler not called")
		}
		if errorReceived {
			t.Error("Error handler should not be called")
		}

		// Emit error event
		bus.Emit(&Event{Type: EventError})
		time.Sleep(50 * time.Millisecond)

		if !errorReceived {
			t.Error("Error handler not called")
		}
	})

	t.Run("Event bus close", func(t *testing.T) {
		bus := NewEventBus()
		bus.Close()

		// Should not panic
		select {
		case <-bus.closeCh:
			// Channel closed as expected
		default:
			t.Error("Close channel should be closed")
		}
	})

	t.Run("Emit when buffer full", func(t *testing.T) {
		bus := NewEventBus()
		defer bus.Close()

		// Fill the buffer (capacity is 1000)
		for i := 0; i < 1001; i++ {
			bus.Emit(&Event{Type: EventMessage})
		}

		// Should not block or panic
	})
}
