package com.hivellm.umicp.types;

/**
 * Enumeration of UMICP operation types.
 *
 * <p>Operation types define the purpose and handling of UMICP messages:
 * <ul>
 *   <li>{@link #CONTROL} - Control and management messages</li>
 *   <li>{@link #DATA} - Regular data messages</li>
 *   <li>{@link #ACK} - Acknowledgment messages</li>
 *   <li>{@link #ERROR} - Error messages</li>
 *   <li>{@link #REQUEST} - Request messages expecting a response</li>
 *   <li>{@link #RESPONSE} - Response messages to requests</li>
 * </ul>
 *
 * <p>Example usage:
 * <pre>{@code
 * Envelope envelope = new Envelope()
 *     .setOperation(OperationType.DATA)
 *     .setFrom("client-001")
 *     .setTo("server-001");
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see Envelope
 */
public enum OperationType {
    /**
     * Control or management message.
     * Used for protocol-level operations like handshake, ping/pong, etc.
     */
    CONTROL(0),

    /**
     * Regular data message.
     * The most common message type for application data.
     */
    DATA(1),

    /**
     * Acknowledgment message.
     * Confirms receipt and processing of a message.
     */
    ACK(2),

    /**
     * Error message.
     * Indicates an error occurred during message processing.
     */
    ERROR(3),

    /**
     * Request message.
     * Expects a response from the receiver.
     */
    REQUEST(4),

    /**
     * Response message.
     * Sent in reply to a request message.
     */
    RESPONSE(5);

    private final int value;

    /**
     * Constructs an OperationType with the given numeric value.
     *
     * @param value the numeric value representing this operation type
     */
    OperationType(int value) {
        this.value = value;
    }

    /**
     * Returns the numeric value of this operation type.
     *
     * @return the numeric value (0-5)
     */
    public int getValue() {
        return value;
    }

    /**
     * Returns the OperationType corresponding to the given numeric value.
     *
     * @param value the numeric value
     * @return the corresponding OperationType
     * @throws IllegalArgumentException if the value is invalid
     */
    public static OperationType fromValue(int value) {
        for (OperationType type : values()) {
            if (type.value == value) {
                return type;
            }
        }
        throw new IllegalArgumentException("Invalid OperationType value: " + value);
    }

    /**
     * Checks if this operation type expects a response.
     *
     * @return true if this is a REQUEST operation, false otherwise
     */
    public boolean expectsResponse() {
        return this == REQUEST;
    }

    /**
     * Checks if this operation type is a response.
     *
     * @return true if this is a RESPONSE or ACK operation, false otherwise
     */
    public boolean isResponse() {
        return this == RESPONSE || this == ACK;
    }

    /**
     * Checks if this operation type indicates an error.
     *
     * @return true if this is an ERROR operation, false otherwise
     */
    public boolean isError() {
        return this == ERROR;
    }
}

