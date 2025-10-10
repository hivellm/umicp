/*!
# Peer System Integration Tests
*/

use parking_lot::RwLock;
use std::sync::Arc;
use tokio::time::{sleep, Duration};
use umicp_core::{
    ConnectionState, Envelope, HandshakeMessage, HandshakeProtocol, HandshakeType,
    OperationType, PeerConnection, PeerInfo, PeerType, WebSocketPeer, WebSocketPeerConfig,
};

#[tokio::test]
async fn test_peer_connection_send() {
    let (tx, mut rx) = tokio::sync::mpsc::unbounded_channel();
    let conn = PeerConnection::new("peer-1", tx);
    
    // Set as connected
    conn.set_state(ConnectionState::Connected);
    
    let envelope = Envelope::builder()
        .from("sender")
        .to("receiver")
        .operation(OperationType::Data)
        .build()
        .unwrap();
    
    // Send should succeed
    assert!(conn.send(envelope).is_ok());
    
    // Should receive message
    assert!(rx.try_recv().is_ok());
    
    let stats = conn.stats();
    assert_eq!(stats.messages_sent, 1);
}

#[tokio::test]
async fn test_peer_connection_not_connected() {
    let (tx, _rx) = tokio::sync::mpsc::unbounded_channel();
    let conn = PeerConnection::new("peer-1", tx);
    
    // Should be in Connecting state
    assert!(!conn.is_connected());
    
    let envelope = Envelope::builder()
        .from("sender")
        .to("receiver")
        .operation(OperationType::Data)
        .build()
        .unwrap();
    
    // Send should fail
    assert!(conn.send(envelope).is_err());
}

#[tokio::test]
async fn test_peer_info_builder() {
    let peer = PeerInfo::client("peer-1", "ws://localhost:8080")
        .with_capability("storage")
        .with_capability("compute")
        .with_metadata("region", "us-east")
        .with_metadata("datacenter", "dc1")
        .with_version("1.0.0");
    
    assert_eq!(peer.id, "peer-1");
    assert_eq!(peer.peer_type, PeerType::Client);
    assert_eq!(peer.capabilities.len(), 2);
    assert!(peer.has_capability("storage"));
    assert!(peer.has_capability("compute"));
    assert_eq!(peer.get_metadata("region"), Some(&"us-east".to_string()));
    assert_eq!(peer.version, Some("1.0.0".to_string()));
}

#[tokio::test]
async fn test_handshake_hello_ack_flow() {
    let protocol = HandshakeProtocol::new("peer-1");
    
    // Create HELLO
    let hello = protocol.create_hello().unwrap();
    assert!(HandshakeMessage::is_handshake(&hello));
    
    // Parse HELLO
    let hello_msg = HandshakeMessage::from_envelope(&hello).unwrap();
    assert_eq!(hello_msg.msg_type, HandshakeType::Hello);
    assert_eq!(hello_msg.peer_id, "peer-1");
    
    // Create ACK
    let ack = protocol.create_ack(&hello_msg).unwrap();
    
    // Parse ACK
    let ack_msg = HandshakeMessage::from_envelope(&ack).unwrap();
    assert_eq!(ack_msg.msg_type, HandshakeType::Ack);
    assert_eq!(ack_msg.peer_id, "peer-1");
}

#[tokio::test]
async fn test_handshake_with_capabilities() {
    let mut protocol = HandshakeProtocol::new("peer-1");
    protocol.add_capability("storage");
    protocol.add_capability("compute");
    protocol.add_metadata("region", "us-east");
    
    let hello = protocol.create_hello().unwrap();
    let hello_msg = HandshakeMessage::from_envelope(&hello).unwrap();
    
    assert_eq!(hello_msg.capabilities.len(), 2);
    assert!(hello_msg.capabilities.contains(&"storage".to_string()));
    assert!(hello_msg.capabilities.contains(&"compute".to_string()));
    assert_eq!(hello_msg.metadata.get("region"), Some(&"us-east".to_string()));
}

#[tokio::test]
async fn test_handshake_to_peer_info() {
    let msg = HandshakeMessage::hello("peer-1")
        .with_capability("storage")
        .with_metadata("region", "us-west")
        .with_version("1.2.0");
    
    let peer_info = msg.to_peer_info();
    
    assert_eq!(peer_info.id, "peer-1");
    assert!(peer_info.has_capability("storage"));
    assert_eq!(peer_info.get_metadata("region"), Some(&"us-west".to_string()));
    assert_eq!(peer_info.version, Some("1.2.0".to_string()));
}

#[tokio::test]
async fn test_websocket_peer_creation() {
    let config = WebSocketPeerConfig {
        peer_id: "test-peer".to_string(),
        server_addr: None, // Client only
        auto_handshake: true,
        handshake_timeout: 5,
    };
    
    let peer = WebSocketPeer::new(config);
    assert_eq!(peer.peer_count(), 0);
}

#[tokio::test]
async fn test_websocket_peer_handlers() {
    let mut config = WebSocketPeerConfig::default();
    config.server_addr = None; // Client only
    
    let mut peer = WebSocketPeer::new(config);
    
    let msg_received: Arc<RwLock<bool>> = Arc::new(RwLock::new(false));
    let msg_clone = Arc::clone(&msg_received);
    
    peer.set_message_handler(Arc::new(move |_env, _from| {
        *msg_clone.write() = true;
    }));
    
    let conn_received: Arc<RwLock<bool>> = Arc::new(RwLock::new(false));
    let conn_clone = Arc::clone(&conn_received);
    
    peer.set_connect_handler(Arc::new(move |_id, _info| {
        *conn_clone.write() = true;
    }));
    
    // Handlers are set (can't test firing without actual server)
    assert!(!*msg_received.read());
    assert!(!*conn_received.read());
}

#[tokio::test]
async fn test_peer_find_by_metadata() {
    // Create multiple peer infos
    let mut peer_map = std::collections::HashMap::new();
    
    peer_map.insert(
        "peer-1".to_string(),
        PeerInfo::client("peer-1", "ws://server1")
            .with_metadata("region", "us-east")
            .with_capability("storage"),
    );
    
    peer_map.insert(
        "peer-2".to_string(),
        PeerInfo::client("peer-2", "ws://server2")
            .with_metadata("region", "us-west")
            .with_capability("compute"),
    );
    
    peer_map.insert(
        "peer-3".to_string(),
        PeerInfo::client("peer-3", "ws://server3")
            .with_metadata("region", "us-east")
            .with_capability("storage"),
    );
    
    // Find by region
    let us_east_peers: Vec<_> = peer_map
        .iter()
        .filter(|(_, info)| info.get_metadata("region") == Some(&"us-east".to_string()))
        .map(|(id, _)| id.clone())
        .collect();
    
    assert_eq!(us_east_peers.len(), 2);
    assert!(us_east_peers.contains(&"peer-1".to_string()));
    assert!(us_east_peers.contains(&"peer-3".to_string()));
}

#[tokio::test]
async fn test_peer_find_by_capability() {
    let mut peer_map = std::collections::HashMap::new();
    
    peer_map.insert(
        "peer-1".to_string(),
        PeerInfo::client("peer-1", "ws://server1")
            .with_capability("storage"),
    );
    
    peer_map.insert(
        "peer-2".to_string(),
        PeerInfo::client("peer-2", "ws://server2")
            .with_capability("compute"),
    );
    
    peer_map.insert(
        "peer-3".to_string(),
        PeerInfo::client("peer-3", "ws://server3")
            .with_capability("storage"),
    );
    
    // Find storage peers
    let storage_peers: Vec<_> = peer_map
        .iter()
        .filter(|(_, info)| info.has_capability("storage"))
        .map(|(id, _)| id.clone())
        .collect();
    
    assert_eq!(storage_peers.len(), 2);
}

#[tokio::test]
async fn test_connection_state_transitions() {
    let (tx, _rx) = tokio::sync::mpsc::unbounded_channel();
    let conn = PeerConnection::new("peer-1", tx);
    
    // Initial state
    assert_eq!(conn.state(), ConnectionState::Connecting);
    
    // Transition to handshaking
    conn.set_state(ConnectionState::Handshaking);
    assert_eq!(conn.state(), ConnectionState::Handshaking);
    
    // Transition to connected
    conn.set_state(ConnectionState::Connected);
    assert!(conn.is_connected());
    
    // Transition to disconnecting
    conn.set_state(ConnectionState::Disconnecting);
    assert!(!conn.is_connected());
    
    // Transition to disconnected
    conn.set_state(ConnectionState::Disconnected);
    assert_eq!(conn.state(), ConnectionState::Disconnected);
}

#[tokio::test]
async fn test_peer_uptime() {
    let peer_info = PeerInfo::client("peer-1", "ws://localhost:8080");
    
    // Should have minimal uptime
    let uptime1 = peer_info.uptime();
    
    sleep(Duration::from_millis(100)).await;
    
    // Uptime should have increased
    let uptime2 = peer_info.uptime();
    assert!(uptime2 >= uptime1);
}

#[tokio::test]
async fn test_connection_stats_tracking() {
    let (tx, _rx) = tokio::sync::mpsc::unbounded_channel();
    let conn = PeerConnection::new("peer-1", tx);
    
    // Record some received messages
    conn.record_received(100);
    conn.record_received(200);
    conn.record_received(300);
    
    let stats = conn.stats();
    assert_eq!(stats.messages_received, 3);
    assert_eq!(stats.bytes_received, 600);
}

