package main

import (
	"fmt"
	"log"

	"github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
	fmt.Println("=== UMICP Go Bindings - Basic Example ===")

	// Example 1: Create a basic envelope
	fmt.Println("1. Creating a basic envelope...")
	envelope, err := umicp.NewEnvelope().
		From("client-001").
		To("server-001").
		Operation(umicp.OperationData).
		Capability("content-type", "application/json").
		Capability("priority", "high").
		Build()

	if err != nil {
		log.Fatalf("Failed to create envelope: %v", err)
	}

	fmt.Printf("   Created envelope: %s -> %s\n", envelope.From, envelope.To)
	fmt.Printf("   Message ID: %s\n", envelope.MessageID)
	fmt.Printf("   Operation: %s\n", envelope.Operation)
	fmt.Printf("   Capabilities: %v\n\n", envelope.Capabilities)

	// Example 2: Serialize envelope
	fmt.Println("2. Serializing envelope...")
	data, err := envelope.Serialize()
	if err != nil {
		log.Fatalf("Failed to serialize: %v", err)
	}

	fmt.Printf("   Serialized to %d bytes\n", len(data))
	fmt.Printf("   JSON: %s\n\n", string(data))

	// Example 3: Deserialize envelope
	fmt.Println("3. Deserializing envelope...")
	received, err := umicp.DeserializeEnvelope(data)
	if err != nil {
		log.Fatalf("Failed to deserialize: %v", err)
	}

	fmt.Printf("   From: %s\n", received.From)
	fmt.Printf("   To: %s\n", received.To)
	fmt.Printf("   Message ID: %s\n", received.MessageID)
	fmt.Printf("   Capabilities: %v\n\n", received.Capabilities)

	// Example 4: Calculate hash
	fmt.Println("4. Calculating envelope hash...")
	hash := envelope.Hash()
	fmt.Printf("   Hash: %s\n\n", hash)

	// Example 5: Clone envelope
	fmt.Println("5. Cloning envelope...")
	cloned := envelope.Clone()
	fmt.Printf("   Original ID: %s\n", envelope.MessageID)
	fmt.Printf("   Cloned ID: %s\n", cloned.MessageID)
	fmt.Printf("   Are same? %v\n\n", envelope.MessageID == cloned.MessageID)

	// Example 6: Different operation types
	fmt.Println("6. Creating different operation types...")

	control, _ := umicp.NewEnvelope().
		From("controller").
		To("worker").
		Operation(umicp.OperationControl).
		Capability("command", "shutdown").
		Build()
	fmt.Printf("   Control: %s\n", control.Operation)

	ack, _ := umicp.NewEnvelope().
		From("server").
		To("client").
		Operation(umicp.OperationAck).
		Capability("status", "received").
		Build()
	fmt.Printf("   Ack: %s\n", ack.Operation)

	errorMsg, _ := umicp.NewEnvelope().
		From("service").
		To("client").
		Operation(umicp.OperationError).
		Capability("error", "not found").
		Build()
	fmt.Printf("   Error: %s\n\n", errorMsg.Operation)

	fmt.Println("=== Example completed successfully ===")
}
