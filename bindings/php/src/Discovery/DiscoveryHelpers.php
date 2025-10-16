<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Helper functions for generating discovery responses
 */
class DiscoveryHelpers
{
    /**
     * Generate JSON response for _list_operations
     */
    public static function generateOperationsResponse(DiscoverableService $service): array
    {
        $operations = $service->listOperations();
        $info = $service->getServerInfo();

        return [
            'operations' => array_map(fn($op) => $op->toArray(), $operations),
            'count' => count($operations),
            'protocol' => $info->protocol,
            'mcp_compatible' => $info->mcp_compatible ?? false,
        ];
    }

    /**
     * Generate JSON response for _get_schema
     */
    public static function generateSchemaResponse(DiscoverableService $service, string $operationName): array
    {
        $schema = $service->getSchema($operationName);

        if ($schema !== null) {
            return $schema->toArray();
        }

        return [
            'error' => 'Operation not found',
            'operation' => $operationName,
        ];
    }

    /**
     * Generate JSON response for _server_info
     */
    public static function generateServerInfoResponse(DiscoverableService $service): array
    {
        return $service->getServerInfo()->toArray();
    }
}

