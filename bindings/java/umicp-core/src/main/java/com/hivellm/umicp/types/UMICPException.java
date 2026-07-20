package com.hivellm.umicp.types;

/**
 * Base exception class for all UMICP-related exceptions.
 *
 * <p>This is the root of the UMICP exception hierarchy. All UMICP-specific
 * exceptions extend from this class, allowing for easy exception handling:
 *
 * <pre>{@code
 * try {
 *     envelope.validate();
 * } catch (UMICPException e) {
 *     // Handle any UMICP exception
 *     logger.error("UMICP error: {}", e.getMessage(), e);
 * }
 * }</pre>
 *
 * <p>Hierarchy:
 * <pre>
 * UMICPException
 * ├── TransportException
 * │   └── ConnectionException
 * ├── SerializationException
 * └── ValidationException
 * </pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
public class UMICPException extends Exception {

    private static final long serialVersionUID = 1L;

    /**
     * Constructs a new UMICP exception with null as its detail message.
     */
    public UMICPException() {
        super();
    }

    /**
     * Constructs a new UMICP exception with the specified detail message.
     *
     * @param message the detail message
     */
    public UMICPException(String message) {
        super(message);
    }

    /**
     * Constructs a new UMICP exception with the specified detail message and cause.
     *
     * @param message the detail message
     * @param cause the cause of this exception
     */
    public UMICPException(String message, Throwable cause) {
        super(message, cause);
    }

    /**
     * Constructs a new UMICP exception with the specified cause.
     *
     * @param cause the cause of this exception
     */
    public UMICPException(Throwable cause) {
        super(cause);
    }

    /**
     * Constructs a new UMICP exception with the specified detail message, cause,
     * suppression enabled or disabled, and writable stack trace enabled or disabled.
     *
     * @param message the detail message
     * @param cause the cause of this exception
     * @param enableSuppression whether suppression is enabled or disabled
     * @param writableStackTrace whether the stack trace should be writable
     */
    protected UMICPException(String message, Throwable cause,
                            boolean enableSuppression,
                            boolean writableStackTrace) {
        super(message, cause, enableSuppression, writableStackTrace);
    }
}

