/*!
# UMICP Error Types

Error handling for UMICP operations.
*/

use thiserror::Error;

/// Main error type for UMICP operations
#[derive(Error, Debug)]
pub enum UmicpError {
    /// Serialization/deserialization error
    #[error("Serialization error: {message}")]
    Serialization { message: String },

    /// Transport layer error
    #[error("Transport error: {message}")]
    Transport { message: String },

    /// Matrix operation error
    #[error("Matrix operation error: {message}")]
    Matrix { message: String },

    /// Validation error
    #[error("Validation error: {message}")]
    Validation { message: String },

    /// Connection error
    #[error("Connection error: {message}")]
    Connection { message: String },

    /// Authentication error
    #[error("Authentication error: {message}")]
    Authentication { message: String },

    /// Configuration error
    #[error("Configuration error: {message}")]
    Configuration { message: String },

    /// I/O error
    #[error("I/O error: {0}")]
    Io(#[from] std::io::Error),

    /// JSON parsing error
    #[error("JSON error: {0}")]
    Json(#[from] serde_json::Error),

    /// UUID generation error
    #[error("UUID error: {0}")]
    Uuid(#[from] uuid::Error),

    /// WebSocket error
    #[cfg(feature = "websocket")]
    #[error("WebSocket error: {0}")]
    WebSocket(#[from] tokio_tungstenite::tungstenite::Error),

    /// HTTP/2 error
    #[cfg(feature = "http2")]
    #[error("HTTP/2 error: {message}")]
    Http2 { message: String },

    /// Generic error
    #[error("Error: {message}")]
    Generic { message: String },
}

/// Result type alias for UMICP operations
pub type Result<T> = std::result::Result<T, UmicpError>;

/// Convenience functions for creating errors
impl UmicpError {
    /// Create a serialization error
    pub fn serialization<S: Into<String>>(message: S) -> Self {
        UmicpError::Serialization {
            message: message.into(),
        }
    }

    /// Create a transport error
    pub fn transport<S: Into<String>>(message: S) -> Self {
        UmicpError::Transport {
            message: message.into(),
        }
    }

    /// Create a matrix operation error
    pub fn matrix<S: Into<String>>(message: S) -> Self {
        UmicpError::Matrix {
            message: message.into(),
        }
    }

    /// Create a validation error
    pub fn validation<S: Into<String>>(message: S) -> Self {
        UmicpError::Validation {
            message: message.into(),
        }
    }

    /// Create a connection error
    pub fn connection<S: Into<String>>(message: S) -> Self {
        UmicpError::Connection {
            message: message.into(),
        }
    }

    /// Create an authentication error
    pub fn authentication<S: Into<String>>(message: S) -> Self {
        UmicpError::Authentication {
            message: message.into(),
        }
    }

    /// Create a configuration error
    pub fn configuration<S: Into<String>>(message: S) -> Self {
        UmicpError::Configuration {
            message: message.into(),
        }
    }

    /// Create a generic error
    pub fn generic<S: Into<String>>(message: S) -> Self {
        UmicpError::Generic {
            message: message.into(),
        }
    }
}
