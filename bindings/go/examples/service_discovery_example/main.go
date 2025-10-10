package main

import (
	"context"
	"fmt"
	"time"

	"github.com/hivellm/umicp-go/pkg/discovery"
)

func main() {
	fmt.Println("=== UMICP Service Discovery Example ===")

	// Create service discovery manager
	sd := discovery.NewServiceDiscovery()

	// Register local service
	fmt.Println("1. Registering local service...")
	local := sd.RegisterLocalService(
		"my-ai-service",
		"ws://localhost:8080",
		[]string{"chat", "text-generation", "embeddings"},
	)
	local.AddMetadata("model", "gpt-4")
	local.AddMetadata("region", "us-east-1")
	fmt.Printf("   ✓ Registered: %s (%s)\n", local.Name, local.ServiceID)

	// Register remote services
	fmt.Println("\n2. Discovering remote services...")
	
	service1 := discovery.NewServiceInfo("svc-001", "translation-service", "ws://localhost:8081")
	service1.AddCapability("translation")
	service1.AddCapability("language-detection")
	service1.AddMetadata("languages", "en,es,fr,de")
	sd.RegisterService(service1)
	fmt.Printf("   ✓ Discovered: %s\n", service1.Name)

	service2 := discovery.NewServiceInfo("svc-002", "vision-service", "ws://localhost:8082")
	service2.AddCapability("image-analysis")
	service2.AddCapability("object-detection")
	service2.AddMetadata("model", "yolo-v8")
	sd.RegisterService(service2)
	fmt.Printf("   ✓ Discovered: %s\n", service2.Name)

	service3 := discovery.NewServiceInfo("svc-003", "chat-service", "ws://localhost:8083")
	service3.AddCapability("chat")
	service3.AddCapability("conversation-memory")
	sd.RegisterService(service3)
	fmt.Printf("   ✓ Discovered: %s\n", service3.Name)

	// List all services
	fmt.Println("\n3. Listing all services:")
	services := sd.ListServices()
	for _, svc := range services {
		fmt.Printf("   • %s (%s) - %s\n", svc.Name, svc.ServiceID[:8], svc.Address)
		fmt.Printf("     Capabilities: %v\n", svc.Capabilities)
	}

	// Find services by capability
	fmt.Println("\n4. Finding services with 'chat' capability:")
	chatServices := sd.FindServicesByCapability("chat")
	for _, svc := range chatServices {
		fmt.Printf("   • %s - %s\n", svc.Name, svc.Address)
	}

	// Get specific service
	fmt.Println("\n5. Getting service by ID:")
	if svc, ok := sd.GetService("svc-001"); ok {
		fmt.Printf("   ✓ Found: %s\n", svc.Name)
		if langs, ok := svc.GetMetadata("languages"); ok {
			fmt.Printf("     Supported languages: %s\n", langs)
		}
	}

	// Update service health
	fmt.Println("\n6. Updating service health...")
	sd.UpdateServiceHealth("svc-001")
	sd.UpdateServiceHealth("svc-002")
	fmt.Println("   ✓ Health updated for active services")

	// Simulate stale service
	fmt.Println("\n7. Simulating stale service detection...")
	staleService := discovery.NewServiceInfo("svc-old", "old-service", "ws://localhost:9999")
	staleService.LastSeen = time.Now().Add(-2 * time.Minute)
	sd.RegisterService(staleService)

	// Check stats before cleanup
	stats := sd.GetStats()
	fmt.Printf("   Before cleanup - Total: %d, Active: %d, Stale: %d\n",
		stats.TotalServices, stats.ActiveServices, stats.StaleServices)

	// Cleanup stale services
	removed := sd.CleanupStaleServices()
	fmt.Printf("   ✓ Cleaned up %d stale service(s)\n", removed)

	// Check stats after cleanup
	stats = sd.GetStats()
	fmt.Printf("   After cleanup - Total: %d, Active: %d, Stale: %d\n",
		stats.TotalServices, stats.ActiveServices, stats.StaleServices)

	// Start auto cleanup
	fmt.Println("\n8. Starting auto-cleanup (5s interval)...")
	ctx, cancel := context.WithTimeout(context.Background(), 15*time.Second)
	defer cancel()

	sd.StartAutoCleanup(ctx, 5*time.Second)
	defer sd.StopAutoCleanup()

	// Simulate service updates
	fmt.Println("\n9. Simulating service lifecycle...")
	for i := 0; i < 3; i++ {
		time.Sleep(2 * time.Second)
		
		// Update some services
		sd.UpdateServiceHealth("svc-001")
		sd.UpdateServiceHealth("svc-002")
		
		stats := sd.GetStats()
		fmt.Printf("   [%ds] Services: %d active, %d stale\n",
			(i+1)*2, stats.ActiveServices, stats.StaleServices)
	}

	// Final stats
	fmt.Println("\n10. Final statistics:")
	finalStats := sd.GetStats()
	fmt.Printf("   Total Services: %d\n", finalStats.TotalServices)
	fmt.Printf("   Active Services: %d\n", finalStats.ActiveServices)
	fmt.Printf("   Stale Services: %d\n", finalStats.StaleServices)
	fmt.Printf("   Has Local Service: %v\n", finalStats.HasLocalService)

	// Find services by name
	fmt.Println("\n11. Finding services by name pattern:")
	localServices := sd.FindServicesByName("my-ai-service")
	for _, svc := range localServices {
		fmt.Printf("   • %s\n", svc.Name)
		for k, v := range svc.Metadata {
			fmt.Printf("     %s: %s\n", k, v)
		}
	}

	fmt.Println("\n✓ Service Discovery example completed!")
}

