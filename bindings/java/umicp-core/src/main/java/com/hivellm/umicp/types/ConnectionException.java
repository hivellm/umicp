package com.hivellm.umicp.types;

/**
 * Exception thrown when connection-specific errors occur.
 *
 * <p>This exception is a specialization of {@link TransportException} for
 * connection-related failures.
 *
 * <p>Common scenarios:
 * <ul>
 *   <li>Failed to establish connection</li>
 *   <li>Connection timeout</li>
 *   <li>Connection refused</li>
 *   <li>Connection reset</li>
 *   <li>TLS/SSL handshake failure</li>
 * </ul>
 *
 * <p>Example:
 * <pre>{@code
 * try {
 *     peer.connectToPeer("ws://unreachable:8080").get();
 * } catch (ExecutionException e) {
 *     if (e.getCause() instanceof ConnectionException) {
 *         logger.warn("Could not connect to peer, will retry later");
 *     }
 * }
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see TransportException
 */
public class ConnectionException extends TransportException {

    private static final long serialVersionUID = 1L;

    /**
     * Constructs a new connection exception with null as its detail message.
     */
    public ConnectionException() {
        super();
    }

    /**
     * Constructs a new connection exception with the specified detail message.
     *
     * @param message the detail message
     */
    public ConnectionException(String message) {
        super(message);
    }

    /**
     * Constructs a new connection exception with the specified detail message and cause.
     *
     * @param message the detail message
     * @param cause the cause of this exception
     */
    public ConnectionException(String message, Throwable cause) {
        super(message, cause);
    }

    /**
     * Constructs a new connection exception with the specified cause.
     *
     * @param cause the cause of this exception
     */
    public ConnectionException(Throwable cause) {
        super(cause);
    }
}

