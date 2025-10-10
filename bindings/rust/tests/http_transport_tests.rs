/*!
# HTTP Transport Integration Tests
*/

use std::sync::Arc;
use tokio::time::{sleep, timeout, Duration};
use umicp_core::{Envelope, HttpClient, HttpServer, OperationType};

#[tokio::test]
async fn test_http_server_startup() {
    let mut server = HttpServer::new("127.0.0.1:3010").unwrap();
    
    let handle = server.start().await.unwrap();
    
    // Give server time to start
    sleep(Duration::from_millis(100)).await;
    
    // Check it's running
    assert!(!handle.is_finished());
    
    // Shutdown
    server.shutdown().unwrap();
    
    // Wait for shutdown
    let _ = timeout(Duration::from_secs(2), handle).await;
}

#[tokio::test]
async fn test_http_client_creation() {
    let client = HttpClient::new("http://localhost:3011").unwrap();
    let stats = client.get_stats();
    
    assert_eq!(stats.requests_sent, 0);
    assert_eq!(stats.responses_received, 0);
}

#[tokio::test]
async fn test_http_client_server_communication() {
    let mut server = HttpServer::new("127.0.0.1:3012").unwrap();
    
    let received = Arc::new(parking_lot::RwLock::new(Vec::new()));
    let received_clone = Arc::clone(&received);
    
    server.set_message_handler(Arc::new(move |envelope, _client_id| {
        received_clone.write().push(envelope.from().to_string());
    }));
    
    let _handle = server.start().await.unwrap();
    
    // Wait for server to be ready
    sleep(Duration::from_millis(200)).await;
    
    // Create client
    let client = HttpClient::new("http://127.0.0.1:3012").unwrap();
    
    // Send message
    let envelope = Envelope::builder()
        .from("test-client")
        .to("test-server")
        .operation(OperationType::Data)
        .capability("message", "test")
        .build()
        .unwrap();
    
    let response = client.send(envelope).await.unwrap();
    
    assert_eq!(response["status"], "ok");
    
    // Give server time to process
    sleep(Duration::from_millis(100)).await;
    
    // Check message was received
    assert_eq!(received.read().len(), 1);
    assert_eq!(received.read()[0], "test-client");
    
    server.shutdown().unwrap();
}

#[tokio::test]
async fn test_http_health_check() {
    let mut server = HttpServer::new("127.0.0.1:3013").unwrap();
    let _handle = server.start().await.unwrap();
    
    sleep(Duration::from_millis(200)).await;
    
    let client = HttpClient::new("http://127.0.0.1:3013").unwrap();
    let health = client.health_check().await.unwrap();
    
    assert_eq!(health["status"], "ok");
    assert!(health.get("uptime_seconds").is_some());
    
    server.shutdown().unwrap();
}

#[tokio::test]
async fn test_http_multiple_messages() {
    let mut server = HttpServer::new("127.0.0.1:3014").unwrap();
    
    let count = Arc::new(parking_lot::RwLock::new(0));
    let count_clone = Arc::clone(&count);
    
    server.set_message_handler(Arc::new(move |_envelope, _client_id| {
        *count_clone.write() += 1;
    }));
    
    let _handle = server.start().await.unwrap();
    sleep(Duration::from_millis(200)).await;
    
    let client = HttpClient::new("http://127.0.0.1:3014").unwrap();
    
    // Send multiple messages
    for i in 1..=5 {
        let envelope = Envelope::builder()
            .from(&format!("client-{}", i))
            .to("server")
            .operation(OperationType::Data)
            .capability("sequence", &i.to_string())
            .build()
            .unwrap();
        
        client.send(envelope).await.unwrap();
    }
    
    sleep(Duration::from_millis(200)).await;
    
    assert_eq!(*count.read(), 5);
    
    let stats = server.get_stats();
    assert_eq!(stats.requests_received, 5);
    assert_eq!(stats.responses_sent, 5);
    
    server.shutdown().unwrap();
}

#[tokio::test]
async fn test_http_client_retry() {
    let client = HttpClient::new("http://127.0.0.1:9999").unwrap(); // Non-existent server
    
    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .operation(OperationType::Data)
        .build()
        .unwrap();
    
    // Should fail after retries
    let result = timeout(
        Duration::from_secs(2),
        client.send_with_retry(envelope)
    ).await;
    
    assert!(result.is_ok()); // Timeout ok
    assert!(result.unwrap().is_err()); // But send failed
    
    let stats = client.get_stats();
    assert!(stats.errors > 0);
}

#[tokio::test]
async fn test_http_server_stats() {
    let mut server = HttpServer::new("127.0.0.1:3015").unwrap();
    let _handle = server.start().await.unwrap();
    
    sleep(Duration::from_millis(100)).await;
    
    let stats1 = server.get_stats();
    assert_eq!(stats1.requests_received, 0);
    
    // Send a request
    let client = HttpClient::new("http://127.0.0.1:3015").unwrap();
    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .operation(OperationType::Data)
        .build()
        .unwrap();
    
    client.send(envelope).await.unwrap();
    sleep(Duration::from_millis(100)).await;
    
    let stats2 = server.get_stats();
    assert_eq!(stats2.requests_received, 1);
    assert_eq!(stats2.responses_sent, 1);
    
    server.shutdown().unwrap();
}

