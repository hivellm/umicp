package com.hivellm.umicp.events;

import org.jetbrains.annotations.Nullable;

/**
 * Functional interface for event listeners
 *
 * @author HiveLLM Team
 * @version 0.1.3
 */
@FunctionalInterface
public interface EventListener {
    /**
     * Handle event with data
     *
     * @param data Event data (can be null)
     */
    void handle(@Nullable Object data);
}

