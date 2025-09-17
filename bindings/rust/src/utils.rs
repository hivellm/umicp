/*!
# UMICP Utilities

Utility functions for UMICP operations.
*/

use crate::error::{Result, UmicpError};
use chrono::{DateTime, Utc};
use sha2::{Digest, Sha256};
use uuid::Uuid;

/// Generate a new UUID v4
pub fn generate_uuid() -> String {
    Uuid::new_v4().to_string()
}

/// Get current timestamp in ISO 8601 format
pub fn get_current_timestamp() -> String {
    Utc::now().to_rfc3339()
}

/// Parse timestamp from ISO 8601 format
pub fn parse_timestamp(timestamp: &str) -> Result<DateTime<Utc>> {
    DateTime::parse_from_rfc3339(timestamp)
        .map_err(|e| UmicpError::validation(format!("Invalid timestamp format: {}", e)))
        .map(|dt| dt.with_timezone(&Utc))
}

/// Generate SHA-256 hash of data
pub fn generate_hash(data: &[u8]) -> String {
    let mut hasher = Sha256::new();
    hasher.update(data);
    let result = hasher.finalize();
    hex::encode(result)
}

/// Validate UUID format
pub fn validate_uuid(uuid_str: &str) -> bool {
    Uuid::parse_str(uuid_str).is_ok()
}

/// Validate that a string is not empty
pub fn validate_non_empty(value: &str, field_name: &str) -> Result<()> {
    if value.trim().is_empty() {
        return Err(UmicpError::validation(format!(
            "Field '{}' cannot be empty",
            field_name
        )));
    }
    Ok(())
}

/// Validate that a number is positive
pub fn validate_positive(value: f64, field_name: &str) -> Result<()> {
    if value <= 0.0 {
        return Err(UmicpError::validation(format!(
            "Field '{}' must be positive, got {}",
            field_name, value
        )));
    }
    Ok(())
}

/// Validate that an index is within bounds
pub fn validate_index(index: usize, max_index: usize, field_name: &str) -> Result<()> {
    if index >= max_index {
        return Err(UmicpError::validation(format!(
            "Field '{}' index {} is out of bounds (max: {})",
            field_name, index, max_index
        )));
    }
    Ok(())
}

/// Base64 encode data
pub fn base64_encode(data: &[u8]) -> String {
    base64::encode(data)
}

/// Base64 decode data
pub fn base64_decode(data: &str) -> Result<Vec<u8>> {
    base64::decode(data).map_err(|e| UmicpError::validation(format!("Invalid base64: {}", e)))
}

/// Convert bytes to human readable format
pub fn format_bytes(bytes: u64) -> String {
    const UNITS: &[&str] = &["B", "KB", "MB", "GB", "TB"];
    let mut size = bytes as f64;
    let mut unit_index = 0;

    while size >= 1024.0 && unit_index < UNITS.len() - 1 {
        size /= 1024.0;
        unit_index += 1;
    }

    if unit_index == 0 {
        format!("{} {}", bytes, UNITS[0])
    } else {
        format!("{:.2} {}", size, UNITS[unit_index])
    }
}

/// Get elapsed time in human readable format
pub fn format_duration(seconds: u64) -> String {
    let days = seconds / 86400;
    let hours = (seconds % 86400) / 3600;
    let minutes = (seconds % 3600) / 60;
    let secs = seconds % 60;

    if days > 0 {
        format!("{}d {}h {}m {}s", days, hours, minutes, secs)
    } else if hours > 0 {
        format!("{}h {}m {}s", hours, minutes, secs)
    } else if minutes > 0 {
        format!("{}m {}s", minutes, secs)
    } else {
        format!("{}s", secs)
    }
}

/// Sanitize string for safe usage
pub fn sanitize_string(input: &str) -> String {
    input
        .chars()
        .filter(|c| c.is_alphanumeric() || *c == '_' || *c == '-' || *c == '.')
        .collect()
}

/// Check if string contains only ASCII characters
pub fn is_ascii_only(input: &str) -> bool {
    input.chars().all(|c| c.is_ascii())
}

/// Truncate string to maximum length
pub fn truncate_string(input: &str, max_length: usize) -> String {
    if input.len() <= max_length {
        input.to_string()
    } else {
        format!("{}...", &input[..max_length.saturating_sub(3)])
    }
}

/// Parse version string
pub fn parse_version(version: &str) -> Result<(u32, u32, u32)> {
    let parts: Vec<&str> = version.split('.').collect();
    if parts.len() != 3 {
        return Err(UmicpError::validation(format!(
            "Invalid version format: {}. Expected x.y.z",
            version
        )));
    }

    let major = parts[0].parse().map_err(|_| {
        UmicpError::validation(format!("Invalid major version: {}", parts[0]))
    })?;

    let minor = parts[1].parse().map_err(|_| {
        UmicpError::validation(format!("Invalid minor version: {}", parts[1]))
    })?;

    let patch = parts[2].parse().map_err(|_| {
        UmicpError::validation(format!("Invalid patch version: {}", parts[2]))
    })?;

    Ok((major, minor, patch))
}

/// Compare versions
pub fn compare_versions(version1: &str, version2: &str) -> Result<std::cmp::Ordering> {
    let v1 = parse_version(version1)?;
    let v2 = parse_version(version2)?;

    Ok(v1.cmp(&v2))
}
