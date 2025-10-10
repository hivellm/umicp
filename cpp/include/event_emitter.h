/**
 * UMICP Event Emitter
 * Thread-safe event system for async communication
 * Based on Node.js EventEmitter pattern
 */

#ifndef UMICP_EVENT_EMITTER_H
#define UMICP_EVENT_EMITTER_H

#include "umicp_types.h"
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <any>
#include <string>
#include <atomic>
#include <stdexcept>

namespace umicp {

// Forward declaration
class Event;

/**
 * Event data container
 * Holds typed data that can be passed to event handlers
 */
class Event {
public:
    Event() = default;
    explicit Event(const std::string& name) : event_name_(name) {}

    // Default copy/move constructors
    Event(const Event&) = default;
    Event& operator=(const Event&) = default;
    Event(Event&&) noexcept = default;
    Event& operator=(Event&&) noexcept = default;

    // Set data with key
    template<typename T>
    void set(const std::string& key, const T& value) {
        data_[key] = value;
    }

    // Get data by key
    template<typename T>
    T get(const std::string& key) const {
        auto it = data_.find(key);
        if (it != data_.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                throw std::runtime_error("Invalid type cast for event data key: " + key);
            }
        }
        throw std::runtime_error("Event data key not found: " + key);
    }

    // Check if key exists
    bool has(const std::string& key) const {
        return data_.find(key) != data_.end();
    }

    // Get event name
    std::string name() const { return event_name_; }

    // Set event name
    void set_name(const std::string& name) { event_name_ = name; }

private:
    std::string event_name_;
    std::unordered_map<std::string, std::any> data_;
};

/**
 * EventEmitter class
 * Provides event-driven programming pattern
 * Thread-safe with support for multiple listeners per event
 */
class EventEmitter {
public:
    // Event callback type
    using EventCallback = std::function<void(const Event&)>;

    EventEmitter() : next_handler_id_(0) {}
    virtual ~EventEmitter() = default;

    // Register event handler
    // Returns handler ID for later removal
    size_t on(const std::string& event_name, EventCallback callback);

    // Register one-time event handler
    // Handler is automatically removed after first invocation
    size_t once(const std::string& event_name, EventCallback callback);

    // Remove specific event handler by ID
    bool off(const std::string& event_name, size_t handler_id);

    // Remove all handlers for an event
    void remove_all_listeners(const std::string& event_name);

    // Remove all handlers for all events
    void remove_all_listeners();

    // Emit event to all registered handlers
    void emit(const std::string& event_name, const Event& event);

    // Emit event with no data
    void emit(const std::string& event_name);

    // Get listener count for event
    size_t listener_count(const std::string& event_name) const;

    // Get list of registered event names
    std::vector<std::string> event_names() const;

    // Check if event has any listeners
    bool has_listeners(const std::string& event_name) const;

protected:
    // Helper to create and emit event
    template<typename... Args>
    void emit_event(const std::string& event_name, Args&&... args) {
        Event event(event_name);
        emit_impl(event_name, event);
    }

protected:
    // Internal emit implementation (protected for AsyncEventEmitter)
    void emit_impl(const std::string& event_name, const Event& event);

private:
    struct EventHandler {
        size_t id;
        EventCallback callback;
        bool once;

        EventHandler(size_t handler_id, EventCallback cb, bool is_once = false)
            : id(handler_id), callback(std::move(cb)), once(is_once) {}
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<EventHandler>>> handlers_;
    std::atomic<size_t> next_handler_id_;
};

/**
 * Async EventEmitter
 * Emits events asynchronously in a thread pool
 * Useful for long-running event handlers
 */
class AsyncEventEmitter : public EventEmitter {
public:
    explicit AsyncEventEmitter(size_t thread_pool_size = 4);
    ~AsyncEventEmitter() override;

    // Emit event asynchronously
    void emit_async(const std::string& event_name, const Event& event);

    // Emit event asynchronously with no data
    void emit_async(const std::string& event_name);

    // Wait for all pending events to complete
    void wait_for_events();

    // Get pending event count
    size_t pending_events() const;

private:
    class ThreadPool;
    std::unique_ptr<ThreadPool> thread_pool_;
};

/**
 * Event builder helper
 * Fluent interface for building events
 */
class EventBuilder {
public:
    explicit EventBuilder(const std::string& event_name) : event_(event_name) {}

    template<typename T>
    EventBuilder& set(const std::string& key, const T& value) {
        event_.set(key, value);
        return *this;
    }

    Event build() {
        return std::move(event_);
    }

    operator Event() {
        return std::move(event_);
    }

private:
    Event event_;
};

} // namespace umicp

#endif // UMICP_EVENT_EMITTER_H


