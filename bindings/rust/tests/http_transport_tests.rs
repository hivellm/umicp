/*!
# Tests for UMICP HTTP/2 Transport - Envelope Tests
*/

#![cfg(feature = "http2")]

use umicp_core::{Envelope, OperationType};
use serde_json::json;

#[tokio::test]
async fn test_http_envelope_creation() {
    // Test that envelopes can be created for HTTP transport
    let envelope = Envelope::builder()
        .operation(OperationType::Data)
        .from("http-client")
        .to("http-server")
        .build();

    assert!(envelope.is_ok());
}

#[tokio::test]
async fn test_http_request_envelope() {
    let envelope = Envelope::builder()
        .operation(OperationType::Request)
        .from("client")
        .to("server")
        .build();

    assert!(envelope.is_ok());
}

#[tokio::test]
async fn test_http_response_envelope() {
    let envelope = Envelope::builder()
        .operation(OperationType::Response)
        .from("server")
        .to("client")
        .build();

    assert!(envelope.is_ok());
}

#[tokio::test]
async fn test_http_ack_envelope() {
    let envelope = Envelope::builder()
        .operation(OperationType::Ack)
        .from("server")
        .to("client")
        .build();

    assert!(envelope.is_ok());
}

#[tokio::test]
async fn test_http_error_envelope() {
    let envelope = Envelope::builder()
        .operation(OperationType::Error)
        .from("server")
        .to("client")
        .build();

    assert!(envelope.is_ok());
}

#[tokio::test]
async fn test_http_control_envelope() {
    let envelope = Envelope::builder()
        .operation(OperationType::Control)
        .from("server")
        .to("client")
        .build();

    assert!(envelope.is_ok());
}

#[tokio::test]
async fn test_http_envelope_with_message_id() {
    let envelope = Envelope::builder()
        .operation(OperationType::Data)
        .from("client")
        .to("server")
        .message_id("test-message-123")
        .build();

    assert!(envelope.is_ok());
}

#[tokio::test]
async fn test_http_envelope_with_capabilities() {
    use umicp_core::Capabilities;

    let mut caps = Capabilities::new();
    caps.insert("http_version".to_string(), json!("2"));
    caps.insert("compression".to_string(), json!("gzip"));

    let envelope = Envelope::builder()
        .operation(OperationType::Data)
        .from("client")
        .to("server")
        .capabilities(caps)
        .build();

    assert!(envelope.is_ok());
}

#[tokio::test]
async fn test_http_envelope_validation() {
    // Valid envelope
    let envelope = Envelope::builder()
        .operation(OperationType::Data)
        .from("client")
        .to("server")
        .build()
        .expect("Failed to create envelope");

    assert!(envelope.validate().is_ok());
}

#[tokio::test]
async fn test_http_all_operation_types() {
    let operations = vec![
        OperationType::Data,
        OperationType::Request,
        OperationType::Response,
        OperationType::Ack,
        OperationType::Error,
        OperationType::Control,
    ];

    for op in operations {
        let envelope = Envelope::builder()
            .operation(op)
            .from("client")
            .to("server")
            .build();

        assert!(envelope.is_ok(), "Failed to create envelope for operation type");
    }
}
