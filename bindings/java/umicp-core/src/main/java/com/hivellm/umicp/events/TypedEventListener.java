package com.hivellm.umicp.events;

import org.jetbrains.annotations.NotNull;

/**
 * Functional interface for typed event listeners
 *
 * @param <T> Event data type
 * @author HiveLLM Team
 * @version 0.1.3
 */
@FunctionalInterface
public interface TypedEventListener<T> {
    /**
     * Handle event with typed data
     *
     * @param data Event data (non-null)
     */
    void handle(@NotNull T data);
}

