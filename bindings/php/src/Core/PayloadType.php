<?php

declare(strict_types=1);

namespace UMICP\Core;

/**
 * UMICP payload types
 *
 * @package UMICP\Core
 */
enum PayloadType: int
{
    /**
     * Vector/embedding data
     */
    case VECTOR = 0;

    /**
     * Text data
     */
    case TEXT = 1;

    /**
     * Metadata
     */
    case METADATA = 2;

    /**
     * Binary data
     */
    case BINARY = 3;

    /**
     * Get payload type name
     *
     * @return string
     */
    public function getName(): string
    {
        return $this->name;
    }

    /**
     * Get payload type value
     *
     * @return int
     */
    public function getValue(): int
    {
        return $this->value;
    }

    /**
     * Convert to string
     *
     * @return string
     */
    public function toString(): string
    {
        return $this->name;
    }
}

