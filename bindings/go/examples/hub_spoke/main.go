package main

import (
	"context"
	"fmt"
	"log"
	"time"

	"github.com/hivellm/umicp-go/pkg/peer"
	"github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
	fmt.Println("=== UMICP Hub-and-Spoke Topology Example ===\n")

	ctx := context.Background()

	// Start hub (central node with server)
	go runHub(ctx)
	time.Sleep(time.Second)

	// Start 3 spokes (workers that connect to hub)
	go runSpoke(ctx, "worker-1")
	time.Sleep(500 * time.Millisecond)

	go runSpoke(ctx, "worker-2")
	time.Sleep(500 * time.Millisecond)

	runSpoke(ctx, "worker-3")
}

func runHub(ctx context.Context) {
	hub, err := peer.NewPeerWithServer(
		peer.PeerConfig{
			PeerID:       "hub",
			AutoProtocol: true,
		},
		peer.ServerConfig{
			Addr: "127.0.0.1:22000",
			Path: "/umicp",
		},
	)
	if err != nil {
		log.Fatal(err)
	}

	hub.On(peer.EventPeerConnect, func(e *peer.Event) {
		fmt.Printf("[HUB] Worker connected: %s\n", e.Connection.ID)
	})

	hub.On(peer.EventPeerReady, func(e *peer.Event) {
		fmt.Printf("[HUB] Worker ready: %s\n", e.PeerInfo.PeerID)
	})

	hub.On(peer.EventData, func(e *peer.Event) {
		fmt.Printf("[HUB] Data from %s: %s\n",
			e.Envelope.From, e.Envelope.Capabilities["message"])

		// Broadcast to all other workers
		broadcast, _ := umicp.NewEnvelope().
			From("hub").
			To("*").
			Operation(umicp.OperationData).
			Capability("relay", e.Envelope.Capabilities["message"]).
			Capability("from_worker", e.Envelope.From).
			Build()

		count := hub.Broadcast(ctx, broadcast, "")
		fmt.Printf("[HUB] Relayed to %d workers\n", count)
	})

	hub.Start(ctx)
	fmt.Println("[HUB] Started on 127.0.0.1:22000")

	// Send periodic broadcasts
	go func() {
		ticker := time.NewTicker(5 * time.Second)
		defer ticker.Stop()

		for range ticker.C {
			env, _ := umicp.NewEnvelope().
				From("hub").
				To("*").
				Operation(umicp.OperationData).
				Capability("message", "Status check").
				Build()

			count := hub.Broadcast(ctx, env, "")
			fmt.Printf("[HUB] Broadcast status check to %d workers\n", count)

			stats := hub.GetStats()
			fmt.Printf("[HUB] Stats: %d workers, %d handshakes complete\n",
				stats.IncomingConnections, stats.HandshakesComplete)
		}
	}()

	select {}
}

func runSpoke(ctx context.Context, workerID string) {
	spoke := peer.NewPeer(peer.PeerConfig{
		PeerID:       workerID,
		AutoProtocol: true,
	})

	spoke.On(peer.EventPeerReady, func(e *peer.Event) {
		fmt.Printf("[%s] Connected to hub\n", workerID)
	})

	spoke.On(peer.EventData, func(e *peer.Event) {
		fmt.Printf("[%s] Hub says: %s\n",
			workerID, e.Envelope.Capabilities["message"])
	})

	// Connect to hub
	peerID, err := spoke.ConnectToPeer(ctx, "ws://127.0.0.1:22000/umicp", nil)
	if err != nil {
		log.Fatalf("[%s] Failed to connect: %v", workerID, err)
	}

	fmt.Printf("[%s] Connected to hub\n", workerID)

	// Send periodic messages to hub
	ticker := time.NewTicker(10 * time.Second)
	defer ticker.Stop()

	for range ticker.C {
		env, _ := umicp.NewEnvelope().
			From(workerID).
			To("hub").
			Operation(umicp.OperationData).
			Capability("message", fmt.Sprintf("Update from %s", workerID)).
			Build()

		spoke.SendToPeer(ctx, peerID, env)
	}
}
