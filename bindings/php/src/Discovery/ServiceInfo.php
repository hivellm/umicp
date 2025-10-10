<?php

namespace Umicp\Discovery;

use DateTime;
use DateTimeInterface;

/**
 * Service Information
 *
 * Contains metadata about a discovered service including
 * capabilities, address, version, and last seen timestamp.
 */
class ServiceInfo
{
    private string $serviceId;
    private string $name;
    private string $address;
    private array $capabilities = [];
    private array $metadata = [];
    private DateTimeInterface $lastSeen;
    private string $version;

    /**
     * Construct a new ServiceInfo
     *
     * @param string $serviceId Unique service identifier
     * @param string $name Service name
     * @param string $address Service address (e.g., ws://localhost:8080)
     */
    public function __construct(string $serviceId, string $name, string $address)
    {
        if (empty($serviceId)) {
            throw new \InvalidArgumentException('serviceId cannot be empty');
        }
        if (empty($name)) {
            throw new \InvalidArgumentException('name cannot be empty');
        }
        if (empty($address)) {
            throw new \InvalidArgumentException('address cannot be empty');
        }

        $this->serviceId = $serviceId;
        $this->name = $name;
        $this->address = $address;
        $this->lastSeen = new DateTime();
        $this->version = '1.0.0';
    }

    // Getters
    public function getServiceId(): string { return $this->serviceId; }
    public function getName(): string { return $this->name; }
    public function getAddress(): string { return $this->address; }
    public function getCapabilities(): array { return $this->capabilities; }
    public function getMetadata(): array { return $this->metadata; }
    public function getLastSeen(): DateTimeInterface { return $this->lastSeen; }
    public function getVersion(): string { return $this->version; }

    /**
     * Set version
     *
     * @param string $version Version string
     */
    public function setVersion(string $version): void
    {
        if (empty($version)) {
            throw new \InvalidArgumentException('version cannot be empty');
        }
        $this->version = $version;
    }

    /**
     * Add a capability to this service
     *
     * @param string $capability Capability name (e.g., "storage", "compute")
     */
    public function addCapability(string $capability): void
    {
        if (!empty($capability) && !in_array($capability, $this->capabilities, true)) {
            $this->capabilities[] = $capability;
        }
    }

    /**
     * Add metadata key-value pair
     *
     * @param string $key Metadata key
     * @param string $value Metadata value
     */
    public function addMetadata(string $key, string $value): void
    {
        if (!empty($key)) {
            $this->metadata[$key] = $value;
        }
    }

    /**
     * Check if service has a specific capability
     *
     * @param string $capability Capability to check
     * @return bool true if service has the capability
     */
    public function hasCapability(string $capability): bool
    {
        return in_array($capability, $this->capabilities, true);
    }

    /**
     * Get metadata value by key
     *
     * @param string $key Metadata key
     * @return string|null Metadata value, or null if not found
     */
    public function getMetadataValue(string $key): ?string
    {
        return $this->metadata[$key] ?? null;
    }

    /**
     * Update the last seen timestamp to now
     */
    public function updateLastSeen(): void
    {
        $this->lastSeen = new DateTime();
    }

    /**
     * Check if service is stale (not seen recently)
     *
     * @param int $timeoutSeconds Timeout duration in seconds
     * @return bool true if service hasn't been seen within timeout period
     */
    public function isStale(int $timeoutSeconds): bool
    {
        $now = new DateTime();
        $elapsed = $now->getTimestamp() - $this->lastSeen->getTimestamp();
        return $elapsed > $timeoutSeconds;
    }

    /**
     * Convert to array representation
     *
     * @return array<string, mixed>
     */
    public function toArray(): array
    {
        return [
            'serviceId' => $this->serviceId,
            'name' => $this->name,
            'address' => $this->address,
            'capabilities' => $this->capabilities,
            'metadata' => $this->metadata,
            'lastSeen' => $this->lastSeen->format('c'),
            'version' => $this->version,
        ];
    }
}

