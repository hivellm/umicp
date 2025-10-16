<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Server information for discovery
 */
class ServerInfo
{
    public function __construct(
        public readonly string $server,
        public readonly string $version,
        public readonly string $protocol,
        public readonly ?array $features = null,
        public readonly ?int $operations_count = null,
        public readonly ?bool $mcp_compatible = null,
        public readonly ?array $metadata = null
    ) {
    }

    /**
     * Convert to array for JSON serialization
     */
    public function toArray(): array
    {
        $result = [
            'server' => $this->server,
            'version' => $this->version,
            'protocol' => $this->protocol,
        ];

        if ($this->features !== null) {
            $result['features'] = $this->features;
        }

        if ($this->operations_count !== null) {
            $result['operations_count'] = $this->operations_count;
        }

        if ($this->mcp_compatible !== null) {
            $result['mcp_compatible'] = $this->mcp_compatible;
        }

        if ($this->metadata !== null) {
            $result['metadata'] = $this->metadata;
        }

        return $result;
    }
}

