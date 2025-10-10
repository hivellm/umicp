<?php

namespace UMICP\Core;

/**
 * UMICP Event Emitter
 *
 * Node.js-inspired event system for PHP
 * Supports multiple listeners per event
 *
 * @package HiveLLM\UMICP\Core
 */
class EventEmitter
{
    private array $listeners = [];
    private array $onceListeners = [];
    private array $stats = [];

    /**
     * Register an event listener
     *
     * @param string $event Event name
     * @param callable $listener Listener function
     * @return self For method chaining
     */
    public function on(string $event, callable $listener): self
    {
        if (!isset($this->listeners[$event])) {
            $this->listeners[$event] = [];
            $this->stats[$event] = ['emits' => 0, 'listeners' => 0];
        }

        $this->listeners[$event][] = $listener;
        $this->stats[$event]['listeners']++;

        return $this;
    }

    /**
     * Register a one-time event listener
     *
     * @param string $event Event name
     * @param callable $listener Listener function
     * @return self For method chaining
     */
    public function once(string $event, callable $listener): self
    {
        if (!isset($this->onceListeners[$event])) {
            $this->onceListeners[$event] = [];
        }

        $this->onceListeners[$event][] = $listener;

        return $this;
    }

    /**
     * Remove an event listener
     *
     * @param string $event Event name
     * @param callable|null $listener Specific listener or null for all
     * @return self For method chaining
     */
    public function off(string $event, ?callable $listener = null): self
    {
        if ($listener === null) {
            // Remove all listeners for this event
            if (isset($this->listeners[$event])) {
                $this->stats[$event]['listeners'] = 0;
                unset($this->listeners[$event]);
            }
            unset($this->onceListeners[$event]);
        } else {
            // Remove specific listener
            if (isset($this->listeners[$event])) {
                $key = array_search($listener, $this->listeners[$event], true);
                if ($key !== false) {
                    unset($this->listeners[$event][$key]);
                    $this->stats[$event]['listeners']--;
                }
            }
        }

        return $this;
    }

    /**
     * Emit an event
     *
     * @param string $event Event name
     * @param mixed ...$args Arguments to pass to listeners
     * @return self For method chaining
     */
    public function emit(string $event, ...$args): self
    {
        if (isset($this->stats[$event])) {
            $this->stats[$event]['emits']++;
        } else {
            $this->stats[$event] = ['emits' => 1, 'listeners' => 0];
        }

        // Call persistent listeners
        if (isset($this->listeners[$event])) {
            foreach ($this->listeners[$event] as $listener) {
                try {
                    $listener(...$args);
                } catch (\Exception $e) {
                    // Continue to next listener
                    error_log("Event listener error for '$event': " . $e->getMessage());
                }
            }
        }

        // Call and remove once listeners
        if (isset($this->onceListeners[$event])) {
            foreach ($this->onceListeners[$event] as $listener) {
                try {
                    $listener(...$args);
                } catch (\Exception $e) {
                    error_log("Once listener error for '$event': " . $e->getMessage());
                }
            }
            unset($this->onceListeners[$event]);
        }

        return $this;
    }

    /**
     * Get listener count for an event
     *
     * @param string $event Event name
     * @return int Number of listeners
     */
    public function listenerCount(string $event): int
    {
        $count = 0;

        if (isset($this->listeners[$event])) {
            $count += count($this->listeners[$event]);
        }

        if (isset($this->onceListeners[$event])) {
            $count += count($this->onceListeners[$event]);
        }

        return $count;
    }

    /**
     * Get all event names
     *
     * @return array Event names
     */
    public function eventNames(): array
    {
        return array_unique(array_merge(
            array_keys($this->listeners),
            array_keys($this->onceListeners)
        ));
    }

    /**
     * Remove all listeners
     *
     * @return self For method chaining
     */
    public function removeAllListeners(): self
    {
        $this->listeners = [];
        $this->onceListeners = [];

        foreach ($this->stats as $event => &$stat) {
            $stat['listeners'] = 0;
        }

        return $this;
    }

    /**
     * Get event statistics
     *
     * @return array Statistics
     */
    public function getStats(): array
    {
        return $this->stats;
    }
}

