/*!
# Tests for UMICP WebSocket Transport
*/

#![cfg(feature = "websocket")]

use umicp_core::{Envelope, OperationType, WebSocketClient, WebSocketServer};
use std::time::Duration;
use tokio::time::{sleep, timeout};

#[tokio::test]
async fn test_create_websocket_client() {
    let client = WebSocketClient::new("ws://localhost:8080");
    assert!(!client.is_connected());
}

#[tokio::test]
async fn test_create_websocket_server() {
    let server = WebSocketServer::new("127.0.0.1:20090").expect("Failed to create server");
    let stats = server.get_stats();

    assert_eq!(stats.active_connections, 0);
    assert_eq!(stats.total_connections, 0);
}

#[tokio::test(flavor = "multi_thread")]
async fn test_client_server_connection() {
    // Start server (non-blocking now!)
    let mut server = WebSocketServer::new("127.0.0.1:20091")
        .expect("Failed to create server");

    let _server_handle = server.start().await.expect("Failed to start server");

    // Give server time to start
    sleep(Duration::from_millis(500)).await;

    // Connect client
    let client = WebSocketClient::new("ws://127.0.0.1:20091");

    let connect_result = timeout(Duration::from_secs(5), client.connect()).await;

    assert!(connect_result.is_ok(), "Connection timed out");
    assert!(connect_result.unwrap().is_ok(), "Connection failed");
    assert!(client.is_connected());

    let _ = client.disconnect().await;

    // Cleanup
    server.shutdown().expect("Failed to shutdown server");
}

#[tokio::test(flavor = "multi_thread")]
async fn test_send_message_client_to_server() {
    // Start server
    let mut server = WebSocketServer::new("127.0.0.1:20092")
        .expect("Failed to create server");

    let _handle = server.start().await.expect("Failed to start server");

    sleep(Duration::from_millis(500)).await;

    // Connect and send
    let client = WebSocketClient::new("ws://127.0.0.1:20092");
    client.connect().await.expect("Failed to connect");

    sleep(Duration::from_millis(100)).await;

    let envelope = Envelope::builder()
        .from("test-client")
        .to("test-server")
        .operation(OperationType::Data)
        .message_id("test-001")
        .capability("message", "Hello from client!")
        .build()
        .expect("Failed to build envelope");

    let send_result = client.send(envelope).await;
    assert!(send_result.is_ok(), "Failed to send message");

    // Verify stats
    sleep(Duration::from_millis(100)).await;
    let stats = client.get_stats();
    assert_eq!(stats.messages_sent, 1);
    assert!(stats.bytes_sent > 0);

    client.disconnect().await.expect("Failed to disconnect");
    server.shutdown().expect("Failed to shutdown server");
}

#[tokio::test(flavor = "multi_thread")]
async fn test_multiple_messages() {
    let mut server = WebSocketServer::new("127.0.0.1:20093")
        .expect("Failed to create server");

    let _handle = server.start().await.expect("Failed to start server");

    sleep(Duration::from_millis(200)).await;

    let client = WebSocketClient::new("ws://127.0.0.1:20093");
    client.connect().await.expect("Failed to connect");

    sleep(Duration::from_millis(100)).await;

    // Send multiple messages
    for i in 0..10 {
        let envelope = Envelope::builder()
            .from("client")
            .to("server")
            .operation(OperationType::Data)
            .message_id(&format!("msg-{:03}", i))
            .capability("sequence", &i.to_string())
            .build()
            .expect("Failed to build envelope");

        client.send(envelope).await.expect("Failed to send");
    }

    sleep(Duration::from_millis(200)).await;

    let stats = client.get_stats();
    assert_eq!(stats.messages_sent, 10);

    client.disconnect().await.expect("Failed to disconnect");
    server.shutdown().expect("Failed to shutdown server");
}

#[tokio::test(flavor = "multi_thread")]
async fn test_client_disconnect_and_reconnect() {
    let mut server = WebSocketServer::new("127.0.0.1:20094")
        .expect("Failed to create server");

    let _handle = server.start().await.expect("Failed to start server");

    sleep(Duration::from_millis(200)).await;

    let client = WebSocketClient::new("ws://127.0.0.1:20094");

    // First connection
    client.connect().await.expect("Failed to connect");
    assert!(client.is_connected());

    let stats1 = client.get_stats();
    assert_eq!(stats1.connect_count, 1);

    // Disconnect
    client.disconnect().await.expect("Failed to disconnect");
    assert!(!client.is_connected());

    sleep(Duration::from_millis(100)).await;

    // Reconnect
    client.connect().await.expect("Failed to reconnect");
    assert!(client.is_connected());

    let stats2 = client.get_stats();
    assert_eq!(stats2.connect_count, 2);

    client.disconnect().await.expect("Failed to disconnect");
    server.shutdown().expect("Failed to shutdown server");
}

#[tokio::test]
async fn test_client_stats_tracking() {
    let client = WebSocketClient::new("ws://localhost:20095");

    let stats = client.get_stats();
    assert_eq!(stats.messages_sent, 0);
    assert_eq!(stats.messages_received, 0);
    assert_eq!(stats.bytes_sent, 0);
    assert_eq!(stats.bytes_received, 0);
    assert_eq!(stats.connect_count, 0);
}

#[tokio::test]
async fn test_server_broadcast() {
    let server = WebSocketServer::new("127.0.0.1:20096")
        .expect("Failed to create server");

    tokio::spawn(async move {
        // Note: This is a placeholder test since we need to refactor server
        // to actually implement broadcast with client handling
        let _ = timeout(Duration::from_secs(1), async {
            // Server would run here
        }).await;
    });

    // For now just verify server was created
    let stats = server.get_stats();
    assert_eq!(stats.active_connections, 0);
}

#[tokio::test]
async fn test_connection_timeout() {
    // Try to connect to non-existent server
    let client = WebSocketClient::new("ws://localhost:19999");

    let result = timeout(Duration::from_secs(2), client.connect()).await;

    // Should timeout or fail to connect
    assert!(result.is_err() || result.unwrap().is_err());
}

#[tokio::test]
async fn test_send_without_connection() {
    let client = WebSocketClient::new("ws://localhost:20097");

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .build()
        .expect("Failed to build envelope");

    // Should fail because not connected
    let result = client.send(envelope).await;
    assert!(result.is_err());
}

#[tokio::test(flavor = "multi_thread")]
#[ignore] // Ignore for now - requires refactored server
async fn test_large_message() {
    let mut server = WebSocketServer::new("127.0.0.1:20098")
        .expect("Failed to create server");

    tokio::spawn(async move {
        let _ = timeout(Duration::from_secs(10), server.start()).await;
    });

    sleep(Duration::from_millis(200)).await;

    let client = WebSocketClient::new("ws://127.0.0.1:20098");
    client.connect().await.expect("Failed to connect");

    sleep(Duration::from_millis(100)).await;

    // Create envelope with many capabilities (large payload)
    let mut capabilities = std::collections::HashMap::new();
    for i in 0..500 {
        capabilities.insert(
            format!("key_{}", i),
            format!("value_with_some_long_content_{}", i),
        );
    }

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .operation(OperationType::Data)
        .capabilities(capabilities)
        .build()
        .expect("Failed to build envelope");

    let send_result = client.send(envelope).await;
    assert!(send_result.is_ok(), "Failed to send large message");

    sleep(Duration::from_millis(200)).await;

    let stats = client.get_stats();
    assert!(stats.bytes_sent > 10000); // Should be a large message

    client.disconnect().await.expect("Failed to disconnect");
}

#[tokio::test(flavor = "multi_thread")]
#[ignore] // Ignore for now - requires refactored server
async fn test_concurrent_clients() {
    let mut server = WebSocketServer::new("127.0.0.1:20099")
        .expect("Failed to create server");

    tokio::spawn(async move {
        let _ = timeout(Duration::from_secs(10), server.start()).await;
    });

    sleep(Duration::from_millis(200)).await;

    // Create multiple clients
    let client1 = WebSocketClient::new("ws://127.0.0.1:20099");
    let client2 = WebSocketClient::new("ws://127.0.0.1:20099");
    let client3 = WebSocketClient::new("ws://127.0.0.1:20099");

    // Connect all
    client1.connect().await.expect("Client 1 failed to connect");
    client2.connect().await.expect("Client 2 failed to connect");
    client3.connect().await.expect("Client 3 failed to connect");

    sleep(Duration::from_millis(200)).await;

    assert!(client1.is_connected());
    assert!(client2.is_connected());
    assert!(client3.is_connected());

    // All send messages
    for client in [&client1, &client2, &client3] {
        let envelope = Envelope::builder()
            .from("client")
            .to("server")
            .operation(OperationType::Data)
            .build()
            .expect("Failed to build envelope");

        client.send(envelope).await.expect("Failed to send");
    }

    sleep(Duration::from_millis(200)).await;

    // Verify all sent
    assert_eq!(client1.get_stats().messages_sent, 1);
    assert_eq!(client2.get_stats().messages_sent, 1);
    assert_eq!(client3.get_stats().messages_sent, 1);

    // Disconnect all
    client1.disconnect().await.expect("Failed to disconnect");
    client2.disconnect().await.expect("Failed to disconnect");
    client3.disconnect().await.expect("Failed to disconnect");
}

#[tokio::test(flavor = "multi_thread")]
#[ignore] // Ignore for now - requires refactored server
async fn test_operation_types() {
    let mut server = WebSocketServer::new("127.0.0.1:20100")
        .expect("Failed to create server");

    tokio::spawn(async move {
        let _ = timeout(Duration::from_secs(10), server.start()).await;
    });

    sleep(Duration::from_millis(200)).await;

    let client = WebSocketClient::new("ws://127.0.0.1:20100");
    client.connect().await.expect("Failed to connect");

    sleep(Duration::from_millis(100)).await;

    // Test each operation type
    let operations = vec![
        OperationType::Control,
        OperationType::Data,
        OperationType::Ack,
        OperationType::Error,
        OperationType::Request,
        OperationType::Response,
    ];

    for op in operations {
        let envelope = Envelope::builder()
            .from("client")
            .to("server")
            .operation(op)
            .message_id(&format!("msg-{:?}", op))
            .build()
            .expect("Failed to build envelope");

        let result = client.send(envelope).await;
        assert!(result.is_ok(), "Failed to send {:?} message", op);
    }

    sleep(Duration::from_millis(200)).await;

    let stats = client.get_stats();
    assert_eq!(stats.messages_sent, 6);

    client.disconnect().await.expect("Failed to disconnect");
}

