/*!
# UMICP Tool Discovery

Tool discovery trait and types for UMICP services, compatible with MCP JSON Schema.
*/

use serde::{Deserialize, Serialize};
use serde_json::Value;

/// Operation schema compatible with MCP JSON Schema
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct OperationSchema {
    /// Operation name
    pub name: String,
    /// Display title
    #[serde(skip_serializing_if = "Option::is_none")]
    pub title: Option<String>,
    /// Description of what the operation does
    #[serde(skip_serializing_if = "Option::is_none")]
    pub description: Option<String>,
    /// JSON Schema for input parameters
    pub input_schema: Value,
    /// Optional output schema
    #[serde(skip_serializing_if = "Option::is_none")]
    pub output_schema: Option<Value>,
    /// Annotations (read_only, idempotent, destructive, etc.)
    #[serde(skip_serializing_if = "Option::is_none")]
    pub annotations: Option<Value>,
}

impl OperationSchema {
    /// Create a new operation schema
    pub fn new(name: impl Into<String>, input_schema: Value) -> Self {
        Self {
            name: name.into(),
            title: None,
            description: None,
            input_schema,
            output_schema: None,
            annotations: None,
        }
    }

    /// Set title (builder pattern)
    pub fn title(mut self, title: impl Into<String>) -> Self {
        self.title = Some(title.into());
        self
    }

    /// Set description (builder pattern)
    pub fn description(mut self, description: impl Into<String>) -> Self {
        self.description = Some(description.into());
        self
    }

    /// Set output schema (builder pattern)
    pub fn output_schema(mut self, schema: Value) -> Self {
        self.output_schema = Some(schema);
        self
    }

    /// Set annotations (builder pattern)
    pub fn annotations(mut self, annotations: Value) -> Self {
        self.annotations = Some(annotations);
        self
    }
}

/// Server information for discovery
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ServerInfo {
    /// Server name/identifier
    pub server: String,
    /// Server version
    pub version: String,
    /// Protocol version (e.g., "UMICP/2.0")
    pub protocol: String,
    /// List of supported features
    #[serde(skip_serializing_if = "Option::is_none")]
    pub features: Option<Vec<String>>,
    /// Number of available operations
    #[serde(skip_serializing_if = "Option::is_none")]
    pub operations_count: Option<usize>,
    /// MCP compatibility flag
    #[serde(skip_serializing_if = "Option::is_none")]
    pub mcp_compatible: Option<bool>,
    /// Additional metadata
    #[serde(skip_serializing_if = "Option::is_none")]
    pub metadata: Option<Value>,
}

impl ServerInfo {
    /// Create new server info
    pub fn new(server: impl Into<String>, version: impl Into<String>, protocol: impl Into<String>) -> Self {
        Self {
            server: server.into(),
            version: version.into(),
            protocol: protocol.into(),
            features: None,
            operations_count: None,
            mcp_compatible: None,
            metadata: None,
        }
    }

    /// Set features (builder pattern)
    pub fn features(mut self, features: Vec<String>) -> Self {
        self.features = Some(features);
        self
    }

    /// Set operations count (builder pattern)
    pub fn operations_count(mut self, count: usize) -> Self {
        self.operations_count = Some(count);
        self
    }

    /// Set MCP compatibility (builder pattern)
    pub fn mcp_compatible(mut self, compatible: bool) -> Self {
        self.mcp_compatible = Some(compatible);
        self
    }

    /// Set metadata (builder pattern)
    pub fn metadata(mut self, metadata: Value) -> Self {
        self.metadata = Some(metadata);
        self
    }
}

/// Trait for services that support tool discovery
pub trait DiscoverableService {
    /// List all available operations with their schemas
    fn list_operations(&self) -> Vec<OperationSchema>;

    /// Get schema for a specific operation by name
    fn get_schema(&self, name: &str) -> Option<OperationSchema> {
        self.list_operations()
            .into_iter()
            .find(|op| op.name == name)
    }

    /// Get server information and metadata
    fn server_info(&self) -> ServerInfo;
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json::json;

    #[test]
    fn test_operation_schema_builder() {
        let schema = OperationSchema::new(
            "test_op",
            json!({
                "type": "object",
                "properties": {
                    "param1": {"type": "string"}
                }
            }),
        )
        .title("Test Operation")
        .description("A test operation")
        .annotations(json!({"read_only": true}));

        assert_eq!(schema.name, "test_op");
        assert_eq!(schema.title, Some("Test Operation".to_string()));
        assert_eq!(schema.description, Some("A test operation".to_string()));
        assert!(schema.annotations.is_some());
    }

    #[test]
    fn test_server_info_builder() {
        let info = ServerInfo::new("test-server", "1.0.0", "UMICP/2.0")
            .features(vec!["discovery".to_string(), "streaming".to_string()])
            .operations_count(42)
            .mcp_compatible(true);

        assert_eq!(info.server, "test-server");
        assert_eq!(info.version, "1.0.0");
        assert_eq!(info.protocol, "UMICP/2.0");
        assert_eq!(info.operations_count, Some(42));
        assert_eq!(info.mcp_compatible, Some(true));
    }

    #[test]
    fn test_serialize_operation_schema() {
        let schema = OperationSchema::new(
            "search",
            json!({
                "type": "object",
                "properties": {
                    "query": {"type": "string"}
                },
                "required": ["query"]
            }),
        )
        .title("Search")
        .description("Search for items");

        let serialized = serde_json::to_string(&schema).unwrap();
        assert!(serialized.contains("search"));
        assert!(serialized.contains("Search"));
    }
}

