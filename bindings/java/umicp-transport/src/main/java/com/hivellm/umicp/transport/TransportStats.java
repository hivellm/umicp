package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;

import java.time.Duration;
import java.time.Instant;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Statistics for transport connections.
 *
 * <p>This class tracks various metrics for transport connections including
 * message counts, byte counts, latency, uptime, and error rates.
 *
 * <p>Example usage:
 * <pre>{@code
 * TransportStats stats = client.getStats();
 *
 * System.out.println("Messages sent: " + stats.getMessagesSent());
 * System.out.println("Bytes received: " + stats.getBytesReceived());
 * System.out.println("Average latency: " + stats.getAverageLatency() + "ms");
 * System.out.println("Uptime: " + stats.getUptime().toSeconds() + "s");
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 1.0.0
 * @since 1.0.0
 */
public class TransportStats {

    private final AtomicLong messagesSent = new AtomicLong(0);
    private final AtomicLong messagesReceived = new AtomicLong(0);
    private final AtomicLong bytesSent = new AtomicLong(0);
    private final AtomicLong bytesReceived = new AtomicLong(0);
    private final AtomicLong errorCount = new AtomicLong(0);
    private final AtomicLong activeConnections = new AtomicLong(0);
    private final AtomicLong totalConnections = new AtomicLong(0);
    private final Instant startTime;

    // Latency tracking
    private final AtomicLong totalLatencyMs = new AtomicLong(0);
    private final AtomicLong latencyMeasurements = new AtomicLong(0);

    /**
     * Creates a new TransportStats instance.
     */
    public TransportStats() {
        this.startTime = Instant.now();
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

    /**
     * Records an error.
     */
    public void recordError() {
        errorCount.incrementAndGet();
    }

    /**
     * Records a latency measurement.
     *
     * @param latencyMs latency in milliseconds
     */
    public void recordLatency(long latencyMs) {
        totalLatencyMs.addAndGet(latencyMs);
        latencyMeasurements.incrementAndGet();
    }

    // ---- Fine-grained counters used by the transport layer ----

    /** Increments the sent-message counter by one. */
    public void incrementMessagesSent() {
        messagesSent.incrementAndGet();
    }

    /** Increments the received-message counter by one. */
    public void incrementMessagesReceived() {
        messagesReceived.incrementAndGet();
    }

    /** Adds to the sent-bytes counter. */
    public void addBytesSent(long bytes) {
        bytesSent.addAndGet(bytes);
    }

    /** Adds to the received-bytes counter. */
    public void addBytesReceived(long bytes) {
        bytesReceived.addAndGet(bytes);
    }

    /** Registers a new connection (increments active and total). */
    public void connectionOpened() {
        activeConnections.incrementAndGet();
        totalConnections.incrementAndGet();
    }

    /** Registers a closed connection (decrements active). */
    public void connectionClosed() {
        activeConnections.updateAndGet(v -> v > 0 ? v - 1 : 0);
    }

    /** @return the number of currently active connections. */
    public long getActiveConnections() {
        return activeConnections.get();
    }

    /** @return the total number of connections seen. */
    public long getTotalConnections() {
        return totalConnections.get();
    }

    /**
     * Returns the number of messages sent.
     *
     * @return message count
     */
    public long getMessagesSent() {
        return messagesSent.get();
    }

    /**
     * Returns the number of messages received.
     *
     * @return message count
     */
    public long getMessagesReceived() {
        return messagesReceived.get();
    }

    /**
     * Returns the number of bytes sent.
     *
     * @return byte count
     */
    public long getBytesSent() {
        return bytesSent.get();
    }

    /**
     * Returns the number of bytes received.
     *
     * @return byte count
     */
    public long getBytesReceived() {
        return bytesReceived.get();
    }

    /**
     * Returns the number of errors encountered.
     *
     * @return error count
     */
    public long getErrorCount() {
        return errorCount.get();
    }

    /**
     * Returns the uptime duration.
     *
     * @return duration since stats were created
     */
    @NotNull
    public Duration getUptime() {
        return Duration.between(startTime, Instant.now());
    }

    /**
     * Returns the average latency in milliseconds.
     *
     * @return average latency, or 0.0 if no measurements
     */
    public double getAverageLatency() {
        long measurements = latencyMeasurements.get();
        if (measurements == 0) {
            return 0.0;
        }
        return (double) totalLatencyMs.get() / measurements;
    }

    /**
     * Returns the throughput in messages per second.
     *
     * @return messages per second, or 0.0 if no time elapsed
     */
    public double getThroughput() {
        long seconds = getUptime().getSeconds();
        if (seconds == 0) {
            return 0.0;
        }
        return (double) (messagesSent.get() + messagesReceived.get()) / seconds;
    }

    /**
     * Returns the error rate as a percentage.
     *
     * @return error rate (0.0 to 100.0)
     */
    public double getErrorRate() {
        long total = messagesSent.get() + messagesReceived.get();
        if (total == 0) {
            return 0.0;
        }
        return (double) errorCount.get() / total * 100.0;
    }

    /**
     * Resets all statistics.
     */
    public void reset() {
        messagesSent.set(0);
        messagesReceived.set(0);
        bytesSent.set(0);
        bytesReceived.set(0);
        errorCount.set(0);
        totalLatencyMs.set(0);
        latencyMeasurements.set(0);
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        TransportStats that = (TransportStats) o;
        return messagesSent.get() == that.messagesSent.get() &&
               messagesReceived.get() == that.messagesReceived.get() &&
               bytesSent.get() == that.bytesSent.get() &&
               bytesReceived.get() == that.bytesReceived.get() &&
               errorCount.get() == that.errorCount.get();
    }

    @Override
    public int hashCode() {
        return Objects.hash(
            messagesSent.get(),
            messagesReceived.get(),
            bytesSent.get(),
            bytesReceived.get(),
            errorCount.get()
        );
    }

    @Override
    public String toString() {
        return "TransportStats{" +
               "messagesSent=" + messagesSent.get() +
               ", messagesReceived=" + messagesReceived.get() +
               ", bytesSent=" + bytesSent.get() +
               ", bytesReceived=" + bytesReceived.get() +
               ", errorCount=" + errorCount.get() +
               ", uptime=" + getUptime().toSeconds() + "s" +
               ", avgLatency=" + String.format("%.2f", getAverageLatency()) + "ms" +
               ", throughput=" + String.format("%.2f", getThroughput()) + " msg/s" +
               '}';
    }
}

