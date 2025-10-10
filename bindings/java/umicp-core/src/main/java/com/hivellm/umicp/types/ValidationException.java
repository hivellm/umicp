package com.hivellm.umicp.types;

/**
 * Exception thrown when validation errors occur.
 *
 * <p>This exception is thrown when an Envelope or other UMICP object
 * fails validation checks.
 *
 * <p>Common scenarios:
 * <ul>
 *   <li>Missing required fields (from, to, operation)</li>
 *   <li>Invalid field values</li>
 *   <li>Invalid capabilities</li>
 *   <li>Invalid payload hints</li>
 *   <li>Protocol violations</li>
 * </ul>
 *
 * <p>Example:
 * <pre>{@code
 * Envelope envelope = new Envelope()
 *     .setFrom("client");
 *     // Missing 'to' and 'operation'
 *
 * try {
 *     if (!envelope.validate()) {
 *         throw new ValidationException("Envelope is missing required fields");
 *     }
 * } catch (ValidationException e) {
 *     logger.error("Validation failed: {}", e.getMessage());
 * }
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
public class ValidationException extends UMICPException {

    private static final long serialVersionUID = 1L;

    /**
     * Constructs a new validation exception with null as its detail message.
     */
    public ValidationException() {
        super();
    }

    /**
     * Constructs a new validation exception with the specified detail message.
     *
     * @param message the detail message
     */
    public ValidationException(String message) {
        super(message);
    }

    /**
     * Constructs a new validation exception with the specified detail message and cause.
     *
     * @param message the detail message
     * @param cause the cause of this exception
     */
    public ValidationException(String message, Throwable cause) {
        super(message, cause);
    }

    /**
     * Constructs a new validation exception with the specified cause.
     *
     * @param cause the cause of this exception
     */
    public ValidationException(Throwable cause) {
        super(cause);
    }
}

