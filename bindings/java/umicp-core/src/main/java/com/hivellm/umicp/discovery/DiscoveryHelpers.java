package com.hivellm.umicp.discovery;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Helper functions for generating discovery responses.
 * Provides utilities for creating MCP-compatible response formats.
 *
 * @author HiveLLM Team
 * @version 0.2.0
 * @since 0.2.0
 */
public class DiscoveryHelpers {

    /**
     * Generate JSON response for _list_operations endpoint.
     *
     * @param service The discoverable service
     * @return Map containing operations list and metadata
     */
    public static Map<String, Object> generateOperationsResponse(DiscoverableService service) {
        List<OperationSchema> operations = service.listOperations();
        ServerInfo info = service.getServerInfo();

        Map<String, Object> response = new HashMap<>();
        response.put("operations", operations);
        response.put("count", operations.size());
        response.put("protocol", info.getProtocol());
        response.put("mcp_compatible", info.getMcpCompatible() != null ? info.getMcpCompatible() : false);

        return response;
    }

    /**
     * Generate JSON response for _get_schema endpoint.
     *
     * @param service The discoverable service
     * @param operationName Name of the operation to get schema for
     * @return Map containing schema or error information
     */
    public static Map<String, Object> generateSchemaResponse(DiscoverableService service, String operationName) {
        OperationSchema schema = service.getSchema(operationName);

        if (schema != null) {
            Map<String, Object> response = new HashMap<>();
            response.put("name", schema.getName());
            response.put("input_schema", schema.getInputSchema());

            if (schema.getTitle() != null) {
                response.put("title", schema.getTitle());
            }
            if (schema.getDescription() != null) {
                response.put("description", schema.getDescription());
            }
            if (schema.getOutputSchema() != null) {
                response.put("output_schema", schema.getOutputSchema());
            }
            if (schema.getAnnotations() != null) {
                response.put("annotations", schema.getAnnotations());
            }

            return response;
        }

        Map<String, Object> errorResponse = new HashMap<>();
        errorResponse.put("error", "Operation not found");
        errorResponse.put("operation", operationName);
        return errorResponse;
    }

    /**
     * Generate JSON response for _server_info endpoint.
     *
     * @param service The discoverable service
     * @return ServerInfo object
     */
    public static ServerInfo generateServerInfoResponse(DiscoverableService service) {
        return service.getServerInfo();
    }
}
