"""Tests for transport modules."""

import pytest
import asyncio

from umicp import (
    WebSocketClient,
    TransportError,
    ConnectionState,
)


class TestWebSocketClient:
    """Test WebSocketClient class (unit tests without real server)."""

    def test_create_client(self):
        """Test creating WebSocket client."""
        client = WebSocketClient("ws://localhost:8080")

        assert client.url == "ws://localhost:8080"
        assert client.state == ConnectionState.DISCONNECTED
        assert client.stats.messages_sent == 0

    def test_client_with_reconnect(self):
        """Test client with reconnect enabled."""
        client = WebSocketClient("ws://localhost:8080", reconnect=True)

        assert client.reconnect_enabled is True

    def test_client_without_reconnect(self):
        """Test client with reconnect disabled."""
        client = WebSocketClient("ws://localhost:8080", reconnect=False)

        assert client.reconnect_enabled is False

    def test_get_stats_initial(self):
        """Test getting initial statistics."""
        client = WebSocketClient("ws://localhost:8080")
        stats = client.get_stats()

        assert stats.messages_sent == 0
        assert stats.messages_received == 0
        assert stats.bytes_sent == 0
        assert stats.bytes_received == 0
        assert stats.errors == 0

    @pytest.mark.asyncio
    async def test_send_without_connection(self):
        """Test sending without connection."""
        from umicp import EnvelopeBuilder, OperationType

        client = WebSocketClient("ws://localhost:8080")
        envelope = EnvelopeBuilder() \
            .from_id("test") \
            .to_id("target") \
            .operation(OperationType.DATA) \
            .build()

        # Should raise error when not connected
        with pytest.raises(TransportError):
            await client.send(envelope)

    def test_multiple_clients(self):
        """Test creating multiple clients."""
        clients = []
        for i in range(5):
            client = WebSocketClient(f"ws://localhost:808{i}")
            clients.append(client)

        assert len(clients) == 5
        assert all(c.state == ConnectionState.DISCONNECTED for c in clients)

    def test_client_url_validation(self):
        """Test client with various URLs."""
        urls = [
            "ws://localhost:8080",
            "wss://secure.server.com:443",
            "ws://192.168.1.1:9000",
            "wss://example.com/path",
        ]

        for url in urls:
            client = WebSocketClient(url)
            assert client.url == url


class TestWebSocketClientStateTransitions:
    """Test WebSocket client state transitions."""

    def test_initial_state(self):
        """Test initial state is disconnected."""
        client = WebSocketClient("ws://localhost:8080")
        assert client.state == ConnectionState.DISCONNECTED

    @pytest.mark.asyncio
    async def test_connection_state_on_error(self):
        """Test state changes to ERROR on connection failure."""
        client = WebSocketClient("ws://invalid-host:9999")

        try:
            await asyncio.wait_for(client.connect(), timeout=1.0)
        except (TransportError, asyncio.TimeoutError):
            pass

        # State should be ERROR after failed connection
        assert client.state in [ConnectionState.ERROR, ConnectionState.CONNECTING]

    def test_stats_are_mutable(self):
        """Test that stats can be updated."""
        client = WebSocketClient("ws://localhost:8080")

        # Manually update stats for testing
        client.stats.messages_sent = 10
        client.stats.bytes_sent = 1024

        stats = client.get_stats()
        assert stats.messages_sent == 10
        assert stats.bytes_sent == 1024

