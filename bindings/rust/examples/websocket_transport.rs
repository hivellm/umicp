/*!
# WebSocket Transport Example

This example demonstrates how to use the WebSocket transport layer for
real-time communication with UMICP envelopes.
*/

#[cfg(feature = "websocket")]
use std::sync::Arc;
#[cfg(feature = "websocket")]
use tokio::sync::Mutex;
#[cfg(feature = "websocket")]
use umicp_core::{Envelope, OperationType, WebSocketClient, WebSocketServer};
#[cfg(feature = "websocket")]
use serde_json::json;

// WebSocket transport example - requires websocket feature

#[cfg(feature = "websocket")]
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("UMICP Rust Bindings - WebSocket Transport Example");
    println!("================================================\n");

    // Choose mode: server or client
    let args: Vec<String> = std::env::args().collect();
    let is_server = args.get(1).map(|s| s == "server").unwrap_or(true);

    if is_server {
        run_server().await?;
    } else {
        run_client().await?;
    }

    Ok(())
}

#[cfg(feature = "websocket")]
async fn run_server() -> Result<(), Box<dyn std::error::Error>> {
    println!("Starting WebSocket server...");

    // Create server transport
    let mut server = WebSocketServer::new("127.0.0.1:8080")?;

    // Message counter
    let message_count = Arc::new(Mutex::new(0));

    // Set message handler
    let message_count_clone = Arc::clone(&message_count);
    let server_clone = server.clone();
    server.set_message_handler(Arc::new(move |envelope: Envelope, conn_id: String| {
        let message_count = Arc::clone(&message_count_clone);
        let server_clone = server_clone.clone();
        tokio::spawn(async move {
            let mut count = message_count.lock().await;
            *count += 1;

            println!("📨 Received message #{} from {}", *count, conn_id);
            println!("   From: {}", envelope.from());
            println!("   Operation: {:?}", envelope.operation());
            println!("   Message ID: {}", envelope.message_id());

            // Create acknowledgment response
            let response = Envelope::builder()
                .from("server")
                .to(envelope.from())
                .operation(OperationType::Ack)
                .message_id(format!("ack-{}", envelope.message_id()).as_str())
                .capability("status", json!("received"))
                .capability("server_time", json!(chrono::Utc::now().to_rfc3339()))
                .build().unwrap();

            // Send response
            let _ = server_clone.send_to_client(&conn_id, response).await;
            println!("   ✓ Sent acknowledgment\n");
        });
    }));

    // Start server
    let handle = server.start().await?;
    println!("🚀 Server listening on ws://127.0.0.1:8080");
    println!("💡 Press Ctrl+C to stop\n");

    // Wait for server to finish
    handle.await?;

    Ok(())
}

#[cfg(feature = "websocket")]
async fn run_client() -> Result<(), Box<dyn std::error::Error>> {
    println!("Starting WebSocket client...");

    // Create client transport
    let client = WebSocketClient::new("ws://127.0.0.1:8080");

    // Message counter
    let message_count = Arc::new(Mutex::new(0));

    // Set message handler for responses
    let message_count_clone = Arc::clone(&message_count);
    client.set_message_handler(Arc::new(move |envelope: Envelope| {
        let message_count = Arc::clone(&message_count_clone);
        tokio::spawn(async move {
            let mut count = message_count.lock().await;
            *count += 1;

            println!("📬 Received response #{} from server", *count);
            println!("   From: {}", envelope.from());
            println!("   Operation: {:?}", envelope.operation());
            println!("   Message ID: {}", envelope.message_id());
        });
    }));

    // Connect client
    client.connect().await?;
    println!("🚀 Client connected to ws://127.0.0.1:8080");
    println!("💡 Sending test messages...\n");

    // Send test messages
    for i in 1..=5 {
        let message = Envelope::builder()
            .from("rust-client")
            .to("server")
            .operation(OperationType::Data)
            .message_id(format!("test-msg-{}", i).as_str())
            .capability("message_type", json!("test"))
            .capability("sequence", json!(i))
            .capability("timestamp", json!(chrono::Utc::now().to_rfc3339()))
            .capability("data", json!(format!("Hello from Rust client! Message #{}", i)))
            .build()?;

        client.send(message).await?;
        println!("📤 Sent message {}", i);

        // Wait a bit between messages
        tokio::time::sleep(tokio::time::Duration::from_millis(500)).await;
    }

    println!("\n✅ All messages sent. Waiting for responses...\n");

    // Wait for responses
    tokio::time::sleep(tokio::time::Duration::from_secs(5)).await;

    println!("👋 Client finished");
    Ok(())
}

#[cfg(not(feature = "websocket"))]
fn main() {
    eprintln!("This example requires the 'websocket' feature.");
    eprintln!("Run with: cargo run --example websocket_transport --features websocket");
}
