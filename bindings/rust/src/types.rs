/*!
# UMICP Types

Core type definitions for the UMICP protocol.
*/

use serde::{Deserialize, Serialize};
use std::collections::HashMap;

/// Operation types for UMICP messages
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum OperationType {
    /// Control message for protocol management
    Control = 0,
    /// Regular data message
    Data = 1,
    /// Acknowledgment message
    Ack = 2,
    /// Error message
    Error = 3,
    /// Request message
    Request = 4,
    /// Response message
    Response = 5,
}

impl Default for OperationType {
    fn default() -> Self {
        OperationType::Control
    }
}

impl std::fmt::Display for OperationType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let op_str = match self {
            OperationType::Control => "control",
            OperationType::Data => "data",
            OperationType::Ack => "ack",
            OperationType::Error => "error",
            OperationType::Request => "request",
            OperationType::Response => "response",
        };
        write!(f, "{}", op_str)
    }
}

/// Payload types for message content
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum PayloadType {
    /// Vector data (numeric arrays)
    Vector = 0,
    /// Text data
    Text = 1,
    /// Metadata information
    Metadata = 2,
    /// Binary data
    Binary = 3,
}

impl Default for PayloadType {
    fn default() -> Self {
        PayloadType::Text
    }
}

impl std::fmt::Display for PayloadType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let type_str = match self {
            PayloadType::Vector => "vector",
            PayloadType::Text => "text",
            PayloadType::Metadata => "metadata",
            PayloadType::Binary => "binary",
        };
        write!(f, "{}", type_str)
    }
}

/// Encoding types for numeric data
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum EncodingType {
    /// 32-bit floating point
    Float32 = 0,
    /// 64-bit floating point
    Float64 = 1,
    /// 32-bit signed integer
    Int32 = 2,
    /// 64-bit signed integer
    Int64 = 3,
    /// 8-bit unsigned integer
    Uint8 = 4,
    /// 16-bit unsigned integer
    Uint16 = 5,
    /// 32-bit unsigned integer
    Uint32 = 6,
    /// 64-bit unsigned integer
    Uint64 = 7,
}

impl Default for EncodingType {
    fn default() -> Self {
        EncodingType::Float32
    }
}

impl std::fmt::Display for EncodingType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let enc_str = match self {
            EncodingType::Float32 => "float32",
            EncodingType::Float64 => "float64",
            EncodingType::Int32 => "int32",
            EncodingType::Int64 => "int64",
            EncodingType::Uint8 => "uint8",
            EncodingType::Uint16 => "uint16",
            EncodingType::Uint32 => "uint32",
            EncodingType::Uint64 => "uint64",
        };
        write!(f, "{}", enc_str)
    }
}

/// Payload hint for message content description
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PayloadHint {
    /// Type of payload
    pub payload_type: PayloadType,
    /// Size in bytes (optional)
    pub size: Option<u64>,
    /// Encoding type for numeric data (optional)
    pub encoding: Option<EncodingType>,
    /// Number of elements (optional)
    pub count: Option<u64>,
}

impl Default for PayloadHint {
    fn default() -> Self {
        PayloadHint {
            payload_type: PayloadType::Text,
            size: None,
            encoding: None,
            count: None,
        }
    }
}

/// Transport statistics
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TransportStats {
    /// Total messages sent
    pub messages_sent: u64,
    /// Total messages received
    pub messages_received: u64,
    /// Total bytes sent
    pub bytes_sent: u64,
    /// Total bytes received
    pub bytes_received: u64,
    /// Current connection count
    pub active_connections: u32,
    /// Total connection count
    pub total_connections: u64,
    /// Uptime in seconds
    pub uptime_seconds: u64,
    /// Average latency in milliseconds
    pub avg_latency_ms: Option<f64>,
}

/// Connection information
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ConnectionInfo {
    /// Unique connection ID
    pub id: String,
    /// Remote address
    pub remote_addr: String,
    /// Local address
    pub local_addr: String,
    /// Connection established timestamp
    pub connected_at: chrono::DateTime<chrono::Utc>,
    /// Last activity timestamp
    pub last_activity: chrono::DateTime<chrono::Utc>,
}

/// Matrix operation result
#[derive(Debug, Clone)]
pub struct MatrixResult {
    /// Operation success status
    pub success: bool,
    /// Error message if operation failed
    pub error: Option<String>,
    /// Numeric result (for dot product, etc.)
    pub result: Option<f64>,
    /// Similarity score (for cosine similarity)
    pub similarity: Option<f64>,
    /// Vector/matrix data result
    pub data: Option<Vec<f32>>,
}

/// Frame options for advanced messaging
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct FrameOptions {
    /// Frame type identifier
    pub frame_type: Option<u32>,
    /// Stream ID for multiplexing
    pub stream_id: Option<u32>,
    /// Sequence number
    pub sequence: Option<u64>,
    /// Frame flags
    pub flags: Option<u32>,
    /// Compression enabled
    pub compressed: bool,
    /// Encryption enabled
    pub encrypted: bool,
}

impl Default for FrameOptions {
    fn default() -> Self {
        FrameOptions {
            frame_type: None,
            stream_id: None,
            sequence: None,
            flags: None,
            compressed: false,
            encrypted: false,
        }
    }
}

/// Transport configuration
#[derive(Debug, Clone)]
pub struct TransportConfig {
    /// Maximum payload size in bytes
    pub max_payload_size: usize,
    /// Heartbeat interval in seconds
    pub heartbeat_interval: u64,
    /// Maximum reconnection attempts
    pub max_reconnect_attempts: u32,
    /// Connection timeout in seconds
    pub connection_timeout: u64,
    /// Enable compression
    pub compression_enabled: bool,
    /// Enable TLS/SSL
    pub tls_enabled: bool,
    /// TLS certificate path (optional)
    pub tls_cert_path: Option<String>,
    /// TLS private key path (optional)
    pub tls_key_path: Option<String>,
}

impl Default for TransportConfig {
    fn default() -> Self {
        TransportConfig {
            max_payload_size: 1024 * 1024, // 1MB
            heartbeat_interval: 30,
            max_reconnect_attempts: 3,
            connection_timeout: 10,
            compression_enabled: true,
            tls_enabled: false,
            tls_cert_path: None,
            tls_key_path: None,
        }
    }
}

/// Envelope capabilities (key-value metadata)
pub type Capabilities = HashMap<String, String>;

/// Accepted content types
pub type AcceptTypes = Vec<String>;

/// Payload references for multi-part messages
pub type PayloadRefs = Vec<HashMap<String, String>>;
