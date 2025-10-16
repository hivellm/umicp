/*!
Comprehensive tests for Rust binding v0.2.0 tool discovery
*/

use umicp_core::{DiscoverableService, OperationSchema, ServerInfo};
use serde_json::json;

// Example service for testing
struct VectorizerService;

impl DiscoverableService for VectorizerService {
    fn list_operations(&self) -> Vec<OperationSchema> {
        vec![
            OperationSchema::new(
                "search_vectors",
                json!({
                    "type": "object",
                    "properties": {
                        "collection": {"type": "string"},
                        "query": {"type": "string"},
                        "limit": {"type": "integer", "default": 10}
                    },
                    "required": ["collection", "query"]
                })
            )
            .title("Search Vectors")
            .description("Search for semantically similar content")
            .annotations(json!({"read_only": true, "idempotent": true})),

            OperationSchema::new(
                "create_collection",
                json!({
                    "type": "object",
                    "properties": {
                        "name": {"type": "string"},
                        "dimension": {"type": "integer"}
                    },
                    "required": ["name", "dimension"]
                })
            )
            .title("Create Collection")
            .annotations(json!({"read_only": false}))
        ]
    }

    fn server_info(&self) -> ServerInfo {
        ServerInfo::new("vectorizer", "0.9.0", "UMICP/0.2")
            .features(vec!["discovery".to_string(), "search".to_string()])
            .operations_count(2)
            .mcp_compatible(true)
    }
}

#[test]
fn test_list_operations_returns_all() {
    let service = VectorizerService;
    let operations = service.list_operations();

    assert_eq!(operations.len(), 2);
    assert_eq!(operations[0].name, "search_vectors");
    assert_eq!(operations[1].name, "create_collection");
}

#[test]
fn test_get_schema_found() {
    let service = VectorizerService;
    let schema = service.get_schema("search_vectors");

    assert!(schema.is_some());
    let schema = schema.unwrap();
    assert_eq!(schema.name, "search_vectors");
    assert_eq!(schema.title, Some("Search Vectors".to_string()));
    assert!(schema.input_schema.is_object());
}

#[test]
fn test_get_schema_not_found() {
    let service = VectorizerService;
    let schema = service.get_schema("non_existent");

    assert!(schema.is_none());
}

#[test]
fn test_server_info_complete() {
    let service = VectorizerService;
    let info = service.server_info();

    assert_eq!(info.server, "vectorizer");
    assert_eq!(info.version, "0.9.0");
    assert_eq!(info.protocol, "UMICP/0.2");
    assert_eq!(info.features, Some(vec!["discovery".to_string(), "search".to_string()]));
    assert_eq!(info.operations_count, Some(2));
    assert_eq!(info.mcp_compatible, Some(true));
}

#[test]
fn test_operation_schema_with_all_fields() {
    let schema = OperationSchema::new(
        "complete_op",
        json!({"type": "object"})
    )
    .title("Complete")
    .description("A complete operation")
    .output_schema(json!({"type": "string"}))
    .annotations(json!({"read_only": true}));

    assert_eq!(schema.name, "complete_op");
    assert!(schema.title.is_some());
    assert!(schema.description.is_some());
    assert!(schema.output_schema.is_some());
    assert!(schema.annotations.is_some());
}

#[test]
fn test_operation_schema_minimal() {
    let schema = OperationSchema::new(
        "minimal",
        json!({"type": "object"})
    );

    assert_eq!(schema.name, "minimal");
    assert!(schema.title.is_none());
    assert!(schema.description.is_none());
    assert!(schema.output_schema.is_none());
    assert!(schema.annotations.is_none());
}

#[test]
fn test_server_info_with_all_fields() {
    let info = ServerInfo::new("test-server", "1.0.0", "UMICP/0.2")
        .features(vec!["a".to_string(), "b".to_string()])
        .operations_count(10)
        .mcp_compatible(true)
        .metadata(json!({"license": "MIT"}));

    assert!(info.features.is_some());
    assert!(info.operations_count.is_some());
    assert!(info.mcp_compatible.is_some());
    assert!(info.metadata.is_some());
}

#[test]
fn test_server_info_minimal() {
    let info = ServerInfo::new("minimal", "1.0", "UMICP/0.2");

    assert_eq!(info.server, "minimal");
    assert!(info.features.is_none());
    assert!(info.operations_count.is_none());
    assert!(info.mcp_compatible.is_none());
    assert!(info.metadata.is_none());
}

#[test]
fn test_operation_schema_serialization() {
    let schema = OperationSchema::new(
        "test_op",
        json!({"type": "object", "properties": {"param": {"type": "string"}}})
    )
    .title("Test")
    .annotations(json!({"read_only": true}));

    let serialized = serde_json::to_string(&schema).unwrap();
    assert!(serialized.contains("test_op"));
    assert!(serialized.contains("Test"));
    assert!(serialized.contains("read_only"));
}

#[test]
fn test_server_info_serialization() {
    let info = ServerInfo::new("my-service", "1.0.0", "UMICP/0.2")
        .features(vec!["discovery".to_string()])
        .mcp_compatible(true);

    let serialized = serde_json::to_string(&info).unwrap();
    assert!(serialized.contains("my-service"));
    assert!(serialized.contains("1.0.0"));
    assert!(serialized.contains("UMICP/0.2"));
    assert!(serialized.contains("discovery"));
}

#[test]
fn test_large_operation_list() {
    struct LargeService;

    impl DiscoverableService for LargeService {
        fn list_operations(&self) -> Vec<OperationSchema> {
            (0..20).map(|i| {
                OperationSchema::new(
                    format!("op_{}", i),
                    json!({"type": "object"})
                )
                .title(format!("Operation {}", i))
            }).collect()
        }

        fn server_info(&self) -> ServerInfo {
            ServerInfo::new("large", "1.0", "UMICP/0.2")
                .operations_count(20)
        }
    }

    let service = LargeService;
    let ops = service.list_operations();
    assert_eq!(ops.len(), 20);
    assert_eq!(ops[0].name, "op_0");
    assert_eq!(ops[19].name, "op_19");
}

