package com.hivellm.umicp.discovery;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.util.List;

/**
 * Interface for services that support tool discovery
 */
public interface DiscoverableService {

    /**
     * List all available operations with their schemas
     *
     * @return list of operation schemas
     */
    @NotNull
    List<OperationSchema> listOperations();

    /**
     * Get schema for a specific operation by name
     *
     * @param name operation name to look up
     * @return operation schema if found, null otherwise
     */
    @Nullable
    default OperationSchema getSchema(@NotNull String name) {
        return listOperations().stream()
                .filter(op -> op.getName().equals(name))
                .findFirst()
                .orElse(null);
    }

    /**
     * Get server information and metadata
     *
     * @return server information
     */
    @NotNull
    ServerInfo getServerInfo();
}

