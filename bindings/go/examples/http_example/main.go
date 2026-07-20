package main

import (
	"context"
	"fmt"
	"log"
	"time"

	httpTransport "github.com/hivellm/umicp-go/pkg/transport/http"
	"github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
	fmt.Println("=== UMICP HTTP/2 Example ===")

	// Start server
	go runHTTPServer()

	// Wait for server to start
	time.Sleep(time.Second)

	// Run client
	runHTTPClient()
}

func runHTTPServer() {
	config := httpTransport.DefaultServerConfig()
	config.Addr = "127.0.0.1:8081"
	config.Path = "/umicp"

	server := httpTransport.NewServer(*config)

	// Setup request handler
	server.OnRequest(func(env *umicp.Envelope) (*umicp.Envelope, error) {
		fmt.Printf("[SERVER] Request from %s: %s\n",
			env.From, env.Capabilities["message"])

		// Create response
		response, _ := umicp.NewEnvelope().
			From("server").
			To(env.From).
			Operation(umicp.OperationResponse).
			Capability("status", "success").
			Capability("echo", env.Capabilities["message"]).
			Build()

		return response, nil
	})

	ctx := context.Background()
	fmt.Println("[SERVER] Starting on 127.0.0.1:8081/umicp")

	if err := server.Start(ctx); err != nil {
		log.Fatalf("[SERVER] Failed to start: %v", err)
	}
}

func runHTTPClient() {
	config := httpTransport.DefaultClientConfig()
	config.BaseURL = "http://127.0.0.1:8081"

	client := httpTransport.NewClient(*config)
	defer client.Close()

	ctx := context.Background()

	// Connect
	if err := client.Connect(ctx); err != nil {
		log.Fatalf("[CLIENT] Failed to connect: %v", err)
	}
	defer client.Disconnect(ctx)

	fmt.Println("[CLIENT] Connected, sending requests...")

	// Send 5 requests
	for i := 1; i <= 5; i++ {
		env, _ := umicp.NewEnvelope().
			From("client").
			To("server").
			Operation(umicp.OperationRequest).
			Capability("message", fmt.Sprintf("Request #%d", i)).
			Build()

		fmt.Printf("[CLIENT] Sending request #%d\n", i)

		err := client.Send(ctx, env)
		if err != nil {
			log.Printf("[CLIENT] Error: %v", err)
			continue
		}

		fmt.Printf("[CLIENT] Request #%d sent successfully\n", i)

		time.Sleep(500 * time.Millisecond)
	}

	// Print statistics
	stats := client.Stats()
	fmt.Printf("\n[CLIENT] Statistics:\n")
	fmt.Printf("  Messages Sent: %d\n", stats.MessagesSent)
	fmt.Printf("  Bytes Sent: %d\n", stats.BytesSent)
	fmt.Printf("  Errors: %d\n", stats.Errors)
	fmt.Printf("  Uptime: %v\n", stats.Uptime)

	fmt.Println("\n=== Example Complete ===")
}
