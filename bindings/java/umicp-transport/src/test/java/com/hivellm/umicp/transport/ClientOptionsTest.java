package com.hivellm.umicp.transport;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.DisplayName;

import java.time.Duration;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link ClientOptions}.
 *
 * @author HiveLLM Team
 * @version 1.0.0
 */
class ClientOptionsTest {

    @Test
    @DisplayName("Should create default options")
    void testDefaults() {
        ClientOptions options = ClientOptions.defaults();

        assertThat(options.getConnectTimeout()).isEqualTo(Duration.ofSeconds(10));
        assertThat(options.getHeartbeatInterval()).isEqualTo(Duration.ofSeconds(30));
        assertThat(options.getMaxReconnectAttempts()).isEqualTo(5);
        assertThat(options.getReconnectDelay()).isEqualTo(Duration.ofSeconds(1));
        assertThat(options.getMaxReconnectDelay()).isEqualTo(Duration.ofSeconds(30));
        assertThat(options.getReconnectBackoffMultiplier()).isEqualTo(2.0);
        assertThat(options.isAutoReconnect()).isTrue();
        assertThat(options.getMaxMessageQueueSize()).isEqualTo(1000);
        assertThat(options.isCompression()).isTrue();
    }

    @Test
    @DisplayName("Should build with custom values")
    void testCustomValues() {
        ClientOptions options = ClientOptions.builder()
            .connectTimeout(Duration.ofSeconds(5))
            .heartbeatInterval(Duration.ofSeconds(15))
            .maxReconnectAttempts(3)
            .reconnectDelay(Duration.ofMillis(500))
            .maxReconnectDelay(Duration.ofSeconds(60))
            .reconnectBackoffMultiplier(1.5)
            .autoReconnect(false)
            .maxMessageQueueSize(500)
            .compression(false)
            .build();

        assertThat(options.getConnectTimeout()).isEqualTo(Duration.ofSeconds(5));
        assertThat(options.getHeartbeatInterval()).isEqualTo(Duration.ofSeconds(15));
        assertThat(options.getMaxReconnectAttempts()).isEqualTo(3);
        assertThat(options.getReconnectDelay()).isEqualTo(Duration.ofMillis(500));
        assertThat(options.getMaxReconnectDelay()).isEqualTo(Duration.ofSeconds(60));
        assertThat(options.getReconnectBackoffMultiplier()).isEqualTo(1.5);
        assertThat(options.isAutoReconnect()).isFalse();
        assertThat(options.getMaxMessageQueueSize()).isEqualTo(500);
        assertThat(options.isCompression()).isFalse();
    }

    @Test
    @DisplayName("Should support fluent builder API")
    void testFluentAPI() {
        ClientOptions options = ClientOptions.builder()
            .connectTimeout(Duration.ofSeconds(5))
            .autoReconnect(true)
            .maxReconnectAttempts(10)
            .build();

        assertThat(options.getConnectTimeout()).isEqualTo(Duration.ofSeconds(5));
        assertThat(options.isAutoReconnect()).isTrue();
        assertThat(options.getMaxReconnectAttempts()).isEqualTo(10);
    }

    @Test
    @DisplayName("Should support equals and hashCode")
    void testEqualsAndHashCode() {
        ClientOptions options1 = ClientOptions.builder()
            .connectTimeout(Duration.ofSeconds(10))
            .autoReconnect(true)
            .build();

        ClientOptions options2 = ClientOptions.builder()
            .connectTimeout(Duration.ofSeconds(10))
            .autoReconnect(true)
            .build();

        ClientOptions options3 = ClientOptions.builder()
            .connectTimeout(Duration.ofSeconds(5))
            .autoReconnect(false)
            .build();

        assertThat(options1).isEqualTo(options2);
        assertThat(options1.hashCode()).isEqualTo(options2.hashCode());
        assertThat(options1).isNotEqualTo(options3);
    }

    @Test
    @DisplayName("Should have meaningful toString")
    void testToString() {
        ClientOptions options = ClientOptions.builder()
            .autoReconnect(true)
            .maxReconnectAttempts(5)
            .build();

        String str = options.toString();
        assertThat(str).contains("ClientOptions");
        assertThat(str).contains("autoReconnect=true");
        assertThat(str).contains("maxReconnectAttempts=5");
    }
}

