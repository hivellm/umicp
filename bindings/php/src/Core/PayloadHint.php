<?php

declare(strict_types=1);

namespace UMICP\Core;

/**
 * Payload hint for UMICP messages
 *
 * @package UMICP\Core
 */
class PayloadHint
{
    /**
     * Create a new payload hint
     *
     * @param PayloadType|null $type Payload type
     * @param int|null $size Payload size in bytes
     * @param EncodingType|null $encoding Encoding type
     * @param int|null $count Number of elements
     */
    public function __construct(
        private ?PayloadType $type = null,
        private ?int $size = null,
        private ?EncodingType $encoding = null,
        private ?int $count = null
    ) {
    }

    /**
     * Get payload type
     *
     * @return PayloadType|null
     */
    public function getType(): ?PayloadType
    {
        return $this->type;
    }

    /**
     * Set payload type
     *
     * @param PayloadType|null $type
     * @return self
     */
    public function setType(?PayloadType $type): self
    {
        $this->type = $type;
        return $this;
    }

    /**
     * Get payload size
     *
     * @return int|null
     */
    public function getSize(): ?int
    {
        return $this->size;
    }

    /**
     * Set payload size
     *
     * @param int|null $size
     * @return self
     */
    public function setSize(?int $size): self
    {
        $this->size = $size;
        return $this;
    }

    /**
     * Get encoding type
     *
     * @return EncodingType|null
     */
    public function getEncoding(): ?EncodingType
    {
        return $this->encoding;
    }

    /**
     * Set encoding type
     *
     * @param EncodingType|null $encoding
     * @return self
     */
    public function setEncoding(?EncodingType $encoding): self
    {
        $this->encoding = $encoding;
        return $this;
    }

    /**
     * Get element count
     *
     * @return int|null
     */
    public function getCount(): ?int
    {
        return $this->count;
    }

    /**
     * Set element count
     *
     * @param int|null $count
     * @return self
     */
    public function setCount(?int $count): self
    {
        $this->count = $count;
        return $this;
    }

    /**
     * Convert to array
     *
     * @return array<string, mixed>
     */
    public function toArray(): array
    {
        return [
            'type' => $this->type?->getValue(),
            'size' => $this->size,
            'encoding' => $this->encoding?->getValue(),
            'count' => $this->count,
        ];
    }

    /**
     * Create from array
     *
     * @param array<string, mixed> $data
     * @return self
     */
    public static function fromArray(array $data): self
    {
        return new self(
            type: isset($data['type']) ? PayloadType::from($data['type']) : null,
            size: $data['size'] ?? null,
            encoding: isset($data['encoding']) ? EncodingType::from($data['encoding']) : null,
            count: $data['count'] ?? null
        );
    }
}

