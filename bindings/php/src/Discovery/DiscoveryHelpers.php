<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Helper functions for generating discovery responses
 *
 * @package UMICP\Discovery
 */
class DiscoveryHelpers
{
    /**
     * Generate JSON response for _list_operations
     *
     * @param DiscoverableService $service
     * @return array<string, mixed>
     */
    public static function generateOperationsResponse(DiscoverableService $service): array
    {
        $operations = $service->listOperations();
        $info = $service->getServerInfo();

        return [
            'operations' => array_map(fn($op) => $op->toArray(), $operations),
            'count' => count($operations),
            'protocol' => $info->protocol,
            'mcp_compatible' => $info->mcpCompatible ?? false,
        ];
    }

    /**
     * Generate JSON response for _get_schema
     *
     * @param DiscoverableService $service
     * @param string $operationName
     * @return array<string, mixed>
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
     *
     * @param DiscoverableService $service
     * @return array<string, mixed>
     */
    public static function generateServerInfoResponse(DiscoverableService $service): array
    {
        return $service->getServerInfo()->toArray();
    }
}
