<?php

declare(strict_types=1);

namespace UMICP\FFI;

use FFI;
use FFI\CData;
use InvalidArgumentException;
use RuntimeException;

/**
 * Type conversion utilities between PHP and C types
 *
 * @package UMICP\FFI
 */
class TypeConverter
{
    /**
     * Convert PHP array to C float array
     *
     * @param array<float|int> $phpArray PHP array of numbers
     * @return CData C float array
     * @throws InvalidArgumentException If array is empty
     */
    public static function phpArrayToCFloatArray(array $phpArray): CData
    {
        $size = count($phpArray);

        if ($size === 0) {
            throw new InvalidArgumentException('Array cannot be empty');
        }

        $cArray = FFI::new("float[$size]");

        foreach ($phpArray as $i => $value) {
            $cArray[$i] = (float) $value;
        }

        return $cArray;
    }

    /**
     * Convert C float array to PHP array
     *
     * @param CData $cArray C float array
     * @param int $size Array size
     * @return array<float> PHP array
     * @throws InvalidArgumentException If size is invalid
     */
    public static function cFloatArrayToPhpArray(CData $cArray, int $size): array
    {
        if ($size <= 0) {
            throw new InvalidArgumentException('Size must be greater than 0');
        }

        $result = [];

        for ($i = 0; $i < $size; $i++) {
            $result[] = $cArray[$i];
        }

        return $result;
    }

    /**
     * Convert PHP string to C string
     *
     * @param string $phpString PHP string
     * @return CData C string (char array)
     */
    public static function phpStringToCString(string $phpString): CData
    {
        $len = strlen($phpString);
        $cString = FFI::new("char[$len + 1]", false);

        FFI::memcpy($cString, $phpString, $len);
        $cString[$len] = "\0";

        return $cString;
    }

    /**
     * Convert C string to PHP string
     *
     * @param CData $cString C string pointer
     * @return string PHP string
     */
    public static function cStringToPhpString(CData $cString): string
    {
        return FFI::string($cString);
    }

    /**
     * Convert PHP array to JSON string
     *
     * @param array<string, mixed> $array PHP associative array
     * @return string JSON string
     * @throws RuntimeException If JSON encoding fails
     */
    public static function phpArrayToJson(array $array): string
    {
        $json = json_encode($array, JSON_THROW_ON_ERROR);

        if ($json === false) {
            throw new RuntimeException(
                'JSON encoding failed: ' . json_last_error_msg()
            );
        }

        return $json;
    }

    /**
     * Convert JSON string to PHP array
     *
     * @param string $json JSON string
     * @return array<string, mixed> PHP array
     * @throws RuntimeException If JSON decoding fails
     */
    public static function jsonToPhpArray(string $json): array
    {
        try {
            $array = json_decode($json, true, 512, JSON_THROW_ON_ERROR);
        } catch (\JsonException $e) {
            throw new RuntimeException(
                'JSON decoding failed: ' . $e->getMessage(),
                0,
                $e
            );
        }

        if (!is_array($array)) {
            throw new RuntimeException('JSON did not decode to an array');
        }

        return $array;
    }

    /**
     * Convert PHP int array to C int array
     *
     * @param array<int> $phpArray PHP array of integers
     * @return CData C int array
     * @throws InvalidArgumentException If array is empty
     */
    public static function phpArrayToCIntArray(array $phpArray): CData
    {
        $size = count($phpArray);

        if ($size === 0) {
            throw new InvalidArgumentException('Array cannot be empty');
        }

        $cArray = FFI::new("int[$size]");

        foreach ($phpArray as $i => $value) {
            $cArray[$i] = (int) $value;
        }

        return $cArray;
    }

    /**
     * Convert C int array to PHP array
     *
     * @param CData $cArray C int array
     * @param int $size Array size
     * @return array<int> PHP array
     * @throws InvalidArgumentException If size is invalid
     */
    public static function cIntArrayToPhpArray(CData $cArray, int $size): array
    {
        if ($size <= 0) {
            throw new InvalidArgumentException('Size must be greater than 0');
        }

        $result = [];

        for ($i = 0; $i < $size; $i++) {
            $result[] = $cArray[$i];
        }

        return $result;
    }

    /**
     * Convert PHP double array to C double array
     *
     * @param array<float> $phpArray PHP array of doubles
     * @return CData C double array
     * @throws InvalidArgumentException If array is empty
     */
    public static function phpArrayToCDoubleArray(array $phpArray): CData
    {
        $size = count($phpArray);

        if ($size === 0) {
            throw new InvalidArgumentException('Array cannot be empty');
        }

        $cArray = FFI::new("double[$size]");

        foreach ($phpArray as $i => $value) {
            $cArray[$i] = (float) $value;
        }

        return $cArray;
    }

    /**
     * Convert C double array to PHP array
     *
     * @param CData $cArray C double array
     * @param int $size Array size
     * @return array<float> PHP array
     * @throws InvalidArgumentException If size is invalid
     */
    public static function cDoubleArrayToPhpArray(CData $cArray, int $size): array
    {
        if ($size <= 0) {
            throw new InvalidArgumentException('Size must be greater than 0');
        }

        $result = [];

        for ($i = 0; $i < $size; $i++) {
            $result[] = $cArray[$i];
        }

        return $result;
    }

    /**
     * Allocate C memory
     *
     * @param int $size Size in bytes
     * @param bool $zero Initialize to zero
     * @return CData C pointer
     * @throws InvalidArgumentException If size is invalid
     */
    public static function allocate(int $size, bool $zero = true): CData
    {
        if ($size <= 0) {
            throw new InvalidArgumentException('Size must be greater than 0');
        }

        return FFI::new("char[$size]", $zero);
    }

    /**
     * Free C memory (no-op in PHP, but kept for API consistency)
     *
     * @param CData $pointer C pointer
     * @return void
     */
    public static function free(CData $pointer): void
    {
        // PHP's FFI handles memory automatically
        // This method exists for API consistency
        unset($pointer);
    }
}

