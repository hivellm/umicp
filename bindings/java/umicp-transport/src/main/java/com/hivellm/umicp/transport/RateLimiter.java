package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.time.Duration;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.locks.ReentrantLock;

/**
 * Token bucket rate limiter implementation.
 *
 * <p>Provides rate limiting to prevent message flooding and DoS attacks.
 * Uses the token bucket algorithm for smooth rate limiting.
 *
 * <p>Example usage:
 * <pre>{@code
 * RateLimiter limiter = RateLimiter.create(1000.0); // 1000 messages/sec
 *
 * if (limiter.tryAcquire()) {
 *     // Process message
 * } else {
 *     // Rate limit exceeded
 * }
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 0.1.1
 * @since 0.1.1
 */
public class RateLimiter {

    private static final Logger logger = LoggerFactory.getLogger(RateLimiter.class);

    private final double permitsPerSecond;
    private final long maxBurstPermits;
    private final AtomicLong storedPermits;
    private final AtomicLong nextFreeTicketNanos;
    private final ReentrantLock lock;
    private final long stableIntervalNanos;

    /**
     * Private constructor. Use {@link #create(double)} to create instances.
     */
    private RateLimiter(double permitsPerSecond) {
        if (permitsPerSecond <= 0.0) {
            throw new IllegalArgumentException("Rate must be positive");
        }

        this.permitsPerSecond = permitsPerSecond;
        this.maxBurstPermits = (long) permitsPerSecond; // Allow 1 second burst
        this.storedPermits = new AtomicLong(0);
        this.nextFreeTicketNanos = new AtomicLong(System.nanoTime());
        this.lock = new ReentrantLock();
        this.stableIntervalNanos = (long) (TimeUnit.SECONDS.toNanos(1) / permitsPerSecond);
    }

    /**
     * Creates a rate limiter with the specified rate.
     *
     * @param permitsPerSecond the rate (permits per second)
     * @return a new RateLimiter instance
     */
    @NotNull
    public static RateLimiter create(double permitsPerSecond) {
        return new RateLimiter(permitsPerSecond);
    }

    /**
     * Acquires a single permit if available immediately.
     *
     * @return true if permit was acquired, false if rate limit exceeded
     */
    public boolean tryAcquire() {
        return tryAcquire(1, 0, TimeUnit.NANOSECONDS);
    }

    /**
     * Acquires a permit, waiting up to the specified timeout.
     *
     * @param timeout maximum time to wait
     * @param unit time unit of the timeout
     * @return true if permit was acquired, false if timeout elapsed
     */
    public boolean tryAcquire(long timeout, @NotNull TimeUnit unit) {
        return tryAcquire(1, timeout, unit);
    }

    /**
     * Acquires the specified number of permits.
     *
     * @param permits number of permits to acquire
     * @param timeout maximum time to wait
     * @param unit time unit of the timeout
     * @return true if permits were acquired, false if timeout elapsed
     */
    public boolean tryAcquire(int permits, long timeout, @NotNull TimeUnit unit) {
        if (permits <= 0) {
            throw new IllegalArgumentException("Requested permits must be positive");
        }

        long timeoutNanos = unit.toNanos(timeout);
        long startNanos = System.nanoTime();

        lock.lock();
        try {
            long nowNanos = System.nanoTime();

            // Replenish tokens based on time elapsed
            replenish(nowNanos);

            // Check if we have enough permits
            if (storedPermits.get() >= permits) {
                storedPermits.addAndGet(-permits);
                return true;
            }

            // Not enough permits immediately available
            if (timeoutNanos <= 0) {
                return false;
            }

            // Calculate wait time
            long requiredPermits = permits - storedPermits.get();
            long waitNanos = requiredPermits * stableIntervalNanos;

            if (waitNanos > timeoutNanos) {
                return false;
            }

            // Wait and retry
            try {
                TimeUnit.NANOSECONDS.sleep(waitNanos);
                replenish(System.nanoTime());

                if (storedPermits.get() >= permits) {
                    storedPermits.addAndGet(-permits);
                    return true;
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                return false;
            }

            return false;

        } finally {
            lock.unlock();
        }
    }

    /**
     * Replenishes tokens based on time elapsed.
     */
    private void replenish(long nowNanos) {
        long lastTicketNanos = nextFreeTicketNanos.get();

        if (nowNanos > lastTicketNanos) {
            long elapsedNanos = nowNanos - lastTicketNanos;
            long newPermits = elapsedNanos / stableIntervalNanos;

            if (newPermits > 0) {
                long current = storedPermits.get();
                long newStored = Math.min(maxBurstPermits, current + newPermits);
                storedPermits.set(newStored);
                nextFreeTicketNanos.set(lastTicketNanos + (newPermits * stableIntervalNanos));
            }
        }
    }

    /**
     * Gets the current rate.
     *
     * @return permits per second
     */
    public double getRate() {
        return permitsPerSecond;
    }

    /**
     * Gets the maximum burst size.
     *
     * @return maximum number of permits that can be accumulated
     */
    public long getMaxBurstPermits() {
        return maxBurstPermits;
    }

    /**
     * Gets the current number of stored permits.
     *
     * @return number of available permits
     */
    public long getStoredPermits() {
        lock.lock();
        try {
            replenish(System.nanoTime());
            return storedPermits.get();
        } finally {
            lock.unlock();
        }
    }

    /**
     * Resets the rate limiter state.
     */
    public void reset() {
        lock.lock();
        try {
            storedPermits.set(0);
            nextFreeTicketNanos.set(System.nanoTime());
        } finally {
            lock.unlock();
        }
    }

    @Override
    public String toString() {
        return String.format("RateLimiter{rate=%.2f/s, stored=%d/%d}",
            permitsPerSecond, getStoredPermits(), maxBurstPermits);
    }
}

