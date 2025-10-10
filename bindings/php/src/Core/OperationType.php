<?php

declare(strict_types=1);

namespace UMICP\Core;

/**
 * UMICP operation types
 *
 * @package UMICP\Core
 */
enum OperationType: int
{
    /**
     * Control message
     */
    case CONTROL = 0;

    /**
     * Data message
     */
    case DATA = 1;

    /**
     * Acknowledgment message
     */
    case ACK = 2;

    /**
     * Error message
     */
    case ERROR = 3;

    /**
     * Request message
     */
    case REQUEST = 4;

    /**
     * Response message
     */
    case RESPONSE = 5;

    /**
     * Get operation type name
     *
     * @return string
     */
    public function getName(): string
    {
        return $this->name;
    }

    /**
     * Get operation type value
     *
     * @return int
     */
    public function getValue(): int
    {
        return $this->value;
    }

    /**
     * Check if this is a data operation
     *
     * @return bool
     */
    public function isData(): bool
    {
        return $this === self::DATA;
    }

    /**
     * Check if this is a control operation
     *
     * @return bool
     */
    public function isControl(): bool
    {
        return $this === self::CONTROL;
    }

    /**
     * Check if this is an acknowledgment
     *
     * @return bool
     */
    public function isAck(): bool
    {
        return $this === self::ACK;
    }

    /**
     * Check if this is an error
     *
     * @return bool
     */
    public function isError(): bool
    {
        return $this === self::ERROR;
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

