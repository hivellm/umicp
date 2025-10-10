"""Integration tests for UMICP Python bindings."""

import pytest
import asyncio
import numpy as np

from umicp import (
    Envelope,
    EnvelopeBuilder,
    OperationType,
    Matrix,
    EventEmitter,
    Event,
    EventType,
    ServiceDiscovery,
    ServiceInfo,
    ConnectionPool,
    PoolConfig,
)


class TestEndToEnd:
    """End-to-end integration tests."""

    def test_envelope_serialization_roundtrip(self):
        """Test envelope serialization and deserialization."""
        # Create envelope
        original = EnvelopeBuilder() \
            .from_id("client-001") \
            .to_id("server-001") \
            .operation(OperationType.DATA) \
            .capability("test", "value") \
            .capability("number", 42) \
            .build()

        # Serialize
        json_str = original.to_json()
        assert len(json_str) > 0

        # Deserialize
        deserialized = Envelope.from_json(json_str)

        # Verify
        assert deserialized.from_id == original.from_id
        assert deserialized.to_id == original.to_id
        assert deserialized.operation == original.operation
        assert deserialized.capabilities == original.capabilities

    def test_matrix_operations_pipeline(self):
        """Test matrix operations pipeline."""
        matrix = Matrix()

        # Create vectors
        v1 = np.array([1.0, 2.0, 3.0, 4.0], dtype=np.float32)
        v2 = np.array([5.0, 6.0, 7.0, 8.0], dtype=np.float32)

        # Operations
        v_sum = matrix.vector_add(v1, v2)
        v_norm = matrix.normalize(v_sum)
        dot_result = matrix.dot_product(v1, v2)
        similarity = matrix.cosine_similarity(v1, v2)

        # Verify results exist
        assert v_sum is not None
        assert v_norm is not None
        assert dot_result.result > 0
        assert 0 <= similarity.similarity <= 1

    @pytest.mark.asyncio
    async def test_event_system_workflow(self):
        """Test event system workflow."""
        emitter = EventEmitter()
        received_events = []

        async def handler(event: Event):
            received_events.append(event)

        # Register handler
        await emitter.on(EventType.MESSAGE, handler)

        # Emit events
        for i in range(3):
            event = Event(
                type=EventType.MESSAGE,
                data={"index": i}
            )
            await emitter.emit(event)

        await asyncio.sleep(0.2)

        # Verify
        assert len(received_events) == 3
        assert received_events[0].data["index"] == 0
        assert received_events[2].data["index"] == 2

    @pytest.mark.asyncio
    async def test_service_discovery_lifecycle(self):
        """Test service discovery full lifecycle."""
        discovery = ServiceDiscovery()
        await discovery.start()

        try:
            # Register services
            service1 = ServiceInfo(
                id="svc-001",
                name="Service1",
                version="1.0.0",
                capabilities={"type": "api", "protocol": "http"}
            )

            service2 = ServiceInfo(
                id="svc-002",
                name="Service2",
                version="2.0.0",
                capabilities={"type": "worker", "protocol": "websocket"}
            )

            await discovery.register(service1)
            await discovery.register(service2)

            # Find by capability
            apis = await discovery.find_by_capability("type", "api")
            assert len(apis) == 1
            assert apis[0].id == "svc-001"

            # Update health
            await discovery.update_health("svc-001", "degraded")

            # Verify update
            found = await discovery.find_by_id("svc-001")
            assert found.health_status == "degraded"

            # List all
            all_services = await discovery.list_all()
            assert len(all_services) == 2

            # Unregister
            await discovery.unregister("svc-001")
            all_services = await discovery.list_all()
            assert len(all_services) == 1

        finally:
            await discovery.stop()

    @pytest.mark.asyncio
    async def test_connection_pool_workflow(self):
        """Test connection pool workflow."""
        config = PoolConfig(min_size=1, max_size=5)
        pool = ConnectionPool(config)

        await pool.start()

        try:
            # Add connections
            connections = []
            for i in range(3):
                conn = {"id": f"conn-{i:03d}", "active": True}
                await pool.add_connection(conn)
                connections.append(conn)

            assert pool.size() == 3

            # Acquire and release
            conn1 = await pool.acquire()
            assert conn1 is not None
            assert pool.in_use_count() == 1

            conn2 = await pool.acquire()
            assert conn2 is not None
            assert pool.in_use_count() == 2

            # Release
            await pool.release(conn1)
            await asyncio.sleep(0.1)

            # Verify availability
            assert pool.available_count() > 0

        finally:
            await pool.stop()


class TestCrossModuleIntegration:
    """Test integration between different modules."""

    @pytest.mark.asyncio
    async def test_envelope_with_events(self):
        """Test envelope creation with event emission."""
        emitter = EventEmitter()
        envelopes_created = []

        async def envelope_handler(event: Event):
            envelopes_created.append(event.data["envelope"])

        await emitter.on(EventType.MESSAGE, envelope_handler)

        # Create and emit
        envelope = EnvelopeBuilder() \
            .from_id("test") \
            .to_id("target") \
            .operation(OperationType.DATA) \
            .build()

        await emitter.emit(Event(
            type=EventType.MESSAGE,
            data={"envelope": envelope}
        ))

        await asyncio.sleep(0.1)

        assert len(envelopes_created) == 1
        assert envelopes_created[0].from_id == "test"

    def test_matrix_with_envelope(self):
        """Test matrix operations with envelope transport."""
        matrix = Matrix()

        # Create vector
        vector = np.array([1.0, 2.0, 3.0], dtype=np.float32)

        # Normalize
        normalized = matrix.normalize(vector)

        # Create envelope with result
        envelope = EnvelopeBuilder() \
            .from_id("processor") \
            .to_id("client") \
            .operation(OperationType.RESPONSE) \
            .capability("vector_size", len(normalized)) \
            .capability("normalized", True) \
            .build()

        assert envelope.capabilities["vector_size"] == 3
        assert envelope.capabilities["normalized"] is True

