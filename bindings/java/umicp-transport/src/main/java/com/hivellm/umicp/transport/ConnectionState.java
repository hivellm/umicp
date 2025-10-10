package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;

/**
 * Represents the state of a WebSocket connection.
 *
 * <p>Connection state transitions:
 * <pre>
 * DISCONNECTED → CONNECTING → CONNECTED → DISCONNECTING → DISCONNECTED
 *                      ↓            ↓
 *                 DISCONNECTED  DISCONNECTING
 * </pre>
 *
 * <p>Thread-safe state management ensures atomic transitions.
 *
 * @author HiveLLM Team
 * @version 0.1.1
 * @since 0.1.1
 */
public enum ConnectionState {
    /**
     * Initial state, not connected.
     */
    DISCONNECTED(false, false),

    /**
     * Connecting to remote endpoint.
     */
    CONNECTING(false, true),

    /**
     * Connected and ready for communication.
     */
    CONNECTED(true, false),

    /**
     * Disconnecting from remote endpoint.
     */
    DISCONNECTING(true, true);

    private final boolean connected;
    private final boolean transitioning;

    /**
     * Constructor.
     *
     * @param connected whether this state represents an active connection
     * @param transitioning whether this is a transitional state
     */
    ConnectionState(boolean connected, boolean transitioning) {
        this.connected = connected;
        this.transitioning = transitioning;
    }

    /**
     * Checks if this state represents an active connection.
     *
     * @return true if connected
     */
    public boolean isConnected() {
        return connected;
    }

    /**
     * Checks if this is a transitional state.
     *
     * @return true if transitioning
     */
    public boolean isTransitioning() {
        return transitioning;
    }

    /**
     * Checks if transition to the target state is valid.
     *
     * @param target the target state
     * @return true if transition is valid
     */
    public boolean canTransitionTo(@NotNull ConnectionState target) {
        switch (this) {
            case DISCONNECTED:
                return target == CONNECTING;

            case CONNECTING:
                return target == CONNECTED || target == DISCONNECTED || target == DISCONNECTING;

            case CONNECTED:
                return target == DISCONNECTING || target == DISCONNECTED;

            case DISCONNECTING:
                return target == DISCONNECTED;

            default:
                return false;
        }
    }

    /**
     * Gets the expected next state after successful operation.
     *
     * @return the next logical state
     */
    @NotNull
    public ConnectionState getNextState() {
        switch (this) {
            case DISCONNECTED:
                return CONNECTING;
            case CONNECTING:
                return CONNECTED;
            case CONNECTED:
                return DISCONNECTING;
            case DISCONNECTING:
                return DISCONNECTED;
            default:
                return DISCONNECTED;
        }
    }

    /**
     * Checks if operations can be performed in this state.
     *
     * @return true if operations are allowed
     */
    public boolean canOperate() {
        return this == CONNECTED;
    }

    /**
     * Checks if connection can be initiated in this state.
     *
     * @return true if connection can be initiated
     */
    public boolean canConnect() {
        return this == DISCONNECTED;
    }

    /**
     * Checks if disconnection can be initiated in this state.
     *
     * @return true if disconnection can be initiated
     */
    public boolean canDisconnect() {
        return this == CONNECTED || this == CONNECTING;
    }
}

