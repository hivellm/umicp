<?php

/**
 * UMICP PHP Bindings Configuration
 *
 * Copy this file to config/umicp.php and adjust paths for your environment
 */

return [
    /**
     * FFI Configuration
     */
    'ffi' => [
        // Path to compiled C++ shared library
        'lib_path' => __DIR__ . '/../../../cpp/build/libumicp_core.so',  // Linux
        // 'lib_path' => __DIR__ . '/../../../cpp/build/libumicp_core.dylib',  // macOS
        // 'lib_path' => __DIR__ . '/../../../cpp/build/Release/umicp_core.dll',  // Windows

        // Path to C FFI header
        'header_path' => __DIR__ . '/../ffi/umicp_core.h',

        // Enable FFI debug mode
        'debug' => false,
    ],

    /**
     * Transport Layer Configuration
     */
    'transport' => [
        // Default connection timeout (ms)
        'default_timeout' => 10000,

        // Maximum reconnection attempts
        'max_reconnect_attempts' => 3,

        // Reconnection delay (ms)
        'reconnect_delay' => 5000,

        // Heartbeat/ping interval (ms)
        'heartbeat_interval' => 30000,

        // Enable compression
        'compression' => true,
    ],

    /**
     * WebSocket Server Configuration
     */
    'server' => [
        // Default server port
        'default_port' => 20081,

        // Default server host
        'default_host' => '0.0.0.0',

        // Default WebSocket path
        'default_path' => '/umicp',

        // Enable per-message compression
        'compression' => true,

        // Maximum payload size (bytes)
        'max_payload' => 100 * 1024 * 1024,  // 100MB

        // Client timeout (ms)
        'client_timeout' => 60000,
    ],

    /**
     * Performance Configuration
     */
    'performance' => [
        // Enable OPcache preloading (production only)
        'enable_opcache_preload' => false,

        // Batch size for bulk operations
        'batch_size' => 1000,

        // Enable JIT compilation (PHP 8.0+)
        'enable_jit' => true,

        // Memory limit for large operations
        'memory_limit' => '512M',
    ],

    /**
     * Logging Configuration
     */
    'logging' => [
        // Enable logging
        'enabled' => true,

        // Log level (debug, info, warning, error)
        'level' => 'info',

        // Log file path
        'path' => __DIR__ . '/../logs/umicp.log',

        // Log to stderr
        'stderr' => false,
    ],

    /**
     * Security Configuration
     */
    'security' => [
        // Enable TLS/SSL (not implemented yet)
        'enable_tls' => false,

        // TLS certificate path
        'cert_path' => null,

        // TLS key path
        'key_path' => null,

        // Validate peer certificates
        'verify_peer' => true,
    ],

    /**
     * Development Configuration
     */
    'development' => [
        // Enable debug mode
        'debug' => false,

        // Enable verbose logging
        'verbose' => false,

        // Enable memory leak detection
        'detect_memory_leaks' => false,

        // Enable profiling
        'profiling' => false,
    ],

    /**
     * Peer Configuration (for MultiplexedPeer)
     */
    'peer' => [
        // Default peer ID prefix
        'id_prefix' => 'php-peer',

        // Auto-generate peer ID
        'auto_generate_id' => true,

        // Enable auto-handshake protocol
        'auto_protocol' => true,

        // Handshake timeout (ms)
        'handshake_timeout' => 10000,

        // Default peer metadata
        'default_metadata' => [
            'language' => 'php',
            'runtime' => 'php-' . PHP_VERSION,
            'platform' => PHP_OS,
        ],
    ],
];

