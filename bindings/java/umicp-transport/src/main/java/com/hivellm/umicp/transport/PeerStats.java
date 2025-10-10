package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;

import java.util.HashMap;
import java.util.Map;

/**
 * Statistics for multiplexed peer.
 */
public class PeerStats {

    private final int totalPeers;
    private final int incomingConnections;
    private final int outgoingConnections;
    private final long totalMessagesSent;
    private final long totalMessagesReceived;
    private final Map<String, TransportStats> perPeerStats;

    public PeerStats(int totalPeers, int incomingConnections, int outgoingConnections,
                    long totalMessagesSent, long totalMessagesReceived,
                    Map<String, TransportStats> perPeerStats) {
        this.totalPeers = totalPeers;
        this.incomingConnections = incomingConnections;
        this.outgoingConnections = outgoingConnections;
        this.totalMessagesSent = totalMessagesSent;
        this.totalMessagesReceived = totalMessagesReceived;
        this.perPeerStats = perPeerStats != null ? new HashMap<>(perPeerStats) : new HashMap<>();
    }

    public int getTotalPeers() { return totalPeers; }
    public int getIncomingConnections() { return incomingConnections; }
    public int getOutgoingConnections() { return outgoingConnections; }
    public long getTotalMessagesSent() { return totalMessagesSent; }
    public long getTotalMessagesReceived() { return totalMessagesReceived; }
    @NotNull public Map<String, TransportStats> getPerPeerStats() { return new HashMap<>(perPeerStats); }

    @Override
    public String toString() {
        return "PeerStats{totalPeers=" + totalPeers +
               ", incoming=" + incomingConnections +
               ", outgoing=" + outgoingConnections +
               ", sent=" + totalMessagesSent +
               ", received=" + totalMessagesReceived + '}';
    }
}

