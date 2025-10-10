package com.hivellm.umicp.types;

/**
 * Compression algorithm types supported by UMICP
 */
public enum CompressionType {
    /**
     * No compression
     */
    NONE(0, "none"),

    /**
     * GZIP compression (RFC 1952)
     */
    GZIP(1, "gzip"),

    /**
     * DEFLATE compression (RFC 1951)
     */
    DEFLATE(2, "deflate"),

    /**
     * LZ4 compression (fast compression)
     */
    LZ4(3, "lz4");

    private final int value;
    private final String algorithm;

    CompressionType(int value, String algorithm) {
        this.value = value;
        this.algorithm = algorithm;
    }

    /**
     * Get numeric value
     *
     * @return Compression type value
     */
    public int getValue() {
        return value;
    }

    /**
     * Get algorithm name
     *
     * @return Algorithm name (lowercase)
     */
    public String getAlgorithm() {
        return algorithm;
    }

    /**
     * Get CompressionType from numeric value
     *
     * @param value Numeric value
     * @return CompressionType enum value
     * @throws IllegalArgumentException if value is invalid
     */
    public static CompressionType fromValue(int value) {
        for (CompressionType type : values()) {
            if (type.value == value) {
                return type;
            }
        }
        throw new IllegalArgumentException("Invalid compression type value: " + value);
    }

    /**
     * Get CompressionType from algorithm name
     *
     * @param algorithm Algorithm name (case-insensitive)
     * @return CompressionType enum value
     * @throws IllegalArgumentException if algorithm is invalid
     */
    public static CompressionType fromAlgorithm(String algorithm) {
        if (algorithm == null) {
            throw new IllegalArgumentException("Algorithm cannot be null");
        }

        String lower = algorithm.toLowerCase();
        for (CompressionType type : values()) {
            if (type.algorithm.equals(lower)) {
                return type;
            }
        }
        throw new IllegalArgumentException("Invalid compression algorithm: " + algorithm);
    }

    @Override
    public String toString() {
        return algorithm;
    }
}

