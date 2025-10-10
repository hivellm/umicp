package com.hivellm.umicp.types;

/**
 * Enumeration of UMICP payload types.
 *
 * <p>Payload types indicate the format and structure of message data:
 * <ul>
 *   <li>{@link #VECTOR} - Numerical vector data (embeddings, features)</li>
 *   <li>{@link #TEXT} - Text/string data</li>
 *   <li>{@link #METADATA} - Key-value metadata</li>
 *   <li>{@link #BINARY} - Raw binary data</li>
 * </ul>
 *
 * <p>Example usage:
 * <pre>{@code
 * PayloadHint hint = PayloadHint.builder()
 *     .type(PayloadType.VECTOR)
 *     .encoding(EncodingType.FLOAT32)
 *     .count(768)
 *     .build();
 *
 * envelope.setPayloadHint(hint);
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see PayloadHint
 * @see EncodingType
 */
public enum PayloadType {
    /**
     * Vector/array of numerical values.
     * Commonly used for embeddings, feature vectors, or matrix data.
     */
    VECTOR(0),

    /**
     * Text or string data.
     * UTF-8 encoded text messages.
     */
    TEXT(1),

    /**
     * Metadata in key-value format.
     * Similar to capabilities but for payload data.
     */
    METADATA(2),

    /**
     * Raw binary data.
     * Arbitrary binary content.
     */
    BINARY(3);

    private final int value;

    /**
     * Constructs a PayloadType with the given numeric value.
     *
     * @param value the numeric value representing this payload type
     */
    PayloadType(int value) {
        this.value = value;
    }

    /**
     * Returns the numeric value of this payload type.
     *
     * @return the numeric value (0-3)
     */
    public int getValue() {
        return value;
    }

    /**
     * Returns the PayloadType corresponding to the given numeric value.
     *
     * @param value the numeric value
     * @return the corresponding PayloadType
     * @throws IllegalArgumentException if the value is invalid
     */
    public static PayloadType fromValue(int value) {
        for (PayloadType type : values()) {
            if (type.value == value) {
                return type;
            }
        }
        throw new IllegalArgumentException("Invalid PayloadType value: " + value);
    }

    /**
     * Checks if this payload type contains numerical data.
     *
     * @return true if this is a VECTOR type, false otherwise
     */
    public boolean isNumerical() {
        return this == VECTOR;
    }

    /**
     * Checks if this payload type contains text data.
     *
     * @return true if this is a TEXT type, false otherwise
     */
    public boolean isText() {
        return this == TEXT;
    }

    /**
     * Checks if this payload type contains structured data.
     *
     * @return true if this is a METADATA type, false otherwise
     */
    public boolean isStructured() {
        return this == METADATA;
    }
}

