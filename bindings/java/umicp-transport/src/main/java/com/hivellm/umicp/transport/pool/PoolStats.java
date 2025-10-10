package com.hivellm.umicp.transport.pool;

/**
 * Connection pool statistics
 */
public class PoolStats {
    private long totalConnections;
    private long availableConnections;
    private long inUseConnections;
    private long totalAcquires;
    private long totalReleases;
    private long totalCreates;
    private long totalCloses;
    private long failedAcquires;

    public PoolStats() {
        this.totalConnections = 0;
        this.availableConnections = 0;
        this.inUseConnections = 0;
        this.totalAcquires = 0;
        this.totalReleases = 0;
        this.totalCreates = 0;
        this.totalCloses = 0;
        this.failedAcquires = 0;
    }

    // Getters
    public long getTotalConnections() { return totalConnections; }
    public long getAvailableConnections() { return availableConnections; }
    public long getInUseConnections() { return inUseConnections; }
    public long getTotalAcquires() { return totalAcquires; }
    public long getTotalReleases() { return totalReleases; }
    public long getTotalCreates() { return totalCreates; }
    public long getTotalCloses() { return totalCloses; }
    public long getFailedAcquires() { return failedAcquires; }

    // Setters (package-private for ConnectionPool)
    void setTotalConnections(long value) { this.totalConnections = value; }
    void setAvailableConnections(long value) { this.availableConnections = value; }
    void setInUseConnections(long value) { this.inUseConnections = value; }
    void setTotalAcquires(long value) { this.totalAcquires = value; }
    void setTotalReleases(long value) { this.totalReleases = value; }
    void setTotalCreates(long value) { this.totalCreates = value; }
    void setTotalCloses(long value) { this.totalCloses = value; }
    void setFailedAcquires(long value) { this.failedAcquires = value; }

    // Increment methods (package-private)
    void incrementTotalConnections() { this.totalConnections++; }
    void incrementAvailableConnections() { this.availableConnections++; }
    void incrementInUseConnections() { this.inUseConnections++; }
    void incrementTotalAcquires() { this.totalAcquires++; }
    void incrementTotalReleases() { this.totalReleases++; }
    void incrementTotalCreates() { this.totalCreates++; }
    void incrementTotalCloses() { this.totalCloses++; }
    void incrementFailedAcquires() { this.failedAcquires++; }

    // Decrement methods (package-private)
    void decrementTotalConnections() { if (totalConnections > 0) this.totalConnections--; }
    void decrementAvailableConnections() { if (availableConnections > 0) this.availableConnections--; }
    void decrementInUseConnections() { if (inUseConnections > 0) this.inUseConnections--; }

    @Override
    public String toString() {
        return "PoolStats{" +
                "totalConnections=" + totalConnections +
                ", availableConnections=" + availableConnections +
                ", inUseConnections=" + inUseConnections +
                ", totalAcquires=" + totalAcquires +
                ", totalReleases=" + totalReleases +
                ", totalCreates=" + totalCreates +
                ", totalCloses=" + totalCloses +
                ", failedAcquires=" + failedAcquires +
                '}';
    }
}

