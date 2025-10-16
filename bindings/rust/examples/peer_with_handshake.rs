/*!
# Peer Network with Auto-Handshake Example

Demonstrates WebSocketPeer with auto-handshake and message handling.
*/

#[cfg(feature = "websocket")]
use std::sync::Arc;
#[cfg(feature = "websocket")]
use tokio::time::{sleep, Duration};
#[cfg(feature = "websocket")]
use umicp_core::{Envelope, OperationType, WebSocketPeer, WebSocketPeerConfig, WebSocketClient};
#[cfg(feature = "websocket")]
use serde_json::json;

#[cfg(feature = "websocket")]
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    tracing_subscriber::fmt::init();

    println!("🌐 Testing Auto-Handshake and Message Handlers\n");

    // Create peer 1 (server with handshake)
    let mut config1 = WebSocketPeerConfig::default();
    config1.peer_id = "server-peer".to_string();
    config1.server_addr = Some("127.0.0.1:9002".parse()?);
    config1.auto_handshake = true;  // Enable auto-handshake
    config1.handshake_timeout = 5;  // 5 seconds timeout

    let mut peer1 = WebSocketPeer::new(config1);

    // Add capabilities and metadata to server
    peer1.add_capability("storage");
    peer1.add_capability("compute");
    peer1.add_metadata("datacenter", "us-east");

    peer1.set_message_handler(Arc::new(|envelope, from_peer| {
        println!("[Server] 📨 Message from {}", from_peer);
        println!("         From: {} -> To: {}", envelope.from(), envelope.to());
        if let Some(caps) = envelope.capabilities() {
            for (k, v) in caps.iter() {
                println!("         {}: {}", k, v);
            }
        }
    }));

    peer1.set_connect_handler(Arc::new(|peer_id, info| {
        println!("[Server] ✅ Peer connected: {}", peer_id);
        println!("         Type: {:?}", info.peer_type);
        if !info.capabilities.is_empty() {
            println!("         Capabilities: {:?}", info.capabilities);
        }
    }));

    // Start server
    println!("🚀 Starting server on 127.0.0.1:9002...");
    peer1.start_server().await?;
    println!("✓ Server started\n");

    sleep(Duration::from_secs(1)).await;

    // Create standalone client with message handler
    println!("🔌 Creating client with receive handler...");
    let client = Arc::new(WebSocketClient::new("ws://127.0.0.1:9002"));

    client.set_message_handler(Arc::new(|envelope| {
        println!("[Client] 📬 Received message!");
        println!("         From: {} -> To: {}", envelope.from(), envelope.to());
        if let Some(caps) = envelope.capabilities() {
            for (k, v) in caps.iter() {
                println!("         {}: {}", k, v);
            }
        }
    }));

    client.connect().await?;
    println!("✓ Client connected\n");

    // Send test messages
    println!("📤 Sending messages...\n");

    for i in 1..=3 {
        let msg_content = format!("Message #{} with receive handler", i);
        let envelope = Envelope::builder()
            .from("standalone-client")
            .to("server-peer")
            .operation(OperationType::Data)
            .capability("message", json!(msg_content))
            .capability("sequence", json!(i))
            .build()?;

        client.send(envelope).await?;
        println!("[{}] Sent message #{}", chrono::Utc::now().format("%H:%M:%S"), i);

        sleep(Duration::from_millis(500)).await;
    }

    sleep(Duration::from_secs(2)).await;

    // Create peer 2 with auto-handshake
    println!("\n🔌 Connecting Peer2 with auto-handshake...");
    let mut config2 = WebSocketPeerConfig::default();
    config2.peer_id = "client-peer".to_string();
    config2.server_addr = None;
    config2.auto_handshake = true;

    let mut peer2 = WebSocketPeer::new(config2);
    peer2.add_capability("processing");
    peer2.add_metadata("region", "eu-west");

    let peer1_id = peer2.connect_to_peer("ws://127.0.0.1:9002").await?;
    println!("✓ Peer2 connected (handshaking...)\n");

    // Wait for handshake
    sleep(Duration::from_secs(2)).await;

    // Send from peer2
    let envelope = Envelope::builder()
        .from("client-peer")
        .to("server-peer")
        .operation(OperationType::Data)
        .capability("message", json!("Hello from peer2 after handshake!"))
        .build()?;

    peer2.send_to_peer(&peer1_id, envelope).await?;
    println!("📤 Peer2 sent message after handshake\n");

    sleep(Duration::from_secs(2)).await;

    // Show peer info
    println!("📊 Network Status:");
    println!("   Server peers: {}", peer1.peer_count());
    for info in peer1.get_all_peer_info() {
        println!("   - Peer: {}", info.id);
        println!("     Type: {:?}", info.peer_type);
        println!("     Capabilities: {:?}", info.capabilities);
        if let Some(metadata) = info.metadata.get("region").or(info.metadata.get("datacenter")) {
            println!("     Location: {}", metadata);
        }
        println!("     Uptime: {}s", info.uptime());
    }

    // Cleanup
    println!("\n🛑 Shutting down...");
    client.disconnect().await?;
    peer2.shutdown().await?;
    peer1.shutdown().await?;
    println!("✓ All connections closed");

    Ok(())
}

#[cfg(not(feature = "websocket"))]
fn main() {
    eprintln!("This example requires the 'websocket' feature.");
    eprintln!("Run with: cargo run --example peer_with_handshake --features websocket");
}

