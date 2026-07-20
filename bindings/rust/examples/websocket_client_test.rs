/*!
# WebSocket Client Test Example

Client that connects to the server and sends messages.
*/

#[cfg(feature = "websocket")]
use std::sync::Arc;
#[cfg(feature = "websocket")]
use tokio::time::{sleep, Duration};
#[cfg(feature = "websocket")]
use umicp_sdk::{Envelope, OperationType, WebSocketClient};
#[cfg(feature = "websocket")]
use serde_json::json;

#[cfg(feature = "websocket")]
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize tracing
    tracing_subscriber::fmt::init();

    println!("🔌 Connecting to WebSocket server...");

    // Create client
    let client = Arc::new(WebSocketClient::new("ws://127.0.0.1:9001"));

    // Connect with retry
    match client.connect_with_retry().await {
        Ok(_) => println!("✓ Connected to server!"),
        Err(e) => {
            eprintln!("❌ Failed to connect: {}", e);
            eprintln!("\n💡 Make sure the server is running:");
            eprintln!("   cargo run --features websocket --example websocket_with_handlers");
            return Err(e.into());
        }
    }

    println!("\n📤 Sending messages...\n");

    // Send multiple messages
    for i in 1..=5 {
        let msg_num = i.to_string();
        let content = format!("Hello from Rust! Message #{}", i);
        let timestamp = chrono::Utc::now().to_rfc3339();

        let envelope = Envelope::builder()
            .from("rust-client")
            .to("server")
            .operation(OperationType::Data)
            .capability("message_number", json!(msg_num))
            .capability("content", json!(content))
            .capability("timestamp", json!(timestamp))
            .build()?;

        println!("[{}] Sending message #{}...", chrono::Utc::now().format("%H:%M:%S"), i);
        client.send(envelope).await?;
        println!("   ✓ Sent");

        sleep(Duration::from_secs(2)).await;
    }

    println!("\n📊 Client statistics:");
    let stats = client.get_stats();
    println!("   Messages sent: {}", stats.messages_sent);
    println!("   Messages received: {}", stats.messages_received);
    println!("   Bytes sent: {}", stats.bytes_sent);
    println!("   Bytes received: {}", stats.bytes_received);
    println!("   Connections: {}", stats.connect_count);

    // Wait a bit to receive any broadcast messages
    println!("\n⏳ Waiting for potential broadcasts...");
    sleep(Duration::from_secs(3)).await;

    // Disconnect
    println!("\n🔌 Disconnecting...");
    client.disconnect().await?;
    println!("✓ Disconnected");

    Ok(())
}

#[cfg(not(feature = "websocket"))]
fn main() {
    eprintln!("This example requires the 'websocket' feature.");
    eprintln!("Run with: cargo run --example websocket_client_test --features websocket");
}

