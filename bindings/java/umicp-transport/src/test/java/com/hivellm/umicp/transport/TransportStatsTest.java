package com.hivellm.umicp.transport;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.BeforeEach;

import static org.assertj.core.api.Assertions.*;
import static org.awaitility.Awaitility.*;

import java.time.Duration;
import java.util.concurrent.TimeUnit;

/**
 * Unit tests for {@link TransportStats}.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 */
class TransportStatsTest {

    private TransportStats stats;

    @BeforeEach
    void setUp() {
        stats = new TransportStats();
    }

    @Test
    @DisplayName("Should initialize with zero values")
    void testInitialValues() {
        assertThat(stats.getMessagesSent()).isZero();
        assertThat(stats.getMessagesReceived()).isZero();
        assertThat(stats.getBytesSent()).isZero();
        assertThat(stats.getBytesReceived()).isZero();
        assertThat(stats.getErrorCount()).isZero();
        assertThat(stats.getAverageLatency()).isZero();
        assertThat(stats.getThroughput()).isZero();
    }

    @Test
    @DisplayName("Should record sent messages")
    void testRecordMessageSent() {
        stats.recordMessageSent(100);
        stats.recordMessageSent(200);

        assertThat(stats.getMessagesSent()).isEqualTo(2);
        assertThat(stats.getBytesSent()).isEqualTo(300);
    }

    @Test
    @DisplayName("Should record received messages")
    void testRecordMessageReceived() {
        stats.recordMessageReceived(150);
        stats.recordMessageReceived(250);
        stats.recordMessageReceived(350);

        assertThat(stats.getMessagesReceived()).isEqualTo(3);
        assertThat(stats.getBytesReceived()).isEqualTo(750);
    }

    @Test
    @DisplayName("Should record errors")
    void testRecordError() {
        stats.recordError();
        stats.recordError();
        stats.recordError();

        assertThat(stats.getErrorCount()).isEqualTo(3);
    }

    @Test
    @DisplayName("Should calculate average latency")
    void testAverageLatency() {
        stats.recordLatency(10);
        stats.recordLatency(20);
        stats.recordLatency(30);

        assertThat(stats.getAverageLatency()).isEqualTo(20.0);
    }

    @Test
    @DisplayName("Should calculate throughput")
    void testThroughput() throws InterruptedException {
        stats.recordMessageSent(100);
        stats.recordMessageSent(200);
        stats.recordMessageReceived(150);

        // Wait at least 1 second
        Thread.sleep(1100);

        double throughput = stats.getThroughput();
        assertThat(throughput).isGreaterThan(0.0);
        assertThat(throughput).isLessThanOrEqualTo(3.0); // 3 messages
    }

    @Test
    @DisplayName("Should calculate error rate")
    void testErrorRate() {
        stats.recordMessageSent(100);
        stats.recordMessageSent(200);
        stats.recordMessageReceived(150);
        stats.recordError();

        double errorRate = stats.getErrorRate();
        assertThat(errorRate).isEqualTo(33.333, within(0.01)); // 1 error out of 3 messages
    }

    @Test
    @DisplayName("Should track uptime")
    void testUptime() throws InterruptedException {
        Thread.sleep(100);

        Duration uptime = stats.getUptime();
        assertThat(uptime.toMillis()).isGreaterThanOrEqualTo(100);
    }

    @Test
    @DisplayName("Should reset statistics")
    void testReset() {
        stats.recordMessageSent(100);
        stats.recordMessageReceived(200);
        stats.recordError();
        stats.recordLatency(50);

        stats.reset();

        assertThat(stats.getMessagesSent()).isZero();
        assertThat(stats.getMessagesReceived()).isZero();
        assertThat(stats.getBytesSent()).isZero();
        assertThat(stats.getBytesReceived()).isZero();
        assertThat(stats.getErrorCount()).isZero();
        assertThat(stats.getAverageLatency()).isZero();
    }

    @Test
    @DisplayName("Should have meaningful toString")
    void testToString() {
        stats.recordMessageSent(100);
        stats.recordMessageReceived(200);

        String str = stats.toString();
        assertThat(str).contains("TransportStats");
        assertThat(str).contains("messagesSent=1");
        assertThat(str).contains("messagesReceived=1");
    }
}

