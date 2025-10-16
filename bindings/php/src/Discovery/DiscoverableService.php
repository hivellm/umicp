<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Interface for services that support tool discovery
 */
interface DiscoverableService
{
    /**
     * List all available operations with their schemas
     *
     * @return OperationSchema[]
     */
    public function listOperations(): array;

    /**
     * Get schema for a specific operation by name
     */
    public function getSchema(string $name): ?OperationSchema;

    /**
     * Get server information and metadata
     */
    public function getServerInfo(): ServerInfo;
}

