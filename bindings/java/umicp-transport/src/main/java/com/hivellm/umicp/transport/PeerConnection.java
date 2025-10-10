package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

import java.time.Instant;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/**
 * Represents a peer connection in a multiplexed peer network.
 */
public class PeerConnection {

    private final String id;
    private final PeerType type;
    private final String url;
    private final Map<String, String> metadata;
    private final Instant connectedAt;
    private volatile boolean handshakeComplete;
    private volatile PeerInfo peerInfo;

    public enum PeerType {
        INCOMING,
        OUTGOING
    }

    public PeerConnection(@NotNull String id, @NotNull PeerType type,
                         @Nullable String url, @Nullable Map<String, String> metadata) {
        this.id = id;
        this.type = type;
        this.url = url;
        this.metadata = metadata != null ? new HashMap<>(metadata) : new HashMap<>();
        this.connectedAt = Instant.now();
        this.handshakeComplete = false;
    }

    @NotNull public String getId() { return id; }
    @NotNull public PeerType getType() { return type; }
    @Nullable public String getUrl() { return url; }
    @NotNull public Map<String, String> getMetadata() { return new HashMap<>(metadata); }
    @NotNull public Instant getConnectedAt() { return connectedAt; }
    public boolean isHandshakeComplete() { return handshakeComplete; }
    @Nullable public PeerInfo getPeerInfo() { return peerInfo; }

    public void setHandshakeComplete(boolean complete) { this.handshakeComplete = complete; }
    public void setPeerInfo(@Nullable PeerInfo info) { this.peerInfo = info; }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        PeerConnection that = (PeerConnection) o;
        return Objects.equals(id, that.id);
    }

    @Override
    public int hashCode() {
        return Objects.hash(id);
    }

    @Override
    public String toString() {
        return "PeerConnection{id='" + id + "', type=" + type +
               ", url='" + url + "', handshakeComplete=" + handshakeComplete + '}';
    }
}

