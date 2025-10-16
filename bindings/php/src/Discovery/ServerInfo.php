<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Server information for discovery
 *
 * @package UMICP\Discovery
 */
class ServerInfo
{
    /**
     * @param string $server Server name/identifier
     * @param string $version Server version
     * @param string $protocol Protocol version
     * @param array<string>|null $features List of supported features
     * @param int|null $operationsCount Number of available operations
     * @param bool|null $mcpCompatible MCP protocol compatibility flag
     * @param array<string, mixed>|null $metadata Additional server metadata
     */
    public function __construct(
        public readonly string $server,
        public readonly string $version,
        public readonly string $protocol,
        public readonly ?array $features = null,
        public readonly ?int $operationsCount = null,
        public readonly ?bool $mcpCompatible = null,
        public readonly ?array $metadata = null
    ) {
        if (empty($server)) {
            throw new \InvalidArgumentException('Server name cannot be empty');
        }
        if (empty($version)) {
            throw new \InvalidArgumentException('Version cannot be empty');
        }
        if (empty($protocol)) {
            throw new \InvalidArgumentException('Protocol cannot be empty');
        }
    }

    /**
     * Convert to array for JSON serialization
     *
     * @return array<string, mixed>
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
        if ($this->operationsCount !== null) {
            $result['operations_count'] = $this->operationsCount;
        }
        if ($this->mcpCompatible !== null) {
            $result['mcp_compatible'] = $this->mcpCompatible;
        }
        if ($this->metadata !== null) {
            $result['metadata'] = $this->metadata;
        }

        return $result;
    }
}
