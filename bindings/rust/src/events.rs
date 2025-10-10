/*!
# Event System

EventEmitter pattern for UMICP with multiple subscribers.
*/

use crate::Envelope;
#[cfg(feature = "websocket")]
use crate::peer::PeerInfo;
use parking_lot::RwLock;
use std::collections::HashMap;
use std::sync::Arc;

/// Event types
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum EventType {
    /// Message received
    Message,
    /// Peer connected
    PeerConnect,
    /// Peer disconnected
    PeerDisconnect,
    /// Handshake completed
    HandshakeComplete,
    /// Connection error
    Error,
    /// Connection state changed
    StateChange,
}

/// Event data
#[derive(Debug, Clone)]
pub enum EventData {
    /// Message event
    Message {
        envelope: Envelope,
        peer_id: String,
    },
    /// Peer connection event
    #[cfg(feature = "websocket")]
    PeerConnect {
        peer_id: String,
        info: PeerInfo,
    },
    /// Peer disconnection event
    #[cfg(feature = "websocket")]
    PeerDisconnect {
        peer_id: String,
        info: PeerInfo,
    },
    /// Handshake complete event
    #[cfg(feature = "websocket")]
    HandshakeComplete {
        peer_id: String,
        info: PeerInfo,
    },
    /// Error event
    Error {
        message: String,
        peer_id: Option<String>,
    },
    /// State change event
    StateChange {
        peer_id: String,
        from: String,
        to: String,
    },
}

/// Event listener callback
pub type EventListener = Arc<dyn Fn(EventData) + Send + Sync>;

/// Event emitter with multiple subscribers
pub struct EventEmitter {
    listeners: Arc<RwLock<HashMap<EventType, Vec<EventListener>>>>,
}

impl EventEmitter {
    /// Create new event emitter
    pub fn new() -> Self {
        Self {
            listeners: Arc::new(RwLock::new(HashMap::new())),
        }
    }

    /// Register event listener
    pub fn on(&self, event_type: EventType, listener: EventListener) {
        self.listeners
            .write()
            .entry(event_type)
            .or_insert_with(Vec::new)
            .push(listener);
    }

    /// Remove all listeners for an event type
    pub fn off(&self, event_type: EventType) {
        self.listeners.write().remove(&event_type);
    }

    /// Emit event to all listeners
    pub fn emit(&self, event_type: EventType, data: EventData) {
        if let Some(listeners) = self.listeners.read().get(&event_type) {
            for listener in listeners {
                listener(data.clone());
            }
        }
    }

    /// Emit event asynchronously
    pub async fn emit_async(&self, event_type: EventType, data: EventData) {
        let listeners = self.listeners.read().get(&event_type).cloned();

        if let Some(listeners) = listeners {
            for listener in listeners {
                let data_clone = data.clone();
                // Call listener synchronously since we're not using async runtime
                listener(data_clone);
            }
        }
    }

    /// Register one-time listener (fires once then removes itself)
    pub fn once(&self, event_type: EventType, listener: EventListener) {
        let listeners = Arc::clone(&self.listeners);
        let event_type_clone = event_type;

        let wrapped_listener: EventListener = Arc::new(move |data| {
            listener(data);
            // Remove this listener after firing
            listeners.write().get_mut(&event_type_clone).map(|v| {
                v.retain(|l| !Arc::ptr_eq(l, &listener));
            });
        });

        self.on(event_type, wrapped_listener);
    }

    /// Get listener count for event type
    pub fn listener_count(&self, event_type: EventType) -> usize {
        self.listeners
            .read()
            .get(&event_type)
            .map(|v| v.len())
            .unwrap_or(0)
    }

    /// Remove all listeners
    pub fn remove_all_listeners(&self) {
        self.listeners.write().clear();
    }

    /// Get all registered event types
    pub fn event_types(&self) -> Vec<EventType> {
        self.listeners.read().keys().copied().collect()
    }
}

impl Default for EventEmitter {
    fn default() -> Self {
        Self::new()
    }
}

impl Clone for EventEmitter {
    fn clone(&self) -> Self {
        Self {
            listeners: Arc::clone(&self.listeners),
        }
    }
}

/// Convenience methods for common events
impl EventEmitter {
    /// Emit message event
    pub fn emit_message(&self, envelope: Envelope, peer_id: String) {
        self.emit(
            EventType::Message,
            EventData::Message { envelope, peer_id },
        );
    }

    /// Emit peer connect event
    #[cfg(feature = "websocket")]
    pub fn emit_peer_connect(&self, peer_id: String, info: PeerInfo) {
        self.emit(
            EventType::PeerConnect,
            EventData::PeerConnect { peer_id, info },
        );
    }

    /// Emit peer disconnect event
    #[cfg(feature = "websocket")]
    pub fn emit_peer_disconnect(&self, peer_id: String, info: PeerInfo) {
        self.emit(
            EventType::PeerDisconnect,
            EventData::PeerDisconnect { peer_id, info },
        );
    }

    /// Emit handshake complete event
    #[cfg(feature = "websocket")]
    pub fn emit_handshake_complete(&self, peer_id: String, info: PeerInfo) {
        self.emit(
            EventType::HandshakeComplete,
            EventData::HandshakeComplete { peer_id, info },
        );
    }

    /// Emit error event
    pub fn emit_error(&self, message: String, peer_id: Option<String>) {
        self.emit(EventType::Error, EventData::Error { message, peer_id });
    }

    /// Emit state change event
    pub fn emit_state_change(&self, peer_id: String, from: String, to: String) {
        self.emit(
            EventType::StateChange,
            EventData::StateChange { peer_id, from, to },
        );
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::Duration;

    #[test]
    fn test_event_emitter_creation() {
        let emitter = EventEmitter::new();
        assert_eq!(emitter.listener_count(EventType::Message), 0);
    }

    #[test]
    fn test_register_listener() {
        let emitter = EventEmitter::new();
        let listener = Arc::new(|_data: EventData| {});

        emitter.on(EventType::Message, listener);
        assert_eq!(emitter.listener_count(EventType::Message), 1);
    }

    #[test]
    fn test_multiple_listeners() {
        let emitter = EventEmitter::new();

        emitter.on(EventType::Message, Arc::new(|_| {}));
        emitter.on(EventType::Message, Arc::new(|_| {}));
        emitter.on(EventType::PeerConnect, Arc::new(|_| {}));

        assert_eq!(emitter.listener_count(EventType::Message), 2);
        assert_eq!(emitter.listener_count(EventType::PeerConnect), 1);
    }

    #[test]
    fn test_remove_listeners() {
        let emitter = EventEmitter::new();

        emitter.on(EventType::Message, Arc::new(|_| {}));
        emitter.on(EventType::Message, Arc::new(|_| {}));

        emitter.off(EventType::Message);
        assert_eq!(emitter.listener_count(EventType::Message), 0);
    }

    #[test]
    fn test_emit_event() {
        let emitter = EventEmitter::new();
        let called = Arc::new(RwLock::new(false));
        let called_clone = Arc::clone(&called);

        emitter.on(
            EventType::Error,
            Arc::new(move |_data| {
                *called_clone.write() = true;
            }),
        );

        emitter.emit_error("Test error".to_string(), None);

        assert!(*called.read());
    }

    #[cfg(feature = "websocket")]
    #[tokio::test]
    async fn test_emit_async() {
        let emitter = EventEmitter::new();
        let called = Arc::new(RwLock::new(0));
        let called_clone = Arc::clone(&called);

        emitter.on(
            EventType::Message,
            Arc::new(move |_data| {
                *called_clone.write() += 1;
            }),
        );

        emitter
            .emit_async(
                EventType::Message,
                EventData::Error {
                    message: "test".to_string(),
                    peer_id: None,
                },
            )
            .await;

        tokio::time::sleep(Duration::from_millis(100)).await;

        assert_eq!(*called.read(), 1);
    }
}

