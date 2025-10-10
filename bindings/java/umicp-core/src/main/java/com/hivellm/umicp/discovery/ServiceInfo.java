package com.hivellm.umicp.discovery;

import java.time.Instant;
import java.util.*;

/**
 * Service information for discovery
 * Contains metadata about a discovered service including capabilities, address, and version
 */
public class ServiceInfo {
    private final String serviceId;
    private final String name;
    private final String address;
    private final List<String> capabilities;
    private final Map<String, String> metadata;
    private Instant lastSeen;
    private String version;

    /**
     * Construct a new ServiceInfo
     *
     * @param serviceId Unique service identifier
     * @param name Service name
     * @param address Service address (e.g., ws://localhost:8080)
     */
    public ServiceInfo(String serviceId, String name, String address) {
        if (serviceId == null || serviceId.isEmpty()) {
            throw new IllegalArgumentException("serviceId cannot be null or empty");
        }
        if (name == null || name.isEmpty()) {
            throw new IllegalArgumentException("name cannot be null or empty");
        }
        if (address == null || address.isEmpty()) {
            throw new IllegalArgumentException("address cannot be null or empty");
        }

        this.serviceId = serviceId;
        this.name = name;
        this.address = address;
        this.capabilities = new ArrayList<>();
        this.metadata = new HashMap<>();
        this.lastSeen = Instant.now();
        this.version = "1.0.0";
    }

    // Getters
    public String getServiceId() { return serviceId; }
    public String getName() { return name; }
    public String getAddress() { return address; }
    public List<String> getCapabilities() { return new ArrayList<>(capabilities); }
    public Map<String, String> getMetadata() { return new HashMap<>(metadata); }
    public Instant getLastSeen() { return lastSeen; }
    public String getVersion() { return version; }

    // Setters
    public void setVersion(String version) {
        if (version == null || version.isEmpty()) {
            throw new IllegalArgumentException("version cannot be null or empty");
        }
        this.version = version;
    }

    /**
     * Add a capability to this service
     *
     * @param capability Capability name (e.g., "storage", "compute")
     */
    public void addCapability(String capability) {
        if (capability != null && !capability.isEmpty() && !capabilities.contains(capability)) {
            capabilities.add(capability);
        }
    }

    /**
     * Add metadata key-value pair
     *
     * @param key Metadata key
     * @param value Metadata value
     */
    public void addMetadata(String key, String value) {
        if (key != null && !key.isEmpty() && value != null) {
            metadata.put(key, value);
        }
    }

    /**
     * Check if service has a specific capability
     *
     * @param capability Capability to check
     * @return true if service has the capability
     */
    public boolean hasCapability(String capability) {
        return capabilities.contains(capability);
    }

    /**
     * Get metadata value by key
     *
     * @param key Metadata key
     * @return Metadata value, or null if not found
     */
    public String getMetadataValue(String key) {
        return metadata.get(key);
    }

    /**
     * Update the last seen timestamp to now
     */
    public void updateLastSeen() {
        this.lastSeen = Instant.now();
    }

    /**
     * Check if service is stale (not seen recently)
     *
     * @param timeoutSeconds Timeout duration in seconds
     * @return true if service hasn't been seen within timeout period
     */
    public boolean isStale(int timeoutSeconds) {
        Instant now = Instant.now();
        long elapsed = now.getEpochSecond() - lastSeen.getEpochSecond();
        return elapsed > timeoutSeconds;
    }

    @Override
    public String toString() {
        return "ServiceInfo{" +
                "serviceId='" + serviceId + '\'' +
                ", name='" + name + '\'' +
                ", address='" + address + '\'' +
                ", capabilities=" + capabilities +
                ", version='" + version + '\'' +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ServiceInfo that = (ServiceInfo) o;
        return Objects.equals(serviceId, that.serviceId);
    }

    @Override
    public int hashCode() {
        return Objects.hash(serviceId);
    }
}

