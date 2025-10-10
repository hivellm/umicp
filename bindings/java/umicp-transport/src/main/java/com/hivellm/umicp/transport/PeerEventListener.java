package com.hivellm.umicp.transport;

import com.hivellm.umicp.core.Envelope;
import org.jetbrains.annotations.NotNull;

/**
 * Event listener interface for peer events.
 */
public interface PeerEventListener {

    default void onMessage(@NotNull Envelope envelope, @NotNull PeerConnection peer) {}

    default void onData(@NotNull Envelope envelope, @NotNull PeerConnection peer) {}

    default void onPeerConnect(@NotNull PeerConnection peer) {}

    default void onPeerDisconnect(@NotNull PeerConnection peer) {}

    default void onPeerReady(@NotNull PeerConnection peer, @NotNull PeerInfo info) {}

    default void onHandshakeTimeout(@NotNull PeerConnection peer) {}

    default void onError(@NotNull Exception error, @NotNull PeerConnection peer) {}

    default void onServerReady() {}

    default void onConnected(@NotNull String peerId, @NotNull String url) {}
}

