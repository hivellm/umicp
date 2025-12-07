package com.hivellm.umicp.transport;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link ConnectionManager}.
 *
 * @author HiveLLM Team
 * @version 0.1.1
 */
class ConnectionManagerTest {

    @Test
    @DisplayName("Should create with default limit")
    void testDefaultLimit() {
        ConnectionManager manager = new ConnectionManager();
        assertThat(manager.getMaxConnections()).isEqualTo(ConnectionManager.DEFAULT_MAX_CONNECTIONS);
    }

    @Test
    @DisplayName("Should create with custom limit")
    void testCustomLimit() {
        ConnectionManager manager = new ConnectionManager(100);
        assertThat(manager.getMaxConnections()).isEqualTo(100);
    }

    @Test
    @DisplayName("Should reject invalid limits")
    void testInvalidLimits() {
        assertThatThrownBy(() -> new ConnectionManager(0))
            .isInstanceOf(IllegalArgumentException.class);

        assertThatThrownBy(() -> new ConnectionManager(-1))
            .isInstanceOf(IllegalArgumentException.class);

        assertThatThrownBy(() -> new ConnectionManager(ConnectionManager.MAX_CONNECTIONS_LIMIT + 1))
            .isInstanceOf(IllegalArgumentException.class);
    }

    @Test
    @DisplayName("Should accept connection")
    void testAcceptConnection() {
        ConnectionManager manager = new ConnectionManager(10);
        boolean accepted = manager.acceptConnection("conn-1");

        assertThat(accepted).isTrue();
        assertThat(manager.getActiveConnections()).isEqualTo(1);
        assertThat(manager.getTotalAccepted()).isEqualTo(1);
    }

    @Test
    @DisplayName("Should reject null connection ID")
    void testNullConnectionId() {
        ConnectionManager manager = new ConnectionManager(10);

        assertThatThrownBy(() -> manager.acceptConnection(null))
            .isInstanceOf(IllegalArgumentException.class);
    }

    @Test
    @DisplayName("Should reject empty connection ID")
    void testEmptyConnectionId() {
        ConnectionManager manager = new ConnectionManager(10);

        assertThatThrownBy(() -> manager.acceptConnection(""))
            .isInstanceOf(IllegalArgumentException.class);
    }

    @Test
    @DisplayName("Should enforce connection limit")
    void testConnectionLimit() {
        ConnectionManager manager = new ConnectionManager(2);

        assertThat(manager.acceptConnection("conn-1")).isTrue();
        assertThat(manager.acceptConnection("conn-2")).isTrue();
        assertThat(manager.acceptConnection("conn-3")).isFalse();

        assertThat(manager.getActiveConnections()).isEqualTo(2);
        assertThat(manager.getTotalRejected()).isEqualTo(1);
    }

    @Test
    @DisplayName("Should release connection")
    void testReleaseConnection() {
        ConnectionManager manager = new ConnectionManager(10);

        manager.acceptConnection("conn-1");
        assertThat(manager.getActiveConnections()).isEqualTo(1);

        boolean released = manager.releaseConnection("conn-1");
        assertThat(released).isTrue();
        assertThat(manager.getActiveConnections()).isEqualTo(0);
    }

    @Test
    @DisplayName("Should handle release of non-existent connection")
    void testReleaseNonExistent() {
        ConnectionManager manager = new ConnectionManager(10);

        boolean released = manager.releaseConnection("non-existent");
        assertThat(released).isFalse();
    }

    @Test
    @DisplayName("Should allow connection reuse after release")
    void testConnectionReuse() {
        ConnectionManager manager = new ConnectionManager(2);

        manager.acceptConnection("conn-1");
        manager.acceptConnection("conn-2");
        assertThat(manager.acceptConnection("conn-3")).isFalse();

        manager.releaseConnection("conn-1");
        assertThat(manager.acceptConnection("conn-3")).isTrue();
    }

    @Test
    @DisplayName("Should check if can accept connection")
    void testCanAcceptConnection() {
        ConnectionManager manager = new ConnectionManager(2);

        assertThat(manager.canAcceptConnection()).isTrue();

        manager.acceptConnection("conn-1");
        assertThat(manager.canAcceptConnection()).isTrue();

        manager.acceptConnection("conn-2");
        assertThat(manager.canAcceptConnection()).isFalse();
    }

    @Test
    @DisplayName("Should calculate usage ratio")
    void testUsageRatio() {
        ConnectionManager manager = new ConnectionManager(10);

        assertThat(manager.getUsageRatio()).isEqualTo(0.0);

        for (int i = 0; i < 5; i++) {
            manager.acceptConnection("conn-" + i);
        }

        assertThat(manager.getUsageRatio()).isEqualTo(0.5);
    }

    @Test
    @DisplayName("Should detect nearing limit")
    void testNearingLimit() {
        ConnectionManager manager = new ConnectionManager(10);

        assertThat(manager.isNearingLimit()).isFalse();

        for (int i = 0; i < 9; i++) {
            manager.acceptConnection("conn-" + i);
        }

        assertThat(manager.isNearingLimit()).isTrue();
    }

    @Test
    @DisplayName("Should track connection duration")
    void testConnectionDuration() throws InterruptedException {
        ConnectionManager manager = new ConnectionManager(10);

        manager.acceptConnection("conn-1");
        Thread.sleep(100);

        long duration = manager.getConnectionDuration("conn-1");
        assertThat(duration).isGreaterThanOrEqualTo(100);
    }

    @Test
    @DisplayName("Should return -1 for non-existent connection duration")
    void testNonExistentConnectionDuration() {
        ConnectionManager manager = new ConnectionManager(10);

        long duration = manager.getConnectionDuration("non-existent");
        assertThat(duration).isEqualTo(-1);
    }

    @Test
    @DisplayName("Should check if connection is registered")
    void testIsConnectionRegistered() {
        ConnectionManager manager = new ConnectionManager(10);

        assertThat(manager.isConnectionRegistered("conn-1")).isFalse();

        manager.acceptConnection("conn-1");
        assertThat(manager.isConnectionRegistered("conn-1")).isTrue();

        manager.releaseConnection("conn-1");
        assertThat(manager.isConnectionRegistered("conn-1")).isFalse();
    }

    @Test
    @DisplayName("Should reset statistics")
    void testResetStatistics() {
        ConnectionManager manager = new ConnectionManager(10);

        manager.acceptConnection("conn-1");
        manager.acceptConnection("conn-2");

        assertThat(manager.getTotalAccepted()).isEqualTo(2);

        manager.resetStatistics();

        assertThat(manager.getTotalAccepted()).isEqualTo(0);
        assertThat(manager.getActiveConnections()).isEqualTo(2); // Active connections not affected
    }

    @Test
    @DisplayName("Should clear all connections")
    void testClearAllConnections() {
        ConnectionManager manager = new ConnectionManager(10);

        manager.acceptConnection("conn-1");
        manager.acceptConnection("conn-2");

        assertThat(manager.getActiveConnections()).isEqualTo(2);

        manager.clearAllConnections();

        assertThat(manager.getActiveConnections()).isEqualTo(0);
    }

    @Test
    @DisplayName("Should handle duplicate connection IDs gracefully")
    void testDuplicateConnectionIds() {
        ConnectionManager manager = new ConnectionManager(10);

        assertThat(manager.acceptConnection("conn-1")).isTrue();
        assertThat(manager.acceptConnection("conn-1")).isTrue(); // Should be idempotent

        assertThat(manager.getActiveConnections()).isEqualTo(1);
    }

    @Test
    @DisplayName("Should provide meaningful toString")
    void testToString() {
        ConnectionManager manager = new ConnectionManager(100);
        manager.acceptConnection("conn-1");

        String str = manager.toString();

        assertThat(str)
            .contains("ConnectionManager")
            .contains("1/100")
            .contains("accepted=");
    }

    @Test
    @DisplayName("Should handle concurrent access")
    void testConcurrentAccess() throws InterruptedException {
        ConnectionManager manager = new ConnectionManager(100);
        int threadCount = 10;
        int connectionsPerThread = 10;

        Thread[] threads = new Thread[threadCount];

        for (int i = 0; i < threadCount; i++) {
            final int threadIndex = i;
            threads[i] = new Thread(() -> {
                for (int j = 0; j < connectionsPerThread; j++) {
                    String connId = String.format("conn-%d-%d", threadIndex, j);
                    manager.acceptConnection(connId);
                }
            });
        }

        for (Thread thread : threads) {
            thread.start();
        }

        for (Thread thread : threads) {
            thread.join();
        }

        assertThat(manager.getActiveConnections()).isEqualTo(100);
        assertThat(manager.getTotalAccepted()).isEqualTo(100);
    }
}

