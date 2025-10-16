package com.hivellm.umicp.discovery

import org.junit.jupiter.api.Test
import org.junit.jupiter.api.Assertions.*

class TestService : DiscoverableService {
    override fun listOperations() = listOf(
        OperationSchema(
            name = "search_vectors",
            inputSchema = mapOf(
                "type" to "object",
                "properties" to mapOf(
                    "collection" to mapOf("type" to "string"),
                    "query" to mapOf("type" to "string"),
                    "limit" to mapOf("type" to "integer", "default" to 10)
                ),
                "required" to listOf("collection", "query")
            ),
            title = "Search Vectors",
            description = "Search for semantically similar content",
            annotations = mapOf("read_only" to true)
        ),
        OperationSchema(
            name = "create_collection",
            inputSchema = mapOf(
                "type" to "object",
                "properties" to mapOf(
                    "name" to mapOf("type" to "string"),
                    "dimension" to mapOf("type" to "integer")
                ),
                "required" to listOf("name", "dimension")
            ),
            title = "Create Collection"
        )
    )

    override fun getServerInfo() = ServerInfo(
        server = "test-service",
        version = "1.0.0",
        protocol = "UMICP/0.2",
        features = listOf("discovery", "search"),
        operationsCount = 2,
        mcpCompatible = true
    )
}

class ToolDiscoveryTest {

    @Test
    fun `test operation schema creation`() {
        val schema = OperationSchema(
            name = "test_op",
            inputSchema = mapOf("type" to "object")
        )

        assertEquals("test_op", schema.name)
        assertEquals(mapOf("type" to "object"), schema.inputSchema)
    }

    @Test
    fun `test operation schema builder`() {
        val schema = OperationSchemaBuilder("test", mapOf("type" to "object"))
            .withTitle("Test Operation")
            .withDescription("A test")
            .withAnnotations(mapOf("read_only" to true))
            .build()

        assertEquals("test", schema.name)
        assertEquals("Test Operation", schema.title)
        assertEquals("A test", schema.description)
        assertEquals(true, schema.annotations?.get("read_only"))
    }

    @Test
    fun `test server info creation`() {
        val info = ServerInfo(
            server = "my-service",
            version = "1.0.0",
            protocol = "UMICP/0.2"
        )

        assertEquals("my-service", info.server)
        assertEquals("1.0.0", info.version)
        assertEquals("UMICP/0.2", info.protocol)
    }

    @Test
    fun `test server info builder`() {
        val info = ServerInfoBuilder("test", "1.0", "UMICP/0.2")
            .withFeatures(listOf("discovery"))
            .withOperationsCount(5)
            .withMcpCompatible(true)
            .build()

        assertEquals("test", info.server)
        assertTrue(info.features?.contains("discovery") == true)
        assertEquals(5, info.operationsCount)
        assertEquals(true, info.mcpCompatible)
    }

    @Test
    fun `test discoverable service list operations`() {
        val service = TestService()
        val operations = service.listOperations()

        assertEquals(2, operations.size)
        assertEquals("search_vectors", operations[0].name)
        assertEquals("create_collection", operations[1].name)
    }

    @Test
    fun `test discoverable service get schema`() {
        val service = TestService()
        val schema = service.getSchema("search_vectors")

        assertNotNull(schema)
        assertEquals("search_vectors", schema?.name)
        assertEquals("Search Vectors", schema?.title)
    }

    @Test
    fun `test discoverable service get schema not found`() {
        val service = TestService()
        val schema = service.getSchema("non_existent")

        assertNull(schema)
    }

    @Test
    fun `test discoverable service get server info`() {
        val service = TestService()
        val info = service.getServerInfo()

        assertEquals("test-service", info.server)
        assertEquals("1.0.0", info.version)
        assertEquals("UMICP/0.2", info.protocol)
        assertTrue(info.features?.contains("discovery") == true)
    }

    @Test
    fun `test generate operations response`() {
        val service = TestService()
        val response = DiscoveryHelpers.generateOperationsResponse(service)

        assertTrue(response.containsKey("operations"))
        assertEquals(2, response["count"])
        assertEquals("UMICP/0.2", response["protocol"])
        assertEquals(true, response["mcp_compatible"])
    }

    @Test
    fun `test generate schema response found`() {
        val service = TestService()
        val response = DiscoveryHelpers.generateSchemaResponse(service, "search_vectors")

        assertEquals("search_vectors", response["name"])
        assertEquals("Search Vectors", response["title"])
        assertFalse(response.containsKey("error"))
    }

    @Test
    fun `test generate schema response not found`() {
        val service = TestService()
        val response = DiscoveryHelpers.generateSchemaResponse(service, "invalid")

        assertEquals("Operation not found", response["error"])
        assertEquals("invalid", response["operation"])
    }

    @Test
    fun `test generate server info response`() {
        val service = TestService()
        val info = DiscoveryHelpers.generateServerInfoResponse(service)

        assertEquals("test-service", info.server)
        assertEquals("1.0.0", info.version)
        assertEquals(true, info.mcpCompatible)
    }

    @Test
    fun `test simple discoverable service`() {
        val operations = listOf(
            OperationSchema("test_op", mapOf("type" to "object"))
        )
        val serverInfo = ServerInfo("test", "1.0", "UMICP/0.2")

        val service = SimpleDiscoverableService(operations, serverInfo)

        assertEquals(1, service.listOperations().size)
        assertNotNull(service.getSchema("test_op"))
        assertEquals(1, service.getServerInfo().operationsCount)
    }

    @Test
    fun `test native types in capabilities`() {
        val capabilities: Map<String, Any> = mapOf(
            "max_tokens" to 100,
            "temperature" to 0.7,
            "enabled" to true,
            "models" to listOf("gpt-4", "claude-3"),
            "config" to mapOf("timeout" to 30),
            "optional" to null
        )

        assertEquals(100, capabilities["max_tokens"])
        assertEquals(0.7, (capabilities["temperature"] as Double), 0.001)
        assertEquals(true, capabilities["enabled"])
        assertTrue(capabilities["models"] is List<*>)
        assertTrue(capabilities["config"] is Map<*, *>)
        assertNull(capabilities["optional"])
    }
}

