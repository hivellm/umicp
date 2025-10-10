"""Advanced tests for envelope module."""

import pytest
from datetime import datetime

from umicp import (
    Envelope,
    EnvelopeBuilder,
    OperationType,
    PayloadType,
    EncodingType,
    PayloadHint,
    ValidationError,
    SerializationError,
)


class TestEnvelopeAdvanced:
    """Advanced envelope tests."""

    def test_envelope_with_all_fields(self):
        """Test envelope with all possible fields."""
        timestamp = datetime.utcnow()

        envelope = Envelope(
            from_id="client-001",
            to_id="server-001",
            operation=OperationType.REQUEST,
            message_id="msg-123",
            correlation_id="corr-456",
            timestamp=timestamp,
            capabilities={"type": "test", "version": "1.0"},
            payload_hints=[
                PayloadHint(
                    type=PayloadType.VECTOR,
                    size=1024,
                    encoding=EncodingType.FLOAT32,
                    count=256
                )
            ],
            payload_refs=["ref-001", "ref-002"]
        )

        assert envelope.from_id == "client-001"
        assert envelope.correlation_id == "corr-456"
        assert len(envelope.payload_hints) == 1
        assert len(envelope.payload_refs) == 2

    def test_envelope_all_operations(self):
        """Test all operation types."""
        operations = [
            OperationType.CONTROL,
            OperationType.DATA,
            OperationType.ACK,
            OperationType.ERROR,
            OperationType.REQUEST,
            OperationType.RESPONSE,
        ]

        for op in operations:
            envelope = EnvelopeBuilder() \
                .from_id("test") \
                .to_id("target") \
                .operation(op) \
                .build()

            assert envelope.operation == op

    def test_envelope_multiple_capabilities(self):
        """Test envelope with multiple capabilities."""
        envelope = EnvelopeBuilder() \
            .from_id("client") \
            .to_id("server") \
            .operation(OperationType.DATA) \
            .capability("key1", "value1") \
            .capability("key2", 123) \
            .capability("key3", {"nested": "object"}) \
            .capability("key4", [1, 2, 3]) \
            .build()

        assert len(envelope.capabilities) == 4
        assert envelope.capabilities["key1"] == "value1"
        assert envelope.capabilities["key2"] == 123
        assert envelope.capabilities["key3"]["nested"] == "object"
        assert envelope.capabilities["key4"] == [1, 2, 3]

    def test_envelope_multiple_payload_hints(self):
        """Test envelope with multiple payload hints."""
        envelope = EnvelopeBuilder() \
            .from_id("client") \
            .to_id("server") \
            .operation(OperationType.DATA) \
            .payload_hint(PayloadHint(type=PayloadType.VECTOR, size=1024)) \
            .payload_hint(PayloadHint(type=PayloadType.TEXT, encoding=EncodingType.UTF8)) \
            .payload_hint(PayloadHint(type=PayloadType.BINARY)) \
            .build()

        assert len(envelope.payload_hints) == 3
        assert envelope.payload_hints[0].type == PayloadType.VECTOR
        assert envelope.payload_hints[1].type == PayloadType.TEXT
        assert envelope.payload_hints[2].type == PayloadType.BINARY

    def test_envelope_multiple_payload_refs(self):
        """Test envelope with multiple payload references."""
        envelope = EnvelopeBuilder() \
            .from_id("client") \
            .to_id("server") \
            .operation(OperationType.DATA) \
            .payload_ref("ref-001") \
            .payload_ref("ref-002") \
            .payload_ref("ref-003") \
            .build()

        assert len(envelope.payload_refs) == 3
        assert "ref-001" in envelope.payload_refs
        assert "ref-003" in envelope.payload_refs

    def test_envelope_serialization_with_hints(self):
        """Test serialization with payload hints."""
        envelope = EnvelopeBuilder() \
            .from_id("client") \
            .to_id("server") \
            .operation(OperationType.DATA) \
            .payload_hint(PayloadHint(
                type=PayloadType.VECTOR,
                size=1024,
                encoding=EncodingType.FLOAT32,
                count=256,
                compression="gzip"
            )) \
            .build()

        json_str = envelope.to_json()
        received = Envelope.from_json(json_str)

        assert len(received.payload_hints) == 1
        assert received.payload_hints[0].type == PayloadType.VECTOR
        assert received.payload_hints[0].size == 1024
        assert received.payload_hints[0].compression == "gzip"

    def test_envelope_dict_conversion(self):
        """Test dict conversion."""
        envelope = EnvelopeBuilder() \
            .from_id("client") \
            .to_id("server") \
            .operation(OperationType.DATA) \
            .capability("test", "value") \
            .build()

        data = envelope.to_dict()

        assert data["from"] == "client"
        assert data["to"] == "server"
        assert data["operation"] == "data"
        assert data["capabilities"]["test"] == "value"
        assert "hash" in data
        assert "timestamp" in data

    def test_envelope_from_dict_minimal(self):
        """Test creating envelope from minimal dict."""
        data = {
            "from": "client",
            "to": "server",
            "operation": "data",
            "message_id": "msg-001",
            "timestamp": "2025-10-10T12:00:00Z"
        }

        envelope = Envelope.from_dict(data)
        assert envelope.from_id == "client"
        assert envelope.to_id == "server"
        assert envelope.operation == OperationType.DATA

    def test_envelope_from_dict_complete(self):
        """Test creating envelope from complete dict."""
        data = {
            "from": "client",
            "to": "server",
            "operation": "request",
            "message_id": "msg-001",
            "correlation_id": "corr-001",
            "timestamp": "2025-10-10T12:00:00Z",
            "capabilities": {"key": "value"},
            "payload_hints": [
                {"type": "vector", "size": 1024, "encoding": "float32"}
            ],
            "payload_refs": ["ref-001"],
            "hash": "abc123"
        }

        envelope = Envelope.from_dict(data)
        assert envelope.correlation_id == "corr-001"
        assert len(envelope.payload_hints) == 1
        assert len(envelope.payload_refs) == 1
        assert envelope.hash_value == "abc123"

    def test_envelope_hash_consistency(self):
        """Test hash computation consistency."""
        envelope = EnvelopeBuilder() \
            .from_id("client") \
            .to_id("server") \
            .operation(OperationType.DATA) \
            .message_id("msg-fixed") \
            .capability("key", "value") \
            .build()

        # Hash should be consistent when computed multiple times
        hash1 = envelope.compute_hash()
        hash2 = envelope.compute_hash()
        hash3 = envelope.compute_hash()

        assert hash1 == hash2 == hash3
        assert len(hash1) == 64  # SHA-256 produces 64 hex characters

    def test_envelope_invalid_json(self):
        """Test deserializing invalid JSON."""
        with pytest.raises(SerializationError):
            Envelope.from_json("invalid json {{{")

    def test_envelope_missing_required_fields(self):
        """Test deserializing with missing fields."""
        data = {"from": "client"}  # Missing required fields

        with pytest.raises(SerializationError):
            Envelope.from_dict(data)

    def test_envelope_builder_missing_from(self):
        """Test builder with missing from_id."""
        with pytest.raises(ValidationError):
            EnvelopeBuilder() \
                .to_id("server") \
                .operation(OperationType.DATA) \
                .build()

    def test_envelope_builder_missing_to(self):
        """Test builder with missing to_id."""
        with pytest.raises(ValidationError):
            EnvelopeBuilder() \
                .from_id("client") \
                .operation(OperationType.DATA) \
                .build()

    def test_envelope_builder_missing_operation(self):
        """Test builder with missing operation."""
        with pytest.raises(ValidationError):
            EnvelopeBuilder() \
                .from_id("client") \
                .to_id("server") \
                .build()

    def test_envelope_repr(self):
        """Test envelope string representation."""
        envelope = Envelope(
            from_id="client",
            to_id="server",
            operation=OperationType.DATA,
            message_id="msg-123"
        )

        repr_str = repr(envelope)
        assert "client" in repr_str
        assert "server" in repr_str
        assert "data" in repr_str
        assert "msg-123" in repr_str

    def test_envelope_timestamp_formats(self):
        """Test different timestamp formats."""
        # ISO format with Z
        data1 = {
            "from": "client",
            "to": "server",
            "operation": "data",
            "timestamp": "2025-10-10T12:00:00Z"
        }
        envelope1 = Envelope.from_dict(data1)
        assert envelope1.timestamp is not None

        # ISO format with timezone
        data2 = {
            "from": "client",
            "to": "server",
            "operation": "data",
            "timestamp": "2025-10-10T12:00:00+00:00"
        }
        envelope2 = Envelope.from_dict(data2)
        assert envelope2.timestamp is not None

