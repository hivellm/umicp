use serde_json::{json, Value};
use umicp_core::{Envelope, EnvelopeBuilder, OperationType};

// ============================================================================
// CAPABILITIES COMPREHENSIVE TESTS (Rust v0.2.0)
// ============================================================================

#[test]
fn test_capability_integer_positive() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("max_tokens", json!(100));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    assert_eq!(deserialized.capabilities.as_ref().unwrap()["max_tokens"], 100);
}

#[test]
fn test_capability_integer_negative() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("offset", json!(-50));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    assert_eq!(deserialized.capabilities.as_ref().unwrap()["offset"], -50);
}

#[test]
fn test_capability_integer_zero() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("count", json!(0));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    assert_eq!(deserialized.capabilities.as_ref().unwrap()["count"], 0);
}

#[test]
fn test_capability_integer_large() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("max_value", json!(2147483647i64));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    assert_eq!(deserialized.capabilities.as_ref().unwrap()["max_value"], 2147483647i64);
}

#[test]
fn test_capability_float_decimal() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("temperature", json!(0.7));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let temp = deserialized.capabilities.as_ref().unwrap()["temperature"].as_f64().unwrap();
    assert!((temp - 0.7).abs() < 0.001);
}

#[test]
fn test_capability_float_scientific() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("learning_rate", json!(1.5e-4));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let rate = deserialized.capabilities.as_ref().unwrap()["learning_rate"].as_f64().unwrap();
    assert!((rate - 1.5e-4).abs() < 1e-10);
}

#[test]
fn test_capability_boolean_true() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("enabled", json!(true));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    assert_eq!(deserialized.capabilities.as_ref().unwrap()["enabled"], true);
}

#[test]
fn test_capability_boolean_false() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("disabled", json!(false));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    assert_eq!(deserialized.capabilities.as_ref().unwrap()["disabled"], false);
}

#[test]
fn test_capability_string_simple() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability_str("model", "gpt-4");

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    assert_eq!(deserialized.capabilities.as_ref().unwrap()["model"], "gpt-4");
}

#[test]
fn test_capability_string_empty() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability_str("empty", "");

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    assert_eq!(deserialized.capabilities.as_ref().unwrap()["empty"], "");
}

#[test]
fn test_capability_string_unicode() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability_str("message", "Hello 世界 🌍");

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    assert_eq!(deserialized.capabilities.as_ref().unwrap()["message"], "Hello 世界 🌍");
}

#[test]
fn test_capability_array_strings() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("models", json!(["gpt-4", "claude-3", "llama-3"]));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let arr = deserialized.capabilities.as_ref().unwrap()["models"].as_array().unwrap();
    assert_eq!(arr.len(), 3);
    assert_eq!(arr[0], "gpt-4");
}

#[test]
fn test_capability_array_integers() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("limits", json!([10, 20, 30, 40]));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let arr = deserialized.capabilities.as_ref().unwrap()["limits"].as_array().unwrap();
    assert_eq!(arr.len(), 4);
    assert_eq!(arr[2], 30);
}

#[test]
fn test_capability_array_mixed() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("mixed", json!([1, "two", true, 4.5]));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let arr = deserialized.capabilities.as_ref().unwrap()["mixed"].as_array().unwrap();
    assert_eq!(arr[0], 1);
    assert_eq!(arr[1], "two");
    assert_eq!(arr[2], true);
    assert!((arr[3].as_f64().unwrap() - 4.5).abs() < 0.001);
}

#[test]
fn test_capability_array_empty() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("empty_array", json!([]));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let arr = deserialized.capabilities.as_ref().unwrap()["empty_array"].as_array().unwrap();
    assert_eq!(arr.len(), 0);
}

#[test]
fn test_capability_array_nested() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("nested", json!([[1, 2], [3, 4]]));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let arr = deserialized.capabilities.as_ref().unwrap()["nested"].as_array().unwrap();
    assert_eq!(arr[0][1], 2);
    assert_eq!(arr[1][0], 3);
}

#[test]
fn test_capability_object_simple() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("config", json!({"timeout": 30, "retries": 3}));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let obj = deserialized.capabilities.as_ref().unwrap()["config"].as_object().unwrap();
    assert_eq!(obj["timeout"], 30);
    assert_eq!(obj["retries"], 3);
}

#[test]
fn test_capability_object_nested() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("auth", json!({
        "type": "oauth",
        "credentials": {
            "client_id": "abc123",
            "scope": ["read", "write"]
        }
    }));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let auth = &deserialized.capabilities.as_ref().unwrap()["auth"];
    assert_eq!(auth["type"], "oauth");
    assert_eq!(auth["credentials"]["client_id"], "abc123");
}

#[test]
fn test_capability_object_empty() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("empty_obj", json!({}));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let obj = deserialized.capabilities.as_ref().unwrap()["empty_obj"].as_object().unwrap();
    assert_eq!(obj.len(), 0);
}

#[test]
fn test_capability_null_value() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("optional", Value::Null);

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    assert!(deserialized.capabilities.as_ref().unwrap()["optional"].is_null());
}

#[test]
fn test_capability_complex_mixed_types() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability_str("string_val", "test");
    envelope.add_capability("int_val", json!(42));
    envelope.add_capability("float_val", json!(3.14));
    envelope.add_capability("bool_val", json!(true));
    envelope.add_capability("null_val", Value::Null);
    envelope.add_capability("array_val", json!([1, 2, 3]));
    envelope.add_capability("object_val", json!({"key": "value"}));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let caps = deserialized.capabilities.as_ref().unwrap();
    assert_eq!(caps["string_val"], "test");
    assert_eq!(caps["int_val"], 42);
    assert!((caps["float_val"].as_f64().unwrap() - 3.14).abs() < 0.001);
    assert_eq!(caps["bool_val"], true);
    assert!(caps["null_val"].is_null());
    assert_eq!(caps["array_val"].as_array().unwrap().len(), 3);
    assert_eq!(caps["object_val"]["key"], "value");
}

#[test]
fn test_capability_special_characters_in_keys() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("key-with-dash", json!(1));
    envelope.add_capability("key_with_underscore", json!(2));
    envelope.add_capability("key.with.dot", json!(3));
    envelope.add_capability("key:with:colon", json!(4));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let caps = deserialized.capabilities.as_ref().unwrap();
    assert_eq!(caps["key-with-dash"], 1);
    assert_eq!(caps["key_with_underscore"], 2);
    assert_eq!(caps["key.with.dot"], 3);
    assert_eq!(caps["key:with:colon"], 4);
}

#[test]
fn test_capability_very_large_object() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);

    for i in 0..100 {
        envelope.add_capability(&format!("key_{}", i), json!(i));
    }

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let caps = deserialized.capabilities.as_ref().unwrap();
    assert_eq!(caps.len(), 100);
    assert_eq!(caps["key_50"], 50);
    assert_eq!(caps["key_99"], 99);
}

#[test]
fn test_capability_deeply_nested_object() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("deep", json!({
        "level1": {
            "level2": {
                "level3": {
                    "level4": {
                        "level5": "deep_value"
                    }
                }
            }
        }
    }));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let deep = &deserialized.capabilities.as_ref().unwrap()["deep"];
    assert_eq!(
        deep["level1"]["level2"]["level3"]["level4"]["level5"],
        "deep_value"
    );
}

#[test]
fn test_capability_type_preservation() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("int", json!(42));
    envelope.add_capability("float", json!(3.14));
    envelope.add_capability("bool", json!(true));
    envelope.add_capability_str("string", "text");
    envelope.add_capability("array", json!([1, 2]));
    envelope.add_capability("object", json!({"a": 1}));

    let serialized = serde_json::to_string(&envelope).unwrap();
    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();

    let caps = deserialized.capabilities.as_ref().unwrap();
    assert!(caps["int"].is_number());
    assert!(caps["float"].is_number());
    assert!(caps["bool"].is_boolean());
    assert!(caps["string"].is_string());
    assert!(caps["array"].is_array());
    assert!(caps["object"].is_object());
}

#[test]
fn test_capability_invalid_json_deserialization() {
    let invalid_json = r#"{"from":"test","to":"test","operation":"Data","capabilities":{invalid}}"#;

    let result: Result<Envelope, _> = serde_json::from_str(invalid_json);
    assert!(result.is_err());
}

#[test]
fn test_capability_missing_field() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    // Don't set capabilities

    let serialized = serde_json::to_string(&envelope).unwrap();
    assert!(!serialized.contains("capabilities"));

    let deserialized: Envelope = serde_json::from_str(&serialized).unwrap();
    assert!(deserialized.capabilities.is_none());
}

#[test]
fn test_builder_with_native_types() {
    let envelope = EnvelopeBuilder::new()
        .from("sender")
        .to("receiver")
        .operation(OperationType::Data)
        .capability("count", json!(100))
        .capability("enabled", json!(true))
        .capability("ratio", json!(0.75))
        .build();

    let caps = envelope.capabilities.as_ref().unwrap();
    assert_eq!(caps["count"], 100);
    assert_eq!(caps["enabled"], true);
    assert!((caps["ratio"].as_f64().unwrap() - 0.75).abs() < 0.001);
}

#[test]
fn test_builder_capability_str_helper() {
    let envelope = EnvelopeBuilder::new()
        .from("sender")
        .to("receiver")
        .capability_str("model", "gpt-4")
        .build();

    assert_eq!(envelope.capabilities.as_ref().unwrap()["model"], "gpt-4");
}

#[test]
fn test_builder_multiple_capabilities() {
    let envelope = EnvelopeBuilder::new()
        .from("sender")
        .to("receiver")
        .capability("a", json!(1))
        .capability("b", json!(2))
        .capability("c", json!(3))
        .build();

    let caps = envelope.capabilities.as_ref().unwrap();
    assert_eq!(caps.len(), 3);
    assert_eq!(caps["a"], 1);
    assert_eq!(caps["b"], 2);
    assert_eq!(caps["c"], 3);
}

#[test]
fn test_backward_compatibility_with_string_values() {
    // Old format: all values as strings
    let old_format = r#"{
        "from": "sender",
        "to": "receiver",
        "operation": "Data",
        "capabilities": {
            "model": "gpt-4",
            "count": "100"
        }
    }"#;

    let envelope: Envelope = serde_json::from_str(old_format).unwrap();

    let caps = envelope.capabilities.as_ref().unwrap();
    assert_eq!(caps["model"], "gpt-4");
    assert_eq!(caps["count"], "100"); // Preserved as string
}

#[test]
fn test_capability_overwrite() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("key", json!(1));
    envelope.add_capability("key", json!(2)); // Overwrite

    assert_eq!(envelope.capabilities.as_ref().unwrap()["key"], 2);
}

#[test]
fn test_capability_clone() {
    let mut envelope1 = Envelope::new("sender", "receiver", OperationType::Data);
    envelope1.add_capability("test", json!(42));

    let envelope2 = envelope1.clone();

    assert_eq!(envelope2.capabilities.as_ref().unwrap()["test"], 42);
}

#[test]
fn test_capability_debug_format() {
    let mut envelope = Envelope::new("sender", "receiver", OperationType::Data);
    envelope.add_capability("test", json!(123));

    let debug_str = format!("{:?}", envelope);
    assert!(debug_str.contains("capabilities"));
}

