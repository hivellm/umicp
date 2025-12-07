package com.hivellm.umicp.events;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import static org.junit.jupiter.api.Assertions.*;

class EventEmitterTest {

    private EventEmitter emitter;

    @BeforeEach
    void setUp() {
        emitter = new EventEmitter();
    }

    @Test
    void testOnAndEmit() {
        AtomicInteger counter = new AtomicInteger(0);

        emitter.on("test-event", data -> counter.incrementAndGet());

        assertTrue(emitter.emit("test-event"));
        assertEquals(1, counter.get());

        // Emit again
        emitter.emit("test-event");
        assertEquals(2, counter.get());
    }

    @Test
    void testOnceListener() {
        AtomicInteger counter = new AtomicInteger(0);

        emitter.once("test-event", data -> counter.incrementAndGet());

        emitter.emit("test-event");
        assertEquals(1, counter.get());

        // Should not trigger again
        emitter.emit("test-event");
        assertEquals(1, counter.get());
    }

    @Test
    void testTypedListener() {
        List<String> received = new ArrayList<>();

        emitter.on("message", String.class, received::add);

        emitter.emit("message", "Hello");
        emitter.emit("message", "World");

        assertEquals(2, received.size());
        assertEquals("Hello", received.get(0));
        assertEquals("World", received.get(1));
    }

    @Test
    void testTypedListenerWrongType() {
        List<String> received = new ArrayList<>();

        emitter.on("message", String.class, received::add);

        // Emit with wrong type - should not crash but log warning
        emitter.emit("message", 123);

        assertTrue(received.isEmpty());
    }

    @Test
    void testOff() {
        AtomicInteger counter = new AtomicInteger(0);
        EventListener listener = data -> counter.incrementAndGet();

        emitter.on("test-event", listener);
        emitter.emit("test-event");
        assertEquals(1, counter.get());

        // Remove listener
        emitter.off("test-event", listener);
        emitter.emit("test-event");
        assertEquals(1, counter.get()); // Should not increment
    }

    @Test
    void testRemoveAllListeners() {
        AtomicInteger counter1 = new AtomicInteger(0);
        AtomicInteger counter2 = new AtomicInteger(0);

        emitter.on("event1", data -> counter1.incrementAndGet());
        emitter.on("event2", data -> counter2.incrementAndGet());

        emitter.removeAllListeners("event1");

        assertFalse(emitter.emit("event1"));
        assertTrue(emitter.emit("event2"));

        assertEquals(0, counter1.get());
        assertEquals(1, counter2.get());
    }

    @Test
    void testRemoveAllListenersForAllEvents() {
        AtomicInteger counter = new AtomicInteger(0);

        emitter.on("event1", data -> counter.incrementAndGet());
        emitter.on("event2", data -> counter.incrementAndGet());

        emitter.removeAllListeners();

        assertFalse(emitter.emit("event1"));
        assertFalse(emitter.emit("event2"));
        assertEquals(0, counter.get());
    }

    @Test
    void testEventNames() {
        emitter.on("event1", data -> {});
        emitter.on("event2", data -> {});
        emitter.once("event3", data -> {});

        String[] names = emitter.eventNames();
        assertEquals(3, names.length);
        assertTrue(List.of(names).contains("event1"));
        assertTrue(List.of(names).contains("event2"));
        assertTrue(List.of(names).contains("event3"));
    }

    @Test
    void testListenerCount() {
        emitter.on("test-event", data -> {});
        emitter.on("test-event", data -> {});
        emitter.once("test-event", data -> {});

        assertEquals(3, emitter.listenerCount("test-event"));
    }

    @Test
    void testListeners() {
        EventListener listener1 = data -> {};
        EventListener listener2 = data -> {};

        emitter.on("test-event", listener1);
        emitter.once("test-event", listener2);

        EventListener[] listeners = emitter.listeners("test-event");
        assertEquals(2, listeners.length);
    }

    @Test
    void testMaxListeners() {
        emitter.setMaxListeners(2);
        assertEquals(2, emitter.getMaxListeners());

        // Should not throw, just log warning
        emitter.on("test-event", data -> {});
        emitter.on("test-event", data -> {});
        emitter.on("test-event", data -> {}); // Exceeds max

        assertEquals(3, emitter.listenerCount("test-event"));
    }

    @Test
    void testMaxListenersInvalid() {
        assertThrows(IllegalArgumentException.class, () -> {
            emitter.setMaxListeners(-1);
        });
    }

    @Test
    void testMultipleListeners() {
        AtomicInteger counter1 = new AtomicInteger(0);
        AtomicInteger counter2 = new AtomicInteger(0);
        AtomicInteger counter3 = new AtomicInteger(0);

        emitter.on("test-event", data -> counter1.incrementAndGet());
        emitter.on("test-event", data -> counter2.incrementAndGet());
        emitter.on("test-event", data -> counter3.incrementAndGet());

        emitter.emit("test-event");

        assertEquals(1, counter1.get());
        assertEquals(1, counter2.get());
        assertEquals(1, counter3.get());
    }

    @Test
    void testPrependListener() {
        List<Integer> order = new ArrayList<>();

        emitter.on("test-event", data -> order.add(1));
        emitter.on("test-event", data -> order.add(2));
        emitter.prependListener("test-event", data -> order.add(0));

        emitter.emit("test-event");

        assertEquals(List.of(0, 1, 2), order);
    }

    @Test
    void testPrependOnceListener() {
        List<Integer> order = new ArrayList<>();

        emitter.once("test-event", data -> order.add(1));
        emitter.prependOnceListener("test-event", data -> order.add(0));

        emitter.emit("test-event");

        assertEquals(List.of(0, 1), order);

        // Both should be gone
        order.clear();
        emitter.emit("test-event");
        assertTrue(order.isEmpty());
    }

    @Test
    void testEmitWithData() {
        List<Object> received = new ArrayList<>();

        emitter.on("test-event", received::add);

        emitter.emit("test-event", "data1");
        emitter.emit("test-event", 42);
        emitter.emit("test-event", null);

        assertEquals(3, received.size());
        assertEquals("data1", received.get(0));
        assertEquals(42, received.get(1));
        assertNull(received.get(2));
    }

    @Test
    void testEmitNoListeners() {
        assertFalse(emitter.emit("non-existent-event"));
    }

    @Test
    void testAliases() {
        AtomicInteger counter = new AtomicInteger(0);
        EventListener listener = data -> counter.incrementAndGet();

        // addListener is alias for on
        emitter.addListener("test-event", listener);
        emitter.emit("test-event");
        assertEquals(1, counter.get());

        // removeListener is alias for off
        emitter.removeListener("test-event", listener);
        emitter.emit("test-event");
        assertEquals(1, counter.get());
    }

    @Test
    void testThreadSafety() throws InterruptedException {
        int threadCount = 10;
        int eventsPerThread = 100;
        CountDownLatch latch = new CountDownLatch(threadCount);
        AtomicInteger counter = new AtomicInteger(0);

        emitter.on("test-event", data -> counter.incrementAndGet());

        // Start threads that emit events
        for (int i = 0; i < threadCount; i++) {
            new Thread(() -> {
                for (int j = 0; j < eventsPerThread; j++) {
                    emitter.emit("test-event");
                }
                latch.countDown();
            }).start();
        }

        assertTrue(latch.await(5, TimeUnit.SECONDS));
        assertEquals(threadCount * eventsPerThread, counter.get());
    }

    @Test
    void testExceptionInListener() {
        AtomicInteger counter = new AtomicInteger(0);

        emitter.on("test-event", data -> {
            throw new RuntimeException("Test exception");
        });
        emitter.on("test-event", data -> counter.incrementAndGet());

        // Should not throw, should log error and continue
        emitter.emit("test-event");

        // Second listener should still execute
        assertEquals(1, counter.get());
    }
}

