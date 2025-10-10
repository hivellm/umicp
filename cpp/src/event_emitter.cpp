/**
 * UMICP Event Emitter Implementation
 */

#include "event_emitter.h"
#include <algorithm>
#include <queue>
#include <thread>
#include <condition_variable>
#include <future>
#include <iostream>

namespace umicp {

// ============================================================================
// EventEmitter Implementation
// ============================================================================

size_t EventEmitter::on(const std::string& event_name, EventCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);

    size_t handler_id = next_handler_id_++;
    auto handler = std::make_shared<EventHandler>(handler_id, std::move(callback), false);

    handlers_[event_name].push_back(handler);

    return handler_id;
}

size_t EventEmitter::once(const std::string& event_name, EventCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);

    size_t handler_id = next_handler_id_++;
    auto handler = std::make_shared<EventHandler>(handler_id, std::move(callback), true);

    handlers_[event_name].push_back(handler);

    return handler_id;
}

bool EventEmitter::off(const std::string& event_name, size_t handler_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = handlers_.find(event_name);
    if (it == handlers_.end()) {
        return false;
    }

    auto& handler_list = it->second;
    auto handler_it = std::remove_if(
        handler_list.begin(),
        handler_list.end(),
        [handler_id](const std::shared_ptr<EventHandler>& h) {
            return h->id == handler_id;
        }
    );

    if (handler_it != handler_list.end()) {
        handler_list.erase(handler_it, handler_list.end());
        return true;
    }

    return false;
}

void EventEmitter::remove_all_listeners(const std::string& event_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_.erase(event_name);
}

void EventEmitter::remove_all_listeners() {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_.clear();
}

void EventEmitter::emit(const std::string& event_name, const Event& event) {
    emit_impl(event_name, event);
}

void EventEmitter::emit(const std::string& event_name) {
    Event event(event_name);
    emit_impl(event_name, event);
}

size_t EventEmitter::listener_count(const std::string& event_name) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = handlers_.find(event_name);
    if (it == handlers_.end()) {
        return 0;
    }

    return it->second.size();
}

std::vector<std::string> EventEmitter::event_names() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::string> names;
    names.reserve(handlers_.size());

    for (const auto& pair : handlers_) {
        if (!pair.second.empty()) {
            names.push_back(pair.first);
        }
    }

    return names;
}

bool EventEmitter::has_listeners(const std::string& event_name) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = handlers_.find(event_name);
    return it != handlers_.end() && !it->second.empty();
}

void EventEmitter::emit_impl(const std::string& event_name, const Event& event) {
    // Copy handlers to avoid holding lock during callback execution
    std::vector<std::shared_ptr<EventHandler>> handlers_copy;
    std::vector<size_t> once_handler_ids;

    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = handlers_.find(event_name);
        if (it == handlers_.end()) {
            return; // No handlers registered
        }

        handlers_copy = it->second;

        // Collect IDs of once handlers for later removal
        for (const auto& handler : handlers_copy) {
            if (handler->once) {
                once_handler_ids.push_back(handler->id);
            }
        }
    }

    // Execute callbacks without holding lock
    for (const auto& handler : handlers_copy) {
        try {
            handler->callback(event);
        } catch (const std::exception& e) {
            // Log error but continue with other handlers
            // In production, use proper logging
            std::cerr << "Event handler error for '" << event_name << "': " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown event handler error for '" << event_name << "'" << std::endl;
        }
    }

    // Remove once handlers
    for (size_t handler_id : once_handler_ids) {
        off(event_name, handler_id);
    }
}

// ============================================================================
// AsyncEventEmitter Implementation
// ============================================================================

/**
 * Simple thread pool for async event handling
 */
class AsyncEventEmitter::ThreadPool {
public:
    explicit ThreadPool(size_t num_threads) : stop_(false), pending_tasks_(0) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                worker_thread();
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();

        for (std::thread& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    template<typename F>
    void enqueue(F&& task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(std::forward<F>(task));
            pending_tasks_++;
        }
        condition_.notify_one();
    }

    void wait_for_completion() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        completion_condition_.wait(lock, [this] {
            return tasks_.empty() && pending_tasks_ == 0;
        });
    }

    size_t pending_tasks() const {
        return pending_tasks_.load();
    }

private:
    void worker_thread() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] {
                    return stop_ || !tasks_.empty();
                });

                if (stop_ && tasks_.empty()) {
                    return;
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }

            // Execute task
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "Async event task error: " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown async event task error" << std::endl;
            }

            // Decrement pending tasks and notify
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                pending_tasks_--;
                if (tasks_.empty() && pending_tasks_ == 0) {
                    completion_condition_.notify_all();
                }
            }
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::condition_variable completion_condition_;
    bool stop_;
    std::atomic<size_t> pending_tasks_;
};

AsyncEventEmitter::AsyncEventEmitter(size_t thread_pool_size)
    : EventEmitter(),
      thread_pool_(std::make_unique<ThreadPool>(thread_pool_size)) {
}

AsyncEventEmitter::~AsyncEventEmitter() {
    wait_for_events();
}

void AsyncEventEmitter::emit_async(const std::string& event_name, const Event& event) {
    // Copy event data for async execution
    Event event_copy = event;
    std::string event_name_copy = event_name;

    thread_pool_->enqueue([this, event_name_copy, event_copy]() {
        emit_impl(event_name_copy, event_copy);
    });
}

void AsyncEventEmitter::emit_async(const std::string& event_name) {
    Event event(event_name);
    emit_async(event_name, event);
}

void AsyncEventEmitter::wait_for_events() {
    if (thread_pool_) {
        thread_pool_->wait_for_completion();
    }
}

size_t AsyncEventEmitter::pending_events() const {
    return thread_pool_ ? thread_pool_->pending_tasks() : 0;
}

} // namespace umicp


