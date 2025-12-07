package com.hivellm.umicp.types;

/**
 * Exception thrown when transport-related errors occur.
 *
 * <p>This exception is thrown for errors related to network transport,
 * including WebSocket and HTTP communication issues.
 *
 * <p>Common scenarios:
 * <ul>
 *   <li>Connection failures</li>
 *   <li>Network timeouts</li>
 *   <li>Protocol errors</li>
 *   <li>Send/receive failures</li>
 * </ul>
 *
 * <p>Example:
 * <pre>{@code
 * try {
 *     client.connect().get();
 * } catch (ExecutionException e) {
 *     if (e.getCause() instanceof TransportException) {
 *         TransportException te = (TransportException) e.getCause();
 *         logger.error("Transport error: {}", te.getMessage());
 *         // Attempt reconnection
 *     }
 * }
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see ConnectionException
 */
public class TransportException extends UMICPException {

    private static final long serialVersionUID = 1L;

    /**
     * Constructs a new transport exception with null as its detail message.
     */
    public TransportException() {
        super();
    }

    /**
     * Constructs a new transport exception with the specified detail message.
     *
     * @param message the detail message
     */
    public TransportException(String message) {
        super(message);
    }

    /**
     * Constructs a new transport exception with the specified detail message and cause.
     *
     * @param message the detail message
     * @param cause the cause of this exception
     */
    public TransportException(String message, Throwable cause) {
        super(message, cause);
    }

    /**
     * Constructs a new transport exception with the specified cause.
     *
     * @param cause the cause of this exception
     */
    public TransportException(Throwable cause) {
        super(cause);
    }
}

