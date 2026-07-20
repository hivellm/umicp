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
	fmt.Println("=== UMICP Multiplexed Peer Example ===")

	// Start 3 interconnected peers
	go runPeerA()
	time.Sleep(500 * time.Millisecond)

	go runPeerB()
	time.Sleep(500 * time.Millisecond)

	runPeerC()
}

func runPeerA() {
	ctx := context.Background()

	// Create peer A with server
	peerA, err := peer.NewPeerWithServer(
		peer.PeerConfig{
			PeerID:       "peer-a",
			AutoProtocol: true,
		},
		peer.ServerConfig{
			Addr: "127.0.0.1:20081",
			Path: "/umicp",
		},
	)
	if err != nil {
		log.Fatalf("[PEER-A] Failed to create: %v", err)
	}

	// Setup event handlers
	peerA.On(peer.EventPeerConnect, func(e *peer.Event) {
		fmt.Printf("[PEER-A] Peer connected: %s (%s)\n",
			e.Connection.ID, e.Connection.Type)
	})

	peerA.On(peer.EventPeerReady, func(e *peer.Event) {
		fmt.Printf("[PEER-A] Handshake complete with: %s\n",
			e.PeerInfo.PeerID)
	})

	peerA.On(peer.EventData, func(e *peer.Event) {
		fmt.Printf("[PEER-A] Data from %s: %s\n",
			e.Envelope.From, e.Envelope.Capabilities["message"])
	})

	// Start server
	if err := peerA.Start(ctx); err != nil {
		log.Fatalf("[PEER-A] Start error: %v", err)
	}

	fmt.Println("[PEER-A] Started on 127.0.0.1:20081")

	// Keep running
	select {}
}

func runPeerB() {
	ctx := context.Background()

	// Create peer B with server
	peerB, err := peer.NewPeerWithServer(
		peer.PeerConfig{
			PeerID:       "peer-b",
			AutoProtocol: true,
		},
		peer.ServerConfig{
			Addr: "127.0.0.1:20082",
			Path: "/umicp",
		},
	)
	if err != nil {
		log.Fatalf("[PEER-B] Failed to create: %v", err)
	}

	// Setup event handlers
	peerB.On(peer.EventPeerConnect, func(e *peer.Event) {
		fmt.Printf("[PEER-B] Peer connected: %s (%s)\n",
			e.Connection.ID, e.Connection.Type)
	})

	peerB.On(peer.EventPeerReady, func(e *peer.Event) {
		fmt.Printf("[PEER-B] Handshake complete with: %s\n",
			e.PeerInfo.PeerID)
	})

	peerB.On(peer.EventData, func(e *peer.Event) {
		fmt.Printf("[PEER-B] Data from %s: %s\n",
			e.Envelope.From, e.Envelope.Capabilities["message"])
	})

	// Start server
	if err := peerB.Start(ctx); err != nil {
		log.Fatalf("[PEER-B] Start error: %v", err)
	}

	fmt.Println("[PEER-B] Started on 127.0.0.1:20082")

	// Connect to peer A
	time.Sleep(time.Second)
	if _, err := peerB.ConnectToPeer(ctx, "ws://127.0.0.1:20081/umicp", nil); err != nil {
		log.Printf("[PEER-B] Failed to connect to A: %v", err)
	} else {
		fmt.Println("[PEER-B] Connected to Peer A")
	}

	// Keep running
	select {}
}

func runPeerC() {
	ctx := context.Background()

	// Create peer C (client-only, no server)
	peerC := peer.NewPeer(peer.PeerConfig{
		PeerID:       "peer-c",
		AutoProtocol: true,
	})

	// Setup event handlers
	peerC.On(peer.EventPeerConnect, func(e *peer.Event) {
		fmt.Printf("[PEER-C] Peer connected: %s (%s)\n",
			e.Connection.ID, e.Connection.Type)
	})

	peerC.On(peer.EventPeerReady, func(e *peer.Event) {
		fmt.Printf("[PEER-C] Handshake complete with: %s\n",
			e.PeerInfo.PeerID)
	})

	peerC.On(peer.EventData, func(e *peer.Event) {
		fmt.Printf("[PEER-C] Data from %s: %s\n",
			e.Envelope.From, e.Envelope.Capabilities["message"])
	})

	fmt.Println("[PEER-C] Starting (client-only mode)")

	// Connect to both A and B
	time.Sleep(time.Second)

	if _, err := peerC.ConnectToPeer(ctx, "ws://127.0.0.1:20081/umicp", nil); err != nil {
		log.Printf("[PEER-C] Failed to connect to A: %v", err)
	} else {
		fmt.Println("[PEER-C] Connected to Peer A")
	}

	if _, err := peerC.ConnectToPeer(ctx, "ws://127.0.0.1:20082/umicp", nil); err != nil {
		log.Printf("[PEER-C] Failed to connect to B: %v", err)
	} else {
		fmt.Println("[PEER-C] Connected to Peer B")
	}

	// Wait for handshakes
	time.Sleep(2 * time.Second)

	// Send messages
	fmt.Println("\n[PEER-C] Sending messages...")

	env, _ := umicp.NewEnvelope().
		From("peer-c").
		To("*").
		Operation(umicp.OperationData).
		Capability("message", "Hello from Peer C!").
		Build()

	count := peerC.Broadcast(ctx, env, "")
	fmt.Printf("[PEER-C] Broadcast sent to %d peers\n", count)

	// Print statistics
	time.Sleep(time.Second)
	stats := peerC.GetStats()
	fmt.Printf("\n[PEER-C] Statistics:\n")
	fmt.Printf("  Total Peers: %d\n", stats.TotalPeers)
	fmt.Printf("  Outgoing Connections: %d\n", stats.OutgoingConnections)
	fmt.Printf("  Handshakes Complete: %d\n", stats.HandshakesComplete)

	// Keep running for a while
	time.Sleep(5 * time.Second)

	// Shutdown
	peerC.Shutdown(ctx)
	fmt.Println("\n[PEER-C] Shutting down...")
	fmt.Println("\n=== Example Complete ===")
}
