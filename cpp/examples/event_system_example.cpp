/**
 * UMICP Event System Example
 * Demonstrates the EventEmitter pattern in C++
 */

#include "event_emitter.h"
#include "envelope.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace umicp;

int main() {
    std::cout << "=== UMICP Event System Example ===" << std::endl << std::endl;

    // Create event emitter
    EventEmitter emitter;

    // Example 1: Basic event handling
    std::cout << "Example 1: Basic Event Handling" << std::endl;

    emitter.on("message", [](const Event& event) {
        std::string text = event.get<std::string>("text");
        std::cout << "  Received message: " << text << std::endl;
    });

    Event message_event("message");
    message_event.set("text", "Hello from UMICP!");
    emitter.emit("message", message_event);

    std::cout << std::endl;

    // Example 2: Multiple listeners
    std::cout << "Example 2: Multiple Listeners" << std::endl;

    emitter.on("data", [](const Event& event) {
        int value = event.get<int>("value");
        std::cout << "  Listener 1 received: " << value << std::endl;
    });

    emitter.on("data", [](const Event& event) {
        int value = event.get<int>("value");
        std::cout << "  Listener 2 received: " << value * 2 << std::endl;
    });

    Event data_event("data");
    data_event.set("value", 42);
    emitter.emit("data", data_event);

    std::cout << std::endl;

    // Example 3: Once handlers
    std::cout << "Example 3: Once Handlers (fire only once)" << std::endl;

    emitter.once("startup", [](const Event& event) {
        std::cout << "  Startup event - this fires only once!" << std::endl;
    });

    Event startup_event("startup");
    emitter.emit("startup", startup_event);
    std::cout << "  Emitting again..." << std::endl;
    emitter.emit("startup", startup_event);  // Won't fire

    std::cout << std::endl;

    // Example 4: Event with complex data
    std::cout << "Example 4: Complex Event Data" << std::endl;

    emitter.on("envelope", [](const Event& event) {
        Envelope env = event.get<Envelope>("envelope");
        std::cout << "  Received envelope:" << std::endl;
        std::cout << "    Message ID: " << env.msg_id << std::endl;
        std::cout << "    Operation: " << static_cast<int>(env.op) << std::endl;
    });

    Envelope env;
    env.msg_id = "msg-123";
    env.version = "1.0";
    env.op = OperationType::CONTROL;

    Event envelope_event("envelope");
    envelope_event.set("envelope", env);
    emitter.emit("envelope", envelope_event);

    std::cout << std::endl;

    // Example 5: Async events
    std::cout << "Example 5: Async Events" << std::endl;

    AsyncEventEmitter async_emitter(4);  // 4 threads

    async_emitter.on("heavy_task", [](const Event& event) {
        int task_id = event.get<int>("task_id");
        std::cout << "  Processing task " << task_id << " on thread "
                  << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    // Emit multiple async events
    for (int i = 1; i <= 5; i++) {
        Event task_event("heavy_task");
        task_event.set("task_id", i);
        async_emitter.emit_async("heavy_task", task_event);
    }

    std::cout << "  Waiting for async tasks to complete..." << std::endl;
    async_emitter.wait_all();
    std::cout << "  All async tasks completed!" << std::endl;

    std::cout << std::endl;

    // Example 6: Removing listeners
    std::cout << "Example 6: Removing Listeners" << std::endl;

    auto handler_id = emitter.on("removable", [](const Event& event) {
        std::cout << "  This handler can be removed" << std::endl;
    });

    Event removable_event("removable");
    emitter.emit("removable", removable_event);

    emitter.off("removable", handler_id);
    std::cout << "  Handler removed, emitting again..." << std::endl;
    emitter.emit("removable", removable_event);  // Won't fire

    std::cout << std::endl;

    // Example 7: Event queries
    std::cout << "Example 7: Event Queries" << std::endl;

    emitter.on("test1", [](const Event& e) {});
    emitter.on("test1", [](const Event& e) {});
    emitter.on("test2", [](const Event& e) {});

    std::cout << "  Listeners for 'test1': " << emitter.listener_count("test1") << std::endl;
    std::cout << "  Listeners for 'test2': " << emitter.listener_count("test2") << std::endl;
    std::cout << "  Has 'test1' listeners: " << (emitter.has_listeners("test1") ? "yes" : "no") << std::endl;

    std::cout << std::endl;
    std::cout << "=== Event System Example Complete ===" << std::endl;

    return 0;
}

