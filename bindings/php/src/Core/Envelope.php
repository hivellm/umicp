<?php

declare(strict_types=1);

namespace UMICP\Core;

use FFI;
use FFI\CData;
use UMICP\Exception\SerializationException;
use UMICP\Exception\ValidationException;
use UMICP\FFI\FFIBridge;
use UMICP\FFI\Traits\AutoCleanup;
use UMICP\FFI\TypeConverter;

/**
 * UMICP Envelope - Message container with metadata
 *
 * @package UMICP\Core
 */
class Envelope
{
    use AutoCleanup;

    /**
     * Native C envelope instance
     *
     * @var CData
     */
    private CData $nativeEnvelope;

    /**
     * FFI bridge instance
     *
     * @var FFIBridge
     */
    private FFIBridge $ffi;

    /**
     * Cached values
     */
    private ?string $from = null;
    private ?string $to = null;
    private OperationType $operation;
    private ?string $messageId = null;
    private ?array $capabilities = null;
    private ?PayloadHint $payloadHint = null;

    /**
     * Create a new envelope
     *
     * @param string|null $from Sender identifier
     * @param string|null $to Recipient identifier
     * @param OperationType $operation Operation type
     * @param string|null $messageId Message identifier
     * @param array<string, string> $capabilities Message metadata
     * @param PayloadHint|null $payloadHint Payload hint
     */
    public function __construct(
        ?string $from = null,
        ?string $to = null,
        OperationType $operation = OperationType::DATA,
        ?string $messageId = null,
        array $capabilities = [],
        ?PayloadHint $payloadHint = null
    ) {
        $this->ffi = FFIBridge::getInstance();
        $this->nativeEnvelope = $this->ffi->createEnvelope();
        $this->operation = $operation;

        // Register cleanup
        $this->registerCleanup(fn() => $this->ffi->destroyEnvelope($this->nativeEnvelope));

        // Set initial values
        if ($from !== null) {
            $this->setFrom($from);
        }
        if ($to !== null) {
            $this->setTo($to);
        }
        $this->setOperation($operation);
        if ($messageId !== null) {
            $this->setMessageId($messageId);
        }
        if (!empty($capabilities)) {
            $this->setCapabilities($capabilities);
        }
        if ($payloadHint !== null) {
            $this->payloadHint = $payloadHint;
        }
    }

    /**
     * Set sender identifier
     *
     * @param string $from Sender identifier
     * @return self
     */
    public function setFrom(string $from): self
    {
        $this->from = $from;
        $this->ffi->getFFI()->umicp_envelope_set_from($this->nativeEnvelope, $from);
        return $this;
    }

    /**
     * Get sender identifier
     *
     * @return string|null
     */
    public function getFrom(): ?string
    {
        if ($this->from === null) {
            $cStr = $this->ffi->getFFI()->umicp_envelope_get_from($this->nativeEnvelope);
            if ($cStr !== null && !FFI::isNull($cStr)) {
                $this->from = FFI::string($cStr);
            }
        }
        return $this->from;
    }

    /**
     * Set recipient identifier
     *
     * @param string $to Recipient identifier
     * @return self
     */
    public function setTo(string $to): self
    {
        $this->to = $to;
        $this->ffi->getFFI()->umicp_envelope_set_to($this->nativeEnvelope, $to);
        return $this;
    }

    /**
     * Get recipient identifier
     *
     * @return string|null
     */
    public function getTo(): ?string
    {
        if ($this->to === null) {
            $cStr = $this->ffi->getFFI()->umicp_envelope_get_to($this->nativeEnvelope);
            if ($cStr !== null && !FFI::isNull($cStr)) {
                $this->to = FFI::string($cStr);
            }
        }
        return $this->to;
    }

    /**
     * Set operation type
     *
     * @param OperationType $operation Operation type
     * @return self
     */
    public function setOperation(OperationType $operation): self
    {
        $this->operation = $operation;
        $this->ffi->getFFI()->umicp_envelope_set_operation(
            $this->nativeEnvelope,
            $operation->value
        );
        return $this;
    }

    /**
     * Get operation type
     *
     * @return OperationType
     */
    public function getOperation(): OperationType
    {
        return $this->operation;
    }

    /**
     * Set message identifier
     *
     * @param string $messageId Message identifier
     * @return self
     */
    public function setMessageId(string $messageId): self
    {
        $this->messageId = $messageId;
        $this->ffi->getFFI()->umicp_envelope_set_message_id($this->nativeEnvelope, $messageId);
        return $this;
    }

    /**
     * Get message identifier
     *
     * @return string|null
     */
    public function getMessageId(): ?string
    {
        if ($this->messageId === null) {
            $cStr = $this->ffi->getFFI()->umicp_envelope_get_message_id($this->nativeEnvelope);
            if ($cStr !== null && !FFI::isNull($cStr)) {
                $this->messageId = FFI::string($cStr);
            }
        }
        return $this->messageId;
    }

    /**
     * Set capabilities (metadata)
     *
     * @param array<string, string> $capabilities Key-value metadata
     * @return self
     * @throws SerializationException If JSON encoding fails
     */
    public function setCapabilities(array $capabilities): self
    {
        $this->capabilities = $capabilities;
        $json = TypeConverter::phpArrayToJson($capabilities);
        $this->ffi->getFFI()->umicp_envelope_set_capabilities($this->nativeEnvelope, $json);
        return $this;
    }

    /**
     * Get capabilities
     *
     * @return array<string, string>
     * @throws SerializationException If JSON decoding fails
     */
    public function getCapabilities(): array
    {
        if ($this->capabilities === null) {
            $cStr = $this->ffi->getFFI()->umicp_envelope_get_capabilities($this->nativeEnvelope);
            if ($cStr !== null && !FFI::isNull($cStr)) {
                $json = FFI::string($cStr);
                $this->capabilities = TypeConverter::jsonToPhpArray($json);
            } else {
                $this->capabilities = [];
            }
        }
        return $this->capabilities;
    }

    /**
     * Get a specific capability value
     *
     * @param string $key Capability key
     * @param string|null $default Default value
     * @return string|null
     */
    public function getCapability(string $key, ?string $default = null): ?string
    {
        $capabilities = $this->getCapabilities();
        return $capabilities[$key] ?? $default;
    }

    /**
     * Set a single capability
     *
     * @param string $key Capability key
     * @param string $value Capability value
     * @return self
     */
    public function setCapability(string $key, string $value): self
    {
        $capabilities = $this->getCapabilities();
        $capabilities[$key] = $value;
        return $this->setCapabilities($capabilities);
    }

    /**
     * Check if capability exists
     *
     * @param string $key Capability key
     * @return bool
     */
    public function hasCapability(string $key): bool
    {
        $capabilities = $this->getCapabilities();
        return isset($capabilities[$key]);
    }

    /**
     * Remove a capability
     *
     * @param string $key Capability key
     * @return self
     */
    public function removeCapability(string $key): self
    {
        $capabilities = $this->getCapabilities();
        unset($capabilities[$key]);
        return $this->setCapabilities($capabilities);
    }

    /**
     * Set payload hint
     *
     * @param PayloadHint $hint Payload hint
     * @return self
     */
    public function setPayloadHint(PayloadHint $hint): self
    {
        $this->payloadHint = $hint;
        return $this;
    }

    /**
     * Get payload hint
     *
     * @return PayloadHint|null
     */
    public function getPayloadHint(): ?PayloadHint
    {
        return $this->payloadHint;
    }

    /**
     * Serialize envelope to JSON
     *
     * @return string JSON string
     * @throws SerializationException If serialization fails
     */
    public function serialize(): string
    {
        $cStr = $this->ffi->getFFI()->umicp_envelope_serialize($this->nativeEnvelope);

        if ($cStr === null || FFI::isNull($cStr)) {
            throw new SerializationException('Failed to serialize envelope');
        }

        return FFI::string($cStr);
    }

    /**
     * Deserialize envelope from JSON
     *
     * @param string $json JSON string
     * @return self
     * @throws SerializationException If deserialization fails
     */
    public static function deserialize(string $json): self
    {
        $ffi = FFIBridge::getInstance();
        $nativeEnvelope = $ffi->getFFI()->umicp_envelope_deserialize($json);

        if ($nativeEnvelope === null || FFI::isNull($nativeEnvelope)) {
            throw new SerializationException('Failed to deserialize envelope');
        }

        // Parse JSON to get values
        try {
            $data = TypeConverter::jsonToPhpArray($json);
        } catch (\Throwable $e) {
            throw new SerializationException(
                'Failed to parse envelope JSON: ' . $e->getMessage(),
                0,
                $e
            );
        }

        // Create envelope from data
        return new self(
            from: $data['from'] ?? null,
            to: $data['to'] ?? null,
            operation: isset($data['operation']) ? OperationType::from($data['operation']) : OperationType::DATA,
            messageId: $data['messageId'] ?? null,
            capabilities: $data['capabilities'] ?? []
        );
    }

    /**
     * Validate envelope
     *
     * @return bool
     */
    public function validate(): bool
    {
        $result = $this->ffi->getFFI()->umicp_envelope_validate($this->nativeEnvelope);
        return $result === 1;
    }

    /**
     * Validate envelope and throw exception if invalid
     *
     * @return self
     * @throws ValidationException If validation fails
     */
    public function validateOrThrow(): self
    {
        if (!$this->validate()) {
            throw new ValidationException('Envelope validation failed');
        }
        return $this;
    }

    /**
     * Get envelope hash
     *
     * @return string SHA-256 hash
     */
    public function getHash(): string
    {
        $cStr = $this->ffi->getFFI()->umicp_envelope_get_hash($this->nativeEnvelope);

        if ($cStr === null || FFI::isNull($cStr)) {
            // Fallback to PHP hash if C++ doesn't provide it
            return hash('sha256', $this->serialize());
        }

        return FFI::string($cStr);
    }

    /**
     * Convert to array
     *
     * @return array<string, mixed>
     */
    public function toArray(): array
    {
        return [
            'from' => $this->getFrom(),
            'to' => $this->getTo(),
            'operation' => $this->operation->value,
            'messageId' => $this->getMessageId(),
            'capabilities' => $this->getCapabilities(),
            'payloadHint' => $this->payloadHint?->toArray(),
        ];
    }

    /**
     * Create envelope from array
     *
     * @param array<string, mixed> $data Array data
     * @return self
     */
    public static function fromArray(array $data): self
    {
        return new self(
            from: $data['from'] ?? null,
            to: $data['to'] ?? null,
            operation: isset($data['operation']) ? OperationType::from($data['operation']) : OperationType::DATA,
            messageId: $data['messageId'] ?? null,
            capabilities: $data['capabilities'] ?? [],
            payloadHint: isset($data['payloadHint']) ? PayloadHint::fromArray($data['payloadHint']) : null
        );
    }

    /**
     * Convert to string (JSON)
     *
     * @return string
     */
    public function __toString(): string
    {
        try {
            return $this->serialize();
        } catch (\Throwable $e) {
            return json_encode($this->toArray());
        }
    }
}

