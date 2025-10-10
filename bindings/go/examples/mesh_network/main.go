package main

import (
	"context"
	"fmt"
	"log"
	"sync"
	"time"

	"github.com/hivellm/umicp-go/pkg/peer"
	"github.com/hivellm/umicp-go/pkg/umicp"
)

func main() {
	fmt.Println("=== UMICP Full Mesh Network Example ===\n")

	ctx := context.Background()
	var wg sync.WaitGroup

	// Create 3 peers in full mesh
	peers := make([]*peer.Peer, 3)
	ports := []int{21001, 21002, 21003}

	// Start each peer with server
	for i := 0; i < 3; i++ {
		peerID := fmt.Sprintf("peer-%d", i+1)
		addr := fmt.Sprintf("127.0.0.1:%d", ports[i])

		p, err := peer.NewPeerWithServer(
			peer.PeerConfig{
				PeerID:       peerID,
				AutoProtocol: true,
			},
			peer.ServerConfig{
				Addr: addr,
				Path: "/umicp",
			},
		)
		if err != nil {
			log.Fatal(err)
		}

		// Event handlers
		p.On(peer.EventPeerReady, func(e *peer.Event) {
			fmt.Printf("[%s] Handshake complete with %s\n",
				peerID, e.PeerInfo.PeerID)
		})

		p.On(peer.EventData, func(e *peer.Event) {
			fmt.Printf("[%s] Data from %s: %s\n",
				peerID, e.Envelope.From, e.Envelope.Capabilities["message"])
		})

		p.Start(ctx)
		peers[i] = p

		fmt.Printf("Started %s on %s\n", peerID, addr)
	}

	time.Sleep(time.Second)

	// Connect in full mesh
	fmt.Println("\nConnecting peers in full mesh...")
	for i, p := range peers {
		for j, port := range ports {
			if i != j {
				url := fmt.Sprintf("ws://127.0.0.1:%d/umicp", port)
				wg.Add(1)
				go func(peer *peer.Peer, url string, from, to int) {
					defer wg.Done()
					if _, err := peer.ConnectToPeer(ctx, url, nil); err != nil {
						log.Printf("Peer %d -> %d error: %v", from+1, to+1, err)
					}
				}(p, url, i, j)
			}
		}
	}

	wg.Wait()
	time.Sleep(3 * time.Second)

	// Broadcast from each peer
	fmt.Println("\nBroadcasting messages...")
	for i, p := range peers {
		env, _ := umicp.NewEnvelope().
			From(fmt.Sprintf("peer-%d", i+1)).
			To("*").
			Operation(umicp.OperationData).
			Capability("message", fmt.Sprintf("Hello from peer-%d", i+1)).
			Build()

		count := p.Broadcast(ctx, env, "")
		fmt.Printf("Peer-%d broadcast to %d peers\n", i+1, count)
	}

	time.Sleep(2 * time.Second)

	// Print statistics
	fmt.Println("\nNetwork Statistics:")
	for i, p := range peers {
		stats := p.GetStats()
		fmt.Printf("Peer-%d: %d total (%d in, %d out) - %d handshakes complete\n",
			i+1, stats.TotalPeers,
			stats.IncomingConnections,
			stats.OutgoingConnections,
			stats.HandshakesComplete)
	}

	fmt.Println("\nMesh network running for 10 seconds...")
	time.Sleep(10 * time.Second)

	// Cleanup
	for _, p := range peers {
		p.Shutdown(ctx)
	}

	fmt.Println("\n=== Example Complete ===")
}
