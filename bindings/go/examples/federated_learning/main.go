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
	fmt.Println("=== UMICP Federated Learning Example ===\n")

	ctx := context.Background()

	// Start coordinator
	go runCoordinator(ctx)
	time.Sleep(time.Second)

	// Start workers
	go runWorker(ctx, "worker-1", 23001)
	time.Sleep(500 * time.Millisecond)

	runWorker(ctx, "worker-2", 23002)
}

func runCoordinator(ctx context.Context) {
	coordinator, err := peer.NewPeerWithServer(
		peer.PeerConfig{
			PeerID:       "coordinator",
			AutoProtocol: true,
		},
		peer.ServerConfig{
			Addr: "127.0.0.1:23000",
			Path: "/umicp",
		},
	)
	if err != nil {
		log.Fatal(err)
	}

	workerGradients := make(map[string][]float32)

	coordinator.On(peer.EventPeerReady, func(e *peer.Event) {
		fmt.Printf("[COORDINATOR] Worker connected: %s\n", e.PeerInfo.PeerID)

		// Send initial model weights
		weights := []float32{0.1, 0.2, 0.3, 0.4}
		env, _ := umicp.NewEnvelope().
			From("coordinator").
			To(e.PeerInfo.PeerID).
			Operation(umicp.OperationData).
			Capability("type", "model_weights").
			Capability("weights", fmt.Sprintf("%v", weights)).
			Build()

		// Get peer ID from peers list
		peers := coordinator.GetPeers()
		for _, p := range peers {
			if p.PeerInfo != nil && p.PeerInfo.PeerID == e.PeerInfo.PeerID {
				coordinator.SendToPeer(ctx, p.ID, env)
				break
			}
		}
	})

	coordinator.On(peer.EventData, func(e *peer.Event) {
		if e.Envelope.Capabilities["type"] == "gradients" {
			fmt.Printf("[COORDINATOR] Received gradients from %s\n", e.Envelope.From)

			// Parse gradients (simplified)
			gradients := []float32{0.01, 0.02, 0.03, 0.04}
			workerGradients[e.Envelope.From] = gradients

			// If all workers reported, aggregate
			if len(workerGradients) >= 2 {
				fmt.Println("[COORDINATOR] Aggregating gradients...")
				aggregated := aggregateGradients(workerGradients)
				fmt.Printf("[COORDINATOR] Aggregated: %v\n", aggregated)

				// Broadcast updated model
				env, _ := umicp.NewEnvelope().
					From("coordinator").
					To("*").
					Operation(umicp.OperationData).
					Capability("type", "model_update").
					Capability("weights", fmt.Sprintf("%v", aggregated)).
					Build()

				count := coordinator.Broadcast(ctx, env, "")
				fmt.Printf("[COORDINATOR] Broadcast update to %d workers\n", count)

				// Reset for next round
				workerGradients = make(map[string][]float32)
			}
		}
	})

	coordinator.Start(ctx)
	fmt.Println("[COORDINATOR] Started on 127.0.0.1:23000")

	select {}
}

func runWorker(ctx context.Context, workerID string, port int) {
	worker := peer.NewPeer(peer.PeerConfig{
		PeerID:       workerID,
		AutoProtocol: true,
	})

	var peerID string

	worker.On(peer.EventPeerReady, func(e *peer.Event) {
		fmt.Printf("[%s] Connected to coordinator\n", workerID)
	})

	worker.On(peer.EventData, func(e *peer.Event) {
		msgType := e.Envelope.Capabilities["type"]

		if msgType == "model_weights" {
			fmt.Printf("[%s] Received model weights from coordinator\n", workerID)

			// Simulate local training
			time.Sleep(time.Second)

			// Compute gradients
			gradients := []float32{0.01, 0.02, 0.03, 0.04}
			fmt.Printf("[%s] Computed gradients: %v\n", workerID, gradients)

			// Send back to coordinator
			env, _ := umicp.NewEnvelope().
				From(workerID).
				To("coordinator").
				Operation(umicp.OperationData).
				Capability("type", "gradients").
				Capability("gradients", fmt.Sprintf("%v", gradients)).
				Build()

			worker.SendToPeer(ctx, peerID, env)
			fmt.Printf("[%s] Sent gradients to coordinator\n", workerID)
		} else if msgType == "model_update" {
			fmt.Printf("[%s] Received model update from coordinator\n", workerID)
		}
	})

	// Connect to coordinator
	id, err := worker.ConnectToPeer(ctx, "ws://127.0.0.1:23000/umicp", nil)
	if err != nil {
		log.Fatalf("[%s] Failed to connect: %v", workerID, err)
	}
	peerID = id

	fmt.Printf("[%s] Connected to coordinator\n", workerID)

	select {}
}

func aggregateGradients(gradients map[string][]float32) []float32 {
	if len(gradients) == 0 {
		return nil
	}

	// Get size from first gradient
	var size int
	for _, g := range gradients {
		size = len(g)
		break
	}

	// Average gradients
	result := make([]float32, size)
	count := float32(len(gradients))

	for _, g := range gradients {
		for i := range g {
			result[i] += g[i] / count
		}
	}

	return result
}
