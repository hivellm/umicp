/*!
Comprehensive tests for Rust binding v0.2.0 native type support
*/

use umicp_core::{Envelope, OperationType, Capabilities};
use serde_json::json;

#[test]
fn test_capabilities_with_integers() {
    let mut caps = Capabilities::new();
    caps.insert("count".to_string(), json!(42));
    caps.insert("negative".to_string(), json!(-100));
    caps.insert("zero".to_string(), json!(0));
    caps.insert("large".to_string(), json!(9223372036854775807i64));

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capabilities(caps.clone())
        .build()
        .unwrap();

    let result_caps = envelope.capabilities().unwrap();
    assert_eq!(result_caps["count"].as_i64().unwrap(), 42);
    assert_eq!(result_caps["negative"].as_i64().unwrap(), -100);
    assert_eq!(result_caps["zero"].as_i64().unwrap(), 0);
    assert_eq!(result_caps["large"].as_i64().unwrap(), 9223372036854775807i64);
}

#[test]
fn test_capabilities_with_floats() {
    let mut caps = Capabilities::new();
    caps.insert("pi".to_string(), json!(3.141592653589793));
    caps.insert("negative".to_string(), json!(-95.5));
    caps.insert("zero".to_string(), json!(0.0));

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capabilities(caps)
        .build()
        .unwrap();

    let result_caps = envelope.capabilities().unwrap();
    let pi = result_caps["pi"].as_f64().unwrap();
    assert!((pi - 3.141592653589793).abs() < 1e-10);
    assert_eq!(result_caps["negative"].as_f64().unwrap(), -95.5);
}

#[test]
fn test_capabilities_with_booleans() {
    let mut caps = Capabilities::new();
    caps.insert("enabled".to_string(), json!(true));
    caps.insert("disabled".to_string(), json!(false));

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capabilities(caps)
        .build()
        .unwrap();

    let result_caps = envelope.capabilities().unwrap();
    assert_eq!(result_caps["enabled"].as_bool().unwrap(), true);
    assert_eq!(result_caps["disabled"].as_bool().unwrap(), false);
}

#[test]
fn test_capabilities_with_null() {
    let mut caps = Capabilities::new();
    caps.insert("optional_field".to_string(), json!(null));

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capabilities(caps)
        .build()
        .unwrap();

    let result_caps = envelope.capabilities().unwrap();
    assert!(result_caps["optional_field"].is_null());
}

#[test]
fn test_capabilities_with_arrays() {
    let mut caps = Capabilities::new();
    caps.insert("tags".to_string(), json!(["rust", "umicp", "v0.2.0"]));
    caps.insert("numbers".to_string(), json!([1, 2, 3, 4, 5]));
    caps.insert("mixed".to_string(), json!([1, "two", true, 4.5, null]));

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capabilities(caps)
        .build()
        .unwrap();

    let result_caps = envelope.capabilities().unwrap();

    let tags = result_caps["tags"].as_array().unwrap();
    assert_eq!(tags.len(), 3);
    assert_eq!(tags[0].as_str().unwrap(), "rust");

    let numbers = result_caps["numbers"].as_array().unwrap();
    assert_eq!(numbers[2].as_i64().unwrap(), 3);

    let mixed = result_caps["mixed"].as_array().unwrap();
    assert_eq!(mixed[0].as_i64().unwrap(), 1);
    assert_eq!(mixed[1].as_str().unwrap(), "two");
    assert_eq!(mixed[2].as_bool().unwrap(), true);
    assert!(mixed[4].is_null());
}

#[test]
fn test_capabilities_with_objects() {
    let mut caps = Capabilities::new();
    caps.insert("metadata".to_string(), json!({
        "author": "HiveLLM",
        "version": "0.2.0",
        "count": 42,
        "enabled": true
    }));

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capabilities(caps)
        .build()
        .unwrap();

    let result_caps = envelope.capabilities().unwrap();
    let metadata = result_caps["metadata"].as_object().unwrap();
    assert_eq!(metadata["author"].as_str().unwrap(), "HiveLLM");
    assert_eq!(metadata["version"].as_str().unwrap(), "0.2.0");
    assert_eq!(metadata["count"].as_i64().unwrap(), 42);
    assert_eq!(metadata["enabled"].as_bool().unwrap(), true);
}

#[test]
fn test_capabilities_deeply_nested() {
    let mut caps = Capabilities::new();
    caps.insert("complex".to_string(), json!({
        "level1": {
            "level2": {
                "level3": {
                    "value": "deep",
                    "count": 100
                }
            }
        }
    }));

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capabilities(caps)
        .build()
        .unwrap();

    let result_caps = envelope.capabilities().unwrap();
    let deep_value = &result_caps["complex"]["level1"]["level2"]["level3"]["value"];
    assert_eq!(deep_value.as_str().unwrap(), "deep");

    let deep_count = &result_caps["complex"]["level1"]["level2"]["level3"]["count"];
    assert_eq!(deep_count.as_i64().unwrap(), 100);
}

#[test]
fn test_serialization_preserves_types() {
    let mut caps = Capabilities::new();
    caps.insert("string".to_string(), json!("hello"));
    caps.insert("int".to_string(), json!(42));
    caps.insert("bool".to_string(), json!(true));
    caps.insert("array".to_string(), json!([1, 2, 3]));
    caps.insert("object".to_string(), json!({"key": "value"}));

    let envelope1 = Envelope::builder()
        .from("client")
        .to("server")
        .operation(OperationType::Data)
        .capabilities(caps)
        .build()
        .unwrap();

    // Serialize
    let serialized = envelope1.serialize().unwrap();

    // Deserialize
    let envelope2 = Envelope::deserialize(&serialized).unwrap();

    // Verify types preserved
    let result_caps = envelope2.capabilities().unwrap();
    assert!(result_caps["string"].is_string());
    assert!(result_caps["int"].is_i64());
    assert!(result_caps["bool"].is_boolean());
    assert!(result_caps["array"].is_array());
    assert!(result_caps["object"].is_object());
}

#[test]
fn test_unicode_strings() {
    let mut caps = Capabilities::new();
    caps.insert("emoji".to_string(), json!("Hello 🚀 World 🌍"));
    caps.insert("chinese".to_string(), json!("你好世界"));
    caps.insert("arabic".to_string(), json!("مرحبا بالعالم"));

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capabilities(caps)
        .build()
        .unwrap();

    let result_caps = envelope.capabilities().unwrap();
    assert_eq!(result_caps["emoji"].as_str().unwrap(), "Hello 🚀 World 🌍");
    assert_eq!(result_caps["chinese"].as_str().unwrap(), "你好世界");
    assert_eq!(result_caps["arabic"].as_str().unwrap(), "مرحبا بالعالم");
}

#[test]
fn test_empty_values() {
    let mut caps = Capabilities::new();
    caps.insert("empty_string".to_string(), json!(""));
    caps.insert("empty_array".to_string(), json!([]));
    caps.insert("empty_object".to_string(), json!({}));

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capabilities(caps)
        .build()
        .unwrap();

    let result_caps = envelope.capabilities().unwrap();
    assert_eq!(result_caps["empty_string"].as_str().unwrap(), "");
    assert_eq!(result_caps["empty_array"].as_array().unwrap().len(), 0);
    assert_eq!(result_caps["empty_object"].as_object().unwrap().len(), 0);
}

#[test]
fn test_backward_compatibility() {
    // Old-style JSON with string values
    let old_json = r#"{
        "v": "1.0",
        "msg_id": "test-123",
        "ts": "2025-10-16T12:00:00.000Z",
        "from": "client",
        "to": "server",
        "op": "data",
        "capabilities": {
            "count": "42",
            "enabled": "true"
        }
    }"#;

    let envelope = Envelope::deserialize(old_json).unwrap();

    let caps = envelope.capabilities().unwrap();
    // Values come in as strings (backward compatible)
    assert_eq!(caps["count"].as_str().unwrap(), "42");
    assert_eq!(caps["enabled"].as_str().unwrap(), "true");
}

#[test]
fn test_capability_builder_methods() {
    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capability("string", json!("value"))
        .capability("number", json!(42))
        .capability("bool", json!(true))
        .build()
        .unwrap();

    let caps = envelope.capabilities().unwrap();
    assert_eq!(caps.len(), 3);
    assert!(caps.contains_key("string"));
    assert!(caps.contains_key("number"));
    assert!(caps.contains_key("bool"));
}

#[test]
fn test_large_array() {
    let large_vec: Vec<i32> = (0..1000).collect();

    let mut caps = Capabilities::new();
    caps.insert("large_array".to_string(), json!(large_vec));

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capabilities(caps)
        .build()
        .unwrap();

    let result_caps = envelope.capabilities().unwrap();
    let arr = result_caps["large_array"].as_array().unwrap();
    assert_eq!(arr.len(), 1000);
    assert_eq!(arr[0].as_i64().unwrap(), 0);
    assert_eq!(arr[999].as_i64().unwrap(), 999);
}

#[test]
fn test_mixed_builder_methods() {
    let mut caps1 = Capabilities::new();
    caps1.insert("batch1".to_string(), json!("value1"));

    let envelope = Envelope::builder()
        .from("client")
        .to("server")
        .capabilities(caps1)
        .capability("additional", json!(42))
        .capability_str("str_method", "hello")
        .build()
        .unwrap();

    let result_caps = envelope.capabilities().unwrap();
    assert_eq!(result_caps.len(), 3);
    assert_eq!(result_caps["batch1"].as_str().unwrap(), "value1");
    assert_eq!(result_caps["additional"].as_i64().unwrap(), 42);
    assert_eq!(result_caps["str_method"].as_str().unwrap(), "hello");
}

