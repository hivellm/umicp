<?php

namespace Umicp\Discovery;

/**
 * Service Discovery Manager
 *
 * Manages service registration, discovery, and automatic cleanup
 * of stale services in a peer-to-peer network.
 */
class ServiceDiscovery
{
    private array $services = [];
    private ?ServiceInfo $localService = null;
    private int $timeoutSeconds;

    /**
     * Construct a new Service Discovery with default timeout
     *
     * @param int $timeoutSeconds Timeout for stale service detection (default: 60)
     */
    public function __construct(int $timeoutSeconds = 60)
    {
        $this->timeoutSeconds = $timeoutSeconds;
    }

    /**
     * Register local service
     *
     * @param ServiceInfo $service Service information
     */
    public function registerLocal(ServiceInfo $service): void
    {
        $this->localService = $service;
        $this->services[$service->getServiceId()] = $service;
    }

    /**
     * Register a discovered service
     *
     * @param ServiceInfo $service Service information
     */
    public function registerService(ServiceInfo $service): void
    {
        $serviceId = $service->getServiceId();

        if (isset($this->services[$serviceId])) {
            // Update existing service
            $this->services[$serviceId]->updateLastSeen();
        } else {
            // Add new service
            $this->services[$serviceId] = $service;
        }
    }

    /**
     * Unregister a service by ID
     *
     * @param string $serviceId Service ID to unregister
     * @return bool true if service was found and removed
     */
    public function unregisterService(string $serviceId): bool
    {
        if (isset($this->services[$serviceId])) {
            unset($this->services[$serviceId]);
            return true;
        }
        return false;
    }

    /**
     * Get service by ID
     *
     * @param string $serviceId Service ID to lookup
     * @return ServiceInfo|null ServiceInfo, or null if not found
     */
    public function getService(string $serviceId): ?ServiceInfo
    {
        return $this->services[$serviceId] ?? null;
    }

    /**
     * Get all registered services
     *
     * @return ServiceInfo[] Array of all services
     */
    public function getAllServices(): array
    {
        return array_values($this->services);
    }

    /**
     * Find services by capability
     *
     * @param string $capability Capability to search for
     * @return ServiceInfo[] Array of services with the capability
     */
    public function findByCapability(string $capability): array
    {
        return array_values(array_filter(
            $this->services,
            fn(ServiceInfo $s) => $s->hasCapability($capability)
        ));
    }

    /**
     * Find services by metadata key-value pair
     *
     * @param string $key Metadata key
     * @param string $value Metadata value
     * @return ServiceInfo[] Array of services matching the metadata
     */
    public function findByMetadata(string $key, string $value): array
    {
        return array_values(array_filter(
            $this->services,
            fn(ServiceInfo $s) => $s->getMetadataValue($key) === $value
        ));
    }

    /**
     * Find services by name pattern (substring match)
     *
     * @param string $namePattern Name pattern to search for
     * @return ServiceInfo[] Array of services matching the pattern
     */
    public function findByName(string $namePattern): array
    {
        return array_values(array_filter(
            $this->services,
            fn(ServiceInfo $s) => str_contains($s->getName(), $namePattern)
        ));
    }

    /**
     * Clean up stale services
     * Removes services that haven't been seen within the timeout period.
     *
     * @return int Number of services removed
     */
    public function cleanupStaleServices(): int
    {
        $removedCount = 0;

        foreach ($this->services as $serviceId => $service) {
            // Don't remove local service
            if ($this->localService && $serviceId === $this->localService->getServiceId()) {
                continue;
            }

            if ($service->isStale($this->timeoutSeconds)) {
                unset($this->services[$serviceId]);
                $removedCount++;
            }
        }

        return $removedCount;
    }

    /**
     * Get total number of registered services
     *
     * @return int Service count
     */
    public function getServiceCount(): int
    {
        return count($this->services);
    }

    /**
     * Get local service info
     *
     * @return ServiceInfo|null Local service, or null if not registered
     */
    public function getLocalService(): ?ServiceInfo
    {
        return $this->localService;
    }

    /**
     * Set timeout for stale service detection
     *
     * @param int $timeoutSeconds Timeout in seconds
     */
    public function setTimeout(int $timeoutSeconds): void
    {
        if ($timeoutSeconds < 0) {
            throw new \InvalidArgumentException('timeoutSeconds must be >= 0');
        }
        $this->timeoutSeconds = $timeoutSeconds;
    }

    /**
     * Get current timeout setting
     *
     * @return int Timeout in seconds
     */
    public function getTimeout(): int
    {
        return $this->timeoutSeconds;
    }

    /**
     * Announce service (placeholder for network announcement)
     *
     * @param ServiceInfo $service Service to announce
     * @return bool true if announcement was successful
     */
    public function announceService(ServiceInfo $service): bool
    {
        // Placeholder - in full implementation, would broadcast to network
        $this->registerService($service);
        return true;
    }
}

