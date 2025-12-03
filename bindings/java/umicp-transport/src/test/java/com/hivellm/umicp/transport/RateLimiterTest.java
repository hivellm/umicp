package com.hivellm.umicp.transport;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.util.concurrent.TimeUnit;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link RateLimiter}.
 *
 * @author HiveLLM Team
 * @version 0.1.1
 */
class RateLimiterTest {

    @Test
    @DisplayName("Should create rate limiter with valid rate")
    void testCreateValid() {
        RateLimiter limiter = RateLimiter.create(1000.0);
        assertThat(limiter.getRate()).isEqualTo(1000.0);
    }

    @Test
    @DisplayName("Should reject negative rate")
    void testCreateNegative() {
        assertThatThrownBy(() -> RateLimiter.create(-1.0))
            .isInstanceOf(IllegalArgumentException.class)
            .hasMessageContaining("positive");
    }

    @Test
    @DisplayName("Should reject zero rate")
    void testCreateZero() {
        assertThatThrownBy(() -> RateLimiter.create(0.0))
            .isInstanceOf(IllegalArgumentException.class)
            .hasMessageContaining("positive");
    }

    @Test
    @DisplayName("Should allow immediate acquire")
    void testImmediateAcquire() {
        RateLimiter limiter = RateLimiter.create(10.0);
        assertThat(limiter.tryAcquire()).isTrue();
    }

    @Test
    @DisplayName("Should enforce rate limit")
    void testRateLimit() {
        RateLimiter limiter = RateLimiter.create(10.0); // 10 per second

        // Exhaust burst permits
        for (int i = 0; i < 10; i++) {
            assertThat(limiter.tryAcquire()).isTrue();
        }

        // Next acquire should fail without waiting
        assertThat(limiter.tryAcquire()).isFalse();
    }

    @Test
    @DisplayName("Should replenish permits over time")
    void testReplenishment() throws InterruptedException {
        RateLimiter limiter = RateLimiter.create(10.0); // 10 per second

        // Exhaust permits
        for (int i = 0; i < 10; i++) {
            limiter.tryAcquire();
        }

        // Wait for replenishment
        Thread.sleep(200); // 200ms = 2 permits at 10/sec

        // Should be able to acquire again
        assertThat(limiter.tryAcquire()).isTrue();
    }

    @Test
    @DisplayName("Should respect burst limit")
    void testBurstLimit() {
        RateLimiter limiter = RateLimiter.create(10.0);

        // Max burst should be equal to rate (10 permits)
        assertThat(limiter.getMaxBurstPermits()).isEqualTo(10);
    }

    @Test
    @DisplayName("Should handle high rate limits")
    void testHighRate() {
        RateLimiter limiter = RateLimiter.create(10000.0);

        // Should allow many immediate acquires
        for (int i = 0; i < 100; i++) {
            assertThat(limiter.tryAcquire()).isTrue();
        }
    }

    @Test
    @DisplayName("Should handle low rate limits")
    void testLowRate() {
        RateLimiter limiter = RateLimiter.create(1.0); // 1 per second

        // First should succeed
        assertThat(limiter.tryAcquire()).isTrue();

        // Second should fail immediately
        assertThat(limiter.tryAcquire()).isFalse();
    }

    @Test
    @DisplayName("Should support acquire with timeout")
    void testAcquireWithTimeout() {
        RateLimiter limiter = RateLimiter.create(10.0);

        // Immediate acquire should succeed
        assertThat(limiter.tryAcquire(0, TimeUnit.NANOSECONDS)).isTrue();
    }

    @Test
    @DisplayName("Should reset state")
    void testReset() {
        RateLimiter limiter = RateLimiter.create(10.0);

        // Exhaust permits
        for (int i = 0; i < 10; i++) {
            limiter.tryAcquire();
        }

        // Reset
        limiter.reset();

        // Should fail as no permits accumulated yet
        assertThat(limiter.getStoredPermits()).isEqualTo(0);
    }

    @Test
    @DisplayName("Should track stored permits")
    void testStoredPermits() throws InterruptedException {
        RateLimiter limiter = RateLimiter.create(10.0);

        // Initial permits should be available
        assertThat(limiter.getStoredPermits()).isGreaterThan(0);

        // Acquire one
        limiter.tryAcquire();

        // Wait for replenishment
        Thread.sleep(100);

        // Should have some permits
        assertThat(limiter.getStoredPermits()).isGreaterThan(0);
    }

    @Test
    @DisplayName("Should provide meaningful toString")
    void testToString() {
        RateLimiter limiter = RateLimiter.create(100.0);
        String str = limiter.toString();

        assertThat(str)
            .contains("RateLimiter")
            .contains("100")
            .contains("/s");
    }

    @Test
    @DisplayName("Should reject negative permit count")
    void testNegativePermits() {
        RateLimiter limiter = RateLimiter.create(10.0);

        assertThatThrownBy(() -> limiter.tryAcquire(-1, 0, TimeUnit.NANOSECONDS))
            .isInstanceOf(IllegalArgumentException.class)
            .hasMessageContaining("positive");
    }

    @Test
    @DisplayName("Should reject zero permit count")
    void testZeroPermits() {
        RateLimiter limiter = RateLimiter.create(10.0);

        assertThatThrownBy(() -> limiter.tryAcquire(0, 0, TimeUnit.NANOSECONDS))
            .isInstanceOf(IllegalArgumentException.class)
            .hasMessageContaining("positive");
    }

    @Test
    @DisplayName("Should handle concurrent access")
    void testConcurrentAccess() throws InterruptedException {
        RateLimiter limiter = RateLimiter.create(1000.0);
        int threadCount = 10;
        int attemptsPerThread = 100;

        Thread[] threads = new Thread[threadCount];
        int[] successCounts = new int[threadCount];

        for (int i = 0; i < threadCount; i++) {
            final int index = i;
            threads[i] = new Thread(() -> {
                for (int j = 0; j < attemptsPerThread; j++) {
                    if (limiter.tryAcquire()) {
                        successCounts[index]++;
                    }
                }
            });
        }

        for (Thread thread : threads) {
            thread.start();
        }

        for (Thread thread : threads) {
            thread.join();
        }

        int totalSuccess = 0;
        for (int count : successCounts) {
            totalSuccess += count;
        }

        // Should not exceed burst limit significantly
        assertThat(totalSuccess).isLessThanOrEqualTo(1010); // Some tolerance
    }
}

