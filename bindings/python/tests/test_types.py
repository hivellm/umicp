"""Tests for types module."""

import pytest
from datetime import datetime

from umicp.types import (
    OperationType,
    PayloadType,
    EncodingType,
    ConnectionState,
    PayloadHint,
    TransportStats,
)


class TestOperationType:
    """Test OperationType enum."""

    def test_operation_types(self):
        """Test all operation types."""
        assert OperationType.CONTROL == "control"
        assert OperationType.DATA == "data"
        assert OperationType.ACK == "ack"
        assert OperationType.ERROR == "error"
        assert OperationType.REQUEST == "request"
        assert OperationType.RESPONSE == "response"


class TestPayloadType:
    """Test PayloadType enum."""

    def test_payload_types(self):
        """Test all payload types."""
        assert PayloadType.VECTOR == "vector"
        assert PayloadType.TEXT == "text"
        assert PayloadType.METADATA == "metadata"
        assert PayloadType.BINARY == "binary"
        assert PayloadType.JSON == "json"
        assert PayloadType.MATRIX == "matrix"


class TestEncodingType:
    """Test EncodingType enum."""

    def test_encoding_types(self):
        """Test all encoding types."""
        assert EncodingType.FLOAT32 == "float32"
        assert EncodingType.FLOAT64 == "float64"
        assert EncodingType.INT32 == "int32"
        assert EncodingType.INT64 == "int64"
        assert EncodingType.UTF8 == "utf8"
        assert EncodingType.BASE64 == "base64"
        assert EncodingType.HEX == "hex"


class TestConnectionState:
    """Test ConnectionState enum."""

    def test_connection_states(self):
        """Test all connection states."""
        assert ConnectionState.DISCONNECTED == "disconnected"
        assert ConnectionState.CONNECTING == "connecting"
        assert ConnectionState.CONNECTED == "connected"
        assert ConnectionState.RECONNECTING == "reconnecting"
        assert ConnectionState.DISCONNECTING == "disconnecting"
        assert ConnectionState.ERROR == "error"


class TestPayloadHint:
    """Test PayloadHint dataclass."""

    def test_create_payload_hint(self):
        """Test creating payload hint."""
        hint = PayloadHint(
            type=PayloadType.VECTOR,
            size=1024,
            encoding=EncodingType.FLOAT32,
            count=256
        )

        assert hint.type == PayloadType.VECTOR
        assert hint.size == 1024
        assert hint.encoding == EncodingType.FLOAT32
        assert hint.count == 256

    def test_to_dict(self):
        """Test converting hint to dict."""
        hint = PayloadHint(
            type=PayloadType.VECTOR,
            size=1024,
            encoding=EncodingType.FLOAT32
        )

        data = hint.to_dict()
        assert data["type"] == "vector"
        assert data["size"] == 1024
        assert data["encoding"] == "float32"

    def test_from_dict(self):
        """Test creating hint from dict."""
        data = {
            "type": "vector",
            "size": 1024,
            "encoding": "float32",
            "count": 256
        }

        hint = PayloadHint.from_dict(data)
        assert hint.type == PayloadType.VECTOR
        assert hint.size == 1024
        assert hint.encoding == EncodingType.FLOAT32
        assert hint.count == 256


class TestTransportStats:
    """Test TransportStats dataclass."""

    def test_create_stats(self):
        """Test creating transport stats."""
        stats = TransportStats()

        assert stats.messages_sent == 0
        assert stats.messages_received == 0
        assert stats.bytes_sent == 0
        assert stats.bytes_received == 0
        assert stats.errors == 0
        assert stats.reconnections == 0

    def test_update_stats(self):
        """Test updating stats."""
        stats = TransportStats()
        stats.messages_sent = 10
        stats.bytes_sent = 1024

        assert stats.messages_sent == 10
        assert stats.bytes_sent == 1024

    def test_to_dict(self):
        """Test converting stats to dict."""
        now = datetime.utcnow()
        stats = TransportStats(
            messages_sent=10,
            messages_received=5,
            bytes_sent=1024,
            bytes_received=512,
            connected_at=now
        )

        data = stats.to_dict()
        assert data["messages_sent"] == 10
        assert data["messages_received"] == 5
        assert data["bytes_sent"] == 1024
        assert data["bytes_received"] == 512
        assert data["connected_at"] is not None

