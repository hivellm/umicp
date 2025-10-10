package com.hivellm.umicp.transport;

import org.junit.jupiter.api.DisplayName;
import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import static org.assertj.core.api.Assertions.*;

/**
 * Unit tests for {@link ResourceManager}.
 *
 * @author HiveLLM Team
 * @version 0.1.1
 */
class ResourceManagerTest {

    @Test
    @DisplayName("Should create with default name")
    void testDefaultName() {
        ResourceManager manager = new ResourceManager();
        assertThat(manager.toString()).contains("ResourceManager");
    }

    @Test
    @DisplayName("Should create with custom name")
    void testCustomName() {
        ResourceManager manager = new ResourceManager("TestManager");
        assertThat(manager.toString()).contains("TestManager");
    }

    @Test
    @DisplayName("Should register resource")
    void testRegisterResource() {
        ResourceManager manager = new ResourceManager();

        TestResource resource = new TestResource();
        TestResource registered = manager.register(resource);

        assertThat(registered).isSameAs(resource);
        assertThat(manager.getResourceCount()).isEqualTo(1);
    }

    @Test
    @DisplayName("Should close registered resources")
    void testCloseRegisteredResources() {
        TestResource resource = new TestResource();

        try (ResourceManager manager = new ResourceManager()) {
            manager.register(resource);
            assertThat(resource.isClosed()).isFalse();
        }

        assertThat(resource.isClosed()).isTrue();
    }

    @Test
    @DisplayName("Should close multiple resources in reverse order")
    void testCloseMultipleResources() {
        AtomicInteger closeOrder = new AtomicInteger(0);
        OrderedTestResource resource1 = new OrderedTestResource(closeOrder, 1);
        OrderedTestResource resource2 = new OrderedTestResource(closeOrder, 2);
        OrderedTestResource resource3 = new OrderedTestResource(closeOrder, 3);

        try (ResourceManager manager = new ResourceManager()) {
            manager.register(resource1);
            manager.register(resource2);
            manager.register(resource3);
        }

        // All should be closed
        assertThat(resource1.isClosed()).isTrue();
        assertThat(resource2.isClosed()).isTrue();
        assertThat(resource3.isClosed()).isTrue();

        // Should close in reverse order (LIFO)
        assertThat(resource3.getCloseOrder()).isEqualTo(1);
        assertThat(resource2.getCloseOrder()).isEqualTo(2);
        assertThat(resource1.getCloseOrder()).isEqualTo(3);
    }

    @Test
    @DisplayName("Should unregister resource")
    void testUnregisterResource() {
        ResourceManager manager = new ResourceManager();
        TestResource resource = new TestResource();

        manager.register(resource);
        assertThat(manager.getResourceCount()).isEqualTo(1);

        boolean unregistered = manager.unregister(resource);
        assertThat(unregistered).isTrue();
        assertThat(manager.getResourceCount()).isEqualTo(0);
    }

    @Test
    @DisplayName("Should handle unregister of non-existent resource")
    void testUnregisterNonExistent() {
        ResourceManager manager = new ResourceManager();
        TestResource resource = new TestResource();

        boolean unregistered = manager.unregister(resource);
        assertThat(unregistered).isFalse();
    }

    @Test
    @DisplayName("Should not close unregistered resources")
    void testNotCloseUnregistered() {
        TestResource resource = new TestResource();

        try (ResourceManager manager = new ResourceManager()) {
            manager.register(resource);
            manager.unregister(resource);
        }

        assertThat(resource.isClosed()).isFalse();
    }

    @Test
    @DisplayName("Should handle exceptions during close")
    void testExceptionDuringClose() {
        FailingResource failing = new FailingResource();
        TestResource normal = new TestResource();

        try (ResourceManager manager = new ResourceManager()) {
            manager.register(normal);
            manager.register(failing);
        }

        // Normal resource should still be closed despite failing resource
        assertThat(normal.isClosed()).isTrue();
    }

    @Test
    @DisplayName("Should safely close null resource")
    void testSafeCloseNull() {
        boolean result = ResourceManager.safeClose(null);
        assertThat(result).isTrue();
    }

    @Test
    @DisplayName("Should safely close valid resource")
    void testSafeCloseValid() {
        TestResource resource = new TestResource();

        boolean result = ResourceManager.safeClose(resource);

        assertThat(result).isTrue();
        assertThat(resource.isClosed()).isTrue();
    }

    @Test
    @DisplayName("Should safely close failing resource")
    void testSafeCloseFailing() {
        FailingResource resource = new FailingResource();

        boolean result = ResourceManager.safeClose(resource);

        assertThat(result).isFalse();
    }

    @Test
    @DisplayName("Should safely close with description")
    void testSafeCloseWithDescription() {
        TestResource resource = new TestResource();

        boolean result = ResourceManager.safeClose(resource, "test-resource");

        assertThat(result).isTrue();
        assertThat(resource.isClosed()).isTrue();
    }

    @Test
    @DisplayName("Should be idempotent on close")
    void testIdempotentClose() {
        TestResource resource = new TestResource();

        try (ResourceManager manager = new ResourceManager()) {
            manager.register(resource);
        }

        assertThat(resource.isClosed()).isTrue();
        assertThat(resource.getCloseCount()).isEqualTo(1);
    }

    @Test
    @DisplayName("Should track closed state")
    void testClosedState() {
        ResourceManager manager = new ResourceManager();

        assertThat(manager.isClosed()).isFalse();

        manager.close();

        assertThat(manager.isClosed()).isTrue();
    }

    @Test
    @DisplayName("Should reject registration after close")
    void testRegisterAfterClose() {
        ResourceManager manager = new ResourceManager();
        manager.close();

        TestResource resource = new TestResource();

        assertThatThrownBy(() -> manager.register(resource))
            .isInstanceOf(IllegalStateException.class)
            .hasMessageContaining("closed");
    }

    @Test
    @DisplayName("Should create scoped manager")
    void testCreateScoped() {
        ResourceManager manager = ResourceManager.createScoped("scoped");

        assertThat(manager.toString()).contains("scoped");
        assertThat(manager.isClosed()).isFalse();
    }

    @Test
    @DisplayName("Should provide meaningful toString")
    void testToString() {
        ResourceManager manager = new ResourceManager("test-manager");
        TestResource resource = new TestResource();
        manager.register(resource);

        String str = manager.toString();

        assertThat(str)
            .contains("ResourceManager")
            .contains("test-manager")
            .contains("resources=1")
            .contains("closed=false");
    }

    // Helper classes for testing

    static class TestResource implements AutoCloseable {
        private final AtomicBoolean closed = new AtomicBoolean(false);
        private final AtomicInteger closeCount = new AtomicInteger(0);

        @Override
        public void close() {
            closed.set(true);
            closeCount.incrementAndGet();
        }

        public boolean isClosed() {
            return closed.get();
        }

        public int getCloseCount() {
            return closeCount.get();
        }
    }

    static class OrderedTestResource extends TestResource {
        private final AtomicInteger globalOrder;
        private int actualCloseOrder = -1;

        OrderedTestResource(AtomicInteger globalOrder, int expectedOrder) {
            this.globalOrder = globalOrder;
            // expectedOrder is used for test documentation
        }

        @Override
        public void close() {
            super.close();
            actualCloseOrder = globalOrder.incrementAndGet();
        }

        public int getCloseOrder() {
            return actualCloseOrder;
        }
    }

    static class FailingResource implements AutoCloseable {
        @Override
        public void close() throws IOException {
            throw new IOException("Intentional failure for testing");
        }
    }
}

