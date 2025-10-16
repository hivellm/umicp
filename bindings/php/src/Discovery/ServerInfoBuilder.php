<?php

declare(strict_types=1);

namespace UMICP\Discovery;

/**
 * Builder for ServerInfo
 */
class ServerInfoBuilder
{
    private ?array $features = null;
    private ?int $operations_count = null;
    private ?bool $mcp_compatible = null;
    private ?array $metadata = null;

    public function __construct(
        private readonly string $server,
        private readonly string $version,
        private readonly string $protocol
    ) {
    }

    public function withFeatures(array $features): self
    {
        $this->features = $features;
        return $this;
    }

    public function withOperationsCount(int $count): self
    {
        $this->operations_count = $count;
        return $this;
    }

    public function withMcpCompatible(bool $compatible): self
    {
        $this->mcp_compatible = $compatible;
        return $this;
    }

    public function withMetadata(array $metadata): self
    {
        $this->metadata = $metadata;
        return $this;
    }

    public function build(): ServerInfo
    {
        return new ServerInfo(
            $this->server,
            $this->version,
            $this->protocol,
            $this->features,
            $this->operations_count,
            $this->mcp_compatible,
            $this->metadata
        );
    }
}

