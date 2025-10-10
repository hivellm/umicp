/*!
# Tests for UMICP Envelope functionality
*/

use umicp_core::{Envelope, OperationType, PayloadType, EncodingType, PayloadHint};

#[test]
fn test_create_envelope_with_default_values() {
    let envelope = Envelope::builder()
        .from("test-sender")
        .to("test-receiver")
        .build()
        .expect("Failed to build envelope");

    assert!(envelope.validate().is_ok());
    assert_eq!(envelope.from(), "test-sender");
    assert_eq!(envelope.to(), "test-receiver");
}

#[test]
fn test_create_envelope_with_custom_options() {
    let mut capabilities = std::collections::HashMap::new();
    capabilities.insert("compression".to_string(), "gzip".to_string());
    capabilities.insert("encryption".to_string(), "aes256".to_string());

    let envelope = Envelope::builder()
        .from("test-sender")
        .to("test-receiver")
        .operation(OperationType::Data)
        .message_id("msg-123")
        .capabilities(capabilities.clone())
        .build()
        .expect("Failed to build envelope");

    assert!(envelope.validate().is_ok());
    assert_eq!(envelope.operation(), OperationType::Data);
    assert_eq!(envelope.message_id(), "msg-123");

    if let Some(caps) = envelope.capabilities() {
        assert_eq!(caps.get("compression"), Some(&"gzip".to_string()));
        assert_eq!(caps.get("encryption"), Some(&"aes256".to_string()));
    } else {
        panic!("Capabilities not found");
    }
}

#[test]
fn test_serialize_and_deserialize_envelope() {
    let original = Envelope::builder()
        .from("sender")
        .to("receiver")
        .operation(OperationType::Data)
        .message_id("test-msg-001")
        .build()
        .expect("Failed to build envelope");

    let json = original.serialize().expect("Failed to serialize");
    assert!(!json.is_empty());

    let deserialized = Envelope::deserialize(&json).expect("Failed to deserialize");
    assert!(deserialized.validate().is_ok());
    assert_eq!(deserialized.from(), original.from());
    assert_eq!(deserialized.to(), original.to());
    assert_eq!(deserialized.message_id(), original.message_id());
}

#[test]
fn test_generate_hash_for_envelope() {
    let envelope = Envelope::builder()
        .from("test")
        .to("receiver")
        .operation(OperationType::Data)
        .build()
        .expect("Failed to build envelope");

    let hash = envelope.hash().expect("Failed to generate hash");
    assert!(!hash.is_empty());

    // Hash should be consistent
    let hash2 = envelope.hash().expect("Failed to generate hash");
    assert_eq!(hash, hash2);
}

#[test]
fn test_handle_payload_hints() {
    let payload_hint = PayloadHint {
        payload_type: PayloadType::Vector,
        size: Some(1024),
        encoding: Some(EncodingType::Float32),
        count: Some(256),
    };

    let envelope = Envelope::builder()
        .from("sender")
        .to("receiver")
        .payload_hint(payload_hint)
        .build()
        .expect("Failed to build envelope");

    assert!(envelope.validate().is_ok());
    assert!(envelope.payload_hint().is_some());

    if let Some(hint) = envelope.payload_hint() {
        assert_eq!(hint.payload_type, PayloadType::Vector);
        assert_eq!(hint.size, Some(1024));
        assert_eq!(hint.encoding, Some(EncodingType::Float32));
        assert_eq!(hint.count, Some(256));
    }
}

#[test]
fn test_handle_capabilities() {
    let mut capabilities = std::collections::HashMap::new();
    capabilities.insert("binary_support".to_string(), "true".to_string());
    capabilities.insert("compression".to_string(), "gzip,brotli".to_string());
    capabilities.insert("formats".to_string(), "cbor,msgpack".to_string());

    let envelope = Envelope::builder()
        .from("sender")
        .to("receiver")
        .capabilities(capabilities.clone())
        .build()
        .expect("Failed to build envelope");

    assert!(envelope.validate().is_ok());

    if let Some(caps) = envelope.capabilities() {
        assert_eq!(caps.get("binary_support"), Some(&"true".to_string()));
        assert_eq!(caps.get("compression"), Some(&"gzip,brotli".to_string()));
        assert_eq!(caps.get("formats"), Some(&"cbor,msgpack".to_string()));
    } else {
        panic!("Capabilities not found");
    }
}

#[test]
fn test_validate_required_fields() {
    // Valid envelope
    let valid = Envelope::builder()
        .from("sender")
        .to("receiver")
        .build()
        .expect("Failed to build envelope");

    assert!(valid.validate().is_ok());
}

#[test]
fn test_operation_types() {
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
            .from("test")
            .to("receiver")
            .operation(op)
            .build()
            .expect("Failed to build envelope");

        assert_eq!(envelope.operation(), op);
        assert!(envelope.validate().is_ok());
    }
}

#[test]
fn test_envelope_immutability() {
    let envelope = Envelope::builder()
        .from("original-sender")
        .to("original-receiver")
        .message_id("original-id")
        .build()
        .expect("Failed to build envelope");

    // Serialize once
    let json1 = envelope.serialize().expect("Failed to serialize");

    // Serialize again
    let json2 = envelope.serialize().expect("Failed to serialize");

    // Should produce identical JSON
    assert_eq!(json1, json2);
}

#[test]
fn test_builder_pattern() {
    let envelope = Envelope::builder()
        .from("sender")
        .to("receiver")
        .operation(OperationType::Data)
        .message_id("test-id")
        .capability("key1", "value1")
        .capability("key2", "value2")
        .build()
        .expect("Failed to build envelope");

    assert_eq!(envelope.from(), "sender");
    assert_eq!(envelope.to(), "receiver");
    assert_eq!(envelope.message_id(), "test-id");

    if let Some(caps) = envelope.capabilities() {
        assert_eq!(caps.len(), 2);
        assert_eq!(caps.get("key1"), Some(&"value1".to_string()));
        assert_eq!(caps.get("key2"), Some(&"value2".to_string()));
    } else {
        panic!("Capabilities not found");
    }
}

#[test]
fn test_large_capabilities() {
    let mut capabilities = std::collections::HashMap::new();

    // Add many capabilities
    for i in 0..100 {
        capabilities.insert(format!("key_{}", i), format!("value_{}", i));
    }

    let envelope = Envelope::builder()
        .from("sender")
        .to("receiver")
        .capabilities(capabilities)
        .build()
        .expect("Failed to build envelope");

    assert!(envelope.validate().is_ok());

    if let Some(caps) = envelope.capabilities() {
        assert_eq!(caps.len(), 100);
    }
}

#[test]
fn test_serialize_deserialize_round_trip() {
    let original = Envelope::builder()
        .from("sender-123")
        .to("receiver-456")
        .operation(OperationType::Data)
        .message_id("msg-unique-001")
        .capability("test", "round-trip")
        .capability("version", "1.0")
        .build()
        .expect("Failed to build envelope");

    // Serialize
    let json = original.serialize().expect("Failed to serialize");

    // Deserialize
    let deserialized = Envelope::deserialize(&json).expect("Failed to deserialize");

    // Verify all fields match
    assert_eq!(deserialized.from(), original.from());
    assert_eq!(deserialized.to(), original.to());
    assert_eq!(deserialized.operation(), original.operation());
    assert_eq!(deserialized.message_id(), original.message_id());

    // Serialize again to verify consistency
    let json2 = deserialized.serialize().expect("Failed to serialize");

    // Should produce equivalent JSON (may not be byte-identical due to ordering)
    let parsed1: serde_json::Value = serde_json::from_str(&json).unwrap();
    let parsed2: serde_json::Value = serde_json::from_str(&json2).unwrap();
    assert_eq!(parsed1, parsed2);
}

#[test]
fn test_hash_consistency() {
    let envelope = Envelope::builder()
        .from("sender")
        .to("receiver")
        .message_id("test-hash")
        .build()
        .expect("Failed to build envelope");

    let hash1 = envelope.hash().expect("Failed to generate hash");
    let hash2 = envelope.hash().expect("Failed to generate hash");
    let hash3 = envelope.hash().expect("Failed to generate hash");

    assert_eq!(hash1, hash2);
    assert_eq!(hash2, hash3);
}

#[test]
fn test_different_envelopes_different_hashes() {
    let envelope1 = Envelope::builder()
        .from("sender-1")
        .to("receiver")
        .build()
        .expect("Failed to build envelope");

    let envelope2 = Envelope::builder()
        .from("sender-2")
        .to("receiver")
        .build()
        .expect("Failed to build envelope");

    let hash1 = envelope1.hash().expect("Failed to generate hash");
    let hash2 = envelope2.hash().expect("Failed to generate hash");

    assert_ne!(hash1, hash2);
}

