package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.time.Instant;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Represents a client connection in a WebSocket server.
 *
 * <p>This class tracks information about a connected client including
 * connection metadata, statistics, and state.
 *
 * <p>Example usage:
 * <pre>{@code
 * server.addEventListener(new ServerEventListener() {
 *     @Override
 *     public void onClientConnected(ClientConnection client) {
 *         System.out.println("Client connected: " + client.getId());
 *         System.out.println("Remote address: " + client.getRemoteAddress());
 *     }
 * });
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
public class ClientConnection {

    private final String id;
    private final String remoteAddress;
    private final Instant connectedAt;
    private final Map<String, String> metadata;
    private final AtomicLong messagesSent = new AtomicLong(0);
    private final AtomicLong messagesReceived = new AtomicLong(0);
    private final AtomicLong bytesSent = new AtomicLong(0);
    private final AtomicLong bytesReceived = new AtomicLong(0);

    /**
     * Creates a new client connection.
     *
     * @param remoteAddress the remote address of the client
     */
    public ClientConnection(@NotNull String remoteAddress) {
        this(UUID.randomUUID().toString(), remoteAddress, new HashMap<>());
    }

    /**
     * Creates a new client connection with the given ID.
     *
     * @param id the connection ID
     * @param remoteAddress the remote address
     * @param metadata connection metadata
     */
    public ClientConnection(@NotNull String id, @NotNull String remoteAddress,
                           @Nullable Map<String, String> metadata) {
        this.id = id;
        this.remoteAddress = remoteAddress;
        this.connectedAt = Instant.now();
        this.metadata = metadata != null ? new HashMap<>(metadata) : new HashMap<>();
    }

    /**
     * Returns the connection ID.
     *
     * @return the connection ID
     */
    @NotNull
    public String getId() {
        return id;
    }

    /**
     * Returns the remote address.
     *
     * @return the remote address
     */
    @NotNull
    public String getRemoteAddress() {
        return remoteAddress;
    }

    /**
     * Returns the connection timestamp.
     *
     * @return when the connection was established
     */
    @NotNull
    public Instant getConnectedAt() {
        return connectedAt;
    }

    /**
     * Returns the connection metadata.
     *
     * @return a copy of the metadata map
     */
    @NotNull
    public Map<String, String> getMetadata() {
        return new HashMap<>(metadata);
    }

    /**
     * Adds or updates metadata.
     *
     * @param key metadata key
     * @param value metadata value
     */
    public void putMetadata(@NotNull String key, @NotNull String value) {
        metadata.put(key, value);
    }

    /**
     * Returns the number of messages sent to this client.
     *
     * @return message count
     */
    public long getMessagesSent() {
        return messagesSent.get();
    }

    /**
     * Returns the number of messages received from this client.
     *
     * @return message count
     */
    public long getMessagesReceived() {
        return messagesReceived.get();
    }

    /**
     * Returns the number of bytes sent to this client.
     *
     * @return byte count
     */
    public long getBytesSent() {
        return bytesSent.get();
    }

    /**
     * Returns the number of bytes received from this client.
     *
     * @return byte count
     */
    public long getBytesReceived() {
        return bytesReceived.get();
    }

    /**
     * Records a sent message.
     *
     * @param bytes number of bytes sent
     */
    public void recordMessageSent(long bytes) {
        messagesSent.incrementAndGet();
        bytesSent.addAndGet(bytes);
    }

    /**
     * Records a received message.
     *
     * @param bytes number of bytes received
     */
    public void recordMessageReceived(long bytes) {
        messagesReceived.incrementAndGet();
        bytesReceived.addAndGet(bytes);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ClientConnection that = (ClientConnection) o;
        return Objects.equals(id, that.id);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id);
    }

    @Override
    public String toString() {
        return "ClientConnection{" +
               "id='" + id + '\'' +
               ", remoteAddress='" + remoteAddress + '\'' +
               ", connectedAt=" + connectedAt +
               ", messagesSent=" + messagesSent.get() +
               ", messagesReceived=" + messagesReceived.get() +
               '}';
    }
}

