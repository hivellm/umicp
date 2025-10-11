package com.hivellm.umicp.events;

import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.function.Consumer;

/**
 * Event emitter for publish-subscribe pattern.
 * Similar to Node.js EventEmitter, allowing components to emit and listen to events.
 * Thread-safe implementation using concurrent collections.
 *
 * @author HiveLLM Team
 * @version 0.1.3
 */
public class EventEmitter {
    private static final Logger logger = LoggerFactory.getLogger(EventEmitter.class);

    /**
     * Map of event types to their listeners
     */
    private final Map<String, List<EventListener>> listeners;

    /**
     * Map of event types to their once-only listeners
     */
    private final Map<String, List<EventListener>> onceListeners;

    /**
     * Maximum number of listeners per event (default: unlimited)
     */
    private int maxListeners = 0;

    /**
     * Create new event emitter
     */
    public EventEmitter() {
        this.listeners = new ConcurrentHashMap<>();
        this.onceListeners = new ConcurrentHashMap<>();
    }

    /**
     * Add event listener
     *
     * @param event    Event type
     * @param listener Event listener
     * @return this emitter for chaining
     */
    @NotNull
    public EventEmitter on(@NotNull String event, @NotNull EventListener listener) {
        List<EventListener> eventListeners = listeners.computeIfAbsent(
                event,
                k -> new CopyOnWriteArrayList<>()
        );

        eventListeners.add(listener);
        checkMaxListeners(event, eventListeners.size());

        return this;
    }

    /**
     * Add event listener with typed data
     *
     * @param event    Event type
     * @param dataClass Expected data class
     * @param listener Typed event listener
     * @param <T>      Data type
     * @return this emitter for chaining
     */
    @NotNull
    public <T> EventEmitter on(@NotNull String event,
                                @NotNull Class<T> dataClass,
                                @NotNull TypedEventListener<T> listener) {
        return on(event, data -> {
            if (data != null && dataClass.isInstance(data)) {
                listener.handle(dataClass.cast(data));
            } else {
                logger.warn("Event {} data type mismatch. Expected {} but got {}",
                        event, dataClass.getSimpleName(),
                        data != null ? data.getClass().getSimpleName() : "null");
            }
        });
    }

    /**
     * Add one-time event listener (automatically removed after first invocation)
     *
     * @param event    Event type
     * @param listener Event listener
     * @return this emitter for chaining
     */
    @NotNull
    public EventEmitter once(@NotNull String event, @NotNull EventListener listener) {
        List<EventListener> eventListeners = onceListeners.computeIfAbsent(
                event,
                k -> new CopyOnWriteArrayList<>()
        );

        eventListeners.add(listener);

        return this;
    }

    /**
     * Add one-time event listener with typed data
     *
     * @param event     Event type
     * @param dataClass Expected data class
     * @param listener  Typed event listener
     * @param <T>       Data type
     * @return this emitter for chaining
     */
    @NotNull
    public <T> EventEmitter once(@NotNull String event,
                                  @NotNull Class<T> dataClass,
                                  @NotNull TypedEventListener<T> listener) {
        return once(event, data -> {
            if (data != null && dataClass.isInstance(data)) {
                listener.handle(dataClass.cast(data));
            }
        });
    }

    /**
     * Remove event listener
     *
     * @param event    Event type
     * @param listener Event listener to remove
     * @return this emitter for chaining
     */
    @NotNull
    public EventEmitter off(@NotNull String event, @NotNull EventListener listener) {
        List<EventListener> eventListeners = listeners.get(event);
        if (eventListeners != null) {
            eventListeners.remove(listener);
            if (eventListeners.isEmpty()) {
                listeners.remove(event);
            }
        }

        List<EventListener> eventOnceListeners = onceListeners.get(event);
        if (eventOnceListeners != null) {
            eventOnceListeners.remove(listener);
            if (eventOnceListeners.isEmpty()) {
                onceListeners.remove(event);
            }
        }

        return this;
    }

    /**
     * Remove all listeners for an event
     *
     * @param event Event type
     * @return this emitter for chaining
     */
    @NotNull
    public EventEmitter removeAllListeners(@NotNull String event) {
        listeners.remove(event);
        onceListeners.remove(event);
        return this;
    }

    /**
     * Remove all listeners for all events
     *
     * @return this emitter for chaining
     */
    @NotNull
    public EventEmitter removeAllListeners() {
        listeners.clear();
        onceListeners.clear();
        return this;
    }

    /**
     * Emit event with data
     *
     * @param event Event type
     * @param data  Event data (nullable)
     * @return true if the event had listeners, false otherwise
     */
    public boolean emit(@NotNull String event, @Nullable Object data) {
        boolean hadListeners = false;

        // Call regular listeners
        List<EventListener> eventListeners = listeners.get(event);
        if (eventListeners != null && !eventListeners.isEmpty()) {
            hadListeners = true;
            for (EventListener listener : eventListeners) {
                try {
                    listener.handle(data);
                } catch (Exception e) {
                    logger.error("Error executing listener for event: {}", event, e);
                    // Continue executing other listeners
                }
            }
        }

        // Call once listeners and remove them
        List<EventListener> eventOnceListeners = onceListeners.remove(event);
        if (eventOnceListeners != null && !eventOnceListeners.isEmpty()) {
            hadListeners = true;
            for (EventListener listener : eventOnceListeners) {
                try {
                    listener.handle(data);
                } catch (Exception e) {
                    logger.error("Error executing once-listener for event: {}", event, e);
                }
            }
        }

        return hadListeners;
    }

    /**
     * Emit event without data
     *
     * @param event Event type
     * @return true if the event had listeners, false otherwise
     */
    public boolean emit(@NotNull String event) {
        return emit(event, null);
    }

    /**
     * Get list of event types that have listeners
     *
     * @return array of event types
     */
    @NotNull
    public String[] eventNames() {
        Set<String> names = new HashSet<>();
        names.addAll(listeners.keySet());
        names.addAll(onceListeners.keySet());
        return names.toArray(new String[0]);
    }

    /**
     * Get listener count for an event
     *
     * @param event Event type
     * @return number of listeners
     */
    public int listenerCount(@NotNull String event) {
        int count = 0;

        List<EventListener> eventListeners = listeners.get(event);
        if (eventListeners != null) {
            count += eventListeners.size();
        }

        List<EventListener> eventOnceListeners = onceListeners.get(event);
        if (eventOnceListeners != null) {
            count += eventOnceListeners.size();
        }

        return count;
    }

    /**
     * Get all listeners for an event
     *
     * @param event Event type
     * @return array of listeners
     */
    @NotNull
    public EventListener[] listeners(@NotNull String event) {
        List<EventListener> result = new ArrayList<>();

        List<EventListener> eventListeners = listeners.get(event);
        if (eventListeners != null) {
            result.addAll(eventListeners);
        }

        List<EventListener> eventOnceListeners = onceListeners.get(event);
        if (eventOnceListeners != null) {
            result.addAll(eventOnceListeners);
        }

        return result.toArray(new EventListener[0]);
    }

    /**
     * Set maximum number of listeners per event (0 = unlimited)
     *
     * @param max Maximum listeners
     * @return this emitter for chaining
     */
    @NotNull
    public EventEmitter setMaxListeners(int max) {
        if (max < 0) {
            throw new IllegalArgumentException("Max listeners must be >= 0");
        }
        this.maxListeners = max;
        return this;
    }

    /**
     * Get maximum number of listeners
     *
     * @return maximum listeners (0 = unlimited)
     */
    public int getMaxListeners() {
        return maxListeners;
    }

    /**
     * Check if max listeners exceeded and log warning
     */
    private void checkMaxListeners(String event, int count) {
        if (maxListeners > 0 && count > maxListeners) {
            logger.warn("Possible EventEmitter memory leak detected. " +
                    "{} {} listeners added for event '{}'. " +
                    "Use setMaxListeners() to increase limit.",
                    count, count == 1 ? "listener" : "listeners", event);
        }
    }

    /**
     * Add listener (alias for on())
     */
    @NotNull
    public EventEmitter addListener(@NotNull String event, @NotNull EventListener listener) {
        return on(event, listener);
    }

    /**
     * Remove listener (alias for off())
     */
    @NotNull
    public EventEmitter removeListener(@NotNull String event, @NotNull EventListener listener) {
        return off(event, listener);
    }

    /**
     * Prepend listener to the beginning of the listeners array
     */
    @NotNull
    public EventEmitter prependListener(@NotNull String event, @NotNull EventListener listener) {
        List<EventListener> eventListeners = listeners.computeIfAbsent(
                event,
                k -> new CopyOnWriteArrayList<>()
        );

        eventListeners.add(0, listener);
        checkMaxListeners(event, eventListeners.size());

        return this;
    }

    /**
     * Prepend once listener to the beginning of the listeners array
     */
    @NotNull
    public EventEmitter prependOnceListener(@NotNull String event, @NotNull EventListener listener) {
        List<EventListener> eventListeners = onceListeners.computeIfAbsent(
                event,
                k -> new CopyOnWriteArrayList<>()
        );

        eventListeners.add(0, listener);

        return this;
    }
}

