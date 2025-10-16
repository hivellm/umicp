/*!
# Basic WebSocket Client-Server Example

Demonstrates basic WebSocket communication using UMICP Rust bindings.
*/

#[cfg(feature = "websocket")]
use umicp_core::{Envelope, OperationType, WebSocketClient, WebSocketServer};
#[cfg(feature = "websocket")]
use std::time::Duration;
#[cfg(feature = "websocket")]
use tokio::time::sleep;

#[cfg(feature = "websocket")]
#[tokio::main]
async fn main() -> anyhow::Result<()> {
    // Initialize tracing
    tracing_subscriber::fmt::init();

    println!("🚀 UMICP Rust WebSocket Example");
    println!("================================\n");

    // Start server in background
    let _server_handle = tokio::spawn(async move {
        run_server().await
    });

    // Wait for server to start
    sleep(Duration::from_secs(1)).await;

    // Run client
    run_client().await?;

    // Let server run a bit longer
    sleep(Duration::from_secs(2)).await;

    println!("\n✅ Example completed successfully!");

    Ok(())
}

async fn run_server() -> anyhow::Result<()> {
    let mut server = WebSocketServer::new("127.0.0.1:20081")?;

    println!("📡 Server: Starting on 127.0.0.1:20081");

    // Start server (this blocks)
    server.start().await?;

    Ok(())
}

async fn run_client() -> anyhow::Result<()> {
    println!("📱 Client: Connecting to ws://127.0.0.1:20081");

    let client = WebSocketClient::new("ws://127.0.0.1:20081");

    // Connect to server
    client.connect().await?;

    println!("✅ Client: Connected!");

    // Wait for connection to stabilize
    sleep(Duration::from_millis(100)).await;

    // Send test message
    println!("📤 Client: Sending test message");

    let envelope = Envelope::builder()
        .from("rust-client")
        .to("rust-server")
        .operation(OperationType::Data)
        .message_id("msg-001")
        .capability("message", "Hello from Rust client!")
        .capability("test", "basic-websocket")
        .build()?;

    client.send(envelope).await?;

    println!("✅ Client: Message sent!");

    // Send another message
    sleep(Duration::from_millis(100)).await;

    let envelope2 = Envelope::builder()
        .from("rust-client")
        .to("rust-server")
        .operation(OperationType::Data)
        .message_id("msg-002")
        .capability("message", "Second message!")
        .capability("sequence", "2")
        .build()?;

    client.send(envelope2).await?;

    println!("✅ Client: Second message sent!");

    // Get stats
    sleep(Duration::from_millis(100)).await;

    let stats = client.get_stats();
    println!("\n📊 Client Statistics:");
    println!("   Messages sent: {}", stats.messages_sent);
    println!("   Bytes sent: {}", stats.bytes_sent);
    println!("   Connect count: {}", stats.connect_count);

    // Disconnect
    println!("\n👋 Client: Disconnecting...");
    client.disconnect().await?;

    println!("✅ Client: Disconnected!");

    Ok(())
}

#[cfg(not(feature = "websocket"))]
fn main() {
    eprintln!("This example requires the 'websocket' feature.");
    eprintln!("Run with: cargo run --example websocket_basic --features websocket");
}
