<?php

declare(strict_types=1);

namespace UMICP\Core;

/**
 * UMICP encoding types
 *
 * @package UMICP\Core
 */
enum EncodingType: int
{
    case FLOAT32 = 0;
    case FLOAT64 = 1;
    case INT32 = 2;
    case INT64 = 3;
    case UINT8 = 4;
    case UINT16 = 5;
    case UINT32 = 6;
    case UINT64 = 7;

    /**
     * Get encoding type name
     *
     * @return string
     */
    public function getName(): string
    {
        return $this->name;
    }

    /**
     * Get encoding type value
     *
     * @return int
     */
    public function getValue(): int
    {
        return $this->value;
    }

    /**
     * Get size in bytes
     *
     * @return int
     */
    public function getSize(): int
    {
        return match ($this) {
            self::FLOAT32 => 4,
            self::FLOAT64 => 8,
            self::INT32 => 4,
            self::INT64 => 8,
            self::UINT8 => 1,
            self::UINT16 => 2,
            self::UINT32 => 4,
            self::UINT64 => 8,
        };
    }

    /**
     * Check if floating point
     *
     * @return bool
     */
    public function isFloat(): bool
    {
        return $this === self::FLOAT32 || $this === self::FLOAT64;
    }

    /**
     * Check if integer
     *
     * @return bool
     */
    public function isInteger(): bool
    {
        return !$this->isFloat();
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

