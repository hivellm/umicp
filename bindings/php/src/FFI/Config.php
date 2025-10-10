<?php

declare(strict_types=1);

namespace UMICP\FFI;

use RuntimeException;

/**
 * Configuration management for UMICP
 *
 * @package UMICP\FFI
 */
class Config
{
    /**
     * Loaded configuration
     *
     * @var array<string, mixed>|null
     */
    private static ?array $config = null;

    /**
     * Path to the configuration file
     *
     * @var string|null
     */
    private static ?string $configPath = null;

    /**
     * Load configuration from file
     *
     * @param string|null $configPath Optional path to config file
     * @return array<string, mixed>
     * @throws RuntimeException If config file not found
     */
    public static function load(?string $configPath = null): array
    {
        if (self::$config === null || ($configPath !== null && $configPath !== self::$configPath)) {
            self::$configPath = $configPath ?? self::findConfigFile();

            if (!file_exists(self::$configPath)) {
                throw new RuntimeException(
                    "Configuration file not found: " . self::$configPath
                );
            }

            self::$config = require self::$configPath;

            if (!is_array(self::$config)) {
                throw new RuntimeException(
                    "Configuration file must return an array: " . self::$configPath
                );
            }
        }

        return self::$config;
    }

    /**
     * Get a configuration value
     *
     * @param string $key Configuration key (supports dot notation)
     * @param mixed $default Default value if key not found
     * @return mixed
     */
    public static function get(string $key, mixed $default = null): mixed
    {
        $config = self::load();

        // Support dot notation: 'ffi.lib_path'
        $keys = explode('.', $key);
        $value = $config;

        foreach ($keys as $k) {
            if (!is_array($value) || !array_key_exists($k, $value)) {
                return $default;
            }
            $value = $value[$k];
        }

        return $value;
    }

    /**
     * Set a configuration value
     *
     * @param string $key Configuration key (supports dot notation)
     * @param mixed $value Value to set
     * @return void
     */
    public static function set(string $key, mixed $value): void
    {
        if (self::$config === null) {
            self::load();
        }

        $keys = explode('.', $key);
        $config = &self::$config;

        $lastKey = array_pop($keys);

        foreach ($keys as $k) {
            if (!isset($config[$k]) || !is_array($config[$k])) {
                $config[$k] = [];
            }
            $config = &$config[$k];
        }

        $config[$lastKey] = $value;
    }

    /**
     * Check if a configuration key exists
     *
     * @param string $key Configuration key
     * @return bool
     */
    public static function has(string $key): bool
    {
        $config = self::load();

        $keys = explode('.', $key);
        $value = $config;

        foreach ($keys as $k) {
            if (!is_array($value) || !array_key_exists($k, $value)) {
                return false;
            }
            $value = $value[$k];
        }

        return true;
    }

    /**
     * Get all configuration
     *
     * @return array<string, mixed>
     */
    public static function all(): array
    {
        return self::load();
    }

    /**
     * Clear loaded configuration
     *
     * @return void
     */
    public static function clear(): void
    {
        self::$config = null;
        self::$configPath = null;
    }

    /**
     * Find configuration file in standard locations
     *
     * @return string
     * @throws RuntimeException If config file not found
     */
    private static function findConfigFile(): string
    {
        $possiblePaths = [
            __DIR__ . '/../../config/umicp.php',
            __DIR__ . '/../../../config/umicp.php',
            getcwd() . '/config/umicp.php',
            dirname($_SERVER['SCRIPT_FILENAME'] ?? '') . '/config/umicp.php',
        ];

        foreach ($possiblePaths as $path) {
            if (file_exists($path)) {
                return $path;
            }
        }

        throw new RuntimeException(
            'Configuration file not found. Searched locations: ' . PHP_EOL .
            implode(PHP_EOL, $possiblePaths) . PHP_EOL .
            'Please create config/umicp.php or specify path manually.'
        );
    }

    /**
     * Get the path to the configuration file
     *
     * @return string|null
     */
    public static function getConfigPath(): ?string
    {
        return self::$configPath;
    }
}

