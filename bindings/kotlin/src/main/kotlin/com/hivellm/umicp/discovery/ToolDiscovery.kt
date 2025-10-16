package com.hivellm.umicp.discovery

import kotlinx.serialization.Serializable

/**
 * Tool Discovery for UMICP v0.2.0
 * MCP-compatible automatic tool introspection
 */

typealias JSONSchema = Map<String, Any>

/**
 * Operation schema compatible with MCP JSON Schema
 */
data class OperationSchema(
    val name: String,
    val inputSchema: JSONSchema,
    val title: String? = null,
    val description: String? = null,
    val outputSchema: JSONSchema? = null,
    val annotations: Map<String, Any?>? = null
)

/**
 * Server information for discovery
 */
data class ServerInfo(
    val server: String,
    val version: String,
    val protocol: String,
    val features: List<String>? = null,
    val operationsCount: Int? = null,
    val mcpCompatible: Boolean? = null,
    val metadata: Map<String, Any?>? = null
)

/**
 * Interface for services that support tool discovery
 */
interface DiscoverableService {
    /**
     * List all available operations with their schemas
     */
    fun listOperations(): List<OperationSchema>

    /**
     * Get schema for a specific operation by name
     */
    fun getSchema(name: String): OperationSchema? {
        return listOperations().find { it.name == name }
    }

    /**
     * Get server information and metadata
     */
    fun getServerInfo(): ServerInfo
}

/**
 * Builder for OperationSchema
 */
class OperationSchemaBuilder(
    private val name: String,
    private val inputSchema: JSONSchema
) {
    private var title: String? = null
    private var description: String? = null
    private var outputSchema: JSONSchema? = null
    private var annotations: Map<String, Any>? = null

    fun withTitle(title: String) = apply { this.title = title }
    fun withDescription(description: String) = apply { this.description = description }
    fun withOutputSchema(schema: JSONSchema) = apply { this.outputSchema = schema }
    fun withAnnotations(annotations: Map<String, Any?>) = apply { this.annotations = annotations }

    fun build() = OperationSchema(name, inputSchema, title, description, outputSchema, annotations)
}

/**
 * Builder for ServerInfo
 */
class ServerInfoBuilder(
    private val server: String,
    private val version: String,
    private val protocol: String
) {
    private var features: List<String>? = null
    private var operationsCount: Int? = null
    private var mcpCompatible: Boolean? = null
    private var metadata: Map<String, Any>? = null

    fun withFeatures(features: List<String>) = apply { this.features = features }
    fun withOperationsCount(count: Int) = apply { this.operationsCount = count }
    fun withMcpCompatible(compatible: Boolean) = apply { this.mcpCompatible = compatible }
    fun withMetadata(metadata: Map<String, Any?>) = apply { this.metadata = metadata }

    fun build() = ServerInfo(server, version, protocol, features, operationsCount, mcpCompatible, metadata)
}

/**
 * Helper functions for generating discovery responses
 */
object DiscoveryHelpers {
    /**
     * Generate JSON response for _list_operations
     */
    fun generateOperationsResponse(service: DiscoverableService): Map<String, Any> {
        val operations = service.listOperations()
        val info = service.getServerInfo()

        return mapOf(
            "operations" to operations,
            "count" to operations.size,
            "protocol" to info.protocol,
            "mcp_compatible" to (info.mcpCompatible ?: false)
        )
    }

    /**
     * Generate JSON response for _get_schema
     */
    fun generateSchemaResponse(service: DiscoverableService, operationName: String): Map<String, Any> {
        val schema = service.getSchema(operationName)

        return if (schema != null) {
            mapOf(
                "name" to schema.name,
                "title" to schema.title,
                "description" to schema.description,
                "input_schema" to schema.inputSchema,
                "output_schema" to schema.outputSchema,
                "annotations" to schema.annotations
            ).filterValues { it != null }
        } else {
            mapOf(
                "error" to "Operation not found",
                "operation" to operationName
            )
        }
    }

    /**
     * Generate JSON response for _server_info
     */
    fun generateServerInfoResponse(service: DiscoverableService): ServerInfo {
        return service.getServerInfo()
    }
}

/**
 * Simple implementation of DiscoverableService
 */
class SimpleDiscoverableService(
    private val operations: List<OperationSchema>,
    private val serverInfo: ServerInfo
) : DiscoverableService {

    override fun listOperations() = operations

    override fun getSchema(name: String) = operations.find { it.name == name }

    override fun getServerInfo() = serverInfo.copy(operationsCount = operations.size)
}

