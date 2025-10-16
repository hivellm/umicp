<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Simple implementation of DiscoverableService
 */
class SimpleDiscoverableService implements DiscoverableService
{
    /**
     * @param OperationSchema[] $operations
     */
    public function __construct(
        private readonly array $operations,
        private readonly ServerInfo $serverInfo
    ) {
    }

    public function listOperations(): array
    {
        return $this->operations;
    }

    public function getSchema(string $name): ?OperationSchema
    {
        foreach ($this->operations as $operation) {
            if ($operation->name === $name) {
                return $operation;
            }
        }

        return null;
    }

    public function getServerInfo(): ServerInfo
    {
        $info = $this->serverInfo;
        $count = count($this->operations);

        // Update operations count
        return new ServerInfo(
            $info->server,
            $info->version,
            $info->protocol,
            $info->features,
            $count,
            $info->mcp_compatible,
            $info->metadata
        );
    }
}

