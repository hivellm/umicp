<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Interface for services that support tool discovery
 *
 * @package UMICP\Discovery
 */
interface DiscoverableService
{
    /**
     * List all available operations with their schemas
     *
     * @return array<OperationSchema>
     */
    public function listOperations(): array;

    /**
     * Get schema for a specific operation by name
     *
     * @param string $name Operation name
     * @return OperationSchema|null
     */
    public function getSchema(string $name): ?OperationSchema;

    /**
     * Get server information and metadata
     *
     * @return ServerInfo
     */
    public function getServerInfo(): ServerInfo;
}
