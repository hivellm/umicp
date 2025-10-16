package com.hivellm.umicp.discovery;

import java.util.List;

/**
 * UMICP v0.2.1 Tool Discovery
 * MCP-compatible automatic tool introspection
 */

/**
 * Interface for services that support tool discovery.
 * Allows automatic introspection of available operations and their schemas.
 *
 * @author HiveLLM Team
 * @version 0.2.1
 * @since 0.2.1
 */
public interface DiscoverableService {

    /**
     * List all available operations with their schemas.
     *
     * @return List of operation schemas
     */
    List<OperationSchema> listOperations();

    /**
     * Get schema for a specific operation by name.
     *
     * @param name Operation name
     * @return Operation schema, or null if not found
     */
    OperationSchema getSchema(String name);

    /**
     * Get server information and metadata.
     *
     * @return Server information
     */
    ServerInfo getServerInfo();
}
