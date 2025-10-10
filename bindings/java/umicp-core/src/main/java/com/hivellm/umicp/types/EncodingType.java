package com.hivellm.umicp.types;

/**
 * Enumeration of UMICP encoding types for numerical data.
 *
 * <p>Encoding types specify how numerical vectors are stored and transmitted:
 * <ul>
 *   <li>{@link #FLOAT32} - 32-bit floating point (single precision)</li>
 *   <li>{@link #FLOAT64} - 64-bit floating point (double precision)</li>
 *   <li>{@link #INT32} - 32-bit signed integer</li>
 *   <li>{@link #INT64} - 64-bit signed integer (long)</li>
 *   <li>{@link #UINT8} - 8-bit unsigned integer (byte)</li>
 *   <li>{@link #UINT16} - 16-bit unsigned integer (short)</li>
 *   <li>{@link #UINT32} - 32-bit unsigned integer</li>
 *   <li>{@link #UINT64} - 64-bit unsigned integer</li>
 * </ul>
 *
 * <p>Example usage:
 * <pre>{@code
 * PayloadHint hint = PayloadHint.builder()
 *     .type(PayloadType.VECTOR)
 *     .encoding(EncodingType.FLOAT32)
 *     .count(768)
 *     .size(768 * 4) // 4 bytes per float32
 *     .build();
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see PayloadType
 * @see PayloadHint
 */
public enum EncodingType {
    /**
     * 32-bit floating point (IEEE 754 single precision).
     * Most common for ML embeddings.
     * Size: 4 bytes per value.
     */
    FLOAT32(0, 4),

    /**
     * 64-bit floating point (IEEE 754 double precision).
     * Higher precision, larger size.
     * Size: 8 bytes per value.
     */
    FLOAT64(1, 8),

    /**
     * 32-bit signed integer.
     * Range: -2^31 to 2^31-1.
     * Size: 4 bytes per value.
     */
    INT32(2, 4),

    /**
     * 64-bit signed integer (long).
     * Range: -2^63 to 2^63-1.
     * Size: 8 bytes per value.
     */
    INT64(3, 8),

    /**
     * 8-bit unsigned integer.
     * Range: 0 to 255.
     * Size: 1 byte per value.
     */
    UINT8(4, 1),

    /**
     * 16-bit unsigned integer.
     * Range: 0 to 65535.
     * Size: 2 bytes per value.
     */
    UINT16(5, 2),

    /**
     * 32-bit unsigned integer.
     * Range: 0 to 2^32-1.
     * Size: 4 bytes per value.
     */
    UINT32(6, 4),

    /**
     * 64-bit unsigned integer.
     * Range: 0 to 2^64-1.
     * Size: 8 bytes per value.
     */
    UINT64(7, 8);

    private final int value;
    private final int bytesPerValue;

    /**
     * Constructs an EncodingType with the given numeric value and size.
     *
     * @param value the numeric value representing this encoding type
     * @param bytesPerValue the number of bytes per value for this encoding
     */
    EncodingType(int value, int bytesPerValue) {
        this.value = value;
        this.bytesPerValue = bytesPerValue;
    }

    /**
     * Returns the numeric value of this encoding type.
     *
     * @return the numeric value (0-7)
     */
    public int getValue() {
        return value;
    }

    /**
     * Returns the number of bytes per value for this encoding.
     *
     * @return the size in bytes (1, 2, 4, or 8)
     */
    public int getBytesPerValue() {
        return bytesPerValue;
    }

    /**
     * Calculates the total size in bytes for a given count of values.
     *
     * @param count the number of values
     * @return the total size in bytes
     */
    public int calculateTotalBytes(int count) {
        return count * bytesPerValue;
    }

    /**
     * Returns the EncodingType corresponding to the given numeric value.
     *
     * @param value the numeric value
     * @return the corresponding EncodingType
     * @throws IllegalArgumentException if the value is invalid
     */
    public static EncodingType fromValue(int value) {
        for (EncodingType type : values()) {
            if (type.value == value) {
                return type;
            }
        }
        throw new IllegalArgumentException("Invalid EncodingType value: " + value);
    }

    /**
     * Checks if this encoding type is floating point.
     *
     * @return true if FLOAT32 or FLOAT64, false otherwise
     */
    public boolean isFloatingPoint() {
        return this == FLOAT32 || this == FLOAT64;
    }

    /**
     * Checks if this encoding type is integer.
     *
     * @return true if any integer type, false otherwise
     */
    public boolean isInteger() {
        return !isFloatingPoint();
    }

    /**
     * Checks if this encoding type is signed.
     *
     * @return true if INT32 or INT64, false otherwise
     */
    public boolean isSigned() {
        return this == INT32 || this == INT64;
    }

    /**
     * Checks if this encoding type is unsigned.
     *
     * @return true if any UINT type, false otherwise
     */
    public boolean isUnsigned() {
        return name().startsWith("UINT");
    }
}

