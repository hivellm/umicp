package com.hivellm.umicp.discovery;

import org.jetbrains.annotations.NotNull;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Helper methods for generating discovery responses
 */
public class DiscoveryHelpers {

    private DiscoveryHelpers() {
        // Utility class
    }

    /**
     * Generate JSON response for _list_operations
     */
    @NotNull
    public static Map<String, Object> generateOperationsResponse(@NotNull DiscoverableService service) {
        List<OperationSchema> operations = service.listOperations();
        ServerInfo info = service.getServerInfo();

        Map<String, Object> response = new HashMap<>();
        response.put("operations", operations.stream()
                .map(op -> mapOperationSchema(op))
                .collect(Collectors.toList()));
        response.put("count", operations.size());
        response.put("protocol", info.getProtocol());
        response.put("mcp_compatible", info.getMcpCompatible() != null ? info.getMcpCompatible() : false);

        return response;
    }

    /**
     * Generate JSON response for _get_schema
     */
    @NotNull
    public static Map<String, Object> generateSchemaResponse(@NotNull DiscoverableService service, @NotNull String operationName) {
        OperationSchema schema = service.getSchema(operationName);

        if (schema != null) {
            return mapOperationSchema(schema);
        }

        Map<String, Object> error = new HashMap<>();
        error.put("error", "Operation not found");
        error.put("operation", operationName);
        return error;
    }

    /**
     * Generate JSON response for _server_info
     */
    @NotNull
    public static Map<String, Object> generateServerInfoResponse(@NotNull DiscoverableService service) {
        return mapServerInfo(service.getServerInfo());
    }

    private static Map<String, Object> mapOperationSchema(OperationSchema schema) {
        Map<String, Object> map = new HashMap<>();
        map.put("name", schema.getName());
        if (schema.getTitle() != null) map.put("title", schema.getTitle());
        if (schema.getDescription() != null) map.put("description", schema.getDescription());
        map.put("input_schema", schema.getInputSchema());
        if (schema.getOutputSchema() != null) map.put("output_schema", schema.getOutputSchema());
        if (schema.getAnnotations() != null) map.put("annotations", schema.getAnnotations());
        return map;
    }

    private static Map<String, Object> mapServerInfo(ServerInfo info) {
        Map<String, Object> map = new HashMap<>();
        map.put("server", info.getServer());
        map.put("version", info.getVersion());
        map.put("protocol", info.getProtocol());
        if (info.getFeatures() != null) map.put("features", info.getFeatures());
        if (info.getOperationsCount() != null) map.put("operations_count", info.getOperationsCount());
        if (info.getMcpCompatible() != null) map.put("mcp_compatible", info.getMcpCompatible());
        if (info.getMetadata() != null) map.put("metadata", info.getMetadata());
        return map;
    }
}

