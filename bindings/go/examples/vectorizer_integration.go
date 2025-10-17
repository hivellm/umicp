// UMICP Vectorizer Integration Example
//
// Demonstrates how to use the UMICP Go SDK with the Vectorizer service
// which uses the /umicp endpoint.

package main

import (
	"context"
	"encoding/json"
	"fmt"
	"time"

	"github.com/hivellm/umicp-go/pkg/transport/http"
	"github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
	fmt.Println("========================================")
	fmt.Println("UMICP Vectorizer Integration Example")
	fmt.Println("========================================\n")

	// Example 1: Discovery operation
	if err := exampleDiscovery(); err != nil {
		fmt.Printf("⚠️  Discovery example failed: %v\n", err)
	}

	// Example 2: Data operation
	if err := exampleDataOperation(); err != nil {
		fmt.Printf("⚠️  Data operation example failed: %v\n", err)
	}

	fmt.Println("\n========================================")
	fmt.Println("Examples completed! ✅")
	fmt.Println("========================================")
}

func exampleDiscovery() error {
	fmt.Println("📝 Example 1: Discovery Operation")
	fmt.Println("   Creating client for Vectorizer at http://localhost:8000/umicp\n")

	// Create client config with custom path for Vectorizer
	config := http.ClientConfig{
		BaseURL:         "http://localhost:8000",
		Path:            "/umicp", // Vectorizer uses /umicp
		Timeout:         30 * time.Second,
		MaxIdleConns:    10,
		IdleConnTimeout: 90 * time.Second,
	}

	client := http.NewClient(config)
	defer client.Close()

	// Connect
	ctx := context.Background()
	if err := client.Connect(ctx); err != nil {
		return fmt.Errorf("connect failed: %w", err)
	}

	// Create discovery envelope
	env := umicp.NewEnvelope(
		"go-example",
		"vectorizer",
		umicp.OperationRequest,
		umicp.WithMessageID("discover-001"),
		umicp.WithCapabilities(map[string]interface{}{
			"operation": "discover",
			"type":      "operations",
		}),
	)

	fmt.Println("📤 Sending discovery request to Vectorizer...")
	fmt.Printf("   Endpoint: %s\n", config.Path)
	fmt.Printf("   From: %s\n", env.GetFrom())
	fmt.Printf("   To: %s\n", env.GetTo())

	// Send envelope
	if err := client.Send(ctx, env); err != nil {
		fmt.Printf("⚠️  Request failed (expected if Vectorizer not running): %v\n", err)
	} else {
		fmt.Println("✅ Discovery request sent successfully!")
	}

	// Show stats
	stats := client.Stats()
	fmt.Println("\n📊 Client Statistics:")
	fmt.Printf("   Messages sent: %d\n", stats.MessagesSent)
	fmt.Printf("   Bytes sent: %d\n", stats.BytesSent)
	fmt.Printf("   Errors: %d\n", stats.Errors)

	return nil
}

func exampleDataOperation() error {
	fmt.Println("\n📝 Example 2: Data Operation")
	fmt.Println("   Sending data to Vectorizer\n")

	// Create client with custom path
	config := http.ClientConfig{
		BaseURL: "http://localhost:8000",
		Path:    "/umicp", // Vectorizer endpoint
		Timeout: 30 * time.Second,
	}

	client := http.NewClient(config)
	defer client.Close()

	ctx := context.Background()
	if err := client.Connect(ctx); err != nil {
		return fmt.Errorf("connect failed: %w", err)
	}

	// Create data envelope
	env := umicp.NewEnvelope(
		"go-example",
		"vectorizer",
		umicp.OperationData,
		umicp.WithMessageID("data-001"),
		umicp.WithCapabilities(map[string]interface{}{
			"action":     "query",
			"collection": "test_collection",
			"query":      "test query",
		}),
	)

	fmt.Println("📤 Sending data envelope to Vectorizer...")
	fmt.Println("   Operation: DATA")

	// Send envelope
	if err := client.Send(ctx, env); err != nil {
		fmt.Printf("⚠️  Request failed (expected if Vectorizer not running): %v\n", err)
	} else {
		fmt.Println("✅ Data envelope sent successfully!")
	}

	// Show envelope as JSON
	data, _ := env.Serialize()
	var pretty map[string]interface{}
	json.Unmarshal(data, &pretty)
	prettyJSON, _ := json.MarshalIndent(pretty, "   ", "  ")
	fmt.Println("\n📦 Envelope:")
	fmt.Println(string(prettyJSON))

	return nil
}

