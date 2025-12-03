package com.hivellm.umicp.discovery

import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import java.util.concurrent.ConcurrentHashMap

/**
 * Service information
 *
 * @property id Service unique identifier
 * @property name Service name
 * @property endpoint Service endpoint (URL or address)
 * @property capabilities Service capabilities
 * @property metadata Additional metadata
 * @property timestamp Registration timestamp
 */
data class ServiceInfo(
    val id: String,
    val name: String,
    val endpoint: String,
    val capabilities: Set<String> = emptySet(),
    val metadata: Map<String, String> = emptyMap(),
    val timestamp: Long = System.currentTimeMillis()
)

/**
 * Service Discovery for UMICP
 *
 * Provides service registration, discovery, and lookup capabilities.
 */
class ServiceDiscovery(
    private val staleThreshold: Long = 300_000 // 5 minutes
) {
    private val services = ConcurrentHashMap<String, ServiceInfo>()
    private val mutex = Mutex()

    /**
     * Register a service
     *
     * @param service Service information
     */
    suspend fun register(service: ServiceInfo) = mutex.withLock {
        services[service.id] = service
    }

    /**
     * Unregister a service
     *
     * @param serviceId Service identifier
     * @return true if service was removed
     */
    suspend fun unregister(serviceId: String): Boolean = mutex.withLock {
        services.remove(serviceId) != null
    }

    /**
     * Lookup a service by ID
     *
     * @param serviceId Service identifier
     * @return Service info or null
     */
    fun lookup(serviceId: String): ServiceInfo? {
        return services[serviceId]
    }

    /**
     * Find services by name
     *
     * @param name Service name (supports wildcards)
     * @return List of matching services
     */
    fun findByName(name: String): List<ServiceInfo> {
        val pattern = name.replace("*", ".*").toRegex()
        return services.values.filter { pattern.matches(it.name) }
    }

    /**
     * Find services by capability
     *
     * @param capability Required capability
     * @return List of services with the capability
     */
    fun findByCapability(capability: String): List<ServiceInfo> {
        return services.values.filter { capability in it.capabilities }
    }

    /**
     * Find services by multiple capabilities (AND logic)
     *
     * @param capabilities Required capabilities
     * @return List of services with all capabilities
     */
    fun findByCapabilities(capabilities: Set<String>): List<ServiceInfo> {
        return services.values.filter { it.capabilities.containsAll(capabilities) }
    }

    /**
     * Get all registered services
     *
     * @return List of all services
     */
    fun getAll(): List<ServiceInfo> {
        return services.values.toList()
    }

    /**
     * Get count of registered services
     */
    fun getCount(): Int = services.size

    /**
     * Remove stale services
     *
     * @return Number of services removed
     */
    suspend fun removeStale(): Int = mutex.withLock {
        val now = System.currentTimeMillis()
        val staleServices = services.filterValues { (now - it.timestamp) > staleThreshold }
        staleServices.keys.forEach { services.remove(it) }
        staleServices.size
    }

    /**
     * Clear all services
     */
    suspend fun clear() = mutex.withLock {
        services.clear()
    }
}

