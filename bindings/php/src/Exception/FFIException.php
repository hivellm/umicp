<?php

declare(strict_types=1);

namespace UMICP\Exception;

use Throwable;

/**
 * Exception thrown when FFI operations fail
 *
 * @package UMICP\Exception
 */
class FFIException extends UMICPException
{
    /**
     * FFI-specific error message
     *
     * @var string|null
     */
    private ?string $ffiError = null;

    /**
     * Path to the library that caused the error
     *
     * @var string|null
     */
    private ?string $libraryPath = null;

    /**
     * Create a new FFI exception
     *
     * @param string $message Exception message
     * @param int $code Exception code
     * @param Throwable|null $previous Previous exception
     * @param string|null $ffiError FFI-specific error
     * @param string|null $libraryPath Library path
     * @param array<string, mixed>|null $context Additional context
     */
    public function __construct(
        string $message = "",
        int $code = 0,
        ?Throwable $previous = null,
        ?string $ffiError = null,
        ?string $libraryPath = null,
        ?array $context = null
    ) {
        parent::__construct($message, $code, $previous, $context);
        $this->ffiError = $ffiError;
        $this->libraryPath = $libraryPath;
    }

    /**
     * Get the FFI error message
     *
     * @return string|null
     */
    public function getFFIError(): ?string
    {
        return $this->ffiError;
    }

    /**
     * Get the library path
     *
     * @return string|null
     */
    public function getLibraryPath(): ?string
    {
        return $this->libraryPath;
    }

    /**
     * Check if FFI error exists
     *
     * @return bool
     */
    public function hasFFIError(): bool
    {
        return $this->ffiError !== null;
    }

    /**
     * Convert to array with FFI details
     *
     * @return array<string, mixed>
     */
    public function toArray(): array
    {
        $array = parent::toArray();
        $array['ffi_error'] = $this->ffiError;
        $array['library_path'] = $this->libraryPath;

        return $array;
    }
}

