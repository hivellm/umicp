package com.hivellm.umicp.transport;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Manages resource lifecycle and cleanup to prevent memory leaks.
 *
 * <p>Ensures all resources are properly cleaned up even in error conditions.
 * Tracks resources and provides automatic cleanup on close.
 *
 * <p>Example usage:
 * <pre>{@code
 * try (ResourceManager rm = new ResourceManager()) {
 *     WebSocketClient client = new WebSocketClient();
 *     rm.register(client);
 *
 *     // Use client...
 *
 *     // Automatic cleanup on close
 * }
 * }</pre>
 *
 * @author HiveLLM Team
 * @version 0.1.1
 * @since 0.1.1
 */
public class ResourceManager implements AutoCloseable {

    private static final Logger logger = LoggerFactory.getLogger(ResourceManager.class);

    private final List<AutoCloseable> resources;
    private final AtomicBoolean closed;
    private final String name;

    /**
     * Creates a resource manager with default name.
     */
    public ResourceManager() {
        this("ResourceManager");
    }

    /**
     * Creates a resource manager with specified name.
     *
     * @param name descriptive name for logging
     */
    public ResourceManager(@NotNull String name) {
        this.name = name;
        this.resources = new ArrayList<>();
        this.closed = new AtomicBoolean(false);
    }

    /**
     * Registers a resource for automatic cleanup.
     *
     * @param resource the resource to manage
     * @param <T> resource type
     * @return the registered resource
     */
    @NotNull
    public <T extends AutoCloseable> T register(@NotNull T resource) {
        if (closed.get()) {
            throw new IllegalStateException("ResourceManager is closed");
        }

        synchronized (resources) {
            resources.add(resource);
        }

        logger.debug("[{}] Registered resource: {}", name, resource.getClass().getSimpleName());
        return resource;
    }

    /**
     * Unregisters a resource (useful if manually closed early).
     *
     * @param resource the resource to unregister
     * @return true if resource was registered
     */
    public boolean unregister(@NotNull AutoCloseable resource) {
        synchronized (resources) {
            boolean removed = resources.remove(resource);
            if (removed) {
                logger.debug("[{}] Unregistered resource: {}",
                    name, resource.getClass().getSimpleName());
            }
            return removed;
        }
    }

    /**
     * Safely closes a single resource.
     *
     * <p>Catches and logs exceptions without propagating them.
     *
     * @param resource the resource to close
     * @return true if closed successfully, false if error occurred
     */
    public static boolean safeClose(@Nullable AutoCloseable resource) {
        if (resource == null) {
            return true;
        }

        try {
            resource.close();
            return true;
        } catch (Exception e) {
            logger.error("Error closing resource: {}",
                resource.getClass().getSimpleName(), e);
            return false;
        }
    }

    /**
     * Safely closes a single resource with logging.
     *
     * @param resource the resource to close
     * @param description description for logging
     * @return true if closed successfully
     */
    public static boolean safeClose(@Nullable AutoCloseable resource, @NotNull String description) {
        if (resource == null) {
            return true;
        }

        try {
            logger.debug("Closing resource: {}", description);
            resource.close();
            return true;
        } catch (Exception e) {
            logger.error("Error closing resource {}: {}",
                description, e.getMessage(), e);
            return false;
        }
    }

    /**
     * Gets the number of registered resources.
     *
     * @return resource count
     */
    public int getResourceCount() {
        synchronized (resources) {
            return resources.size();
        }
    }

    /**
     * Checks if the resource manager is closed.
     *
     * @return true if closed
     */
    public boolean isClosed() {
        return closed.get();
    }

    /**
     * Closes all registered resources.
     *
     * <p>Resources are closed in reverse order of registration (LIFO).
     * All resources are attempted to be closed even if some fail.
     */
    @Override
    public void close() {
        if (!closed.compareAndSet(false, true)) {
            logger.warn("[{}] Already closed", name);
            return;
        }

        List<AutoCloseable> toClose;
        synchronized (resources) {
            toClose = new ArrayList<>(resources);
            resources.clear();
        }

        logger.info("[{}] Closing {} resources", name, toClose.size());

        int successCount = 0;
        int failureCount = 0;
        List<Exception> exceptions = new ArrayList<>();

        // Close in reverse order (LIFO)
        for (int i = toClose.size() - 1; i >= 0; i--) {
            AutoCloseable resource = toClose.get(i);
            try {
                resource.close();
                successCount++;
                logger.debug("[{}] Closed resource {}/{}: {}",
                    name, toClose.size() - i, toClose.size(),
                    resource.getClass().getSimpleName());
            } catch (Exception e) {
                failureCount++;
                exceptions.add(e);
                logger.error("[{}] Failed to close resource {}/{}: {}",
                    name, toClose.size() - i, toClose.size(),
                    resource.getClass().getSimpleName(), e);
            }
        }

        if (failureCount > 0) {
            logger.warn("[{}] Resource cleanup completed: {} succeeded, {} failed",
                name, successCount, failureCount);
        } else {
            logger.info("[{}] All resources closed successfully ({})",
                name, successCount);
        }
    }

    /**
     * Creates a scoped resource manager.
     *
     * <p>Helper for try-with-resources pattern.
     *
     * @param name descriptive name
     * @return new resource manager
     */
    @NotNull
    public static ResourceManager createScoped(@NotNull String name) {
        return new ResourceManager(name);
    }

    @Override
    public String toString() {
        return String.format("%s{name='%s', resources=%d, closed=%s}",
            getClass().getSimpleName(), name, getResourceCount(), closed.get());
    }
}

