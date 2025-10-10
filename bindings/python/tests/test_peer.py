"""Tests for peer modules."""

import pytest

from umicp import (
    PeerInfo,
    PeerConnection,
    HandshakeProtocol,
    OperationType,
    ConnectionState,
)


class TestPeerInfo:
    """Test PeerInfo dataclass."""

    def test_create_peer_info(self):
        """Test creating peer info."""
        info = PeerInfo(
            id="peer-001",
            url="ws://localhost:8080",
            metadata={"name": "TestPeer"},
            capabilities={"type": "processor"}
        )

        assert info.id == "peer-001"
        assert info.url == "ws://localhost:8080"
        assert info.metadata["name"] == "TestPeer"
        assert info.capabilities["type"] == "processor"


class TestPeerConnection:
    """Test PeerConnection class."""

    def test_create_connection(self):
        """Test creating peer connection."""
        conn = PeerConnection("peer-001", "ws://localhost:8080")

        assert conn.peer_id == "peer-001"
        assert conn.url == "ws://localhost:8080"
        assert conn.state == ConnectionState.DISCONNECTED

    def test_is_connected(self):
        """Test checking connection status."""
        conn = PeerConnection("peer-001", "ws://localhost:8080")

        assert not conn.is_connected()

        conn.state = ConnectionState.CONNECTED
        assert conn.is_connected()


class TestHandshakeProtocol:
    """Test HandshakeProtocol class."""

    def test_create_hello(self):
        """Test creating HELLO envelope."""
        envelope = HandshakeProtocol.create_hello("client-001", "server-001")

        assert envelope.from_id == "client-001"
        assert envelope.to_id == "server-001"
        assert envelope.operation == OperationType.CONTROL
        assert envelope.capabilities.get("type") == "HELLO"

    def test_create_ack(self):
        """Test creating ACK envelope."""
        envelope = HandshakeProtocol.create_ack(
            "server-001", "client-001", "corr-123"
        )

        assert envelope.from_id == "server-001"
        assert envelope.to_id == "client-001"
        assert envelope.operation == OperationType.ACK
        assert envelope.correlation_id == "corr-123"
        assert envelope.capabilities.get("type") == "ACK"

    def test_is_hello(self):
        """Test checking if envelope is HELLO."""
        hello = HandshakeProtocol.create_hello("client", "server")
        assert HandshakeProtocol.is_hello(hello)

        ack = HandshakeProtocol.create_ack("server", "client", "corr-123")
        assert not HandshakeProtocol.is_hello(ack)

    def test_is_ack(self):
        """Test checking if envelope is ACK."""
        ack = HandshakeProtocol.create_ack("server", "client", "corr-123")
        assert HandshakeProtocol.is_ack(ack)

        hello = HandshakeProtocol.create_hello("client", "server")
        assert not HandshakeProtocol.is_ack(hello)

