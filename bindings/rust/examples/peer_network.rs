/*!
# Peer Network Example

Demonstrates WebSocketPeer with multiplexed connections.
*/

#[cfg(feature = "websocket")]
use std::sync::Arc;
#[cfg(feature = "websocket")]
use tokio::time::{sleep, Duration};
#[cfg(feature = "websocket")]
use umicp_core::{Envelope, OperationType, WebSocketPeer, WebSocketPeerConfig};

#[cfg(feature = "websocket")]
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    tracing_subscriber::fmt::init();

    println!("🌐 Creating peer network...\n");

    // Create peer 1 (server + client)
    let mut config1 = WebSocketPeerConfig::default();
    config1.peer_id = "peer-1".to_string();
    config1.server_addr = Some("127.0.0.1:9001".parse()?);

    let mut peer1 = WebSocketPeer::new(config1);

    peer1.set_message_handler(Arc::new(|envelope, from_peer| {
        println!("[Peer1] 📨 Message from {}: {} -> {}",
            from_peer, envelope.from(), envelope.to());
    }));

    peer1.set_connect_handler(Arc::new(|peer_id, info| {
        println!("[Peer1] ✅ Peer connected: {} (type: {:?})", peer_id, info.peer_type);
    }));

    peer1.set_disconnect_handler(Arc::new(|peer_id, _info| {
        println!("[Peer1] ❌ Peer disconnected: {}", peer_id);
    }));

    // Start server
    println!("🚀 Starting Peer1 server on 127.0.0.1:9001...");
    peer1.start_server().await?;
    println!("✓ Peer1 server started\n");

    sleep(Duration::from_millis(500)).await;

    // Create peer 2 (client only)
    let mut config2 = WebSocketPeerConfig::default();
    config2.peer_id = "peer-2".to_string();
    config2.server_addr = None; // Client only

    let mut peer2 = WebSocketPeer::new(config2);

    peer2.set_message_handler(Arc::new(|envelope, from_peer| {
        println!("[Peer2] 📨 Message from {}: {} -> {}",
            from_peer, envelope.from(), envelope.to());
    }));

    // Connect peer2 to peer1
    println!("🔌 Connecting Peer2 to Peer1...");
    let peer1_id = peer2.connect_to_peer("ws://127.0.0.1:9001").await?;
    println!("✓ Peer2 connected to Peer1 (ID: {})\n", peer1_id);

    sleep(Duration::from_secs(1)).await;

    // Send message from peer2 to peer1
    println!("📤 Peer2 sending message to Peer1...");
    let envelope = Envelope::builder()
        .from("peer-2")
        .to("peer-1")
        .operation(OperationType::Data)
        .capability("message", "Hello from Peer2!")
        .build()?;

    peer2.send_to_peer(&peer1_id, envelope).await?;
    println!("✓ Message sent\n");

    sleep(Duration::from_secs(1)).await;

    // Broadcast from peer1
    println!("📡 Peer1 broadcasting to all peers...");
    let broadcast_msg = Envelope::builder()
        .from("peer-1")
        .to("all")
        .operation(OperationType::Data)
        .capability("type", "broadcast")
        .capability("message", "Hello everyone!")
        .build()?;

    peer1.broadcast(broadcast_msg).await?;
    println!("✓ Broadcast sent\n");

    sleep(Duration::from_secs(1)).await;

    // Show peer statistics
    println!("📊 Network Status:");
    println!("   Peer1 connections: {}", peer1.peer_count());
    println!("   Peer2 connections: {}", peer2.peer_count());

    println!("\n📋 Peer1 connected peers:");
    for info in peer1.get_all_peer_info() {
        println!("   - {} (type: {:?}, uptime: {}s)",
            info.id, info.peer_type, info.uptime());
    }

    // Cleanup
    println!("\n🛑 Shutting down...");
    peer2.shutdown().await?;
    peer1.shutdown().await?;
    println!("✓ All peers shutdown");

    Ok(())
}

