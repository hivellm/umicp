<?php

declare(strict_types=1);

namespace UMICP\FFI;

use FFI;
use FFI\CData;
use UMICP\Exception\FFIException;

/**
 * FFI Bridge to C++ core library
 *
 * Singleton class that manages the FFI interface to the native UMICP library
 *
 * @package UMICP\FFI
 */
class FFIBridge
{
    /**
     * Singleton instance
     *
     * @var FFIBridge|null
     */
    private static ?FFIBridge $instance = null;

    /**
     * FFI instance
     *
     * @var FFI
     */
    private FFI $ffi;

    /**
     * Path to the shared library
     *
     * @var string
     */
    private string $libPath;

    /**
     * Path to the header file
     *
     * @var string
     */
    private string $headerPath;

    /**
     * Whether the bridge is initialized
     *
     * @var bool
     */
    private bool $initialized = false;

    /**
     * Private constructor for singleton pattern
     *
     * @param string $libPath Path to shared library
     * @param string $headerPath Path to header file
     * @throws FFIException If initialization fails
     */
    private function __construct(string $libPath, string $headerPath)
    {
        $this->libPath = $libPath;
        $this->headerPath = $headerPath;

        $this->validateEnvironment();
        $this->initializeFFI();
    }

    /**
     * Get singleton instance
     *
     * @param string|null $libPath Optional path to shared library
     * @param string|null $headerPath Optional path to header file
     * @return FFIBridge
     * @throws FFIException If initialization fails
     */
    public static function getInstance(?string $libPath = null, ?string $headerPath = null): self
    {
        if (self::$instance === null) {
            // Load from config if not provided
            if ($libPath === null || $headerPath === null) {
                $libPath ??= Config::get('ffi.lib_path');
                $headerPath ??= Config::get('ffi.header_path');

                if ($libPath === null || $headerPath === null) {
                    throw new FFIException(
                        'FFI library and header paths must be provided or configured'
                    );
                }
            }

            self::$instance = new self($libPath, $headerPath);
        }

        return self::$instance;
    }

    /**
     * Validate FFI environment
     *
     * @return void
     * @throws FFIException If environment is invalid
     */
    private function validateEnvironment(): void
    {
        // Check if FFI extension is loaded
        if (!extension_loaded('ffi')) {
            throw new FFIException(
                'FFI extension is not loaded. Enable it in php.ini with: extension=ffi'
            );
        }

        // Check if FFI is enabled
        $ffiEnabled = ini_get('ffi.enable');
        if ($ffiEnabled !== '1' && $ffiEnabled !== 'preload') {
            throw new FFIException(
                'FFI is disabled. Set ffi.enable=1 in php.ini'
            );
        }

        // Check if library exists
        if (!file_exists($this->libPath)) {
            throw new FFIException(
                message: "C++ library not found: {$this->libPath}",
                libraryPath: $this->libPath
            );
        }

        // Check if header exists
        if (!file_exists($this->headerPath)) {
            throw new FFIException(
                message: "FFI header not found: {$this->headerPath}",
                libraryPath: $this->libPath
            );
        }
    }

    /**
     * Initialize FFI interface
     *
     * @return void
     * @throws FFIException If initialization fails
     */
    private function initializeFFI(): void
    {
        try {
            $header = file_get_contents($this->headerPath);

            if ($header === false) {
                throw new FFIException(
                    message: "Failed to read FFI header: {$this->headerPath}",
                    libraryPath: $this->libPath
                );
            }

            $this->ffi = FFI::cdef($header, $this->libPath);
            $this->initialized = true;

        } catch (\FFI\Exception $e) {
            throw new FFIException(
                message: "Failed to initialize FFI: " . $e->getMessage(),
                previous: $e,
                ffiError: $e->getMessage(),
                libraryPath: $this->libPath
            );
        }
    }

    /**
     * Get FFI instance
     *
     * @return FFI
     */
    public function getFFI(): FFI
    {
        return $this->ffi;
    }

    /**
     * Check if bridge is initialized
     *
     * @return bool
     */
    public function isInitialized(): bool
    {
        return $this->initialized;
    }

    // ========================================================================
    // Envelope Operations
    // ========================================================================

    /**
     * Create envelope
     *
     * @return CData
     * @throws FFIException If creation fails
     */
    public function createEnvelope(): CData
    {
        $envelope = $this->ffi->umicp_envelope_create();

        if ($envelope === null || FFI::isNull($envelope)) {
            throw new FFIException('Failed to create envelope');
        }

        return $envelope;
    }

    /**
     * Destroy envelope
     *
     * @param CData $envelope
     * @return void
     */
    public function destroyEnvelope(CData $envelope): void
    {
        $this->ffi->umicp_envelope_destroy($envelope);
    }

    // ========================================================================
    // Matrix Operations
    // ========================================================================

    /**
     * Create matrix
     *
     * @return CData
     * @throws FFIException If creation fails
     */
    public function createMatrix(): CData
    {
        $matrix = $this->ffi->umicp_matrix_create();

        if ($matrix === null || FFI::isNull($matrix)) {
            throw new FFIException('Failed to create matrix');
        }

        return $matrix;
    }

    /**
     * Destroy matrix
     *
     * @param CData $matrix
     * @return void
     */
    public function destroyMatrix(CData $matrix): void
    {
        $this->ffi->umicp_matrix_destroy($matrix);
    }

    // ========================================================================
    // Frame Operations
    // ========================================================================

    /**
     * Create frame
     *
     * @return CData
     * @throws FFIException If creation fails
     */
    public function createFrame(): CData
    {
        $frame = $this->ffi->umicp_frame_create();

        if ($frame === null || FFI::isNull($frame)) {
            throw new FFIException('Failed to create frame');
        }

        return $frame;
    }

    /**
     * Destroy frame
     *
     * @param CData $frame
     * @return void
     */
    public function destroyFrame(CData $frame): void
    {
        $this->ffi->umicp_frame_destroy($frame);
    }

    // ========================================================================
    // Information
    // ========================================================================

    /**
     * Get library information
     *
     * @return array<string, mixed>
     */
    public function getInfo(): array
    {
        $info = [
            'lib_path' => $this->libPath,
            'header_path' => $this->headerPath,
            'ffi_version' => phpversion('ffi'),
            'php_version' => PHP_VERSION,
            'initialized' => $this->initialized,
        ];

        // Try to get UMICP version if available
        try {
            $version = $this->ffi->umicp_get_version();
            $info['umicp_version'] = FFI::string($version);
        } catch (\Throwable $e) {
            $info['umicp_version'] = 'unknown';
        }

        // Try to get build info if available
        try {
            $buildInfo = $this->ffi->umicp_get_build_info();
            $info['build_info'] = FFI::string($buildInfo);
        } catch (\Throwable $e) {
            $info['build_info'] = 'unknown';
        }

        return $info;
    }

    /**
     * Get library path
     *
     * @return string
     */
    public function getLibraryPath(): string
    {
        return $this->libPath;
    }

    /**
     * Get header path
     *
     * @return string
     */
    public function getHeaderPath(): string
    {
        return $this->headerPath;
    }

    /**
     * Reset singleton instance (mainly for testing)
     *
     * @return void
     */
    public static function reset(): void
    {
        self::$instance = null;
    }
}

