package com.hivellm.umicp.discovery;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

/**
 * Service Discovery Manager
 * Manages service registration, discovery, and automatic cleanup
 * of stale services in a peer-to-peer network.
 */
public class ServiceDiscovery {
    private final Map<String, ServiceInfo> services;
    private ServiceInfo localService;
    private int timeoutSeconds;

    /**
     * Construct a new Service Discovery with default timeout
     */
    public ServiceDiscovery() {
        this(60); // Default 60 seconds timeout
    }

    /**
     * Construct with custom timeout
     *
     * @param timeoutSeconds Timeout for stale service detection
     */
    public ServiceDiscovery(int timeoutSeconds) {
        this.services = new ConcurrentHashMap<>();
        this.timeoutSeconds = timeoutSeconds;
        this.localService = null;
    }

    /**
     * Register local service
     *
     * @param service Service information
     */
    public synchronized void registerLocal(ServiceInfo service) {
        if (service == null) {
            throw new IllegalArgumentException("service cannot be null");
        }
        this.localService = service;
        services.put(service.getServiceId(), service);
    }

    /**
     * Register a discovered service
     *
     * @param service Service information
     */
    public void registerService(ServiceInfo service) {
        if (service == null) {
            throw new IllegalArgumentException("service cannot be null");
        }

        ServiceInfo existing = services.get(service.getServiceId());
        if (existing != null) {
            existing.updateLastSeen();
        } else {
            services.put(service.getServiceId(), service);
        }
    }

    /**
     * Unregister a service by ID
     *
     * @param serviceId Service ID to unregister
     * @return true if service was found and removed
     */
    public boolean unregisterService(String serviceId) {
        if (serviceId == null || serviceId.isEmpty()) {
            return false;
        }
        return services.remove(serviceId) != null;
    }

    /**
     * Get service by ID
     *
     * @param serviceId Service ID to lookup
     * @return ServiceInfo, or null if not found
     */
    public ServiceInfo getService(String serviceId) {
        if (serviceId == null || serviceId.isEmpty()) {
            return null;
        }
        return services.get(serviceId);
    }

    /**
     * Get all registered services
     *
     * @return List of all services
     */
    public List<ServiceInfo> getAllServices() {
        return new ArrayList<>(services.values());
    }

    /**
     * Find services by capability
     *
     * @param capability Capability to search for
     * @return List of services with the capability
     */
    public List<ServiceInfo> findByCapability(String capability) {
        if (capability == null || capability.isEmpty()) {
            return new ArrayList<>();
        }

        return services.values().stream()
                .filter(s -> s.hasCapability(capability))
                .collect(Collectors.toList());
    }

    /**
     * Find services by metadata key-value pair
     *
     * @param key Metadata key
     * @param value Metadata value
     * @return List of services matching the metadata
     */
    public List<ServiceInfo> findByMetadata(String key, String value) {
        if (key == null || key.isEmpty() || value == null) {
            return new ArrayList<>();
        }

        return services.values().stream()
                .filter(s -> value.equals(s.getMetadataValue(key)))
                .collect(Collectors.toList());
    }

    /**
     * Find services by name pattern (substring match)
     *
     * @param namePattern Name pattern to search for
     * @return List of services matching the pattern
     */
    public List<ServiceInfo> findByName(String namePattern) {
        if (namePattern == null || namePattern.isEmpty()) {
            return new ArrayList<>();
        }

        return services.values().stream()
                .filter(s -> s.getName().contains(namePattern))
                .collect(Collectors.toList());
    }

    /**
     * Clean up stale services
     * Removes services that haven't been seen within the timeout period.
     *
     * @return Number of services removed
     */
    public int cleanupStaleServices() {
        int removedCount = 0;

        Iterator<Map.Entry<String, ServiceInfo>> iterator = services.entrySet().iterator();
        while (iterator.hasNext()) {
            Map.Entry<String, ServiceInfo> entry = iterator.next();
            ServiceInfo service = entry.getValue();

            // Don't remove local service
            if (localService != null && service.getServiceId().equals(localService.getServiceId())) {
                continue;
            }

            if (service.isStale(timeoutSeconds)) {
                iterator.remove();
                removedCount++;
            }
        }

        return removedCount;
    }

    /**
     * Get total number of registered services
     *
     * @return Service count
     */
    public int getServiceCount() {
        return services.size();
    }

    /**
     * Get local service info
     *
     * @return Local service, or null if not registered
     */
    public synchronized ServiceInfo getLocalService() {
        return localService;
    }

    /**
     * Set timeout for stale service detection
     *
     * @param timeoutSeconds Timeout in seconds
     */
    public synchronized void setTimeout(int timeoutSeconds) {
        if (timeoutSeconds < 0) {
            throw new IllegalArgumentException("timeoutSeconds must be >= 0");
        }
        this.timeoutSeconds = timeoutSeconds;
    }

    /**
     * Get current timeout setting
     *
     * @return Timeout in seconds
     */
    public int getTimeout() {
        return timeoutSeconds;
    }

    /**
     * Announce service (placeholder for network announcement)
     *
     * @param service Service to announce
     * @return true if announcement was successful
     */
    public boolean announceService(ServiceInfo service) {
        if (service == null) {
            return false;
        }
        // Placeholder - in full implementation, would broadcast to network
        registerService(service);
        return true;
    }
}

