package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;

import java.time.Instant;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/**
 * Information about a connected peer after handshake.
 */
public class PeerInfo {

    private final String peerId;
    private final Map<String, String> metadata;
    private final Map<String, String> capabilities;
    private final Instant handshakeCompletedAt;

    public PeerInfo(@NotNull String peerId, @NotNull Map<String, String> metadata,
                   @NotNull Map<String, String> capabilities, @NotNull Instant handshakeCompletedAt) {
        this.peerId = peerId;
        this.metadata = new HashMap<>(metadata);
        this.capabilities = new HashMap<>(capabilities);
        this.handshakeCompletedAt = handshakeCompletedAt;
    }

    @NotNull public String getPeerId() { return peerId; }
    @NotNull public Map<String, String> getMetadata() { return new HashMap<>(metadata); }
    @NotNull public Map<String, String> getCapabilities() { return new HashMap<>(capabilities); }
    @NotNull public Instant getHandshakeCompletedAt() { return handshakeCompletedAt; }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        PeerInfo peerInfo = (PeerInfo) o;
        return Objects.equals(peerId, peerInfo.peerId) &&
               Objects.equals(metadata, peerInfo.metadata) &&
               Objects.equals(capabilities, peerInfo.capabilities);
    }

    @Override
    public int hashCode() {
        return Objects.hash(peerId, metadata, capabilities);
    }

    @Override
    public String toString() {
        return "PeerInfo{peerId='" + peerId + "', metadata=" + metadata +
               ", handshakeCompletedAt=" + handshakeCompletedAt + '}';
    }
}

