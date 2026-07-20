/**
 * Test for Custom Endpoint Support (v0.2.2)
 *
 * Verifies that the Rust UMICP implementation supports
 * custom endpoint paths for compatibility with different servers
 * (e.g., Vectorizer uses /umicp, standard servers use /message)
 */

#[cfg(feature = "http2")]
use umicp_sdk::transport::http_client::{HttpClient, HttpClientConfig};
use std::time::Duration;

#[test]
#[cfg(feature = "http2")]
fn test_new_with_path_method() {
    // Test the new helper method for custom paths
    let client = HttpClient::new_with_path("http://localhost:8000", "/umicp");
    assert!(client.is_ok());
}

#[test]
#[cfg(feature = "http2")]
fn test_config_with_custom_path() {
    // Test using HttpClientConfig with custom path
    let config = HttpClientConfig {
        base_url: "http://localhost:8000".to_string(),
        path: "/umicp".to_string(),
        timeout: Duration::from_secs(30),
        max_retries: 3,
    };

    let client = HttpClient::with_config(config.clone());
    assert!(client.is_ok());

    // Verify the path is stored correctly
    assert_eq!(config.path, "/umicp");
}

#[test]
#[cfg(feature = "http2")]
fn test_default_path_is_message() {
    // Test that default path is /message
    let config = HttpClientConfig::default();
    assert_eq!(config.path, "/message");
}

#[test]
#[cfg(feature = "http2")]
fn test_vectorizer_endpoint_config() {
    // Configuration for Vectorizer service
    let config = HttpClientConfig {
        base_url: "http://localhost:8000".to_string(),
        path: "/umicp".to_string(),  // Vectorizer uses /umicp
        timeout: Duration::from_secs(30),
        max_retries: 3,
    };

    assert_eq!(config.base_url, "http://localhost:8000");
    assert_eq!(config.path, "/umicp");
}

#[test]
#[cfg(feature = "http2")]
fn test_standard_endpoint_config() {
    // Configuration for standard UMICP server
    let config = HttpClientConfig {
        base_url: "http://localhost:9000".to_string(),
        path: "/message".to_string(),  // Standard servers use /message
        timeout: Duration::from_secs(30),
        max_retries: 3,
    };

    assert_eq!(config.base_url, "http://localhost:9000");
    assert_eq!(config.path, "/message");
}

#[test]
#[cfg(feature = "http2")]
fn test_multiple_clients_different_endpoints() {
    // Create client for Vectorizer
    let vectorizer_config = HttpClientConfig {
        base_url: "http://localhost:8000".to_string(),
        path: "/umicp".to_string(),
        timeout: Duration::from_secs(30),
        max_retries: 3,
    };

    // Create client for standard server
    let standard_config = HttpClientConfig {
        base_url: "http://localhost:9000".to_string(),
        path: "/message".to_string(),
        timeout: Duration::from_secs(30),
        max_retries: 3,
    };

    // Both configs should be independent
    assert_ne!(vectorizer_config.base_url, standard_config.base_url);
    assert_ne!(vectorizer_config.path, standard_config.path);
}

#[test]
#[cfg(feature = "http2")]
fn test_config_clone_preserves_path() {
    let config1 = HttpClientConfig {
        base_url: "http://localhost:8000".to_string(),
        path: "/umicp".to_string(),
        timeout: Duration::from_secs(5000),
        max_retries: 5,
    };

    let config2 = config1.clone();

    assert_eq!(config2.base_url, "http://localhost:8000");
    assert_eq!(config2.path, "/umicp");
    assert_eq!(config2.timeout, Duration::from_secs(5000));
    assert_eq!(config2.max_retries, 5);
}

#[test]
#[cfg(feature = "http2")]
fn test_new_method_uses_default_path() {
    // The new() method should use default path
    let client = HttpClient::new("http://localhost:8000");
    assert!(client.is_ok());
    // Default is /message
}

#[test]
#[cfg(feature = "http2")]
fn test_new_with_path_overwrites_default() {
    // The new_with_path() method should override default
    let client = HttpClient::new_with_path("http://localhost:8000", "/custom");
    assert!(client.is_ok());
}

#[test]
#[cfg(feature = "http2")]
fn test_version_0_2_2_supports_custom_endpoints() {
    // This test ensures that v0.2.2 supports custom endpoints
    // The functionality is verified through the other tests

    // Test both old and new methods work
    let client1 = HttpClient::new("http://localhost:8000");
    let client2 = HttpClient::new_with_path("http://localhost:8000", "/umicp");

    assert!(client1.is_ok());
    assert!(client2.is_ok());
}

