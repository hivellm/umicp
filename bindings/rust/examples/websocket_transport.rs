/*!
# WebSocket Transport Example

This example demonstrates how to use the WebSocket transport layer for
real-time communication with UMICP envelopes.
*/

// WebSocket transport example - requires websocket feature

#[cfg(feature = "websocket")]
fn main() -> Result<(), Box<dyn std::error::Error>> {
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
    let mut server = WebSocketTransport::new_server("127.0.0.1:8080").await?;

    // Message counter
    let message_count = Arc::new(Mutex::new(0));

    // Set message handler
    let message_count_clone = Arc::clone(&message_count);
    server.set_message_handler(move |envelope: Envelope, conn_id: String| {
        let message_count = Arc::clone(&message_count_clone);
        async move {
            let mut count = message_count.lock().await;
            *count += 1;

            println!("📨 Received message #{} from {}", *count, conn_id);
            println!("   From: {}", envelope.from());
            println!("   Operation: {:?}", envelope.operation());
            println!("   Message ID: {}", envelope.message_id());

            if let Some(capabilities) = envelope.capabilities() {
                println!("   Capabilities:");
                for (key, value) in capabilities {
                    println!("     {}: {}", key, value);
                }
            }

            // Create acknowledgment response
            let response = Envelope::builder()
                .from("server")
                .to(envelope.from())
                .operation(OperationType::Ack)
                .message_id(format!("ack-{}", envelope.message_id()))
                .capability("status", "received")
                .capability("server_time", &chrono::Utc::now().to_rfc3339())
                .build()?;

            // Send response
            server.send(response, &conn_id).await?;
            println!("   ✓ Sent acknowledgment\n");

            Ok(())
        }
    });

    // Set connection handler
    server.set_connection_handler(|connected: bool, conn_id: String| async move {
        if connected {
            println!("🔗 Client connected: {}", conn_id);
        } else {
            println!("📴 Client disconnected: {}", conn_id);
        }
    });

    println!("🚀 Server running on ws://127.0.0.1:8080");
    println!("💡 Press Ctrl+C to stop\n");

    // Run server
    server.run().await?;

    Ok(())
}

#[cfg(feature = "websocket")]
async fn run_client() -> Result<(), Box<dyn std::error::Error>> {
    println!("Starting WebSocket client...");

    // Create client transport
    let mut client = WebSocketTransport::new_client("ws://127.0.0.1:8080").await?;

    // Message counter
    let message_count = Arc::new(Mutex::new(0));

    // Set message handler for responses
    let message_count_clone = Arc::clone(&message_count);
    client.set_message_handler(move |envelope: Envelope, _conn_id: String| {
        let message_count = Arc::clone(&message_count_clone);
        async move {
            let mut count = message_count.lock().await;
            *count += 1;

            println!("📬 Received response #{} from server", *count);
            println!("   From: {}", envelope.from());
            println!("   Operation: {:?}", envelope.operation());
            println!("   Message ID: {}", envelope.message_id());

            if let Some(capabilities) = envelope.capabilities() {
                println!("   Capabilities:");
                for (key, value) in capabilities {
                    println!("     {}: {}", key, value);
                }
            }
            println!();

            Ok(())
        }
    });

    // Set connection handler
    client.set_connection_handler(|connected: bool, conn_id: String| async move {
        if connected {
            println!("🔗 Connected to server: {}", conn_id);
        } else {
            println!("📴 Disconnected from server: {}", conn_id);
        }
    });

    // Start client in background
    let client_clone = client.clone();
    tokio::spawn(async move {
        if let Err(e) = client.run().await {
            eprintln!("Client error: {}", e);
        }
    });

    // Wait a moment for connection
    tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;

    println!("🚀 Client connected to ws://127.0.0.1:8080");
    println!("💡 Sending test messages...\n");

    // Send test messages
    for i in 1..=5 {
        let message = Envelope::builder()
            .from("rust-client")
            .to("server")
            .operation(OperationType::Data)
            .message_id(format!("test-msg-{}", i))
            .capability("message_type", "test")
            .capability("sequence", &i.to_string())
            .capability("timestamp", &chrono::Utc::now().to_rfc3339())
            .capability("data", &format!("Hello from Rust client! Message #{}", i))
            .build()?;

        println!("📤 Sending message {}", i);
        client_clone.send_to_server(message).await?;

        // Wait between messages
        tokio::time::sleep(tokio::time::Duration::from_secs(1)).await;
    }

    // Wait for responses
    println!("\n⏳ Waiting for responses...");
    tokio::time::sleep(tokio::time::Duration::from_secs(3)).await;

    // Get statistics
    let stats = client_clone.get_stats().await;
    println!("\n📊 Client Statistics:");
    println!("   Messages sent: {}", stats.messages_sent);
    println!("   Messages received: {}", stats.messages_received);
    println!("   Bytes sent: {}", stats.bytes_sent);
    println!("   Bytes received: {}", stats.bytes_received);
    println!("   Active connections: {}", stats.active_connections);
    println!("   Total connections: {}", stats.total_connections);

    // Shutdown
    client_clone.shutdown().await?;
    println!("\n✓ Client example completed successfully!");

    Ok(())
}

#[cfg(not(feature = "websocket"))]
fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("WebSocket transport is not available.");
    println!("To enable WebSocket support, compile with:");
    println!("  cargo build --features websocket");
    Ok(())
}
