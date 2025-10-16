/*!
# Basic HTTP Transport Example

Demonstrates HTTP server and client communication.
*/

#[cfg(feature = "http2")]
use std::sync::Arc;
#[cfg(feature = "http2")]
use tokio::time::{sleep, Duration};
#[cfg(feature = "http2")]
use umicp_core::{Envelope, HttpClient, HttpServer, OperationType};
#[cfg(feature = "http2")]
use serde_json::json;

#[cfg(feature = "http2")]
#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    tracing_subscriber::fmt::init();

    println!("🌐 HTTP Transport Example\n");

    // Create HTTP server
    let mut server = HttpServer::new("127.0.0.1:3001")?;

    server.set_message_handler(Arc::new(|envelope, client_id| {
        println!("[Server] 📨 Received from {}", client_id);
        println!("         From: {} -> To: {}", envelope.from(), envelope.to());
        if let Some(caps) = envelope.capabilities() {
            if let Some(msg) = caps.get("message") {
                println!("         Message: {}", msg);
            }
        }
    }));

    println!("🚀 Starting HTTP server on http://127.0.0.1:3001...");
    let server_handle = server.start().await?;
    println!("✓ Server started\n");

    // Wait for server to be ready
    sleep(Duration::from_millis(500)).await;

    // Create HTTP client
    println!("🔌 Creating HTTP client...");
    let client = HttpClient::new("http://127.0.0.1:3001")?;

    // Health check
    println!("💊 Checking server health...");
    match client.health_check().await {
        Ok(health) => println!("✓ Server healthy: {:?}\n", health),
        Err(e) => println!("⚠ Health check failed: {}\n", e),
    }

    // Send messages
    println!("📤 Sending messages via HTTP...\n");

    for i in 1..=3 {
        let msg_content = format!("HTTP message #{}", i);
        let envelope = Envelope::builder()
            .from("http-client")
            .to("http-server")
            .operation(OperationType::Data)
            .capability("message", json!(msg_content))
            .capability("sequence", json!(i))
            .capability("protocol", json!("http"))
            .build()?;

        println!("[{}] Sending message #{}...", chrono::Utc::now().format("%H:%M:%S"), i);

        match client.send(envelope).await {
            Ok(response) => {
                println!("   ✓ Response: {:?}", response);
            }
            Err(e) => {
                println!("   ❌ Error: {}", e);
            }
        }

        sleep(Duration::from_millis(500)).await;
    }

    // Show statistics
    println!("\n📊 Client Statistics:");
    let stats = client.get_stats();
    println!("   Requests sent: {}", stats.requests_sent);
    println!("   Responses received: {}", stats.responses_received);
    println!("   Bytes sent: {}", stats.bytes_sent);
    println!("   Bytes received: {}", stats.bytes_received);
    println!("   Errors: {}", stats.errors);

    println!("\n📊 Server Statistics:");
    let server_stats = server.get_stats();
    println!("   Requests received: {}", server_stats.requests_received);
    println!("   Responses sent: {}", server_stats.responses_sent);
    println!("   Errors: {}", server_stats.errors);

    // Shutdown
    println!("\n🛑 Shutting down...");
    server.shutdown()?;
    server_handle.await?;
    println!("✓ Server stopped");

    Ok(())
}

#[cfg(not(feature = "http2"))]
fn main() {
    println!("This example requires the 'http2' feature.");
    println!("Run with: cargo run --example http_basic --features http2");
}

