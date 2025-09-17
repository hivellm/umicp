/*!
# UMICP Envelope

Type-safe message container with JSON serialization for UMICP protocol.
*/

use crate::error::{Result, UmicpError};
use crate::types::*;
use crate::utils::*;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

/// Internal envelope structure for JSON serialization
#[derive(Debug, Clone, Serialize, Deserialize)]
struct EnvelopeData {
    /// Protocol version
    v: String,
    /// Message ID (UUID)
    msg_id: String,
    /// Timestamp in ISO 8601 format
    ts: String,
    /// Sender identifier
    from: String,
    /// Recipient identifier
    to: String,
    /// Operation type
    op: String,
    /// Optional capabilities (metadata)
    #[serde(skip_serializing_if = "Option::is_none")]
    capabilities: Option<HashMap<String, String>>,
    /// Optional schema URI
    #[serde(skip_serializing_if = "Option::is_none")]
    schema_uri: Option<String>,
    /// Optional accepted content types
    #[serde(skip_serializing_if = "Option::is_none")]
    accept: Option<Vec<String>>,
    /// Optional payload hint
    #[serde(skip_serializing_if = "Option::is_none")]
    payload_hint: Option<PayloadHintData>,
    /// Optional payload references
    #[serde(skip_serializing_if = "Option::is_none")]
    payload_refs: Option<Vec<HashMap<String, String>>>,
}

/// Payload hint structure for JSON serialization
#[derive(Debug, Clone, Serialize, Deserialize)]
struct PayloadHintData {
    /// Payload type
    #[serde(rename = "type")]
    payload_type: String,
    /// Size in bytes
    #[serde(skip_serializing_if = "Option::is_none")]
    size: Option<u64>,
    /// Encoding type
    #[serde(skip_serializing_if = "Option::is_none")]
    encoding: Option<String>,
    /// Element count
    #[serde(skip_serializing_if = "Option::is_none")]
    count: Option<u64>,
}

/// UMICP Envelope - the main message container
#[derive(Debug, Clone)]
pub struct Envelope {
    /// Protocol version
    version: String,
    /// Unique message identifier
    message_id: String,
    /// Timestamp when envelope was created
    timestamp: String,
    /// Sender identifier
    from: String,
    /// Recipient identifier
    to: String,
    /// Operation type
    operation: OperationType,
    /// Optional capabilities/metadata
    capabilities: Option<Capabilities>,
    /// Optional schema URI
    schema_uri: Option<String>,
    /// Optional accepted content types
    accept: Option<AcceptTypes>,
    /// Optional payload hint
    payload_hint: Option<PayloadHint>,
    /// Optional payload references
    payload_refs: Option<PayloadRefs>,
}

impl Envelope {
    /// Create a new envelope with default values
    pub fn new() -> Self {
        Envelope {
            version: "1.0".to_string(),
            message_id: generate_uuid(),
            timestamp: get_current_timestamp(),
            from: String::new(),
            to: String::new(),
            operation: OperationType::Control,
            capabilities: None,
            schema_uri: None,
            accept: None,
            payload_hint: None,
            payload_refs: None,
        }
    }

    /// Create an envelope builder for fluent construction
    pub fn builder() -> EnvelopeBuilder {
        EnvelopeBuilder::new()
    }

    /// Serialize envelope to JSON string
    pub fn serialize(&self) -> Result<String> {
        let data = self.to_envelope_data();
        serde_json::to_string(&data)
            .map_err(|e| UmicpError::serialization(format!("Failed to serialize envelope: {}", e)))
    }

    /// Deserialize envelope from JSON string
    pub fn deserialize(json: &str) -> Result<Self> {
        let data: EnvelopeData = serde_json::from_str(json)
            .map_err(|e| UmicpError::serialization(format!("Failed to deserialize envelope: {}", e)))?;

        Self::from_envelope_data(data)
    }

    /// Validate envelope data
    pub fn validate(&self) -> Result<()> {
        validate_non_empty(&self.from, "from")?;
        validate_non_empty(&self.to, "to")?;
        validate_non_empty(&self.message_id, "message_id")?;
        if !validate_uuid(&self.message_id) {
            return Err(UmicpError::validation(format!("Invalid UUID format: {}", self.message_id)));
        }

        if let Some(capabilities) = &self.capabilities {
            for (key, value) in capabilities {
                validate_non_empty(key, "capability key")?;
                validate_non_empty(value, "capability value")?;
            }
        }

        if let Some(schema_uri) = &self.schema_uri {
            validate_non_empty(schema_uri, "schema_uri")?;
        }

        if let Some(accept) = &self.accept {
            for content_type in accept {
                validate_non_empty(content_type, "accept type")?;
            }
        }

        Ok(())
    }

    /// Generate hash of the envelope for integrity checking
    pub fn hash(&self) -> Result<String> {
        let serialized = self.serialize()?;
        Ok(generate_hash(serialized.as_bytes()))
    }

    /// Get protocol version
    pub fn version(&self) -> &str {
        &self.version
    }

    /// Get message ID
    pub fn message_id(&self) -> &str {
        &self.message_id
    }

    /// Get timestamp
    pub fn timestamp(&self) -> &str {
        &self.timestamp
    }

    /// Get sender identifier
    pub fn from(&self) -> &str {
        &self.from
    }

    /// Set sender identifier
    pub fn set_from(&mut self, from: &str) {
        self.from = from.to_string();
    }

    /// Get recipient identifier
    pub fn to(&self) -> &str {
        &self.to
    }

    /// Set recipient identifier
    pub fn set_to(&mut self, to: &str) {
        self.to = to.to_string();
    }

    /// Get operation type
    pub fn operation(&self) -> OperationType {
        self.operation
    }

    /// Set operation type
    pub fn set_operation(&mut self, operation: OperationType) {
        self.operation = operation;
    }

    /// Get capabilities
    pub fn capabilities(&self) -> Option<&Capabilities> {
        self.capabilities.as_ref()
    }

    /// Set capabilities
    pub fn set_capabilities(&mut self, capabilities: Capabilities) {
        self.capabilities = Some(capabilities);
    }

    /// Add a single capability
    pub fn add_capability(&mut self, key: &str, value: &str) {
        if self.capabilities.is_none() {
            self.capabilities = Some(HashMap::new());
        }
        if let Some(caps) = &mut self.capabilities {
            caps.insert(key.to_string(), value.to_string());
        }
    }

    /// Get schema URI
    pub fn schema_uri(&self) -> Option<&str> {
        self.schema_uri.as_deref()
    }

    /// Set schema URI
    pub fn set_schema_uri(&mut self, schema_uri: &str) {
        self.schema_uri = Some(schema_uri.to_string());
    }

    /// Get accepted content types
    pub fn accept(&self) -> Option<&AcceptTypes> {
        self.accept.as_ref()
    }

    /// Set accepted content types
    pub fn set_accept(&mut self, accept: AcceptTypes) {
        self.accept = Some(accept);
    }

    /// Get payload hint
    pub fn payload_hint(&self) -> Option<&PayloadHint> {
        self.payload_hint.as_ref()
    }

    /// Set payload hint
    pub fn set_payload_hint(&mut self, hint: PayloadHint) {
        self.payload_hint = Some(hint);
    }

    /// Get payload references
    pub fn payload_refs(&self) -> Option<&PayloadRefs> {
        self.payload_refs.as_ref()
    }

    /// Set payload references
    pub fn set_payload_refs(&mut self, refs: PayloadRefs) {
        self.payload_refs = Some(refs);
    }

    /// Convert to internal envelope data for serialization
    fn to_envelope_data(&self) -> EnvelopeData {
        EnvelopeData {
            v: self.version.clone(),
            msg_id: self.message_id.clone(),
            ts: self.timestamp.clone(),
            from: self.from.clone(),
            to: self.to.clone(),
            op: self.operation.to_string(),
            capabilities: self.capabilities.clone(),
            schema_uri: self.schema_uri.clone(),
            accept: self.accept.clone(),
            payload_hint: self.payload_hint.as_ref().map(|hint| PayloadHintData {
                payload_type: hint.payload_type.to_string(),
                size: hint.size,
                encoding: hint.encoding.map(|e| e.to_string()),
                count: hint.count,
            }),
            payload_refs: self.payload_refs.clone(),
        }
    }

    /// Convert from internal envelope data after deserialization
    fn from_envelope_data(data: EnvelopeData) -> Result<Self> {
        let operation = match data.op.as_str() {
            "control" => OperationType::Control,
            "data" => OperationType::Data,
            "ack" => OperationType::Ack,
            "error" => OperationType::Error,
            "request" => OperationType::Request,
            "response" => OperationType::Response,
            _ => return Err(UmicpError::validation(format!("Unknown operation type: {}", data.op))),
        };

        let payload_hint = if let Some(hint) = data.payload_hint {
            let payload_type = match hint.payload_type.as_str() {
                "vector" => PayloadType::Vector,
                "text" => PayloadType::Text,
                "metadata" => PayloadType::Metadata,
                "binary" => PayloadType::Binary,
                _ => return Err(UmicpError::validation(format!("Unknown payload type: {}", hint.payload_type))),
            };

            let encoding = if let Some(enc) = hint.encoding {
                Some(match enc.as_str() {
                    "float32" => EncodingType::Float32,
                    "float64" => EncodingType::Float64,
                    "int32" => EncodingType::Int32,
                    "int64" => EncodingType::Int64,
                    "uint8" => EncodingType::Uint8,
                    "uint16" => EncodingType::Uint16,
                    "uint32" => EncodingType::Uint32,
                    "uint64" => EncodingType::Uint64,
                    _ => return Err(UmicpError::validation(format!("Unknown encoding type: {}", enc))),
                })
            } else {
                None
            };

            Some(PayloadHint {
                payload_type,
                size: hint.size,
                encoding,
                count: hint.count,
            })
        } else {
            None
        };

        Ok(Envelope {
            version: data.v,
            message_id: data.msg_id,
            timestamp: data.ts,
            from: data.from,
            to: data.to,
            operation,
            capabilities: data.capabilities,
            schema_uri: data.schema_uri,
            accept: data.accept,
            payload_hint,
            payload_refs: data.payload_refs,
        })
    }
}

impl Default for Envelope {
    fn default() -> Self {
        Self::new()
    }
}

/// Builder pattern for creating envelopes
pub struct EnvelopeBuilder {
    envelope: Envelope,
}

impl EnvelopeBuilder {
    /// Create a new envelope builder
    pub fn new() -> Self {
        EnvelopeBuilder {
            envelope: Envelope::new(),
        }
    }

    /// Set the sender identifier
    pub fn from(mut self, from: &str) -> Self {
        self.envelope.set_from(from);
        self
    }

    /// Set the recipient identifier
    pub fn to(mut self, to: &str) -> Self {
        self.envelope.set_to(to);
        self
    }

    /// Set the operation type
    pub fn operation(mut self, operation: OperationType) -> Self {
        self.envelope.set_operation(operation);
        self
    }

    /// Set the message ID
    pub fn message_id(mut self, message_id: &str) -> Self {
        self.envelope.message_id = message_id.to_string();
        self
    }

    /// Add a capability
    pub fn capability(mut self, key: &str, value: &str) -> Self {
        self.envelope.add_capability(key, value);
        self
    }

    /// Set all capabilities
    pub fn capabilities(mut self, capabilities: Capabilities) -> Self {
        self.envelope.set_capabilities(capabilities);
        self
    }

    /// Set schema URI
    pub fn schema_uri(mut self, schema_uri: &str) -> Self {
        self.envelope.set_schema_uri(schema_uri);
        self
    }

    /// Set accepted content types
    pub fn accept(mut self, accept: AcceptTypes) -> Self {
        self.envelope.set_accept(accept);
        self
    }

    /// Set payload hint
    pub fn payload_hint(mut self, hint: PayloadHint) -> Self {
        self.envelope.set_payload_hint(hint);
        self
    }

    /// Set payload references
    pub fn payload_refs(mut self, refs: PayloadRefs) -> Self {
        self.envelope.set_payload_refs(refs);
        self
    }

    /// Build the envelope
    pub fn build(self) -> Result<Envelope> {
        self.envelope.validate()?;
        Ok(self.envelope)
    }
}

impl Default for EnvelopeBuilder {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_envelope_creation() {
        let envelope = Envelope::new();
        assert_eq!(envelope.version(), "1.0");
        assert!(!envelope.message_id().is_empty());
        assert!(!envelope.timestamp().is_empty());
        assert_eq!(envelope.operation(), OperationType::Control);
    }

    #[test]
    fn test_envelope_builder() {
        let envelope = Envelope::builder()
            .from("sender")
            .to("recipient")
            .operation(OperationType::Data)
            .capability("priority", "high")
            .build()
            .unwrap();

        assert_eq!(envelope.from(), "sender");
        assert_eq!(envelope.to(), "recipient");
        assert_eq!(envelope.operation(), OperationType::Data);
        assert_eq!(envelope.capabilities().unwrap().get("priority").unwrap(), "high");
    }

    #[test]
    fn test_envelope_serialization() {
        let envelope = Envelope::builder()
            .from("test-from")
            .to("test-to")
            .operation(OperationType::Data)
            .capability("test", "value")
            .build()
            .unwrap();

        let serialized = envelope.serialize().unwrap();
        let deserialized = Envelope::deserialize(&serialized).unwrap();

        assert_eq!(deserialized.from(), envelope.from());
        assert_eq!(deserialized.to(), envelope.to());
        assert_eq!(deserialized.operation(), envelope.operation());
        assert_eq!(deserialized.capabilities(), envelope.capabilities());
    }

    #[test]
    fn test_envelope_validation() {
        // Valid envelope
        let valid = Envelope::builder()
            .from("sender")
            .to("recipient")
            .build();
        assert!(valid.is_ok());

        // Invalid - empty from
        let invalid = Envelope::builder()
            .from("")
            .to("recipient")
            .build();
        assert!(invalid.is_err());

        // Invalid - empty to
        let invalid = Envelope::builder()
            .from("sender")
            .to("")
            .build();
        assert!(invalid.is_err());
    }
}
