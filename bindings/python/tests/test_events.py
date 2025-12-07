"""Tests for events module."""

import pytest
import asyncio

from umicp.events import EventEmitter, Event, EventType


class TestEvent:
    """Test Event dataclass."""

    def test_create_event(self):
        """Test creating event."""
        event = Event(
            type=EventType.MESSAGE,
            data={"message": "test"},
            source="client-001"
        )

        assert event.type == EventType.MESSAGE
        assert event.data["message"] == "test"
        assert event.source == "client-001"


class TestEventEmitter:
    """Test EventEmitter class."""

    @pytest.mark.asyncio
    async def test_create_emitter(self):
        """Test creating event emitter."""
        emitter = EventEmitter()
        assert emitter is not None

    @pytest.mark.asyncio
    async def test_register_handler(self):
        """Test registering event handler."""
        emitter = EventEmitter()
        called = []

        async def handler(event: Event):
            called.append(event.data)

        await emitter.on(EventType.MESSAGE, handler)

        event = Event(type=EventType.MESSAGE, data={"test": "value"})
        await emitter.emit(event)

        await asyncio.sleep(0.1)  # Give time for handler to run
        assert len(called) == 1
        assert called[0]["test"] == "value"

    @pytest.mark.asyncio
    async def test_multiple_handlers(self):
        """Test multiple handlers for same event."""
        emitter = EventEmitter()
        called = []

        async def handler1(event: Event):
            called.append(1)

        async def handler2(event: Event):
            called.append(2)

        await emitter.on(EventType.MESSAGE, handler1)
        await emitter.on(EventType.MESSAGE, handler2)

        event = Event(type=EventType.MESSAGE, data={})
        await emitter.emit(event)

        await asyncio.sleep(0.1)
        assert len(called) == 2
        assert 1 in called
        assert 2 in called

    @pytest.mark.asyncio
    async def test_remove_handler(self):
        """Test removing event handler."""
        emitter = EventEmitter()
        called = []

        async def handler(event: Event):
            called.append(1)

        await emitter.on(EventType.MESSAGE, handler)
        await emitter.off(EventType.MESSAGE, handler)

        event = Event(type=EventType.MESSAGE, data={})
        await emitter.emit(event)

        await asyncio.sleep(0.1)
        assert len(called) == 0

    @pytest.mark.asyncio
    async def test_remove_all_listeners(self):
        """Test removing all listeners."""
        emitter = EventEmitter()

        async def handler(event: Event):
            pass

        await emitter.on(EventType.MESSAGE, handler)
        await emitter.on(EventType.ERROR, handler)

        emitter.remove_all_listeners()

        # No handlers should be present
        assert len(emitter._handlers) == 0

    @pytest.mark.asyncio
    async def test_different_event_types(self):
        """Test different event types."""
        emitter = EventEmitter()
        received = {}

        async def message_handler(event: Event):
            received["message"] = event.data

        async def error_handler(event: Event):
            received["error"] = event.data

        await emitter.on(EventType.MESSAGE, message_handler)
        await emitter.on(EventType.ERROR, error_handler)

        await emitter.emit(Event(type=EventType.MESSAGE, data={"msg": "hello"}))
        await emitter.emit(Event(type=EventType.ERROR, data={"err": "oops"}))

        await asyncio.sleep(0.1)
        assert "message" in received
        assert "error" in received
        assert received["message"]["msg"] == "hello"
        assert received["error"]["err"] == "oops"

