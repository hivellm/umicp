package com.hivellm.umicp.transport;

import com.hivellm.umicp.core.Envelope;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

/**
 * Event listener interface for server events.
 *
 * <p>This interface defines callback methods for server-related events including
 * client connections, message reception, and errors.
 *
 * <p>All methods have default implementations (no-op), so you only need to
 * override the methods you're interested in.
 *
 * <p>Example usage:
 * <pre>{@code
 * server.addEventListener(new ServerEventListener() {
 *     @Override
 *     public void onMessage(Envelope envelope, ClientConnection client) {
 *         System.out.println("Received from: " + client.getId());
 *         // Process message
 *     }
 *
 *     @Override
 *     public void onClientConnected(ClientConnection client) {
 *         System.out.println("Client connected: " + client.getRemoteAddress());
 *     }
 * });
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
public interface ServerEventListener {

    /**
     * Called when the server is started.
     */
    default void onServerStarted() {
        // Default implementation does nothing
    }

    /**
     * Called when the server is stopped.
     */
    default void onServerStopped() {
        // Default implementation does nothing
    }

    /**
     * Called when a client connects.
     *
     * @param client the connected client
     */
    default void onClientConnected(@NotNull ClientConnection client) {
        // Default implementation does nothing
    }

    /**
     * Called when a client disconnects.
     *
     * @param client the disconnected client
     * @param code the close code
     * @param reason the close reason
     */
    default void onClientDisconnected(@NotNull ClientConnection client,
                                     int code, @NotNull String reason) {
        // Default implementation does nothing
    }

    /**
     * Called when a message is received from a client.
     *
     * @param envelope the received envelope
     * @param client the client that sent the message
     */
    default void onMessage(@NotNull Envelope envelope, @NotNull ClientConnection client) {
        // Default implementation does nothing
    }

    /**
     * Called when an error occurs.
     *
     * @param error the exception that occurred
     * @param client the client associated with the error (may be null)
     */
    default void onError(@NotNull Exception error, @Nullable ClientConnection client) {
        // Default implementation does nothing
    }

    /**
     * Called when a message is sent to a client.
     *
     * @param envelope the sent envelope
     * @param client the client that received the message
     */
    default void onMessageSent(@NotNull Envelope envelope, @NotNull ClientConnection client) {
        // Default implementation does nothing
    }
}

