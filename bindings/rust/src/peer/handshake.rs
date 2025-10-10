/*!
# Handshake Protocol

Auto-handshake protocol for peer discovery and capability exchange.
*/

use crate::error::{Result, UmicpError};
use crate::peer::PeerInfo;
use crate::{Envelope, OperationType};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

/// Handshake message types
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "UPPERCASE")]
pub enum HandshakeType {
    /// Initial handshake request
    Hello,

    /// Handshake acknowledgment
    Ack,

    /// Handshake error
    Error,
}

/// Handshake message payload
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct HandshakeMessage {
    /// Handshake type
    #[serde(rename = "type")]
    pub msg_type: HandshakeType,

    /// Peer ID
    pub peer_id: String,

    /// Peer capabilities
    #[serde(default)]
    pub capabilities: Vec<String>,

    /// Peer metadata
    #[serde(default)]
    pub metadata: HashMap<String, String>,

    /// Protocol version
    pub version: String,

    /// Error message (for Error type)
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error: Option<String>,
}

impl HandshakeMessage {
    /// Create HELLO message
    pub fn hello(peer_id: impl Into<String>) -> Self {
        Self {
            msg_type: HandshakeType::Hello,
            peer_id: peer_id.into(),
            capabilities: Vec::new(),
            metadata: HashMap::new(),
            version: "1.0".to_string(),
            error: None,
        }
    }

    /// Create ACK message
    pub fn ack(peer_id: impl Into<String>) -> Self {
        Self {
            msg_type: HandshakeType::Ack,
            peer_id: peer_id.into(),
            capabilities: Vec::new(),
            metadata: HashMap::new(),
            version: "1.0".to_string(),
            error: None,
        }
    }

    /// Create ERROR message
    pub fn error(peer_id: impl Into<String>, error: impl Into<String>) -> Self {
        Self {
            msg_type: HandshakeType::Error,
            peer_id: peer_id.into(),
            capabilities: Vec::new(),
            metadata: HashMap::new(),
            version: "1.0".to_string(),
            error: Some(error.into()),
        }
    }

    /// Add capability
    pub fn with_capability(mut self, capability: impl Into<String>) -> Self {
        self.capabilities.push(capability.into());
        self
    }

    /// Add metadata
    pub fn with_metadata(mut self, key: impl Into<String>, value: impl Into<String>) -> Self {
        self.metadata.insert(key.into(), value.into());
        self
    }

    /// Set version
    pub fn with_version(mut self, version: impl Into<String>) -> Self {
        self.version = version.into();
        self
    }

    /// Convert to envelope
    pub fn to_envelope(&self) -> Result<Envelope> {
        let json = serde_json::to_string(self)
            .map_err(|e| UmicpError::serialization(format!("Failed to serialize handshake: {}", e)))?;

        Envelope::builder()
            .from(&self.peer_id)
            .to("*")
            .operation(OperationType::Control)
            .message_id(&format!("handshake-{}", uuid::Uuid::new_v4()))
            .capability("handshake", &format!("{:?}", self.msg_type))
            .capability("payload", &json)
            .build()
    }

    /// Parse from envelope
    pub fn from_envelope(envelope: &Envelope) -> Result<Self> {
        let payload = envelope
            .capabilities()
            .and_then(|caps| caps.get("payload").map(|s| s.as_str()))
            .ok_or_else(|| UmicpError::validation("Missing handshake payload".to_string()))?;

        serde_json::from_str(payload)
            .map_err(|e| UmicpError::serialization(format!("Failed to parse handshake: {}", e)))
    }

    /// Check if envelope is a handshake message
    pub fn is_handshake(envelope: &Envelope) -> bool {
        envelope
            .capabilities()
            .and_then(|caps| caps.get("handshake"))
            .is_some()
    }

    /// Extract peer info from handshake
    pub fn to_peer_info(&self) -> PeerInfo {
        let mut info = PeerInfo::new(&self.peer_id, crate::peer::PeerType::Client);

        for cap in &self.capabilities {
            info = info.with_capability(cap);
        }

        for (key, value) in &self.metadata {
            info = info.with_metadata(key, value);
        }

        info = info.with_version(&self.version);

        info
    }
}

/// Handshake protocol handler
pub struct HandshakeProtocol {
    peer_id: String,
    capabilities: Vec<String>,
    metadata: HashMap<String, String>,
    version: String,
}

impl HandshakeProtocol {
    /// Create new handshake protocol
    pub fn new(peer_id: impl Into<String>) -> Self {
        Self {
            peer_id: peer_id.into(),
            capabilities: Vec::new(),
            metadata: HashMap::new(),
            version: "1.0".to_string(),
        }
    }

    /// Add capability
    pub fn add_capability(&mut self, capability: impl Into<String>) {
        self.capabilities.push(capability.into());
    }

    /// Add metadata
    pub fn add_metadata(&mut self, key: impl Into<String>, value: impl Into<String>) {
        self.metadata.insert(key.into(), value.into());
    }

    /// Create HELLO envelope
    pub fn create_hello(&self) -> Result<Envelope> {
        let mut msg = HandshakeMessage::hello(&self.peer_id)
            .with_version(&self.version);

        for cap in &self.capabilities {
            msg = msg.with_capability(cap);
        }

        for (k, v) in &self.metadata {
            msg = msg.with_metadata(k, v);
        }

        msg.to_envelope()
    }

    /// Create ACK envelope in response to HELLO
    pub fn create_ack(&self, hello: &HandshakeMessage) -> Result<Envelope> {
        let mut msg = HandshakeMessage::ack(&self.peer_id)
            .with_version(&self.version);

        for cap in &self.capabilities {
            msg = msg.with_capability(cap);
        }

        for (k, v) in &self.metadata {
            msg = msg.with_metadata(k, v);
        }

        let mut envelope = msg.to_envelope()?;

        // Set 'to' field to the sender of HELLO
        envelope = Envelope::builder()
            .from(envelope.from())
            .to(&hello.peer_id)
            .operation(envelope.operation())
            .message_id(envelope.message_id())
            .capability("handshake", "Ack")
            .capability("payload", envelope.capabilities()
                .and_then(|c| c.get("payload"))
                .map(|s| s.as_str())
                .unwrap_or(""))
            .build()?;

        Ok(envelope)
    }

    /// Handle incoming handshake message
    pub fn handle_handshake(&self, envelope: &Envelope) -> Result<Option<Envelope>> {
        if !HandshakeMessage::is_handshake(envelope) {
            return Ok(None);
        }

        let msg = HandshakeMessage::from_envelope(envelope)?;

        match msg.msg_type {
            HandshakeType::Hello => {
                // Respond with ACK
                Ok(Some(self.create_ack(&msg)?))
            }
            HandshakeType::Ack => {
                // Handshake complete
                tracing::info!("Handshake completed with peer: {}", msg.peer_id);
                Ok(None)
            }
            HandshakeType::Error => {
                tracing::error!("Handshake error from {}: {:?}", msg.peer_id, msg.error);
                Ok(None)
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_handshake_message_creation() {
        let hello = HandshakeMessage::hello("peer-1")
            .with_capability("sync")
            .with_metadata("app", "test")
            .with_version("1.0.0");

        assert_eq!(hello.msg_type, HandshakeType::Hello);
        assert_eq!(hello.peer_id, "peer-1");
        assert_eq!(hello.capabilities, vec!["sync"]);
        assert_eq!(hello.metadata.get("app"), Some(&"test".to_string()));
        assert_eq!(hello.version, "1.0.0");
    }

    #[test]
    fn test_handshake_envelope_conversion() -> Result<()> {
        let hello = HandshakeMessage::hello("peer-1")
            .with_capability("sync");

        let envelope = hello.to_envelope()?;

        assert!(HandshakeMessage::is_handshake(&envelope));

        let parsed = HandshakeMessage::from_envelope(&envelope)?;
        assert_eq!(parsed.peer_id, "peer-1");
        assert_eq!(parsed.msg_type, HandshakeType::Hello);

        Ok(())
    }

    #[test]
    fn test_handshake_protocol() -> Result<()> {
        let mut protocol = HandshakeProtocol::new("peer-1");
        protocol.add_capability("sync");
        protocol.add_metadata("app", "test");

        let hello_env = protocol.create_hello()?;
        assert!(HandshakeMessage::is_handshake(&hello_env));

        let hello = HandshakeMessage::from_envelope(&hello_env)?;
        let ack_env = protocol.create_ack(&hello)?;

        let ack = HandshakeMessage::from_envelope(&ack_env)?;
        assert_eq!(ack.msg_type, HandshakeType::Ack);

        Ok(())
    }
}

