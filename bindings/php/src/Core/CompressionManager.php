<?php

namespace UMICP\Core;

use UMICP\Exception\TransportException;

/**
 * UMICP Compression Manager
 *
 * Handles compression and decompression of data
 * Supports GZIP and DEFLATE algorithms
 *
 * @package HiveLLM\UMICP\Core
 */
class CompressionManager
{
    public const ALGORITHM_NONE = 'none';
    public const ALGORITHM_GZIP = 'gzip';
    public const ALGORITHM_DEFLATE = 'deflate';

    private string $algorithm;
    private int $level;

    /**
     * Constructor
     *
     * @param string $algorithm Compression algorithm
     * @param int $level Compression level (1-9)
     */
    public function __construct(string $algorithm = self::ALGORITHM_GZIP, int $level = 6)
    {
        $this->algorithm = $algorithm;
        $this->level = max(1, min(9, $level));
    }

    /**
     * Compress data
     *
     * @param string $data Data to compress
     * @return string Compressed data
     * @throws TransportException
     */
    public function compress(string $data): string
    {
        if (empty($data)) {
            return $data;
        }

        switch ($this->algorithm) {
            case self::ALGORITHM_NONE:
                return $data;

            case self::ALGORITHM_GZIP:
                $compressed = gzencode($data, $this->level);
                if ($compressed === false) {
                    throw new TransportException('GZIP compression failed');
                }
                return $compressed;

            case self::ALGORITHM_DEFLATE:
                $compressed = gzdeflate($data, $this->level);
                if ($compressed === false) {
                    throw new TransportException('DEFLATE compression failed');
                }
                return $compressed;

            default:
                throw new TransportException("Unsupported compression algorithm: {$this->algorithm}");
        }
    }

    /**
     * Decompress data
     *
     * @param string $compressedData Compressed data
     * @return string Decompressed data
     * @throws TransportException
     */
    public function decompress(string $compressedData): string
    {
        if (empty($compressedData)) {
            return $compressedData;
        }

        switch ($this->algorithm) {
            case self::ALGORITHM_NONE:
                return $compressedData;

            case self::ALGORITHM_GZIP:
                $decompressed = gzdecode($compressedData);
                if ($decompressed === false) {
                    throw new TransportException('GZIP decompression failed');
                }
                return $decompressed;

            case self::ALGORITHM_DEFLATE:
                $decompressed = gzinflate($compressedData);
                if ($decompressed === false) {
                    throw new TransportException('DEFLATE decompression failed');
                }
                return $decompressed;

            default:
                throw new TransportException("Unsupported compression algorithm: {$this->algorithm}");
        }
    }

    /**
     * Get compression ratio
     *
     * @param string $original Original data
     * @param string $compressed Compressed data
     * @return float Compression ratio
     */
    public static function getCompressionRatio(string $original, string $compressed): float
    {
        if (empty($original)) {
            return 1.0;
        }

        return strlen($compressed) / strlen($original);
    }

    /**
     * Get algorithm
     *
     * @return string Current algorithm
     */
    public function getAlgorithm(): string
    {
        return $this->algorithm;
    }

    /**
     * Get compression level
     *
     * @return int Compression level
     */
    public function getLevel(): int
    {
        return $this->level;
    }
}

