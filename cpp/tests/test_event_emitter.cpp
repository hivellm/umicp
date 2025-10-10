/**
 * UMICP Event Emitter Tests
 */

#include "event_emitter.h"
#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <chrono>

using namespace umicp;

class EventEmitterTest : public ::testing::Test {
protected:
    void SetUp() override {
        emitter = std::make_unique<EventEmitter>();
    }

    void TearDown() override {
        emitter.reset();
    }

    std::unique_ptr<EventEmitter> emitter;
};

// ============================================================================
// Basic Event Registration and Emission Tests
// ============================================================================

TEST_F(EventEmitterTest, RegisterAndEmitBasicEvent) {
    std::atomic<int> call_count{0};

    emitter->on("test", [&call_count](const Event& event) {
        call_count++;
    });

    emitter->emit("test");

    EXPECT_EQ(call_count, 1);
}

TEST_F(EventEmitterTest, RegisterMultipleHandlers) {
    std::atomic<int> handler1_calls{0};
    std::atomic<int> handler2_calls{0};

    emitter->on("test", [&handler1_calls](const Event& event) {
        handler1_calls++;
    });

    emitter->on("test", [&handler2_calls](const Event& event) {
        handler2_calls++;
    });

    emitter->emit("test");

    EXPECT_EQ(handler1_calls, 1);
    EXPECT_EQ(handler2_calls, 1);
}

TEST_F(EventEmitterTest, EmitMultipleTimes) {
    std::atomic<int> call_count{0};

    emitter->on("test", [&call_count](const Event& event) {
        call_count++;
    });

    emitter->emit("test");
    emitter->emit("test");
    emitter->emit("test");

    EXPECT_EQ(call_count, 3);
}

// ============================================================================
// Event Data Tests
// ============================================================================

TEST_F(EventEmitterTest, PassEventData) {
    std::string received_message;
    int received_value = 0;

    emitter->on("data", [&](const Event& event) {
        received_message = event.get<std::string>("message");
        received_value = event.get<int>("value");
    });

    Event event("data");
    event.set("message", std::string("Hello"));
    event.set("value", 42);

    emitter->emit("data", event);

    EXPECT_EQ(received_message, "Hello");
    EXPECT_EQ(received_value, 42);
}

TEST_F(EventEmitterTest, EventBuilder) {
    std::string received_text;
    bool received_flag = false;

    emitter->on("builder_test", [&](const Event& event) {
        received_text = event.get<std::string>("text");
        received_flag = event.get<bool>("flag");
    });

    EventBuilder builder("builder_test");
    builder.set("text", std::string("Test"))
           .set("flag", true);

    emitter->emit("builder_test", builder.build());

    EXPECT_EQ(received_text, "Test");
    EXPECT_TRUE(received_flag);
}

TEST_F(EventEmitterTest, EventDataHasKey) {
    Event event("test");
    event.set("key1", 123);

    EXPECT_TRUE(event.has("key1"));
    EXPECT_FALSE(event.has("key2"));
}

TEST_F(EventEmitterTest, EventDataTypeMismatch) {
    Event event("test");
    event.set("value", 42);

    EXPECT_THROW(event.get<std::string>("value"), std::runtime_error);
}

// ============================================================================
// Once Handler Tests
// ============================================================================

TEST_F(EventEmitterTest, OnceHandlerCalledOnce) {
    std::atomic<int> call_count{0};

    emitter->once("test", [&call_count](const Event& event) {
        call_count++;
    });

    emitter->emit("test");
    emitter->emit("test");
    emitter->emit("test");

    EXPECT_EQ(call_count, 1);
}

TEST_F(EventEmitterTest, OnceAndRegularHandlers) {
    std::atomic<int> once_calls{0};
    std::atomic<int> regular_calls{0};

    emitter->once("test", [&once_calls](const Event& event) {
        once_calls++;
    });

    emitter->on("test", [&regular_calls](const Event& event) {
        regular_calls++;
    });

    emitter->emit("test");
    emitter->emit("test");

    EXPECT_EQ(once_calls, 1);
    EXPECT_EQ(regular_calls, 2);
}

// ============================================================================
// Handler Removal Tests
// ============================================================================

TEST_F(EventEmitterTest, RemoveSpecificHandler) {
    std::atomic<int> handler1_calls{0};
    std::atomic<int> handler2_calls{0};

    size_t id1 = emitter->on("test", [&handler1_calls](const Event& event) {
        handler1_calls++;
    });

    emitter->on("test", [&handler2_calls](const Event& event) {
        handler2_calls++;
    });

    emitter->emit("test");
    EXPECT_EQ(handler1_calls, 1);
    EXPECT_EQ(handler2_calls, 1);

    // Remove first handler
    bool removed = emitter->off("test", id1);
    EXPECT_TRUE(removed);

    emitter->emit("test");
    EXPECT_EQ(handler1_calls, 1); // Should not increase
    EXPECT_EQ(handler2_calls, 2); // Should increase
}

TEST_F(EventEmitterTest, RemoveAllListenersForEvent) {
    std::atomic<int> call_count{0};

    emitter->on("test", [&call_count](const Event& event) {
        call_count++;
    });

    emitter->on("test", [&call_count](const Event& event) {
        call_count++;
    });

    emitter->emit("test");
    EXPECT_EQ(call_count, 2);

    emitter->remove_all_listeners("test");

    emitter->emit("test");
    EXPECT_EQ(call_count, 2); // Should not increase
}

TEST_F(EventEmitterTest, RemoveAllListeners) {
    std::atomic<int> event1_calls{0};
    std::atomic<int> event2_calls{0};

    emitter->on("event1", [&event1_calls](const Event& event) {
        event1_calls++;
    });

    emitter->on("event2", [&event2_calls](const Event& event) {
        event2_calls++;
    });

    emitter->emit("event1");
    emitter->emit("event2");
    EXPECT_EQ(event1_calls, 1);
    EXPECT_EQ(event2_calls, 1);

    emitter->remove_all_listeners();

    emitter->emit("event1");
    emitter->emit("event2");
    EXPECT_EQ(event1_calls, 1); // Should not increase
    EXPECT_EQ(event2_calls, 1); // Should not increase
}

// ============================================================================
// Query Tests
// ============================================================================

TEST_F(EventEmitterTest, ListenerCount) {
    EXPECT_EQ(emitter->listener_count("test"), 0);

    emitter->on("test", [](const Event& event) {});
    EXPECT_EQ(emitter->listener_count("test"), 1);

    emitter->on("test", [](const Event& event) {});
    EXPECT_EQ(emitter->listener_count("test"), 2);

    emitter->on("other", [](const Event& event) {});
    EXPECT_EQ(emitter->listener_count("test"), 2);
    EXPECT_EQ(emitter->listener_count("other"), 1);
}

TEST_F(EventEmitterTest, HasListeners) {
    EXPECT_FALSE(emitter->has_listeners("test"));

    emitter->on("test", [](const Event& event) {});
    EXPECT_TRUE(emitter->has_listeners("test"));

    emitter->remove_all_listeners("test");
    EXPECT_FALSE(emitter->has_listeners("test"));
}

TEST_F(EventEmitterTest, EventNames) {
    auto names = emitter->event_names();
    EXPECT_TRUE(names.empty());

    emitter->on("event1", [](const Event& event) {});
    emitter->on("event2", [](const Event& event) {});
    emitter->on("event3", [](const Event& event) {});

    names = emitter->event_names();
    EXPECT_EQ(names.size(), 3);

    EXPECT_TRUE(std::find(names.begin(), names.end(), "event1") != names.end());
    EXPECT_TRUE(std::find(names.begin(), names.end(), "event2") != names.end());
    EXPECT_TRUE(std::find(names.begin(), names.end(), "event3") != names.end());
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(EventEmitterTest, ConcurrentEmit) {
    std::atomic<int> call_count{0};

    emitter->on("test", [&call_count](const Event& event) {
        call_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    });

    std::vector<std::thread> threads;
    const int num_threads = 10;
    const int emits_per_thread = 10;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, emits_per_thread]() {
            for (int j = 0; j < emits_per_thread; ++j) {
                emitter->emit("test");
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(call_count, num_threads * emits_per_thread);
}

TEST_F(EventEmitterTest, ConcurrentRegisterAndEmit) {
    std::atomic<int> call_count{0};
    std::atomic<bool> stop{false};

    // Thread that continuously emits
    std::thread emitter_thread([this, &stop]() {
        while (!stop) {
            emitter->emit("test");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    // Threads that register handlers
    std::vector<std::thread> register_threads;
    for (int i = 0; i < 5; ++i) {
        register_threads.emplace_back([this, &call_count]() {
            emitter->on("test", [&call_count](const Event& event) {
                call_count++;
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }

    for (auto& thread : register_threads) {
        thread.join();
    }

    stop = true;
    emitter_thread.join();

    // Should have at least some calls
    EXPECT_GT(call_count, 0);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_F(EventEmitterTest, HandlerExceptionDoesNotStopOthers) {
    std::atomic<int> handler1_calls{0};
    std::atomic<int> handler2_calls{0};

    emitter->on("test", [&handler1_calls](const Event& event) {
        handler1_calls++;
        throw std::runtime_error("Handler 1 error");
    });

    emitter->on("test", [&handler2_calls](const Event& event) {
        handler2_calls++;
    });

    // Should not throw, error should be caught internally
    EXPECT_NO_THROW(emitter->emit("test"));

    EXPECT_EQ(handler1_calls, 1);
    EXPECT_EQ(handler2_calls, 1); // Should still be called
}

// ============================================================================
// AsyncEventEmitter Tests
// ============================================================================

TEST(AsyncEventEmitterTest, AsyncEmit) {
    AsyncEventEmitter async_emitter(4);
    std::atomic<int> call_count{0};

    async_emitter.on("async_test", [&call_count](const Event& event) {
        call_count++;
    });

    async_emitter.emit_async("async_test");
    async_emitter.emit_async("async_test");
    async_emitter.emit_async("async_test");

    async_emitter.wait_for_events();

    EXPECT_EQ(call_count, 3);
}

TEST(AsyncEventEmitterTest, AsyncWithEventData) {
    AsyncEventEmitter async_emitter(4);
    std::atomic<int> sum{0};

    async_emitter.on("add", [&sum](const Event& event) {
        int value = event.get<int>("value");
        sum += value;
    });

    for (int i = 1; i <= 10; ++i) {
        Event event("add");
        event.set("value", i);
        async_emitter.emit_async("add", event);
    }

    async_emitter.wait_for_events();

    EXPECT_EQ(sum, 55); // 1+2+3+...+10 = 55
}

TEST(AsyncEventEmitterTest, PendingEventsCount) {
    AsyncEventEmitter async_emitter(2); // Small pool to ensure queueing

    async_emitter.on("slow", [](const Event& event) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    // Queue multiple events
    for (int i = 0; i < 10; ++i) {
        async_emitter.emit_async("slow");
    }

    // Should have pending events
    EXPECT_GT(async_emitter.pending_events(), 0);

    async_emitter.wait_for_events();

    // All should be completed
    EXPECT_EQ(async_emitter.pending_events(), 0);
}

TEST(AsyncEventEmitterTest, MixedSyncAndAsync) {
    AsyncEventEmitter async_emitter(4);
    std::atomic<int> sync_calls{0};
    std::atomic<int> async_calls{0};

    async_emitter.on("test", [&](const Event& event) {
        if (event.has("is_async") && event.get<bool>("is_async")) {
            async_calls++;
        } else {
            sync_calls++;
        }
    });

    // Sync emit
    Event sync_event("test");
    sync_event.set("is_async", false);
    async_emitter.emit("test", sync_event);

    // Async emit
    Event async_event("test");
    async_event.set("is_async", true);
    async_emitter.emit_async("test", async_event);
    async_emitter.emit_async("test", async_event);

    async_emitter.wait_for_events();

    EXPECT_EQ(sync_calls, 1);
    EXPECT_EQ(async_calls, 2);
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


