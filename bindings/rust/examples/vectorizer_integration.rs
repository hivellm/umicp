//! UMICP Vectorizer Integration Example
//!
//! Demonstrates how to use the UMICP Rust SDK with the Vectorizer service
//! which uses the /umicp endpoint instead of the default /message endpoint.
//!
//! Note: This example requires the 'http2' and 'tokio' features to be enabled.
//! Run with: cargo run --example vectorizer_integration --features http2,tokio

#[cfg(all(feature = "http2", feature = "tokio"))]
fn main() -> Result<(), Box<dyn std::error::Error>> {
    use std::collections::HashMap;
    use umicp_core::{Envelope, OperationType};
    use umicp_core::transport::http_client::{HttpClient, HttpClientConfig};
    use std::time::Duration;

    println!("Note: This example requires async runtime.");
    println!("The HTTP client would be used in an async context.");
    println!("Example usage shown in test suite.");

    Ok(())
}

#[cfg(not(all(feature = "http2", feature = "tokio")))]
fn main() {
    println!("This example requires 'http2' and 'tokio' features.");
    println!("Run with: cargo run --example vectorizer_integration --features http2,tokio");
}

// Async version for when features are available
#[cfg(all(feature = "http2", feature = "tokio"))]
#[allow(dead_code)]
async fn example_async() -> Result<(), Box<dyn std::error::Error>> {
    // Initialize tracing
    tracing_subscriber::fmt::init();

    println!("========================================");
    println!("UMICP Vectorizer Integration Example");
    println!("========================================\n");

    // Example 1: Using new_with_path helper
    example1_with_path_helper().await?;

    // Example 2: Using full config
    example2_with_config().await?;

    println!("\n========================================");
    println!("All examples completed successfully! ✅");
    println!("========================================");

    Ok(())
}

/// Example 1: Using the new_with_path helper method
async fn example1_with_path_helper() -> Result<(), Box<dyn std::error::Error>> {
    println!("📝 Example 1: Using new_with_path()");
    println!("   Creating client for Vectorizer at http://localhost:8000/umicp\n");

    // Create client with custom path for Vectorizer
    let client = HttpClient::new_with_path("http://localhost:8000", "/umicp")?;

    // Create a discovery envelope
    let mut capabilities = HashMap::new();
    capabilities.insert("operation".to_string(), serde_json::json!("discover"));
    capabilities.insert("type".to_string(), serde_json::json!("operations"));

    let envelope = Envelope::builder()
        .from("rust-example")
        .to("vectorizer")
        .operation(OperationType::Request)
        .message_id(&uuid::Uuid::new_v4().to_string())
        .capabilities(capabilities)
        .build()?;

    println!("📤 Sending discovery request to Vectorizer...");
    println!("   Endpoint: /umicp");
    println!("   From: {}", envelope.from());
    println!("   To: {}", envelope.to());

    // Send envelope
    match client.send(envelope).await {
        Ok(response) => {
            println!("✅ Success! Received response:");
            println!("   Response: {}", serde_json::to_string_pretty(&response)?);
        }
        Err(e) => {
            println!("⚠️  Request failed (this is expected if Vectorizer is not running):");
            println!("   Error: {}", e);
        }
    }

    // Show stats
    let stats = client.get_stats();
    println!("\n📊 Client Statistics:");
    println!("   Requests sent: {}", stats.requests_sent);
    println!("   Responses received: {}", stats.responses_received);
    println!("   Errors: {}", stats.errors);

    Ok(())
}

/// Example 2: Using full HttpClientConfig
async fn example2_with_config() -> Result<(), Box<dyn std::error::Error>> {
    println!("\n📝 Example 2: Using HttpClientConfig");
    println!("   Creating client with full configuration\n");

    // Create config with custom settings
    let config = HttpClientConfig {
        base_url: "http://localhost:8000".to_string(),
        path: "/umicp".to_string(),  // Vectorizer endpoint
        timeout: Duration::from_secs(30),
        max_retries: 3,
    };

    let client = HttpClient::with_config(config)?;

    // Create a data envelope
    let mut capabilities = HashMap::new();
    capabilities.insert("action".to_string(), serde_json::json!("query"));
    capabilities.insert("collection".to_string(), serde_json::json!("test_collection"));
    capabilities.insert("query".to_string(), serde_json::json!("test query"));

    let envelope = Envelope::builder()
        .from("rust-example")
        .to("vectorizer")
        .operation(OperationType::Data)
        .message_id(&uuid::Uuid::new_v4().to_string())
        .capabilities(capabilities)
        .build()?;

    println!("📤 Sending data envelope to Vectorizer...");
    println!("   Endpoint: /umicp");
    println!("   Operation: DATA");

    match client.send_with_retry(envelope).await {
        Ok(response) => {
            println!("✅ Success with retry! Received response:");
            println!("   Response: {}", serde_json::to_string_pretty(&response)?);
        }
        Err(e) => {
            println!("⚠️  Request failed (this is expected if Vectorizer is not running):");
            println!("   Error: {}", e);
        }
    }

    Ok(())
}

