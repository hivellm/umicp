<?php

declare(strict_types=1);

namespace UMICP\Exception;

use Exception;
use Throwable;

/**
 * Base exception for all UMICP-related errors
 *
 * @package UMICP\Exception
 */
class UMICPException extends Exception
{
    /**
     * Optional context data for the exception
     *
     * @var array<string, mixed>|null
     */
    protected ?array $context = null;

    /**
     * Create a new UMICP exception
     *
     * @param string $message Exception message
     * @param int $code Exception code
     * @param Throwable|null $previous Previous exception
     * @param array<string, mixed>|null $context Optional context data
     */
    public function __construct(
        string $message = "",
        int $code = 0,
        ?Throwable $previous = null,
        ?array $context = null
    ) {
        parent::__construct($message, $code, $previous);
        $this->context = $context;
    }

    /**
     * Get the exception context
     *
     * @return array<string, mixed>|null
     */
    public function getContext(): ?array
    {
        return $this->context;
    }

    /**
     * Check if context exists
     *
     * @return bool
     */
    public function hasContext(): bool
    {
        return $this->context !== null && !empty($this->context);
    }

    /**
     * Get a specific context value
     *
     * @param string $key Context key
     * @param mixed $default Default value if key not found
     * @return mixed
     */
    public function getContextValue(string $key, mixed $default = null): mixed
    {
        return $this->context[$key] ?? $default;
    }

    /**
     * Convert exception to array
     *
     * @return array<string, mixed>
     */
    public function toArray(): array
    {
        return [
            'message' => $this->getMessage(),
            'code' => $this->getCode(),
            'file' => $this->getFile(),
            'line' => $this->getLine(),
            'context' => $this->context,
            'trace' => $this->getTraceAsString(),
        ];
    }

    /**
     * Convert exception to string with context
     *
     * @return string
     */
    public function __toString(): string
    {
        $str = parent::__toString();

        if ($this->hasContext()) {
            $str .= "\nContext: " . json_encode($this->context, JSON_PRETTY_PRINT);
        }

        return $str;
    }
}

