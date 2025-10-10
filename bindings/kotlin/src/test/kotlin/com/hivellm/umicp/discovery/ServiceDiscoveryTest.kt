package com.hivellm.umicp.discovery

import io.kotest.matchers.shouldBe
import kotlinx.coroutines.test.runTest
import org.junit.jupiter.api.Test
import org.junit.jupiter.api.BeforeEach

class ServiceDiscoveryTest {

    private lateinit var discovery: ServiceDiscovery

    @BeforeEach
    fun setup() {
        discovery = ServiceDiscovery()
    }

    @Test
    fun `should register service`() = runTest {
        val service = ServiceInfo(
            id = "service-1",
            name = "test-service",
            endpoint = "http://localhost:8080"
        )

        discovery.register(service)
        discovery.getCount() shouldBe 1
    }

    @Test
    fun `should lookup service by id`() = runTest {
        val service = ServiceInfo(
            id = "service-1",
            name = "test-service",
            endpoint = "http://localhost:8080"
        )

        discovery.register(service)
        val found = discovery.lookup("service-1")

        found shouldBe service
    }

    @Test
    fun `should unregister service`() = runTest {
        val service = ServiceInfo(
            id = "service-1",
            name = "test-service",
            endpoint = "http://localhost:8080"
        )

        discovery.register(service)
        discovery.unregister("service-1") shouldBe true
        discovery.getCount() shouldBe 0
    }

    @Test
    fun `should find services by name`() = runTest {
        discovery.register(ServiceInfo("1", "api-service", "http://localhost:8080"))
        discovery.register(ServiceInfo("2", "api-gateway", "http://localhost:8081"))
        discovery.register(ServiceInfo("3", "data-service", "http://localhost:8082"))

        val found = discovery.findByName("api-*")
        found.size shouldBe 2
    }

    @Test
    fun `should find services by capability`() = runTest {
        discovery.register(ServiceInfo(
            "1", "service-1", "http://localhost:8080",
            capabilities = setOf("http", "websocket")
        ))
        discovery.register(ServiceInfo(
            "2", "service-2", "http://localhost:8081",
            capabilities = setOf("http")
        ))

        val found = discovery.findByCapability("websocket")
        found.size shouldBe 1
        found[0].id shouldBe "1"
    }

    @Test
    fun `should find services by multiple capabilities`() = runTest {
        discovery.register(ServiceInfo(
            "1", "service-1", "http://localhost:8080",
            capabilities = setOf("http", "websocket", "grpc")
        ))
        discovery.register(ServiceInfo(
            "2", "service-2", "http://localhost:8081",
            capabilities = setOf("http", "websocket")
        ))

        val found = discovery.findByCapabilities(setOf("http", "websocket", "grpc"))
        found.size shouldBe 1
        found[0].id shouldBe "1"
    }

    @Test
    fun `should get all services`() = runTest {
        discovery.register(ServiceInfo("1", "service-1", "http://localhost:8080"))
        discovery.register(ServiceInfo("2", "service-2", "http://localhost:8081"))
        discovery.register(ServiceInfo("3", "service-3", "http://localhost:8082"))

        discovery.getAll().size shouldBe 3
    }

    @Test
    fun `should clear all services`() = runTest {
        discovery.register(ServiceInfo("1", "service-1", "http://localhost:8080"))
        discovery.register(ServiceInfo("2", "service-2", "http://localhost:8081"))

        discovery.clear()
        discovery.getCount() shouldBe 0
    }

    @Test
    fun `should remove stale services`() = runTest {
        val staleThreshold = 1000L // 1 second for test
        discovery = ServiceDiscovery(staleThreshold)

        // Register a service
        discovery.register(ServiceInfo(
            "1", "service-1", "http://localhost:8080",
            timestamp = System.currentTimeMillis() - 2000 // 2 seconds ago
        ))

        // Register a fresh service
        discovery.register(ServiceInfo(
            "2", "service-2", "http://localhost:8081"
        ))

        val removed = discovery.removeStale()
        removed shouldBe 1
        discovery.getCount() shouldBe 1
    }
}

