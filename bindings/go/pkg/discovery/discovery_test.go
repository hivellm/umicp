package discovery

import (
	"context"
	"testing"
	"time"
)

func TestServiceInfo(t *testing.T) {
	service := NewServiceInfo("service-1", "test-service", "ws://localhost:8080")

	if service.ServiceID != "service-1" {
		t.Errorf("Expected service ID 'service-1', got '%s'", service.ServiceID)
	}

	if service.Name != "test-service" {
		t.Errorf("Expected name 'test-service', got '%s'", service.Name)
	}

	if service.Address != "ws://localhost:8080" {
		t.Errorf("Expected address 'ws://localhost:8080', got '%s'", service.Address)
	}
}

func TestServiceCapabilities(t *testing.T) {
	service := NewServiceInfo("service-1", "test-service", "ws://localhost:8080")

	service.AddCapability("chat")
	service.AddCapability("file-transfer")
	service.AddCapability("chat") // Duplicate should be ignored

	if len(service.Capabilities) != 2 {
		t.Errorf("Expected 2 capabilities, got %d", len(service.Capabilities))
	}

	if !service.HasCapability("chat") {
		t.Error("Expected service to have 'chat' capability")
	}

	if !service.HasCapability("file-transfer") {
		t.Error("Expected service to have 'file-transfer' capability")
	}

	if service.HasCapability("video") {
		t.Error("Expected service to not have 'video' capability")
	}
}

func TestServiceMetadata(t *testing.T) {
	service := NewServiceInfo("service-1", "test-service", "ws://localhost:8080")

	service.AddMetadata("region", "us-east-1")
	service.AddMetadata("env", "production")

	if val, ok := service.GetMetadata("region"); !ok || val != "us-east-1" {
		t.Errorf("Expected region 'us-east-1', got '%s' (ok=%v)", val, ok)
	}

	if val, ok := service.GetMetadata("env"); !ok || val != "production" {
		t.Errorf("Expected env 'production', got '%s' (ok=%v)", val, ok)
	}

	if _, ok := service.GetMetadata("missing"); ok {
		t.Error("Expected missing key to return false")
	}
}

func TestServiceStale(t *testing.T) {
	service := NewServiceInfo("service-1", "test-service", "ws://localhost:8080")

	// Freshly created service should not be stale
	if service.IsStale(1 * time.Second) {
		t.Error("New service should not be stale")
	}

	// Manually set old timestamp
	service.LastSeen = time.Now().Add(-2 * time.Second)

	if !service.IsStale(1 * time.Second) {
		t.Error("Old service should be stale")
	}
}

func TestServiceDiscovery(t *testing.T) {
	sd := NewServiceDiscovery()

	// Register local service
	local := sd.RegisterLocalService("my-service", "ws://localhost:8080", []string{"chat", "file-transfer"})

	if local == nil {
		t.Fatal("Expected local service to be registered")
	}

	if !local.HasCapability("chat") {
		t.Error("Local service should have 'chat' capability")
	}

	// Get local service
	retrieved := sd.GetLocalService()
	if retrieved == nil || retrieved.ServiceID != local.ServiceID {
		t.Error("Failed to retrieve local service")
	}
}

func TestRegisterAndGetService(t *testing.T) {
	sd := NewServiceDiscovery()

	service := NewServiceInfo("service-1", "remote-service", "ws://localhost:8081")
	service.AddCapability("video")

	sd.RegisterService(service)

	// Get service
	retrieved, ok := sd.GetService("service-1")
	if !ok {
		t.Fatal("Expected to find registered service")
	}

	if retrieved.ServiceID != "service-1" {
		t.Errorf("Expected service ID 'service-1', got '%s'", retrieved.ServiceID)
	}
}

func TestUnregisterService(t *testing.T) {
	sd := NewServiceDiscovery()

	service := NewServiceInfo("service-1", "test-service", "ws://localhost:8080")
	sd.RegisterService(service)

	// Verify it's registered
	if _, ok := sd.GetService("service-1"); !ok {
		t.Fatal("Service should be registered")
	}

	// Unregister
	sd.UnregisterService("service-1")

	// Verify it's gone
	if _, ok := sd.GetService("service-1"); ok {
		t.Error("Service should be unregistered")
	}
}

func TestListServices(t *testing.T) {
	sd := NewServiceDiscovery()

	sd.RegisterService(NewServiceInfo("service-1", "svc1", "ws://localhost:8081"))
	sd.RegisterService(NewServiceInfo("service-2", "svc2", "ws://localhost:8082"))
	sd.RegisterService(NewServiceInfo("service-3", "svc3", "ws://localhost:8083"))

	services := sd.ListServices()

	if len(services) != 3 {
		t.Errorf("Expected 3 services, got %d", len(services))
	}
}

func TestFindServicesByCapability(t *testing.T) {
	sd := NewServiceDiscovery()

	service1 := NewServiceInfo("service-1", "svc1", "ws://localhost:8081")
	service1.AddCapability("chat")
	service1.AddCapability("file-transfer")

	service2 := NewServiceInfo("service-2", "svc2", "ws://localhost:8082")
	service2.AddCapability("video")

	service3 := NewServiceInfo("service-3", "svc3", "ws://localhost:8083")
	service3.AddCapability("chat")

	sd.RegisterService(service1)
	sd.RegisterService(service2)
	sd.RegisterService(service3)

	// Find chat services
	chatServices := sd.FindServicesByCapability("chat")
	if len(chatServices) != 2 {
		t.Errorf("Expected 2 chat services, got %d", len(chatServices))
	}

	// Find video services
	videoServices := sd.FindServicesByCapability("video")
	if len(videoServices) != 1 {
		t.Errorf("Expected 1 video service, got %d", len(videoServices))
	}
}

func TestFindServicesByName(t *testing.T) {
	sd := NewServiceDiscovery()

	sd.RegisterService(NewServiceInfo("service-1", "my-service", "ws://localhost:8081"))
	sd.RegisterService(NewServiceInfo("service-2", "other-service", "ws://localhost:8082"))
	sd.RegisterService(NewServiceInfo("service-3", "my-service", "ws://localhost:8083"))

	services := sd.FindServicesByName("my-service")

	if len(services) != 2 {
		t.Errorf("Expected 2 services with name 'my-service', got %d", len(services))
	}
}

func TestUpdateServiceHealth(t *testing.T) {
	sd := NewServiceDiscovery()

	service := NewServiceInfo("service-1", "test-service", "ws://localhost:8080")
	service.LastSeen = time.Now().Add(-10 * time.Second)
	sd.RegisterService(service)

	// Update health
	updated := sd.UpdateServiceHealth("service-1")
	if !updated {
		t.Error("Failed to update service health")
	}

	// Check that last seen was updated
	retrieved, _ := sd.GetService("service-1")
	if time.Since(retrieved.LastSeen) > 1*time.Second {
		t.Error("Service last seen should be recent")
	}
}

func TestCleanupStaleServices(t *testing.T) {
	sd := NewServiceDiscoveryWithTimeout(1 * time.Second)

	// Add fresh service
	fresh := NewServiceInfo("service-1", "fresh", "ws://localhost:8081")
	sd.RegisterService(fresh)

	// Add stale service
	stale := NewServiceInfo("service-2", "stale", "ws://localhost:8082")
	stale.LastSeen = time.Now().Add(-2 * time.Second)
	sd.RegisterService(stale)

	// Cleanup
	removed := sd.CleanupStaleServices()

	if removed != 1 {
		t.Errorf("Expected 1 service removed, got %d", removed)
	}

	// Verify stale service is gone
	if _, ok := sd.GetService("service-2"); ok {
		t.Error("Stale service should be removed")
	}

	// Verify fresh service remains
	if _, ok := sd.GetService("service-1"); !ok {
		t.Error("Fresh service should remain")
	}
}

func TestAutoCleanup(t *testing.T) {
	sd := NewServiceDiscoveryWithTimeout(100 * time.Millisecond)

	ctx, cancel := context.WithTimeout(context.Background(), 1*time.Second)
	defer cancel()

	// Add stale service
	stale := NewServiceInfo("service-1", "stale", "ws://localhost:8080")
	stale.LastSeen = time.Now().Add(-200 * time.Millisecond)
	sd.RegisterService(stale)

	// Start auto cleanup
	sd.StartAutoCleanup(ctx, 150*time.Millisecond)
	defer sd.StopAutoCleanup()

	// Wait for cleanup to run
	time.Sleep(300 * time.Millisecond)

	// Verify stale service was removed
	if _, ok := sd.GetService("service-1"); ok {
		t.Error("Stale service should be removed by auto cleanup")
	}
}

func TestGetStats(t *testing.T) {
	sd := NewServiceDiscoveryWithTimeout(1 * time.Second)

	// Register local service
	sd.RegisterLocalService("local", "ws://localhost:8080", []string{"chat"})

	// Add fresh service
	fresh := NewServiceInfo("service-1", "fresh", "ws://localhost:8081")
	sd.RegisterService(fresh)

	// Add stale service
	stale := NewServiceInfo("service-2", "stale", "ws://localhost:8082")
	stale.LastSeen = time.Now().Add(-2 * time.Second)
	sd.RegisterService(stale)

	stats := sd.GetStats()

	if stats.TotalServices != 3 {
		t.Errorf("Expected 3 total services, got %d", stats.TotalServices)
	}

	if stats.ActiveServices != 2 {
		t.Errorf("Expected 2 active services, got %d", stats.ActiveServices)
	}

	if stats.StaleServices != 1 {
		t.Errorf("Expected 1 stale service, got %d", stats.StaleServices)
	}

	if !stats.HasLocalService {
		t.Error("Expected has local service to be true")
	}
}
