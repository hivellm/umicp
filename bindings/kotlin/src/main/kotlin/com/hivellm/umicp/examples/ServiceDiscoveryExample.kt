package com.hivellm.umicp.examples

import com.hivellm.umicp.discovery.ServiceDiscovery
import com.hivellm.umicp.discovery.ServiceInfo
import kotlinx.coroutines.runBlocking

/**
 * Service Discovery Example
 *
 * Demonstrates:
 * - Service registration
 * - Service lookup
 * - Capability-based discovery
 * - Pattern matching
 */
fun main() = runBlocking {
    println("=== UMICP Kotlin SDK - Service Discovery Example ===\n")

    val discovery = ServiceDiscovery()

    // Register services
    println("1. Registering Services:")

    val services = listOf(
        ServiceInfo(
            id = "api-gateway-1",
            name = "api-gateway",
            endpoint = "http://localhost:8080",
            capabilities = setOf("http", "websocket", "grpc"),
            metadata = mapOf("region" to "us-east-1", "version" to "1.0.0")
        ),
        ServiceInfo(
            id = "api-gateway-2",
            name = "api-gateway",
            endpoint = "http://localhost:8081",
            capabilities = setOf("http", "websocket"),
            metadata = mapOf("region" to "us-west-1", "version" to "1.0.0")
        ),
        ServiceInfo(
            id = "data-processor-1",
            name = "data-processor",
            endpoint = "http://localhost:9000",
            capabilities = setOf("batch", "stream"),
            metadata = mapOf("region" to "us-east-1", "version" to "2.0.0")
        ),
        ServiceInfo(
            id = "ml-inference-1",
            name = "ml-inference",
            endpoint = "http://localhost:9100",
            capabilities = setOf("inference", "batch", "gpu"),
            metadata = mapOf("region" to "us-east-1", "model" to "gpt-4")
        )
    )

    services.forEach { service ->
        discovery.register(service)
        println("  ✓ Registered: ${service.name} (${service.id})")
    }

    println("  Total services: ${discovery.getCount()}")
    println()

    // Lookup by ID
    println("2. Lookup by ID:")
    val found = discovery.lookup("api-gateway-1")
    if (found != null) {
        println("  Found: ${found.name}")
        println("  Endpoint: ${found.endpoint}")
        println("  Capabilities: ${found.capabilities}")
    }
    println()

    // Find by name pattern
    println("3. Find by Name Pattern:")
    val apiServices = discovery.findByName("api-*")
    println("  Pattern: 'api-*'")
    println("  Found ${apiServices.size} services:")
    apiServices.forEach { service ->
        println("    - ${service.name} @ ${service.endpoint}")
    }
    println()

    // Find by capability
    println("4. Find by Capability:")
    val wsServices = discovery.findByCapability("websocket")
    println("  Capability: 'websocket'")
    println("  Found ${wsServices.size} services:")
    wsServices.forEach { service ->
        println("    - ${service.name} (${service.id})")
    }
    println()

    // Find by multiple capabilities
    println("5. Find by Multiple Capabilities:")
    val requiredCaps = setOf("http", "websocket", "grpc")
    val fullServices = discovery.findByCapabilities(requiredCaps)
    println("  Required: $requiredCaps")
    println("  Found ${fullServices.size} services:")
    fullServices.forEach { service ->
        println("    - ${service.name} @ ${service.endpoint}")
    }
    println()

    // List all services
    println("6. All Registered Services:")
    val all = discovery.getAll()
    all.forEach { service ->
        println("  - ${service.name} (${service.id})")
        println("    Endpoint: ${service.endpoint}")
        println("    Capabilities: ${service.capabilities}")
        println("    Metadata: ${service.metadata}")
        println()
    }

    // Use case: Load balancing
    println("7. Use Case - Load Balancing:")
    val gateways = discovery.findByName("api-gateway")
    println("  Available gateways: ${gateways.size}")

    if (gateways.isNotEmpty()) {
        val selected = gateways.random() // Simple random selection
        println("  Selected: ${selected.name} @ ${selected.endpoint}")
        println("  Region: ${selected.metadata["region"]}")
    }
    println()

    // Use case: Service mesh
    println("8. Use Case - Service Mesh:")
    val eastRegion = discovery.getAll().filter {
        it.metadata["region"] == "us-east-1"
    }
    println("  Services in us-east-1: ${eastRegion.size}")
    eastRegion.forEach { service ->
        println("    - ${service.name}")
    }
    println()

    // Unregister a service
    println("9. Unregister Service:")
    val removed = discovery.unregister("data-processor-1")
    println("  Removed: $removed")
    println("  Remaining services: ${discovery.getCount()}")
    println()

    // Cleanup
    println("10. Cleanup:")
    discovery.clear()
    println("  All services cleared")
    println("  Services count: ${discovery.getCount()}")
    println()

    println("=== Example Complete ===")
}

