package http

import (
	"testing"
	"time"
)

// Test custom endpoint configuration
func TestClientConfig_CustomPath(t *testing.T) {
	config := ClientConfig{
		BaseURL: "http://localhost:8000",
		Path:    "/umicp", // Custom path for Vectorizer
		Timeout: 30 * time.Second,
	}

	if config.Path != "/umicp" {
		t.Errorf("Expected path /umicp, got %s", config.Path)
	}
}

// Test default path
func TestClientConfig_DefaultPath(t *testing.T) {
	config := DefaultClientConfig()

	// Default path should be /umicp
	if config.Path != "/umicp" {
		t.Errorf("Expected default path /umicp, got %s", config.Path)
	}
}

// Test Vectorizer endpoint configuration
func TestClientConfig_VectorizerEndpoint(t *testing.T) {
	config := ClientConfig{
		BaseURL: "http://localhost:8000",
		Path:    "/umicp",
		Timeout: 30 * time.Second,
	}

	client := NewClient(config)
	if client == nil {
		t.Fatal("Failed to create client")
	}

	// Verify path is set
	if client.config.Path != "/umicp" {
		t.Errorf("Expected path /umicp, got %s", client.config.Path)
	}
}

// Test standard endpoint configuration
func TestClientConfig_StandardEndpoint(t *testing.T) {
	config := ClientConfig{
		BaseURL: "http://localhost:9000",
		Path:    "/message",
		Timeout: 30 * time.Second,
	}

	client := NewClient(config)
	if client == nil {
		t.Fatal("Failed to create client")
	}

	// Verify path is set
	if client.config.Path != "/message" {
		t.Errorf("Expected path /message, got %s", client.config.Path)
	}
}

// Test empty path defaults to /umicp
func TestClientConfig_EmptyPathUsesDefault(t *testing.T) {
	config := ClientConfig{
		BaseURL: "http://localhost:8000",
		Path:    "", // Empty path
	}

	client := NewClient(config)
	if client == nil {
		t.Fatal("Failed to create client")
	}

	// Should use default /umicp
	if client.config.Path != "/umicp" {
		t.Errorf("Expected default path /umicp, got %s", client.config.Path)
	}
}

// Test multiple clients with different endpoints
func TestClient_MultipleEndpoints(t *testing.T) {
	// Client for Vectorizer
	vectorizerConfig := ClientConfig{
		BaseURL: "http://localhost:8000",
		Path:    "/umicp",
	}
	vectorizerClient := NewClient(vectorizerConfig)

	// Client for standard server
	standardConfig := ClientConfig{
		BaseURL: "http://localhost:9000",
		Path:    "/message",
	}
	standardClient := NewClient(standardConfig)

	// Both should be independent
	if vectorizerClient.config.Path == standardClient.config.Path {
		t.Error("Clients should have different paths")
	}

	if vectorizerClient.config.BaseURL == standardClient.config.BaseURL {
		t.Error("Clients should have different base URLs")
	}
}

// Test client creation with custom path
func TestNewClient_WithCustomPath(t *testing.T) {
	config := ClientConfig{
		BaseURL: "http://localhost:8000",
		Path:    "/custom/endpoint",
	}

	client := NewClient(config)
	if client == nil {
		t.Fatal("Failed to create client")
	}

	if client.config.Path != "/custom/endpoint" {
		t.Errorf("Expected path /custom/endpoint, got %s", client.config.Path)
	}
}

// Test backward compatibility - old code still works
func TestClient_BackwardCompatibility(t *testing.T) {
	// Old code without Path field
	config := ClientConfig{
		BaseURL: "http://localhost:8000",
		// Path not specified - should use default
	}

	client := NewClient(config)
	if client == nil {
		t.Fatal("Failed to create client")
	}

	// Should use default
	if client.config.Path != "/umicp" {
		t.Errorf("Expected default path /umicp for backward compatibility, got %s", client.config.Path)
	}
}

// Test version 0.2.2 supports custom endpoints
func TestVersion_0_2_2_SupportsCustomEndpoints(t *testing.T) {
	// Test that clients can be created with different endpoints
	config1 := ClientConfig{
		BaseURL: "http://localhost:8000",
		Path:    "/umicp",
	}
	client1 := NewClient(config1)

	config2 := ClientConfig{
		BaseURL: "http://localhost:9000",
		Path:    "/message",
	}
	client2 := NewClient(config2)

	if client1 == nil || client2 == nil {
		t.Fatal("Failed to create clients")
	}

	// Verify paths are different
	if client1.config.Path == client2.config.Path {
		t.Error("Clients should support different paths")
	}
}

// Test config validation initializes empty path
func TestClient_ConfigValidation(t *testing.T) {
	config := ClientConfig{
		BaseURL: "http://localhost:8000",
		// All other fields zero/empty
	}

	client := NewClient(config)
	if client == nil {
		t.Fatal("Failed to create client")
	}

	// Path should be initialized
	if client.config.Path == "" {
		t.Error("Path should be initialized to default")
	}

	// Timeout should be initialized
	if client.config.Timeout == 0 {
		t.Error("Timeout should be initialized to default")
	}
}
