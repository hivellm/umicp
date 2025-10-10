package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.time.Duration;
import java.util.Objects;

/**
 * Configuration options for multiplexed WebSocket peer.
 */
public class PeerOptions {

    private final String peerId;
    private final boolean autoProtocol;
    private final Duration handshakeTimeout;
    private final ServerOptions serverOptions;
    private final ClientOptions clientOptions;

    private PeerOptions(String peerId, boolean autoProtocol, Duration handshakeTimeout,
                       ServerOptions serverOptions, ClientOptions clientOptions) {
        this.peerId = peerId;
        this.autoProtocol = autoProtocol;
        this.handshakeTimeout = handshakeTimeout;
        this.serverOptions = serverOptions != null ? serverOptions : ServerOptions.defaults();
        this.clientOptions = clientOptions != null ? clientOptions : ClientOptions.defaults();
    }

    @NotNull public String getPeerId() { return peerId; }
    public boolean isAutoProtocol() { return autoProtocol; }
    @NotNull public Duration getHandshakeTimeout() { return handshakeTimeout; }
    @NotNull public ServerOptions getServerOptions() { return serverOptions; }
    @NotNull public ClientOptions getClientOptions() { return clientOptions; }

    @NotNull
    public static Builder builder(@NotNull String peerId) {
        return new Builder(peerId);
    }

    public static class Builder {
        private final String peerId;
        private boolean autoProtocol = true;
        private Duration handshakeTimeout = Duration.ofSeconds(10);
        private ServerOptions serverOptions;
        private ClientOptions clientOptions;

        private Builder(String peerId) {
            this.peerId = peerId;
        }

        @NotNull public Builder autoProtocol(boolean enabled) {
            this.autoProtocol = enabled;
            return this;
        }

        @NotNull public Builder handshakeTimeout(@NotNull Duration timeout) {
            this.handshakeTimeout = timeout;
            return this;
        }

        @NotNull public Builder serverOptions(@Nullable ServerOptions options) {
            this.serverOptions = options;
            return this;
        }

        @NotNull public Builder clientOptions(@Nullable ClientOptions options) {
            this.clientOptions = options;
            return this;
        }

        @NotNull public PeerOptions build() {
            return new PeerOptions(peerId, autoProtocol, handshakeTimeout,
                                 serverOptions, clientOptions);
        }
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        PeerOptions that = (PeerOptions) o;
        return autoProtocol == that.autoProtocol &&
               Objects.equals(peerId, that.peerId) &&
               Objects.equals(handshakeTimeout, that.handshakeTimeout);
    }

    @Override
    public int hashCode() {
        return Objects.hash(peerId, autoProtocol, handshakeTimeout);
    }
}

