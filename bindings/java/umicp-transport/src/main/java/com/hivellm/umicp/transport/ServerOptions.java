package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;

import java.time.Duration;
import java.util.Objects;

/**
 * Configuration options for WebSocket server.
 *
 * <p>This class provides configuration for WebSocket server including
 * connection limits, timeouts, and server parameters.
 *
 * <p>Example usage:
 * <pre>{@code
 * ServerOptions options = ServerOptions.builder()
 *     .maxClients(1000)
 *     .maxPayloadSize(100 * 1024 * 1024) // 100MB
 *     .heartbeatInterval(Duration.ofSeconds(30))
 *     .compression(true)
 *     .path("/umicp")
 *     .build();
 *
 * UMICPWebSocketServer server = new UMICPWebSocketServer(8080, options);
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see UMICPWebSocketServer
 */
public class ServerOptions {

    private final int maxClients;
    private final long maxPayloadSize;
    private final Duration heartbeatInterval;
    private final boolean compression;
    private final String path;
    private final Duration clientTimeout;

    /**
     * Private constructor. Use {@link Builder} to create instances.
     */
    private ServerOptions(int maxClients, long maxPayloadSize,
                         Duration heartbeatInterval, boolean compression,
                         String path, Duration clientTimeout) {
        this.maxClients = maxClients;
        this.maxPayloadSize = maxPayloadSize;
        this.heartbeatInterval = heartbeatInterval;
        this.compression = compression;
        this.path = path;
        this.clientTimeout = clientTimeout;
    }

    // Getters
    public int getMaxClients() { return maxClients; }
    public long getMaxPayloadSize() { return maxPayloadSize; }
    @NotNull public Duration getHeartbeatInterval() { return heartbeatInterval; }
    public boolean isCompression() { return compression; }
    @NotNull public String getPath() { return path; }
    @NotNull public Duration getClientTimeout() { return clientTimeout; }

    /**
     * Creates a new builder with default values.
     *
     * @return a new builder instance
     */
    @NotNull
    public static Builder builder() {
        return new Builder();
    }

    /**
     * Creates a builder with default options.
     *
     * @return default options
     */
    @NotNull
    public static ServerOptions defaults() {
        return builder().build();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ServerOptions that = (ServerOptions) o;
        return maxClients == that.maxClients &&
               maxPayloadSize == that.maxPayloadSize &&
               compression == that.compression &&
               Objects.equals(heartbeatInterval, that.heartbeatInterval) &&
               Objects.equals(path, that.path) &&
               Objects.equals(clientTimeout, that.clientTimeout);
    }

    @Override
    public int hashCode() {
        return Objects.hash(maxClients, maxPayloadSize, heartbeatInterval,
                          compression, path, clientTimeout);
    }

    @Override
    public String toString() {
        return "ServerOptions{" +
               "maxClients=" + maxClients +
               ", maxPayloadSize=" + maxPayloadSize +
               ", heartbeatInterval=" + heartbeatInterval +
               ", compression=" + compression +
               ", path='" + path + '\'' +
               ", clientTimeout=" + clientTimeout +
               '}';
    }

    /**
     * Builder for ServerOptions.
     */
    public static class Builder {
        private int maxClients = 1000;
        private long maxPayloadSize = 100 * 1024 * 1024; // 100MB
        private Duration heartbeatInterval = Duration.ofSeconds(30);
        private boolean compression = true;
        private String path = "/umicp";
        private Duration clientTimeout = Duration.ofMinutes(5);

        /**
         * Sets the maximum number of concurrent clients.
         *
         * @param maxClients maximum clients
         * @return this builder
         */
        @NotNull
        public Builder maxClients(int maxClients) {
            if (maxClients <= 0) {
                throw new IllegalArgumentException("maxClients must be positive");
            }
            this.maxClients = maxClients;
            return this;
        }

        /**
         * Sets the maximum payload size in bytes.
         *
         * @param maxPayloadSize maximum payload size
         * @return this builder
         */
        @NotNull
        public Builder maxPayloadSize(long maxPayloadSize) {
            if (maxPayloadSize <= 0) {
                throw new IllegalArgumentException("maxPayloadSize must be positive");
            }
            this.maxPayloadSize = maxPayloadSize;
            return this;
        }

        /**
         * Sets the heartbeat interval.
         *
         * @param interval the heartbeat interval
         * @return this builder
         */
        @NotNull
        public Builder heartbeatInterval(@NotNull Duration interval) {
            this.heartbeatInterval = interval;
            return this;
        }

        /**
         * Sets whether compression is enabled.
         *
         * @param enabled true to enable compression
         * @return this builder
         */
        @NotNull
        public Builder compression(boolean enabled) {
            this.compression = enabled;
            return this;
        }

        /**
         * Sets the WebSocket path.
         *
         * @param path the path (e.g., "/umicp")
         * @return this builder
         */
        @NotNull
        public Builder path(@NotNull String path) {
            this.path = path;
            return this;
        }

        /**
         * Sets the client timeout duration.
         *
         * @param timeout the timeout duration
         * @return this builder
         */
        @NotNull
        public Builder clientTimeout(@NotNull Duration timeout) {
            this.clientTimeout = timeout;
            return this;
        }

        /**
         * Builds the ServerOptions instance.
         *
         * @return a new ServerOptions instance
         */
        @NotNull
        public ServerOptions build() {
            return new ServerOptions(
                maxClients,
                maxPayloadSize,
                heartbeatInterval,
                compression,
                path,
                clientTimeout
            );
        }
    }
}

