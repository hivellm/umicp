package com.hivellm.umicp.types;

/**
 * Exception thrown when serialization or deserialization errors occur.
 *
 * <p>This exception is thrown when converting between Envelope objects and
 * their JSON/binary representations fails.
 *
 * <p>Common scenarios:
 * <ul>
 *   <li>Invalid JSON format</li>
 *   <li>Missing required fields</li>
 *   <li>Type conversion errors</li>
 *   <li>Encoding/decoding failures</li>
 *   <li>Corrupted data</li>
 * </ul>
 *
 * <p>Example:
 * <pre>{@code
 * try {
 *     Envelope envelope = UMICP.deserialize(jsonString);
 * } catch (SerializationException e) {
 *     logger.error("Failed to deserialize envelope: {}", e.getMessage());
 *     // Handle invalid data
 * }
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
public class SerializationException extends UMICPException {

    private static final long serialVersionUID = 1L;

    /**
     * Constructs a new serialization exception with null as its detail message.
     */
    public SerializationException() {
        super();
    }

    /**
     * Constructs a new serialization exception with the specified detail message.
     *
     * @param message the detail message
     */
    public SerializationException(String message) {
        super(message);
    }

    /**
     * Constructs a new serialization exception with the specified detail message and cause.
     *
     * @param message the detail message
     * @param cause the cause of this exception
     */
    public SerializationException(String message, Throwable cause) {
        super(message, cause);
    }

    /**
     * Constructs a new serialization exception with the specified cause.
     *
     * @param cause the cause of this exception
     */
    public SerializationException(Throwable cause) {
        super(cause);
    }
}

