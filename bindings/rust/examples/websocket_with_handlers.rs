/*!
# WebSocket Server with Message Handlers Example

Demonstrates the refactored non-blocking WebSocket server with message handlers.
*/

#[cfg(feature = "websocket")]
use std::sync::Arc;
#[cfg(feature = "websocket")]
use tokio::time::{sleep, Duration};
#[cfg(feature = "websocket")]
use umicp_core::{Envelope, OperationType, WebSocketServer};
#[cfg(feature = "websocket")]
use serde_json::json;

#[cfg(feature = "websocket")]
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize tracing
    tracing_subscriber::fmt::init();

    // Create server
    let mut server = WebSocketServer::new("127.0.0.1:9001")?;

    // Set message handler
    server.set_message_handler(Arc::new(|envelope, client_id| {
        println!("📨 Message from client {}: {} -> {}",
            client_id, envelope.from(), envelope.to());
        println!("   Operation: {:?}", envelope.operation());
        if let Some(caps) = envelope.capabilities() {
            println!("   Capabilities: {} entries", caps.len());
        }
    }));

    // Set connection handler
    server.set_connection_handler(Arc::new(|client_id, addr| {
        println!("✅ Client connected: {} from {}", client_id, addr);
    }));

    // Set disconnection handler
    server.set_disconnection_handler(Arc::new(|client_id, addr| {
        println!("❌ Client disconnected: {} from {}", client_id, addr);
    }));

    // Start server (non-blocking)
    println!("🚀 Starting WebSocket server on 127.0.0.1:9001...");
    let server_handle = server.start().await?;
    println!("✓ Server started and running in background!");

    // Server is now running in background, we can do other things
    println!("\n📊 Server statistics:");

    for i in 1..=10 {
        sleep(Duration::from_secs(2)).await;

        let stats = server.get_stats();
        println!("\n[{}s] Active: {} | Total: {} | Messages: ↓{} ↑{}",
            i * 2,
            stats.active_connections,
            stats.total_connections,
            stats.messages_received,
            stats.messages_sent
        );

        // Send broadcast message every 4 seconds if there are clients
        if i % 2 == 0 && stats.active_connections > 0 {
            let message = format!("Server time: {}s", i * 2);
            let envelope = Envelope::builder()
                .from("server")
                .to("all")
                .operation(OperationType::Data)
                .capability("type", json!("broadcast"))
                .capability("message", json!(message))
                .build()?;

            server.broadcast(envelope).await?;
            println!("📡 Broadcast sent to all clients");
        }
    }

    // Shutdown server
    println!("\n🛑 Shutting down server...");
    server.shutdown()?;

    // Wait for server to stop
    server_handle.await?;

    println!("✓ Server stopped gracefully");

    Ok(())
}

#[cfg(not(feature = "websocket"))]
fn main() {
    eprintln!("This example requires the 'websocket' feature.");
    eprintln!("Run with: cargo run --example websocket_with_handlers --features websocket");
}

