package discovery

import (
	"context"
	"sync"
	"time"

	"github.com/google/uuid"
)

// ServiceInfo represents discovered service information
type ServiceInfo struct {
	ServiceID    string            `json:"service_id"`
	Name         string            `json:"name"`
	Address      string            `json:"address"`
	Capabilities []string          `json:"capabilities"`
	Metadata     map[string]string `json:"metadata"`
	LastSeen     time.Time         `json:"last_seen"`
	Version      string            `json:"version"`
}

// NewServiceInfo creates a new service info
func NewServiceInfo(serviceID, name, address string) *ServiceInfo {
	return &ServiceInfo{
		ServiceID:    serviceID,
		Name:         name,
		Address:      address,
		Capabilities: make([]string, 0),
		Metadata:     make(map[string]string),
		LastSeen:     time.Now(),
		Version:      "1.0.0",
	}
}

// AddCapability adds a capability to the service
func (s *ServiceInfo) AddCapability(capability string) {
	for _, c := range s.Capabilities {
		if c == capability {
			return
		}
	}
	s.Capabilities = append(s.Capabilities, capability)
}

// AddMetadata adds metadata to the service
func (s *ServiceInfo) AddMetadata(key, value string) {
	s.Metadata[key] = value
}

// HasCapability checks if service has a capability
func (s *ServiceInfo) HasCapability(capability string) bool {
	for _, c := range s.Capabilities {
		if c == capability {
			return true
		}
	}
	return false
}

// GetMetadata gets metadata value
func (s *ServiceInfo) GetMetadata(key string) (string, bool) {
	val, ok := s.Metadata[key]
	return val, ok
}

// UpdateLastSeen updates the last seen timestamp
func (s *ServiceInfo) UpdateLastSeen() {
	s.LastSeen = time.Now()
}

// IsStale checks if service is stale (older than timeout)
func (s *ServiceInfo) IsStale(timeout time.Duration) bool {
	return time.Since(s.LastSeen) > timeout
}

// ServiceDiscovery manages service discovery
type ServiceDiscovery struct {
	mu            sync.RWMutex
	services      map[string]*ServiceInfo
	timeout       time.Duration
	localService  *ServiceInfo
	cleanupTicker *time.Ticker
	stopCleanup   chan struct{}
}

// NewServiceDiscovery creates a new service discovery manager
func NewServiceDiscovery() *ServiceDiscovery {
	return &ServiceDiscovery{
		services:    make(map[string]*ServiceInfo),
		timeout:     60 * time.Second,
		stopCleanup: make(chan struct{}),
	}
}

// NewServiceDiscoveryWithTimeout creates a new service discovery manager with custom timeout
func NewServiceDiscoveryWithTimeout(timeout time.Duration) *ServiceDiscovery {
	return &ServiceDiscovery{
		services:    make(map[string]*ServiceInfo),
		timeout:     timeout,
		stopCleanup: make(chan struct{}),
	}
}

// RegisterLocalService registers the local service
func (sd *ServiceDiscovery) RegisterLocalService(name, address string, capabilities []string) *ServiceInfo {
	sd.mu.Lock()
	defer sd.mu.Unlock()

	serviceID := uuid.New().String()
	service := NewServiceInfo(serviceID, name, address)

	for _, cap := range capabilities {
		service.AddCapability(cap)
	}

	sd.localService = service
	sd.services[serviceID] = service

	return service
}

// RegisterService registers a discovered service
func (sd *ServiceDiscovery) RegisterService(service *ServiceInfo) {
	sd.mu.Lock()
	defer sd.mu.Unlock()

	sd.services[service.ServiceID] = service
}

// UnregisterService removes a service from the registry
func (sd *ServiceDiscovery) UnregisterService(serviceID string) {
	sd.mu.Lock()
	defer sd.mu.Unlock()

	delete(sd.services, serviceID)
}

// GetService gets a service by ID
func (sd *ServiceDiscovery) GetService(serviceID string) (*ServiceInfo, bool) {
	sd.mu.RLock()
	defer sd.mu.RUnlock()

	service, ok := sd.services[serviceID]
	return service, ok
}

// GetLocalService gets the local service info
func (sd *ServiceDiscovery) GetLocalService() *ServiceInfo {
	sd.mu.RLock()
	defer sd.mu.RUnlock()

	return sd.localService
}

// ListServices lists all registered services
func (sd *ServiceDiscovery) ListServices() []*ServiceInfo {
	sd.mu.RLock()
	defer sd.mu.RUnlock()

	services := make([]*ServiceInfo, 0, len(sd.services))
	for _, service := range sd.services {
		services = append(services, service)
	}

	return services
}

// FindServicesByCapability finds services with a specific capability
func (sd *ServiceDiscovery) FindServicesByCapability(capability string) []*ServiceInfo {
	sd.mu.RLock()
	defer sd.mu.RUnlock()

	services := make([]*ServiceInfo, 0)
	for _, service := range sd.services {
		if service.HasCapability(capability) {
			services = append(services, service)
		}
	}

	return services
}

// FindServicesByName finds services by name
func (sd *ServiceDiscovery) FindServicesByName(name string) []*ServiceInfo {
	sd.mu.RLock()
	defer sd.mu.RUnlock()

	services := make([]*ServiceInfo, 0)
	for _, service := range sd.services {
		if service.Name == name {
			services = append(services, service)
		}
	}

	return services
}

// UpdateServiceHealth updates the last seen timestamp for a service
func (sd *ServiceDiscovery) UpdateServiceHealth(serviceID string) bool {
	sd.mu.Lock()
	defer sd.mu.Unlock()

	service, ok := sd.services[serviceID]
	if !ok {
		return false
	}

	service.UpdateLastSeen()
	return true
}

// CleanupStaleServices removes services that haven't been seen recently
func (sd *ServiceDiscovery) CleanupStaleServices() int {
	sd.mu.Lock()
	defer sd.mu.Unlock()

	removed := 0
	for serviceID, service := range sd.services {
		if service.IsStale(sd.timeout) {
			delete(sd.services, serviceID)
			removed++
		}
	}

	return removed
}

// StartAutoCleanup starts automatic cleanup of stale services
func (sd *ServiceDiscovery) StartAutoCleanup(ctx context.Context, interval time.Duration) {
	sd.cleanupTicker = time.NewTicker(interval)

	go func() {
		for {
			select {
			case <-sd.cleanupTicker.C:
				sd.CleanupStaleServices()
			case <-sd.stopCleanup:
				return
			case <-ctx.Done():
				return
			}
		}
	}()
}

// StopAutoCleanup stops automatic cleanup
func (sd *ServiceDiscovery) StopAutoCleanup() {
	if sd.cleanupTicker != nil {
		sd.cleanupTicker.Stop()
		close(sd.stopCleanup)
	}
}

// GetStats returns discovery statistics
func (sd *ServiceDiscovery) GetStats() DiscoveryStats {
	sd.mu.RLock()
	defer sd.mu.RUnlock()

	activeCount := 0
	staleCount := 0

	for _, service := range sd.services {
		if service.IsStale(sd.timeout) {
			staleCount++
		} else {
			activeCount++
		}
	}

	return DiscoveryStats{
		TotalServices:   len(sd.services),
		ActiveServices:  activeCount,
		StaleServices:   staleCount,
		HasLocalService: sd.localService != nil,
	}
}

// DiscoveryStats contains discovery statistics
type DiscoveryStats struct {
	TotalServices   int  `json:"total_services"`
	ActiveServices  int  `json:"active_services"`
	StaleServices   int  `json:"stale_services"`
	HasLocalService bool `json:"has_local_service"`
}
