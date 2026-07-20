package com.hivellm.umicp.transport;

import com.hivellm.umicp.core.Envelope;
import org.jetbrains.annotations.NotNull;

/**
 * Event listener interface for transport events.
 *
 * <p>This interface defines callback methods for various transport-related events
 * including connection state changes, message reception, and errors.
 *
 * <p>All methods have default implementations (no-op), so you only need to
 * override the methods you're interested in.
 *
 * <p>Example usage:
 * <pre>{@code
 * client.addEventListener(new TransportEventListener() {
 *     @Override
 *     public void onMessage(Envelope envelope) {
 *         System.out.println("Received: " + envelope.getMessageId());
 *     }
 *
 *     @Override
 *     public void onConnected() {
 *         System.out.println("Connected!");
 *     }
 *
 *     @Override
 *     public void onError(Exception error) {
 *         System.err.println("Error: " + error.getMessage());
 *     }
 * });
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
public interface TransportEventListener {

    /**
     * Called when a message is received.
     *
     * @param envelope the received envelope
     */
    default void onMessage(@NotNull Envelope envelope) {
        // Default implementation does nothing
    }

    /**
     * Called when connection is established.
     */
    default void onConnected() {
        // Default implementation does nothing
    }

    /**
     * Called when connection is closed.
     *
     * @param code the close code
     * @param reason the close reason
     */
    default void onDisconnected(int code, @NotNull String reason) {
        // Default implementation does nothing
    }

    /**
     * Called when an error occurs.
     *
     * @param error the exception that occurred
     */
    default void onError(@NotNull Exception error) {
        // Default implementation does nothing
    }

    /**
     * Called when reconnection attempts have failed.
     */
    default void onReconnectFailed() {
        // Default implementation does nothing
    }

    /**
     * Called when a message is sent successfully.
     *
     * @param envelope the sent envelope
     */
    default void onMessageSent(@NotNull Envelope envelope) {
        // Default implementation does nothing
    }
}

