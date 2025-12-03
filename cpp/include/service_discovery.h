/**
 * @file service_discovery.h
 * @brief Service Discovery for UMICP peer networks
 *
 * Provides service discovery capabilities for peer-to-peer networks.
 * Allows peers to discover and connect to each other based on capabilities and metadata.
 */

#ifndef UMICP_SERVICE_DISCOVERY_H
#define UMICP_SERVICE_DISCOVERY_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <mutex>
#include <functional>

namespace umicp {

/**
 * @brief Service information structure
 *
 * Contains all metadata about a discovered service including
 * capabilities, address, version, and last seen timestamp.
 */
class ServiceInfo {
public:
    /**
     * @brief Construct a new Service Info object
     *
     * @param service_id Unique service identifier
     * @param name Service name
     * @param address Service address (e.g., ws://localhost:8080)
     */
    ServiceInfo(const std::string& service_id,
                const std::string& name,
                const std::string& address);

    // Getters
    const std::string& getServiceId() const { return service_id_; }
    const std::string& getName() const { return name_; }
    const std::string& getAddress() const { return address_; }
    const std::vector<std::string>& getCapabilities() const { return capabilities_; }
    const std::map<std::string, std::string>& getMetadata() const { return metadata_; }
    const std::string& getVersion() const { return version_; }
    std::chrono::system_clock::time_point getLastSeen() const { return last_seen_; }

    // Setters
    void setVersion(const std::string& version) { version_ = version; }

    /**
     * @brief Add a capability to this service
     *
     * @param capability Capability name (e.g., "storage", "compute")
     */
    void addCapability(const std::string& capability);

    /**
     * @brief Add metadata key-value pair
     *
     * @param key Metadata key
     * @param value Metadata value
     */
    void addMetadata(const std::string& key, const std::string& value);

    /**
     * @brief Check if service has a specific capability
     *
     * @param capability Capability to check
     * @return true if service has the capability
     */
    bool hasCapability(const std::string& capability) const;

    /**
     * @brief Get metadata value by key
     *
     * @param key Metadata key
     * @return Metadata value, or empty string if not found
     */
    std::string getMetadataValue(const std::string& key) const;

    /**
     * @brief Update the last seen timestamp to now
     */
    void updateLastSeen();

    /**
     * @brief Check if service is stale (not seen recently)
     *
     * @param timeout Timeout duration in seconds
     * @return true if service hasn't been seen within timeout period
     */
    bool isStale(int timeout_seconds) const;

private:
    std::string service_id_;
    std::string name_;
    std::string address_;
    std::vector<std::string> capabilities_;
    std::map<std::string, std::string> metadata_;
    std::chrono::system_clock::time_point last_seen_;
    std::string version_;
};

/**
 * @brief Service Discovery Manager
 *
 * Manages service registration, discovery, and automatic cleanup
 * of stale services in a peer-to-peer network.
 */
class ServiceDiscovery {
public:
    /**
     * @brief Construct a new Service Discovery object
     */
    ServiceDiscovery();

    /**
     * @brief Construct with custom timeout
     *
     * @param timeout_seconds Timeout for stale service detection (default: 60)
     */
    explicit ServiceDiscovery(int timeout_seconds);

    /**
     * @brief Destroy the Service Discovery object
     */
    ~ServiceDiscovery() = default;

    /**
     * @brief Register local service
     *
     * @param service Service information
     */
    void registerLocal(const ServiceInfo& service);

    /**
     * @brief Register a discovered service
     *
     * @param service Service information
     */
    void registerService(const ServiceInfo& service);

    /**
     * @brief Unregister a service by ID
     *
     * @param service_id Service ID to unregister
     * @return true if service was found and removed
     */
    bool unregisterService(const std::string& service_id);

    /**
     * @brief Get service by ID
     *
     * @param service_id Service ID to lookup
     * @return Pointer to ServiceInfo, or nullptr if not found
     */
    std::shared_ptr<ServiceInfo> getService(const std::string& service_id) const;

    /**
     * @brief Get all registered services
     *
     * @return Vector of all services
     */
    std::vector<std::shared_ptr<ServiceInfo>> getAllServices() const;

    /**
     * @brief Find services by capability
     *
     * @param capability Capability to search for
     * @return Vector of services with the capability
     */
    std::vector<std::shared_ptr<ServiceInfo>> findByCapability(const std::string& capability) const;

    /**
     * @brief Find services by metadata key-value pair
     *
     * @param key Metadata key
     * @param value Metadata value
     * @return Vector of services matching the metadata
     */
    std::vector<std::shared_ptr<ServiceInfo>> findByMetadata(
        const std::string& key, const std::string& value) const;

    /**
     * @brief Find services by name pattern
     *
     * @param name_pattern Name pattern (substring match)
     * @return Vector of services matching the pattern
     */
    std::vector<std::shared_ptr<ServiceInfo>> findByName(const std::string& name_pattern) const;

    /**
     * @brief Clean up stale services
     *
     * Removes services that haven't been seen within the timeout period.
     *
     * @return Number of services removed
     */
    int cleanupStaleServices();

    /**
     * @brief Get total number of registered services
     *
     * @return Service count
     */
    size_t getServiceCount() const;

    /**
     * @brief Get local service info
     *
     * @return Pointer to local service, or nullptr if not registered
     */
    std::shared_ptr<ServiceInfo> getLocalService() const;

    /**
     * @brief Set timeout for stale service detection
     *
     * @param timeout_seconds Timeout in seconds
     */
    void setTimeout(int timeout_seconds);

    /**
     * @brief Get current timeout setting
     *
     * @return Timeout in seconds
     */
    int getTimeout() const { return timeout_seconds_; }

    /**
     * @brief Announce service (placeholder for network announcement)
     *
     * In a full implementation, this would broadcast the service
     * information to the network.
     *
     * @param service Service to announce
     * @return true if announcement was successful
     */
    bool announceService(const ServiceInfo& service);

private:
    mutable std::mutex mutex_;
    std::map<std::string, std::shared_ptr<ServiceInfo>> services_;
    std::shared_ptr<ServiceInfo> local_service_;
    int timeout_seconds_;
};

} // namespace umicp

#endif // UMICP_SERVICE_DISCOVERY_H

