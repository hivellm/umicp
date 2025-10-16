<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Simple implementation of DiscoverableService
 *
 * @package UMICP\Discovery
 */
class SimpleDiscoverableService implements DiscoverableService
{
    /**
     * @param array<OperationSchema> $operations
     * @param ServerInfo $serverInfo
     */
    public function __construct(
        private readonly array $operations,
        private readonly ServerInfo $serverInfo
    ) {}

    /**
     * @return array<OperationSchema>
     */
    public function listOperations(): array
    {
        return $this->operations;
    }

    /**
     * @param string $name
     * @return OperationSchema|null
     */
    public function getSchema(string $name): ?OperationSchema
    {
        foreach ($this->operations as $op) {
            if ($op->name === $name) {
                return $op;
            }
        }

        return null;
    }

    /**
     * @return ServerInfo
     */
    public function getServerInfo(): ServerInfo
    {
        return new ServerInfo(
            server: $this->serverInfo->server,
            version: $this->serverInfo->version,
            protocol: $this->serverInfo->protocol,
            features: $this->serverInfo->features,
            operationsCount: count($this->operations),
            mcpCompatible: $this->serverInfo->mcpCompatible,
            metadata: $this->serverInfo->metadata
        );
    }
}
