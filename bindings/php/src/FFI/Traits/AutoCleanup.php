<?php

declare(strict_types=1);

namespace UMICP\FFI\Traits;

use Throwable;

/**
 * Trait for automatic resource cleanup on object destruction
 *
 * Implements RAII (Resource Acquisition Is Initialization) pattern for PHP
 *
 * @package UMICP\FFI\Traits
 */
trait AutoCleanup
{
    /**
     * Cleanup callbacks to execute on destruction
     *
     * @var array<callable>
     */
    private array $cleanupCallbacks = [];

    /**
     * Flag to track if cleanup has been performed
     *
     * @var bool
     */
    private bool $cleanupPerformed = false;

    /**
     * Register a cleanup callback
     *
     * The callback will be executed when the object is destroyed
     * or when cleanup() is called manually.
     *
     * @param callable $callback Cleanup function to execute
     * @return void
     */
    protected function registerCleanup(callable $callback): void
    {
        $this->cleanupCallbacks[] = $callback;
    }

    /**
     * Destructor - execute all cleanup callbacks
     *
     * Errors during cleanup are logged but not thrown to avoid
     * issues in destructors.
     */
    public function __destruct()
    {
        $this->cleanup();
    }

    /**
     * Manually trigger cleanup
     *
     * Can be called multiple times safely (idempotent).
     * Useful for testing or explicit resource release.
     *
     * @return void
     */
    public function cleanup(): void
    {
        if ($this->cleanupPerformed) {
            return;
        }

        foreach ($this->cleanupCallbacks as $callback) {
            try {
                $callback();
            } catch (Throwable $e) {
                // Log error but don't throw in destructor
                error_log(sprintf(
                    '[UMICP] Cleanup error in %s: %s in %s:%d',
                    static::class,
                    $e->getMessage(),
                    $e->getFile(),
                    $e->getLine()
                ));

                // In debug mode, you might want to trigger a warning
                if (defined('UMICP_DEBUG') && UMICP_DEBUG) {
                    trigger_error(
                        'Cleanup error: ' . $e->getMessage(),
                        E_USER_WARNING
                    );
                }
            }
        }

        $this->cleanupCallbacks = [];
        $this->cleanupPerformed = true;
    }

    /**
     * Check if cleanup has been performed
     *
     * @return bool
     */
    public function isCleanedUp(): bool
    {
        return $this->cleanupPerformed;
    }

    /**
     * Get number of registered cleanup callbacks
     *
     * Useful for testing
     *
     * @return int
     */
    protected function getCleanupCallbackCount(): int
    {
        return count($this->cleanupCallbacks);
    }
}

