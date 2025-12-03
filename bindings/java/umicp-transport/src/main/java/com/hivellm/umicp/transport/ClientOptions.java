package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;

import java.time.Duration;
import java.util.Objects;

/**
 * Configuration options for WebSocket client.
 *
 * <p>This class provides configuration for WebSocket client connections including
 * timeouts, reconnection settings, and connection parameters.
 *
 * <p>Example usage:
 * <pre>{@code
 * ClientOptions options = ClientOptions.builder()
 *     .connectTimeout(Duration.ofSeconds(10))
 *     .heartbeatInterval(Duration.ofSeconds(30))
 *     .maxReconnectAttempts(5)
 *     .autoReconnect(true)
 *     .build();
 *
 * UMICPWebSocketClient client = new UMICPWebSocketClient(url, options);
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 * @see UMICPWebSocketClient
 */
public class ClientOptions {

    private final Duration connectTimeout;
    private final Duration heartbeatInterval;
    private final int maxReconnectAttempts;
    private final Duration reconnectDelay;
    private final Duration maxReconnectDelay;
    private final double reconnectBackoffMultiplier;
    private final boolean autoReconnect;
    private final int maxMessageQueueSize;
    private final boolean compression;

    /**
     * Private constructor. Use {@link Builder} to create instances.
     */
    private ClientOptions(Duration connectTimeout, Duration heartbeatInterval,
                         int maxReconnectAttempts, Duration reconnectDelay,
                         Duration maxReconnectDelay, double reconnectBackoffMultiplier,
                         boolean autoReconnect, int maxMessageQueueSize,
                         boolean compression) {
        this.connectTimeout = connectTimeout;
        this.heartbeatInterval = heartbeatInterval;
        this.maxReconnectAttempts = maxReconnectAttempts;
        this.reconnectDelay = reconnectDelay;
        this.maxReconnectDelay = maxReconnectDelay;
        this.reconnectBackoffMultiplier = reconnectBackoffMultiplier;
        this.autoReconnect = autoReconnect;
        this.maxMessageQueueSize = maxMessageQueueSize;
        this.compression = compression;
    }

    // Getters
    @NotNull public Duration getConnectTimeout() { return connectTimeout; }
    @NotNull public Duration getHeartbeatInterval() { return heartbeatInterval; }
    public int getMaxReconnectAttempts() { return maxReconnectAttempts; }
    @NotNull public Duration getReconnectDelay() { return reconnectDelay; }
    @NotNull public Duration getMaxReconnectDelay() { return maxReconnectDelay; }
    public double getReconnectBackoffMultiplier() { return reconnectBackoffMultiplier; }
    public boolean isAutoReconnect() { return autoReconnect; }
    public int getMaxMessageQueueSize() { return maxMessageQueueSize; }
    public boolean isCompression() { return compression; }

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
    public static ClientOptions defaults() {
        return builder().build();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        ClientOptions that = (ClientOptions) o;
        return maxReconnectAttempts == that.maxReconnectAttempts &&
               Double.compare(that.reconnectBackoffMultiplier, reconnectBackoffMultiplier) == 0 &&
               autoReconnect == that.autoReconnect &&
               maxMessageQueueSize == that.maxMessageQueueSize &&
               compression == that.compression &&
               Objects.equals(connectTimeout, that.connectTimeout) &&
               Objects.equals(heartbeatInterval, that.heartbeatInterval) &&
               Objects.equals(reconnectDelay, that.reconnectDelay) &&
               Objects.equals(maxReconnectDelay, that.maxReconnectDelay);
    }

    @Override
    public int hashCode() {
        return Objects.hash(connectTimeout, heartbeatInterval, maxReconnectAttempts,
                          reconnectDelay, maxReconnectDelay, reconnectBackoffMultiplier,
                          autoReconnect, maxMessageQueueSize, compression);
    }

    @Override
    public String toString() {
        return "ClientOptions{" +
               "connectTimeout=" + connectTimeout +
               ", heartbeatInterval=" + heartbeatInterval +
               ", maxReconnectAttempts=" + maxReconnectAttempts +
               ", reconnectDelay=" + reconnectDelay +
               ", autoReconnect=" + autoReconnect +
               ", maxMessageQueueSize=" + maxMessageQueueSize +
               ", compression=" + compression +
               '}';
    }

    /**
     * Builder for ClientOptions.
     */
    public static class Builder {
        private Duration connectTimeout = Duration.ofSeconds(10);
        private Duration heartbeatInterval = Duration.ofSeconds(30);
        private int maxReconnectAttempts = 5;
        private Duration reconnectDelay = Duration.ofSeconds(1);
        private Duration maxReconnectDelay = Duration.ofSeconds(30);
        private double reconnectBackoffMultiplier = 2.0;
        private boolean autoReconnect = true;
        private int maxMessageQueueSize = 1000;
        private boolean compression = true;

        /**
         * Sets the connection timeout.
         *
         * @param timeout the timeout duration
         * @return this builder
         */
        @NotNull
        public Builder connectTimeout(@NotNull Duration timeout) {
            this.connectTimeout = timeout;
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
         * Sets the maximum reconnect attempts.
         *
         * @param attempts number of attempts (0 = no reconnect, -1 = infinite)
         * @return this builder
         */
        @NotNull
        public Builder maxReconnectAttempts(int attempts) {
            this.maxReconnectAttempts = attempts;
            return this;
        }

        /**
         * Sets the initial reconnect delay.
         *
         * @param delay the initial delay
         * @return this builder
         */
        @NotNull
        public Builder reconnectDelay(@NotNull Duration delay) {
            this.reconnectDelay = delay;
            return this;
        }

        /**
         * Sets the maximum reconnect delay.
         *
         * @param maxDelay the maximum delay
         * @return this builder
         */
        @NotNull
        public Builder maxReconnectDelay(@NotNull Duration maxDelay) {
            this.maxReconnectDelay = maxDelay;
            return this;
        }

        /**
         * Sets the reconnect backoff multiplier.
         *
         * @param multiplier the backoff multiplier
         * @return this builder
         */
        @NotNull
        public Builder reconnectBackoffMultiplier(double multiplier) {
            this.reconnectBackoffMultiplier = multiplier;
            return this;
        }

        /**
         * Sets whether auto-reconnect is enabled.
         *
         * @param enabled true to enable auto-reconnect
         * @return this builder
         */
        @NotNull
        public Builder autoReconnect(boolean enabled) {
            this.autoReconnect = enabled;
            return this;
        }

        /**
         * Sets the maximum message queue size.
         *
         * @param size the queue size
         * @return this builder
         */
        @NotNull
        public Builder maxMessageQueueSize(int size) {
            this.maxMessageQueueSize = size;
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
         * Builds the ClientOptions instance.
         *
         * @return a new ClientOptions instance
         */
        @NotNull
        public ClientOptions build() {
            return new ClientOptions(
                connectTimeout,
                heartbeatInterval,
                maxReconnectAttempts,
                reconnectDelay,
                maxReconnectDelay,
                reconnectBackoffMultiplier,
                autoReconnect,
                maxMessageQueueSize,
                compression
            );
        }
    }
}

