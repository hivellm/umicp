package main

import (
	"context"
	"fmt"
	"log"
	"time"

	"github.com/hivellm/umicp-go/pkg/transport/websocket"
	"github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
	fmt.Println("=== UMICP WebSocket Example ===")

	// Start server in goroutine
	go runServer()

	// Give server time to start
	time.Sleep(time.Second)

	// Start client
	runClient()
}

func runServer() {
	config := websocket.DefaultServerConfig()
	config.Addr = "127.0.0.1:8080"
	config.Path = "/umicp"

	server := websocket.NewServer(*config)

	// Setup event handlers
	server.OnConnect(func(conn *websocket.Connection) {
		fmt.Printf("[SERVER] Client connected: %s\n", conn.ID)
	})

	server.OnMessage(func(env *umicp.Envelope, conn *websocket.Connection) {
		fmt.Printf("[SERVER] Message from %s: %s\n", env.From, env.Capabilities["message"])

		// Send acknowledgment
		ack, _ := umicp.NewEnvelope().
			From("server").
			To(env.From).
			Operation(umicp.OperationAck).
			Capability("status", "received").
			Capability("original_id", env.MessageID).
			Build()

		ctx := context.Background()
		if err := server.Send(ctx, conn.ID, ack); err != nil {
			fmt.Printf("[SERVER] Send error: %v\n", err)
		}
	})

	server.OnDisconnect(func(conn *websocket.Connection) {
		fmt.Printf("[SERVER] Client disconnected: %s\n", conn.ID)
	})

	server.OnError(func(err error) {
		log.Printf("[SERVER] Error: %v", err)
	})

	// Start server
	ctx := context.Background()
	fmt.Println("[SERVER] Starting on 127.0.0.1:8080/umicp")

	if err := server.Start(ctx); err != nil {
		log.Fatalf("[SERVER] Failed to start: %v", err)
	}
}

func runClient() {
	config := websocket.DefaultClientConfig()
	config.URL = "ws://127.0.0.1:8080/umicp"

	client := websocket.NewClient(*config)

	// Setup event handlers
	client.OnConnect(func() {
		fmt.Println("[CLIENT] Connected to server")
	})

	client.OnMessage(func(env *umicp.Envelope) {
		fmt.Printf("[CLIENT] Received ACK from %s: %s\n",
			env.From, env.Capabilities["status"])
	})

	client.OnDisconnect(func(err error) {
		if err != nil {
			fmt.Printf("[CLIENT] Disconnected with error: %v\n", err)
		} else {
			fmt.Println("[CLIENT] Disconnected")
		}
	})

	client.OnError(func(err error) {
		log.Printf("[CLIENT] Error: %v", err)
	})

	// Connect
	ctx := context.Background()
	if err := client.Connect(ctx); err != nil {
		log.Fatalf("[CLIENT] Failed to connect: %v", err)
	}
	defer client.Disconnect(ctx)

	fmt.Println("[CLIENT] Connected, sending messages...")

	// Send messages
	for i := 1; i <= 5; i++ {
		env, _ := umicp.NewEnvelope().
			From("client").
			To("server").
			Operation(umicp.OperationData).
			Capability("message", fmt.Sprintf("Hello #%d", i)).
			Capability("index", fmt.Sprintf("%d", i)).
			Build()

		if err := client.Send(ctx, env); err != nil {
			log.Printf("[CLIENT] Send error: %v", err)
		} else {
			fmt.Printf("[CLIENT] Sent message #%d\n", i)
		}

		time.Sleep(500 * time.Millisecond)
	}

	// Wait for responses
	time.Sleep(2 * time.Second)

	// Print statistics
	stats := client.Stats()
	fmt.Printf("\n[CLIENT] Statistics:\n")
	fmt.Printf("  Messages Sent: %d\n", stats.MessagesSent)
	fmt.Printf("  Messages Received: %d\n", stats.MessagesReceived)
	fmt.Printf("  Bytes Sent: %d\n", stats.BytesSent)
	fmt.Printf("  Bytes Received: %d\n", stats.BytesReceived)

	fmt.Println("\n=== Example Complete ===")
}
